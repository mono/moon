
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(FF2_CFLAGS) \
	$(MOON_CFLAGS) \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES=libmoonplugin-ff2bridge.la

libmoonplugin_ff2bridge_la_SOURCES=ff2-bridge.cpp

libmoonplugin_ff2bridge_la_LIBADD=$(FF2_LIBS)