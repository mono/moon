AC_INIT(README)
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(moon,0.4)
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# from berkman the genius
changequote(,)dnl
LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl

PKG_PROG_PKG_CONFIG

AC_DEFINE(__STDC_CONSTANT_MACROS,[],[To make avformat work with C++])

dnl *************************************
dnl *** Checks for large file support ***
dnl *************************************
AC_SYS_LARGEFILE
AC_CACHE_CHECK([for _LARGEFILE64_SOURCE value needed for large files], largefile64_source,
[
   AC_TRY_COMPILE([
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>
   ],[
      int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
   ],[
      largefile64_source="no"
   ],[
      AC_TRY_COMPILE([
         #define _LARGEFILE64_SOURCE 1
         #include <sys/types.h>
         #include <sys/stat.h>
         #include <fcntl.h>
      ],[
         int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
      ],[
         largefile64_source="yes"
      ],[
         largefile64_source="undefined"
      ])
   ])
])

if test "x$largefile64_source" = "xyes"; then
   LARGEFILE_CFLAGS="-D_LARGEFILE64_SOURCE=1"
elif test "x$largefile64_source" = "xundefined"; then
   AC_DEFINE(O_LARGEFILE, 0, [Define to 0 if your system does not have the O_LARGEFILE flag])
fi

if test -n "$ac_cv_sys_large_files" -a "x$ac_cv_sys_large_files" != "xno"; then
   LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_LARGE_FILES=1"
fi

if test "x$ac_cv_sys_file_offset_bits" != "xno"; then
   LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
fi

#
# Debugging support
#

DEBUG=yes
AC_ARG_WITH(debug, [ --with-debug=yes,no.   If you want to enable debug support (default=yes)],[
	if test x$with_debug = xyes; then
		DEBUG=yes
	fi
	if test x$with_debug = xno; then
		DEBUG=no
	fi
])
if test x$DEBUG = xyes; then
AC_DEFINE([DEBUG], [1], [Include debugging support])
fi

#
# Check for ffmpeg
#

FFMPEG=yes
AC_ARG_WITH(ffmpeg,[ --with-ffmpeg=yes,no   If you want to enable support for ffmpeg ],[
	if test x$with_ffmpeg = xyes; then
		FFMPEG=yes
	fi
	if test x$with_ffmpeg = xno; then
		FFMPEG=no
	fi
])

if test x$FFMPEG = xyes; then
AC_DEFINE([INCLUDE_FFMPEG], [1], [Include support for ffmpeg])
fi
AM_CONDITIONAL(INCLUDE_FFMPEG, test x$FFMPEG = xyes)

#
# Check for cairo "extensions"
#

# hackish but I couldn't get AC_EGREP_HEADER to work with our embedded cairo :(
if cat cairo/src/cairo.h | grep CAIRO_LINE_CAP_TRIANGLE >/dev/null 2>&1; then
	AC_DEFINE(HAVE_CAIRO_LINE_CAP_TRIANGLE,,Define if cairo has CAIRO_LINE_CAP_TRIANGLE as a cairo_line_cap_t enum member)
fi

#
# Check for mono
#

MONO_RUNTIME=yes
AC_ARG_WITH(mono-runtime,[ --with-mono-runtime=yes,no   If you want to enable support for Silverlight 2.0],[
	if test x$with_mono_runtime = xyes; then
	      MONO_RUNTIME=yes
	fi
	if test x$with_mono_runtime = xno; then
	      MONO_RUNTIME=no
	fi
])

if test $MONO_RUNTIME = yes; then
AC_DEFINE([INCLUDE_MONO_RUNTIME], [1], [Include the mono runtime in the build (and enable Silverlight 2.0 plugin usage)])
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

ac_configure_args="$ac_configure_args --with-pic=yes --disable-pdf --disable-svg --disable-ps --disable-png"
AC_CONFIG_SUBDIRS([pixman cairo])

AC_MSG_CHECKING(Gtk+ 2.0)
if pkg-config gtk+-2.0; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([Gtk+ 2.0 development kit is missing])
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
if test x$FFMPEG = xyes; then
	AC_MSG_CHECKING(libavcodec, libavformat libswscale)
	if pkg-config libavcodec libavformat libswscale ; then
	   AC_MSG_RESULT(found)
	else
	   AC_MSG_ERROR([ffmpeg and its associated libraries are not installed. You need libavcodec, libavformat and libswscale development kits])
	fi

	avutil_libdir="$(pkg-config --variable=libdir libavutil)"
	swscale_libdir="$(pkg-config --variable=libdir libswscale)"
	avcodec_libdir="$(pkg-config --variable=libdir libavcodec)"
	avformat_libdir="$(pkg-config --variable=libdir libavformat)"

	AC_SUBST([avutil_libdir])
	AC_SUBST([swscale_libdir])
	AC_SUBST([avcodec_libdir])
	AC_SUBST([avformat_libdir])
fi # ffmpeg

mono_libdir="$(pkg-config --variable=libdir mono)"
mono_dlldir="$(pkg-config --variable=prefix mono)/lib"
mono_sysconfdir="$(pkg-config --variable=sysconfdir mono)"

AC_SUBST([mono_libdir])
AC_SUBST([mono_dlldir])
AC_SUBST([mono_sysconfdir])

AC_MSG_CHECKING(alsa)
if pkg-config alsa ; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([alsa development kit is missing])
fi

#
# FontConfig and FreeType2 are used directly by the text rendering code (instead of going thru pango)
#
AC_MSG_CHECKING(freetype2)
if pkg-config freetype2; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([freetype2 development kit is missing])
fi

AC_MSG_CHECKING(fontconfig)
if pkg-config fontconfig; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([fontconfig development kit is missing.])
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
   AC_MSG_ERROR([You are missing Mono.  I was looking for the mono.pc file])
fi

AC_MSG_CHECKING(Olive's Silverlight)
if pkg-config silver silverdesktop; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([You are missing Mono project Olive module with Silverlight support. I was looking for the silver.pc and silverdesktop.pc files])
fi

AC_MSG_CHECKING(gtk-sharp-2.0)
if pkg-config gtk-sharp-2.0; then
   AC_MSG_RESULT(found)
else
   AC_MSG_ERROR([gtk-sharp bindings is missing])
fi

AC_MSG_CHECKING(rsvg-sharp-2.0)
if pkg-config rsvg-sharp-2.0; then
   AC_MSG_RESULT(found)
   RSVG_SHARP="rsvg-sharp-2.0"
else
   AC_MSG_RESULT(not found)
   AC_MSG_CHECKING(rsvg2-sharp-2.0)
   if pkg-config rsvg2-sharp-2.0; then
      AC_MSG_RESULT(found)
      RSVG_SHARP="rsvg2-sharp-2.0"
   else
      AC_MSG_ERROR([rsvg-sharp bindings is missing])
   fi
fi

AC_SUBST(RSVG_SHARP)

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
   AC_MSG_ERROR([You need to install the Mozilla NSPR development package.])
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
   AC_MSG_ERROR([You need to install the Mozilla NSS development package.])
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
   AC_MSG_ERROR([You need to install the Mozilla XPCOM development package.])
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
   AC_MSG_ERROR([You need to install the Mozilla PLUGIN development package.])
fi

XPIDL=`pkg-config --variable=libdir $mozilla_xpcom`/xpidl
XPFLAGS=-I`pkg-config --variable=includedir $mozilla_xpcom`

AC_SUBST(XPIDL)
AC_SUBST(XPFLAGS)

HAVE_MMX=no
AC_COMPILE_IFELSE([
int main () {
        int i = 0;
        int j = -1;
        __asm__ __volatile__ (
                "movd (%0), %%mm1;"
                "movd %%mm1, (%1);"
                : : "r" (&i), "r" (&j));
        return j;
}], HAVE_MMX=yes)

if test x$HAVE_MMX = xyes; then
	AC_DEFINE(HAVE_MMX, 1, [MMX support])
fi

HAVE_SSE2=no
AC_COMPILE_IFELSE([
#include <stdio.h>
#include <string.h>
int main () {
	char buffer[[128]];
	int j = 1;

	memset(buffer, 0x0, 128);
        __asm__ __volatile__ (
                "movdqu (%0), %%xmm1;"
                "movd %%xmm1, (%1);"
                : : "r" (buffer), "r" (&j));
        return (int)j;
}], HAVE_SSE2=yes)

if test x$HAVE_SSE2 = xyes; then
	AC_DEFINE(HAVE_SSE2, 1, [SSE2 support])
fi

if test x$FFMPEG = xyes; then
FFMPEG_PKGS="libswscale libavcodec libavformat"
else
FFMPEG_PKGS=""
fi

CAIRO_CFLAGS='-I$(top_srcdir)/cairo/src'
CAIRO_LIBS='$(top_builddir)/cairo/src/libcairo.la'

MOON_PKGS="gtk+-2.0 pango gthread-2.0 alsa freetype2 fontconfig $RANDR_PKG"
MOON_CFLAGS="$LARGEFILE_CFLAGS $CAIRO_CFLAGS $(pkg-config --cflags $MOON_PKGS $FFMPEG_PKGS)"
MOON_LIBS="$CAIRO_LIBS $(pkg-config --libs $MOON_PKGS)"

if test $user_plugin != yes; then
MOON_LIBS="$MOON_LIBS $(pkg-config --libs $FFMPEG_PKGS)"
fi

# Remove glitz from the link flags (should fix Bug 338506)
MOON_LIBS="$(echo $MOON_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

PLUGIN_CFLAGS=`pkg-config --cflags gtk+-2.0 $mozilla_nspr $mozilla_xpcom $mozilla_plugin $MOON_PKGS $MONO_PKGS`
PLUGIN_LIBS=`pkg-config --libs gtk+-2.0 $MOON_PKGS $MONO_PKGS`
PLUGIN_CFLAGS="$PLUGIN_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""

# Remove glitz from the link flags (should fix Bug 338506)
PLUGIN_LIBS="$(echo $PLUGIN_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

MOON_PROG_LIBS=`pkg-config --libs $MOON_PKGS $MONO_PKGS $FFMPEG_PKGS`
AC_SUBST(MOON_PROG_LIBS)

if test x$DEBUG = xyes; then
	DEBUG_OPTIONS="-fno-inline -g -fno-inline-functions"
	CFLAGS=`echo $CFLAGS | sed 's/-O2//'`
	CFLAGS="$DEBUG_OPTIONS $CFLAGS"
	CXXFLAGS=`echo $CXXFLAGS | sed 's/-O2//'`
	CXXFLAGS="$DEBUG_OPTIONS $CXXFLAGS"
fi

#CFLAGS="$CFLAGS `pkg-config --cflags gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#CXXFLAGS="$CXXFLAGS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"
#LIBS="$LIBS `pkg-config --libs gtk+-2.0 libavcodec libavformat gthread-2.0 libswscale alsa $mozilla_nspr $mozilla_nss`"

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
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
tools/xaml2html/Makefile
plugin/Makefile
plugin/install.js
plugin/test/Makefile
plugin/samples/Makefile
])

cat <<EOF
Moonlight configuration:

	Support for 1.0: yes
	Support for 2.0: $MONO_RUNTIME
	Support for ffmpeg: $FFMPEG
	Support for mmx: $HAVE_MMX
	Support for sse2: $HAVE_SSE2

EOF
