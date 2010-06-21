AC_DEFUN([MOONLIGHT_CHECK_MOZILLA],
[
	with_mozilla=no

	if test x$browser_support = xno; then
		with_ff3=no
		with_ff2=no
	fi

	dnl
	dnl Firefox 3.6
	dnl


	AC_ARG_WITH(ff3, AC_HELP_STRING([--with-ff3=no|yes],
		[If you want to enable the xulrunner 1.9 (Firefox 3) bridge]),
		[], [with_ff3=yes])

	if test x$with_ff3 = xyes -a x$browser_support = xyes; then
		FF3_MODULES="libxul mozilla-plugin mozilla-js"

		PKG_CHECK_EXISTS($FF3_MODULES,
			[with_ff3=yes],
			[with_ff3=no ff3_reason="(reason: missing FF3 development packages)"])

		if test x$with_ff3 = xyes; then
			AC_DEFINE([HAVE_GECKO_1_9], [1], [Gecko 1.9 support])

			PKG_CHECK_MODULES(FF3, [$FF3_MODULES glib-2.0])
			FF3_CFLAGS=`$PKG_CONFIG --cflags --define-variable=includetype=unstable "$FF3_MODULES glib-2.0"`
			FF3_LIBS=`$PKG_CONFIG --libs --define-variable=includetype=unstable "$FF3_MODULES glib-2.0"`

			dnl Strip out problem libraries (should already be in process space)
			FF3_LIBS="$(echo $FF3_LIBS | sed -e 's/-lmozjs\|-lplds4\|-lplc4\|-lnspr4//g')"
		fi
	fi

	AM_CONDITIONAL(HAVE_GECKO_1_9,test x$with_ff3 = xyes)

	dnl
	dnl Firefox 2
	dnl

	AC_ARG_WITH(ff2, AC_HELP_STRING([--with-ff2=no|yes],
		[If you want to enable the xulrunner 1.8.1 (Firefox 2)]),
		[], [with_ff2=yes])

	if test x$with_ff2 = xyes -a x$browser_support = xyes; then
		mozilla_xpcom="libxul-missing"
  		mozilla_xpcom_pcs="xpcom mozilla-xpcom firefox-xpcom xulrunner-xpcom"
  		for pc in $mozilla_xpcom_pcs; do
			PKG_CHECK_EXISTS($pc, [mozilla_xpcom=$pc])
		done
		
		mozilla_plugin="plugin-missing"
		mozilla_plugin_pcs="plugin firefox-plugin xulrunner-plugin"
		for pc in $mozilla_plugin_pcs; do
			PKG_CHECK_EXISTS($pc, [mozilla_plugin=$pc])
		done
		
		if test $mozilla_xpcom = "libxul-missing" -o $mozilla_plugin = "plugin-missing"; then
			with_ff2=no
    		ff2_reason="(reason: missing FF2 development packages)"
		else
    		PKG_CHECK_MODULES(FF2, [$mozilla_xpcom $mozilla_plugin glib-2.0])
			dnl Strip out problem libraries (should already be in process space)
			FF2_LIBS="$(echo $FF2_LIBS | sed -e 's/-lmozjs\|-lplds4\|-lplc4\|-lnspr4//g')"
		fi
	fi
	
	AM_CONDITIONAL(HAVE_GECKO_1_8,test x$with_ff2 = xyes)

	dnl
	dnl Put it all together
	dnl

	if test x$with_ff2 = xyes; then
		with_mozilla=yes
		MIN_FIREFOX_VERSION="1.5"
		if test x$with_ff3 = xyes; then
			MAX_FIREFOX_VERSION="3.6.*"
		else
			MAX_FIREFOX_VERSION="2.0.0.*"
		fi
	elif test x$with_ff3 = xyes; then
   		with_mozilla=yes
		MIN_FIREFOX_VERSION="2.9.*"
		MAX_FIREFOX_VERSION="3.6.*"
  	fi

	AC_SUBST([MIN_FIREFOX_VERSION])
	AC_SUBST([MAX_FIREFOX_VERSION])
	AM_CONDITIONAL(HAVE_MOZILLA, test x$with_mozilla = xyes)
])
