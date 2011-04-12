#!/bin/sh
cairo=system
curl=system
pal=android
alsa=no
pulse=no
sles=yes
ffmpeg=yes
gallium=no
egl=yes
glx=no

PKG_CONFIG_PATH=$MOONLIGHT_PREFIX/lib/pkgconfig
if [ -d /usr/X11/share/aclocal ]; then
  ACLOCAL_FLAGS="-I /usr/X11/share/aclocal"
fi

./autogen.sh --host=arm-linux-androideabi --prefix=$MOONLIGHT_PREFIX --with-manual-mono=yes --with-testing=no --disable-browser-support --with-unwind=no --with-ffmpeg=$ffmpeg --with-alsa=$alsa --with-pulseaudio=$pulse --with-opensles=$sles --with-pal=$pal --with-curl=$curl --with-cairo=$cairo --with-gallium-path=$gallium --enable-sdk=no --with-egl=$egl --with-glx=$glx CFLAGS="-DPLATFORM_ANDROID -I$MOONLIGHT_PREFIX/include $CFLAGS" LDFLAGS="-L$MOONLIGHT_PREFIX/lib $LDFLAGS" CXXFLAGS="-fno-rtti -DPLATFORM_ANDROID -I$MOONLIGHT_PREFIX/include $CFLAGS"
make -j 4
