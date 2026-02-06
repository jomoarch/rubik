#ifndef RUBIKSCUBE_PIECE_HPP
#define RUBIKSCUBE_PIECE_HPP

#include "Enums.hpp" // 包含枚举定义
#include "Quaternion.hpp"
#include "Vector3.hpp"
#include <map>
#include <string>
#include <vector>

/**
 * @class RubiksCubePiece
 * @brief 表示魔方的一个块（角块、边块或中心块）
 * @details 每个块有初始位置、当前位置、类型和局部旋转状态
 */
class RubiksCubePiece {
private:
  Vector3 initialPosition;  ///< 块的初始位置（标准化坐标，如(-1,-1,-1)）
  Vector3 currentPosition;  ///< 块的当前位置（考虑旋转后）
  PieceType pieceType;      ///< 块类型（角块、边块、中心块）
  Quaternion localRotation; ///< 块的局部旋转四元数
  std::map<std::string, Color> initialColors; ///< 初始颜色映射（面名->颜色）

  /**
   * @brief 根据块类型和位置初始化颜色
   */
  void initColors();

public:
  /**
   * @brief 构造函数，创建魔方块
   * @param position 块的位置
   * @param type 块类型
   */
  RubiksCubePiece(const Vector3 &position, PieceType type);

  /**
   * @brief 绕轴旋转块
   * @param axis 旋转轴
   * @param angle 旋转角度（弧度）
   */
  void rotate(const Vector3 &axis, float angle);

  /**
   * @brief 获取指定面的角点坐标（局部坐标系）
   * @param faceName 面名称（"F", "B", "L", "R", "U", "D"）
   * @return 面的四个角点坐标向量
   */
  std::vector<Vector3> getFaceCorners(const std::string &faceName) const;

  /**
   * @brief 获取当前指定面的颜色
   * @param faceName 面名称
   * @return 面的颜色
   */
  Color getCurrentFaceColor(const std::string &faceName) const;

  /**
   * @brief 重置块到初始状态（位置和旋转）
   */
  void reset();

  // 获取器方法
  Vector3 getCurrentPosition() const {
    return currentPosition;
  } ///< 获取当前位置
  Vector3 getInitialPosition() const {
    return initialPosition;
  }                                                    ///< 获取初始位置
  PieceType getPieceType() const { return pieceType; } ///< 获取块类型
  Quaternion getLocalRotation() const {
    return localRotation;
  } ///< 获取局部旋转

  /**
   * @brief 将块转换为字符串表示
   * @return 包含块类型和位置的字符串
   */
  std::string toString() const;

  /**
   * @brief 获取指定旋转状态下指定面的颜色
   * @param faceName 面名称
   * @param rotation 旋转四元数
   * @return 面的颜色
   * @note 用于动画期间的颜色计算
   */
  Color getFaceColorWithRotation(const std::string &faceName,
                                 const Quaternion &rotation) const;
};

#endif
