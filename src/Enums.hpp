#ifndef ENUMS_HPP
#define ENUMS_HPP

/**
 * @enum Color
 * @brief 魔方面颜色枚举
 * @note 遵循标准魔方配色方案
 */
enum Color {
  _COLOR_RED = 0,    ///< 红色，通常对应前面
  _COLOR_ORANGE = 1, ///< 橙色，通常对应后面
  _COLOR_BLUE = 2,   ///< 蓝色，通常对应左面
  _COLOR_GREEN = 3,  ///< 绿色，通常对应右面
  _COLOR_WHITE = 4,  ///< 白色，通常对应上面
  _COLOR_YELLOW = 5, ///< 黄色，通常对应下面
  _COLOR_NONE = -1   ///< 无颜色（用于未初始化或错误情况）
};

/**
 * @enum PieceType
 * @brief 魔方块类型枚举
 */
enum PieceType {
  PIECE_CORNER, ///< 角块（8个）
  PIECE_EDGE,   ///< 边块（12个）
  PIECE_CENTER  ///< 中心块（6个）
};

#endif
