AC_DEFUN([MOONLIGHT_CHECK_GLX],
[
	AC_ARG_WITH(glx, AC_HELP_STRING([--with-glx=yes|no],
		[If you want to enable support for glx]),
		[], [with_glx=no])

	GLX_CFLAGS=
	GLX_LIBS=

	if test x$with_glx = xyes; then
		GLX_LIBS="-lGL"
		AC_DEFINE(USE_GLX, 1, [Include glx support])
	fi

	AC_SUBST(GLX_CFLAGS)
	AC_SUBST(GLX_LIBS)

	AM_CONDITIONAL(HAVE_GLX, [test x$with_glx != xno])
])

