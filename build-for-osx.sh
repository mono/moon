#!/bin/sh
./autogen.sh --prefix=/opt/moonlight-osx --with-pal=cocoa --with-cgl=yes --disable-browser-support --with-curl=system CFLAGS="-arch i386" \
	--enable-xlib=no --enable-trace=no --enable-svg=no --enable-png=yes --enable-ft=no --enable-fc=no --enable-gobject=no --enable-xlib-xrender=no
