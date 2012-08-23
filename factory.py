from numpy import array
import numpy as np
import threading
from planet import *
from node import *
import subprocess
from Queue import LifoQueue

generatorQueue = LifoQueue()

def geocentricToCarthesian(lat, lon, alt):
    position = [0., 0., 0.]

    position[2] = math.cos(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[0] = math.sin(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[1] = math.sin(lat * 0.0174532925) * alt

    return position

camera = Node()
camera.nodes = {'yaw': Node()}

sun = Node()

# planet instance
planet = None

# some properties
wireframe = False

keys = []
for i in range(512):
    keys.append(False)