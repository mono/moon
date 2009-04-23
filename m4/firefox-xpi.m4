AC_DEFUN([MOONLIGHT_CHECK_FIREFOX_XPI],
[
	avutil_libdir="$(pkg-config --variable=libdir libavutil)"
	avcodec_libdir="$(pkg-config --variable=libdir libavcodec)"
	AC_SUBST([avutil_libdir])
	AC_SUBST([avcodec_libdir])

	user_plugin="yes"
	if test x$with_ff3 = xno -a x$with_ff2 = xno; then
		user_plugin="no"
	fi

	AM_CONDITIONAL(PLUGIN_INSTALL, [test x$user_plugin = xyes])
		
	case "$target_os" in
		*linux*)
			TARGET_PLATFORM="Linux"
			;;
		*)
			AC_MSG_ERROR([Target os $target_os is unknown.
				Please add the appropriate string to configure.ac.
				See http://developer.mozilla.org/en/docs/OS_TARGET])
			;;
	esac
	
	case "$target_cpu" in
		i*86)
			TARGET_PLATFORM="$TARGET_PLATFORM"_x86-gcc3
			INSTALL_ARCH=i586
			;;
		x86_64)
				TARGET_PLATFORM="$TARGET_PLATFORM"_x86_64-gcc3
			INSTALL_ARCH=x86_64
			;;
		powerpc)
			TARGET_PLATFORM="$TARGET_PLATFORM"_ppc-gcc3
			INSTALL_ARCH=ppc
			;;
		sparc64)
			TARGET_PLATFORM="$TARGET_PLATFORM"_sparc64-gcc3
			INSTALL_ARCH=sparc
			;;
		*)
			AC_MSG_ERROR([Target cpu $target_cpu is unknown.
				Please add the appropriate string to configure.ac.
				See http://developer.mozilla.org/en/docs/XPCOM_ABI])
			;;
	esac
	
	AC_SUBST(TARGET_PLATFORM)
	AC_SUBST(INSTALL_ARCH)
])
