from ConfigParser import SafeConfigParser

from terrainquadtree import *
from shaderprogram import *

class Planet:
    def __init__(self, conffile):
        parser = SafeConfigParser()
        parser.read(conffile)

        self.radius = parser.get('planet', 'circumference') / (2.0 * math.PI)
        self.maxlod = maxlod
        self.atmosphere_radius = 1.025*self.radius
        self.quadtrees = []

        self.shader = ShaderProgram('planet')

        baselat = 0.
        degreespan = 90.
        for i in range(0,8):
            baselon = i%4 * 90.
            if i > 3:
                baselat = -90.
            
            qt = TerrainQuadtree(parent=None, maxlod=maxlod, index=1, baselat=baselat, baselon=baselon, span=degreespan)
            self.quadtrees.append(qt)

    def draw(self):
        self.shader.attach()
        [x.draw() for x in self.quadtrees]
        self.shader.dettach()