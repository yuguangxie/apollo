#include "demo_can_codec.h"

#include <stdexcept>

#include "can_codec/byte.h"

namespace standalone_can_codec {

std::array<std::uint8_t, 8> DemoCanCodec::EncodeVehStatus(const VehStatus& input) {
  std::array<std::uint8_t, 8> out{};
  std::int32_t x_0 = static_cast<std::int32_t>((input.speed - 0.0) / 0.1);
  std::uint8_t t_0_0 = static_cast<std::uint8_t>(x_0 & 0xff);
  Byte(out.data() + 0).set_value(t_0_0, 0, 8);
  x_0 >>= 8;
  std::uint8_t t_0_1 = static_cast<std::uint8_t>(x_0 & 0xff);
  Byte(out.data() + 1).set_value(t_0_1, 0, 8);
  std::int32_t x_1 = static_cast<std::int32_t>((input.steer_angle - 0.0) / 0.1);
  std::uint8_t t_1_0 = static_cast<std::uint8_t>(x_1 & 0xff);
  Byte(out.data() + 2).set_value(t_1_0, 0, 8);
  x_1 >>= 8;
  std::uint8_t t_1_1 = static_cast<std::uint8_t>(x_1 & 0xff);
  Byte(out.data() + 3).set_value(t_1_1, 0, 8);
  return out;
}

DemoCanCodec::VehStatus DemoCanCodec::DecodeVehStatus(const std::uint8_t* data, std::size_t len) {
  if (len < 8) throw std::runtime_error("invalid frame length");
  VehStatus out;
  std::int32_t x_0 = 0;
  x_0 = Byte(data + 1).get_byte(0, 8);
  x_0 = (x_0 << 8) | Byte(data + 0).get_byte(0, 8);
  out.speed = static_cast<double>(x_0) * 0.1 + 0.0;
  std::int32_t x_1 = 0;
  x_1 = Byte(data + 3).get_byte(0, 8);
  x_1 = (x_1 << 8) | Byte(data + 2).get_byte(0, 8);
  x_1 <<= 16;
  x_1 >>= 16;
  out.steer_angle = static_cast<double>(x_1) * 0.1 + 0.0;
  return out;
}

std::array<std::uint8_t, 8> DemoCanCodec::EncodeCtrlCmd(const CtrlCmd& input) {
  std::array<std::uint8_t, 8> out{};
  std::int32_t x_0 = static_cast<std::int32_t>((input.throttle_cmd - 0.0) / 0.1);
  std::uint8_t t_0_0 = static_cast<std::uint8_t>(x_0 & 0xf);
  Byte(out.data() + 1).set_value(t_0_0, 4, 4);
  x_0 >>= 4;
  std::uint8_t t_0_1 = static_cast<std::uint8_t>(x_0 & 0xff);
  Byte(out.data() + 0).set_value(t_0_1, 0, 8);
  std::int32_t x_1 = static_cast<std::int32_t>((input.brake_cmd - 0.0) / 0.1);
  std::uint8_t t_1_0 = static_cast<std::uint8_t>(x_1 & 0xf);
  Byte(out.data() + 3).set_value(t_1_0, 4, 4);
  x_1 >>= 4;
  std::uint8_t t_1_1 = static_cast<std::uint8_t>(x_1 & 0xff);
  Byte(out.data() + 2).set_value(t_1_1, 0, 8);
  return out;
}

DemoCanCodec::CtrlCmd DemoCanCodec::DecodeCtrlCmd(const std::uint8_t* data, std::size_t len) {
  if (len < 8) throw std::runtime_error("invalid frame length");
  CtrlCmd out;
  std::int32_t x_0 = 0;
  x_0 = Byte(data + 0).get_byte(0, 8);
  x_0 = (x_0 << 4) | Byte(data + 1).get_byte(4, 4);
  out.throttle_cmd = static_cast<double>(x_0) * 0.1 + 0.0;
  std::int32_t x_1 = 0;
  x_1 = Byte(data + 2).get_byte(0, 8);
  x_1 = (x_1 << 4) | Byte(data + 3).get_byte(4, 4);
  out.brake_cmd = static_cast<double>(x_1) * 0.1 + 0.0;
  return out;
}

}  // namespace standalone_can_codec
