#!/bin/sh
ACLOCAL_FLAGS="-I /usr/X11/share/aclocal" PKG_CONFIG_PATH=$MOONLIGHT_PREFIX/lib/pkgconfig ./autogen.sh --host=arm-linux-androideabi --prefix=$MOONLIGHT_PREFIX --with-ffmpeg=no --with-alsa=no --with-pulseaudio=no --with-pal=android --with-unwind=no --with-curl=system --with-manual-mono=yes --with-cairo=system --disable-browser-support --with-gallium-path=no --enable-sdk=no --with-egl=yes --with-glx=no --with-testing=no CFLAGS="-DPLATFORM_ANDROID -I$MOONLIGHT_PREFIX/include $CFLAGS" LDFLAGS="-L$MOONLIGHT_PREFIX/lib $LDFLAGS" CXXFLAGS="-fno-rtti -DPLATFORM_ANDROID -I$MOONLIGHT_PREFIX/include $CFLAGS"
make
make install
