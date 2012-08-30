from OpenGL import *
from OpenGL import *
import math
import numpy as np
from operator import attrgetter

from ConfigParser import SafeConfigParser

from terrainquadtree import *
from shaderprogram import *
import factory

class Planet:
    def __init__(self, conffile):
        self.parser = SafeConfigParser()
        self.parser.read(conffile)

        self.radius = float(self.parser.get('planet', 'circumference')) / (2.0 * math.pi)
        self.maxlod = int(self.parser.get('terrain', 'maxlod'))

        self.atmosphere_radius = 1.025
        self.quadtrees = []
        self.scenegraph = []

        self.shader = ShaderProgram('planet')
        self.atmosphereshader = ShaderProgram('planetatmosphere')

        baselat = 0.
        degreespan = 90.
        for i in range(0,8):
            baselon = -180 + (i%4 * 90.)
            if i > 3:
                baselat = -90.
            
            qt = TerrainQuadtree(parent=None, maxlod=self.maxlod, index=i+1, baselat=baselat, baselon=baselon, span=degreespan)
            self.quadtrees.append(qt)

    def draw(self, shader, framenumber):
        localshader = None
        if shader:
            localshader = self.shader
            glDepthMask(GL_FALSE)
            self.drawAtmosphere()
            glDepthMask(GL_TRUE)
        else:
            localshader = self.shadowshader

        localshader.attach()

        # tile textures
        glUniform1i(glGetUniformLocation(localshader.shader, 'normalTexture'), 0)
        glUniform1i(glGetUniformLocation(localshader.shader, 'colorTexture'), 1)
        glUniform1i(glGetUniformLocation(localshader.shader, 'topoTexture'), 2)
        
        # tile parent textures
        glUniform1i(glGetUniformLocation(localshader.shader, 'pnormalTexture'), 3)
        glUniform1i(glGetUniformLocation(localshader.shader, 'pcolorTexture'), 4)
        glUniform1i(glGetUniformLocation(localshader.shader, 'ptopoTexture'), 5)

        # near far
        glUniform1f(glGetUniformLocation(localshader.shader, 'near'), factory.near)
        glUniform1f(glGetUniformLocation(localshader.shader, 'far'), factory.far)
            
        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = factory.normalize(factory.sun.position)
        
        glUniform3f(glGetUniformLocation(localshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        glUniform3f(glGetUniformLocation(localshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        if framenumber % 4 == 0:
            # push drawables to local scenegraph
            for x in self.quadtrees:
                x.analyse()

        # sort to draw near to far
        self.scenegraph.sort(key=attrgetter('distance'))
        for x in self.scenegraph:
            x.draw()

        localshader.dettach()

    def drawAtmosphere(self):
        self.atmosphereshader.attach()

        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = lightPos = factory.normalize(factory.sun.position)
        glUniform3f(glGetUniformLocation(self.atmosphereshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        glUniform3f(glGetUniformLocation(self.atmosphereshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        glFrontFace(GL_CW)
        glutSolidSphere(self.atmosphere_radius, 100, 100)
        glFrontFace(GL_CCW)
        self.atmosphereshader.dettach()
        