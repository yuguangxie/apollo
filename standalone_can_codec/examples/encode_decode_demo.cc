#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "demo_can_codec.h"

using standalone_can_codec::DemoCanCodec;

namespace {
void PrintHex(const std::array<std::uint8_t, 8>& payload) {
  for (std::size_t i = 0; i < payload.size(); ++i) {
    std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
              << static_cast<int>(payload[i]);
    if (i + 1 != payload.size()) {
      std::cout << ' ';
    }
  }
  std::cout << std::dec << '\n';
}
}  // namespace

int main() {
  // 1) 将物理量编码为 CAN payload。
  DemoCanCodec::VehStatus status_in;
  status_in.speed = 88.8;
  status_in.steer_angle = -12.3;

  auto encoded_status = DemoCanCodec::EncodeVehStatus(status_in);
  std::cout << "Encoded VEH_STATUS(0x100): ";
  PrintHex(encoded_status);

  // 2) 将原始 CAN payload 解码为物理量。
  std::array<std::uint8_t, 8> raw_ctrl_cmd = {0x16, 0x10, 0x07, 0x80,
                                               0x00, 0x00, 0x00, 0x00};
  auto decoded_ctrl =
      DemoCanCodec::DecodeCtrlCmd(raw_ctrl_cmd.data(), raw_ctrl_cmd.size());
  std::cout << "Decoded CTRL_CMD(0x200): throttle=" << decoded_ctrl.throttle_cmd
            << ", brake=" << decoded_ctrl.brake_cmd << '\n';

  return 0;
}
