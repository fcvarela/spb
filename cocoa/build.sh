/bin/sh bundle.sh Vaalbara

make -j5

cp -r conf Vaalbara.app/Contents/Resources
cp -r data Vaalbara.app/Contents/Resources
cp -r iconstuff/myIcon.icns Vaalbara.app/Contents/Resources/Icon.icns
