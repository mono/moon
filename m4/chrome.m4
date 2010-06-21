AC_DEFUN([MOONLIGHT_CHECK_CHROME_CRX],
[
	dnl chrome mandates the stupid restriction that version numbers be
	dnl 4 dot separated numbers (i.e. 1.2.3.4)  we require 5 numbers
	dnl for our previews, so we convert from, e.g. 2.99.0.3.99 to 2.99.0.399
	dnl for the chrome version.

	CHROME_VERSION=`echo $VERSION | sed -e 's/\.99$/99/' -e 's/\.\([1-9]\)$/\.\100/'`
	AC_SUBST(CHROME_VERSION)

	chrome_extension="yes"

	AC_PATH_PROG(CHROME, google-chrome, no)
	if test "x$CHROME" = "xno" ; then
	   AC_PATH_PROG(CHROME, chromium, no)
        fi
	if test "x$CHROME" = "xno" ; then
	   AC_PATH_PROG(CHROME, chromium-browser, no)
        fi

	if test "x$CHROME" = "xno" ; then
	   chrome_extension="no"
	   chrome_reason="(no chrome executable)"
	fi

	if test "x$with_curl" = "xno" ; then
	   chrome_extension="no"
	   chrome_reason="(chrome support requires the curl bridge)"
	fi

	AM_CONDITIONAL(CHROME_INSTALL, [test x$chrome_extension = xyes])
])
