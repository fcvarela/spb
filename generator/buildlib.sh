#!/bin/bash

#g++ -shared -Wl,soname,libgenlib.so -o libgenlib.so genlib.o noiseutils.o
g++ -dynamiclib -flat_namespace -o genlib.dylib -lnoise genlib.cpp noiseutils.cpp

