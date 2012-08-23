from ctypes import cdll
import math

class generator:
    def __init__(self):
        self.lib = cdll.LoadLibrary('./genlib.dylib')

    def generate(baselat, baselon, latspan, lonspan, width, height):
        d2r = math.pi/180.0

        for y in range(0, height):
            latitude = baselat + float(y)/float(height) * latspan
            for x in range(0, width):
                longitude = float(x)/float(width) * lonspan

            coords = range(3)
            coords[2] = math.cos(lon) * math.cos(lat);
            coords[0] = math.sin(lon) * math.cos(lat);
            coords[1] = math.sin(lat);

        H = 0.8
        lacunarity = 1.5
        octaves = 10
        offset = 0.8
        threshold = 40.0

        out = self.lib.RidgedMultifractal(coords, H, lacunarity, octaves, offset, threshold)
        