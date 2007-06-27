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

CAIRO_VERSION=1.4.0
AC_DEFINE(__STDC_CONSTANT_MACROS,[],[To make avformat work with C++])

AC_MSG_CHECKING(cairo 1.4.x)
if pkg-config "cairo >= $CAIRO_VERSION"; then
   AC_MSG_RESULT(found)
else
   echo Cairo 1.4.x development kit is missing
   exit 1
fi

AC_MSG_CHECKING(Gtk+ 2.0)
if pkg-config gtk+-2.0; then
   AC_MSG_RESULT(found)
else
   echo Gtk+ 2.0 development kit is missing
   exit 1
fi

#
# these come from ffmpeg
#
AC_MSG_CHECKING(libavcodec, libavformat libswscale)
if pkg-config libavcodec libavformat libswscale ; then
   AC_MSG_RESULT(found)
else
   echo ffmpeg and its associated libraries are not installed
   echo You need libavcodec, libavformat and libswscale development kits
   exit 1
fi

AC_MSG_CHECKING(Mono)
if pkg-config mono; then
   AC_MSG_RESULT(found)
else
   echo You are missing Mono
   echo I was looking for mono.pc file
fi

AC_MSG_CHECKING(Olive's Silverlight)
if pkg-config silver; then
   AC_MSG_RESULT(found)
else
   echo You are missing Mono project Olive module with Silverlight support
   echo I was looking for silver.pc file
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

AC_MSG_CHECKING(Mozilla PLUGIN pkg-config module name)
mozilla_plugin_pcs="plugin mozilla-plugin firefox-plugin xulrunner-plugin"
for pc in $mozilla_plugin_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_plugin=$pc
		break;
	fi
done

if test -z $mozilla_plugin; then
   echo You need to install the Mozilla PLUGIN development package.
   exit 1
fi

XPIDL=`pkg-config --variable=libdir $mozilla_xpcom`/xpidl
XPFLAGS=-I`pkg-config --variable=includedir $mozilla_xpcom`

AC_SUBST(XPIDL)
AC_SUBST(XPFLAGS)

MOON_PKGS="cairo >= $CAIRO_VERSION gtk+-2.0 pango libavcodec libavformat gthread-2.0 libswscale alsa"
MOON_CFLAGS=`pkg-config --cflags $MOON_PKGS`
MOON_LIBS=`pkg-config --libs $MOON_PKGS`

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

PLUGIN_CFLAGS=`pkg-config --cflags gtk+-2.0 $mozilla_nspr $mozilla_xpcom $mozilla_plugin $MOON_PKGS mono`
PLUGIN_LIBS=`pkg-config --libs gtk+-2.0 $mozilla_nspr $mozilla_xpcom $mozilla_plugin $MOON_PKGS mono`
PLUGIN_CFLAGS="$PLUGIN_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""
echo THIS IS IT:
echo $PLUGIN_CFLAGS
AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)


#CFLAGS="$CFLAGS `pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#CXXFLAGS="$CXXFLAGS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#LIBS="$LIBS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"

#AC_SUBST(CFLAGS)
#AC_SUBST(CXXFLAGS)
#AC_SUBST(LIBS)

AC_OUTPUT([
moon.pc
Makefile
src/Makefile
gtk/Makefile
man/Makefile
gtk/gtksilver.pc
tools/Makefile
tools/mopen/Makefile
tools/svg2xaml/Makefile
plugin/Makefile
plugin/test/Makefile
plugin/samples/Makefile
])
