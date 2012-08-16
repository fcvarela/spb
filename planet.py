from OpenGL import *
import math
from ConfigParser import SafeConfigParser

from terrainquadtree import *
from shaderprogram import *

class Planet:
    def __init__(self, conffile):
        self.parser = SafeConfigParser()
        self.parser.read(conffile)

        self.radius = float(self.parser.get('planet', 'circumference')) / (2.0 * math.pi)
        self.maxlod = int(self.parser.get('terrain', 'maxlod'))

        self.atmosphere_radius = 1.025*self.radius
        self.quadtrees = []

        self.shader = ShaderProgram('planet')

        baselat = 0.
        degreespan = 90.
        for i in range(0,8):
            baselon = -180 + (i%4 * 90.)
            if i > 3:
                baselat = -90.
            
            qt = TerrainQuadtree(parent=None, maxlod=self.maxlod, index=i+1, baselat=baselat, baselon=baselon, span=degreespan, seed=self.generator_seed())
            self.quadtrees.append(qt)

    def draw(self):
        self.shader.attach()
        GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'normalTexture'), 0)
        GL.glUniform1i(GL.glGetUniformLocation(self.shader.shader, 'colorTexture'), 1)
        [x.draw() for x in self.quadtrees]
        self.shader.dettach()

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