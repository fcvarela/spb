from ctypes import cdll
lib = cdll.LoadLibrary('./genlib.dylib')

class genlib(object):
    def __init__(self):
        self.obj = lib.genlib_new()

    def initParams(self):
        lib.genlib_initParams(self.obj)

    def createGeometry(self):
        lib.genlib_createGeometry(self.obj)

    def getTopography(self):
        lib.genlib_getTopography(self.obj)

    def getNormals(self):
        lib.genlib_getNormals(self.obj)

    def getSpecular(self):
        lib.genlib_getSpecular(self.obj)

    def getTexture(self):
        lib.genlib_getTexture(self.obj)


if __name__ == '__main__':
    g = genlib()
    g.initParams()
    g.createGeometry()
    g.getTopography()
    g.getTexture()
