/bin/sh bundle.sh Vaalbara

cp /opt/X11/lib/libfreetype.6.dylib libs/

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
-o Vaalbara.app/Contents/MacOS/Vaalbara

cp -r conf Vaalbara.app/Contents/Resources
cp -r data Vaalbara.app/Contents/Resources
cp -r libs Vaalbara.app/Contents/Resources

install_name_tool -change /opt/X11/lib/libfreetype.6.dylib @executable_path/../Resources/libs/libfreetype.6.dylib Vaalbara.app/Contents/MacOS/Vaalbara

