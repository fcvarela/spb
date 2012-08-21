from OpenGL import *
import math
import numpy as np

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

        self.atmosphere_radius = 1.025*self.radius
        self.quadtrees = []

        self.shader = ShaderProgram('planet')
        self.atmosphereshader = ShaderProgram('planetatmosphere')

        baselat = 0.
        degreespan = 90.
        for i in range(0,8):
            baselon = -180 + (i%4 * 90.)
            if i > 3:
                baselat = -90.
            
            qt = TerrainQuadtree(parent=None, maxlod=self.maxlod, index=i+1, baselat=baselat, baselon=baselon, span=degreespan, seed=self.generator_seed())
            self.quadtrees.append(qt)

    def draw(self, shader):
        localshader = None
        if shader:
            localshader = self.shader
            self.drawAtmosphere()
        else:
            localshader = self.shadowshader

        localshader.attach()

        GL.glUniform1i(GL.glGetUniformLocation(localshader.shader, 'normalTexture'), 0)
        GL.glUniform1i(GL.glGetUniformLocation(localshader.shader, 'topoTexture'), 2)
        GL.glUniform1i(GL.glGetUniformLocation(localshader.shader, 'specularTexture'), 3)
        GL.glUniform1i(GL.glGetUniformLocation(localshader.shader, 'justTopoAndNormals'), not int(shader))
            
        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = np.array(factory.sun.position)/np.linalg.norm(factory.sun.position)
        GL.glUniform3f(GL.glGetUniformLocation(localshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        GL.glUniform3f(GL.glGetUniformLocation(localshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        [x.draw(shader) for x in self.quadtrees]

        localshader.dettach()

    def drawAtmosphere(self):
        self.atmosphereshader.attach()

        cameraPos = np.array(factory.camera.position)/self.radius
        lightPos = np.array(factory.sun.position)/np.linalg.norm(factory.sun.position)
        GL.glUniform3f(GL.glGetUniformLocation(self.atmosphereshader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
        GL.glUniform3f(GL.glGetUniformLocation(self.atmosphereshader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])

        GL.glFrontFace(GL.GL_CW)
        glutSolidSphere(self.atmosphere_radius, 100, 100)
        GL.glFrontFace(GL.GL_CCW)
        self.atmosphereshader.dettach()

    def generator_seed(self):
        # assemble all options into cli str
        options = {}
        for item in self.parser.items('planet'):
            options[item[0]] = item[1]
        for item in self.parser.items('terrain'):
            options[item[0]] = item[1]
        for item in self.parser.items('cache'):
            options[item[0]] = item[1]
        return options