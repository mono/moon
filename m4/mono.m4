AC_DEFUN([MOONLIGHT_CHECK_MONO],
[
	MONO_REQUIRED_VERSION=2.0
	MONO_REQUIRED_BROWSER_VERSION=2.5

	AC_ARG_WITH([mcs-path],
		    [  --with-mcs-path=/path/to/mcs      Specify an alternate mcs source tree],
		[],
		[with_mcs_path=$srcdir/../mono/mcs]
	)

	AC_ARG_WITH([mono-path],
		    [  --with-mono-path=/path/to/mono      Specify an alternate mono source tree],
		[],
		[with_mono_path=$srcdir/../mono]
	)

	if test ! -d "$with_mcs_path"; then
		AC_ERROR($with_mcs_path doesn't exist)
	fi

	if test ! -d $with_mono_path; then
		with_mono_path=$with_mcs_path/..
	fi

	MCS_PATH=$(cd "$with_mcs_path" && pwd)
	AC_SUBST(MCS_PATH)

	MONO_PATH=$(cd "$with_mono_path" && pwd)
	AC_SUBST(MONO_PATH)

	MOON_ARG_ENABLED_BY_DEFAULT([browser-support], [Disable the browser plugin])
	browser_support=$enableval
	if test "x$browser_support" = xyes; then
		dnl
		dnl path to mono-basic checkout
		dnl
		AC_ARG_WITH([mono-basic-path],
				[  --with-mono-basic-path=/path/to/mono-basic      Path to the mono-basic checkout],
			[],
			[with_mono_basic_path=$srcdir/../mono-basic]
		)
		if test ! -d "$with_mono_basic_path"; then
			AM_CONDITIONAL([HAVE_MONO_BASIC], false)
		else
			MONO_BASIC_PATH=$(cd "$with_mono_basic_path" && pwd)
			AC_SUBST(MONO_BASIC_PATH)
			AM_CONDITIONAL([HAVE_MONO_BASIC], true)
		fi
		AC_DEFINE([PLUGIN_SL_2_0], [1], [Enable Silverlight 2.0 support for the plugin])
	else
		AM_CONDITIONAL([HAVE_MONO_BASIC], false)
	fi

	MOON_ARG_ENABLED_BY_DEFAULT([desktop-support], [Disable support for Moonlight-based desktop applications])
	desktop_support=$enableval
	if test "x$desktop_support" = xyes; then
		if test "x$with_pal" = "xgtk"; then
			PKG_CHECK_MODULES(GTKSHARP, gtk-sharp-2.0, [
				AM_CONDITIONAL(HAVE_GTK_SHARP, true)
			], [
				AM_CONDITIONAL(HAVE_GTK_SHARP, false)
			])

			PKG_CHECK_MODULES(WNCKSHARP, wnck-sharp-1.0)

			rsvg_sharp_pcs="rsvg-sharp-2.0 rsvg2-sharp-2.0"
			for pc in $rsvg_sharp_pcs; do
				PKG_CHECK_EXISTS($pc, [rsvg_sharp=$pc])
			done

			PKG_CHECK_MODULES(RSVGSHARP, $rsvg_sharp, [
				RSVG_SHARP=$rsvg_sharp
				AC_SUBST(RSVG_SHARP)
				AM_CONDITIONAL(HAVE_RSVG_SHARP, true)
			], [
				AM_CONDITIONAL(HAVE_RSVG_SHARP, false)
			])
		else
			AM_CONDITIONAL(HAVE_RSVG_SHARP, false)
			AM_CONDITIONAL(HAVE_GTK_SHARP, false)
		fi
	else
		AM_CONDITIONAL(HAVE_RSVG_SHARP, false)
		AM_CONDITIONAL(HAVE_GTK_SHARP, false)
	fi

	if test "x$desktop_support" = xno -a "x$browser_support" = xno; then
		AC_ERROR(You cannot disable both Browser and Desktop support)
	fi

	MONO_CFLAGS=-I$MONO_PATH

	AC_DEFINE([MONO_ENABLE_APP_DOMAIN_CONTROL], [1], [Whether Mono 2.5 is available and Deployment should create/destroy App Domains])
	AC_DEFINE([MONO_ENABLE_CORECLR_SECURITY], [1], [Whether Mono 2.5 is available and CoreCLR security should be enabled])
	AC_DEFINE([SL_2_0], [1], [Enable Silverlight 2.0 support in the runtime])

	AM_CONDITIONAL(INCLUDE_MANAGED_CODE, true)
	AM_CONDITIONAL(INCLUDE_BROWSER_MANAGED_CODE, test x$browser_support = xyes)
	AM_CONDITIONAL(INCLUDE_DESKTOP_MANAGED_CODE, test x$desktop_support = xyes)

	SL_PROFILE=3.0
	AC_SUBST([SL_PROFILE])


	dnl
	dnl monodevelop sdk installation
	dnl

	MOON_ARG_ENABLED_BY_DEFAULT([sdk], [Disable installation of the monodevelop sdk])
	enable_sdk=$enableval
	if test "x$enable_sdk" = xyes -a "x$browser_support" = xno; then
	   enabled_sdk=no
	   sdk_reason="(SDK requires browser support)"
	fi
	if test "x$enable_sdk" = xyes -a "x$with_mono_basic_path" = "xno"; then
	   enabled_sdk=no
	   sdk_reason="(SDK requires mono-basic support)"
	fi

	AM_CONDITIONAL([INSTALL_MONODEVELOP_SDK],test x$enable_sdk = xyes)
])

