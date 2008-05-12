
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(FF3_CFLAGS) \
	$(MOON_CFLAGS) \
	-DWITH_LIBXUL_UNSTABLE \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES=libmoonplugin-ff3bridge.la

libmoonplugin_ff3bridge_la_SOURCES=	ff3-bridge.cpp \
					ff3-bridge.h \
					ff3-dom.cpp \
					ff3-http.cpp \
					ff3-mmsh.cpp

libmoonplugin_ff3bridge_la_LIBADD=$(FF3_LIBS) ../libmoonplugin.la ../../src/libmoon.la
