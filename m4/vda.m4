AC_DEFUN([MOONLIGHT_CHECK_VDA],
[
	AC_ARG_WITH(vda, AC_HELP_STRING([--with-vda=yes|no],
		[If you want to enable support for vda]),
		[], [with_vda=no])

	if test x$with_vda = xyes; then
		LDFLAGS="$LDFLAGS -framework VideoDecodeAcceleration"
    	AC_DEFINE([INCLUDE_VDA], [1], [Include support for vda])
	fi

	AM_CONDITIONAL(INCLUDE_VDA, test x$with_vda = xyes)
])
