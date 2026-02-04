#ifndef RUBIKSCUBE_HPP
#define RUBIKSCUBE_HPP

#include "ColorConverter.hpp"
#include "Enums.hpp" // 包含枚举定义
#include "RubiksCubePiece.hpp"
#include <chrono>
#include <curses.h>
#include <map>
#include <memory>
#include <vector>

class RubiksCube {
private:
  std::vector<std::shared_ptr<RubiksCubePiece>> pieces;
  Quaternion rotation;
  float scale;
  Vector3 position;
  Vector3 lightDir;

  float aspectRatio;
  Vector3 cameraPosition;
  float focalLength;

  // Animation
  bool animating;
  float animationProgress;
  std::tuple<Vector3, std::string, bool> currentAnimation;
  std::chrono::steady_clock::time_point animationStartTime;
  std::vector<std::shared_ptr<RubiksCubePiece>> animationPieces;
  Quaternion animationRotation;

  // View mapping
  std::map<std::string, std::string> viewMapping;
  std::map<std::string, Vector3> viewDirections;

  // Constants
  static constexpr float ANIMATION_DURATION = 0.3f;
  static constexpr float ROTATION_ANGLE = 3.14159265359f / 2.0f;

  // Color definitions
  static const std::vector<RGB> COLOR_RGB;
  static const std::vector<char> COLOR_CHARS;
  static const std::vector<std::string> COLOR_NAMES;
  static const std::map<std::string, Color> FACE_TO_COLOR;
  static const std::map<std::string, Vector3> ROTATION_AXES;

  void createPieces();
  void updateViewMapping();
  void completeAnimation();
  std::vector<std::shared_ptr<RubiksCubePiece>>
  getPiecesOnFace(const std::string &faceChar);
  Vector3 getPiecePosition(const std::shared_ptr<RubiksCubePiece> &piece) const;
  Color getPieceFaceColor(const std::shared_ptr<RubiksCubePiece> &piece,
                          const std::string &faceName) const;
  std::vector<Vector3>
  getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                      const std::string &faceName) const;
  float calculateBrightness(const Vector3 &normal) const;
  std::tuple<int, int, float> projectPoint(const Vector3 &point, int width,
                                           int height) const;
  void drawPolygon(WINDOW *win, const std::vector<std::pair<int, int>> &points,
                   int colorPair, char colorChar);
  void drawDirectionMarker(WINDOW *win, int width, int height);
  void drawUI(WINDOW *win, int width, int height);
  std::shared_ptr<RubiksCubePiece> getFrontTopEdgePiece() const;

public:
  RubiksCube();

  void rotateByMouseDelta(float dx, float dy);
  void zoom(float factor);
  void rotateViewDirection(const std::string &viewDirection, bool clockwise);
  void updateAnimation();
  void draw(WINDOW *win, int width, int height, std::map<int, int> &colorCache);
  void reset();
  void scramble(int moves = 20);
};

#endif
