SUBDIRS = . test samples firefox

CLEANFILES =

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la

noinst_LTLIBRARIES = libmoonpluginxpi.la libmoonloaderxpi.la

pluginlib_DATA=README

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zip \
	-I$(srcdir)/moz-sdk \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS) \
	-Wall


#--- libmoonloader ----------------------
libmoonloader_la_LDFLAGS = \
	-avoid-version

libmoonloader_la_LIBADD = -ldl $(LOADER_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

#--- libmoonloaderxpi -------------------
libmoonloaderxpi_la_LDFLAGS = $(libmoonloader_la_LDFLAGS) -rpath /nowhere

libmoonloaderxpi_la_LIBADD = $(libmoonloader_la_LIBADD)

libmoonloaderxpi_la_SOURCES = \
	plugin-proxy-xpi.cpp

#--- libmoonplugin ----------------------

libmoonplugin_la_LDFLAGS = \
	-avoid-version

libmoonplugin_la_LIBADD = $(top_builddir)/src/libmoon.la $(PLUGIN_LIBS)

INCLUDES += -I$(top_srcdir)/src/asf

libmoonplugin_la_SOURCES = \
	cbinding.h	\
	cbinding.cpp	\
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


#--- libmoonpluginxpi ----------------------
libmoonpluginxpi_la_LDFLAGS = $(libmoonplugin_la_LDFLAGS) -rpath /nowhere

libmoonpluginxpi_la_LIBADD = $(PLUGIN_XPI_LIBS)

libmoonpluginxpi_la_SOURCES = $(libmoonplugin_la_SOURCES)

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/libmoonloader.so
	cp .libs/libmoonloader.so ~/.mozilla/plugins

user-plugin: libmoonloaderxpi.la libmoonpluginxpi.la
	$(MAKE) $(AM_MAKEFLAGS) -C install

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
