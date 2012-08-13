#!/usr/bin/env python

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

from quat import *
from node import *
from planet import *

import sys

name = 'ball_glut'

lastframe = 0.0
dt = 0.0

planet = None
camera = Node()
camera.position = [0.2, 0.2, 3.0]
camera.nodes = {'yaw': Node()}

keys = []
specialkeys = []

for i in range(256):
    keys.append(False)
    specialkeys.append(False)

def main():
    glutInit(sys.argv)
    lastframe = glutGet(GLUT_ELAPSED_TIME)/1000.;

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutInitWindowSize(800, 500)
    glutCreateWindow(name)
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
    global planet
    planet = Planet(6371, 3)

def changeSize(width, height):
    if height == 0:
        height = 1

    ratio = 1.0 * (width/height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    glViewport(0, 0, width, height)
    gluPerspective(60., ratio, 1., 1000.)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def keydownhandler(*args):
    keys[ord(args[0])] = True

def keyuphandler(*args):
    keys[ord(args[0])] = False

def specialkeydownhandler(*args):
    specialkeys[args[0]] = True

def specialkeyuphandler(*args):
    specialkeys[args[0]] = False
    
def mouseclickhandler(button, state, x, y):
    global roll, pitch, yaw

def step():
    global position, dt, lastframe, cameraquat

    time = glutGet(GLUT_ELAPSED_TIME) / 1000.;
    dt = time - lastframe
    lastframe = time

    if keys[ord('w')] == True:
        camera.move((0., 0., -1.*dt))

    if keys[ord('s')] == True:
        camera.move((0., 0., 1.*dt))

    if keys[ord('a')] == True:
        camera.move((-1.*dt, 0., 0.))

    if keys[ord('d')] == True:
        camera.move((1.*dt, 0., 0.))

    if keys[ord('q')] == True:
        camera.move((0., 1.*dt, 0.))

    if keys[ord('z')] == True:
        camera.move((0., -1.*dt, 0.))

    if keys[ord('x')] == True:
        camera.rotate((0., 0., 1.), 25.*dt)

    if keys[ord('c')] == True:
        camera.rotate((0., 0., 1.), -25.*dt)

    if specialkeys[GLUT_KEY_LEFT] == True:
        camera.nodes['yaw'].rotate((0., 1., 0.), 25.*dt)

    if specialkeys[GLUT_KEY_RIGHT] == True:
        camera.nodes['yaw'].rotate((0., 1., 0.), -25.*dt)

    if specialkeys[GLUT_KEY_UP] == True:
        camera.rotate((1., 0., 0.), -25.*dt)

    if specialkeys[GLUT_KEY_DOWN] == True:
        camera.rotate((1., 0., 0.), 25.*dt)

def display():
    global cameraquat, position
    step()
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    
    glMultMatrixd(camera.rotation.gl_matrix())
    glMultMatrixd(camera.nodes['yaw'].rotation.gl_matrix())
    glTranslatef(-camera.position[0], -camera.position[1], -camera.position[2])
    
    drawAxes()
    planet.draw()
    
    glutSwapBuffers()

def drawAxes():
    glPushMatrix()
    
    glColor3f(1.0, 0.0, 0.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(1., 0., 0.)
    glEnd()
    
    glColor3f(0.0, 1.0, 0.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(0., 1., 0.)
    glEnd()

    glColor3f(0.0, 0.0, 1.0)
    glBegin(GL_LINES)
    glVertex3f(0., 0., 0.)
    glVertex3f(0., 0., 1.)
    glEnd()
    
    glPopMatrix()

if __name__ == '__main__':
    main()
