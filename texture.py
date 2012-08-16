from OpenGL import *
import Image

class Texture:
    def __init__(self, imagefile):
        textureMode = GL.GL_RGBA

        if imagefile[-3:] == 'bmp':
            im = Image.open(imagefile)
            try:
                ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBA", 0, -1)
            except SystemError:
                ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBX", 0, -1)

        if imagefile[-3:] == 'raw':
            ix, iy, image = 256, 256, open(imagefile, 'r').read()
            textureMode = GL.GL_RGB

        self.id = GL.glGenTextures(1)

        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)
        GL.glPixelStorei(GL.GL_UNPACK_ALIGNMENT, 1)
        GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, textureMode, ix, iy, 0, textureMode, GL.GL_UNSIGNED_BYTE, image)

        self.textureunits = [\
            GL.GL_TEXTURE0,
            GL.GL_TEXTURE1,
            GL.GL_TEXTURE2,
            GL.GL_TEXTURE3]

    def bind(self, textureUnit=0):
        GL.glActiveTexture(self.textureunits[textureUnit])
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_S, GL.GL_CLAMP_TO_EDGE);
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_T, GL.GL_CLAMP_TO_EDGE);
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR)
        GL.glTexParameterf(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_GENERATE_MIPMAP, GL.GL_FALSE);

        GL.glBindTexture(GL.GL_TEXTURE_2D, self.id)

    def unbind(self):
        GL.glDisable(GL.GL_TEXTURE_2D)