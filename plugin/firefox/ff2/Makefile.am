
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/asf \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(FF2_CFLAGS) \
	$(MOON_CFLAGS) \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin-ff2bridge.la
noinst_LTLIBRARIES = libmoonpluginxpi-ff2bridge.la

libmoonplugin_ff2bridge_la_SOURCES=	ff2-bridge.cpp \
					ff2-bridge.h \
					ff2-http.cpp \
					../ff-common.h

libmoonplugin_ff2bridge_la_LDFLAGS=-module -avoid-version
libmoonplugin_ff2bridge_la_LIBADD=$(FF2_LIBS) ../../libmoonplugin.la ../../../src/libmoon.la

libmoonpluginxpi_ff2bridge_la_SOURCES = $(libmoonplugin_ff2bridge_la_SOURCES)
libmoonpluginxpi_ff2bridge_la_LDFLAGS = $(libmoonplugin_ff2bridge_la_LDFLAGS) -rpath /nowhere
libmoonpluginxpi_ff2bridge_la_LIBADD = $(FF2_LIBS) ../../libmoonpluginxpi.la

