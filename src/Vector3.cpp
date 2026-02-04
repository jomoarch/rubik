#include "Vector3.hpp"
#include <iomanip>
#include <sstream>

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vector3::dot(const Vector3 &other) const {
  return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3 &other) const {
  return Vector3(y * other.z - z * other.y, z * other.x - x * other.z,
                 x * other.y - y * other.x);
}

float Vector3::length() const { return std::sqrt(dot(*this)); }

Vector3 Vector3::normalized() const {
  float l = length();
  if (l > 0.0f) {
    return Vector3(x / l, y / l, z / l);
  }
  return Vector3(0, 0, 0);
}

Vector3 Vector3::operator+(const Vector3 &other) const {
  return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3 &other) const {
  return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const {
  return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator-() const { return Vector3(-x, -y, -z); }

bool Vector3::operator==(const Vector3 &other) const {
  return std::abs(x - other.x) < 1e-6f && std::abs(y - other.y) < 1e-6f &&
         std::abs(z - other.z) < 1e-6f;
}

std::string Vector3::toString() const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  oss << "Vec3(" << x << ", " << y << ", " << z << ")";
  return oss.str();
}
