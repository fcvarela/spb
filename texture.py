from OpenGL import *
from OpenGL.GL import *
import Image

class Texture:
    def __init__(self, size):
        self.size = size

        self.id = glGenTextures(1)

        glBindTexture(GL_TEXTURE_2D, self.id)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, None)

    def bind(self, textureUnit=GL_TEXTURE0):
        glActiveTexture(textureUnit)
        glBindTexture(GL_TEXTURE_2D, self.id)

    def unbind(self):
        glBindTexture(GL_TEXTURE_2D, 0)
        glDisable(GL_TEXTURE_2D)
