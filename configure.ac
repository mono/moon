AC_INIT([moon],[0.7])
AC_CONFIG_SRCDIR(README)
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([tar-ustar])
AM_MAINTAINER_MODE

AC_PROG_CXX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
DOLT

PKG_PROG_PKG_CONFIG

AC_DEFINE(__STDC_CONSTANT_MACROS,[],[To make avformat work with C++])
AC_DEFINE(__STDC_LIMIT_MACROS,[],[To get limits of specified-width integer types])

AC_SEARCH_LIBS(clock_gettime,rt)

dnl from berkman the genius
changequote(,)dnl
LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl

dnl *****************************
dnl *** Checks for zlib       ***
dnl *****************************
AC_CHECK_HEADERS(zlib.h)
AC_CHECK_LIB(z, inflate, ZLIB="-lz")

dnl
dnl Pthread checks
dnl
PTHREAD_CHECK()

dnl
dnl Pipeline checks
dnl
AC_ARG_WITH(ffmpeg,[  --with-ffmpeg=yes,no   If you want to enable support for ffmpeg],[],[with_ffmpeg=yes])

if test x$with_ffmpeg = xyes; then
  if pkg-config --exists libavutil libavcodec; then
    AC_DEFINE([INCLUDE_FFMPEG],[1],[Include support for ffmpeg])
    PKG_CHECK_MODULES(FFMPEG,[libavutil libavcodec])
    save_CFLAGS=$CFLAGS
    CFLAGS="$FFMPEG_CFLAGS $CFLAGS"
    AC_CHECK_HEADERS([libavcodec/avcodec.h])
    CFLAGS=$save_CFLAGS
  else
    with_ffmpeg=no
    ffmpeg_warning="(WARNING: AUDIO AND VIDEO WON'T WORK)"
  fi
fi
AM_CONDITIONAL(INCLUDE_FFMPEG,test x$with_ffmpeg = xyes)

dnl
dnl Mozilla checks
dnl
with_mozilla=no

AC_ARG_WITH(ff3,[  --with-ff3=no,yes  If you want to enable the xulrunner 1.9 (Firefox 3) bridge],[],[with_ff3=yes])
if test x$with_ff3 = xyes; then
  FF3_MODULES="libxul-unstable mozilla-plugin mozilla-js"

  PKG_CHECK_EXISTS($FF3_MODULES,[with_ff3=yes],[with_ff3=no])

  if test x$with_ff3 = xyes; then
    AC_DEFINE([HAVE_GECKO_1_9],[1],[Gecko 1.9 support])
    PKG_CHECK_MODULES(FF3, [$FF3_MODULES])
    dnl Strip out problem libraries (should already be in process space)
    FF3_LIBS="$(echo $FF3_LIBS | sed -e 's/-lmozjs\|-lplds4\|-lplc4\|-lnspr4//g')"
  fi
fi
AM_CONDITIONAL(HAVE_GECKO_1_9,test x$with_ff3 = xyes)

AC_ARG_WITH(ff2,[  --with-ff2=no,yes  If you want to enable the xulrunner 1.8.1 (Firefox 2)],[],[with_ff2=yes])
if test x$with_ff2 = xyes; then
  mozilla_xpcom="libxul-missing"
  mozilla_xpcom_pcs="xpcom mozilla-xpcom firefox-xpcom xulrunner-xpcom"
  for pc in $mozilla_xpcom_pcs; do
    PKG_CHECK_EXISTS($pc,[mozilla_xpcom=$pc])
  done
  mozilla_plugin="plugin-missing"
  mozilla_plugin_pcs="plugin firefox-plugin xulrunner-plugin"
  for pc in $mozilla_plugin_pcs; do
    PKG_CHECK_EXISTS($pc,[mozilla_plugin=$pc])
  done

  if test $mozilla_xpcom = "libxul-missing" -o $mozilla_plugin = "plugin-missing"; then
    with_ff2=no
  else
    PKG_CHECK_MODULES(FF2,[$mozilla_xpcom $mozilla_plugin])
    dnl Strip out problem libraries (should already be in process space)
    FF2_LIBS="$(echo $FF2_LIBS | sed -e 's/-lmozjs\|-lplds4\|-lplc4\|-lnspr4//g')"
  fi
fi
AM_CONDITIONAL(HAVE_GECKO_1_8,test x$with_ff2 = xyes)

if test x$with_ff2 = xyes; then
  with_mozilla=yes
  MIN_FIREFOX_VERSION="1.5"
  if test x$with_ff3 = xyes; then
    MAX_FIREFOX_VERSION="3.0.*"
  else
    MAX_FIREFOX_VERSION="2.0.0.*"
  fi
else
  if test x$with_ff3 = xyes; then
    with_mozilla=yes
    MIN_FIREFOX_VERSION="2.9.*"
    MAX_FIREFOX_VERSION="3.0.*"
  fi
fi
AC_SUBST([MIN_FIREFOX_VERSION])
AC_SUBST([MAX_FIREFOX_VERSION])
AM_CONDITIONAL(HAVE_MOZILLA, test x$with_mozilla = xyes)

dnl
dnl Mono (Silverlight 2.0) checks
dnl
AC_ARG_WITH(mono,[  --with-mono=yes,no   If you want to enable support for Silverlight 2.0],[],[with_mono=no])
if test x$with_mono = xyes; then
  if pkg-config mono; then
    PKG_CHECK_MODULES(MONO,mono)
    if pkg-config --variable=Libraries smcs; then
        PKG_CHECK_MODULES(SMCS,smcs,
      		    [SMCS_ASM=`pkg-config --variable=Libraries smcs`
     		     AC_SUBST(SMCS_ASM)])
        rsvg_sharp_pcs="rsvg-sharp-2.0 rsvg2-sharp-2.0"
        for pc in $rsvg_sharp_pcs; do
            PKG_CHECK_EXISTS($pc,[rsvg_sharp=$pc])
        done
        if pkg-config gtk-sharp-2.0 $rsvg_sharp; then
            PKG_CHECK_MODULES(GTKSHARP,gtk-sharp-2.0)
            PKG_CHECK_MODULES(RSVGSHARP,$rsvg_sharp,[
            		    RSVG_SHARP=$rsvg_sharp
          		    AC_SUBST(RSVG_SHARP)])
            AC_DEFINE([SL_2_0],[1],[Enable Silverlight 2.0 support)])
        else
	    with_mono=no
	    mreason="(reason: gtk-sharp-2.0 or rsvg-sharp-2.0 is missing)"
	fi
    else
        with_mono=no
	mreason="(reason: Mono newer than 1.9 not installed)"
    fi
  else
    mreason="(reason: Mono newer than 1.9 not installed)"
  fi
else
    mreason="(reason: disabled at configure time)"
fi
AM_CONDITIONAL(INCLUDE_MONO_RUNTIME,test x$with_mono = xyes)

dnl
dnl Cairo checks
dnl
AC_ARG_WITH(cairo,[  --with-cairo=embedded,system   Enable linking against system cairo ],[],[with_cairo=embedded])
if test x$with_cairo = xembedded; then
  ac_configure_args="$ac_configure_args --with-pic=yes --disable-pdf --disable-svg --disable-ps --disable-png"
  AC_CONFIG_SUBDIRS([pixman cairo])

  # hackish but I couldn't get AC_EGREP_HEADER to work with our embedded cairo :(
  if cat cairo/src/cairo.h | grep CAIRO_LINE_CAP_TRIANGLE >/dev/null 2>&1; then
  	AC_DEFINE(HAVE_CAIRO_LINE_CAP_TRIANGLE,,Define if cairo has CAIRO_LINE_CAP_TRIANGLE as a cairo_line_cap_t enum member)
  fi
  CAIRO_CFLAGS='-I$(top_srcdir)/cairo/src'
  CAIRO_LIBS='$(top_builddir)/cairo/src/libcairo.la'
else
  PKG_CHECK_MODULES(CAIRO,cairo >= 1.6)
fi

dnl
dnl Gtk+ checks
dnl
PKG_CHECK_MODULES(GTK,gtk+-2.0 gthread-2.0)

dnl
dnl xrandr checks
dnl
PKG_CHECK_MODULES(XRANDR,xrandr,
		  [AC_DEFINE([USE_RANDR],[1],[Include support for the XRANDR extension for querying a monitor's refresh rate])],[xrandr_present=no])

dnl
dnl sound checks
dnl
PKG_CHECK_MODULES(ALSA,alsa)

dnl
dnl font checks
dnl
PKG_CHECK_MODULES(FREETYPE2,freetype2)
PKG_CHECK_MODULES(FONTCONFIG,fontconfig)

dnl
dnl testing checks
dnl
with_testing=yes
if test x$with_ff3 = xyes; then
  PKG_CHECK_MODULES(XULRUNNER,[mozilla-gtkmozembed mozilla-js],,
		  [with_testing=no])
elif test x$with_ff2 = xyes; then
  PKG_CHECK_MODULES(XULRUNNER,xulrunner-gtkmozembed,,
		  [with_testing=no])
fi

PKG_CHECK_MODULES(XTST,xtst >= 1.0,,
		  [with_testing=no])
PKG_CHECK_MODULES(IMAGEMAGICK,ImageMagick++ >= 6.3.5,,
		  [with_testing=no])
PKG_CHECK_MODULES(DBUS_GLIB,dbus-glib-1 >= 0.60,,
		  [with_testing=no])
PKG_CHECK_MODULES(NDESK_DBUS,ndesk-dbus-1.0 >= 0.6.0,,
		  [with_testing=no])
AM_CONDITIONAL(INCLUDE_TESTING,test x$with_testing = xyes)

MS_DRTLIST=none
# Look to see if the MS tests are installed
if test -f $PWD/../moonlight-ms/tests/port/drop1030/built/drtlist.xml; then
   MS_DRTLIST=$PWD/../moonlight-ms/tests/port/drop1030/built/drtlist.xml
   AC_SUBST(MS_DRTLIST)
elif test -f $PWD/../../extras/moonlight-ms/tests/port/drop1030/built/drtlist.xml; then
   MS_DRTLIST=$PWD/../../extras/moonlight-ms/tests/port/drop1030/built/drtlist.xml
   AC_SUBST(MS_DRTLIST)
fi

dnl
dnl mmx/sse2 compiler checks
dnl
AC_COMPILE_IFELSE([
int main () {
        int i = 0;
        int j = -1;
        __asm__ __volatile__ (
                "movd (%0), %%mm1;"
                "movd %%mm1, (%1);"
                : : "r" (&i), "r" (&j));
        return j;
}],AC_DEFINE(HAVE_MMX,1,[MMX support]))

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
}],AC_DEFINE(HAVE_SSE2,1,[SSE2 support]))

dnl
dnl debug checks
dnl
AC_ARG_WITH(debug,[  --with-debug=yes,no.   If you want to enable debug support (default=yes)],[],[with_debug=yes])
if test x$with_debug = xyes; then
  DEBUG_OPTIONS="-fno-inline -g -fno-inline-functions"
  CFLAGS=`echo $CFLAGS | sed 's/-O2//'`
  CFLAGS="$DEBUG_OPTIONS $CFLAGS"
  CXXFLAGS=`echo $CXXFLAGS | sed 's/-O2//'`
  CXXFLAGS="$DEBUG_OPTIONS $CXXFLAGS"
  AC_DEFINE([DEBUG],[1],[Include debugging support])
fi

dnl
dnl compatibility checks
dnl
AC_ARG_WITH(compatibility-bugs,[  --with-compatibility-bugs=yes,no.   If you want to build including Silverlight 1.0-compatible bugs (default=yes)],[],[with_compatibility_bugs=yes])
if test x$with_compatibility_bugs = xyes; then
  CFLAGS="$CXXFLAGS -DCOMPATIBILITY_BUGS=1"
  CXXFLAGS="$CXXFLAGS -DCOMPATIBILITY_BUGS=1"
  AC_DEFINE([COMPATIBILITY_BUGS],[1],[Include compatibility bugs support])
fi

dnl
dnl plugin checks
dnl
AC_ARG_ENABLE(user-plugin,[  --enable-user-plugin    Build the plugin installer],
			  [user_plugin=yes
			   AC_DEFINE([PLUGIN_INSTALL],[1],[Load libraries from ~/.mozilla/plugins and build a Mozilla XPInstall file])
			   avutil_libdir="$(pkg-config --variable=libdir libavutil)"
			   avcodec_libdir="$(pkg-config --variable=libdir libavcodec)"
			   AC_SUBST([avutil_libdir])
			   AC_SUBST([avcodec_libdir])],
			  [user_plugin=no
			   dnl We only want to link against these if we are NOT building
			   dnl the user plugin installer
			   MOON_LIBS="$FFMPEG_LIBS $SWSCALE_LIBS $MONO_LIBS"
			  ])
AM_CONDITIONAL([PLUGIN_INSTALL],[test x$user_plugin = xyes])

if test x$user_plugin = xyes; then
  if test x$with_ff3 = xno -a x$with_ff2 = xno; then
    AC_MSG_ERROR([Building the plugin installer requires Firefox2 or Firefox3 support.])
  fi
  case "$target_os" in
    *linux* )
    TARGET_PLATFORM="Linux"
    ;;
    * )
    AC_MSG_ERROR([Target os $target_os is unknown.
    Please add the appropriate string to configure.ac.
    See http://developer.mozilla.org/en/docs/OS_TARGET])
    ;;
  esac
  
  case "$target_cpu" in
    i*86 )
    TARGET_PLATFORM="$TARGET_PLATFORM"_x86-gcc3
    ;;
    x86_64 )
    TARGET_PLATFORM="$TARGET_PLATFORM"_x86_64-gcc3
    ;;
    powerpc )
    TARGET_PLATFORM="$TARGET_PLATFORM"_ppc-gcc3
    ;;
    sparc64 )
    TARGET_PLATFORM="$TARGET_PLATFORM"_sparc64-gcc3
    ;;
    * )
    AC_MSG_ERROR([Target cpu $target_cpu is unknown.
    Please add the appropriate string to configure.ac.
    See http://developer.mozilla.org/en/docs/XPCOM_ABI])
    ;;
  esac
  AC_SUBST(TARGET_PLATFORM)
fi

dnl *************************************
dnl *** Checks for large file support ***
dnl *************************************
AC_SYS_LARGEFILE
AC_CACHE_CHECK([for _LARGEFILE64_SOURCE value needed for large files],largefile64_source,
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
   AC_DEFINE(O_LARGEFILE,0,[Define to 0 if your system does not have the O_LARGEFILE flag])
fi

if test -n "$ac_cv_sys_large_files" -a "x$ac_cv_sys_large_files" != "xno"; then
   LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_LARGE_FILES=1"
fi

if test "x$ac_cv_sys_file_offset_bits" != "xno"; then
   LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
fi

AGVIEWER_CFLAGS="$GTK_CFLAGS $DBUS_GLIB_CFLAGS $XULRUNNER_CFLAGS"
AGVIEWER_LIBS="$GTK_LIBS $DBUS_GLIB_LIBS $XULRUNNER_LIBS"
AC_SUBST(AGVIEWER_CFLAGS)
AC_SUBST(AGVIEWER_LIBS)

MOON_CFLAGS="$CAIRO_CFLAGS $FREETYPE2_CFLAGS $FONTCONFIG_CFLAGS $GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $SWSCALE_CFLAGS $MONO_CFLAGS"
dnl MOON_LIBS is partially defined above for the user-plugin build
MOON_LIBS="$MOON_LIBS $CAIRO_LIBS $FREETYPE2_LIBS $FONTCONFIG_LIBS $GTK_LIBS $ALSA_LIBS $ZLIB $PTHREAD_LIBS"

# Remove glitz from the link flags (fixes Bug 338506)
MOON_LIBS="$(echo $MOON_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

MOON_EXTRA_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $SWSCALE_CFLAGS"
MOON_EXTRA_LIBS="$GTK_LIBS $ALSA_LIBS $FFMPEG_LIBS $SWSCALE_LIBS"
AC_SUBST(MOON_EXTRA_CFLAGS)
AC_SUBST(MOON_EXTRA_LIBS)

MOON_PROG_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $SWSCALE_CFLAGS -I\$(top_srcdir)/src/"
MOON_PROG_LIBS="$GTK_LIBS $ALSA_LIBS $FFMPEG_LIBS $SWSCALE_LIBS \$(top_builddir)/src/libmoon.la"
AC_SUBST(MOON_PROG_CFLAGS)
AC_SUBST(MOON_PROG_LIBS)

PLUGIN_CFLAGS="$MOON_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""
PLUGIN_LIBS="$MOON_LIBS"

# Remove glitz from the link flags (fixes Bug 338506)
PLUGIN_LIBS="$(echo $PLUGIN_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

if test x$with_ff3 = xyes; then
   MOZILLA_CFLAGS=$FF3_CFLAGS
else
   MOZILLA_CFLAGS=$FF2_CFLAGS
fi
SHOCKER_CFLAGS="$GTK_CFLAGS $DBUS_GLIB_CFLAGS $MOZILLA_CFLAGS $IMAGEMAGICK_CFLAGS $XTST_CFLAGS"
SHOCKER_LIBS="$GTK_LIBS $DBUS_GLIB_LIBS $IMAGEMAGICK_LIBS $XTST_LIBS"
AC_SUBST(SHOCKER_CFLAGS)
AC_SUBST(SHOCKER_LIBS)

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_OUTPUT([
moon.pc
Makefile
src/Makefile
class/Makefile
class/Mono.Moonlight/Makefile
class/System.Windows/Makefile
class/System.Windows.Browser/Makefile
gtk/Makefile
man/Makefile
gtk/gtksilver.pc
data/Makefile
test/Makefile
test/harness/Makefile
test/harness/shocker/Makefile
test/harness/agviewer/Makefile
test/harness/test-runner/Makefile
test/parsertest/Makefile
test/sizes/Makefile
tools/Makefile
tools/mopen/Makefile
tools/mxap/Makefile
tools/xamlg/Makefile
tools/xaml2html/Makefile
tools/respack/Makefile
examples/Makefile
examples/desklet/Makefile
examples/desklet/calculator/Makefile
examples/desklet/calendar/Makefile
examples/desklet/cpumonitor/Makefile
examples/desklet/ruler/Makefile
examples/desklet/glassycalendar/Makefile
examples/desklet/glassyclock/Makefile
examples/desklet/glassymonitor/Makefile
examples/desklet/glassyslides/Makefile
examples/desklet/simpleclock/Makefile
examples/desklet/weather/Makefile
plugin/Makefile
plugin/firefox/Makefile
plugin/firefox/ff2/Makefile
plugin/firefox/ff3/Makefile
plugin/install/Makefile
plugin/install/install.rdf
plugin/test/Makefile
plugin/samples/Makefile
])

cat <<EOF
Moonlight configuration:

	Support for 1.0: yes
	Support for 2.0: $with_mono $mreason
	Support for ffmpeg: $with_ffmpeg $ffmpeg_warning
	Support for running the test harness: $with_testing
	Using cairo: $with_cairo
	Support for Firefox: $with_mozilla
	With Silverlight 1.0 bugs: $with_compatibility_bugs
EOF

if test x$with_mozilla = xyes; then
cat <<EOF
          Support for plugin installer: $user_plugin
          Support for Gecko 1.8 (Firefox 2): $with_ff2
          Support for Gecko 1.9 (Firefox 3): $with_ff3

EOF
else
cat <<EOF

EOF
fi
