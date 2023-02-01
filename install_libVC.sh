#!/bin/bash

rm -rf libVC
git clone https://github.com/Focshole/libVersioningCompiler.git libVC &&
cd libVC &&
git checkout libtool-dev &&
mkdir build &&
cd build &&
cmake -DUSE_LIBTOOL=1 -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
mkdir -p ../../cmake &&
cp lib/cmake/FindLibVersioningCompiler.cmake ../../cmake/FindLibVersioningCompiler.cmake &&
cd ../../


