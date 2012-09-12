import os
from numpy import array
import numpy as np
import math
import threading
import time
from ctypes import *

from OpenGL import *
from OpenGL.GL import *
from OpenGL.GLUT import *

import factory
from texture import *
from shaderprogram import *
from framebuffer import *

def threadDebug(fn):
    from functools import wraps
    @wraps(fn)
    def wrapper(*args, **kw):
        #print "%r - %r started" % (fn, threading.currentThread())
        start = time.clock()
        fn(*args, **kw)
        #print "%s - %r ended" % (fn, threading.currentThread())
        end = time.clock()
        print "%r took %f seconds" % (fn, (end-start))
        return None
    return wrapper

class TerrainQuadtree:
    def __init__(self, parent, maxlod, face, index, center, dx, dy):
        self.parent = parent
        self.maxlod = maxlod
        self.face = face
        self.index = index
        self.center = center
        self.dx = dx
        self.dy = dy

        self.gridSize = 32
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
        self.nextStep = {'function': self.generateVertices, 'threaded': True}

        # spawn init
        factory.generatorQueue.put((self, ))

    def generateTextures(self):
        if self.framebuffer is None:
            self.framebuffer = Framebuffer()

        # bind framebuffer to our texture
        self.framebuffer.bind(self.topoTexture.id)
        self.positionTexture.bind(GL_TEXTURE0)
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
        glUniform1i(glGetUniformLocation(self.generatorShader.shader, 'positionTexture'), 0)
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
        self.framebuffer.unbind()

        # bind the normals framebuffer
        self.framebuffer.bind(self.normalTexture.id)
        self.topoTexture.bind(GL_TEXTURE1)
        self.generatorShaderN.attach()
        glUniform1f(glGetUniformLocation(self.generatorShaderN.shader, 'size'), self.textureSize)
        glUniform1i(glGetUniformLocation(self.generatorShaderN.shader, 'topoTexture'), 1)
        glUniform1i(glGetUniformLocation(self.generatorShaderN.shader, 'positionTexture'), 0)
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
        self.generatorShaderC.attach()
        glUniform1i(glGetUniformLocation(self.generatorShaderC.shader, 'topoTexture'), 1)
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
        # vertices
        self.vertices = np.arange(self.gridSizep1*self.gridSizep1*3 + 3, dtype='float32')
        self.positionTextureContent = np.arange((self.gridSizep1+2)*(self.gridSizep1+2)*3, dtype='float32')

        # bounding box blues
        bmin = [0.0, 0.0, 0.0]
        bmax = [0.0, 0.0, 0.0]

        # generate the position buffer for position texture
        gsize = self.gridSizep1+2
        dxovergridsize = self.dx/self.gridSize
        dyovergridsize = self.dy/self.gridSize
        gridsizeover2 = self.gridSize/2.
        
        start = time.clock()
        for u in range(0, gsize):
            for v in range(0, gsize):
                coord = array(self.center +\
                    dxovergridsize * ((v-1)-gridsizeover2) +\
                    dyovergridsize * (gridsizeover2 - (u-1)))
                
                coord = coord/factory.veclen(coord)
                upos = (gsize-1-u)*(gsize)*3
                vpos = v*3

                self.positionTextureContent[upos + vpos + 0] = coord[0]
                self.positionTextureContent[upos + vpos + 1] = coord[1]
                self.positionTextureContent[upos + vpos + 2] = coord[2]
        
        for u in range(0, self.gridSizep1):
            for v in range(0, self.gridSizep1):
                coord = array(self.center +\
                    dxovergridsize * (v-gridsizeover2) +\
                    dyovergridsize * (gridsizeover2 - u))

                coord = coord/factory.veclen(coord)

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


                upos = self.gridSizep1*u*3
                vpos = v*3
                self.vertices[upos + vpos + 0] = coord[0]
                self.vertices[upos + vpos + 1] = coord[1]
                self.vertices[upos + vpos + 2] = coord[2]

                if u == 0 and v == 0:
                    self.topleft = coord
                if u == 0 and v == self.gridSize:
                    self.topright = coord
                if u == self.gridSize and v == 0:
                    self.botleft = coord
                if u == self.gridSize and v == self.gridSize:
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

        box_p = c_double*24
        self.box = box_p(*array(self.box).flatten())

        sphere = list(self.center*1738140.0)
        sphere.append((self.diagonal*1738140.0/2.0))

        sphere_p = c_double*4
        self.sphere = sphere_p(*array(sphere).flatten())

        # normal
        self.normal = array(factory.cross(self.topleft, self.botleft))
        self.normal = self.normal / factory.veclen(self.normal)

        # init
        self.nextStep = {'function': self.finishVertices, 'threaded': False}
        factory.generatorQueue.put((self, ))

        print "ELAPSED: %f" % (time.clock() - start)

    def finishVertices(self):
        # copy vertex data to position texture
        self.positionTexture = Texture(self.textureSize, False)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, self.gridSizep1+2, self.gridSizep1+2, 0, GL_RGB, GL_FLOAT, self.positionTextureContent)

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
                    texelstep = 1.0/23.0
                    steprange = 1.0 - texelstep*2.0

                    cx = texelstep + cx*steprange
                    cy = texelstep + cy*steprange

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

        self.nextStep = {'function': self.generateTextures, 'threaded': False}

    def analyse(self, weight = 0.0):
        if not self.ready:
            return

        #if not factory.sphereInFrustum(self.sphere):
        #   return

        # do we need to draw our children
        d1 = factory.veclen(factory.camera.position - self.center*1738140.0)
        d2 = factory.veclen(factory.camera.position - self.topleft*1738140.0)
        d3 = factory.veclen(factory.camera.position - self.topright*1738140.0)
        d4 = factory.veclen(factory.camera.position - self.botleft*1738140.0)
        d5 = factory.veclen(factory.camera.position - self.botright*1738140.0)
        mindistance = min(d1, min(d2, min(d3, min(d4, d5))))
        self.distance = mindistance

        far = self.sphere[3]*2.0*1.15
        near = self.sphere[3]*2.0*1.01

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
        glTranslatef(self.sphere[0], self.sphere[1], self.sphere[2])
        glutSolidSphere(self.sphere[3], 20, 20);
        factory.planet.shader.attach()
        glPopMatrix()

        factory.drawnNodes += 1

    def initChildren(self):
        dx = self.dx / 2.0
        dy = self.dy / 2.0

        # index 1
        center = self.center - self.dx/4.0 + self.dy/4.0
        qt1 = TerrainQuadtree(self, self.maxlod-1, self.face, 1, center, dx, dy)

        # index 2
        center = self.center + self.dx/4.0 + self.dy/4.0
        qt2 = TerrainQuadtree(self, self.maxlod-1, self.face, 2, center, dx, dy)

        # index 3
        center = self.center - self.dx/4.0 - self.dy/4.0
        qt3 = TerrainQuadtree(self, self.maxlod-1, self.face, 3, center, dx, dy)
        
        # index 4
        center = self.center + self.dx/4.0 - self.dy/4.0
        qt4 = TerrainQuadtree(self, self.maxlod-1, self.face, 4, center, dx, dy)
        
        self.children = [qt1, qt2, qt3, qt4]