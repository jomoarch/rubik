#include "RubiksCubePiece.hpp"
#include <map>
#include <sstream>

// Face normals (pointing outward)
static const std::map<std::string, Vector3> FACE_NORMALS = {
    {"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
    {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
    {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

RubiksCubePiece::RubiksCubePiece(const Vector3 &position, PieceType type)
    : initialPosition(position), currentPosition(position), pieceType(type),
      localRotation(1, 0, 0, 0) {
  initColors();
}

void RubiksCubePiece::initColors() {
  float x = initialPosition.x;
  float y = initialPosition.y;
  float z = initialPosition.z;

  // Center pieces
  if (pieceType == PIECE_CENTER) {
    if (x == -1.0f)
      initialColors["L"] = static_cast<Color>(_COLOR_BLUE);
    else if (x == 1.0f)
      initialColors["R"] = static_cast<Color>(_COLOR_GREEN);
    else if (y == -1.0f)
      initialColors["D"] = static_cast<Color>(_COLOR_YELLOW);
    else if (y == 1.0f)
      initialColors["U"] = static_cast<Color>(_COLOR_WHITE);
    else if (z == -1.0f)
      initialColors["B"] = static_cast<Color>(_COLOR_ORANGE);
    else if (z == 1.0f)
      initialColors["F"] = static_cast<Color>(_COLOR_RED);
    return;
  }

  // Edge and corner pieces
  if (x == -1.0f)
    initialColors["L"] = static_cast<Color>(_COLOR_BLUE);
  else if (x == 1.0f)
    initialColors["R"] = static_cast<Color>(_COLOR_GREEN);

  if (y == -1.0f)
    initialColors["D"] = static_cast<Color>(_COLOR_YELLOW);
  else if (y == 1.0f)
    initialColors["U"] = static_cast<Color>(_COLOR_WHITE);

  if (z == -1.0f)
    initialColors["B"] = static_cast<Color>(_COLOR_ORANGE);
  else if (z == 1.0f)
    initialColors["F"] = static_cast<Color>(_COLOR_RED);
}

void RubiksCubePiece::rotate(const Vector3 &axis, float angle) {
  Quaternion rotation = Quaternion::fromAxisAngle(axis, angle);
  currentPosition = rotation.rotateVector(currentPosition);
  localRotation = rotation.multiply(localRotation).normalize();
}

std::vector<Vector3>
RubiksCubePiece::getFaceCorners(const std::string &faceName) const {
  static const std::map<std::string,
                        std::vector<std::tuple<float, float, float>>>
      FACE_CORNERS = {{"F",
                       {{-0.5f, -0.5f, 0.5f},
                        {0.5f, -0.5f, 0.5f},
                        {0.5f, 0.5f, 0.5f},
                        {-0.5f, 0.5f, 0.5f}}},
                      {"B",
                       {{-0.5f, -0.5f, -0.5f},
                        {-0.5f, 0.5f, -0.5f},
                        {0.5f, 0.5f, -0.5f},
                        {0.5f, -0.5f, -0.5f}}},
                      {"L",
                       {{-0.5f, -0.5f, -0.5f},
                        {-0.5f, -0.5f, 0.5f},
                        {-0.5f, 0.5f, 0.5f},
                        {-0.5f, 0.5f, -0.5f}}},
                      {"R",
                       {{0.5f, -0.5f, 0.5f},
                        {0.5f, -0.5f, -0.5f},
                        {0.5f, 0.5f, -0.5f},
                        {0.5f, 0.5f, 0.5f}}},
                      {"U",
                       {{-0.5f, 0.5f, 0.5f},
                        {0.5f, 0.5f, 0.5f},
                        {0.5f, 0.5f, -0.5f},
                        {-0.5f, 0.5f, -0.5f}}},
                      {"D",
                       {{-0.5f, -0.5f, -0.5f},
                        {0.5f, -0.5f, -0.5f},
                        {0.5f, -0.5f, 0.5f},
                        {-0.5f, -0.5f, 0.5f}}}};

  auto it = FACE_CORNERS.find(faceName);
  if (it == FACE_CORNERS.end()) {
    return {};
  }

  std::vector<Vector3> corners;
  for (const auto &corner : it->second) {
    Vector3 cornerVec(std::get<0>(corner), std::get<1>(corner),
                      std::get<2>(corner));
    Vector3 rotatedCorner = localRotation.rotateVector(cornerVec);
    corners.push_back(rotatedCorner);
  }

  return corners;
}

Color RubiksCubePiece::getCurrentFaceColor(const std::string &faceName) const {
  return getFaceColorWithRotation(faceName, localRotation);
}

void RubiksCubePiece::reset() {
  currentPosition = initialPosition;
  localRotation = Quaternion(1, 0, 0, 0);
}

std::string RubiksCubePiece::toString() const {
  std::ostringstream oss;
  oss << (pieceType == PIECE_CORNER ? "Corner"
          : pieceType == PIECE_EDGE ? "Edge"
                                    : "Center");
  oss << currentPosition.toString();
  return oss.str();
}

Color RubiksCubePiece::getFaceColorWithRotation(
    const std::string &faceName, const Quaternion &rotation) const {
  auto it = FACE_NORMALS.find(faceName);
  if (it == FACE_NORMALS.end()) {
    return _COLOR_NONE; // Default
  }

  Vector3 targetNormal = it->second;

  // Invert the rotation to get back to initial coordinate system
  Quaternion invRotation = rotation.conjugate();
  Vector3 initialNormal = invRotation.rotateVector(targetNormal);

  // Find which initial face has the closest normal
  std::string bestMatch;
  float bestDot = -1.0f;

  for (const auto &[initFace, initColor] : initialColors) {
    auto normalIt = FACE_NORMALS.find(initFace);
    if (normalIt != FACE_NORMALS.end()) {
      Vector3 initNormal = normalIt->second;
      float dot = initialNormal.dot(initNormal);
      if (dot > bestDot) {
        bestDot = dot;
        bestMatch = initFace;
      }
    }
  }

  // Return the matched face color
  if (!bestMatch.empty() && bestDot > 0.9f) {
    return initialColors.at(bestMatch);
  }

  return _COLOR_NONE; // Default
}
