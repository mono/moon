AC_DEFUN([MOONLIGHT_CHECK_PAL],
[
	dnl
	dnl eventually we need to replace all this with platform and/or --enable checks, but for now...
	dnl

	AC_DEFINE([PAL_GTK],1,[Hack in support for the pal gtk cpp symbol so we can start using it.])
	pal_backend="gtk (hacked)"
])