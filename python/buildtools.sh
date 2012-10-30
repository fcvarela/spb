#!/bin/bash

gcc -c frustumtools.c -o frustumtools.o
gcc -c meshtools.c -o meshtools.o

gcc -dynamiclib -flat_namespace -o ctools.dylib frustumtools.o meshtools.o -framework OpenGL

