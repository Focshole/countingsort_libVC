#!/bin/bash
# Clone libVC
rm -rf libVC
git clone https://github.com/Focshole/libVersioningCompiler.git libVC &&
cd libVC &&
git checkout renovation-work &&
# Build libVC
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cp lib/cmake/FindLibVersioningCompiler.cmake ../../cmake/FindLibVersioningCompiler.cmake &&
cd ../../ &&
# Install libVC-dht-prod
rm -rf libVersioningCompilerProd &&
git clone https://github.com/Focshole/libvc-dht-producer libVersioningCompilerProd &&
cd libVersioningCompilerProd &&
# add libVC cmake file
cp ../libVC/build/lib/cmake/FindLibVersioningCompiler.cmake ./cmake/ &&
# Build libVC-dht-prod
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cd ../../ &&
# Install libVC-dht-cons
rm -rf libVersioningCompilerCons &&
git clone https://github.com/Focshole/libvc-dht-consumer libVersioningCompilerCons &&
cd libVersioningCompilerCons &&
# add libVC cmake file
cp ../libVC/build/lib/cmake/FindLibVersioningCompiler.cmake ./cmake/ &&
# Build libVC-dht-cons
mkdir build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX="." .. &&
make -j4 &&
make install &&
cd ../../ &&
# Copy cmake files
mkdir -p cmake &&
cp libVC/build/lib/cmake/FindLibVersioningCompiler.cmake ./cmake/ &&
cp libVersioningCompilerProd/build/lib/cmake/FindLibVersioningCompilerProd.cmake cmake/ &&
cp libVersioningCompilerProd/cmake/Findlibzmq.cmake cmake/ && # It is enough to use the one from libVC-dht-prod
cp libVersioningCompilerCons/build/lib/cmake/FindLibVersioningCompilerCons.cmake cmake/

