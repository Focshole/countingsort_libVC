#!/bin/bash

rm -rf libVC
git clone https://github.com/skeru/libVersioningCompiler.git libVC &&
cd libVC &&
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
mkdir -p ../../cmake &&
cp lib/cmake/FindLibVersioningCompiler.cmake ../../cmake/FindLibVersioningCompiler.cmake &&
cd ../../


