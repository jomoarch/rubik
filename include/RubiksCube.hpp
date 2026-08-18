#ifndef RUBIKSCUBE_HPP
#define RUBIKSCUBE_HPP

#include "ColorConverter.hpp"
#include "Enums.hpp"
#include "RubiksCubePiece.hpp"
#include <chrono>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define PDC_NCMOUSE
#include <pdcurses.h>
#else
#include <curses.h>
#endif

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
  Vector3 position;      ///< 魔方在世界坐标系中的位置
  Vector3 lightPosition; ///< 点光源位置（位于摄像头上方）

  static constexpr int NUM_LIGHT_LEVELS = 8; ///< 光照量化级数
  static constexpr float AMBIENT_INTENSITY =
      0.28f; ///< 环境光强度（暗面保底可见）
  static constexpr float DIFFUSE_STRENGTH = 0.68f; ///< 漫反射强度
  static constexpr float ATTENUATION_FACTOR =
      0.002f;                              ///< 距离衰减系数（温和，主要靠方向）
  int shadedColorLUT[6][NUM_LIGHT_LEVELS]; ///< 预计算：[基色索引][光照等级] ->
                                           ///< 256色终端索引

  float aspectRatio;      ///< 宽高比
  Vector3 cameraPosition; ///< 相机位置
  float focalLength;      ///< 焦距

  // Animation
  bool animating;          ///< 是否正在进行动画
  float animationProgress; ///< 动画进度（0-1）
  bool dirty;              ///< 脏标记：状态变化时设为 true，渲染后清除
  std::tuple<Vector3, std::string, bool>
      currentAnimation; ///< 当前动画信息（轴，面，方向）
  std::chrono::steady_clock::time_point animationStartTime; ///< 动画开始时间
  std::vector<std::shared_ptr<RubiksCubePiece>>
      animationPieces;          ///< 动画涉及的块
  Quaternion animationRotation; ///< 动画旋转四元数

  // View mapping
  std::unordered_map<std::string, std::string>
      viewMapping; ///< 视图方向到实际魔方面的映射
  std::unordered_map<std::string, Vector3> viewDirections; ///< 视图方向向量

  // Constants
  static constexpr float ANIMATION_DURATION = 0.3f; ///< 动画持续时间（秒）
  static constexpr float ROTATION_ANGLE =
      3.14159265359f / 2.0f;                  ///< 单次旋转角度（90度）
  static constexpr float LIGHT_HEIGHT = 7.0f; ///< 光源在摄像头上方的高度

  // Color definitions
  static const std::vector<RGB> COLOR_RGB;           ///< RGB颜色定义
  static const std::vector<char> COLOR_CHARS;        ///< 颜色字符表示
  static const std::vector<std::string> COLOR_NAMES; ///< 颜色名称
  static const std::unordered_map<std::string, Color>
      FACE_TO_COLOR; ///< 面到颜色的映射
  static const std::unordered_map<std::string, Vector3>
      ROTATION_AXES; ///< 面旋转轴定义

  std::vector<std::pair<Vector3, bool>> history; // 存储历史旋转

  void rotateFaceInternal(const std::string &actualFace, bool clockwise,
                          bool recordHistory);

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
   * @brief 获取块指定面的角点（传入已计算好的 piece 位置，避免重复计算）
   */
  std::vector<Vector3>
  getPieceFaceCorners(const std::shared_ptr<RubiksCubePiece> &piece,
                      const std::string &faceName,
                      const Vector3 &piecePos) const;

  /**
   * @brief 构建着色查找表（LUT）：预计算每种基色在每个光照等级下的 256 色索引
   * @details 在构造函数中调用一次，运行时直接查表，消除 to256Color() 的重复计算
   */
  void buildShadingLUT();

  /**
   * @brief 计算面的光照等级（量化后的整数，0..NUM_LIGHT_LEVELS-1）
   * @param normalWorld 世界空间面法线
   * @param worldCenter 面中心世界坐标
   * @return 量化光照等级
   * @details 采用 Lambert 漫反射 + 距离衰减 + 环境光，结果量化以命中 LUT
   */
  int calculateLightLevel(const Vector3 &normalWorld,
                          const Vector3 &worldCenter) const;

  /**
   * @brief 计算面在光照下的显示颜色（含光影处理）
   * @param baseColor 基础颜色
   * @param normal 面法线（世界空间）
   * @param worldPos 面中心世界坐标
   * @return 经 Lambert 漫反射 + 环境光着色后的 RGB
   * @note 渲染主循环使用 LUT 快速路径，此方法保留用于辅助/调试场景
   */
  RGB calculateShadedColor(const RGB &baseColor, const Vector3 &normal,
                           const Vector3 &worldPos) const;

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
   * @brief 在多边形的边上绘制黑线（模拟 Rubik's cube 贴纸边框）
   */
  void drawPolygonEdges(WINDOW *win,
                        const std::vector<std::pair<int, int>> &points);

  /**
   * @brief 绘制用户界面（控制说明和状态信息）
   * @param win ncurses窗口指针
   * @param width 窗口宽度
   * @param height 窗口高度
   * @param colorCache 颜色缓存（用于给面颜色名称上色）
   */
  void drawUI(WINDOW *win, int width, int height,
              std::unordered_map<int, int> &colorCache);

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
  void draw(WINDOW *win, int width, int height,
            std::unordered_map<int, int> &colorCache);

  /**
   * @brief 重置魔方到初始状态（已解决状态）
   */
  void reset();

  /**
   * @brief 随机打乱魔方
   * @param moves 打乱步数，默认为20
   */
  void scramble(int moves = 20);

  /**
   * @brief 撤销上一步操作
   * @return 是否成功撤销（历史为空时返回 false，例如打乱后立即撤销）
   */
  bool undo();

  /**
   * @brief 获取用户最近执行的面旋转操作（本体坐标）
   * @param face 输出面字母（'F','B','L','R','U','D'）
   * @param cw 输出是否顺时针
   * @return 历史为空时返回 false，否则 true
   * @note 用于提示系统判断退格/逆操作的回正语义
   */
  bool getLastPerformedMove(char &face, bool &cw) const;

  /**
   * @brief 导出当前魔方状态的 54 贴纸 facelet 字符串（min2phase 输入格式）
   * @details 按 U1..U9, R1..R9, F1..F9, D1..D9, L1..L9, B1..B9 的固定顺序
   *          收集每个贴纸颜色，并映射为面符号（白->U、黄->D、红->B、
   *          橙->F、蓝->L、绿->R，与 FACE_TO_COLOR 一致）。
   * @note 会先完成任何进行中的旋转动画
   */
  std::string getFaceletString();

  /**
   * @brief 获取视图方向到实际魔方面的映射（只读）
   */
  const std::unordered_map<std::string, std::string> &getViewMapping() const {
    return viewMapping;
  }

  /**
   * @brief 获取指定实际面对应的颜色
   * @param face 实际面名（"F","B","L","R","U","D"）
   * @return 该面的颜色，未知面返回 _COLOR_NONE
   */
  Color getFaceColor(const std::string &face) const;

  /**
   * @brief 获取面颜色 RGB 表（按 Color 枚举下标索引，与渲染共用同一套配色）
   */
  static const std::vector<RGB> &getColorRGB() { return COLOR_RGB; }

  /**
   * @brief 检查是否需要重绘
   */
  bool needsRedraw() const { return dirty || animating; }

  /**
   * @brief 清除脏标记（渲染完成后调用）
   */
  void clearDirty() { dirty = false; }
};

#endif
