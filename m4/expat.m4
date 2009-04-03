AC_DEFUN([MOONLIGHT_CHECK_EXPAT],
[
	AC_CHECK_HEADER(expat.h)

	AC_MSG_CHECKING(for libexpat)
	
	LDFLAGS_save="$LDFLAGS"
	LIBS_save="$LIBS"
	
	LDFLAGS="$LDFLAGS -lexpat"
	LIBS="$LIBS -lexpat"

	AC_TRY_LINK_FUNC(XML_ParserCreateNS, have_expat_libs="yes", have_expat_libs="no")
	
	LDFLAGS="$LDFLAGS_save"
	LIBS="$LIBS_save"

	if test "$have_expat_libs" = "no"; then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Failed to find suitable libexpat libraries. Please make sure that libexpat-2.0.x is installed.])
	else
		AC_MSG_RESULT([yes])
		EXPAT_LIBS="-lexpat"
	fi
])
