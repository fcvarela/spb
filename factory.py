from numpy import array
import numpy as np

from planet import *
from node import *

camera = Node()
camera.position = [0.2, 0.2, 3.0]
camera.nodes = {'yaw': Node()}

# planet instance
planet = None

# some properties
wireframe = False

keys = []
specialkeys = []
for i in range(256):
    keys.append(False)
    specialkeys.append(False)

def geocentricToCarthesian(lat, lon, alt):
    position = array([0., 0., 0.])

    position[2] = math.cos(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[0] = math.sin(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[1] = math.sin(lat * 0.0174532925) * alt

    return position