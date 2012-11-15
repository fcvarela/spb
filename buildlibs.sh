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

cd libnoise/build
make
cd ..

cd ..
