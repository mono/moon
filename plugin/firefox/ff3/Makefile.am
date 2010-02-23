
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/asf \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(FF3_CFLAGS) \
	$(MOON_CFLAGS) \
	-DWITH_LIBXUL_UNSTABLE \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin-ff3bridge.la
noinst_LTLIBRARIES = libmoonpluginxpi-ff3bridge.la


libmoonplugin_ff3bridge_la_SOURCES=	ff3-bridge.cpp \
					ff3-bridge.h \
					ff3-http.cpp \
					../ff-common.h

libmoonplugin_ff3bridge_la_LDFLAGS=-module -avoid-version
libmoonplugin_ff3bridge_la_LIBADD=$(FF3_LIBS) ../../libmoonplugin.la ../../../src/libmoon.la

libmoonpluginxpi_ff3bridge_la_SOURCES = $(libmoonplugin_ff3bridge_la_SOURCES)
libmoonpluginxpi_ff3bridge_la_LDFLAGS = $(libmoonplugin_ff3bridge_la_LDFLAGS) -rpath /nowhere
libmoonpluginxpi_ff3bridge_la_LIBADD = $(FF3_LIBS) ../../libmoonpluginxpi.la
