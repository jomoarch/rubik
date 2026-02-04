#include "ColorConverter.hpp"
#include <algorithm>

RGB RGB::applyBrightness(float brightness) const {
  brightness = std::max(0.1f, std::min(1.5f, brightness));

  uint8_t newR = static_cast<uint8_t>(std::min(255.0f, r * brightness));
  uint8_t newG = static_cast<uint8_t>(std::min(255.0f, g * brightness));
  uint8_t newB = static_cast<uint8_t>(std::min(255.0f, b * brightness));

  return RGB(newR, newG, newB);
}

int RGB::to256Color() const {
  uint8_t r_val = std::max(0, std::min(255, static_cast<int>(r)));
  uint8_t g_val = std::max(0, std::min(255, static_cast<int>(g)));
  uint8_t b_val = std::max(0, std::min(255, static_cast<int>(b)));

  // Grayscale handling
  if (r_val == g_val && g_val == b_val) {
    if (r_val < 8) {
      return 232;
    } else if (r_val > 247) {
      return 255;
    } else {
      int gray_index = static_cast<int>((r_val - 8) / 247.0f * 23);
      return 232 + gray_index;
    }
  }

  // Color handling
  int r_idx = static_cast<int>(r_val / 255.0f * 5);
  int g_idx = static_cast<int>(g_val / 255.0f * 5);
  int b_idx = static_cast<int>(b_val / 255.0f * 5);
  return 16 + 36 * r_idx + 6 * g_idx + b_idx;
}
