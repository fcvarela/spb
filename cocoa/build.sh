/bin/sh bundle.sh spb

clang++ -O2 \
*.cpp \
tinythread/*.cpp \
-g \
-I. \
-I./glfw/include \
-I/usr/local/include \
-I/usr/X11/include \
-I/usr/X11/include/freetype2 \
-I./tinythread \
-framework Cocoa \
-framework OpenGL \
-framework IOKit \
-framework OpenAL \
-framework GLUT \
/usr/local/lib/libconfig++.a \
/usr/local/lib/libGLEW.a \
/usr/local/lib/libftgl.a \
./glfw/lib/cocoa/libglfw.a \
-L/usr/X11/lib \
-lfreetype \
-o SPB.app/Contents/MacOS/spb

cp -r conf spb.app/Contents/Resources
cp -r data spb.app/Contents/Resources
