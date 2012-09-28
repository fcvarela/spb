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
        
        # top
        center = array([.0, .5, .0])
        dx = array([1., 0., 0.])
        dy = array([0., 0., -1.])
        qt1 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=1, center=center, dx=dx, dy=dy)

        # left
        center = array([-.5, 0., 0.])
        dx = array([0., 0., 1.])
        dy = array([0., 1., 0.])
        qt2 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=2, center=center, dx=dx, dy=dy)
        
        # front
        center = array([.0, .0, .5])
        dx = array([1., .0, .0])
        dy = array([.0, 1., .0])
        qt3 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=3, center=center, dx=dx, dy=dy)

        # right
        center = array([.5, .0, .0])
        dx = array([0., .0, -1.])
        dy = array([.0, 1., .0])
        qt4 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=4, center=center, dx=dx, dy=dy)

        # back
        center = array([.0, .0, -.5])
        dx = array([-1., .0, 0.])
        dy = array([.0, 1., .0])
        qt5 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=5, center=center, dx=dx, dy=dy)

        # bottom
        center = array([.0, -.5, .0])
        dx = array([1., .0, 0.])
        dy = array([.0, 0., 1.])
        qt6 = TerrainQuadtree(parent=None, maxlod=self.maxlod, face=0, index=6, center=center, dx=dx, dy=dy)

        # add them
        self.quadtrees.extend([qt1, qt2, qt3, qt4, qt5, qt6])

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
        glUniform1f(glGetUniformLocation(localshader.shader, 'far'), factory.far)
            
        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = factory.normalize(factory.sun.position)
        
        glUniform3f(glGetUniformLocation(localshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        glUniform3f(glGetUniformLocation(localshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        if framenumber % 2 == 0:
            # push drawables to local scenegraph
            self.scenegraph = []
            [x.analyse() for x in self.quadtrees]

        # sort to draw near to far
        self.scenegraph.sort(key=attrgetter('distance'))

        # draw skirts
        [x.draw(skirts = True) for x in self.scenegraph]
        [x.draw(skirts = False) for x in self.scenegraph]

        localshader.dettach()

    def drawAtmosphere(self):
        self.atmosphereshader.attach()

        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = factory.normalize(factory.sun.position)
        glUniform3f(glGetUniformLocation(self.atmosphereshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        glUniform3f(glGetUniformLocation(self.atmosphereshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        glFrontFace(GL_CW)
        glutSolidSphere(self.atmosphere_radius, 100, 100)
        glFrontFace(GL_CCW)
        self.atmosphereshader.dettach()
        