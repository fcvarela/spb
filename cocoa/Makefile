# Include files to watch for changes
INCLUDE_FILES = $(wildcard src/*.h)

# Local source files
SOURCE_FILES = $(wildcard src/*.cpp)

# Object files
objects = $(subst src,build,$(SOURCE_FILES:.cpp=.o))

# Compiler flags
CFLAGS = -Isrc -I. -I./SOIL -I./glfw/include -I./libnoise -I./libnoise/src

# Linker flags
LINKFLAGS = SOIL/libsoil.a \
			/usr/local/lib/libGLEW.a \
			/usr/local/lib/libftgl.a \
			/usr/local/lib/libglfw.a \
			./libnoise/build/src/libnoise.a \
			-L/usr/X11/lib \
			-lfreetype

# Compile command
COMPILE = g++ -O3 -Wall -Wfatal-errors 

# OS Detection
OS=$(shell uname -s)
ifeq ($(OS), Linux)
    CFLAGS += -I/usr/include/freetype2
    LINKFLAGS += -lopenal -lGLU
    COMPILE += -Dlinux
    OUTPUT = Vaalbara
endif
ifeq ($(OS), Darwin)
    CFLAGS += -I/usr/local/include -I/usr/X11/include -I/usr/X11/include/freetype2 -I/usr/local/include/freetype2
    LINKFLAGS += -L/usr/local/lib -lobjc -framework Cocoa -framework OpenGL -framework IOKit -framework OpenAL
    COMPILE += -arch x86_64 -Dmacosx
    OUTPUT = Vaalbara.app/Contents/MacOS/Vaalbara
endif

COMPILE += $(CFLAGS)

vaalbara: $(objects)
	$(COMPILE) -o $(OUTPUT) $(objects) $(LINKFLAGS)

$(objects): build/%.o: src/%.cpp $(INCLUDE_FILES) Makefile
	$(COMPILE) $< -c -o $@

clean:
	rm -rf Vaalbara.app
	rm -r build/*.o
