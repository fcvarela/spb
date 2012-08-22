from OpenGL.GL import *
from OpenGL import *
import Image

class Texture:
    def __init__(self, size):
        self.size = size

        self.id = GL.glGenTextures(1)

        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_S, GL.GL_CLAMP_TO_EDGE);
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_T, GL.GL_CLAMP_TO_EDGE);
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR)
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR_MIPMAP_LINEAR)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_GENERATE_MIPMAP, GL.GL_TRUE);
        GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, GL.GL_RGBA, size, size, 0, GL.GL_RGBA, GL.GL_UNSIGNED_BYTE, None)

    def bind(self, textureUnit=GL.GL_TEXTURE0):
        GL.glActiveTexture(textureUnit)
        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)

    def unbind(self):
        GL.glDisable(GL.GL_TEXTURE_2D)
