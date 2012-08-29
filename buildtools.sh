#!/bin/bash

gcc -dynamiclib -flat_namespace -o frustumtools.dylib frustumtools.c -framework OpenGL

