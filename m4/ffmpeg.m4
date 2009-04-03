AC_DEFUN([MOONLIGHT_CHECK_FFMPEG],
[
	AC_ARG_WITH(ffmpeg, AC_HELP_STRING([--with-ffmpeg=yes|no],
		[If you want to enable support for ffmpeg]),
		[], [with_ffmpeg=yes])

	if test x$with_ffmpeg = xyes; then
		if pkg-config --exists libavutil libavcodec; then
	    	AC_DEFINE([INCLUDE_FFMPEG], [1], [Include support for ffmpeg])
			PKG_CHECK_MODULES(FFMPEG, [libavutil libavcodec])
			save_CFLAGS=$CFLAGS
			CFLAGS="$FFMPEG_CFLAGS $CFLAGS"
			AC_CHECK_HEADERS([libavcodec/avcodec.h])
			CFLAGS=$save_CFLAGS
		else
			with_ffmpeg=no
			ffmpeg_reason="(reason: could not find libavutil and libavcodec packages)"
		fi
	fi

	AM_CONDITIONAL(INCLUDE_FFMPEG, test x$with_ffmpeg = xyes)

	if test x$with_ffmpeg = no; then
		media_warning="WARNING: No media decoding backends configured.  AUDIO AND VIDEO WON'T WORK"
	fi
])
