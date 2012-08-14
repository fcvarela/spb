from terrainquadtree import *

class Planet:
    def __init__(self, radius, maxlod):
        self.radius = radius
        self.maxlod = maxlod
        self.atmosphere_radius = 1.025*self.radius
        self.quadtrees = []

        # create 8 quadtrees, octagon shaped sphere
        # one facing top, the others are simply rotations of this shape

        center = (0., .5, 0.0)
        dx = (1., 0., 0.)
        dy = (0., 0., -1.)
    
        qt = TerrainQuadtree(parent=None, maxlod=maxlod, face=0, index=0, center=center, dx=dx, dy=dy)
        self.quadtrees.append(qt)

        center = (-.5, 0., 0.)
        dx = (0., 0., 1.)
        dy = (0., 1., 0.)

        qt = TerrainQuadtree(parent=None, maxlod=maxlod, face=0, index=1, center=center, dx=dx, dy=dy)
        self.quadtrees.append(qt)

    def draw(self):
        [x.draw() for x in self.quadtrees]