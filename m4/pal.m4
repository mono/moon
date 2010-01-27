AC_DEFUN([MOONLIGHT_CHECK_PAL],
[
	dnl
	dnl eventually we need to replace all this with platform and/or --enable checks, but for now...
	dnl

	AC_DEFINE([PAL_GTK_WINDOWING],1,[Hack in support for the pal-gtk so we can start using it.])
	pal_windowing="gtk (hardcoded)"

	AC_DEFINE([PAL_GLIB_MESSAGING],1,[Hack in support for the pal-glib so we can start using it.])
	pal_messaging="glib+unix (hardcoded)"
])