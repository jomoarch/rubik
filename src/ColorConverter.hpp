#ifndef COLOR_CONVERTER_HPP
#define COLOR_CONVERTER_HPP

#include <cstdint>

struct RGB {
  uint8_t r, g, b;

  RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : r(r), g(g), b(b) {}

  RGB applyBrightness(float brightness) const;
  int to256Color() const;
};

class ColorConverter {
public:
  static RGB applyBrightness(const RGB &rgb, float brightness);
  static int rgbTo256Color(const RGB &rgb);
};

#endif
