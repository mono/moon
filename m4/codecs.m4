AC_DEFUN([MOONLIGHT_CHECK_CODECS],
[
	dnl Do we know of a way to detect when the GCC ABI breaks? 
	dnl will it ever again? we'll hardcode this for now
	CODECS_SUPPORTED="yes"
	CODECS_PATH=""
	CODECS_LIBS=""

	AC_ARG_WITH([codecs-path],
		    [  --with-codecs-path=/path/to/codecs    Specify the path to the codecs],
		[],
		[with_codecs_path=$srcdir/../codecs2]
	)

	if test ! -d "$with_codecs_path"; then
		codec_message="The codecs path was not found"
		CODECS_SUPPORTED="no"
	else
		CODECS_PATH=$(cd "$with_codecs_path" && pwd)
		codec_message=$CODECS_PATH
	fi
	
	case "$host" in
		i*86-*-*)
			;;
		x86_64-*-* | amd64-*-*)
			;;
		default)
			CODECS_SUPPORTED="no"
			;;
	esac
	
	case "$host" in
		*-*-*linux*)
			;;
		default)	
			CODECS_SUPPORTED="no"
			;;
	esac

	if test x$CODECS_SUPPORTED = xyes; then
		AC_DEFINE([CODECS_SUPPORTED], [1], [Enable codecs build])
		CODECS_LIBS="$CODECS_PATH/libmoon/libmoon_private.la"
	fi

	AM_CONDITIONAL(CODECS_SUPPORTED, test x$CODECS_SUPPORTED = xyes)

	AC_SUBST(CODECS_PATH)
	AC_SUBST(CODECS_LIBS)
])
