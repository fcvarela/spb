#!/bin/bash -ex

find . | grep .a$ | xargs rm
find . | grep .o$ | xargs rm
find . | grep .dylib | xargs rm

rm -rf libs/libnoise/build

