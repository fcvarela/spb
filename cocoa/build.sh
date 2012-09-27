/bin/sh bundle.sh spb

clang -O2 \
main.cpp SceneManager.cpp \
-I. \
-I./glfw/include \
./glfw/lib/cocoa/libglfw.a \
-framework Cocoa \
-framework OpenGL \
-framework IOKit \
-o SPB.app/Contents/MacOS/spb
