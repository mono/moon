AC_DEFUN([MOONLIGHT_CHECK_SOUND],
[
	dnl
	dnl ALSA
	dnl

	AC_ARG_WITH(alsa, AC_HELP_STRING([--with-alsa=yes|no], 
		[If you want to enable alsa sound support]),
		[], [with_alsa=yes])

	if test x$with_alsa = xyes; then
		if pkg-config --exists alsa; then
			AC_DEFINE([INCLUDE_ALSA], [1], [Include alsa sound support])
			PKG_CHECK_MODULES(ALSA, alsa)
		else
			with_alsa=no
			alsa_reason="(reason: could not find alsa development package)"
		fi
	else
  		alsa_reason="(reason: disabled at configure time)"
	fi

	AM_CONDITIONAL(INCLUDE_ALSA, test x$with_alsa = xyes)

	dnl
	dnl PulseAudio
	dnl

	AC_ARG_WITH(pulseaudio, AC_HELP_STRING([--with-pulseaudio=yes|no], 
		[If you want to enable pulseaudio sound support]),
		[], [with_pulseaudio=yes])

	if test x$with_pulseaudio = xyes; then
		if pkg-config --exists libpulse; then
			AC_DEFINE([INCLUDE_PULSEAUDIO], [1], [Include pulseaudio sound support])
			PKG_CHECK_MODULES(PULSEAUDIO, libpulse)
		else
			with_pulseaudio=no
			pulseaudio_reason="(reason: could not find libpulse development package)"
		fi
	else
		pulseaudio_reason="(reason: disabled at configure time)"
	fi
	
	AM_CONDITIONAL(INCLUDE_PULSEAUDIO, test x$with_pulseaudio = xyes)
])
