# standalone_can_codec（C++版）

这是一个从 Apollo `gen_vehicle_protocol` 相关逻辑抽取出的 **独立 C++ CAN 报文编解码仓库**。

你可以用它做两件事：

1. 通过 `.dbc` **生成 C++ 编解码代码**；
2. 在 C++ 里把“物理量数据”**编码成 CAN 报文**，或把“CAN 报文”**解码成物理量**。

---

## 1. 目录结构

```text
standalone_can_codec/
├── CMakeLists.txt
├── include/
│   └── can_codec/
│       └── byte.h
├── tools/
│   └── dbc_codegen.py
├── examples/
│   ├── demo.dbc
│   ├── encode_decode_demo.cc
│   └── generated/
│       ├── demo_can_codec.h
│       └── demo_can_codec.cc
└── tests/
    ├── test_generated_codec.cc
    └── test_known_payload.cc
```

---

## 2. 从 DBC 生成 C++ 编解码代码

> 这一步会把 DBC 中 `BO_` / `SG_` 转成可直接调用的 C++ 类。

```bash
cd standalone_can_codec
python3 tools/dbc_codegen.py \
  --dbc examples/demo.dbc \
  --out-dir examples/generated \
  --base-name demo_can_codec \
  --class-name DemoCanCodec
```

生成结果：

- `examples/generated/demo_can_codec.h`
- `examples/generated/demo_can_codec.cc`

---

## 3. 编译、测试、运行示例

```bash
cd standalone_can_codec
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/encode_decode_demo
```

---

## 4. 详细代码示例：编码成 CAN 报文

下面代码把物理量编码成 CAN payload（8 字节）：

```cpp
#include "demo_can_codec.h"

using standalone_can_codec::DemoCanCodec;

DemoCanCodec::VehStatus in;
in.speed = 88.8;
in.steer_angle = -12.3;

auto payload = DemoCanCodec::EncodeVehStatus(in);
// payload 预期: 77 03 85 FF 00 00 00 00
```

对应测试见：`tests/test_known_payload.cc`。

---

## 5. 详细代码示例：解码 CAN 报文

下面代码把原始 CAN payload 解码成物理量：

```cpp
#include <array>
#include "demo_can_codec.h"

using standalone_can_codec::DemoCanCodec;

std::array<std::uint8_t, 8> raw = {
  0x16, 0x10, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00
};

auto decoded = DemoCanCodec::DecodeCtrlCmd(raw.data(), raw.size());
// decoded.throttle_cmd == 35.3
// decoded.brake_cmd    == 12.0
```

对应测试见：`tests/test_known_payload.cc`。

---

## 6. 编码 + 解码完整示例程序

文件：`examples/encode_decode_demo.cc`

- 先编码 `VehStatus`：`speed=88.8`, `steer_angle=-12.3`
- 再解码原始 `CtrlCmd` CAN 报文：`16 10 07 80 00 00 00 00`

运行：

```bash
./build/encode_decode_demo
```

---

## 7. 当前支持范围

`dbc_codegen.py` 当前支持：

- `BO_`：报文 ID、名称、DLC
- `SG_`：start_bit / length / byte order / signed / factor / offset

并生成：

- `Encode<Message>(const Message&)`
- `Decode<Message>(const uint8_t* data, size_t len)`

---

## 8. 上传到你的 GitHub 仓库（完整步骤）

假设你已经在 GitHub 创建了新仓库：`git@github.com:<your_name>/standalone_can_codec.git`

在 `apollo` 仓库根目录执行：

```bash
# 1) 提取子目录为独立仓库（可选，推荐）
mkdir -p /tmp/standalone_can_codec_export
rsync -av --delete standalone_can_codec/ /tmp/standalone_can_codec_export/
cd /tmp/standalone_can_codec_export

# 2) 初始化 git
git init
git add .
git commit -m "init: standalone C++ DBC CAN codec with codegen and tests"

# 3) 关联你的 GitHub 远端
git branch -M main
git remote add origin git@github.com:<your_name>/standalone_can_codec.git

# 4) 推送
git push -u origin main
```

如果你使用 HTTPS：

```bash
git remote add origin https://github.com/<your_name>/standalone_can_codec.git
git push -u origin main
```

---

## 9. 与 Apollo 的关系

本仓库保留 Apollo 风格位域处理算法思路（Intel/Motorola 位段切分、signed 扩展、factor/offset 换算），
但不依赖 Apollo 运行时组件，可独立在标准 C++17 环境下构建运行。
