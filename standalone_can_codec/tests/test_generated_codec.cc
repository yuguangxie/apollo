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
    auto out = DemoCanCodec::DecodeVehStatus(payload.data(), payload.size());

    if (std::fabs(out.speed - in.speed) > 0.11) {
      std::cerr << "speed mismatch: " << out.speed << " vs " << in.speed << '\n';
      return 1;
    }
    if (std::fabs(out.steer_angle - in.steer_angle) > 0.11) {
      std::cerr << "steer mismatch: " << out.steer_angle << " vs " << in.steer_angle << '\n';
      return 1;
    }
  }

  {
    DemoCanCodec::CtrlCmd in;
    in.throttle_cmd = 35.4;
    in.brake_cmd = 12.1;

    auto payload = DemoCanCodec::EncodeCtrlCmd(in);
    auto out = DemoCanCodec::DecodeCtrlCmd(payload.data(), payload.size());

    if (std::fabs(out.throttle_cmd - in.throttle_cmd) > 0.11) {
      std::cerr << "throttle mismatch\n";
      return 1;
    }
    if (std::fabs(out.brake_cmd - in.brake_cmd) > 0.11) {
      std::cerr << "brake mismatch\n";
      return 1;
    }
  }

  std::cout << "all tests passed\n";
  return 0;
}
