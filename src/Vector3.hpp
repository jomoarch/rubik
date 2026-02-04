#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>
#include <string>

class Vector3 {
public:
  float x, y, z;

  Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);

  float dot(const Vector3 &other) const;
  Vector3 cross(const Vector3 &other) const;
  float length() const;
  Vector3 normalized() const;

  Vector3 operator+(const Vector3 &other) const;
  Vector3 operator-(const Vector3 &other) const;
  Vector3 operator*(float scalar) const;
  Vector3 operator-() const;
  bool operator==(const Vector3 &other) const;

  std::string toString() const;
};

#endif
