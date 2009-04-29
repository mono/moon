AC_DEFUN([MOONLIGHT_CHECK_UNWIND],
[
	AC_CHECK_HEADER(libunwind.h)
	AC_CHECK_HEADER(demangle.h)

	AC_MSG_CHECKING(for libunwind)

	LDFLAGS_save="$LDFLAGS"
	LIBS_save="$LIBS"

	LDFLAGS="$LDFLAGS -liberty -lunwind"
	LIBS="$LIBS -liberty -lunwind"

	AC_TRY_LINK_FUNC(unw_get_proc_name, have_unwind_libs="yes", have_unwind_libs="no")
	AC_TRY_LINK_FUNC(cplus_demangle, have_iberty_libs="yes", have_iberty_libs="no")

	LDFLAGS="$LDFLAGS_save"
	LIBS="$LIBS_save"

	if test "$have_unwind_libs" = "no" or test "$have_iberty_libs" = "no"; then
		AC_MSG_RESULT([no])
		#AC_MSG_ERROR([Failed to find suitable libunwind libraries. Please make sure that libunwind is installed.])
	else
		AC_MSG_RESULT([yes])
		UNWIND_LIBS="-liberty -lunwind"
		AC_DEFINE([HAVE_UNWIND], [1], [libunwind support])
	fi
])
