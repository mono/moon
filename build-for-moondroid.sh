#!/bin/sh
ACLOCAL_FLAGS="-I /usr/X11/share/aclocal" ./autogen.sh --host=arm-linux-androideabi --prefix=/opt/moonlight-android --with-pal=android --with-unwind=no --with-curl=no --with-mono-path=/Users/plasma/Work/moonlight-android/mono --with-manual-mono=yes --with-cairo=system --disable-browser-support CFLAGS="-DPLATFORM_ANDROID -I/opt/moonlight-android/include $CFLAGS" LDFLAGS="-L/opt/moonlight-android/lib $LDFLAGS" CXXFLAGS="-fno-rtti -DPLATFORM_ANDROID -I/opt/moonlight-android/include $CFLAGS"
make
make install
