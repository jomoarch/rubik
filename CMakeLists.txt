cmake_minimum_required(VERSION 3.10)
project(rubik)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 寻找ncurses库
find_package(Curses REQUIRED)

# 包含目录
include_directories(src)

# 添加可执行文件
add_executable(rubik
    src/main.cpp
    src/Vector3.cpp
    src/Quaternion.cpp
    src/ColorConverter.cpp
    src/RubiksCubePiece.cpp
    src/RubiksCube.cpp
)

# 链接ncurses库
target_link_libraries(rubik ${CURSES_LIBRARIES})
