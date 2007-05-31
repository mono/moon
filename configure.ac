AC_INIT(README)
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(moon,0.1)
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

LIBS_TO_CHECK="gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa"

if pkg-config gtk+-2.0; then
   echo Gtk+ 2.0 found
else
   echo Gtk+ 2.0 development kit is missing
   exit 1
fi

#
# these come from ffmpeg
#
if pkg-config libavcodec libavformat libswscale ; then
   echo ffmpeg developer installation found
else
   echo ffmpeg and its associated libraries are not installed
   echo You need libavcodec, libavformat and libswscale development kits
   exit 1
fi

if pkg-config alsa; then
   echo Found Alsa
else
   echo You are missing ALSA
   echo looking for the alsa.pc file through pkgconfig.
   exit 1
fi

CFLAGS="$CFLAGS `pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa`"
CXXFLAGS="$CXXFLAGS `pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa`"
LIBS="$LIBS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa`"

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LIBS)

AC_OUTPUT([
Makefile
src/Makefile
])