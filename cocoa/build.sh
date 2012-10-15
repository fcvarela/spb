/bin/sh bundle.sh Vaalbara

cp /usr/X11/lib/libfreetype.6.dylib libs/

clang++ -pipe -std=c++0x -O3 \
*.cpp \
tinythread/*.cpp \
-g \
-I. \
-I./SOIL \
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
SOIL/libsoil.a \
/usr/local/lib/libconfig++.a \
./libconfig-1.4.9/lib/.libs/libconfig++.a \
./libconfig-1.4.9/lib/.libs/libconfig.a \
/usr/local/lib/libGLEW.a \
/usr/local/lib/libftgl.a \
./glfw/lib/cocoa/libglfw.a \
-L/usr/X11/lib \
-lfreetype \
-o Vaalbara.app/Contents/MacOS/Vaalbara

cp -r conf Vaalbara.app/Contents/Resources
cp -r data Vaalbara.app/Contents/Resources
cp -r libs Vaalbara.app/Contents/Resources
cp -r iconstuff/myIcon.icns Vaalbara.app/Contents/Resources/Icon.icns

install_name_tool -change /opt/X11/lib/libfreetype.6.dylib @executable_path/../Resources/libs/libfreetype.6.dylib Vaalbara.app/Contents/MacOS/Vaalbara
install_name_tool -change /usr/X11/lib/libfreetype.6.dylib @executable_path/../Resources/libs/libfreetype.6.dylib Vaalbara.app/Contents/MacOS/Vaalbara
install_name_tool -id @executable_path/../Resources/libs/ Vaalbara.app/Contents/Resources/libs/libfreetype.6.dylib


