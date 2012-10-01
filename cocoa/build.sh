/bin/sh bundle.sh spb

clang++ -O2 \
*.cpp \
-I. \
-I./glfw/include \
./glfw/lib/cocoa/libglfw.a \
-framework Cocoa \
-framework OpenGL \
-framework IOKit \
-framework OpenAL \
-framework GLUT \
-o SPB.app/Contents/MacOS/spb
