#!/usr/bin/env python2.7

from OpenGL.GL import *
from OpenGL.GLU import *
from pygame.locals import *

import pygame, math, sys
import factory

from planet import *

framenumber = 0
sunlon = 0.
screen = None
stop = False

def main():
    global screen

    pygame.init()
    pygame.display.init()
    info = pygame.display.Info()
    factory.width = info.current_w
    factory.height = info.current_h
    
    fullscreen = False
    
    if fullscreen is False:
        factory.width /= 2
        factory.height /= 2
        gl_flags = OPENGL|DOUBLEBUF|HWSURFACE
    else:
        gl_flags = FULLSCREEN|OPENGL|DOUBLEBUF|HWSURFACE

    pygame.display.gl_set_attribute(pygame.locals.GL_MULTISAMPLEBUFFERS, 0)
    pygame.display.gl_set_attribute(pygame.locals.GL_MULTISAMPLESAMPLES, 1)

    size = (factory.width, factory.height)
    screen = pygame.display.set_mode(size, gl_flags)
    initialize()
    
    while not stop:
        # handle events
        handle_events(pygame.event.get())

        # process stuff
        step()

        # render
        display()

        # switch buffers
        pygame.display.flip()

        # ask gl to draw
        glFinish()

    #pycallgraph.make_dot_graph('test.png')

def handle_events(events):
    global stop

    for event in events:
        if event.type == QUIT:
            stop = True

        if event.type == KEYDOWN:
            factory.keys[event.key] = True
            if event.key == K_ESCAPE:
                stop = True
                
            if event.key == K_l:
                toggleWireframe()

        if event.type == KEYUP:
            factory.keys[event.key] = False

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

    factory.calculateFrustum()

    # got gl state, spawn factory singleton
    factory.planet = Planet('planets/planet1.conf')
    factory.lastframe = pygame.time.get_ticks()/1000.0

    factory.camera.position = [0.0, 0.0, factory.planet.radius*2.0]
    factory.sun.position = [0.0, 0.0, factory.planet.radius*8.0]

def changeSize(width, height):
    if height == 0:
        height = 1

    factory.width = width
    factory.height = height

    ratio = float(width)/float(height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    glViewport(0, 0, width, height)
    gluPerspective(35.0, ratio, 1.0, factory.planet.radius * 4.0)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def step():
    time = pygame.time.get_ticks()/1000.0
    factory.dt = time - factory.lastframe
    factory.lastframe = time
    
    keys = factory.keys
    camera = factory.camera
    dt = factory.dt

    dist = (np.linalg.norm(camera.position)-factory.planet.radius)/2.0

    if keys[K_w] == True:
        camera.move((0., 0., -1.*dt*dist))

    if keys[K_s] == True:
        camera.move((0., 0., 1.*dt*dist))

    if keys[K_a] == True:
        camera.move((-1.*dt*dist, 0., 0.))

    if keys[K_d] == True:
        camera.move((1.*dt*dist, 0., 0.))

    if keys[K_q] == True:
        camera.move((0., 1.*dt*dist, 0.))

    if keys[K_z] == True:
        camera.move((0., -1.*dt*dist, 0.))

    if keys[K_x] == True:
        camera.rotate((0., 0., 1.), 25.*dt)

    if keys[K_c] == True:
        camera.rotate((0., 0., 1.), -25.*dt)

    if keys[K_LEFT] == True:
        camera.rotate((0., 1., 0.), 25.*dt)

    if keys[K_RIGHT] == True:
        camera.rotate((0., 1., 0.), -25.*dt)

    if keys[K_UP] == True:
        camera.rotate((1., 0., 0.), -25.*dt)

    if keys[K_DOWN] == True:
        camera.rotate((1., 0., 0.), 25.*dt)

def toggleWireframe():
    if factory.wireframe is True:
        glPolygonMode(GL_FRONT, GL_FILL)
        factory.wireframe = False
    else:
        glPolygonMode(GL_FRONT, GL_LINE)
        factory.wireframe = True

def renderObjects(shader):
    glPushMatrix()
    factory.planet.draw(shader)
    glPopMatrix()

def display():
    factory.drawnNodes = 0
    instance = None
    try:
        (instance, ) = factory.generatorQueue.get_nowait()
        framenumber = 0
    except:
        pass

    if instance is not None:
        instance.generateTextures()

    # reset the projection matrix
    ratio = float(factory.width)/float(factory.height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glViewport(0, 0, factory.width, factory.height)
    gluPerspective(35.0, ratio, 1.0, factory.planet.radius * 4.0)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    # Render from camera
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity()
    glMultMatrixd(factory.camera.rotation.gl_matrix())
    glTranslatef(-factory.camera.position[0], -factory.camera.position[1], -factory.camera.position[2])

    factory.calculateFrustum()
    global sunlon
    #sunlon += factory.dt*10.
    factory.sun.position = factory.geocentricToCarthesian(0., sunlon, factory.planet.radius*8.0)
    glLightfv(GL_LIGHT0, GL_POSITION, factory.sun.position);

    renderObjects(True)

if __name__ == '__main__':
    main()
