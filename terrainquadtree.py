from numpy import array
import numpy as np

from OpenGL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import factory

class TerrainQuadtree:
    def __init__(self, parent, maxlod, face, index, center, dx, dy):
        self.parent = parent
        self.maxlod = maxlod
        self.face = face
        self.index = index
        self.center = array(center)
        self.dx = array(dx)
        self.dy = array(dy)

        self.gridSize = 8
        self.gridSizep1 = self.gridSize + 1

        self.vertices = []
        self.indexes = []
        self.children = []

        self.positionBufferObject = None
        self.indexBufferObject = None

        self.generateVertices()

    def generateVertices(self):
        for u in range(0, self.gridSizep1):
            for v in range(0, self.gridSizep1):
                coord = array(self.center +\
                    (self.dx/self.gridSize) * (v-self.gridSize/2.) +\
                    (self.dy/self.gridSize) * (self.gridSize/2. - u))

                coord /= np.linalg.norm(coord)
                self.vertices.append(coord)

                if u == 0 and v == 0:
                    self.topleft = coord
                if u == 0 and v == self.gridSize:
                    self.topright = coord
                if u == self.gridSize and v == 0:
                    self.botleft = coord
                if u == self.gridSize and v == self.gridSize:
                    self.botright = coord

        self.sidelength = np.linalg.norm(self.topleft - self.topright)

        for i in range(0, self.gridSize):
            for j in range(0, self.gridSizep1):
                self.indexes.append(i * self.gridSizep1 + j)
                self.indexes.append((i+1) * self.gridSizep1 + j)

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

        print "TerrainQuadtree inited with lod %d" % self.maxlod

    def draw(self):
        # do we need to draw our children
        d1 = np.linalg.norm(factory.camera.position - self.center)
        d2 = np.linalg.norm(factory.camera.position - self.topleft)
        d3 = np.linalg.norm(factory.camera.position - self.topright)
        d4 = np.linalg.norm(factory.camera.position - self.botleft)
        d5 = np.linalg.norm(factory.camera.position - self.botright)

        distance = min(d1, min(d2, min(d3, min(d4, d5))))

        #print "Dist: %f, len: %f" % (distance, self.sidelength)
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
        qt = TerrainQuadtree(parent=self, maxlod=self.maxlod-1, face=self.face, index=1, center=self.center - self.dx/4. + self.dy/4., dx=self.dx/2., dy=self.dy/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(parent=self, maxlod=self.maxlod-1, face=self.face, index=1, center=self.center + self.dx/4. + self.dy/4., dx=self.dx/2., dy=self.dy/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(parent=self, maxlod=self.maxlod-1, face=self.face, index=1, center=self.center - self.dx/4. - self.dy/4., dx=self.dx/2., dy=self.dy/2.)
        self.children.append(qt)

        qt = TerrainQuadtree(parent=self, maxlod=self.maxlod-1, face=self.face, index=1, center=self.center + self.dx/4. - self.dy/4., dx=self.dx/2., dy=self.dy/2.)
        self.children.append(qt)
