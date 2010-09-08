AC_DEFUN([MOONLIGHT_CHECK_PAL],
[
	dnl
	dnl eventually we need to replace all this with platform and/or --enable checks, but for now...
	dnl

	AC_DEFINE([PAL_GTK_WINDOWING],1,[Hack in support for the pal-gtk so we can start using it.])
	pal_windowing="gtk (hardcoded)"

	AC_DEFINE([PAL_GLIB_MESSAGING],1,[Hack in support for the pal-glib so we can start using it.])
	pal_messaging="glib+unix (hardcoded)"

	AC_DEFINE([PAL_LINUX],1,[Hack in support for the linux network container.])
	pal_networking="linux (hardcoded)"

	AC_DEFINE([PAL_DBUS_NETWORKAVAILABILITY],1,[Hack in support for the dbus-glib so we can start using it.])
	pal_networking="$pal_networking dbus-glib (hardcoded)"

	AC_DEFINE([PAL_LINUX_CAPTURE],1,[Hack in support for the linux capture container])
	pal_capture="linux (hardcoded)"

	AC_DEFINE([PAL_V4L2_VIDEO_CAPTURE],1,[Hack in support for pal-v4l2 so we can start using it.])
	pal_video_capture="v4l2 (hardcoded)"

	dnl AC_DEFINE([PAL_PULSE_AUDIO_CAPTURE],1,[Hack in support for pal-pulse so we can start using it.])
	pal_audio_capture="none (hardcoded)"
])