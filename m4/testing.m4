AC_DEFUN([TEST_CHECK_MODULES],
[
	if test x$with_testing = xyes; then
		PKG_CHECK_MODULES($1, $2, [], [
			with_testing=no
			if test x"$3" = "x"; then
				testing_reason="(reason: failed to find $2)"
			else
				testing_reason="(reason: $3)"
			fi
		])
	fi
])

AC_DEFUN([MOONLIGHT_CHECK_TESTING],
[
	dnl
	dnl test suite
	dnl
	
	AC_ARG_WITH(testing, [  --with-testing=yes|no      Enable unit tests (defaults=yes)],[],[with_testing=yes])

	if test x$with_testing = xyes; then

		if test x$with_ff3 = xyes; then
			TEST_CHECK_MODULES(XULRUNNER, [mozilla-gtkmozembed mozilla-js], 
				[failed to find FF3 development packages])
		elif test x$with_ff2 = xyes; then
			TEST_CHECK_MODULES(XULRUNNER, [xulrunner-gtkmozembed], 
				[failed to find FF2 development packages])
		fi
	else
        testing_reason="(reason: disabled by user)"
    fi

	TEST_CHECK_MODULES(XTST, [xtst >= 1.0])
	TEST_CHECK_MODULES(IMAGEMAGICK, [ImageMagick++ >= 6.2.5])

	AM_CONDITIONAL(INCLUDE_TESTING, test x$with_testing = xyes)

	dnl
	dnl performance suite
	dnl

	AC_ARG_WITH(performance, [  --with-performance=yes|no      Enable performance tests (defaults=yes)],[],[with_performance=yes])

	if test x$with_performance = xyes; then

		if test x$with_ff3 = xyes; then
			PKG_CHECK_MODULES(XULRUNNER, [mozilla-gtkmozembed mozilla-js], [], [
				with_performance=no
				performance-reason="(reason: failed to find FF3 development packages)"
			])
		else
			with_performance=no
			performance_reason="(reason: performance suite requires FF3)"
		fi
    else
        performance_reason="(reason: disabled by user)"
	fi

	AM_CONDITIONAL(INCLUDE_PERFORMANCE,test x$with_performance = xyes)

	dnl Look to see if the MS tests are installed
	MS_DRTLIST=none
	if test -f $PWD/../moonlight-ms/tests/port/drop1030/built/drtlist.xml; then
		MS_DRTLIST=$PWD/../moonlight-ms/tests/port/drop1030/built/drtlist.xml
		AC_SUBST(MS_DRTLIST)
	elif test -f $PWD/../../extras/moonlight-ms/tests/port/drop1030/built/drtlist.xml; then
		MS_DRTLIST=$PWD/../../extras/moonlight-ms/tests/port/drop1030/built/drtlist.xml
		AC_SUBST(MS_DRTLIST)
	fi
])
