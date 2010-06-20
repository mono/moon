AC_DEFUN([MOONLIGHT_CHECK_GTK],
[
	PKG_CHECK_MODULES(GTK, gtk+-2.0 gthread-2.0)
])

AC_DEFUN([MOONLIGHT_CHECK_GLIB], 
[
	PKG_CHECK_MODULES(GLIB, glib-2.0)
])

AC_DEFUN([MOONLIGHT_CHECK_ZLIB],
[
	AC_CHECK_HEADERS(zlib.h)
	AC_CHECK_LIB(z, inflate, ZLIB="-lz")
])

AC_DEFUN([MOONLIGHT_CHECK_FONTS],
[
	PKG_CHECK_MODULES(FREETYPE2, freetype2)
	PKG_CHECK_MODULES(FONTCONFIG, fontconfig)
])

AC_DEFUN([MOONLIGHT_CHECK_XRANDR],
[
	PKG_CHECK_MODULES(XRANDR, xrandr, [
		AC_DEFINE([USE_RANDR], [1], 
			[Include support for the XRANDR extension for querying a monitor's refresh rate])
	], [xrandr_present=no])
])

AC_DEFUN([MOONLIGHT_CHECK_CURL],
[
	AC_ARG_WITH(curl, AC_HELP_STRING([--with-curl=no|yes],
		[If you want to enable the curl bridge]),
		[], [with_curl=yes])

	if test x$with_curl = xyes; then
		PKG_CHECK_MODULES(CURL, libcurl, [has_curl=yes], [has_curl=no])

		if test x$has_curl = xyes; then
			AC_DEFINE([HAVE_CURL], [1], [curl support for the bridge])
		else
			with_curl=no
		fi
	fi

	AM_CONDITIONAL(HAVE_CURL, test x$with_curl = xyes)
])
