#include "Quaternion.hpp"
#include <iomanip>
#include <sstream>

Quaternion::Quaternion(float w, float x, float y, float z)
    : w(w), x(x), y(y), z(z) {}

Quaternion Quaternion::fromAxisAngle(const Vector3 &axis, float angle) {
  Vector3 normAxis = axis.normalized();
  float halfAngle = angle / 2.0f;
  float sinHalf = std::sin(halfAngle);

  return Quaternion(std::cos(halfAngle), normAxis.x * sinHalf,
                    normAxis.y * sinHalf, normAxis.z * sinHalf);
}

Quaternion Quaternion::normalize() const {
  float length = std::sqrt(w * w + x * x + y * y + z * z);
  if (length == 0.0f) {
    return Quaternion(1, 0, 0, 0);
  }
  return Quaternion(w / length, x / length, y / length, z / length);
}

Quaternion Quaternion::conjugate() const { return Quaternion(w, -x, -y, -z); }

Quaternion Quaternion::multiply(const Quaternion &other) const {
  float newW = w * other.w - x * other.x - y * other.y - z * other.z;
  float newX = w * other.x + x * other.w + y * other.z - z * other.y;
  float newY = w * other.y - x * other.z + y * other.w + z * other.x;
  float newZ = w * other.z + x * other.y - y * other.x + z * other.w;

  return Quaternion(newW, newX, newY, newZ);
}

Vector3 Quaternion::rotateVector(const Vector3 &vec) const {
  Quaternion qVec(0, vec.x, vec.y, vec.z);
  Quaternion qConj = conjugate();
  Quaternion result = multiply(qVec).multiply(qConj);
  return Vector3(result.x, result.y, result.z);
}

std::string Quaternion::toString() const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(3);
  oss << "Quat(" << w << ", " << x << ", " << y << ", " << z << ")";
  return oss.str();
}
