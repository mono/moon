AC_DEFUN([MOONLIGHT_CHECK_CODECS],
[
	dnl Do we know of a way to detect when the GCC ABI breaks? 
	dnl will it ever again? we'll hardcode this for now
	MOONLIGHT_CODEC_GCC_ABI_VERSION="1"
	MOONLIGHT_CODEC_OSTYPE="unknown"
	MOONLIGHT_CODEC_ARCH="unknown"
	CODECS_SUPPORTED="no"
	CODECS_ARCH_SUPPORTED="no"
	CODECS_OS_SUPPORTED="no"

	case "$host" in
		i*86-*-*)
			MOONLIGHT_CODEC_ARCH=x86
			CODECS_ARCH_SUPPORTED="yes"
			;;
		x86_64-*-* | amd64-*-*)
			MOONLIGHT_CODEC_ARCH=x64
			CODECS_ARCH_SUPPORTED="yes"
			;;
	esac
	
	case "$host" in
		*-*-*linux*)
			MOONLIGHT_CODEC_OSTYPE=linux
			CODECS_OS_SUPPORTED="yes"
			;;
	esac

	if test ${MOONLIGHT_CODEC_ARCH} = unknown; then
		AC_WARN([The codecs have not been configured to build on this architecture yet])
	fi
	
	if test ${MOONLIGHT_CODEC_OSTYPE} = unknown; then
		AC_WARN([The codecs have not been configured to build on this operating system yet])
	fi

	if test x$CODECS_ARCH_SUPPORTED = xyes; then
		if test x$CODECS_OS_SUPPORTED = xyes; then
			CODECS_SUPPORTED="yes"
		fi
	fi

	AM_CONDITIONAL(CODECS_SUPPORTED, test x$CODECS_SUPPORTED = xyes)

	AC_SUBST(MOONLIGHT_CODEC_OSTYPE)
	AC_SUBST(MOONLIGHT_CODEC_ARCH)
	AC_SUBST(MOONLIGHT_CODEC_GCC_ABI_VERSION)
])
