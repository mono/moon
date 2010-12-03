AC_DEFUN([MOONLIGHT_CHECK_UNWIND],
[
	AC_ARG_WITH(unwind, AC_HELP_STRING([--with-unwind=no|yes],
		            [Enable stacktraces using libunwind]),
		            [], [with_unwind=no])

    if test x$with_unwind = xyes; then

		AC_CHECK_HEADER(libunwind.h)

		AC_MSG_CHECKING(for libunwind)

		LDFLAGS_save="$LDFLAGS"
		LIBS_save="$LIBS"

		case "$host" in
			x86_64-*-* | amd64-*-*)
				IBERTY="iberty_pic"
				LDFLAGS="$LDFLAGS -l$IBERTY"
				LIBS="$LIBS -l$IBERTY"
				AC_CHECK_LIB($IBERTY, cplus_demangle, have_iberty="yes", have_iberty="no")
				if test x$have_iberty = xno; then
					IBERTY="iberty"
				fi
				LDFLAGS="$LDFLAGS_save"
				LIBS="$LIBS_save"
				;;
			default )
				IBERTY="iberty" ;;
		esac

		UNWIND_LIBS="-l$IBERTY -lunwind"
		LDFLAGS="$LDFLAGS $UNWIND_LIBS"
		LIBS="$LIBS $UNWIND_LIBS"

		if test x$have_iberty = xno; then
			AC_CHECK_LIB($IBERTY, cplus_demangle, have_iberty="yes", have_iberty="no")
		fi
		AC_CHECK_LIB(unwind, backtrace, have_unwind="yes", have_unwind="no")

		LDFLAGS="$LDFLAGS_save"
		LIBS="$LIBS_save"

		if test x$have_unwind = xyes -a x$have_iberty = xyes; then
			AC_MSG_RESULT([yes])
			if test "x$with_debug" = "xyes"; then
				AC_DEFINE([HAVE_UNWIND], [1], [libunwind support])
			fi
		else
			with_unwind=no
			AC_MSG_RESULT([no])
		fi
	fi
])
