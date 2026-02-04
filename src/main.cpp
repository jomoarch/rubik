#include "RubiksCube.hpp"
#include <chrono>
#include <curses.h>
#include <iostream>
#include <thread>

void printInstructions() {
  std::cout << "======================================" << std::endl;
  std::cout << "      3x3 Rubik's Cube Simulator      " << std::endl;
  std::cout << "======================================" << std::endl;
  std::cout << std::endl;
  std::cout << "Controls:" << std::endl;
  std::cout << "  Arrow Keys - Rotate cube" << std::endl;
  std::cout << "  +/-        - Zoom in/out" << std::endl;
  std::cout << "  C          - Reset cube" << std::endl;
  std::cout << "  X          - Scramble cube" << std::endl;
  std::cout << "  ESC        - Exit" << std::endl;
  std::cout << std::endl;
  std::cout << "Rotate faces (based on current view):" << std::endl;
  std::cout << "  f/F - Front clockwise/counter" << std::endl;
  std::cout << "  b/B - Back clockwise/counter" << std::endl;
  std::cout << "  l/L - Left clockwise/counter" << std::endl;
  std::cout << "  r/R - Right clockwise/counter" << std::endl;
  std::cout << "  u/U - Up clockwise/counter" << std::endl;
  std::cout << "  d/D - Down clockwise/counter" << std::endl;
  std::cout << std::endl;
  std::cout << "======================================" << std::endl;
  std::cout << "   Please use full-screen terminal    " << std::endl;
  std::cout << "======================================" << std::endl;
  std::cout << std::endl;
  std::cout << "Press Enter to start..." << std::endl;

  std::cin.ignore();
}

int main() {
  printInstructions();

  // Initialize ncurses
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  if (!has_colors()) {
    endwin();
    std::cout << "Terminal does not support colors!" << std::endl;
    return 1;
  }

  start_color();
  use_default_colors();

  // Create cube
  RubiksCube cube;
  std::map<int, int> colorCache;

  try {
    while (true) {
      int ch = getch();

      if (ch == 27) { // ESC
        break;
      } else if (ch == 'c' || ch == 'C') {
        cube.reset();
      } else if (ch == 'x' || ch == 'X') {
        cube.scramble(20);
      } else if (ch == KEY_UP) {
        cube.rotateByMouseDelta(0, -10);
      } else if (ch == KEY_DOWN) {
        cube.rotateByMouseDelta(0, 10);
      } else if (ch == KEY_LEFT) {
        cube.rotateByMouseDelta(-10, 0);
      } else if (ch == KEY_RIGHT) {
        cube.rotateByMouseDelta(10, 0);
      } else if (ch == '+' || ch == '=') {
        cube.zoom(1);
      } else if (ch == '-' || ch == '_') {
        cube.zoom(-1);
      } else if (ch == 'f') {
        cube.rotateViewDirection("F", true);
      } else if (ch == 'F') {
        cube.rotateViewDirection("F", false);
      } else if (ch == 'b') {
        cube.rotateViewDirection("B", true);
      } else if (ch == 'B') {
        cube.rotateViewDirection("B", false);
      } else if (ch == 'l') {
        cube.rotateViewDirection("L", true);
      } else if (ch == 'L') {
        cube.rotateViewDirection("L", false);
      } else if (ch == 'r') {
        cube.rotateViewDirection("R", true);
      } else if (ch == 'R') {
        cube.rotateViewDirection("R", false);
      } else if (ch == 'u') {
        cube.rotateViewDirection("U", true);
      } else if (ch == 'U') {
        cube.rotateViewDirection("U", false);
      } else if (ch == 'd') {
        cube.rotateViewDirection("D", true);
      } else if (ch == 'D') {
        cube.rotateViewDirection("D", false);
      }

      int width, height;
      getmaxyx(stdscr, height, width);

      if (width < 80 || height < 40) {
        clear();
        std::string msg = "Please resize terminal to at least 80x40";
        mvprintw(height / 2,
                 std::max(0, (width - static_cast<int>(msg.length())) / 2),
                 "%s", msg.c_str());
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        continue;
      }

      cube.draw(stdscr, width, height, colorCache);
      refresh();

      std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
  } catch (const std::exception &e) {
    endwin();
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  endwin();
  std::cout << "Game ended. Goodbye!" << std::endl;
  return 0;
}
