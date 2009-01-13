SUBDIRS = . test samples firefox

if PLUGIN_INSTALL
SUBDIRS += install
endif

CLEANFILES =

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la

pluginlib_DATA=README

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zip \
	-I$(srcdir)/moz-sdk \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS) \
	-Wall

libmoonloader_la_LDFLAGS = \
	-avoid-version

libmoonloader_la_LIBADD = -ldl $(LOADER_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

libmoonplugin_la_LDFLAGS = \
	-avoid-version

if !PLUGIN_INSTALL
MAYBE_LIBMOON = $(top_builddir)/src/libmoon.la
endif

libmoonplugin_la_LIBADD = $(MAYBE_LIBMOON) $(PLUGIN_LIBS)

INCLUDES += -I$(top_srcdir)/src/asf

libmoonplugin_la_SOURCES = \
	browser-bridge.h	\
	moonlight.h		\
	npstream-request.cpp	\
	npstream-request.h	\
	plugin.cpp		\
	plugin.h		\
	plugin-downloader.cpp	\
	plugin-downloader.h	\
	plugin-entry.cpp 	\
	plugin-glue.cpp 	\
	plugin-class.cpp 	\
	plugin-class.h		\
	plugin-debug.cpp	\
	plugin-debug.h		\
	windowless.cpp		\
	windowless.h

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/libmoonloader.so
	cp .libs/libmoonloader.so ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so

EXTRA_DIST = moon.config

dist-hook:
	mkdir $(distdir)/moz-sdk
	-cp $(srcdir)/moz-sdk/*.h $(distdir)/moz-sdk

# gross hack needed because of the way the bridges are located
all-local:
	@mkdir -p .libs
	test -L .libs/libmoonplugin-ff2bridge.so || (cd .libs && ln -s ../firefox/ff2/.libs/libmoonplugin-ff2bridge.so)
	test -L .libs/libmoonplugin-ff3bridge.so || (cd .libs && ln -s ../firefox/ff3/.libs/libmoonplugin-ff3bridge.so)
