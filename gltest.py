#!/usr/bin/env python
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from OpenGL.GL.ARB.shader_objects import *
from OpenGL.GL.ARB.fragment_shader import *
from OpenGL.GL.ARB.vertex_shader import *

from planet import *
import factory
import sys

shadow_fbo = None
depth_texture = None
sunlon = 0.

def main():
    glutInit(sys.argv)

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutInitWindowSize(1280, 800)
    glutCreateWindow('spb')
    glutDisplayFunc(display)
    glutIdleFunc(display)
    glutReshapeFunc(changeSize)
    
    glutMouseFunc(mouseclickhandler)
    #glutPassiveMotionFunc(passivemotionhandler)
    
    glutKeyboardFunc(keydownhandler)
    glutKeyboardUpFunc(keyuphandler)

    glutSpecialFunc(specialkeydownhandler)
    glutSpecialUpFunc(specialkeyuphandler)

    initialize()

    factory.camera.position = [0.0, 0.0, 1738140*2.0]
    factory.sun.position = [0.0, 0.0, 1738140.0*10.0]

    generateShadowFBO(1280.0, 800.0)

    glutMainLoop()

def generateShadowFBO(width, height):
    global shadow_fbo
    global depth_texture

    shadowmap_ratio = 2.0

    shadowMapWidth = width * shadowmap_ratio
    shadowMapHeight = height * shadowmap_ratio

    depth_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, depth_texture)

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    
    # Remove artefact on the edges of the shadowmap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
    
    # This is to allow usage of shadow2DProj function in the shader
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL)
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY)

    # No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, None)
    glBindTexture(GL_TEXTURE_2D, 0)
    
    # create a framebuffer object
    shadow_fbo = glGenFramebuffers(1)
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo)
    
    # Instruct openGL that we won't bind a color texture with the currently binded FBO
    glDrawBuffer(GL_NONE)
    glReadBuffer(GL_NONE)
    
    # attach the texture to FBO depth attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0)
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER)
    if FBOstatus != GL_FRAMEBUFFER_COMPLETE:
        print("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO");
    
    # switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0)

def setTextureMatrix():
    bias = [\
        0.5, 0.0, 0.0, 0.0, \
        0.0, 0.5, 0.0, 0.0, \
        0.0, 0.0, 0.5, 0.0, \
        0.5, 0.5, 0.5, 1.0]

    # Grab modelview and transformation matrices
    modelView = glGetDoublev(GL_MODELVIEW_MATRIX);
    projection = glGetDoublev(GL_PROJECTION_MATRIX);
    
    glMatrixMode(GL_TEXTURE);
    glActiveTexture(GL_TEXTURE1);
    
    glLoadIdentity();   
    glLoadMatrixd(bias);
    
    # concatating all matrice into one.
    glMultMatrixd (projection);
    glMultMatrixd (modelView);
    
    # Go back to normal matrix mode
    glMatrixMode(GL_MODELVIEW);

def initialize():
    glDepthFunc(GL_LEQUAL)
    glEnable(GL_DEPTH_TEST)
    glClearDepth(1.0)
    glCullFace(GL_BACK)
    glEnable(GL_CULL_FACE)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_AMBIENT, array([0.1, 0.1, 0.1, 1.0]))
    glLightfv(GL_LIGHT0, GL_DIFFUSE, array([1.0, 1.0, 1.0, 1.0]))
    glLightfv(GL_LIGHT0, GL_SPECULAR, array([1.0, 1.0, 1.0, 1.0]))

    # got gl state, spawn factory singleton
    factory.planet = Planet('planets/planet1.conf')
    factory.lastframe = glutGet(GLUT_ELAPSED_TIME)/1000.

def changeSize(width, height):
    if height == 0:
        height = 1

    ratio = float(width)/float(height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    glViewport(0, 0, width, height)
    vfov = 35.
    gluPerspective(vfov, ratio, 1.0, 1738140*3.0)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def keydownhandler(*args):
    factory.keys[ord(args[0])] = True

def keyuphandler(*args):
    factory.keys[ord(args[0])] = False

    if args[0] is 'l':
        toggleWireframe()

def specialkeydownhandler(*args):
    factory.specialkeys[args[0]] = True

def specialkeyuphandler(*args):
    factory.specialkeys[args[0]] = False
    
def mouseclickhandler(button, state, x, y):
    global roll, pitch, yaw

def step():
    time = glutGet(GLUT_ELAPSED_TIME) / 1000.
    factory.dt = time - factory.lastframe
    factory.lastframe = time
    
    keys = factory.keys
    specialkeys = factory.specialkeys
    camera = factory.camera
    dt = factory.dt

    dist = (np.linalg.norm(camera.position)-1738140.0)/2.0

    if keys[ord('w')] == True:
        camera.move((0., 0., -1.*dt*dist))

    if keys[ord('s')] == True:
        camera.move((0., 0., 1.*dt*dist))

    if keys[ord('a')] == True:
        camera.move((-1.*dt*dist, 0., 0.))

    if keys[ord('d')] == True:
        camera.move((1.*dt*dist, 0., 0.))

    if keys[ord('q')] == True:
        camera.move((0., 1.*dt*dist, 0.))

    if keys[ord('z')] == True:
        camera.move((0., -1.*dt*dist, 0.))

    if keys[ord('x')] == True:
        camera.rotate((0., 0., 1.), 25.*dt)

    if keys[ord('c')] == True:
        camera.rotate((0., 0., 1.), -25.*dt)

    if specialkeys[GLUT_KEY_LEFT] == True:
        camera.rotate((0., 1., 0.), 25.*dt)

    if specialkeys[GLUT_KEY_RIGHT] == True:
        camera.rotate((0., 1., 0.), -25.*dt)

    if specialkeys[GLUT_KEY_UP] == True:
        camera.rotate((1., 0., 0.), -25.*dt)

    if specialkeys[GLUT_KEY_DOWN] == True:
        camera.rotate((1., 0., 0.), 25.*dt)
        

def toggleWireframe():
    if factory.wireframe is True:
        glPolygonMode(GL_FRONT, GL_FILL)
        factory.wireframe = False
    else:
        glPolygonMode(GL_FRONT, GL_LINE)
        factory.wireframe = True

def display():
    global sunlon
    global shadow_fbo

    step()

    # render the shadow
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo)
    glUseProgram(0)
    glViewport(0, 0, 1280*2, 800*2)
    glClear(GL_DEPTH_BUFFER_BIT)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 

    # setup matrices
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(35, 1280.0/800.0, 1, factory.planet.radius*2.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    gluLookAt(factory.sun.position[0], factory.sun.position[1], factory.sun.position[2],\
        0, 0, 0, 0, 1, 0);
    glCullFace(GL_FRONT)
    factory.planet.draw(False)

    setTextureMatrix()
    glBindFramebuffer(GL_FRAMEBUFFER, 0)
    glViewport(0, 0, 1280, 800)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1)
    glBindTexture(GL_TEXTURE_2D, depth_texture)

    # setup matrices
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(35, 1280.0/800.0, 1, factory.planet.radius*30)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    glMultMatrixd(factory.camera.rotation.gl_matrix())
    glMultMatrixd(factory.camera.nodes['yaw'].rotation.gl_matrix())
    glTranslatef(-factory.camera.position[0], -factory.camera.position[1], -factory.camera.position[2])
    
    glCullFace(GL_BACK)
    factory.planet.draw(True)

    sunlon += factory.dt*10.
    #if sunlon > 360.:
    #    sunlon = sunlon-360.

    factory.sun.position = factory.geocentricToCarthesian(0., sunlon, 1738140*10.0)
    glLightfv(GL_LIGHT0, GL_POSITION, factory.sun.position);

    glutSwapBuffers()

def drawAxes():
    glDisable(GL_LIGHTING)
    glPushMatrix()
    
    glColor3f(1.0, 0.0, 0.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(10., 0., 0.)
    glEnd()
    
    glColor3f(0.0, 1.0, 0.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(0., 10., 0.)
    glEnd()

    glColor3f(0.0, 0.0, 1.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(0., 0., 10.)
    glEnd()
    
    glPopMatrix()
    glEnable(GL_LIGHTING)

if __name__ == '__main__':
    main()
