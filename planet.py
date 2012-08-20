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
        self.shadowshader = ShaderProgram('planetshadow')

        baselat = 0.
        degreespan = 90.
        for i in range(0,8):
            baselon = -180 + (i%4 * 90.)
            if i > 3:
                baselat = -90.
            
            qt = TerrainQuadtree(parent=None, maxlod=self.maxlod, index=i+1, baselat=baselat, baselon=baselon, span=degreespan, seed=self.generator_seed())
            self.quadtrees.append(qt)

    def bindTopoTexture(self):
        [x.bindTopoTexture() for x in self.quadtrees]

    def draw(self, shader):
        if shader is True:
            self.shader.attach()
            GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'normalTexture'), 0)
            GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'shadowTexture'), 1)
            GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'topoTexture'), 2)
            GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'specularTexture'), 3)
            GL.glUniform1f(GL.glGetUniformLocation(self.shader.shader, 'shadowMapStepX'), 1.0/(factory.width*2.0))
            GL.glUniform1f(GL.glGetUniformLocation(self.shader.shader, 'shadowMapStepY'), 1.0/(factory.height*2.0))

            cameraPos = np.array(factory.camera.position)/self.radius
            lightPos = np.array(factory.sun.position)/np.linalg.norm(factory.sun.position)
            GL.glUniform3f(GL.glGetUniformLocation(self.shader.shader, 'v3CameraPos'), cameraPos[0], cameraPos[1], cameraPos[2])
            GL.glUniform3f(GL.glGetUniformLocation(self.shader.shader, 'v3LightPos'), lightPos[0], lightPos[1], lightPos[2])
        else:
            self.shadowshader.attach()
            GL.glUniform1i(GL.glGetUniformLocation(self.shadowshader.shader, 'topoTexture'), 2)

        [x.draw(shader) for x in self.quadtrees]

        if shader is True:
            self.shader.dettach()
        else:
            self.shadowshader.dettach()

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