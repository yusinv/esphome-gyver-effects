#pragma once

#include "esphome/core/color.h"
#include "esphome/components/light/esp_hsv_color.h"

namespace esphome {
namespace gyver_effects {

struct GRGB;

struct GHSV : light::ESPHSVColor {
  using light::ESPHSVColor::ESPHSVColor;
  // GRGB to_grgb() const ESPHOME_ALWAYS_INLINE;
};

struct GRGB : Color {
  using Color::Color;

  /// construction from Color
  inline GRGB(const Color &rhs) ESPHOME_ALWAYS_INLINE : Color(rhs) {}

  /// Allow assignment from one Color struct to GRGB
  inline GRGB &operator=(const Color &rhs) ESPHOME_ALWAYS_INLINE {
    this->raw_32 = rhs.raw_32;
    return *this;
  }

  /// Allow assignment from one GHSV struct to GRGB
  inline GRGB &operator=(const GHSV &rhs) ESPHOME_ALWAYS_INLINE {
    *this = static_cast<GRGB>(rhs.to_rgb());
    return *this;
  }

  /// Right shift each of the channels by a constant
  inline GRGB &operator>>=(uint8_t d) ESPHOME_ALWAYS_INLINE {
    r >>= d;
    g >>= d;
    b >>= d;
    return *this;
  }
};

}  // namespace gyver_effects
}  // namespace esphome