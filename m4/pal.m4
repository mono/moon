AC_DEFUN([MOONLIGHT_CHECK_PAL],
[
	AC_ARG_WITH([pal],[  --with-pal=gtk|cocoa   Specify which PAL to build (defaults to gtk)], [], [with_pal=gtk])

if test "x$with_pal" = "x"; then

	AC_MSG_ERROR([unknown PAL specified])

elif test "x$with_pal" = "xgtk"; then

	AC_DEFINE([PAL_GTK_WINDOWING],1,[Hack in support for the pal-gtk so we can start using it.])
	pal_windowing="gtk"

	AC_DEFINE([PAL_GLIB_MESSAGING],1,[Hack in support for the pal-glib so we can start using it.])
	pal_messaging="glib+unix"

	AC_DEFINE([PAL_LINUX_NETWORKAVAILABILITY],1,[Hack in support for the linux network container.])
	pal_networking="linux"

	AC_DEFINE([PAL_DBUS_NETWORKAVAILABILITY],1,[Hack in support for the dbus-glib so we can start using it.])
	pal_networking="$pal_networking dbus-glib"

	AC_DEFINE([PAL_LINUX_CAPTURE],1,[Hack in support for the linux capture container])
	pal_capture="linux"

	AC_DEFINE([PAL_V4L2_VIDEO_CAPTURE],1,[Hack in support for pal-v4l2 so we can start using it.])
	pal_video_capture="v4l2"

	dnl AC_DEFINE([PAL_PULSE_AUDIO_CAPTURE],1,[Hack in support for pal-pulse so we can start using it.])
	pal_audio_capture="none"

	AC_DEFINE([PAL_FONTCONFIG_FONTSERVICE],1,[Hack in support for fontconfig so we can start using it.])
	pal_font_service="fontconfig"

	PKG_CHECK_MODULES(GTK, gtk+-2.0 gthread-2.0 atk)
	PKG_CHECK_MODULES(DBUS, dbus-1 dbus-glib-1)
	PKG_CHECK_MODULES(XRANDR, xrandr, [
		AC_DEFINE([USE_RANDR], [1], 
			[Include support for the XRANDR extension for querying a monitor's refresh rate])
	], [xrandr_present=no])
	PKG_CHECK_MODULES(FREETYPE2, freetype2)
	PKG_CHECK_MODULES(FONTCONFIG, fontconfig)
	PKG_CHECK_MODULES(GLIB, glib-2.0)

	PAL=gtk

elif test "x$with_pal" = "xcocoa"; then

	dnl We force parsing as objective-c++ since thats what we are, but automake doesn't understand .mm
	OBJCFLAGS="-x objective-c++ $CFLAGS"
	LDFLAGS="$LDFLAGS -liconv -framework AppKit"

	AC_DEFINE([PAL_COCOA_WINDOWING],1,[Hack in support for the pal-cocoa so we can start using it.])
	pal_windowing="cocoa"

	AC_DEFINE([PAL_OSX_MESSAGING],1,[Hack in support for the pal-osx so we can start using it.])
	pal_messaging="cocoa"

	pal_networking="none"
	pal_capture="none"
	pal_video_capture="none"
	pal_audio_capture="none"
	pal_font_service="none"

	dnl We need to flesh out eglib enough to drop this dep
	PKG_CHECK_MODULES(GLIB, glib-2.0)

	PAL=cocoa
fi

AM_CONDITIONAL(GTK_PAL, test x$with_pal = xgtk)
AM_CONDITIONAL(COCOA_PAL, test x$with_pal = xcocoa)
AC_SUBST([PAL])
])
