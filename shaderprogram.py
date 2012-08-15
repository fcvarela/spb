from OpenGL import *
from OpenGL.GL import shaders

class ShaderProgram:
    def __init__(self, programname):
        vertfile = open('shaders/%s.vs' % programname, 'r')
        self.vertshader = shaders.compileShader(vertfile.read(), GL.GL_VERTEX_SHADER)

        fragfile = open('shaders/%s.fs' % programname, 'r')
        self.fragshader = shaders.compileShader(fragfile.read(), GL.GL_FRAGMENT_SHADER)

        self.shader = shaders.compileProgram(self.vertshader, self.fragshader)

    def attach(self):
        GL.glUseProgram(self.shader)

    def dettach(self):
        GL.glUseProgram(0)