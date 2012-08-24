from OpenGL import *
from OpenGL.GL import *

class Framebuffer:
    def __init__(self):
        self.id = glGenFramebuffers(1)

    def bind(self, texture_id):
        glBindFramebuffer(GL_FRAMEBUFFER, self.id)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0)

    def unbind(self):
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
