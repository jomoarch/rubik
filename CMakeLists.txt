cmake_minimum_required(VERSION 3.10)
project(rubik)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows特定设置
if(WIN32)
    find_path(PDCURSES_INCLUDE_DIR curses.h PATH_SUFFIXES pdcurses)
    find_library(PDCURSES_LIBRARY pdcurses)
    
    if(PDCURSES_INCLUDE_DIR AND PDCURSES_LIBRARY)
        include_directories(${PDCURSES_INCLUDE_DIR})
        set(CURSES_LIBRARIES ${PDCURSES_LIBRARY})
    else()
        message(FATAL_ERROR "PDCurses not found! Please install PDCurses.")
    endif()
else()
    # Linux/Mac使用ncurses
    find_package(Curses REQUIRED)
endif()

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

# 链接库
target_link_libraries(rubik ${CURSES_LIBRARIES})
