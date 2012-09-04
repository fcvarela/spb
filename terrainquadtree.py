import os
from numpy import array
import numpy as np
import math
import threading
from ctypes import *

from OpenGL import *
from OpenGL.GL import *
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

        self.gridSize = 16
        self.gridSizep1 = self.gridSize + 1

        self.textureBorder = 2
        self.textureSize = 256+2*self.textureBorder

        self.centerHeight = 0
        self.sphere = []
        self.box = []

        # children
        self.children = []

        # sorting
        self.distance = 0.0
        self.weight = 1.0

        # vbo
        self.positionBufferObject = None
        self.texcoordBufferObject = None
        self.indexBufferObject = None
        self.skirtIndexBufferObject = None
        if parent is not None:
            self.texcoordBufferObject = parent.texcoordBufferObject
            self.indexBufferObject = parent.indexBufferObject
            self.skirtIndexBufferObject = parent.skirtIndexBufferObject

        # vertices
        self.vertices = np.arange(self.gridSizep1*self.gridSizep1*3 + 3, dtype='float32')

        # generation programs
        self.generatorShader = None
        self.generatorShaderN = None
        self.generatorShaderC = None
        
        if parent is not None:
            self.generatorShader = parent.generatorShader
            self.generatorShaderN = parent.generatorShaderN
            self.generatorShaderC = parent.generatorShaderC

        # morphing
        self.distance = 0.0
        self.deltadist = 0.0

        # framebuffer
        self.framebuffer = None

        # start init pipeline
        self.ready = False
        self.busy = False
        self.initStep = 0

        # spawn init
        self.processInit()

    def processInit(self):
        if self.busy:
            return

        if self.initStep == 0:
            self.busy = True
            # spawn thread for first step of vertex init (non opengl, so threaded)
            t1 = threading.Thread(target=self.generateVertices(), args=(self, ))
            t1.start()

        if self.initStep == 1:
            self.busy = True
            # finish initializing vertexes. needs gl so can't be threaded
            self.finishVertices()
            self.initStep += 1
            self.busy = False
            
        if self.initStep == 2:
            self.busy = True
            factory.generatorQueue.put((self, ))

        if self.initStep == 3:
            self.busy = False
            self.ready = True

    def generateTextures(self):
        # we'll fetch an extra pixel on both directions
        degreesPerPixel = self.span/float(self.textureSize-self.textureBorder*2)
        
        baselat = self.baselat - degreesPerPixel*float(self.textureBorder)
        latspan = self.span + degreesPerPixel*float(self.textureBorder*2.0)
        baselon = self.baselon - degreesPerPixel*float(self.textureBorder)
        lonspan = self.span + degreesPerPixel*float(self.textureBorder*2.0)

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
        glUniform1f(glGetUniformLocation(self.generatorShader.shader, 'baselat'), baselat*d2r)
        glUniform1f(glGetUniformLocation(self.generatorShader.shader, 'baselon'), baselon*d2r)
        glUniform1f(glGetUniformLocation(self.generatorShader.shader, 'latspan'), latspan*d2r)
        glUniform1f(glGetUniformLocation(self.generatorShader.shader, 'lonspan'), lonspan*d2r)
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
        # load data into localfile and get min and max altitude
        offset = self.textureSize*(self.textureSize/2)+self.textureSize/2
        offset *= 4

        self.framebuffer.unbind()

        # bind the normals framebuffer
        self.framebuffer.bind(self.normalTexture.id)
        self.topoTexture.bind(GL_TEXTURE0)
        self.generatorShaderN.attach()
        glUniform1f(glGetUniformLocation(self.generatorShaderN.shader, 'lonspan'), lonspan)
        glUniform1f(glGetUniformLocation(self.generatorShaderN.shader, 'size'), self.textureSize)
        glUniform1i(glGetUniformLocation(self.generatorShaderN.shader, 'topoTexture'), 0)
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
        glUniform1f(glGetUniformLocation(self.generatorShaderC.shader, 'lonspan'), lonspan)
        glUniform1f(glGetUniformLocation(self.generatorShaderC.shader, 'size'), self.textureSize)
        glUniform1i(glGetUniformLocation(self.generatorShaderC.shader, 'topoTexture'), 0)
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

        self.busy = False
        self.initStep += 1
        self.processInit()

    def generateVertices(self):
        step = self.span / self.gridSize

        # bounding box blues
        bmin = [0.0, 0.0, 0.0]
        bmax = [0.0, 0.0, 0.0]

        for y in range(0, self.gridSizep1):
            lat = self.baselat + (self.gridSize-y)*step
            for x in range(0, self.gridSizep1):
                lon = self.baselon + x*step

                coord = array(factory.geocentricToCarthesian(lat, lon, 1.0))

                coordLow = coord*(1738140.0-32768.0)
                coordHigh = coord*(1738140.0+32768.0)

                # box
                for i in range(0, 3):
                    if coord[i] < bmin[i]:
                        bmin[i] = coord[i]
                    if coord[i] > bmax[i]:
                        bmax[i] = coord[i]

                    if coordLow[i] < bmin[i]:
                        bmin[i] = coordLow[i]
                    if coordLow[i] > bmax[i]:
                        bmax[i] = coordLow[i]

                    if coordHigh[i] < bmin[i]:
                        bmin[i] = coordHigh[i]
                    if coordHigh[i] > bmax[i]:
                        bmax[i] = coordHigh[i]

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

        # finish bounding box
        self.box.extend([bmin[0], bmax[1], bmin[2]]);
        self.box.extend([bmin[0], bmax[1], bmax[2]]);

        self.box.extend([bmax[0], bmax[1], bmin[2]]);
        self.box.extend([bmax[0], bmax[1], bmax[2]]);

        self.box.extend([bmin[0], bmin[1], bmin[2]]);
        self.box.extend([bmin[0], bmin[1], bmax[2]]);

        self.box.extend([bmax[0], bmin[1], bmin[2]]);
        self.box.extend([bmax[0], bmin[1], bmax[2]]);

        # add planet center vertex for skirts
        self.vertices[self.gridSizep1*self.gridSizep1*3 + 0] = self.center[0] * 0.1
        self.vertices[self.gridSizep1*self.gridSizep1*3 + 1] = self.center[1] * 0.1
        self.vertices[self.gridSizep1*self.gridSizep1*3 + 2] = self.center[2] * 0.1

        self.sidelength = math.sqrt(\
            (self.topleft[0] - self.botleft[0])**2+\
            (self.topleft[1] - self.botleft[1])**2+\
            (self.topleft[2] - self.botleft[2])**2)
        self.diagonal = math.sqrt(\
            (self.topleft[0] - self.botright[0])**2+\
            (self.topleft[1] - self.botright[1])**2+\
            (self.topleft[2] - self.botright[2])**2)

        self.vertices = array(self.vertices, dtype='float32')

        box_p = c_double*24
        self.box = box_p(*array(self.box).flatten())

        sphere = list(self.center*1738140.0)
        sphere.append((self.diagonal*1738140.0)/2.0)

        sphere_p = c_double*4
        self.sphere = sphere_p(*array(sphere).flatten())

        # normal
        self.normal = factory.normalize(factory.cross(self.topleft, self.botleft))

        self.initStep += 1
        self.busy = False
        self.processInit()

    def finishVertices(self):
        self.positionBufferObject = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.positionBufferObject)
        glBufferData(GL_ARRAY_BUFFER, self.vertices, GL_STATIC_DRAW)
        self.vertices = None

        # texture coords
        if self.indexBufferObject is None:
            indexes = []
            added_indexes = 0
            for y in range(0, self.gridSize):
                for x in range(0, self.gridSizep1):
                    indexes.append(y * self.gridSizep1 + x)
                    indexes.append((y+1) * self.gridSizep1 + x)

                    added_indexes += 2
                    if added_indexes % self.gridSizep1*2 == 0:
                        # repeat last triangle
                        indexes.append((y+1) * self.gridSizep1 + x)

                        # repeat next triangle twice
                        indexes.append((y+1) * self.gridSizep1)
                        indexes.append((y+1) * self.gridSizep1)

                        # add next triangle
                        indexes.append((y+2) * self.gridSizep1)

            # skirts
            skirt_indexes = []

            # left skirt
            skirt_indexes.append(self.gridSizep1*self.gridSizep1)
            for i in range(0, self.gridSizep1):
                skirt_indexes.append(self.gridSizep1*(self.gridSizep1-i-1))

            # right skirt
            skirt_indexes.append(self.gridSizep1*self.gridSizep1)
            for i in range(0, self.gridSizep1):
                skirt_indexes.append(self.gridSizep1*i + self.gridSize)

            # top skirt
            skirt_indexes.append(self.gridSizep1*self.gridSizep1)
            for i in range(0, self.gridSizep1):
                skirt_indexes.append(i)

            # bottom skirt
            skirt_indexes.append(self.gridSizep1*self.gridSizep1)
            for i in range(0, self.gridSizep1):
                skirt_indexes.append(self.gridSizep1*self.gridSizep1-i-1)

            # store in gl
            indexes = array(indexes, dtype='ushort')
            self.indexBufferObject = glGenBuffers(1)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes, GL_STATIC_DRAW)

            skirt_indexes = array(skirt_indexes, dtype='ushort')
            self.skirtIndexBufferObject = glGenBuffers(1)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.skirtIndexBufferObject)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, skirt_indexes, GL_STATIC_DRAW)

        if self.texcoordBufferObject is None:
            texcoords = []
            for y in range(0, self.gridSizep1):
                for x in range(0, self.gridSizep1):
                    cx = float(x) / float(self.gridSize)
                    cy = float(self.gridSize-y) / float(self.gridSize)

                    # coords are 0,1. map to texelstep,1-texelstep
                    texelstep = 1.0/self.textureSize
                    steprange = 1.0 - texelstep*float(self.textureBorder)*2.0

                    cx = texelstep*self.textureBorder+cx*steprange
                    cy = texelstep*self.textureBorder+cy*steprange

                    texcoords.append([cx, cy])

            texcoords.append([0.5, 0.5])

            # store in gl
            texcoords = array(texcoords, dtype='float32')
            self.texcoordBufferObject = glGenBuffers(1)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.texcoordBufferObject)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, texcoords, GL_STATIC_DRAW)

        self.topoTexture = Texture(self.textureSize)
        self.colorTexture = Texture(self.textureSize)
        self.normalTexture = Texture(self.textureSize)

        if self.generatorShader is None:
            self.generatorShader = ShaderProgram('generator')
            self.generatorShaderN = ShaderProgram('generatornormals')
            self.generatorShaderC = ShaderProgram('generatorcolor')

        self.processInit()

    def analyse(self, weight = 0.0):
        if not self.ready:
            return

        if not factory.sphereInFrustum(self.sphere):
           return

        #normalized_cam = factory.normalize(factory.camera.position)
        #dot = factory.dot(list(normalized_cam), self.normal)
        #if dot < -0.8:
        #    return

        # do we need to draw our children
        d1 = factory.veclen(factory.camera.position - self.center*1738140.0)
        d2 = factory.veclen(factory.camera.position - self.topleft*1738140.0)
        d3 = factory.veclen(factory.camera.position - self.topright*1738140.0)
        d4 = factory.veclen(factory.camera.position - self.botleft*1738140.0)
        d5 = factory.veclen(factory.camera.position - self.botright*1738140.0)

        self.distance = d1
        mindistance = min(d1, min(d2, min(d3, min(d4, d5))))

        far = self.sidelength*1.2*1738140.0
        near = self.sidelength*1.01*1738140.0

        if self.maxlod > 0 and mindistance <= far:
            # are they ready?
            if len(self.children) > 0:
                readycount = 0
                for x in range(0, len(self.children)):
                    if self.children[x].ready == True:
                        readycount += 1
                if readycount == len(self.children):
                    # we can and need to draw our children
                    # two choices. either exclusively or ourself morphed

                    if mindistance >= near:
                        factor = (mindistance - near) / (far-near)
                        if factor > 1.0:
                            factor = 1.0
                        if factor < 0.0:
                            factor = 0.0
                        factor = factor * factor * (3.0 - 2.0 * factor)
                        [x.analyse(weight=factor) for x in self.children]
                    else:
                        [x.analyse() for x in self.children]
                    return
            else:
                self.initChildren()

        self.weight = weight
        factory.planet.scenegraph.append(self)

    def draw(self, skirts=False):
        glUniform1f(glGetUniformLocation(factory.planet.shader.shader, 'weight'), self.weight)
        glUniform1i(glGetUniformLocation(factory.planet.shader.shader, 'index'), self.index)
        glUniform1f(glGetUniformLocation(factory.planet.shader.shader, 'texturesize'), self.textureSize-self.textureBorder)

        glEnableClientState(GL_VERTEX_ARRAY)
        glBindBuffer(GL_ARRAY_BUFFER, self.positionBufferObject)
        glVertexPointer(3, GL_FLOAT, 0, None)

        glEnableClientState(GL_INDEX_ARRAY)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.indexBufferObject)

        glEnableClientState(GL_TEXTURE_COORD_ARRAY)
        glBindBuffer(GL_ARRAY_BUFFER, self.texcoordBufferObject)
        glTexCoordPointer(2, GL_FLOAT, 0, None)
            
        self.normalTexture.bind(GL_TEXTURE0)
        self.colorTexture.bind(GL_TEXTURE1)
        self.topoTexture.bind(GL_TEXTURE2)
        try:
            # we need to attach the parent heightmap (for now)
            self.parent.normalTexture.bind(GL_TEXTURE3)
            self.parent.colorTexture.bind(GL_TEXTURE4)
            self.parent.topoTexture.bind(GL_TEXTURE5)
        except:
            pass

        if not skirts:
            # vertices
            indexcount = (self.gridSizep1*self.gridSize*2)+(self.gridSize*4)
            glDrawElements(GL_TRIANGLE_STRIP, indexcount, GL_UNSIGNED_SHORT, c_void_p(0))
        else:
            skirtcount = (self.gridSizep1 + 2) * 4
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.skirtIndexBufferObject)
            glDrawElements(GL_TRIANGLE_FAN, skirtcount, GL_UNSIGNED_SHORT, c_void_p(0))

        factory.drawnNodes += 1
        
        # draw bounding sphere
        return
        glPushMatrix()
        factory.planet.shader.dettach()
        #glTranslatef(factory.camera.position[0], factory.camera.position[1], factory.camera.position[2])
        glTranslatef(-self.sphere[0], -self.sphere[1], -self.sphere[2])
        glutSolidSphere(self.sidelength*1738140.0/2.0, 20, 20);
        factory.planet.shader.attach()
        glPopMatrix()

        factory.drawnNodes += 1

    def initChildren(self):
        qt1 = TerrainQuadtree(self, self.maxlod-1, 1, self.baselat, self.baselon, self.span/2.)
        qt2 = TerrainQuadtree(self, self.maxlod-1, 2, self.baselat, self.baselon+self.span/2., self.span/2.)
        qt3 = TerrainQuadtree(self, self.maxlod-1, 3, self.baselat+self.span/2., self.baselon, self.span/2.)
        qt4 = TerrainQuadtree(self, self.maxlod-1, 4, self.baselat+self.span/2., self.baselon+self.span/2., self.span/2.)

        self.children = [qt1, qt2, qt3, qt4]