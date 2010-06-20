dnl add conf options for the subdirectory only
AC_DEFUN([CUSTOM_SUBDIR_OPTION],
[
AC_CONFIG_SUBDIRS([$1])

m4_ifblank([$2], [rm -f $1/configure.gnu],
    [AX_PRINT_TO_FILE([$1/configure.gnu],
[#!/bin/sh
./configure $2 ${AX_DOLLAR}@
])
])

dnl If this macro or the invocation of it is changed, we want configure
dnl to be re-run in subdirs. We cannot touch configure because that
dnl would make it younger than its dependencies (e.g. configure.ac) and
dnl we would potentially miss to regenerate configure. Instead we let
dnl autoconf touch configure.ac, meaning when autoconf is run in topdir
dnl it will trigger autotools to be run in subdirs.
m4_syscmd([
test -f $1/configure.ac && touch $1/configure.ac
test -f $1/configure.in && touch $1/configure.in
])

])

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
			with_curl=no
		fi
	else
		with_curl=no
	fi

	AM_CONDITIONAL(HAVE_CURL, test x$with_curl != xno)
])
