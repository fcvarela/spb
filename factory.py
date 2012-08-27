from numpy import array
import numpy as np
import threading
import subprocess

from Queue import LifoQueue

from OpenGL import *
from OpenGL.GL import *

from planet import *
from node import *

generatorQueue = LifoQueue()

# lrbtnf
globalFrustum = range(0, 6)
drawnNodes = 0

def geocentricToCarthesian(lat, lon, alt):
    position = [0., 0., 0.]

    position[2] = math.cos(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[0] = math.sin(lon * 0.0174532925) * math.cos(lat * 0.0174532925) * alt
    position[1] = math.sin(lat * 0.0174532925) * alt

    return position

def calculateFrustum():
    projectionMatrix = glGetFloatv(GL_PROJECTION_MATRIX)
    modelviewMatrix = glGetFloatv(GL_MODELVIEW_MATRIX)

    glPushMatrix()
    glLoadMatrixf(projectionMatrix)
    glMultMatrixf(modelviewMatrix)
    modelviewMatrix = glGetFloatv(GL_MODELVIEW_MATRIX).flatten('C')
    glPopMatrix()

    # extract planes
    global globalFrustum
    globalFrustum[0] = extractPlane(modelviewMatrix, 1)
    globalFrustum[1] = extractPlane(modelviewMatrix, -1)
    globalFrustum[2] = extractPlane(modelviewMatrix, 2)
    globalFrustum[3] = extractPlane(modelviewMatrix, -2)
    globalFrustum[4] = extractPlane(modelviewMatrix, 3)
    globalFrustum[5] = extractPlane(modelviewMatrix, -3)

def extractPlane(matrix, row):
    scale = 1.0
    if row < 0:
        scale = -1.0

    row = abs(row) - 1

    # calculate plane coefficients from the matrix
    plane = [0, 0, 0, 0]
    plane[0] = matrix[3] + scale * matrix[row]
    plane[1] = matrix[7] + scale * matrix[row + 4]
    plane[2] = matrix[11] + scale * matrix[row + 8]
    plane[3] = matrix[15] + scale * matrix[row + 12]

    # normalize the plane
    length = math.sqrt(\
        plane[0] * plane[0] + \
        plane[1] * plane[1] + \
        plane[2] * plane[2])
    plane[0] /= length
    plane[1] /= length
    plane[2] /= length
    plane[3] /= length

    return plane

def boxInFrustum(box):
    total_inside = 0

    for i in range(0, 6):
        in_count = 8
        pt_in = 1

        for j in range(0, 8):
           dist = \
            globalFrustum[i][0] * sphere[0] +\
            globalFrustum[i][1] * sphere[1] +\
            globalFrustum[i][2] * sphere[2] +\
            globalFrustum[i][3]
        
        if dist < 0:
            in_count -= 1
        if in_count == 0:
            return False

    return True

def sphereInFrustum(sphere):
    global drawnNodes

    # first check if the camera is inside the sphere
    distance = np.linalg.norm(array(factory.camera.position) - array(sphere[:3]))
    if abs(distance) < abs(sphere[3]):
        print "Camera inside sphere"
        return True

    dist = 0.0;
    for i in range(0, 6):
        dist = \
            globalFrustum[i][0] * sphere[0] +\
            globalFrustum[i][1] * sphere[1] +\
            globalFrustum[i][2] * sphere[2] +\
            globalFrustum[i][3]
        
        if dist < -sphere[3]:
            return False
        if abs(dist) < sphere[3]:
            return True

    return True

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