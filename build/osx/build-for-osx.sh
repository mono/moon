#!/bin/sh
./autogen.sh --prefix=/opt/moonlight-osx --with-pal=cocoa --with-cgl=yes --with-vda=yes --with-curl=system \
	--enable-xlib=no --enable-trace=no --enable-svg=no --enable-png=yes --enable-ft=no --enable-fc=no --enable-gobject=no --enable-xlib-xrender=no $DARWOON_CONFIGURE_HOST
