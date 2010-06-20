
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/asf \
	-I$(top_srcdir)/src/pal \
	-I$(top_srcdir)/plugin \
	-I$(top_srcdir)/plugin/moz-sdk \
	-I/usr/include/X11 \
	$(CURL_CFLAGS) \
	$(MOON_CFLAGS) \
	-DWITH_LIBXUL_UNSTABLE \
	-Wall

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin-curlbridge.la
noinst_LTLIBRARIES = libmoonpluginxpi-curlbridge.la


libmoonplugin_curlbridge_la_SOURCES=	curl-bridge.cpp \
					curl-bridge.h \
					curl-http.cpp \
					curl-http.h

libmoonplugin_curlbridge_la_LDFLAGS=-module -avoid-version
libmoonplugin_curlbridge_la_LIBADD=$(CURL_LIBS) ../libmoonplugin.la ../../src/libmoon.la

libmoonpluginxpi_curlbridge_la_SOURCES = $(libmoonplugin_curlbridge_la_SOURCES)
libmoonpluginxpi_curlbridge_la_LDFLAGS = $(libmoonplugin_curlbridge_la_LDFLAGS) -rpath /nowhere
libmoonpluginxpi_curlbridge_la_LIBADD = $(CURL_LIBS) ../libmoonpluginxpi.la
