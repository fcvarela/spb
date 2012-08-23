#!/bin/bash

#g++ -shared -Wl,soname,libgenlib.so -o libgenlib.so genlib.o noiseutils.o
gcc -dynamiclib -flat_namespace -o genlib.dylib *.c
