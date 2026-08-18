#include "RubiksCube.hpp"
#include "Enums.hpp"
#include <algorithm>
#include <array>
#include <chrono>

#ifdef _WIN32
#define PDC_NCMOUSE
#include <pdcurses.h>
#else
#include <curses.h>
#endif

#include <cmath>
#include <functional>
#include <random>
#include <string>
#include <unordered_map>

const std::vector<RGB> RubiksCube::COLOR_RGB = {
    RGB(220, 60, 60),   // Red
    RGB(220, 120, 0),   // Orange
    RGB(60, 100, 220),  // Blue
    RGB(60, 220, 100),  // Green
    RGB(255, 255, 255), // White
    RGB(220, 220, 60)   // Yellow
};

const std::vector<char> RubiksCube::COLOR_CHARS = {'R', 'O', 'B',
                                                   'G', 'W', 'Y'};
const std::vector<std::string> RubiksCube::COLOR_NAMES = {
    "Red", "Orange", "Blue", "Green", "White", "Yellow"};

const std::unordered_map<std::string, Color> RubiksCube::FACE_TO_COLOR = {
    {"F", static_cast<Color>(_COLOR_ORANGE)}, // Front
    {"B", static_cast<Color>(_COLOR_RED)},    // Back
    {"L", static_cast<Color>(_COLOR_BLUE)},   // Left
    {"R", static_cast<Color>(_COLOR_GREEN)},  // Right
    {"U", static_cast<Color>(_COLOR_WHITE)},  // Up
    {"D", static_cast<Color>(_COLOR_YELLOW)}  // Down
};

const std::unordered_map<std::string, Vector3> RubiksCube::ROTATION_AXES = {
    {"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
    {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
    {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

RubiksCube::RubiksCube()
    : rotation(1, 0, 0, 0), scale(25.0f), position(0, 0, 10),
      lightPosition(0, LIGHT_HEIGHT, 0), aspectRatio(2.0f),
      cameraPosition(0, 0, 0), focalLength(8.0f), animating(false),
      animationProgress(0.0f), dirty(true) {

  viewDirections = {{"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
                    {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
                    {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

  viewMapping = {{"F", "F"}, {"B", "B"}, {"L", "L"},
                 {"R", "R"}, {"U", "U"}, {"D", "D"}};

  createPieces();
  buildShadingLUT();
}

void RubiksCube::createPieces() {
  for (float x : {-1.0f, 1.0f}) {
    for (float y : {-1.0f, 1.0f}) {
      for (float z : {-1.0f, 1.0f}) {
        pieces.push_back(
            std::make_shared<RubiksCubePiece>(Vector3(x, y, z), PIECE_CORNER));
      }
    }
  }

  for (float y : {-1.0f, 1.0f}) {
    for (float z : {-1.0f, 1.0f}) {
      pieces.push_back(
          std::make_shared<RubiksCubePiece>(Vector3(0, y, z), PIECE_EDGE));
    }
  }

  for (float x : {-1.0f, 1.0f}) {
    for (float z : {-1.0f, 1.0f}) {
      pieces.push_back(
          std::make_shared<RubiksCubePiece>(Vector3(x, 0, z), PIECE_EDGE));
    }
  }

  for (float x : {-1.0f, 1.0f}) {
    for (float y : {-1.0f, 1.0f}) {
      pieces.push_back(
          std::make_shared<RubiksCubePiece>(Vector3(x, y, 0), PIECE_EDGE));
    }
  }

  std::vector<Vector3> centerPositions = {
      Vector3(-1, 0, 0), // Blue center
      Vector3(1, 0, 0),  // Green center
      Vector3(0, -1, 0), // Yellow center
      Vector3(0, 1, 0),  // White center
      Vector3(0, 0, -1), // Orange center
      Vector3(0, 0, 1)   // Red center
  };

  for (const auto &position : centerPositions) {
    pieces.push_back(std::make_shared<RubiksCubePiece>(position, PIECE_CENTER));
  }
}

void RubiksCube::updateViewMapping() {
  Quaternion invRotation = rotation.conjugate();

  static const std::unordered_map<std::string, Vector3> FACE_NORMALS = {
      {"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
      {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
      {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

  struct Match {
    std::string viewDir;
    std::string faceName;
    float dot;
  };
  std::vector<Match> matches;

  for (const auto &[viewDirName, viewDir] : viewDirections) {
    Vector3 dirInCubeSpace = invRotation.rotateVector(viewDir);
    for (const auto &[faceName, faceNormalVec] : FACE_NORMALS) {
      float dot = dirInCubeSpace.dot(faceNormalVec);
      matches.push_back({viewDirName, faceName, dot});
    }
  }

  std::sort(matches.begin(), matches.end(),
            [](const Match &a, const Match &b) { return a.dot > b.dot; });

  std::vector<std::string> assignedViews;
  std::vector<std::string> assignedFaces;

  for (const auto &match : matches) {
    if (std::find(assignedViews.begin(), assignedViews.end(), match.viewDir) !=
        assignedViews.end())
      continue;
    if (std::find(assignedFaces.begin(), assignedFaces.end(), match.faceName) !=
        assignedFaces.end())
      continue;

    viewMapping[match.viewDir] = match.faceName;
    assignedViews.push_back(match.viewDir);
    assignedFaces.push_back(match.faceName);
  }
}

void RubiksCube::rotateByMouseDelta(float dx, float dy) {
  float rotateSpeed = 0.01f;
  if (dx != 0) {
    Quaternion rotY =
        Quaternion::fromAxisAngle(Vector3(0, 1, 0), -dx * rotateSpeed);
    rotation = rotY.multiply(rotation).normalize();
    updateViewMapping();
    dirty = true;
  }

  if (dy != 0) {
    Quaternion rotX =
        Quaternion::fromAxisAngle(Vector3(1, 0, 0), -dy * rotateSpeed);
    rotation = rotX.multiply(rotation).normalize();
    updateViewMapping();
    dirty = true;
  }
}

void RubiksCube::zoom(float factor) {
  scale = std::max(15.0f, std::min(50.0f, scale + factor * 0.5f));
  dirty = true;
}

void RubiksCube::rotateViewDirection(const std::string &viewDirection,
                                     bool clockwise) {
  auto it = viewMapping.find(viewDirection);
  if (it == viewMapping.end()) {
    return;
  }

  std::string actualFace = it->second;
  completeAnimation();

  rotateFaceInternal(actualFace, clockwise, true);
}

void RubiksCube::updateAnimation() {
  if (!animating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration<float>(now - animationStartTime).count();
  animationProgress = std::min(1.0f, elapsed / ANIMATION_DURATION);

  if (animationProgress >= 1.0f) {
    completeAnimation();
  }
}

void RubiksCube::completeAnimation() {
  if (!std::get<1>(currentAnimation).empty() && !animationPieces.empty()) {
    auto [axis, actualFace, clockwise] = currentAnimation;
    float angle = clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE;

    for (auto &piece : animationPieces) {
      piece->rotate(axis, angle);
    }
  }

  animating = false;
  animationProgress = 0.0f;
  currentAnimation = std::make_tuple(Vector3(), "", false);
  animationPieces.clear();
  animationRotation = Quaternion(1, 0, 0, 0);
}

std::vector<std::shared_ptr<RubiksCubePiece>>
RubiksCube::getPiecesOnFace(const std::string &faceChar) {
  std::function<bool(const Vector3 &)> positionCheck;

  if (faceChar == "F") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.z + 1) < 0.1f;
    };
  } else if (faceChar == "B") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.z - 1) < 0.1f;
    };
  } else if (faceChar == "L") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.x + 1) < 0.1f;
    };
  } else if (faceChar == "R") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.x - 1) < 0.1f;
    };
  } else if (faceChar == "U") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.y - 1) < 0.1f;
    };
  } else if (faceChar == "D") {
    positionCheck = [](const Vector3 &pos) {
      return std::abs(pos.y + 1) < 0.1f;
    };
  } else {
    return {};
  }

  std::vector<std::shared_ptr<RubiksCubePiece>> result;
  for (const auto &piece : pieces) {
    if (positionCheck(getPiecePosition(piece))) {
      result.push_back(piece);
    }
  }

  return result;
}

Vector3 RubiksCube::getPiecePosition(
    const std::shared_ptr<RubiksCubePiece> &piece) const {
  if (animating && std::find(animationPieces.begin(), animationPieces.end(),
                             piece) != animationPieces.end()) {
    auto [axis, _, clockwise] = currentAnimation;
    float partialAngle =
        (clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE) * animationProgress;
    Quaternion partialRotation = Quaternion::fromAxisAngle(axis, partialAngle);
    return partialRotation.rotateVector(piece->getCurrentPosition());
  }

  return piece->getCurrentPosition();
}

Color RubiksCube::getPieceFaceColor(
    const std::shared_ptr<RubiksCubePiece> &piece,
    const std::string &faceName) const {
  return piece->getCurrentFaceColor(faceName);
}

Color RubiksCube::getFaceColor(const std::string &face) const {
  auto it = FACE_TO_COLOR.find(face);
  if (it != FACE_TO_COLOR.end()) {
    return it->second;
  }
  return _COLOR_NONE;
}

std::string RubiksCube::getFaceletString() {
  completeAnimation();

  static const char COLOR_TO_FACE[] = {'B', 'F', 'L', 'R', 'U', 'D'};

  struct FaceTable {
    Vector3 normal;
    Vector3 rowAxis;
    int rowSign;
    Vector3 colAxis;
    int colSign;
  };
  static const std::unordered_map<std::string, FaceTable> FACE_TABLES = {
      {"U", {Vector3(0, 1, 0), Vector3(0, 0, 1), 1, Vector3(1, 0, 0), -1}},
      {"R", {Vector3(1, 0, 0), Vector3(0, 1, 0), 1, Vector3(0, 0, 1), -1}},
      {"F", {Vector3(0, 0, -1), Vector3(0, 1, 0), 1, Vector3(1, 0, 0), -1}},
      {"D", {Vector3(0, -1, 0), Vector3(0, 0, 1), -1, Vector3(1, 0, 0), -1}},
      {"L", {Vector3(-1, 0, 0), Vector3(0, 1, 0), 1, Vector3(0, 0, 1), 1}},
      {"B", {Vector3(0, 0, 1), Vector3(0, 1, 0), 1, Vector3(1, 0, 0), 1}},
  };
  static const std::vector<std::string> FACE_ORDER = {"U", "R", "F",
                                                      "D", "L", "B"};

  auto slotIndex = [](const Vector3 &p) {
    return (static_cast<int>(std::lround(p.x)) + 1) * 9 +
           (static_cast<int>(std::lround(p.y)) + 1) * 3 +
           (static_cast<int>(std::lround(p.z)) + 1);
  };
  std::array<std::shared_ptr<RubiksCubePiece>, 27> slot{};
  for (const auto &piece : pieces) {
    slot[slotIndex(piece->getCurrentPosition())] = piece;
  }

  static const char LOCAL_FACES[] = {'U', 'R', 'F', 'D', 'L', 'B'};
  static const Vector3 LOCAL_NORMALS[] = {
      Vector3(0, 1, 0),  // U
      Vector3(1, 0, 0),  // R
      Vector3(0, 0, 1),  // F
      Vector3(0, -1, 0), // D
      Vector3(-1, 0, 0), // L
      Vector3(0, 0, -1), // B
  };

  std::string facelet;
  facelet.reserve(54);

  for (const auto &face : FACE_ORDER) {
    const FaceTable &ft = FACE_TABLES.at(face);

    for (int rowVal = 1; rowVal >= -1; rowVal--) {
      for (int colVal = 1; colVal >= -1; colVal--) {
        Vector3 pos = ft.normal;
        pos = pos + ft.rowAxis * (ft.rowSign * rowVal);
        pos = pos + ft.colAxis * (ft.colSign * colVal);

        auto piece = slot[slotIndex(pos)];
        if (!piece) {
          return std::string();
        }

        Color color = _COLOR_NONE;
        const Quaternion &lr = piece->getLocalRotation();
        for (int i = 0; i < 6; i++) {
          Vector3 worldN = lr.rotateVector(LOCAL_NORMALS[i]);
          if (ft.normal.dot(worldN) > 0.99f) {
            color = piece->getCurrentFaceColor(std::string(1, LOCAL_FACES[i]));
            break;
          }
        }
        if (color == _COLOR_NONE) {
          return std::string();
        }
        facelet += COLOR_TO_FACE[static_cast<int>(color)];
      }
    }
  }

  return facelet;
}

std::vector<Vector3>
RubiksCube::getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                                const std::string &faceName) const {
  return getPieceFaceCorners(piece, faceName, getPiecePosition(piece));
}

std::vector<Vector3>
RubiksCube::getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                                const std::string &faceName,
                                const Vector3 &piecePos) const {
  auto corners = piece->getFaceCorners(faceName);

  if (animating && std::find(animationPieces.begin(), animationPieces.end(),
                             piece) != animationPieces.end()) {
    auto [axis, _, clockwise] = currentAnimation;
    float partialAngle =
        (clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE) * animationProgress;
    Quaternion partialRotation = Quaternion::fromAxisAngle(axis, partialAngle);

    std::vector<Vector3> rotatedCorners;
    rotatedCorners.reserve(corners.size());
    for (const auto &corner : corners) {
      rotatedCorners.push_back(partialRotation.rotateVector(corner));
    }
    corners = rotatedCorners;
  }

  std::vector<Vector3> worldCorners;
  worldCorners.reserve(corners.size());
  for (const auto &corner : corners) {
    worldCorners.push_back(corner + piecePos);
  }

  return worldCorners;
}

void RubiksCube::buildShadingLUT() {
  for (int c = 0; c < static_cast<int>(COLOR_RGB.size()); c++) {
    for (int lv = 0; lv < NUM_LIGHT_LEVELS; lv++) {
      float t = static_cast<float>(lv) / (NUM_LIGHT_LEVELS - 1);
      float intensity =
          std::min(1.0f, AMBIENT_INTENSITY + DIFFUSE_STRENGTH * t);
      RGB shaded = COLOR_RGB[c].shadeInHSL(intensity);
      shadedColorLUT[c][lv] = shaded.to256Color();
    }
  }
}

int RubiksCube::calculateLightLevel(const Vector3 &normalWorld,
                                    const Vector3 &worldCenter) const {
  Vector3 toLight = lightPosition - worldCenter;
  float distSq = toLight.dot(toLight);
  float attenuation = 1.0f / (1.0f + ATTENUATION_FACTOR * distSq);
  Vector3 L = toLight.normalized();

  float diffuse = normalWorld.dot(L);
  if (diffuse < 0.0f)
    diffuse = 0.0f;

  float intensity =
      AMBIENT_INTENSITY + DIFFUSE_STRENGTH * diffuse * attenuation;
  if (intensity > 1.0f)
    intensity = 1.0f;

  int level = static_cast<int>(intensity * (NUM_LIGHT_LEVELS - 1) + 0.5f);
  if (level < 0)
    level = 0;
  if (level >= NUM_LIGHT_LEVELS)
    level = NUM_LIGHT_LEVELS - 1;
  return level;
}

RGB RubiksCube::calculateShadedColor(const RGB &baseColor,
                                     const Vector3 &normal,
                                     const Vector3 &worldPos) const {
  int level = calculateLightLevel(normal, worldPos);
  float t = static_cast<float>(level) / (NUM_LIGHT_LEVELS - 1);
  float intensity = std::min(1.0f, AMBIENT_INTENSITY + DIFFUSE_STRENGTH * t);
  return baseColor.shadeInHSL(intensity);
}

std::tuple<int, int, float>
RubiksCube::projectPoint(const Vector3 &point, int width, int height) const {
  Vector3 rotatedPoint = rotation.rotateVector(point);
  Vector3 worldPoint = rotatedPoint + position;

  Vector3 relativePoint = worldPoint - cameraPosition;

  if (relativePoint.z <= 0) {
    return std::make_tuple(-1000, -1000, -relativePoint.z);
  }

  float screenX = (relativePoint.x * focalLength) / relativePoint.z;
  float screenY = (-relativePoint.y * focalLength) / relativePoint.z;

  int screenXInt = static_cast<int>(screenX * scale + width / 2.0f);
  int screenYInt =
      static_cast<int>(screenY * scale / aspectRatio + height / 2.0f);

  return std::make_tuple(screenXInt, screenYInt, relativePoint.length());
}

void RubiksCube::drawPolygon(WINDOW *win,
                             const std::vector<std::pair<int, int>> &points,
                             int colorPair, char colorChar) {
  if (points.size() < 3)
    return;

  int maxY, maxX;
  getmaxyx(win, maxY, maxX);

  int minY = maxY, maxYVal = 0;
  for (const auto &p : points) {
    if (p.second < minY)
      minY = p.second;
    if (p.second > maxYVal)
      maxYVal = p.second;
  }
  minY = std::max(0, minY);
  maxYVal = std::min(maxY - 1, maxYVal);

  static thread_local std::vector<int> intersections;

  const attr_t attr = colorPair > 0 ? COLOR_PAIR(colorPair) : A_NORMAL;
  if (colorPair > 0)
    wattron(win, attr);

  for (int y = minY; y <= maxYVal; y++) {
    intersections.clear();

    for (size_t i = 0; i < points.size(); i++) {
      size_t j = (i + 1) % points.size();
      int y1 = points[i].second, y2 = points[j].second;
      int x1 = points[i].first, x2 = points[j].first;

      if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
        float t = static_cast<float>(y - y1) / (y2 - y1);
        intersections.push_back(static_cast<int>(x1 + t * (x2 - x1)));
      }
    }

    std::sort(intersections.begin(), intersections.end());

    for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
      int startX = std::max(0, intersections[i]);
      int endX = std::min(maxX - 1, intersections[i + 1]);
      int len = endX - startX + 1;
      if (len > 0) {
        mvwhline(win, y, startX, colorChar, len);
      }
    }
  }

  if (colorPair > 0)
    wattroff(win, attr);
}

void RubiksCube::drawPolygonEdges(
    WINDOW *win, const std::vector<std::pair<int, int>> &points) {
  if (points.size() < 2)
    return;

  int maxY, maxX;
  getmaxyx(win, maxY, maxX);

  auto drawLine = [&](int x0, int y0, int x1, int y1) {
    if (y0 == y1) {
      if (y0 < 0 || y0 >= maxY)
        return;
      int lx = std::max(0, std::min(x0, x1));
      int rx = std::min(maxX - 1, std::max(x0, x1));
      if (rx >= lx)
        mvwhline(win, y0, lx, ' ', rx - lx + 1);
      return;
    }

    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    int curY = y0;
    int rowMin = x0, rowMax = x0;

    auto flushRow = [&]() {
      if (curY >= 0 && curY < maxY) {
        int lx = std::max(0, std::min(rowMin, rowMax));
        int rx = std::min(maxX - 1, std::max(rowMin, rowMax));
        if (rx >= lx)
          mvwhline(win, curY, lx, ' ', rx - lx + 1);
      }
    };

    while (true) {
      if (y0 != curY) {
        flushRow();
        curY = y0;
        rowMin = rowMax = x0;
      } else {
        rowMin = std::min(rowMin, x0);
        rowMax = std::max(rowMax, x0);
      }
      if (x0 == x1 && y0 == y1)
        break;
      int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
    flushRow();
  };

  for (size_t i = 0; i < points.size(); i++) {
    size_t j = (i + 1) % points.size();
    drawLine(points[i].first, points[i].second, points[j].first,
             points[j].second);
  }
}

void RubiksCube::draw(WINDOW *win, int width, int height,
                      std::unordered_map<int, int> &colorCache) {
  bool wasAnimating = animating;
  updateAnimation();

  if (wasAnimating && !animating)
    dirty = true;

  if (!animating && !dirty)
    return;

  werase(win);

  struct FaceData {
    std::vector<std::pair<int, int>> points;
    int colorPair;
    float depthSq;
    char colorChar;
  };

  static thread_local std::vector<FaceData> facesToDraw;
  facesToDraw.clear();

  static const std::vector<std::string> FACE_NAMES = {"F", "B", "L",
                                                      "R", "U", "D"};

  for (const auto &piece : pieces) {
    Vector3 piecePos = getPiecePosition(piece);

    for (const auto &faceName : FACE_NAMES) {
      Color colorIdx = piece->getCurrentFaceColor(faceName);
      if (colorIdx == _COLOR_NONE)
        continue;

      auto corners = getPieceFaceCorners(piece, faceName, piecePos);
      if (corners.size() < 3)
        continue;

      Vector3 center(0, 0, 0);
      for (const auto &corner : corners)
        center = center + corner;
      center = center * (1.0f / corners.size());

      Vector3 rotatedCenter = rotation.rotateVector(center);
      Vector3 worldCenter = rotatedCenter + position;

      Vector3 v1 = corners[1] - corners[0];
      Vector3 v2 = corners[2] - corners[0];
      Vector3 normal = v1.cross(v2).normalized();
      Vector3 normalWorld = rotation.rotateVector(normal);
      Vector3 viewDir = cameraPosition - worldCenter;
      if (normalWorld.dot(viewDir) <= 0)
        continue;

      int colorIndexInt = static_cast<int>(colorIdx);
      if (colorIndexInt < 0 ||
          colorIndexInt >= static_cast<int>(COLOR_RGB.size()))
        continue;

      int lightLevel = calculateLightLevel(normalWorld, worldCenter);
      int termIdx = shadedColorLUT[colorIndexInt][lightLevel];

      int colorPair;
      auto it = colorCache.find(termIdx);
      if (it != colorCache.end()) {
        colorPair = it->second;
      } else {
        colorPair = static_cast<int>(colorCache.size()) + 1;
        init_pair(colorPair, termIdx, COLOR_BLACK);
        colorCache[termIdx] = colorPair;
      }

      std::vector<std::pair<int, int>> screenPoints;
      screenPoints.reserve(corners.size());
      for (const auto &c : corners) {
        auto [sx, sy, _] = projectPoint(c, width, height);
        screenPoints.emplace_back(sx, sy);
      }

      char colorChar = COLOR_CHARS[colorIndexInt];
      Vector3 rel = worldCenter - cameraPosition;
      float depthSq = rel.x * rel.x + rel.y * rel.y + rel.z * rel.z;
      facesToDraw.push_back(
          {std::move(screenPoints), colorPair, depthSq, colorChar});
    }
  }

  std::sort(facesToDraw.begin(), facesToDraw.end(),
            [](const FaceData &a, const FaceData &b) {
              return a.depthSq > b.depthSq;
            });

  for (const auto &face : facesToDraw) {
    drawPolygon(win, face.points, face.colorPair, face.colorChar);
    drawPolygonEdges(win, face.points);
  }

  dirty = false;
  drawUI(win, width, height, colorCache);
}

void RubiksCube::drawUI(WINDOW *win, int width, int height,
                        std::unordered_map<int, int> &colorCache) {
  std::string title = "3x3 Rubik's Cube";
  if (width >= static_cast<int>(title.length())) {
    mvwprintw(win, 0, (width - title.length()) / 2, "%s", title.c_str());
  }

  updateViewMapping();

  auto getFaceColorIndex = [this](const std::string &face) -> int {
    try {
      auto viewIt = viewMapping.find(face);
      if (viewIt != viewMapping.end()) {
        auto colorIt = FACE_TO_COLOR.find(viewIt->second);
        if (colorIt != FACE_TO_COLOR.end()) {
          return static_cast<int>(colorIt->second);
        }
      }
    } catch (...) {
    }
    return -1;
  };

  auto getColorName = [this](const std::string &face) -> std::string {
    try {
      auto viewIt = viewMapping.find(face);
      if (viewIt != viewMapping.end()) {
        auto colorIt = FACE_TO_COLOR.find(viewIt->second);
        if (colorIt != FACE_TO_COLOR.end()) {
          int colorIndex = static_cast<int>(colorIt->second);
          if (colorIndex >= 0 &&
              colorIndex < static_cast<int>(COLOR_NAMES.size())) {
            return COLOR_NAMES[colorIndex];
          }
        }
      }
    } catch (...) {
    }
    return "Unknown";
  };

  auto getOrCreateColorPair = [&](int colorIndex) -> int {
    if (colorIndex < 0 || colorIndex >= static_cast<int>(COLOR_RGB.size()))
      return 0;
    const RGB &rgb = COLOR_RGB[colorIndex];
    int termIdx = rgb.to256Color();
    auto it = colorCache.find(termIdx);
    if (it != colorCache.end())
      return it->second;
    int pair = static_cast<int>(colorCache.size()) + 1;
    init_pair(pair, termIdx, COLOR_BLACK);
    colorCache[termIdx] = pair;
    return pair;
  };

  std::vector<std::string> controls = {
      "Controls:",
      "  Arrow Keys - Rotate cube",
      "  +/-        - Zoom in/out",
      "  C          - Reset cube",
      "  X          - Scramble cube",
      "  H          - Toggle hint system",
      "  SPACE      - Solve & show hints",
      "  ESC        - Exit",
      "",
      "Rotate faces (based on current view):",
      "  f - Front clockwise  F - Front counter",
      "  b - Back clockwise   B - Back counter",
      "  l - Left clockwise   L - Left counter",
      "  r - Right clockwise  R - Right counter",
      "  u - Up clockwise     U - Up counter",
      "  d - Down clockwise   D - Down counter",
      "",
      "Current view mapping:",

      "  Front(F) -> " + getColorName("F") + " face",
      "  Back(B)  -> " + getColorName("B") + " face",
      "  Left(L)  -> " + getColorName("L") + " face",
      "  Right(R) -> " + getColorName("R") + " face",
      "  Up(U)    -> " + getColorName("U") + " face",
      "  Down(D)  -> " + getColorName("D") + " face",
      "",
      "Scale: " + std::to_string(static_cast<int>(scale)),
      "Animation: " + std::string(animating ? "Active" : "None")};

  static const int VIEW_MAP_START = 18;
  static const int VIEW_MAP_END = 23;

  static const std::vector<std::string> VIEW_ORDER = {"F", "B", "L",
                                                      "R", "U", "D"};

  static const std::vector<std::string> VIEW_PREFIXES = {
      "  Front(F) -> ", "  Back(B)  -> ", "  Left(L)  -> ",
      "  Right(R) -> ", "  Up(U)    -> ", "  Down(D)  -> "};

  int boxWidth = 0;
  for (const auto &line : controls) {
    boxWidth = std::max(boxWidth, static_cast<int>(line.length()));
  }
  boxWidth += 4;

  int boxHeight = static_cast<int>(controls.size()) + 2;
  int boxX = width - boxWidth - 2;
  int boxY = 2;

  if (boxX > 0 && boxY > 0 && boxX + boxWidth < width &&
      boxY + boxHeight < height) {
    mvwaddch(win, boxY, boxX, '+');
    mvwaddch(win, boxY, boxX + boxWidth - 1, '+');
    mvwaddch(win, boxY + boxHeight - 1, boxX, '+');
    mvwaddch(win, boxY + boxHeight - 1, boxX + boxWidth - 1, '+');

    for (int x = boxX + 1; x < boxX + boxWidth - 1; x++) {
      mvwaddch(win, boxY, x, '-');
      mvwaddch(win, boxY + boxHeight - 1, x, '-');
    }

    for (int y = boxY + 1; y < boxY + boxHeight - 1; y++) {
      mvwaddch(win, y, boxX, '|');
      mvwaddch(win, y, boxX + boxWidth - 1, '|');
    }

    for (size_t i = 0; i < controls.size(); i++) {
      int row = boxY + 1 + static_cast<int>(i);
      int col = boxX + 2;

      if (static_cast<int>(i) >= VIEW_MAP_START &&
          static_cast<int>(i) <= VIEW_MAP_END) {
        int viewIdx = static_cast<int>(i) - VIEW_MAP_START;
        const std::string &face = VIEW_ORDER[viewIdx];
        const std::string &prefix = VIEW_PREFIXES[viewIdx];
        std::string colorName = getColorName(face);
        int colorIdx = getFaceColorIndex(face);
        int colorPair = getOrCreateColorPair(colorIdx);

        mvwprintw(win, row, col, "%s", prefix.c_str());

        int nameCol = col + static_cast<int>(prefix.length());
        if (colorPair > 0) {
          wattron(win, COLOR_PAIR(colorPair) | A_BOLD);
          mvwprintw(win, row, nameCol, "%s", colorName.c_str());
          wattroff(win, COLOR_PAIR(colorPair) | A_BOLD);
        } else {
          mvwprintw(win, row, nameCol, "%s", colorName.c_str());
        }

        int suffixCol = nameCol + static_cast<int>(colorName.length());
        mvwprintw(win, row, suffixCol, " face");
      } else {
        mvwprintw(win, row, col, "%s", controls[i].c_str());
      }
    }
  }

  std::string footer = "Press ESC to exit | C to reset | X to scramble";
  if (width >= static_cast<int>(footer.length())) {
    wattron(win, A_REVERSE);
    mvwprintw(win, height - 1, (width - footer.length()) / 2, "%s",
              footer.c_str());
    wattroff(win, A_REVERSE);
  }
}

void RubiksCube::reset() {
  for (auto &piece : pieces) {
    piece->reset();
  }

  animating = false;
  animationProgress = 0.0f;
  currentAnimation = std::make_tuple(Vector3(), "", false);
  animationPieces.clear();
  animationRotation = Quaternion(1, 0, 0, 0);

  history.clear();
  dirty = true;
}

void RubiksCube::scramble(int moves) {
  std::vector<std::string> viewDirections = {"F", "B", "L", "R", "U", "D"};

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dirDist(
      0, static_cast<int>(viewDirections.size()) - 1);
  std::uniform_int_distribution<> boolDist(0, 1);

  for (int i = 0; i < moves; i++) {
    std::string viewDir = viewDirections[dirDist(gen)];
    bool clockwise = boolDist(gen) == 0;

    auto it = viewMapping.find(viewDir);
    if (it == viewMapping.end())
      continue;

    std::string actualFace = it->second;
    auto piecesToRotate = getPiecesOnFace(actualFace);

    auto axesIt = ROTATION_AXES.find(actualFace);
    if (axesIt == ROTATION_AXES.end())
      continue;

    Vector3 axis = axesIt->second;
    float angle = clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE;

    for (auto &piece : piecesToRotate) {
      piece->rotate(axis, angle);
    }
  }

  animating = false;
  animationProgress = 0.0f;
  currentAnimation = std::make_tuple(Vector3(), "", false);
  animationPieces.clear();
  animationRotation = Quaternion(1, 0, 0, 0);
  history.clear();
  dirty = true;
}

void RubiksCube::rotateFaceInternal(const std::string &actualFace,
                                    bool clockwise, bool recordHistory) {
  auto axesIt = ROTATION_AXES.find(actualFace);
  if (axesIt == ROTATION_AXES.end())
    return;

  Vector3 axis = axesIt->second;

  if (recordHistory) {
    history.push_back({axis, !clockwise});
  }

  animating = true;
  animationProgress = 0.0f;
  animationStartTime = std::chrono::steady_clock::now();
  currentAnimation = std::make_tuple(axis, actualFace, clockwise);
  animationPieces = getPiecesOnFace(actualFace);
  animationRotation = Quaternion::fromAxisAngle(
      axis, clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE);
  dirty = true;
}

bool RubiksCube::getLastPerformedMove(char &face, bool &cw) const {
  if (history.empty())
    return false;
  const auto &[axis, clockwise] = history.back();
  for (const auto &[f, ax] : ROTATION_AXES) {
    if ((ax - axis).length() < 0.1f) {
      face = f[0];
      cw = !clockwise;
      return true;
    }
  }
  return false;
}

bool RubiksCube::undo() {
  if (history.empty())
    return false;

  completeAnimation();

  auto [axis, clockwise] = history.back();
  history.pop_back();

  std::string actualFace;
  for (const auto &[face, ax] : ROTATION_AXES) {
    if ((ax - axis).length() < 0.1f) {
      actualFace = face;
      break;
    }
  }
  if (actualFace.empty())
    return false;

  rotateFaceInternal(actualFace, clockwise, false);
  return true;
}
