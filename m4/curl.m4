AC_DEFUN([MOONLIGHT_CHECK_CURL],
[
	AC_ARG_WITH(curl, AC_HELP_STRING([--with-curl=no|embedded|system],
		[If you want to enable the curl bridge]),
		[], [with_curl=embedded])

	if test x$with_curl = xembedded; then
		mozilla_nss_pcs="nss mozilla-nss firefox-nss xulrunner-nss seamonkey-nss"
		for pc in $mozilla_nss_pcs; do
				if $PKG_CONFIG --exists $pc; then
				        AC_MSG_RESULT($pc)
				        mozilla_nss=$pc
				        break;
				fi
		done
		PKG_CHECK_MODULES(NSS, $mozilla_nss, [has_nss=yes], [has_nss=no])

		if test x$has_nss = xyes; then
			CUSTOM_SUBDIR_OPTION(curl, [--disable-shared --disable-manual --without-libssh2 --disable-ldap --disable-ldaps --without-libidn --without-ssl --with-nss])

			CURL_CFLAGS='-I$(top_srcdir)/curl/include'
			CURL_LIBS="\$(top_builddir)/curl/lib/libcurl.la $NSS_LIBS"

			AC_DEFINE([HAVE_CURL], [1], [curl support for the bridge])
		else
			AC_MSG_ERROR(embedded curl build requires nss devel package)
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
