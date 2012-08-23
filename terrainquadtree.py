import os
from numpy import array
import numpy as np
import math

from OpenGL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import factory
from texture import *
from shaderprogram import *

class TerrainQuadtree:
    def __init__(self, parent, maxlod, index, baselat, baselon, span):
        self.parent = parent
        self.maxlod = maxlod
        self.baselat = baselat
        self.baselon = baselon
        self.index = index
        self.span = span

        self.gridSize = 16
        self.gridSizep1 = self.gridSize + 1
        self.textureSize = 256

        self.vertices = np.arange(self.gridSizep1*self.gridSizep1*3, dtype='float32')
        self.texcoords = []
        self.indexes = []
        self.children = []

        self.positionBufferObject = None
        self.texcoordBufferObject = None
        self.indexBufferObject = None

        # thread this later
        self.generateVertices()

        self.topoTexture = Texture(self.textureSize)
        self.specularTexture = Texture(self.textureSize)
        self.normalTexture = Texture(self.textureSize)

        self.ready = False
        self.generatorShader = ShaderProgram('generator')
        self.generatorShaderN = ShaderProgram('generatornormals')

        factory.generatorQueue.put((self, ))

    def generateTextures(self):
        degreesPerVertex = 1./self.textureSize
        
        baselat = self.baselat
        latspan = self.span+(self.span*degreesPerVertex)
        baselon = self.baselon
        lonspan = self.span+(self.span*degreesPerVertex)

        glDisable(GL_DEPTH_TEST)
        glViewport(0, 0, self.textureSize, self.textureSize)

        glMatrixMode(GL_PROJECTION)
        glPolygonMode(GL_FRONT, GL_FILL)
        glLoadIdentity()
        glOrtho(0, self.textureSize, 0, self.textureSize, 0, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        glTranslated(0, 0, -1)

        # use the generator shader
        self.generatorShader.attach()
        d2r = math.pi/180.0
        glUniform1f(GL.glGetUniformLocation(self.generatorShader.shader, 'baselat'), baselat*d2r)
        glUniform1f(GL.glGetUniformLocation(self.generatorShader.shader, 'baselon'), baselon*d2r)
        glUniform1f(GL.glGetUniformLocation(self.generatorShader.shader, 'latspan'), latspan*d2r)
        glUniform1f(GL.glGetUniformLocation(self.generatorShader.shader, 'lonspan'), lonspan*d2r)
        glBegin(GL_QUADS)
        glTexCoord2f(0., 0.)
        glVertex3f(0., 0., 0.)
        glTexCoord2f(1., 0.)
        glVertex3f(self.textureSize, 0., 0.)
        glTexCoord2f(1., 1.)
        glVertex3f(self.textureSize, self.textureSize, 0.)
        glTexCoord2f(0., 1.)
        glVertex3f(0., self.textureSize, 0.)
        glEnd()
        self.topoTexture.bind(GL_TEXTURE0)
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, self.textureSize, self.textureSize)
        self.topoTexture.unbind()
        self.generatorShader.dettach()

        self.generatorShaderN = ShaderProgram('generatornormals');
        self.generatorShaderN.attach()
        glUniform1f(GL.glGetUniformLocation(self.generatorShaderN.shader, 'lonspan'), lonspan)
        glUniform1f(GL.glGetUniformLocation(self.generatorShaderN.shader, 'size'), self.textureSize)
        glUniform1i(GL.glGetUniformLocation(self.generatorShaderN.shader, 'topoTexture'), 0)
        glBegin(GL_QUADS)
        glTexCoord2f(0., 0.)
        glVertex3f(0., 0., 0.)
        glTexCoord2f(1., 0.)
        glVertex3f(self.textureSize, 0., 0.)
        glTexCoord2f(1., 1.)
        glVertex3f(self.textureSize, self.textureSize, 0.)
        glTexCoord2f(0., 1.)
        glVertex3f(0., self.textureSize, 0.)
        glEnd()
        self.normalTexture.bind(GL_TEXTURE1)
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, self.textureSize, self.textureSize)
        self.normalTexture.unbind()
        self.generatorShaderN.dettach()

        glEnable(GL_DEPTH_TEST)
        self.ready = True

    def generateVertices(self):
        step = self.span / self.gridSize

        for y in range(0, self.gridSizep1):
            lat = self.baselat + (self.gridSize-y)*step
            for x in range(0, self.gridSizep1):
                lon = self.baselon + x*step

                coord = array(factory.geocentricToCarthesian(lat, lon, 1.0))
                #coord /= np.linalg.norm(coord)
                self.vertices[self.gridSizep1*y*3 + x*3 + 0] = coord[0]
                self.vertices[self.gridSizep1*y*3 + x*3 + 1] = coord[1]
                self.vertices[self.gridSizep1*y*3 + x*3 + 2] = coord[2]

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
        self.vertices = array(self.vertices, dtype='float32')
        self.positionBufferObject = GL.glGenBuffers(1)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.positionBufferObject)
        GL.glBufferData(GL.GL_ARRAY_BUFFER, self.vertices, GL.GL_STATIC_DRAW)
        self.vertices = None

        # texture coords
        if self.indexBufferObject is None:
            added_indexes = 0
            for y in range(0, self.gridSize):
                for x in range(0, self.gridSizep1):
                    self.indexes.append(y * self.gridSizep1 + x)
                    self.indexes.append((y+1) * self.gridSizep1 + x)

                    added_indexes += 2
                    if added_indexes % self.gridSizep1*2 == 0:
                        # repeat last triangle
                        self.indexes.append((y+1) * self.gridSizep1 + x)

                        # repeat next triangle twice
                        self.indexes.append((y+1) * self.gridSizep1)
                        self.indexes.append((y+1) * self.gridSizep1)

                        # add next triangle
                        self.indexes.append((y+2) * self.gridSizep1 )

            # store in gl
            self.indexes = array(self.indexes, dtype='ushort')
            self.indexBufferObject = GL.glGenBuffers(1)
            GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)
            GL.glBufferData(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexes, GL.GL_STATIC_DRAW)

            # release local copy
            self.indexes = None

        if self.texcoordBufferObject is None:
            step = self.textureSize/self.gridSizep1
            for y in range(0, self.gridSizep1):
                for x in range(0, self.gridSizep1):
                    cx = float(x) / float(self.gridSize)
                    cy = float(self.gridSize-y) / float(self.gridSize)
                    self.texcoords.append([cx, cy])

            # store in gl
            self.texcoords = array(self.texcoords, dtype='float32')
            self.texcoordBufferObject = GL.glGenBuffers(1)
            GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.texcoordBufferObject)
            GL.glBufferData(GL.GL_ELEMENT_ARRAY_BUFFER, self.texcoords, GL.GL_STATIC_DRAW)

            # release local copy
            self.texcoords = None

    def draw(self, textures):
        if not self.ready:
            return
        # do we need to draw our children
        d1 = np.linalg.norm(factory.camera.position - self.center*1738140.0)
        d2 = np.linalg.norm(factory.camera.position - self.topleft*1738140.0)
        d3 = np.linalg.norm(factory.camera.position - self.topright*1738140.0)
        d4 = np.linalg.norm(factory.camera.position - self.botleft*1738140.0)
        d5 = np.linalg.norm(factory.camera.position - self.botright*1738140.0)

        distance = min(d1, min(d2, min(d3, min(d4, d5))))
        if self.maxlod > 0 and distance < self.sidelength*1.2*1738140.0:
            # are they ready?
            if len(self.children) > 0:
                readycount = 0
                for x in range(0, len(self.children)):
                    if self.children[x].ready == True:
                        readycount += 1
                if readycount == len(self.children):
                    [x.draw(textures) for x in self.children]
                    return
            else:
                self.initChildren()

        GL.glEnableClientState(GL.GL_VERTEX_ARRAY)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.positionBufferObject)
        GL.glVertexPointer(3, GL.GL_FLOAT, 0, None)

        GL.glEnableClientState(GL.GL_INDEX_ARRAY)
        GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)

        GL.glEnableClientState(GL.GL_TEXTURE_COORD_ARRAY)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.texcoordBufferObject)
        GL.glTexCoordPointer(2, GL.GL_FLOAT, 0, None)
            
        self.normalTexture.bind(GL.GL_TEXTURE0)
        self.specularTexture.bind(GL.GL_TEXTURE3)
        self.topoTexture.bind(GL.GL_TEXTURE2)

        indexcount = (self.gridSizep1*self.gridSize*2)+(self.gridSize*4)
        GL.glDrawElements(GL.GL_TRIANGLE_STRIP, indexcount, GL.GL_UNSIGNED_SHORT, None)

    def initChildren(self):
        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+1, self.baselat, self.baselon, self.span/2.)

        qt.texcoordBufferObject = self.texcoordBufferObject
        qt.indexBufferObject = self.indexBufferObject
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+2, self.baselat, self.baselon+self.span/2., self.span/2.)
        qt.texcoordBufferObject = self.texcoordBufferObject
        qt.indexBufferObject = self.indexBufferObject
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+3, self.baselat+self.span/2., self.baselon, self.span/2.)
        qt.texcoordBufferObject = self.texcoordBufferObject
        qt.indexBufferObject = self.indexBufferObject
        self.children.append(qt)

        qt = TerrainQuadtree(self, self.maxlod-1, self.index*10+4, self.baselat+self.span/2., self.baselon+self.span/2., self.span/2.)
        qt.texcoordBufferObject = self.texcoordBufferObject
        qt.indexBufferObject = self.indexBufferObject
        self.children.append(qt)
