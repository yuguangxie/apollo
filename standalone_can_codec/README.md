# standalone_can_codec（C++版）

这是一个从 Apollo `gen_vehicle_protocol` 相关逻辑中抽取并解耦出的**独立 C++ CAN 报文编解码仓库示例**。

目标：

1. 使用 DBC 文件生成 Apollo 风格（位域切分 + Byte.set/get）的 C++ 编解码代码；
2. 不依赖 Apollo 运行时（不依赖 `ProtocolData` / chassis proto / cyber）；
3. 代码可直接编译、运行、测试。

---

## 1. 仓库结构

```text
standalone_can_codec/
├── CMakeLists.txt
├── include/
│   └── can_codec/
│       └── byte.h                    # 轻量版 Byte 类（兼容 Apollo 思路）
├── tools/
│   └── dbc_codegen.py                # DBC -> C++ 编解码代码生成器
├── examples/
│   ├── demo.dbc                      # 示例 DBC
│   └── generated/
│       ├── demo_can_codec.h          # 由 dbc_codegen.py 生成
│       └── demo_can_codec.cc         # 由 dbc_codegen.py 生成
└── tests/
    └── test_generated_codec.cc       # 可执行测试
```

---

## 2. 生成 C++ 编解码代码（完整示例）

下面命令会读取 `examples/demo.dbc`，生成完整 C++ 编解码代码：

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

> 该生成代码风格与 Apollo 的 `gen_protocols.py` 核心思想一致：
> - Intel/Motorola 按 bit 段切分；
> - 多字节拼接/拆分；
> - signed 信号符号扩展；
> - factor/offset 物理量换算。

---

## 3. 直接编译与测试

```bash
cd standalone_can_codec
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

预期：`test_generated_codec` 通过。

---

## 4. 使用方式（生成后的 C++ 代码）

`DemoCanCodec` 暴露了每个 DBC message 对应的结构体和静态函数：

- `Encode<Message>(const Message&) -> std::array<uint8_t, DLC>`
- `Decode<Message>(const uint8_t* data, size_t len) -> Message`

示例（来自测试）：

```cpp
DemoCanCodec::VehStatus in;
in.speed = 88.8;
in.steer_angle = -12.3;

auto payload = DemoCanCodec::EncodeVehStatus(in);
auto out = DemoCanCodec::DecodeVehStatus(payload.data(), payload.size());
```

---

## 5. DBC 支持范围（当前版本）

当前生成器支持以下常见语法：

- `BO_`：报文 ID、名称、DLC
- `SG_`：start_bit、len、@0/@1、+/-、factor、offset

当前示例不包含枚举 `VAL_` 到 C++ enum 的生成（后续可继续扩展）。

---

## 6. 与 Apollo 依赖解耦说明

该项目仅保留 Apollo 风格的核心位操作算法，不依赖：

- `modules/drivers/canbus` 运行时库
- `ProtocolData`/`MessageManager`
- protobuf chassis 结构
- cyber runtime

因此可作为独立 C++ 项目在任意标准环境下编译运行。
