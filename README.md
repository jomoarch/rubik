# rubik

一个使用 C++ 和 ncurses 库编写的 3D 魔方模拟器，可在终端中运行。

## 存在的问题
局部旋转时会出现过度旋转，具体为每个块的坐标被旋转 90 度，而其上的贴纸则被旋转了 180 度。

## 构建方法
```bash
cd rubik
chmod +x build.sh
./build.sh
./build/rubik
```
