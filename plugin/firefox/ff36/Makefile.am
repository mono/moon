
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/asf \
	-I$(top_srcdir)/src/pal \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(FF36_CFLAGS) \
	$(MOON_CFLAGS) \
	-DWITH_LIBXUL_UNSTABLE \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin-ff36bridge.la
noinst_LTLIBRARIES = libmoonpluginxpi-ff36bridge.la


libmoonplugin_ff36bridge_la_SOURCES=	ff36-bridge.cpp \
					ff36-bridge.h \
					ff36-http.cpp \
					../ff-common.h

libmoonplugin_ff36bridge_la_LDFLAGS=-module -avoid-version
libmoonplugin_ff36bridge_la_LIBADD=$(FF36_LIBS) ../../libmoonplugin.la ../../../src/libmoon.la

libmoonpluginxpi_ff36bridge_la_SOURCES = $(libmoonplugin_ff36bridge_la_SOURCES)
libmoonpluginxpi_ff36bridge_la_LDFLAGS = $(libmoonplugin_ff36bridge_la_LDFLAGS) -rpath /nowhere
libmoonpluginxpi_ff36bridge_la_LIBADD = $(FF36_LIBS) ../../libmoonpluginxpi.la
