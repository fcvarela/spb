from ctypes import *
import numpy as np
import math

lib = cdll.LoadLibrary('./genlib.dylib')
c_double_ptr = c_double*3
lib.RidgedMultifractal.argtypes = [c_double_ptr, c_double, c_double, c_int, c_double, c_double]
lib.RidgedMultifractal.restype = c_double

d2r = math.pi/180.0

width = 256
height = 128

baselat = -90.0
baselon = -180.0
latspan = 180.0
lonspan = 360.0

H = 0.8
lacunarity = 2.5
octaves = 1
offset = 0.8
threshold = 10.0

coords = range(0, 3)

for y in range(0, height):
    lat = baselat + float(y)/float(height) * latspan
    for x in range(0, width):
        lon = float(x)/float(width) * lonspan

    coords[2] = math.cos(lon) * math.cos(lat)
    coords[0] = math.sin(lon) * math.cos(lat)
    coords[1] = math.sin(lat)
    
    out = lib.RidgedMultifractal(c_double_ptr(*coords), H, lacunarity, octaves, offset, threshold)
    print out