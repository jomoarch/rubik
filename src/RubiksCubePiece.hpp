#ifndef RUBIKSCUBE_PIECE_HPP
#define RUBIKSCUBE_PIECE_HPP

#include "Enums.hpp" // 包含枚举定义
#include "Quaternion.hpp"
#include "Vector3.hpp"
#include <map>
#include <string>
#include <vector>

class RubiksCubePiece {
private:
  Vector3 initialPosition;
  Vector3 currentPosition;
  PieceType pieceType;
  Quaternion localRotation;
  std::map<std::string, Color> initialColors;

  void initColors();

public:
  RubiksCubePiece(const Vector3 &position, PieceType type);

  void rotate(const Vector3 &axis, float angle);
  std::vector<Vector3> getFaceCorners(const std::string &faceName) const;
  Color getCurrentFaceColor(const std::string &faceName) const;
  void reset();

  Vector3 getCurrentPosition() const { return currentPosition; }
  Vector3 getInitialPosition() const { return initialPosition; }
  PieceType getPieceType() const { return pieceType; }

  std::string toString() const;
};

#endif
