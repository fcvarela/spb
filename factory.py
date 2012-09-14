from numpy import array
import numpy as np
import threading
import subprocess

from Queue import LifoQueue

from OpenGL import *
from OpenGL.GL import *

from planet import *
from node import *

from ctypes import *

#debug
import ubigraph
G = ubigraph.Ubigraph()
G.clear()
# debug
gRoot = factory.G.newVertex(shape="sphere", size="1.0", label="Planet")
gQuadtreeStyle = G.newVertexStyle(shape="sphere", color="#0000FF", size="1.0")
gQuadtreeNodeStyle = G.newVertexStyle(shape="sphere", color="#FF0000", size="1.0")

lib = cdll.LoadLibrary('./ctools.dylib')
lib.boxInFrustum.restype = c_int
lib.sphereInFrustum.restype = c_int
lib.veclen.restype = c_double
vecptr = c_double*3

generatorQueue = LifoQueue()
drawnNodes = 0
trackFrustum = True

def geocentricToCarthesian(lat, lon, alt):
    position1 = vecptr()
    lib.geocentricToCarthesian(position1, c_float(lat), c_float(lon))
    return array(list(position1))*alt

def carthesianToGeocentric(position):
    latlon = vecptr()
    lib.carthesianToGeocentric(latlon, vecptr(*position))
    return array(list(latlon))

def calculateFrustum():
    if trackFrustum:
        lib.calculateFrustum()

def boxInFrustum(box):
    return lib.boxInFrustum(box)

def sphereInFrustum(sphere):
    return lib.sphereInFrustum(sphere)

def veclen(vec):
    length = lib.veclen(vecptr(*vec))
    return length

def cross(a, b):
    output = range(0, 3)
    output[0] = a[1] * b[2] - a[2] * b[1];
    output[1] = a[2] * b[0] - a[0] * b[2];
    output[2] = a[0] * b[1] - a[1] * b[0];

    return output

def normalize(a):
    b = vecptr(*a)
    lib.normalize(b)
    return array(list(b))

def dot(a, b):
    output = a[0]*b[0] + a[1] * b[1] + a[2] * b[2]
    return output


camera = Node()

sun = Node()

# planet instance
planet = None

# some properties
wireframe = False
trackFrustum = True

keys = []
for i in range(512):
    keys.append(False)