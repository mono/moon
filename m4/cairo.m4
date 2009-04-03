AC_DEFUN([MOONLIGHT_CHECK_CAIRO],
[
	AC_ARG_WITH(cairo, AC_HELP_STRING([--with-cairo=embedded|system],
		[Enable linking against system cairo]),
		[], [with_cairo=embedded])

	if test x$with_cairo = xembedded; then
		ac_configure_args="$ac_configure_args --with-pic=yes --disable-pdf --disable-svg --disable-ps --disable-png --disable-xcb"
		AC_CONFIG_SUBDIRS([pixman cairo])
		
		dnl hackish but I couldn't get AC_EGREP_HEADER to work with our embedded cairo :(
		if grep CAIRO_LINE_CAP_TRIANGLE $srcdir/cairo/src/cairo.h >/dev/null 2>&1; then
			AC_DEFINE(HAVE_CAIRO_LINE_CAP_TRIANGLE, [], [Define if cairo has CAIRO_LINE_CAP_TRIANGLE as a cairo_line_cap_t enum member])
		fi
		
		CAIRO_CFLAGS='-I$(top_srcdir)/cairo/src'
		CAIRO_LIBS='$(top_builddir)/cairo/src/libcairo.la'
	else
		PKG_CHECK_MODULES(CAIRO,cairo >= 1.8)
	fi
])
