/bin/sh bundle.sh Vaalbara

cp /usr/X11/lib/libfreetype.6.dylib libs/
make -j8

cp -r conf Vaalbara.app/Contents/Resources
cp -r data Vaalbara.app/Contents/Resources
cp -r libs Vaalbara.app/Contents/Resources
cp -r iconstuff/myIcon.icns Vaalbara.app/Contents/Resources/Icon.icns

install_name_tool -change /opt/X11/lib/libfreetype.6.dylib @executable_path/../Resources/libs/libfreetype.6.dylib Vaalbara.app/Contents/MacOS/Vaalbara
install_name_tool -change /usr/X11/lib/libfreetype.6.dylib @executable_path/../Resources/libs/libfreetype.6.dylib Vaalbara.app/Contents/MacOS/Vaalbara
install_name_tool -id @executable_path/../Resources/libs/ Vaalbara.app/Contents/Resources/libs/libfreetype.6.dylib
