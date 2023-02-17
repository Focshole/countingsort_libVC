#!/bin/bash

rm -rf libVC
git clone https://github.com/Focshole/libVersioningCompiler.git libVC &&
cd libVC &&
git checkout dev &&
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cd ../../ &&
# Install libVC-dht-prod
rm -rf libVersioningCompilerProd &&
git clone https://github.com/Focshole/libvc-dht-producer libVersioningCompilerProd &&
cd libVersioningCompilerProd &&
ln -s ../libVC libVC  &&
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cd ../../ &&
# Install libVC-dht-cons
rm -rf libVersioningCompilerCons &&
ln -s libVC ../libVC &&
git clone https://github.com/Focshole/libvc-dht-consumer libVersioningCompilerCons &&
cd libVersioningCompilerCons &&
ln -s ../libVC libVC  &&
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cd ../../ &&
# Copy cmake files
mkdir -p ../../cmake &&
cp lib/cmake/FindLibVersioningCompiler.cmake ../../cmake/FindLibVersioningCompiler.cmake &&
cd ../../


