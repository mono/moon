AC_DEFUN([MOONLIGHT_CHECK_GLX],
[
	AC_ARG_WITH(glx, AC_HELP_STRING([--with-glx=yes|no],
		[If you want to enable support for glx]),
		[], [with_glx=yes])

	if test x$with_glx = xyes; then
		if pkg-config --exists gl x11; then
			AC_DEFINE(USE_GLX, 1, [Include glx support])
			PKG_CHECK_MODULES(GLX, gl x11)
		else
			with_glx=no
		fi
	fi

	AM_CONDITIONAL(HAVE_GL, [test x$with_glx != xno])
	AM_CONDITIONAL(HAVE_GLX, [test x$with_glx != xno])
])
AC_DEFUN([MOONLIGHT_CHECK_EGL],
[
	AC_ARG_WITH(egl, AC_HELP_STRING([--with-egl=yes|no],
		[If you want to enable support for egl]),
		[], [with_egl=no])

	if test x$with_egl = xyes; then
		AC_DEFINE(USE_EGL, 1, [Include egl support])
	fi

	AM_CONDITIONAL(HAVE_GL, [test x$with_egl != xno])
	AM_CONDITIONAL(HAVE_EGL, [test x$with_egl != xno])
])
