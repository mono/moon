AC_INIT(README)
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(moon,0.1)
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG

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

AC_MSG_CHECKING(Mozilla NSPR pkg-config module name)
mozilla_nspr_pcs="nspr mozilla-nspr firefox-nspr xulrunner-nspr seamonkey-nspr"
for pc in $mozilla_nspr_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_nspr=$pc
		break;
	fi
done

if test -z $mozilla_nspr; then
   echo You need to install the Mozilla NSPR development package.
   exit 1
fi

AC_MSG_CHECKING(Mozilla NSS pkg-config module name)
mozilla_nss_pcs="nss mozilla-nss firefox-nss xulrunner-nss seamonkey-nss"
for pc in $mozilla_nss_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_nss=$pc
		break;
	fi
done

if test -z $mozilla_nss; then
   echo You need to install the Mozilla NSS development package.
   exit 1
fi

AC_MSG_CHECKING(Mozilla XPCOM pkg-config module name)
mozilla_xpcom_pcs="xpcom mozilla-xpcom firefox-xpcom xulrunner-xpcom"
for pc in $mozilla_xpcom_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_xpcom=$pc
		break;
	fi
done

if test -z $mozilla_xpcom; then
   echo You need to install the Mozilla XPCOM development package.
   exit 1
fi

PLUGIN_CFLAGS=`pkg-config --cflags gtk+-2.0 $mozilla_nspr $mozilla_xpcom`
PLUGIN_LIBS=`pkg-config --libs gtk+-2.0 $mozilla_nspr $mozilla_xpcom`

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

XPIDL=`pkg-config --variable=libdir $mozilla_xpcom`/xpidl
XPFLAGS=-I`pkg-config --variable=includedir $mozilla_xpcom`

AC_SUBST(XPIDL)
AC_SUBST(XPFLAGS)

MOON_CFLAGS=`pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa`
MOON_LIBS=`pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa`

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

#CFLAGS="$CFLAGS `pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#CXXFLAGS="$CXXFLAGS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#LIBS="$LIBS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"

#AC_SUBST(CFLAGS)
#AC_SUBST(CXXFLAGS)
#AC_SUBST(LIBS)

AC_OUTPUT([
Makefile
src/Makefile
plugin/Makefile
plugin/test/Makefile
])
