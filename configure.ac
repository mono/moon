AC_INIT([moon],[1.9])
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
AC_PATH_PROG([GACUTIL], [gacutil])
AC_PATH_PROG([MONO], [mono])

AC_DEFINE(__STDC_CONSTANT_MACROS,[],[To make avformat work with C++])
AC_DEFINE(__STDC_LIMIT_MACROS,[],[To get limits of specified-width integer types])

AC_SEARCH_LIBS(clock_gettime,rt)

changequote(,)dnl
dnl LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl

AC_CHECK_HEADERS(sys/time.h)

dnl *****************************
dnl *** Checks for zlib       ***
dnl *****************************
AC_CHECK_HEADERS(zlib.h)
AC_CHECK_LIB(z, inflate, ZLIB="-lz")

dnl
dnl Pthread checks
dnl
PTHREAD_CHECK()

dnl add -fstack-protector-all when compiling with gcc
if test "x$GCC" = "xyes" ; then
	CFLAGS="-fstack-protector-all $CFLAGS"
	CXXFLAGS="-fstack-protector-all $CXXFLAGS"
fi

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
    ffmpeg_reason="(reason: could not find libavutil and libavcodec packages)"
  fi
fi
AM_CONDITIONAL(INCLUDE_FFMPEG,test x$with_ffmpeg = xyes)

if test x$with_ffmpeg = no; then
  media_warning = "WARNING: No media decoding backends configured.  AUDIO AND VIDEO WON'T WORK"
fi

dnl
dnl Mozilla checks
dnl
with_mozilla=no

AC_ARG_WITH(ff3,[  --with-ff3=no,yes  If you want to enable the xulrunner 1.9 (Firefox 3) bridge],[],[with_ff3=yes])
if test x$with_ff3 = xyes; then
  FF3_MODULES="libxul-unstable mozilla-plugin mozilla-js"

  PKG_CHECK_EXISTS($FF3_MODULES,
		   [with_ff3=yes],
		   [with_ff3=no
		    ff3_reason="(reason: missing FF3 development packages)"])

  if test x$with_ff3 = xyes; then
    AC_DEFINE([HAVE_GECKO_1_9],[1],[Gecko 1.9 support])
    PKG_CHECK_MODULES(FF3, [$FF3_MODULES glib-2.0])
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
    ff2_reason="(reason: missing FF2 development packages)"
  else
    PKG_CHECK_MODULES(FF2,[$mozilla_xpcom $mozilla_plugin glib-2.0])
    dnl Strip out problem libraries (should already be in process space)
    FF2_LIBS="$(echo $FF2_LIBS | sed -e 's/-lmozjs\|-lplds4\|-lplc4\|-lnspr4//g')"
  fi
fi
AM_CONDITIONAL(HAVE_GECKO_1_8,test x$with_ff2 = xyes)

if test x$with_ff2 = xyes; then
  with_mozilla=yes
  MIN_FIREFOX_VERSION="1.5"
  if test x$with_ff3 = xyes; then
    MAX_FIREFOX_VERSION="3.1.*"
  else
    MAX_FIREFOX_VERSION="2.0.0.*"
  fi
else
  if test x$with_ff3 = xyes; then
    with_mozilla=yes
    MIN_FIREFOX_VERSION="2.9.*"
    MAX_FIREFOX_VERSION="3.1.*"
  fi
fi
AC_SUBST([MIN_FIREFOX_VERSION])
AC_SUBST([MAX_FIREFOX_VERSION])
AM_CONDITIONAL(HAVE_MOZILLA, test x$with_mozilla = xyes)

dnl
dnl Mono (both 2.1 and 3.0 profiles) checks
dnl
managed_code=yes
desktop_assemblies=yes
browser_assemblies=yes

AC_ARG_WITH(managed,
	    [ --with-managed=yes/no/browser/desktop   If you want to enable support for managed code.  Default is "yes"],
	    [case "$with_managed" in
    	     yes)
	     ;;
	     browser)
		 desktop_assemblies=no
	     ;;
	     desktop)
		 browser_assemblies=no
	     ;;
	     no)
	     	 managed_code=no
		 desktop_assemblies=no
		 browser_assemblies=no
	     ;;
	     *)
	         AC_MSG_ERROR([Unknown value for --with-managed.  Please specify "yes", "no", "desktop", or "browser"])
	     ;;
	     esac],[])

if test x$managed_code = xyes; then
  if pkg-config mono; then
    PKG_CHECK_MODULES(MONO,mono >= 2.5)
    # browser assembly specific checks
    #
    AC_ARG_WITH(mcspath, [ --with-mcspath=<path>], [],[with_mcspath=../mcs])

    if test "x$with_mcspath" = "xno"; then
      AC_ERROR(You need to set the path to mcs)
    fi
    if test ! -e "$with_mcspath" ; then
      AC_ERROR(mcs_path doesn't exist)
    fi

    MCS_PATH=$(cd "$with_mcspath" && pwd)
    AC_SUBST(MCS_PATH)

    # desktop assembly specific checks
    #
    # desktop assemblies/tools require gtk-sharp-2.0 and an
    # rsvg-sharp-2.0 package (name varies with distro)
    #
    if test x$desktop_assemblies = xyes; then
        rsvg_sharp_pcs="rsvg-sharp-2.0 rsvg2-sharp-2.0"
        for pc in $rsvg_sharp_pcs; do
            PKG_CHECK_EXISTS($pc,[rsvg_sharp=$pc])
        done
        if pkg-config gtk-sharp-2.0 $rsvg_sharp; then
            PKG_CHECK_MODULES(GTKSHARP,gtk-sharp-2.0)
            PKG_CHECK_MODULES(RSVGSHARP,$rsvg_sharp,[
            		    RSVG_SHARP=$rsvg_sharp
          		    AC_SUBST(RSVG_SHARP)])
        else
	    desktop_assemblies=no
	    desktop_reason="(reason: gtk-sharp-2.0 or rsvg-sharp-2.0 is missing)"
	fi
    fi

    if test x$desktop_assemblies = xno -a x$browser_assemblies = xno; then
	# if browser and desktop failed to configure, disable managed code
	# entirely
        managed_code=no
	managed_reason="(reason: desktop and browser assemblies failed to configure)"
    else
	# we need 2 AC_DEFINES to deal with the case where the src/
	# dir might need to have managed code support enabled (with
	# --enable-desktop-assemblies), but the plugin/ dir might need
	# to have it disabled (for --disable-desktop-assemblies).
	AC_DEFINE([SL_2_0],[1],[Enable Silverlight 2.0 support in the runtime])
	if test $browser_assemblies = yes; then
	    AC_DEFINE([PLUGIN_SL_2_0],[1],[Enable Silverlight 2.0 support for the plugin])
	fi
    fi

  else
    # mono.pc wasn't found at all
    managed_code=no
    browser_assemblies=no
    desktop_assemblies=no

    managed_reason="(reason: Mono >= 2.2 not installed)"
    browser_reason="(reason: Mono >= 2.2 not installed)"
    desktop_reason="(reason: Mono >= 2.2 not installed)"
  fi

fi
AM_CONDITIONAL(INCLUDE_MANAGED_CODE,test x$managed_code = xyes)
AM_CONDITIONAL(INCLUDE_BROWSER_MANAGED_CODE,test x$browser_assemblies = xyes)
AM_CONDITIONAL(INCLUDE_DESKTOP_MANAGED_CODE,test x$desktop_assemblies = xyes)

if test x$managed_code = xyes; then
	SL_PROFILE=2.0
else
	SL_PROFILE=1.0
fi
AC_SUBST([SL_PROFILE])

dnl 
dnl glib
dnl
PKG_CHECK_MODULES(GLIB,glib-2.0)

dnl
dnl Cairo checks
dnl
AC_ARG_WITH(cairo,[  --with-cairo=embedded,system   Enable linking against system cairo ],[],[with_cairo=embedded])
if test x$with_cairo = xembedded; then
  ac_configure_args="$ac_configure_args --with-pic=yes --disable-pdf --disable-svg --disable-ps --disable-png --disable-xcb"
  AC_CONFIG_SUBDIRS([pixman cairo])

  # hackish but I couldn't get AC_EGREP_HEADER to work with our embedded cairo :(
  if grep CAIRO_LINE_CAP_TRIANGLE $srcdir/cairo/src/cairo.h >/dev/null 2>&1; then
  	AC_DEFINE(HAVE_CAIRO_LINE_CAP_TRIANGLE,,[Define if cairo has CAIRO_LINE_CAP_TRIANGLE as a cairo_line_cap_t enum member])
  fi
  CAIRO_CFLAGS='-I$(top_srcdir)/cairo/src'
  CAIRO_LIBS='$(top_builddir)/cairo/src/libcairo.la'
else
  PKG_CHECK_MODULES(CAIRO,cairo >= 1.8)
fi

dnl
dnl Expat checks
dnl
AC_CHECK_HEADER(expat.h)

AC_MSG_CHECKING(for libexpat)
LDFLAGS_save="$LDFLAGS"
LIBS_save="$LIBS"
LDFLAGS="$LDFLAGS -lexpat"
LIBS="$LIBS -lexpat"
AC_TRY_LINK_FUNC(XML_ParserCreateNS, have_expat_libs="yes", have_expat_libs="no")
LDFLAGS="$LDFLAGS_save"
LIBS="$LIBS_save"

if test "$have_expat_libs" = "no"; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([Failed to find suitable libexpat libraries. Please make sure that libexpat-2.0.x is installed.])
else
  AC_MSG_RESULT(yes)
  EXPAT_LIBS="-lexpat"
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
AC_ARG_WITH(alsa,[  --with-alsa=yes,no   If you want to enable alsa sound support],[],[with_alsa=yes])
if test x$with_alsa = xyes; then
  if pkg-config --exists alsa; then
    AC_DEFINE([INCLUDE_ALSA],[1],[Include alsa sound support])
    PKG_CHECK_MODULES(ALSA,alsa)
  else
    with_alsa=no
    alsa_reason="(reason: could not find alsa development package)"
  fi
else
  alsa_reason="(reason: disabled at configure time)"
fi
AM_CONDITIONAL(INCLUDE_ALSA,test x$with_alsa = xyes)

AC_ARG_WITH(pulseaudio,[  --with-pulseaudio=yes,no   If you want to enable pulseaudio sound support],[],[with_pulseaudio=yes])
if test x$with_pulseaudio = xyes; then
  if pkg-config --exists libpulse; then
    AC_DEFINE([INCLUDE_PULSEAUDIO],[1],[Include pulseaudio sound support])
    PKG_CHECK_MODULES(PULSEAUDIO,libpulse)
  else
    with_pulseaudio=no
    pulseaudio_reason="(reason: could not find libpulse development package)"
  fi
else
  pulseaudio_reason="(reason: disabled at configure time)"
fi
AM_CONDITIONAL(INCLUDE_PULSEAUDIO,test x$with_pulseaudio = xyes)

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
		  [with_testing=no
		   testing_reason="(reason: failed to find FF3 development packages)"])
elif test x$with_ff2 = xyes; then
  PKG_CHECK_MODULES(XULRUNNER,xulrunner-gtkmozembed,,
		  [with_testing=no
		   testing_reason="(reason: failed to find FF2 development packages)"])
fi

PKG_CHECK_MODULES(XTST,xtst >= 1.0,,
		  [with_testing=no
		   testing_reason="(reason: failed to find xtst >= 1.0)"])
PKG_CHECK_MODULES(IMAGEMAGICK,ImageMagick++ >= 6.2.8,,
		  [with_testing=no
		   testing_reason="(reason: failed to find ImageMagick++ >= 6.2.8)"])
PKG_CHECK_MODULES(DBUS_GLIB,dbus-glib-1 >= 0.60,,
		  [with_testing=no
		   testing_reason="(reason: failed to find dbus-glib-1 >= 0.60)"])
PKG_CHECK_MODULES(NDESK_DBUS,ndesk-dbus-1.0 >= 0.6.0,,
		  [with_testing=no
		   testing_reason="(reason: failed to find ndesk-dbus-1.0 >= 0.6.0)"])
AM_CONDITIONAL(INCLUDE_TESTING,test x$with_testing = xyes)

dnl
dnl performance tests
dnl 
with_performance=yes
if test x$with_ff3 = xyes; then
  PKG_CHECK_MODULES(XULRUNNER,[mozilla-gtkmozembed mozilla-js],,
		  [with_performance=no
		   performance_reason="(reason: failed to find FF3 development packages)"])
else
  with_performance=no
  performance_reason="(reason: performance suite requires FF3)"
fi
AM_CONDITIONAL(INCLUDE_PERFORMANCE,test x$with_performance = xyes)

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
  if test x$managed_code = xyes; then
    MOON_LIBS="$MOON_LIBS $MONO_LIBS"
  fi
  AC_DEFINE([DEBUG],[1],[Include debugging support])
fi

dnl
dnl sanity checks
dnl
AC_ARG_WITH(sanity-checks,[  --with-sanity-checks=yes,no.   If you want to enable sanity checks (default=no)],[],[with_sanity_checks=no])
if test x$with_sanity_checks = xyes; then
  AC_DEFINE([SANITY],[1],[Include sanity checks])
fi

dnl 
dnl object tracking
dnl
AC_ARG_WITH(object-tracking,[  --with-object-tracking=yes,no.   If you want to enable object tracking (default=no)],[],[with_object_tracking=no])
if test x$with_object_tracking = xyes; then
  AC_DEFINE([OBJECT_TRACKING],[1],[Include object tracking])
fi

dnl
dnl logging
dnl
AC_ARG_WITH(logging,[  --with-logging=yes,no. If you want to enable support for logging with MOONLIGHT_DEBUG (default=yes)],[],[with_logging=yes])
if test x$with_logging = xyes; then
  AC_DEFINE([LOGGING],[1],[Include support for logging with MOONLIGHT_DEBUG])
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
			   MOON_LIBS="$FFMPEG_LIBS $MONO_LIBS"
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
    INSTALL_ARCH=i586
    ;;
    x86_64 )
    TARGET_PLATFORM="$TARGET_PLATFORM"_x86_64-gcc3
    INSTALL_ARCH=x86_64
    ;;
dnl    powerpc )
dnl    TARGET_PLATFORM="$TARGET_PLATFORM"_ppc-gcc3
dnl    INSTALL_ARCH=ppc
dnl    ;;
dnl    sparc64 )
dnl    TARGET_PLATFORM="$TARGET_PLATFORM"_sparc64-gcc3
dnl    INSTALL_ARCH=sparc
dnl    ;;
    * )
    AC_MSG_ERROR([Target cpu $target_cpu is unknown.
    Please add the appropriate string to configure.ac.
    See http://developer.mozilla.org/en/docs/XPCOM_ABI])
    ;;
  esac
  AC_SUBST(TARGET_PLATFORM)
  AC_SUBST([INSTALL_ARCH])
fi

dnl *************************************
dnl *** Checks for large file support ***
dnl *************************************
AC_SYS_LARGEFILE
AC_CACHE_CHECK([whether _LARGEFILE64_SOURCE needs to be defined for large files],moon_cv_largefile64_source,
[
   AC_TRY_COMPILE([
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>
   ],[
      int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
   ],[
      moon_cv_largefile64_source="no"
   ],[
      AC_TRY_COMPILE([
         #define _LARGEFILE64_SOURCE 1
         #include <sys/types.h>
         #include <sys/stat.h>
         #include <fcntl.h>
      ],[
         int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
      ],[
         moon_cv_largefile64_source="yes"
      ],[
         moon_cv_largefile64_source="unknown (large files may not be supported)"
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

dnl *************************************
dnl ***       Codec settings          ***
dnl *************************************
MOONLIGHT_CODEC_OSTYPE="unknown"
MOONLIGHT_CODEC_ARCH="unknown"
# Do we know of a way to detect when the GCC ABI breaks? will it ever again? we'll hardcode this for now
MOONLIGHT_CODEC_GCC_ABI_VERSION="1"
CODECS_SUPPORTED="no"
CODECS_ARCH_SUPPORTED="no"
CODECS_OS_SUPPORTED="no"

case "$host" in
        i*86-*-*)
                MOONLIGHT_CODEC_ARCH=x86
		CODECS_ARCH_SUPPORTED="yes"
                ;;
        x86_64-*-* | amd64-*-*)
                MOONLIGHT_CODEC_ARCH=x64
		CODECS_ARCH_SUPPORTED="yes"
                ;;
esac
case "$host" in
    *-*-*linux*)
                MOONLIGHT_CODEC_OSTYPE=linux
		CODECS_OS_SUPPORTED="yes"
                ;;
esac

if test ${MOONLIGHT_CODEC_ARCH} = unknown; then
        AC_WARN([The codecs have not been configured to build on this architecture yet])
fi
if test ${MOONLIGHT_CODEC_OSTYPE} = unknown; then
        AC_WARN([The codecs have not been configured to build on this operating system yet])
fi

if test x$CODECS_ARCH_SUPPORTED = xyes; then
	if test x$CODECS_OS_SUPPORTED = xyes; then
		CODECS_SUPPORTED="yes"
	fi
fi

AM_CONDITIONAL(CODECS_SUPPORTED,test x$CODECS_SUPPORTED = xyes)

AC_SUBST(MOONLIGHT_CODEC_OSTYPE)
AC_SUBST(MOONLIGHT_CODEC_ARCH)
AC_SUBST(MOONLIGHT_CODEC_GCC_ABI_VERSION)

AGVIEWER_CFLAGS="$GTK_CFLAGS $DBUS_GLIB_CFLAGS $XULRUNNER_CFLAGS"
AGVIEWER_LIBS="$GTK_LIBS $DBUS_GLIB_LIBS $XULRUNNER_LIBS"
AC_SUBST(AGVIEWER_CFLAGS)
AC_SUBST(AGVIEWER_LIBS)

PERF_TOOL_CFLAGS="$GTK_CFLAGS $XULRUNNER_CFLAGS"
PERF_TOOL_LIBS="$GTK_LIBS $XULRUNNER_LIBS"
AC_SUBST(PERF_TOOL_CFLAGS)
AC_SUBST(PERF_TOOL_LIBS)

MOON_CFLAGS="$CAIRO_CFLAGS $FREETYPE2_CFLAGS $FONTCONFIG_CFLAGS $GTK_CFLAGS $ALSA_CFLAGS $PULSEAUDIO_CFLAGS $FFMPEG_CFLAGS $MONO_CFLAGS"
dnl MOON_LIBS is partially defined above for the user-plugin build
MOON_LIBS="$MOON_LIBS $CAIRO_LIBS $FREETYPE2_LIBS $FONTCONFIG_LIBS $GTK_LIBS $ZLIB $PTHREAD_LIBS $EXPAT_LIBS"

# Remove glitz from the link flags (fixes Bug 338506)
MOON_LIBS="$(echo $MOON_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)

MOON_EXTRA_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $PULSEAUDIO_CFLAGS"
MOON_EXTRA_LIBS="$GTK_LIBS $FFMPEG_LIBS"
AC_SUBST(MOON_EXTRA_CFLAGS)
AC_SUBST(MOON_EXTRA_LIBS)

MOON_PROG_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $PULSEAUDIO_CFLAGS $MONO_CFLAGS -I\$(top_srcdir)/src/"
MOON_PROG_LIBS="$GTK_LIBS $FFMPEG_LIBS \$(top_builddir)/src/libmoon.la"
AC_SUBST(MOON_PROG_CFLAGS)
AC_SUBST(MOON_PROG_LIBS)

PLUGIN_CFLAGS="$GTK_CFLAGS $MONO_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""
PLUGIN_LIBS="$GTK_LIBS $MONO_LIBS"

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

LOADER_LIBS="$GLIB_LIBS"
AC_SUBST(LOADER_LIBS)

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
src/codec-version.h
class/Makefile
class/tuning/Makefile
class/tuning/SecurityAttributes/Makefile
class/System.Windows/Makefile
class/System.Windows.Browser/Makefile
class/System.Windows.Controls/Makefile
class/System.Windows.Controls.Data/Makefile
scripts/Makefile
scripts/smcs
gtk/Makefile
man/Makefile
gtk/gtksilver.pc
data/Makefile
test/Makefile
test/2.0/Makefile
test/2.0/moon-unit/Makefile
test/2.0/Mono.Moonlight.UnitTesting/Makefile
test/2.0/Microsoft.Silverlight.Testing/Makefile
test/2.0/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Makefile
test/2.0/Microsoft.SilverlightControls/Makefile
test/2.0/Microsoft.SilverlightControls/Controls.Test.Common/Makefile
test/2.0/Microsoft.SilverlightControls/Controls.Data.Test/Makefile
test/2.0/Microsoft.SilverlightControls/Controls.Extended.Test/Makefile
test/2.0/Microsoft.SilverlightControls/site/Makefile
test/harness/Makefile
test/harness/shocker/Makefile
test/harness/agviewer/Makefile
test/harness/test-runner/Makefile
test/objecttypes/Makefile
test/parsertest/Makefile
test/sizes/Makefile
tools/Makefile
tools/generators/Makefile
tools/mopen/Makefile
tools/mxap/Makefile
tools/munxap/Makefile
tools/unsign/Makefile
tools/xamlg/Makefile
tools/xaml2html/Makefile
tools/respack/Makefile
examples/Makefile
examples/desklet/Makefile
examples/desklet/calculator/Makefile
examples/desklet/cpumonitor/Makefile
examples/desklet/glassyclock/Makefile
examples/desklet/glassymonitor/Makefile
examples/desklet/glassyslides/Makefile
examples/desklet/ruler/Makefile
examples/desklet/simpleclock/Makefile
plugin/Makefile
plugin/firefox/Makefile
plugin/firefox/ff2/Makefile
plugin/firefox/ff3/Makefile
plugin/install/Makefile
plugin/install/install.rdf
plugin/test/Makefile
plugin/samples/Makefile
perf/Makefile
])

# general configuration info
cat <<EOF
Moonlight configuration
=======================

  General configuration:
	Using cairo: $with_cairo
	Test Harness: $with_testing $testing_reason
	Performance Suite: $with_performance $performance_reason
EOF

dnl debug info
cat <<EOF

  Debug configuration:
	Sanity checks: $with_sanity_checks
	Object tracking: $with_object_tracking
	Logging support with MOONLIGHT_DEBUG: $with_logging
EOF

# media info
cat <<EOF

  Media Support: $media_warning
	FFmpeg: $with_ffmpeg $ffmpeg_reason
	Alsa: $with_alsa $alsa_reason
	Pulseaudio: $with_pulseaudio $pulseaudio_reason
EOF

# silverlight version info
cat <<EOF

  Silverlight Support:
	Silverlight 1.0: yes (bug compatibility: $with_compatibility_bugs)
	Silverlight 2.0 (managed code): $managed_code $managed_reason
EOF

if test x$managed_code = xyes; then
cat <<EOF
          Browser plugin assemblies: $browser_assemblies $browser_reason
          Desktop application assemblies: $desktop_assemblies $desktop_reason
          Path to mcs: $with_mcspath
EOF
fi

# plugin installer/bridge info
cat <<EOF

  Browser Support:
	Firefox: $with_mozilla
EOF

if test x$with_mozilla = xyes; then
cat <<EOF
          Plugin Installer (.xpi): $user_plugin
          Gecko 1.8 (Firefox 2): $with_ff2 $ff2_reason
          Gecko 1.9 (Firefox 3): $with_ff3 $ff3_reason
EOF
fi

# blank line
cat <<EOF

EOF
