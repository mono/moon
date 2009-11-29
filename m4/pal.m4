AC_DEFUN([MOONLIGHT_CHECK_PAL],
[
	 AC_DEFINE([PAL_GTK],1,[Hack in support for the pal gtk cpp symbol so we can start using it.])
	 pal_backend="gtk (hacked)"
])