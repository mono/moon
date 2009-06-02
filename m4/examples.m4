
AC_DEFUN([MOONLIGHT_CHECK_EXAMPLES],
[
	dnl
	dnl examples
	dnl
	
	AC_ARG_WITH(examples, [  --with-examples=yes|no    Enable examples (defaults=yes)],[],[with_examples=yes])

	AM_CONDITIONAL(INCLUDE_EXAMPLES, test x$with_examples = xyes)


])
