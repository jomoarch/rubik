#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include "Vector3.hpp"
#include <cmath>
#include <string>

/**
 * @class Quaternion
 * @brief 四元数类，用于表示三维空间中的旋转
 * @note 四元数格式为 w + xi + yj + zk，其中w是标量部分，(x, y, z)是向量部分
 */
class Quaternion {
public:
  float w, x, y, z; ///< 四元数的四个分量

  /**
   * @brief 构造函数，初始化四元数
   * @param w 标量部分，默认为1（表示无旋转）
   * @param x i分量，默认为0
   * @param y j分量，默认为0
   * @param z k分量，默认为0
   */
  Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

  /**
   * @brief 从旋转轴和角度创建四元数
   * @param axis 旋转轴向量（会自动归一化）
   * @param angle 旋转角度（弧度）
   * @return 表示该旋转的四元数
   */
  static Quaternion fromAxisAngle(const Vector3 &axis, float angle);

  /**
   * @brief 归一化四元数
   * @return 归一化后的四元数
   */
  Quaternion normalize() const;

  /**
   * @brief 计算四元数的共轭
   * @return 共轭四元数
   */
  Quaternion conjugate() const;

  /**
   * @brief 四元数乘法
   * @param other 另一个四元数
   * @return 乘法结果四元数
   */
  Quaternion multiply(const Quaternion &other) const;

  /**
   * @brief 使用四元数旋转向量
   * @param vec 要旋转的向量
   * @return 旋转后的向量
   */
  Vector3 rotateVector(const Vector3 &vec) const;

  /**
   * @brief 将四元数转换为字符串表示
   * @return 格式为"Quat(w, x, y, z)"的字符串
   */
  std::string toString() const;
};

#endif
