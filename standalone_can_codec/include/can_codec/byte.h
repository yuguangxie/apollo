#pragma once

#include <algorithm>
#include <cstdint>

namespace standalone_can_codec {

class Byte final {
 public:
  explicit Byte(uint8_t* value) : value_(value) {}
  explicit Byte(const uint8_t* value)
      : value_(const_cast<uint8_t*>(value)) {}

  void set_value(uint8_t value, int32_t start_pos, int32_t length) {
    if (!value_ || start_pos > 7 || start_pos < 0 || length < 1) return;
    int32_t end_pos = std::min(start_pos + length - 1, 7);
    int32_t real_len = end_pos + 1 - start_pos;

    static constexpr uint8_t RANGE_MASK_1_L[] = {0x01, 0x03, 0x07, 0x0F,
                                                  0x1F, 0x3F, 0x7F, 0xFF};
    static constexpr uint8_t RANGE_MASK_0_L[] = {0xFE, 0xFC, 0xF8, 0xF0,
                                                  0xE0, 0xC0, 0x80, 0x00};

    uint8_t current_low = 0x00;
    if (start_pos > 0) current_low = *value_ & RANGE_MASK_1_L[start_pos - 1];
    uint8_t current_high = *value_ & RANGE_MASK_0_L[end_pos];

    uint8_t middle = value & RANGE_MASK_1_L[real_len - 1];
    middle = static_cast<uint8_t>(middle << start_pos);

    *value_ = static_cast<uint8_t>(current_high + middle + current_low);
  }

  uint8_t get_byte(int32_t start_pos, int32_t length) const {
    if (!value_ || start_pos > 7 || start_pos < 0 || length < 1) return 0;
    int32_t end_pos = std::min(start_pos + length - 1, 7);
    int32_t real_len = end_pos + 1 - start_pos;
    static constexpr uint8_t RANGE_MASK_1_L[] = {0x01, 0x03, 0x07, 0x0F,
                                                  0x1F, 0x3F, 0x7F, 0xFF};

    uint8_t result = static_cast<uint8_t>(*value_ >> start_pos);
    result &= RANGE_MASK_1_L[real_len - 1];
    return result;
  }

 private:
  uint8_t* value_ = nullptr;
};

}  // namespace standalone_can_codec
