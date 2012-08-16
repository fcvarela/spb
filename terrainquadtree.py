import subprocess
from numpy import array
import numpy as np

from OpenGL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import factory

class TerrainQuadtree:
    def __init__(self, parent, maxlod, index, baselat, baselon, span, seed):
        self.parent = parent
        self.maxlod = maxlod
        self.baselat = baselat
        self.baselon = baselon
        self.index = index
        self.span = span
        self.seed = seed

        self.gridSize = 8
        self.gridSizep1 = self.gridSize + 1

        self.vertices = []
        self.indexes = []
        self.children = []

        self.positionBufferObject = None
        self.indexBufferObject = None

        self.generateVertices()

        self.generateTextures()
        self.loadTextures()

    def generateTextures(self):
        extra = '--width 256 --height 256 --south %f --north %f --west %f --east %f --outfile %d' % (\
            self.baselat, self.baselat+self.span, self.baselon-180., self.baselon-180.+self.span, self.index)
        command = "%s %s" % (self.seed(), extra)
        print command
        
        cmd = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        for line in cmd.stdout:
            pass

    def loadTextures(self):
        pass

    def generateVertices(self):
        step = self.span / self.gridSize

        for y in range(0, self.gridSizep1):
            lat = self.baselat + (self.gridSize-y)*step
            for x in range(0, self.gridSizep1):
                lon = self.baselon + x*step

                coord = factory.geocentricToCarthesian(lat, lon, 1.0)
                coord /= np.linalg.norm(coord)
                self.vertices.append(coord)

                if y == x and x == self.gridSize/2:
                    self.center = coord
                if y == 0 and x == 0:
                    self.topleft = coord
                if y == 0 and x == self.gridSize:
                    self.topright = coord
                if y == self.gridSize and x == 0:
                    self.botleft = coord
                if y == self.gridSize and x == self.gridSize:
                    self.botright = coord

        self.sidelength = np.linalg.norm(self.topleft - self.botleft)

        for y in range(0, self.gridSize):
            for x in range(0, self.gridSizep1):
                self.indexes.append(y * self.gridSizep1 + x)
                self.indexes.append((y+1) * self.gridSizep1 + x)

        self.vertices = array(self.vertices, dtype='float32')
        self.indexes = array(self.indexes, dtype='ushort')

        self.indexBufferObject = GL.glGenBuffers(1)
        GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)
        GL.glBufferData(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexes, GL.GL_STATIC_DRAW)

        self.positionBufferObject = GL.glGenBuffers(1)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.positionBufferObject)
        GL.glBufferData(GL.GL_ARRAY_BUFFER, self.vertices, GL.GL_STATIC_DRAW)

        self.vertices = None
        self.indexes = None

    def draw(self):
        # do we need to draw our children
        d1 = np.linalg.norm(factory.camera.position - self.center)
        d2 = np.linalg.norm(factory.camera.position - self.topleft)
        d3 = np.linalg.norm(factory.camera.position - self.topright)
        d4 = np.linalg.norm(factory.camera.position - self.botleft)
        d5 = np.linalg.norm(factory.camera.position - self.botright)

        distance = min(d1, min(d2, min(d3, min(d4, d5))))
        if self.maxlod > 0 and distance < self.sidelength*1.1:
            # are they ready?
            if len(self.children) > 0:
                [x.draw() for x in self.children]
                return
            else:
                self.initChildren()

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.positionBufferObject)
        GL.glEnableClientState(GL.GL_VERTEX_ARRAY)
        GL.glVertexPointer(3, GL.GL_FLOAT, 0, None)

        GL.glEnableClientState(GL.GL_INDEX_ARRAY)
        GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)
        GL.glDrawElements(GL.GL_TRIANGLE_STRIP, self.gridSize*self.gridSizep1*2, GL.GL_UNSIGNED_SHORT, c_void_p(0))
            

    def initChildren(self):
        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+1, self.baselat, self.baselon, self.span/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+2, self.baselat, self.baselon+self.span/2., self.span/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+3, self.baselat+self.span/2., self.baselon, self.span/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+4, self.baselat+self.span/2., self.baselon+self.span/2., self.span/2.)
        self.children.append(qt)
