
AC_DEFUN([MOONLIGHT_CHECK_EXAMPLES],
[
	dnl
	dnl examples
	dnl
	
	AC_ARG_WITH(examples, [  --with-examples=yes|no    Enable examples (defaults=no)],[],[with_examples=no])

	AM_CONDITIONAL(INCLUDE_EXAMPLES, test x$with_examples = xyes)


])
