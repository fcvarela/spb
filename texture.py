from OpenGL import *
import Image

class Texture:
    def __init__(self, imagefile):
        im = Image.open(imagefile)
        try:
            ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBA", 0, -1)
        except SystemError:
            ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBX", 0, -1)

        self.id = GL.glGenTextures(1)

        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)
        GL.glPixelStorei(GL.GL_UNPACK_ALIGNMENT, 1)

        GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, 3, ix, iy, 0, GL.GL_RGBA, GL.GL_UNSIGNED_BYTE, image)

    def bind(self, textureUnit=0):
        GL.glEnable(GL.GL_TEXTURE_2D)
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_NEAREST)
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_NEAREST)

        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)

    def unbind(self):
        GL.glDisable(GL.GL_TEXTURE_2D)