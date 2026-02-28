#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace standalone_can_codec {

class DemoCanCodec final {
 public:
  struct VehStatus {
    double speed = 0.0;
    double steer_angle = 0.0;
  };
  static constexpr std::uint32_t ID_VEHSTATUS = 0x100;
  static std::array<std::uint8_t, 8> EncodeVehStatus(const VehStatus& input);
  static VehStatus DecodeVehStatus(const std::uint8_t* data, std::size_t len);

  struct CtrlCmd {
    double throttle_cmd = 0.0;
    double brake_cmd = 0.0;
  };
  static constexpr std::uint32_t ID_CTRLCMD = 0x200;
  static std::array<std::uint8_t, 8> EncodeCtrlCmd(const CtrlCmd& input);
  static CtrlCmd DecodeCtrlCmd(const std::uint8_t* data, std::size_t len);

};

}  // namespace standalone_can_codec
