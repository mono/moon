#!/bin/sh

rm -rf freetype-2.4.4

if [ ! -e freetype-2.4.4.tar.bz2 ]; then
	wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.4.tar.bz2
fi

bzip2 -dc freetype-2.4.4.tar.bz2 | tar xvf -

cd freetype-2.4.4

./configure --prefix=/opt/moonlight-osx CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"
make
make install
