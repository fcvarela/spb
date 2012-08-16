#!/usr/bin/env python
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

from planet import *
import factory

import sys

sunlon = 0.

def main():
    glutInit(sys.argv)

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutInitWindowSize(800, 500)
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

def initialize():
    glDepthFunc(GL_LEQUAL)
    glEnable(GL_DEPTH_TEST)
    glClearDepth(1.0)
    glCullFace(GL_BACK)
    glEnable(GL_CULL_FACE)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_AMBIENT, array([1.0, 1.0, 1.0, 1.0]))
    glLightfv(GL_LIGHT0, GL_DIFFUSE, array([1.0, 1.0, 1.0, 1.0]))
    glLightfv(GL_LIGHT0, GL_SPECULAR, array([1.0, 1.0, 1.0, 1.0]))

    # got gl state, spawn factory singleton
    factory.planet = Planet('planets/planet1.conf')
    factory.lastframe = glutGet(GLUT_ELAPSED_TIME)/1000.

def changeSize(width, height):
    if height == 0:
        height = 1

    ratio = width/height

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    glViewport(0, 0, width, height)
    gluPerspective(45., ratio, .001, 6371000.*2.)

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

    dist = np.linalg.norm(camera.position)-1.0

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
    step()
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    
    glMultMatrixd(factory.camera.rotation.gl_matrix())
    glMultMatrixd(factory.camera.nodes['yaw'].rotation.gl_matrix())
    glTranslatef(-factory.camera.position[0], -factory.camera.position[1], -factory.camera.position[2])

    sunlon += factory.dt*10.
    if sunlon > 360.:
        sunlon = sunlon-360.

    factory.sun.position = factory.camera.position#geocentricToCarthesian(sunlon, sunlon, 2.)
    glLightfv(GL_LIGHT0, GL_POSITION, factory.sun.position);
    
    drawAxes()

    glColor3f(1., 1., 1.)
    factory.planet.draw()
    
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
