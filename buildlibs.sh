#!/bin/bash -ex

echo "Will build libs"
cd libs

echo "libsoil..."
cd libSOIL
./build.sh
cd ..
echo "done"

echo "libtinythread..."
cd libtinythread
./build.sh
cd ..
echo "done"

echo "libcdf..."
cd libcdf
./build.sh
cd ..
echo "done"

echo "libnoise..."
mkdir -p libnoise/build
cd libnoise/build
/usr/local/bin/cmake ..
make
cd ..
echo "done"

cd ..
echo "all built"

