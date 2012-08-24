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
from framebuffer import *

class TerrainQuadtree:
    def __init__(self, parent, maxlod, index, baselat, baselon, span):
        self.parent = parent
        self.maxlod = maxlod
        self.baselat = baselat
        self.baselon = baselon
        self.index = index
        self.span = span

        self.gridSize = 8
        self.gridSizep1 = self.gridSize + 1
        self.textureSize = 260

        self.vertices = np.arange(self.gridSizep1*self.gridSizep1*3, dtype='float32')
        self.texcoords = []
        self.indexes = []
        self.children = []

        # vbo
        self.positionBufferObject = None
        self.texcoordBufferObject = None
        self.indexBufferObject = None

        # morphing
        self.distance = 0.0
        self.deltadist = 0.0

        # framebuffer
        self.framebuffer = None

        # thread this later
        self.generateVertices()

        self.topoTexture = Texture(self.textureSize)
        self.colorTexture = Texture(self.textureSize)
        self.normalTexture = Texture(self.textureSize)

        self.ready = False
        self.generatorShader = ShaderProgram('generator')
        self.generatorShaderN = ShaderProgram('generatornormals')
        self.generatorShaderC = ShaderProgram('generatorcolor')

        factory.generatorQueue.put((self, ))

    def generateTextures(self):
        # we'll fetch an extra pixel on both directions
        degreesPerVertex = self.span/(self.textureSize-4.0)
        
        baselat = self.baselat - degreesPerVertex*2.0
        latspan = self.span + degreesPerVertex*4.0
        baselon = self.baselon - degreesPerVertex*2.0
        lonspan = self.span+ degreesPerVertex*4.0

        if self.framebuffer is None:
            self.framebuffer = Framebuffer()

        # bind framebuffer to our texture
        self.framebuffer.bind(self.topoTexture.id)

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
        self.generatorShader.dettach()

        # unbind framebuffer
        self.framebuffer.unbind()

        # bind the normals framebuffer
        self.framebuffer.bind(self.normalTexture.id)
        self.topoTexture.bind(GL_TEXTURE0)
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
        self.generatorShaderN.dettach()

        self.framebuffer.unbind()

        # bind the color framebuffer
        self.framebuffer.bind(self.colorTexture.id)
        self.topoTexture.bind(GL_TEXTURE0)
        self.generatorShaderC.attach()
        glUniform1f(GL.glGetUniformLocation(self.generatorShaderC.shader, 'lonspan'), lonspan)
        glUniform1f(GL.glGetUniformLocation(self.generatorShaderC.shader, 'size'), self.textureSize)
        glUniform1i(GL.glGetUniformLocation(self.generatorShaderC.shader, 'topoTexture'), 0)
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
        self.generatorShaderC.dettach()

        self.framebuffer.unbind()

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
            for y in range(0, self.gridSizep1):
                for x in range(0, self.gridSizep1):
                    cx = float(x) / float(self.gridSize)
                    cy = float(self.gridSize-y) / float(self.gridSize)

                    # coords are 0,1. map to texelstep,1-texelstep
                    texelstep = 1.0/self.textureSize
                    steprange = 1.0 - texelstep*4.0

                    self.texcoords.append([texelstep*2.0+cx*steprange, texelstep*2.0+cy*steprange])

            # store in gl
            self.texcoords = array(self.texcoords, dtype='float32')
            self.texcoordBufferObject = GL.glGenBuffers(1)
            GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.texcoordBufferObject)
            GL.glBufferData(GL.GL_ELEMENT_ARRAY_BUFFER, self.texcoords, GL.GL_STATIC_DRAW)

            # release local copy
            self.texcoords = None

    def draw(self, textures, weight = 0.0):
        if not self.ready:
            return
        # do we need to draw our children
        d1 = np.linalg.norm(factory.camera.position - self.center*1738140.0)
        d2 = np.linalg.norm(factory.camera.position - self.topleft*1738140.0)
        d3 = np.linalg.norm(factory.camera.position - self.topright*1738140.0)
        d4 = np.linalg.norm(factory.camera.position - self.botleft*1738140.0)
        d5 = np.linalg.norm(factory.camera.position - self.botright*1738140.0)

        self.distance = min(d1, min(d2, min(d3, min(d4, d5))))

        if self.maxlod > 0 and self.distance < self.sidelength*1.4*1738140.0:
            # are they ready?
            if len(self.children) > 0:
                readycount = 0
                for x in range(0, len(self.children)):
                    if self.children[x].ready == True:
                        readycount += 1
                if readycount == len(self.children):
                    # we can and need to draw our children
                    # two choices. either exclusively or ourself morphed

                    if self.distance > self.sidelength*1.2*1738140.0:
                        # morphed
                        # if we're up until 0.8, draw our vertices morphed with the childs
                        # proportion is 0.8 to 1.2  equals 0 to 1
                        # self weight is 1-factor
                        factor = (self.distance - self.sidelength*1.2*1738140.0) / (self.sidelength*1.4*1738140.0-self.sidelength*1.2*1738140.0)
                        if factor > 1.0:
                            factor = 1.0
                        factor = 1.0 - (factor * factor * (3.0 - 2.0 * factor))
                        [x.draw(textures, factor) for x in self.children]
                    else:
                        [x.draw(textures) for x in self.children]
                    return
            else:
                self.initChildren()

        if weight > 0.0:
            # we need to attach the parent heightmap (for now)
            self.parent.normalTexture.bind(GL_TEXTURE3)
            self.parent.colorTexture.bind(GL_TEXTURE4)
            self.parent.topoTexture.bind(GL_TEXTURE5)
            glUniform1f(glGetUniformLocation(factory.planet.shader.shader, 'weight'), weight)

        GL.glEnableClientState(GL.GL_VERTEX_ARRAY)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.positionBufferObject)
        GL.glVertexPointer(3, GL.GL_FLOAT, 0, None)

        GL.glEnableClientState(GL.GL_INDEX_ARRAY)
        GL.glBindBuffer(GL.GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)

        GL.glEnableClientState(GL.GL_TEXTURE_COORD_ARRAY)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.texcoordBufferObject)
        GL.glTexCoordPointer(2, GL.GL_FLOAT, 0, None)
            
        self.normalTexture.bind(GL.GL_TEXTURE0)
        self.colorTexture.bind(GL.GL_TEXTURE1)
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
