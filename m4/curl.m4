AC_DEFUN([MOONLIGHT_CHECK_CURL],
[
	AC_ARG_WITH(curl, AC_HELP_STRING([--with-curl=no|embedded|system],
		[If you want to enable the curl bridge]),
		[], [with_curl=embedded])

	if test x$with_curl = xembedded; then
		PKG_CHECK_MODULES(OPENSSL, openssl, [has_openssl=yes], [has_openssl=no])
		if test x$has_openssl = xyes; then
			CUSTOM_SUBDIR_OPTION(curl, [--disable-shared --disable-manual])

			CURL_CFLAGS='-I$(top_srcdir)/curl/include'
			CURL_LIBS="\$(top_builddir)/curl/lib/libcurl.la $OPENSSL_LIBS"

			AC_DEFINE([HAVE_CURL], [1], [curl support for the bridge])
		else
			with_curl=no
		fi

	elif test x$with_curl = xsystem; then
		PKG_CHECK_MODULES(CURL, libcurl, [has_curl=yes], [has_curl=no])

		if test x$has_curl = xyes; then
			AC_DEFINE([HAVE_CURL], [1], [curl support for the bridge])
		else
			AC_MSG_ERROR(system curl build requires curl devel package)
		fi
	else
		with_curl=no
	fi

	AM_CONDITIONAL(HAVE_CURL, test x$with_curl != xno)
])
