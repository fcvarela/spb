#!/bin/bash -ex

cd libs

find . | grep grep "[.]a$" | xargs rm
find . | grep grep "[.]o$" | xargs rm
find . | grep grep "[.]dylib$" | xargs rm

cd ..
rm -rf libs/libnoise/build

