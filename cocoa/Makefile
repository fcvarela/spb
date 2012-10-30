# Include files to watch for changes
INCLUDE_FILES = $(wildcard src/*.h)

# Local source files
SOURCE_FILES = $(wildcard src/*.cpp)

# Object files
objects = $(subst src,build,$(SOURCE_FILES:.cpp=.o))

# Compiler flags
CFLAGS = -Isrc -I./libs/libSOIL -I./libs/libcdf -I./libs/libnoise -I./libs/libnoise/src -I./libs/libtinythread

# Linker flags
LINKFLAGS = \
			libs/libSOIL/libsoil.a \
			libs/libcdf/libcdf.a \
			libs/libtinythread/libtinythread.a \
			/usr/local/lib/libGLEW.a \
			/usr/local/lib/libftgl.a \
			/usr/local/lib/libglfw.a \
			libs/libnoise/build/src/libnoise.a \
			-L/usr/X11/lib \
			-lfreetype

# Compile command
COMPILE = g++ -O3 -Wfatal-errors 

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
    OUTPUT = Vaalbara
endif

COMPILE += $(CFLAGS)

vaalbara: $(objects)
	$(COMPILE) -o $(OUTPUT) $(objects) $(LINKFLAGS)
	./bundle.sh $(OUTPUT)
	cp -r conf $(OUTPUT).app/Contents/Resources
	cp -r data $(OUTPUT).app/Contents/Resources
	cp -r iconstuff/myIcon.icns $(OUTPUT).app/Contents/Resources/Icon.icns
	mv $(OUTPUT) $(OUTPUT).app/Contents/MacOS/

$(objects): build/%.o: src/%.cpp $(INCLUDE_FILES) Makefile
	$(COMPILE) $< -c -o $@

clean:
	rm -rf Vaalbara.app
	rm -r build/*.o
