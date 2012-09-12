#!/usr/bin/env python2.7 -OO

import OpenGL
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.arrays import numpymodule

from pygame.locals import *

import pygame, math, sys
import factory

from planet import *

framenumber = 0
sunlon = 0.
screen = None
stop = False
clock = None

def main():
    global screen
    global clock

    pygame.init()
    pygame.display.init()

    # initialize clock
    clock = pygame.time.Clock()

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

    factory.aspect_ratio = float(factory.width)/float(factory.height)
    factory.hfov = 60.0
    factory.vfov = 2.0 * math.atan(math.tan(factory.hfov/2.0*math.pi/180.0)/factory.aspect_ratio)*180.0/math.pi
    
    size = (factory.width, factory.height)
    screen = pygame.display.set_mode(size, gl_flags)

    rendererString = "Renderer: %s %s" % (\
        glGetString(GL_RENDERER),\
        glGetString(GL_VERSION))
    print rendererString
    initialize()
    
    while not stop:
        # handle events
        handle_events(pygame.event.get())

        # tick the clock
        clock.tick()

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

            if event.key == K_f:
                factory.trackFrustum = not factory.trackFrustum

        if event.type == KEYUP:
            factory.keys[event.key] = False

def initialize():
    glDepthFunc(GL_LEQUAL)
    glEnable(GL_DEPTH_TEST)
    glClearDepth(1.0)
    glCullFace(GL_BACK)
    glEnable(GL_CULL_FACE)

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_AMBIENT, array([.05, .05, .05, 1.0]))
    glLightfv(GL_LIGHT0, GL_DIFFUSE, array([.8, .8, .8, 1.0]))
    glLightfv(GL_LIGHT0, GL_SPECULAR, array([.4, .4, .4, 1.0]))

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
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def step():
    time = pygame.time.get_ticks()/1000.0
    factory.dt = time - factory.lastframe
    factory.lastframe = time
    
    keys = factory.keys
    camera = factory.camera
    dt = factory.dt
    dist = (factory.veclen(camera.position)-factory.planet.radius)/2.0

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
    factory.planet.draw(shader, framenumber)
    glPopMatrix()

def determineFrustum(camera, planet):
    distance = factory.veclen(camera.position)
    altitude = distance - planet.radius

    if distance > planet.radius * planet.atmosphere_radius:
        near = 1.0#max(1.0, (distance - planet.radius)/2.0)
        far = distance+planet.radius*3.0
    else:
        near = 1.0
        far = math.sqrt(altitude * (2 * planet.radius + altitude))*1000.0

    factory.near = near
    factory.far = far

    return (near, far)

def processQueue():
    instance = None
    done = False
    threadcount = 0
    while done == False:
        try:
            (instance, ) = factory.generatorQueue.get_nowait()
            if instance.ready == True:
                continue

            function = instance.nextStep['function']
            if instance.nextStep['threaded'] is True:
                threading.Thread(target=function, args=()).start()
                # do not return. threaded items run in bg, do ONE task in fg
                # do not spawn more than 4 threads at a time
                threadcount += 1
                if threadcount == 1:
                    done = True
            else:
                function()
                factory.generatorQueue.task_done()
                factory.generatorQueue.put((instance, ))
                done = True
        except:
            done = True

def display():
    global framenumber
    
    factory.drawnNodes = 0
    if framenumber % 2 == 0:
        processQueue()

    # reset the projection matrix
    ratio = float(factory.width)/float(factory.height)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glViewport(0, 0, factory.width, factory.height)

    # outside atmosphere? from surface to radius*2
    (near, far) = determineFrustum(factory.camera, factory.planet)
    gluPerspective(factory.vfov, factory.aspect_ratio, near, far)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    # Render from camera
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity()
    glMultMatrixd(factory.camera.rotation.gl_matrix())
    glTranslatef(-factory.camera.position[0], -factory.camera.position[1], -factory.camera.position[2])
    factory.calculateFrustum()
    
    global sunlon
    #sunlon += factory.dt*2.
    factory.sun.position = factory.geocentricToCarthesian(0.0, sunlon, factory.planet.radius*8.0)
    glLightfv(GL_LIGHT0, GL_POSITION, list(factory.sun.position))

    renderObjects(True)
    framenumber += 1
    #print factory.drawnNodes
    #print clock.get_fps()
    #print factory.veclen(factory.camera.position) - factory.planet.radius;

if __name__ == '__main__':
    main()
