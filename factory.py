from numpy import array
import numpy as np

import threading

from planet import *
from node import *

import subprocess
from Queue import LifoQueue
from threading import Thread

generatorQueue = LifoQueue()

def geocentricToCarthesian(lat, lon, alt):
    position = [0., 0., 0.]

    position[2] = math.cos(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[0] = math.sin(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[1] = math.sin(lat * 0.0174532925) * alt

    return position

def generatorThread():
    while True:
        # dequeue everything into a local list
        entries = []
        
        while not generatorQueue.empty():
            (command, instance) = generatorQueue.get_nowait()
            entries.append((command, instance, np.linalg.norm(camera.position - instance.center)))

        # sort the list acording to distance from camera to instance
        entries = sorted(entries, key=lambda entry: entry[2])

        for entry in entries:
            generatorQueue.put((entry[0], entry[1]))

        (command, instance) = generatorQueue.get(True, None)
        subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).communicate()
        instance.ready = True


camera = Node()
camera.nodes = {'yaw': Node()}

sun = Node()

# planet instance
planet = None

# some properties
wireframe = False

# terrain generator
for i in range(0, 8):
    genthread = Thread(target=generatorThread, args=())
    genthread.start()

keys = []
specialkeys = []
for i in range(256):
    keys.append(False)
    specialkeys.append(False)