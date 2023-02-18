#!/bin/bash
rm -rf libVC/build/ &&
rm -rf libVersioningCompilerProd/build/ &&
rm -rf libVersioningCompilerCons/build/ &&
rm -rf build/ &&
rm -rf cmake/ &&

mkdir libVC/build &&
mkdir libVersioningCompilerProd/build &&
mkdir libVersioningCompilerCons/build &&
mkdir build &&
mkdir cmake &&

cd libVC/build &&
cmake .. -DCMAKE_INSTALL_PREFIX="." &&
make && make install || { echo "Failed to build libVC" && exit 2; } &&
cd ../.. &&

cd libVersioningCompilerProd/build &&
cmake .. -DCMAKE_INSTALL_PREFIX="." &&
make && make install || { echo "Failed to build libVCProd" && exit 2; } &&
cd ../.. &&

cd libVersioningCompilerCons/build &&
cmake .. -DCMAKE_INSTALL_PREFIX="." &&
make && make install || { echo "Failed to build libVCCons" && exit 2; } &&
cd ../.. &&

cp libVC/build/lib/cmake/FindLibVersioningCompiler.cmake cmake/ &&
cp libVersioningCompilerProd/build/lib/cmake/FindLibVersioningCompilerProd.cmake cmake/ &&
cp libVersioningCompilerCons/build/lib/cmake/FindLibVersioningCompilerCons.cmake cmake/ &&
cp libVersioningCompilerProd/cmake/Findlibzmq.cmake cmake/ &&

cd build/ &&
cmake .. -DCMAKE_INSTALL_PREFIX="." &&
make && make install || { echo "Failed to build program" && exit 2; }
