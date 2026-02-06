#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>
#include <string>

/**
 * @class Vector3
 * @brief 表示三维空间中的向量，提供基本的向量运算功能
 */
class Vector3 {
public:
  float x, y, z;

  /**
   * @brief 构造函数，初始化向量
   * @param x X分量，默认为0
   * @param y Y分量，默认为0
   * @param z Z分量，默认为0
   */
  Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);
  /**
   * @brief 计算与另一个向量的点积
   * @param other 另一个向量
   * @return 点积结果
   */
  float dot(const Vector3 &other) const;
  /**
   * @brief 计算与另一个向量的叉积
   * @param other 另一个向量
   * @return 叉积结果向量
   */
  Vector3 cross(const Vector3 &other) const;
  /**
   * @brief 计算向量的长度（模）
   * @return 向量的长度
   */
  float length() const;
  /**
   * @brief 获取单位向量
   * @return 单位化后的向量
   */
  Vector3 normalized() const;
  /**
   * @brief 向量加法运算符重载
   * @param other 另一个向量
   * @return 相加后的向量
   */
  Vector3 operator+(const Vector3 &other) const;
  /**
   * @brief 向量减法运算符重载
   * @param other 另一个向量
   * @return 相减后的向量
   */
  Vector3 operator-(const Vector3 &other) const;
  /**
   * @brief 向量标量乘法运算符重载
   * @param scalar 标量值
   * @return 缩放后的向量
   */
  Vector3 operator*(float scalar) const;
  /**
   * @brief 向量取反运算符重载
   * @return 取反后的向量
   */
  Vector3 operator-() const;
  /**
   * @brief 向量相等运算符重载
   * @param other 另一个向量
   * @return 如果两个向量相等返回true
   */
  bool operator==(const Vector3 &other) const;
  /**
   * @brief 将向量转换为字符串表示
   * @return 格式为"Vec3(x, y, z)"的字符串
   */
  std::string toString() const;
};

#endif
