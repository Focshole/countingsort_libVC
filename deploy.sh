#!/bin/bash
rm -rf libVC/build/
rm -rf build/
rm -rf cmake/

mkdir libVC/build
mkdir build
mkdir cmake

cd libVC/build
cmake .. -DUSE_LIBTOOL=1 -DCMAKE_INSTALL_PREFIX="."
make && make install || { echo "Failed to build libVC" && exit 2; }

cd ../..
cp libVC/config/FindLibVersioningCompiler.cmake cmake/

cd build/
cmake .. -DCMAKE_INSTALL_PREFIX="." -DUSE_LIBTOOL=1
make && make install || { echo "Failed to build program" && exit 2; }
