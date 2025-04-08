
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace gyver_effects {

/// @file scale8.h
/// Fast, efficient 8-bit scaling functions specifically
/// designed for high-performance LED programming.

/// @addtogroup lib8tion
/// @{

/// @defgroup Scaling Scaling Functions
/// Fast, efficient 8-bit scaling functions specifically
/// designed for high-performance LED programming.
///
/// Because of the AVR(Arduino) and ARM assembly language
/// implementations provided, using these functions often
/// results in smaller and faster code than the equivalent
/// program using plain "C" arithmetic and logic.
/// @{

/// Scale one byte by a second one, which is treated as
/// the numerator of a fraction whose denominator is 256.
///
/// In other words, it computes i * (scale / 256)
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
/// @note Takes 4 clocks on AVR with MUL, 2 clocks on ARM
ESPHOME_ALWAYS_INLINE static inline uint8_t scale8(uint8_t i, uint8_t scale) {
  return (((uint16_t) i) * (1 + (uint16_t) (scale))) >> 8;
}

/// Scale a 16-bit unsigned value by an 16-bit value, which is treated
/// as the numerator of a fraction whose denominator is 65536.
/// In other words, it computes i * (scale / 65536)
/// @param i input value to scale
/// @param scale scale factor, in n/65536 units
/// @returns scaled value
ESPHOME_ALWAYS_INLINE static inline uint16_t scale16(uint16_t i, uint16_t scale) {
  return ((uint32_t) (i) * (1 + (uint32_t) (scale))) / 65536;
}
/// @} Scaling

/// Calculate an integer average of two signed 15-bit
/// integers (int16_t).
/// If the first argument is even, result is rounded down.
/// If the first argument is odd, result is rounded up.
/// @param i first value to average
/// @param j second value to average
/// @returns mean average of i and j, rounded
ESPHOME_ALWAYS_INLINE static inline int16_t avg15(int16_t i, int16_t j) { return (i >> 1) + (j >> 1) + (i & 0x1); }

/// 8-bit quadratic ease-in / ease-out function.
ESPHOME_ALWAYS_INLINE static inline uint8_t ease8InOutQuad(uint8_t i) {
  uint8_t j = i;
  if (j & 0x80) {
    j = 255 - j;
  }
  uint8_t jj = scale8(j, j);
  uint8_t jj2 = jj << 1;
  if (i & 0x80) {
    jj2 = 255 - jj2;
  }
  return jj2;
}

/// Calculate an integer average of two signed 7-bit
/// integers (int8_t).
/// If the first argument is even, result is rounded down.
/// If the first argument is odd, result is rounded up.
/// @param i first value to average
/// @param j second value to average
/// @returns mean average of i and j, rounded
ESPHOME_ALWAYS_INLINE static inline int8_t avg7(int8_t i, int8_t j) { return (i >> 1) + (j >> 1) + (i & 0x1); }

/// 16-bit quadratic ease-in / ease-out function.
ESPHOME_ALWAYS_INLINE static inline uint16_t ease16InOutQuad(uint16_t i) {
  uint16_t j = i;
  if (j & 0x8000) {
    j = 65535 - j;
  }
  uint16_t jj = scale16(j, j);
  uint16_t jj2 = jj << 1;
  if (i & 0x8000) {
    jj2 = 65535 - jj2;
  }
  return jj2;
}

/// Linear interpolation between two signed 15-bit values,
/// with 8-bit fraction
ESPHOME_ALWAYS_INLINE static inline int16_t lerp15by16(int16_t a, int16_t b, uint16_t frac) {
  int16_t result;
  if (b > a) {
    uint16_t delta = b - a;
    uint16_t scaled = scale16(delta, frac);
    result = a + scaled;
  } else {
    uint16_t delta = a - b;
    uint16_t scaled = scale16(delta, frac);
    result = a - scaled;
  }
  return result;
}

/// Add one byte to another, saturating at 0xFF
/// @param i first byte to add
/// @param j second byte to add
/// @returns the sum of i + j, capped at 0xFF
ESPHOME_ALWAYS_INLINE static inline uint8_t qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;
  if (t > 255)
    t = 255;
  return t;
}

}  // namespace gyver_effects
}  // namespace esphome