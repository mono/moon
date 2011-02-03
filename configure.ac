AC_INIT([moonlight], [2.4])

AC_DEFINE([PREVIEW_VERSION], ["10"], "")

AC_CONFIG_SRCDIR(README)
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([1.9 dist-bzip2 tar-ustar no-dist-gzip foreign])
AM_MAINTAINER_MODE

AC_SUBST([ACLOCAL_AMFLAGS], ["-I m4 \${ACLOCAL_FLAGS}"])

AC_PROG_CXX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
DOLT
PKG_PROG_PKG_CONFIG

AC_DEFINE(__STDC_CONSTANT_MACROS, [], [To make avformat work with C++])
AC_DEFINE(__STDC_LIMIT_MACROS, [], [To get limits of specified-width integer types])

AC_SEARCH_LIBS(clock_gettime,rt)
AC_CHECK_HEADERS(sys/time.h)

dnl ********************************************************
dnl *** libiberty.h (included by demangle.h) will define ***
dnl *** basename() unless HAVE_DECL_BASENAME is defined  ***
dnl *** to 1, so check if libc defines it.               ***
dnl ***                                                  ***
dnl *** Note: g++ defines _GNU_SOURCE                    ***
dnl ********************************************************
CFLAGS_save="$CFLAGS"
CFLAGS="$CFLAGS -D_GNU_SOURCE"
AC_CHECK_DECLS([basename])
CFLAGS="$CFLAGS_save"

changequote(,)dnl
dnl LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl

dnl
dnl Dependencies
dnl

PTHREAD_CHECK
MOONLIGHT_CHECK_C_COMPILER
MOONLIGHT_CHECK_LARGE_FILE
MOONLIGHT_CHECK_ZLIB
MOONLIGHT_CHECK_EXPAT

MOONLIGHT_CHECK_FFMPEG
MOONLIGHT_CHECK_CODECS
MOONLIGHT_CHECK_SOUND

MOONLIGHT_CHECK_GLIB
MOONLIGHT_CHECK_GTK
MOONLIGHT_CHECK_XRANDR
MOONLIGHT_CHECK_FONTS
MOONLIGHT_CHECK_CAIRO

MOONLIGHT_CHECK_MONO
MOONLIGHT_CHECK_MOZILLA

MOONLIGHT_CHECK_TESTING
MOONLIGHT_CHECK_EXAMPLES
MOONLIGHT_CHECK_BUILD_CONFIGURATION
MOONLIGHT_CHECK_FIREFOX_XPI
MOONLIGHT_CHECK_CURL
dnl the chrome check must come after curl
MOONLIGHT_CHECK_CHROME_CRX

MOONLIGHT_CHECK_UNWIND

dnl
dnl The silverlight TargetRuntime versions we claim to support

SILVERLIGHT_1_0_TARGETRUNTIME="1.0.30401.0"
SILVERLIGHT_2_0_TARGETRUNTIME="2.0.31005.0"
SILVERLIGHT_3_0_TARGETRUNTIME="3.0.40818.0"
AC_SUBST(SILVERLIGHT_1_0_TARGETRUNTIME)
AC_SUBST(SILVERLIGHT_2_0_TARGETRUNTIME)
AC_SUBST(SILVERLIGHT_3_0_TARGETRUNTIME)

dnl
dnl Export Variables
dnl

AC_PATH_PROG(GACUTIL, gacutil)
AC_PATH_PROG(MONO, mono)
AC_PATH_PROG(ZIP, zip, no)
if test "x$ZIP" = "xno" ; then
        AC_MSG_ERROR([No zip tool found])
fi

PERF_TOOL_CFLAGS="$MONO_CFLAGS $GTK_CFLAGS $XULRUNNER_CFLAGS"
PERF_TOOL_LIBS="$MONO_LIBS $GTK_LIBS $XULRUNNER_LIBS"
AC_SUBST(PERF_TOOL_CFLAGS)
AC_SUBST(PERF_TOOL_LIBS)

MOON_CFLAGS="$CAIRO_CFLAGS $FREETYPE2_CFLAGS $FONTCONFIG_CFLAGS $GTK_CFLAGS $ALSA_CFLAGS $PULSEAUDIO_CFLAGS $FFMPEG_CFLAGS $MONO_CFLAGS"
dnl MOON_LIBS is partially defined above for the user-plugin build
MOON_LIBS="$MOON_LIBS $MONO_LIBS $FFMPEG_LIBS $CAIRO_LIBS $FREETYPE2_LIBS $FONTCONFIG_LIBS $GTK_LIBS $ZLIB $PTHREAD_LIBS $EXPAT_LIBS $UNWIND_LIBS"

dnl For MOON_XPI_LIBS, only link against libs which will be on all systems
MOON_XPI_LIBS="$CAIRO_LIBS $FREETYPE2_LIBS $FONTCONFIG_LIBS $GTK_LIBS $ZLIB $PTHREAD_LIBS $EXPAT_LIBS" 

# Remove glitz from the link flags (fixes Bug 338506)
MOON_LIBS="$(echo $MOON_LIBS | sed -e s/-lglitz//g)"
MOON_XPI_LIBS="$(echo $MOON_XPI_LIBS | sed -e s/-lglitz//g)"

AC_SUBST(MOON_CFLAGS)
AC_SUBST(MOON_LIBS)
AC_SUBST(MOON_XPI_LIBS)

MOON_EXTRA_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $PULSEAUDIO_CFLAGS"
MOON_EXTRA_LIBS="$GTK_LIBS $FFMPEG_LIBS"
AC_SUBST(MOON_EXTRA_CFLAGS)
AC_SUBST(MOON_EXTRA_LIBS)

MOON_PROG_CFLAGS="$GTK_CFLAGS $ALSA_CFLAGS $FFMPEG_CFLAGS $PULSEAUDIO_CFLAGS $MONO_CFLAGS -I\$(top_srcdir)/src/"
MOON_PROG_LIBS="$GTK_LIBS $FFMPEG_LIBS $UNWIND_LIBS \$(top_builddir)/src/libmoon.la"
AC_SUBST(MOON_PROG_CFLAGS)
AC_SUBST(MOON_PROG_LIBS)

PLUGIN_CFLAGS="$GTK_CFLAGS $MONO_CFLAGS -DPLUGIN_DIR=\\\"\$(pkglibdir)\\\""
PLUGIN_LIBS="$GTK_LIBS $MONO_LIBS"
PLUGIN_XPI_LIBS="$GTK_LIBS"

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PLUGIN_XPI_LIBS)

LOADER_LIBS="$GLIB_LIBS"
AC_SUBST(LOADER_LIBS)

DOCDIR=$(pkg-config --variable=sourcesdir monodoc)

if test x$with_ff3 = xyes; then
   MOZILLA_CFLAGS=$FF3_CFLAGS
else
   MOZILLA_CFLAGS=$FF2_CFLAGS
fi
SHOCKER_CFLAGS="$GTK_CFLAGS $MOZILLA_CFLAGS $IMAGEMAGICK_CFLAGS $XTST_CFLAGS"
SHOCKER_LIBS="$GTK_LIBS $IMAGEMAGICK_LIBS $XTST_LIBS $UNWIND_LIBS"
AC_SUBST(SHOCKER_CFLAGS)
AC_SUBST(SHOCKER_LIBS)

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_OUTPUT([
moon.pc
moonlight.spec
Makefile
m4/Makefile
src/Makefile
src/codec-version.h
class/Makefile
class/tuning/Makefile
class/tuning/SecurityAttributes/Makefile
class/System.Windows/Makefile
class/System.Windows.Browser/Makefile
class/System.Windows.Controls/Makefile
class/System.Windows.Controls.Data/Makefile
class/Moon.Windows.Desktop/Makefile
scripts/Makefile
scripts/smcs
gtk/Makefile
man/Makefile
data/Makefile
test/Makefile
test/2.0/Makefile
test/2.0/FakeA11yBridge/Makefile
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
test/parsertest/Makefile
test/sizes/Makefile
test/templates/Makefile
test/templates/managed/Makefile
test/cpp/Makefile
tools/Makefile
tools/generators/Makefile
tools/mopen/Makefile
tools/mxap/Makefile
tools/munxap/Makefile
tools/unsign/Makefile
tools/xamlg/Makefile
tools/xaml2html/Makefile
tools/respack/Makefile
tools/moonvisi/Makefile
tools/sockpol/Makefile
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
plugin/silverlight-versions.h
plugin/curl/Makefile
plugin/firefox/Makefile
plugin/firefox/ff2/Makefile
plugin/firefox/ff3/Makefile
plugin/install/Makefile
plugin/install/install.rdf
plugin/install/manifest.json
plugin/test/Makefile
plugin/samples/Makefile
perf/Makefile
sdk/Makefile
])

# general configuration info
cat <<EOF
Moonlight configuration
=======================

  General configuration:
	Using cairo: $with_cairo
	Test Harness: $with_testing $testing_reason
	Performance Suite: $with_performance $performance_reason
	MonoDevelop SDK: $enable_sdk $sdk_reason
EOF

dnl debug info
cat <<EOF

  Debug configuration: $with_debug
	Sanity checks: $with_sanity_checks
	Object tracking: $with_object_tracking
	Logging support with MOONLIGHT_DEBUG: $with_logging
EOF

# media info
cat <<EOF

  Media Support: $media_warning
	FFmpeg: $with_ffmpeg $ffmpeg_reason
	ALSA: $with_alsa $alsa_reason
	PulseAudio: $with_pulseaudio $pulseaudio_reason
EOF

# silverlight version info
cat <<EOF

  Silverlight Support:
	Silverlight 1.0: yes
	Silverlight 2.0: yes
EOF

cat <<EOF
          Browser plugin assemblies: $browser_support $browser_reason
EOF
if test x$browser_support = xyes; then
cat <<EOF
            Path to mcs tree: $with_mcspath
            Path to mono-basic tree: $with_mono_basic_path
EOF
fi
cat <<EOF
          Desktop application assemblies: $desktop_support $desktop_reason
EOF

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

cat <<EOF
          Curl bridge: $with_curl $curl_reason
EOF

cat <<EOF
        Chrome (.crx): $chrome_extension $chrome_reason
EOF

# blank line
cat <<EOF

EOF
