from numpy import array
import numpy as np

from OpenGL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

class TerrainQuadtree:
    def __init__(self, parent, maxlod, face, index, center, dx, dy):
        self.parent = parent
        self.maxlod = maxlod
        self.face = face
        self.index = index
        self.center = array(center)
        self.dx = array(dx)
        self.dy = array(dy)

        self.gridSize = 32
        self.gridSizep1 = 33

        self.vertices = []
        self.indexes = []

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

        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)

    def draw(self):
        self.vbo.bind()
        glEnableClientState(GL_VERTEX_ARRAY)
        glVertexPointer(3, GL_FLOAT, 0, None)
        glDrawArrays(GL_TRIANGLE_FAN, 0)
        glDisableClientState(GL_VERTEX_ARRAY)
        self.vbo.unbind()
