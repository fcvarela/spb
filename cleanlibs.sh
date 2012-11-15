#!/bin/bash -ex

cd libs

find . | grep .a$ | xargs rm
find . | grep .o$ | xargs rm
find . | grep .dylib | xargs rm

cd ..
rm -rf libs/libnoise/build

