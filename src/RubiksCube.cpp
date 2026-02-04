#include "RubiksCube.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <random>

// Static constants initialization
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

const std::map<std::string, Color> RubiksCube::FACE_TO_COLOR = {
    {"F", static_cast<Color>(COLOR_ORANGE)}, // Front
    {"B", static_cast<Color>(COLOR_RED)},    // Back
    {"L", static_cast<Color>(COLOR_BLUE)},   // Left
    {"R", static_cast<Color>(COLOR_GREEN)},  // Right
    {"U", static_cast<Color>(COLOR_WHITE)},  // Up
    {"D", static_cast<Color>(COLOR_YELLOW)}  // Down
};

const std::map<std::string, Vector3> RubiksCube::ROTATION_AXES = {
    {"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
    {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
    {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

RubiksCube::RubiksCube()
    : rotation(1, 0, 0, 0), scale(25.0f), position(0, 0, 10), aspectRatio(2.0f),
      cameraPosition(0, 0, 0), focalLength(8.0f), animating(false),
      animationProgress(0.0f) {

  // Initialize light direction
  lightDir = Vector3(0.3f, 0.5f, -0.8f).normalized();

  // Initialize view directions
  viewDirections = {{"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
                    {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
                    {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

  // Initialize view mapping
  viewMapping = {{"F", "F"}, {"B", "B"}, {"L", "L"},
                 {"R", "R"}, {"U", "U"}, {"D", "D"}};

  createPieces();
}

void RubiksCube::createPieces() {
  // Create corner pieces (8)
  for (float x : {-1.0f, 1.0f}) {
    for (float y : {-1.0f, 1.0f}) {
      for (float z : {-1.0f, 1.0f}) {
        pieces.push_back(
            std::make_shared<RubiksCubePiece>(Vector3(x, y, z), PIECE_CORNER));
      }
    }
  }

  // Create edge pieces (12)
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

  // Create center pieces (6)
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
  for (const auto &[viewDirName, viewDir] : viewDirections) {
    std::string bestFace;
    float bestDot = -1.0f;

    Quaternion invRotation(rotation.w, -rotation.x, -rotation.y, -rotation.z);
    Vector3 dirInCubeSpace = invRotation.rotateVector(viewDir);

    static const std::map<std::string, Vector3> FACE_NORMALS = {
        {"F", Vector3(0, 0, -1)}, {"B", Vector3(0, 0, 1)},
        {"L", Vector3(-1, 0, 0)}, {"R", Vector3(1, 0, 0)},
        {"U", Vector3(0, 1, 0)},  {"D", Vector3(0, -1, 0)}};

    for (const auto &[faceName, faceNormalVec] : FACE_NORMALS) {
      float dot = dirInCubeSpace.dot(faceNormalVec);
      if (dot > bestDot) {
        bestDot = dot;
        bestFace = faceName;
      }
    }

    if (!bestFace.empty() && bestDot > 0.5f) {
      viewMapping[viewDirName] = bestFace;
    }
  }
}

void RubiksCube::rotateByMouseDelta(float dx, float dy) {
  float rotateSpeed = 0.01f;
  if (dx != 0) {
    Quaternion rotY =
        Quaternion::fromAxisAngle(Vector3(0, 1, 0), -dx * rotateSpeed);
    rotation = rotY.multiply(rotation).normalize();
    updateViewMapping();
  }

  if (dy != 0) {
    Quaternion rotX =
        Quaternion::fromAxisAngle(Vector3(1, 0, 0), -dy * rotateSpeed);
    rotation = rotX.multiply(rotation).normalize();
    updateViewMapping();
  }
}

void RubiksCube::zoom(float factor) {
  scale = std::max(15.0f, std::min(50.0f, scale + factor * 0.5f));
}

void RubiksCube::rotateViewDirection(const std::string &viewDirection,
                                     bool clockwise) {
  auto it = viewMapping.find(viewDirection);
  if (it == viewMapping.end()) {
    return;
  }

  std::string actualFace = it->second;
  completeAnimation();

  animating = true;
  animationProgress = 0.0f;
  animationStartTime = std::chrono::steady_clock::now();

  auto axesIt = ROTATION_AXES.find(actualFace);
  if (axesIt == ROTATION_AXES.end()) {
    return;
  }

  Vector3 axis = axesIt->second;
  currentAnimation = std::make_tuple(axis, actualFace, clockwise);
  animationPieces = getPiecesOnFace(actualFace);
  animationRotation = Quaternion::fromAxisAngle(
      axis, clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE);
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
  if (animating && std::find(animationPieces.begin(), animationPieces.end(),
                             piece) != animationPieces.end()) {
    auto [axis, _, clockwise] = currentAnimation;
    float partialAngle =
        (clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE) * animationProgress;
    Quaternion partialRotation = Quaternion::fromAxisAngle(axis, partialAngle);

    // This is a simplified approach - in a real implementation,
    // we'd need to properly handle combined rotations
    return piece->getCurrentFaceColor(faceName);
  }

  return piece->getCurrentFaceColor(faceName);
}

std::vector<Vector3>
RubiksCube::getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                                const std::string &faceName) const {
  auto corners = piece->getFaceCorners(faceName);

  if (animating && std::find(animationPieces.begin(), animationPieces.end(),
                             piece) != animationPieces.end()) {
    auto [axis, _, clockwise] = currentAnimation;
    float partialAngle =
        (clockwise ? ROTATION_ANGLE : -ROTATION_ANGLE) * animationProgress;
    Quaternion partialRotation = Quaternion::fromAxisAngle(axis, partialAngle);

    std::vector<Vector3> rotatedCorners;
    for (const auto &corner : corners) {
      rotatedCorners.push_back(partialRotation.rotateVector(corner));
    }
    corners = rotatedCorners;
  }

  Vector3 piecePos = getPiecePosition(piece);
  std::vector<Vector3> worldCorners;
  for (const auto &corner : corners) {
    worldCorners.push_back(corner + piecePos);
  }

  return worldCorners;
}

float RubiksCube::calculateBrightness(const Vector3 &normal) const {
  float dot = normal.dot(lightDir);
  float ambient = 0.3f;
  float diffuse = std::max(0.0f, dot) * 0.7f;
  float brightness = ambient + diffuse;

  if (dot > 0.8f) {
    brightness = std::min(1.2f, brightness + 0.2f);
  }

  return std::max(0.25f, std::min(1.2f, brightness));
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

  // 获取窗口边界
  int maxY, maxX;
  getmaxyx(win, maxY, maxX);

  // 找到多边形的y范围
  int yMin = maxY, yMax = 0;
  for (const auto &p : points) {
    if (p.second < yMin)
      yMin = p.second;
    if (p.second > yMax)
      yMax = p.second;
  }

  // 确保y范围在窗口内
  yMin = std::max(0, yMin);
  yMax = std::min(maxY - 1, yMax);

  // 准备边列表
  struct Edge {
    int yMin, yMax;
    float x, slope;
  };

  std::vector<Edge> edges;
  size_t n = points.size();

  for (size_t i = 0; i < n; i++) {
    const auto &p1 = points[i];
    const auto &p2 = points[(i + 1) % n];

    // 跳过水平线
    if (p1.second == p2.second)
      continue;

    Edge edge;
    if (p1.second < p2.second) {
      edge.yMin = p1.second;
      edge.yMax = p2.second;
      edge.x = static_cast<float>(p1.first);
    } else {
      edge.yMin = p2.second;
      edge.yMax = p1.second;
      edge.x = static_cast<float>(p2.first);
    }

    // 计算斜率
    float dx = static_cast<float>(p2.first - p1.first);
    float dy = static_cast<float>(p2.second - p1.second);
    edge.slope = dx / dy;

    edges.push_back(edge);
  }

  // 扫描线算法
  for (int y = yMin; y <= yMax; y++) {
    std::vector<float> intersections;

    // 收集与当前扫描线的交点
    for (const auto &edge : edges) {
      if (y >= edge.yMin && y < edge.yMax) {
        float x = edge.x + (y - edge.yMin) * edge.slope;
        intersections.push_back(x);
      }
    }

    // 对交点排序
    std::sort(intersections.begin(), intersections.end());

    // 填充交点之间的区域
    for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
      int startX = static_cast<int>(intersections[i]);
      int endX = static_cast<int>(intersections[i + 1]);

      // 确保x在边界内
      startX = std::max(0, startX);
      endX = std::min(maxX - 1, endX);

      // 绘制填充线
      for (int x = startX; x <= endX; x++) {
        if (colorPair > 0) {
          wattron(win, COLOR_PAIR(colorPair));
          mvwaddch(win, y, x, colorChar);
          wattroff(win, COLOR_PAIR(colorPair));
        } else {
          mvwaddch(win, y, x, colorChar);
        }
      }
    }
  }
}

void RubiksCube::draw(WINDOW *win, int width, int height,
                      std::map<int, int> &colorCache) {
  werase(win);
  updateAnimation();

  // 定义要绘制的面数据结构
  struct FaceData {
    std::vector<std::pair<int, int>> points;
    int colorPair;
    float depth;
    char colorChar;
  };

  std::vector<FaceData> facesToDraw;

  static const std::vector<std::string> FACE_NAMES = {"F", "B", "L",
                                                      "R", "U", "D"};

  for (const auto &piece : pieces) {
    for (const auto &faceName : FACE_NAMES) {
      Color colorIdx = getPieceFaceColor(piece, faceName);
      auto corners = getPieceFaceCorners(piece, faceName);

      if (corners.empty())
        continue;

      // 计算面中心
      Vector3 center(0, 0, 0);
      for (const auto &corner : corners) {
        center = center + corner;
      }
      center = center * (1.0f / corners.size());

      Vector3 rotatedCenter = rotation.rotateVector(center);
      Vector3 worldCenter = rotatedCenter + position;

      // 计算面法向量
      if (corners.size() >= 3) {
        Vector3 v1 = corners[1] - corners[0];
        Vector3 v2 = corners[2] - corners[0];
        Vector3 normal = v1.cross(v2).normalized();
        Vector3 normalRotated = rotation.rotateVector(normal);

        Vector3 cameraToFace = worldCenter - cameraPosition;

        // 背面剔除
        if (normalRotated.dot(cameraToFace) >= 0) {
          continue;
        }

        float brightness = calculateBrightness(normalRotated);

        // 安全检查颜色索引
        int colorIndexInt = static_cast<int>(colorIdx);
        if (colorIndexInt < 0 ||
            colorIndexInt >= static_cast<int>(COLOR_RGB.size())) {
          continue; // 跳过无效的颜色索引
        }

        RGB baseColor = COLOR_RGB[colorIndexInt];
        RGB shadedColor = baseColor.applyBrightness(brightness);

        int terminalColorIndex = shadedColor.to256Color();
        int colorPair;

        if (colorCache.find(terminalColorIndex) != colorCache.end()) {
          colorPair = colorCache[terminalColorIndex];
        } else {
          colorPair = static_cast<int>(colorCache.size()) + 1;
          init_pair(colorPair, terminalColorIndex, COLOR_BLACK);
          colorCache[terminalColorIndex] = colorPair;
        }

        // 投影角点到屏幕
        std::vector<std::pair<int, int>> screenPoints;
        for (const auto &corner3d : corners) {
          auto [x, y, _] = projectPoint(corner3d, width, height);
          screenPoints.emplace_back(x, y);
        }

        float depth = cameraToFace.length();

        // 安全检查字符索引
        if (colorIndexInt >= 0 &&
            colorIndexInt < static_cast<int>(COLOR_CHARS.size())) {
          char colorChar = COLOR_CHARS[colorIndexInt];
          facesToDraw.push_back({screenPoints, colorPair, depth, colorChar});
        }
      }
    }
  }

  // 按深度排序（最远的先绘制）
  std::sort(
      facesToDraw.begin(), facesToDraw.end(),
      [](const FaceData &a, const FaceData &b) { return a.depth > b.depth; });

  // 绘制面（现在是填充的）
  for (const auto &face : facesToDraw) {
    drawPolygon(win, face.points, face.colorPair, face.colorChar);
  }

  drawUI(win, width, height);
}

void RubiksCube::drawUI(WINDOW *win, int width, int height) {
  std::string title = "3x3 Rubik's Cube";
  if (width >= static_cast<int>(title.length())) {
    mvwprintw(win, 0, (width - title.length()) / 2, "%s", title.c_str());
  }

  updateViewMapping();

  // 安全地获取颜色名称
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
      // 忽略异常
    }
    return "Unknown";
  };

  std::vector<std::string> controls = {
      "Controls:",
      "  Arrow Keys - Rotate cube",
      "  +/-        - Zoom in/out",
      "  C          - Reset cube",
      "  X          - Scramble cube",
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
    // Draw box border
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

    // Draw text
    for (size_t i = 0; i < controls.size(); i++) {
      mvwprintw(win, boxY + 1 + static_cast<int>(i), boxX + 2, "%s",
                controls[i].c_str());
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

  rotation = Quaternion(1, 0, 0, 0);
  scale = 25.0f;
  position = Vector3(0, 0, 10);
  animating = false;
  animationProgress = 0.0f;
  currentAnimation = std::make_tuple(Vector3(), "", false);
  animationPieces.clear();
  animationRotation = Quaternion(1, 0, 0, 0);

  viewMapping = {{"F", "F"}, {"B", "B"}, {"L", "L"},
                 {"R", "R"}, {"U", "U"}, {"D", "D"}};
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
}
