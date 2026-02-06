#ifndef COLOR_CONVERTER_HPP
#define COLOR_CONVERTER_HPP

#include <cstdint>

/**
 * @struct RGB
 * @brief 表示RGB颜色，包含红绿蓝三个分量
 */
struct RGB {
  uint8_t r, g, b; ///< 红绿蓝分量，范围0-255

  /**
   * @brief 构造函数，初始化RGB颜色
   * @param r 红色分量，默认为0
   * @param g 绿色分量，默认为0
   * @param b 蓝色分量，默认为0
   */
  RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : r(r), g(g), b(b) {}

  /**
   * @brief 应用亮度调整
   * @param brightness 亮度因子（0.1-1.5）
   * @return 调整后的RGB颜色
   */
  RGB applyBrightness(float brightness) const;

  /**
   * @brief 将RGB颜色转换为256色终端颜色索引
   * @return 256色终端颜色索引（16-255）
   */
  int to256Color() const;
};

/**
 * @class ColorConverter
 * @brief 颜色转换工具类，提供静态颜色转换方法
 */
class ColorConverter {
public:
  /**
   * @brief 应用亮度调整到RGB颜色
   * @param rgb 原始RGB颜色
   * @param brightness 亮度因子
   * @return 调整后的RGB颜色
   */
  static RGB applyBrightness(const RGB &rgb, float brightness);

  /**
   * @brief 将RGB颜色转换为256色终端颜色索引
   * @param rgb RGB颜色
   * @return 256色终端颜色索引
   */
  static int rgbTo256Color(const RGB &rgb);
};

#endif
