from quat import *

class Node:
    def __init__(self):
        self.rotation = Quat(0., 0., 0., 1.)
        self.position = [0., 0., 0.]
        self.nodes = None

    def rotate(self, vector, value):
        q = Quat(vector[0], vector[1], vector[2], value)
        self.rotation = self.rotation * q

    def move(self, vector):
        delta = self.rotation.rotate(vector)
        
        self.position[0] += delta[0]
        self.position[1] += delta[1]
        self.position[2] += delta[2]