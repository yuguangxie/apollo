#include <array>
#include <cmath>
#include <iostream>

#include "demo_can_codec.h"

using standalone_can_codec::DemoCanCodec;

int main() {
  {
    DemoCanCodec::VehStatus in;
    in.speed = 88.8;
    in.steer_angle = -12.3;

    auto payload = DemoCanCodec::EncodeVehStatus(in);
    const std::array<std::uint8_t, 8> expected = {0x77, 0x03, 0x85, 0xFF,
                                                  0x00, 0x00, 0x00, 0x00};
    if (payload != expected) {
      std::cerr << "VEH_STATUS encoded payload mismatch" << std::endl;
      return 1;
    }

    auto decoded = DemoCanCodec::DecodeVehStatus(expected.data(), expected.size());
    if (std::fabs(decoded.speed - 88.7) > 0.11 ||
        std::fabs(decoded.steer_angle - (-12.3)) > 0.11) {
      std::cerr << "VEH_STATUS decode mismatch" << std::endl;
      return 1;
    }
  }

  {
    DemoCanCodec::CtrlCmd in;
    in.throttle_cmd = 35.4;
    in.brake_cmd = 12.1;

    auto payload = DemoCanCodec::EncodeCtrlCmd(in);
    const std::array<std::uint8_t, 8> expected = {0x16, 0x10, 0x07, 0x80,
                                                  0x00, 0x00, 0x00, 0x00};
    if (payload != expected) {
      std::cerr << "CTRL_CMD encoded payload mismatch" << std::endl;
      return 1;
    }

    auto decoded = DemoCanCodec::DecodeCtrlCmd(expected.data(), expected.size());
    if (std::fabs(decoded.throttle_cmd - 35.3) > 0.11 ||
        std::fabs(decoded.brake_cmd - 12.0) > 0.11) {
      std::cerr << "CTRL_CMD decode mismatch" << std::endl;
      return 1;
    }
  }

  std::cout << "known payload tests passed" << std::endl;
  return 0;
}
