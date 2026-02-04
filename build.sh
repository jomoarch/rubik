#!/bin/bash
echo "Building the funny rubik ..."
if [ -d "build" ]; then
  rm -rf build
fi
mkdir -p build
cd build
cmake ..
make -j$(nproc)
if [ $? -eq 0 ]; then
  echo "Built all right!"
  echo "You can run './build/rubik' to play."
else
  echo "Failed!"
  exit 1
fi
