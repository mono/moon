AC_DEFUN([MOONLIGHT_CHECK_GL],
[
	with_gl=no

	dnl GLX
	AC_ARG_WITH(glx, AC_HELP_STRING([--with-glx=yes|no],
		[If you want to enable support for glx]),
		[], [with_glx=yes])
	if test x$with_glx = xyes; then
		if pkg-config --exists gl x11; then
			AC_DEFINE(USE_GLX, 1, [Include glx support])
			PKG_CHECK_MODULES(GLX, gl x11)
			with_gl=yes
			MOONLIGHT_CHECK_GLCHAR([GL/gl.h])
		else
			with_glx=no
		fi
	fi
	AM_CONDITIONAL(HAVE_GLX, [test x$with_glx != xno])

	dnl CGL
	AC_ARG_WITH(cgl, AC_HELP_STRING([--with-cgl=yes|no],
		[If you want to enable support for cgl]),
		[], [with_cgl=no])
	if test x$with_cgl = xyes; then
		AC_DEFINE(USE_CGL, 1, [Include cgl support])
		with_gl=yes
		MOONLIGHT_CHECK_GLCHAR([OpenGL/OpenGL.h])
	fi
	AM_CONDITIONAL(HAVE_CGL, [test x$with_cgl != xno])

	dnl EGL
	AC_ARG_WITH(egl, AC_HELP_STRING([--with-egl=yes|no],
		[If you want to enable support for egl]),
		[], [with_egl=no])
	if test x$with_egl = xyes; then
		AC_DEFINE(USE_EGL, 1, [Include egl support])
		with_gl=yes
		MOONLIGHT_CHECK_GLCHAR([GLES/gl.h])
	fi
	AM_CONDITIONAL(HAVE_EGL, [test x$with_egl != xno])

	AM_CONDITIONAL(HAVE_GL, [test x$with_gl != xno])


])

AC_DEFUN([MOONLIGHT_CHECK_GLCHAR],
  [AC_CACHE_CHECK([for GLchar], ac_cv_type_GLchar,
    [AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
         [AC_INCLUDES_DEFAULT
#           include <$1>],
         [GLchar x; return sizeof x;])],

      [ac_cv_type_glchar=yes],
      [ac_cv_type_glchar=no])])
 if test x$type_glchar = xyes; then
   AC_DEFINE([HAVE_GLCHAR], 1,
             [Define to 1 if GL header declares GLchar.])
 else
   AC_DEFINE([GLchar], char,
             [Define to a type if GL header does not define.])
 fi])
