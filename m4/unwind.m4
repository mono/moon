AC_DEFUN([MOONLIGHT_CHECK_UNWIND],
[
	AC_CHECK_HEADER(libunwind.h)

	AC_MSG_CHECKING(for libunwind)

	LDFLAGS_save="$LDFLAGS"
	LIBS_save="$LIBS"

	LDFLAGS="$LDFLAGS -liberty -lunwind"
	LIBS="$LIBS -liberty -lunwind"

	AC_CHECK_LIB(unwind, backtrace, have_unwind_libs="yes", have_unwind_libs="no")

	LDFLAGS="$LDFLAGS_save"
	LIBS="$LIBS_save"

	if test "$have_unwind_libs" = "no"; then
		AC_MSG_RESULT([no])
	else
		AC_MSG_RESULT([yes])
		if test "x$with_debug" = "xyes"; then
			UNWIND_LIBS="-liberty -lunwind"
			AC_DEFINE([HAVE_UNWIND], [1], [libunwind support])
		fi
	fi
])
