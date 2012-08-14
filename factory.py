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
