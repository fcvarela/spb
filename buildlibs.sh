#!/bin/bash

cd libs

cd libSOIL
./build.sh
cd ..

cd libtinythread
./build.sh
cd ..

cd libcdf
./build.sh
cd ..

mkdir -p libnoise/build
cd libnoise/build
cmake ..
make
cd ..

cd ..
