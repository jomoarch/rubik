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

/**
 * @class RubiksCube
 * @brief 3x3魔方类，管理所有魔方块并提供渲染和交互功能
 * @details 使用四元数进行旋转，支持3D投影到终端显示
 */
class RubiksCube {
private:
  std::vector<std::shared_ptr<RubiksCubePiece>> pieces; ///< 所有魔方块的集合
  Quaternion rotation;                                  ///< 魔方的整体旋转
  float scale;                                          ///< 缩放因子
  Vector3 position; ///< 魔方在世界坐标系中的位置
  Vector3 lightDir; ///< 光照方向（用于计算亮度）

  float aspectRatio;      ///< 宽高比
  Vector3 cameraPosition; ///< 相机位置
  float focalLength;      ///< 焦距

  // Animation
  bool animating;          ///< 是否正在进行动画
  float animationProgress; ///< 动画进度（0-1）
  std::tuple<Vector3, std::string, bool>
      currentAnimation; ///< 当前动画信息（轴，面，方向）
  std::chrono::steady_clock::time_point animationStartTime; ///< 动画开始时间
  std::vector<std::shared_ptr<RubiksCubePiece>>
      animationPieces;          ///< 动画涉及的块
  Quaternion animationRotation; ///< 动画旋转四元数

  // View mapping
  std::map<std::string, std::string>
      viewMapping;                               ///< 视图方向到实际魔方面的映射
  std::map<std::string, Vector3> viewDirections; ///< 视图方向向量

  // Constants
  static constexpr float ANIMATION_DURATION = 0.3f; ///< 动画持续时间（秒）
  static constexpr float ROTATION_ANGLE =
      3.14159265359f / 2.0f; ///< 单次旋转角度（90度）

  // Color definitions
  static const std::vector<RGB> COLOR_RGB;                   ///< RGB颜色定义
  static const std::vector<char> COLOR_CHARS;                ///< 颜色字符表示
  static const std::vector<std::string> COLOR_NAMES;         ///< 颜色名称
  static const std::map<std::string, Color> FACE_TO_COLOR;   ///< 面到颜色的映射
  static const std::map<std::string, Vector3> ROTATION_AXES; ///< 面旋转轴定义

  /**
   * @brief 创建所有魔方块（27个）
   */
  void createPieces();

  /**
   * @brief 更新视图映射（根据当前旋转确定哪个面朝前等）
   */
  void updateViewMapping();

  /**
   * @brief 完成当前动画，更新块状态
   */
  void completeAnimation();

  /**
   * @brief 获取指定面上的所有块
   * @param faceChar 面字符（"F", "B", "L", "R", "U", "D"）
   * @return 该面上的块集合
   */
  std::vector<std::shared_ptr<RubiksCubePiece>>
  getPiecesOnFace(const std::string &faceChar);

  /**
   * @brief 获取块的当前位置（考虑动画）
   * @param piece 块指针
   * @return 当前位置
   */
  Vector3 getPiecePosition(const std::shared_ptr<RubiksCubePiece> &piece) const;

  /**
   * @brief 获取块指定面的颜色（考虑动画）
   * @param piece 块指针
   * @param faceName 面名称
   * @return 颜色
   */
  Color getPieceFaceColor(const std::shared_ptr<RubiksCubePiece> &piece,
                          const std::string &faceName) const;

  /**
   * @brief 获取块指定面的角点坐标（世界坐标系）
   * @param piece 块指针
   * @param faceName 面名称
   * @return 角点坐标向量
   */
  std::vector<Vector3>
  getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                      const std::string &faceName) const;

  /**
   * @brief 根据法线计算亮度
   * @param normal 面法线
   * @return 亮度值（0.25-1.2）
   */
  float calculateBrightness(const Vector3 &normal) const;

  /**
   * @brief 将3D点投影到2D屏幕
   * @param point 3D点
   * @param width 屏幕宽度
   * @param height 屏幕高度
   * @return 包含(x, y, depth)的元组
   */
  std::tuple<int, int, float> projectPoint(const Vector3 &point, int width,
                                           int height) const;

  /**
   * @brief 在终端上绘制填充多边形
   * @param win ncurses窗口指针
   * @param points 多边形顶点坐标
   * @param colorPair 颜色对索引
   * @param colorChar 表示颜色的字符
   */
  void drawPolygon(WINDOW *win, const std::vector<std::pair<int, int>> &points,
                   int colorPair, char colorChar);

  /**
   * @brief 绘制用户界面（控制说明和状态信息）
   * @param win ncurses窗口指针
   * @param width 窗口宽度
   * @param height 窗口高度
   */
  void drawUI(WINDOW *win, int width, int height);

public:
  /**
   * @brief 构造函数，初始化魔方
   */
  RubiksCube();

  /**
   * @brief 根据鼠标移动旋转魔方
   * @param dx 水平移动量
   * @param dy 垂直移动量
   */
  void rotateByMouseDelta(float dx, float dy);

  /**
   * @brief 缩放魔方
   * @param factor 缩放因子（正为放大，负为缩小）
   */
  void zoom(float factor);

  /**
   * @brief 根据视图方向旋转魔方面
   * @param viewDirection 视图方向（"F", "B", "L", "R", "U", "D"）
   * @param clockwise 是否顺时针旋转
   */
  void rotateViewDirection(const std::string &viewDirection, bool clockwise);

  /**
   * @brief 更新动画状态
   */
  void updateAnimation();

  /**
   * @brief 绘制魔方到终端窗口
   * @param win ncurses窗口指针
   * @param width 窗口宽度
   * @param height 窗口高度
   * @param colorCache 颜色缓存，避免重复初始化颜色对
   */
  void draw(WINDOW *win, int width, int height, std::map<int, int> &colorCache);

  /**
   * @brief 重置魔方到初始状态（已解决状态）
   */
  void reset();

  /**
   * @brief 随机打乱魔方
   * @param moves 打乱步数，默认为20
   */
  void scramble(int moves = 20);
};

#endif
