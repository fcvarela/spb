from libnoise import *

class TerrainGenerator:
    def __init__(self):
        self.SOUTH_COORD = -90
        self.NORTH_COORD = 90
        self.WEST_COORD = -180
        self.EAST_COORD = 180
        self.GRID_WIDTH = 1024
        self.GRID_HEIGHT = 512
        self.CUR_SEED = 0
        self.PLANET_CIRCUMFERENCE = 10917000.0
        self.MIN_ELEV = -3000.0
        self.MAX_ELEV = 28000.0
        self.CONTINENT_FREQUENCY = 1.0
        self.CONTINENT_LACUNARITY = 2.208984375
        self.MOUNTAIN_LACUNARITY = 2.142578125
        self.HILLS_LACUNARITY = 2.162109375
        self.PLAINS_LACUNARITY = 2.314453125
        self.BADLANDS_LACUNARITY = 2.212890625
        self.MOUNTAINS_TWIST = 1.0
        self.HILLS_TWIST = 1.0
        self.BADLANDS_TWIST = 1.0
        self.SEA_LEVEL = 0.0
        self.SHELF_LEVEL = -0.375
        self.MOUNTAINS_AMOUNT = 0.5;
        self.HILLS_AMOUNT = (1.0 + self.MOUNTAINS_AMOUNT) / 2.0;
        self.BADLANDS_AMOUNT = 0.03125;
        self.TERRAIN_OFFSET = 1.0;
        self.MOUNTAIN_GLACIATION = 1.375;
        self.CONTINENT_HEIGHT_SCALE = (1.0 - self.SEA_LEVEL) / 4.0;
        self.RIVER_DEPTH = 0.0234375;

        # init the c++ app with these generator params

        # sequentially call normals, topo, etc


