#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include "Vector3.hpp"
#include <cmath>
#include <string>

class Quaternion {
public:
  float w, x, y, z;

  Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

  static Quaternion fromAxisAngle(const Vector3 &axis, float angle);
  Quaternion normalize() const;
  Quaternion conjugate() const;
  Quaternion multiply(const Quaternion &other) const;
  Vector3 rotateVector(const Vector3 &vec) const;

  std::string toString() const;
};

#endif
