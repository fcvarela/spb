#!/usr/bin/env python
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import threading

from planet import *
import factory
import sys

depth_texture = None
sunlon = 0.

def main():
    glutInit(sys.argv)

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutInitWindowSize(800, 600)
    glutFullScreen()
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

    glutMainLoop()

def generateShadowFBO(width, height):
    depth_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, depth_texture)

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, factory.width, factory.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, None)

    glBindTexture(GL_TEXTURE_2D, 0)

    return depth_texture

def initialize():
    glDepthFunc(GL_LEQUAL)
    glEnable(GL_DEPTH_TEST)
    glClearDepth(1.0)
    glCullFace(GL_BACK)
    glEnable(GL_CULL_FACE)

    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_AMBIENT, array([0.1, 0.1, 0.1, 1.0]))
    glLightfv(GL_LIGHT0, GL_DIFFUSE, array([1.0, 1.0, 1.0, 1.0]))
    glLightfv(GL_LIGHT0, GL_SPECULAR, array([.6, .6, .6, 1.0]))

    # got gl state, spawn factory singleton
    factory.planet = Planet('planets/planet1.conf')
    factory.lastframe = glutGet(GLUT_ELAPSED_TIME)/1000.

    factory.camera.position = [0.0, 0.0, factory.planet.radius*2.0]
    factory.sun.position = [0.0, 0.0, factory.planet.radius*8.0]

def changeSize(width, height):
    if height == 0:
        height = 1

    print "Setting factory"
    factory.width = width
    factory.height = height

    ratio = float(width)/float(height)

    global depth_texture
    depth_texture = generateShadowFBO(width, height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    glViewport(0, 0, width, height)
    gluPerspective(35.0, ratio, 10000.0, factory.planet.radius * 10.0)

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

    dist = (np.linalg.norm(camera.position)-factory.planet.radius)/2.0

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

    #global sunlon
    #sunlon += factory.dt*10.
    #factory.sun.position = factory.geocentricToCarthesian(0., sunlon, factory.planet.radius*8.0)
    glLightfv(GL_LIGHT0, GL_POSITION, factory.sun.position);

def toggleWireframe():
    if factory.wireframe is True:
        glPolygonMode(GL_FRONT, GL_FILL)
        factory.wireframe = False
    else:
        glPolygonMode(GL_FRONT, GL_LINE)
        factory.wireframe = True

def RenderShadowCompareAfter():
    'reset gl params after comparison'
    glDisable(GL_TEXTURE_2D)

    glDisable(GL_TEXTURE_GEN_S)
    glDisable(GL_TEXTURE_GEN_T)
    glDisable(GL_TEXTURE_GEN_R)
    glDisable(GL_TEXTURE_GEN_Q)

    glDisable(GL_ALPHA_TEST)

def renderObjects(shader):
    glPushMatrix()
    glTranslatef(0., 0., factory.planet.radius)
    #glutSolidSphere(factory.planet.radius, 100, 100)
    glPopMatrix()

    glPushMatrix()
    factory.planet.draw(shader)
    glPopMatrix()

def display():
    step()

    # Render from camera
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity()
    glMultMatrixd(factory.camera.rotation.gl_matrix())
    glMultMatrixd(factory.camera.nodes['yaw'].rotation.gl_matrix())
    glTranslatef(-factory.camera.position[0], -factory.camera.position[1], -factory.camera.position[2])

    renderObjects(True)

    glutSwapBuffers()

if __name__ == '__main__':
    main()
