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

AC_DEFINE(__STDC_CONSTANT_MACROS,[],[To make avformat work with C++])

MONO_RUNTIME=yes
AC_ARG_WITH(mono-runtime,[ --with-mono-runtime=yes,no   If you want to enable support for Silverlight 1.1],[
	if test x$with_mono_runtime = xyes; then
	      MONO_RUNTIME=yes
	fi
	if test x$with_mono_runtime = xno; then
	      MONO_RUNTIME=no
	fi
])

if test $MONO_RUNTIME = yes; then
AC_DEFINE([INCLUDE_MONO_RUNTIME], [1], [Include the mono runtime in the build (and enable Silverlight 1.1 plugin usage)])
fi
AM_CONDITIONAL(INCLUDE_MONO_RUNTIME, test $MONO_RUNTIME = yes)

AC_ARG_ENABLE(user-plugin, [AS_HELP_STRING([--enable-user-plugin], [Build the plugin as installable in ~/.mozilla/plugins])],
[case "${enable_user_plugin}" in
	yes) user_plugin=yes ;;
	no) user_plugin=no ;;
	*) AC_MSG_ERROR([bad value ${enable_user_plugin} for --enable-user-plugin]) ;;
esac],[user_plugin=no])

if test $user_plugin = yes; then
AC_DEFINE([PLUGIN_INSTALL], [1], [Load libraries from ~/.mozilla/plugins and build a Mozilla XPInstall file])
fi
AM_CONDITIONAL([PLUGIN_INSTALL], [test $user_plugin = yes])

ac_configure_args="$ac_configure_args --with-pic=yes --disable-pdf --disable-svg --disable-ps"
AC_CONFIG_SUBDIRS([pixman cairo])

AC_MSG_CHECKING(Gtk+ 2.0)
if pkg-config gtk+-2.0; then
   AC_MSG_RESULT(found)
else
   echo Gtk+ 2.0 development kit is missing
   exit 1
fi

AC_MSG_CHECKING(XRANDR extension)
if pkg-config xrandr; then
  AC_MSG_RESULT(found)
  AC_DEFINE([USE_XRANDR], [1], [Include support for the XRANDR extension for querying a monitor's refresh rate)])
  RANDR_PKG="xrandr"
else
  AC_MSG_RESULT(not found, using default max fps of 30)
  RANDR_PKG=""
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

AC_MSG_CHECKING(alsa)
if pkg-config alsa ; then
   AC_MSG_RESULT(found)
else
   echo alsa development kit is missing
   exit 1
fi

#
# FontConfig and FreeType2 are used directly by the text rendering code (instead of going thru pango)
#
AC_MSG_CHECKING(freetype2)
if pkg-config freetype2; then
   AC_MSG_RESULT(found)
else
   echo freetype2 development kit is missing
   exit 1
fi

AC_MSG_CHECKING(fontconfig)
if pkg-config fontconfig; then
   AC_MSG_RESULT(found)
else
   echo fontconfig development kit is missing
   exit 1
fi


#
# Enable/Disable linking with Pango as an alternative text rendering engine
# (accessable via environment variable)
#
AC_ARG_WITH(pango-support,[ --with-pango-support=yes,no   Include Pango text rendering support],with_pango_support=yes,with_pango_support=no)
if test x$with_pango_support != xno; then
   	AC_DEFINE(ENABLE_PANGO_SUPPORT, 1, [Include Pango text rendering support])
fi


if test $MONO_RUNTIME = "yes"; then
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

MONO_PKGS="mono"

fi # MONO_RUNTIME

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

CAIRO_CFLAGS='-I$(top_srcdir)/cairo/src'
CAIRO_LIBS='$(top_builddir)/cairo/src/libcairo.la'

MOON_PKGS="gtk+-2.0 pango libavcodec libavformat gthread-2.0 libswscale alsa freetype2 fontconfig $RANDR_PKG"
MOON_CFLAGS="$CAIRO_CFLAGS $(pkg-config --cflags $MOON_PKGS)"
MOON_LIBS="$CAIRO_LIBS $(pkg-config --libs $MOON_PKGS)"

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

PLUGIN_CFLAGS=`pkg-config --cflags gtk+-2.0 $mozilla_nspr $mozilla_xpcom $mozilla_plugin $MOON_PKGS $MONO_PKGS`
PLUGIN_LIBS=`pkg-config --libs gtk+-2.0 $mozilla_nspr $mozilla_xpcom $mozilla_plugin $MOON_PKGS $MONO_PKGS`
PLUGIN_CFLAGS="$PLUGIN_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""
AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

MOON_PROG_LIBS=`pkg-config --libs $MOON_PKGS $MONO_PKGS`
AC_SUBST(MOON_PROG_LIBS)

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
test/harness/Makefile
tools/Makefile
tools/mopen/Makefile
tools/svg2xaml/Makefile
tools/xamlg/Makefile
plugin/Makefile
plugin/test/Makefile
plugin/samples/Makefile
])

cat <<EOF
Moonlight configuration:

	Support for 1.0: yes
	Support for 1.1: $MONO_RUNTIME

EOF
