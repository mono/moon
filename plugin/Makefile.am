SUBDIRS = . test samples firefox

if HAVE_CURL
SUBDIRS += curl
endif

DIST_SUBDIRS = $(SUBDIRS) install

CLEANFILES =

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la
noinst_LTLIBRARIES = libmoonpluginxpi.la libmoonloaderxpi.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zip \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS) \
	-Wall \
	-DBUILDING_MOONLIGHT=1 \
	-fvisibility=hidden -fvisibility-inlines-hidden


#--- libmoonloader ----------------------
libmoonloader_la_LDFLAGS = \
	-avoid-version -rpath $(pluginlibdir) -R $(pluginlibdir) -R $(libdir)

libmoonloader_la_LIBADD = -ldl $(LOADER_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

#--- libmoonloaderxpi -------------------
libmoonloaderxpi_la_LDFLAGS = $(libmoonloader_la_LDFLAGS) -rpath /nowhere

libmoonloaderxpi_la_LIBADD = $(libmoonloader_la_LIBADD)

libmoonloaderxpi_la_SOURCES = $(libmoonloader_la_SOURCES)

#--- libmoonplugin ----------------------

libmoonplugin_la_LDFLAGS = \
	-avoid-version -rpath $(pluginlibdir) -R $(pluginlibdir) -R $(libdir)

libmoonplugin_la_LIBADD = $(top_builddir)/src/libmoon.la $(PLUGIN_LIBS)

INCLUDES += -I$(top_srcdir)/src/asf

libmoonplugin_la_SOURCES = \
	cbinding.h	\
	cbinding.cpp	\
	browser-bridge.h		\
	moonlight.h			\
	npstream-request.cpp		\
	npstream-request.h		\
	plugin.cpp			\
	plugin.h			\
	plugin-accessibility.cpp	\
	plugin-accessibility.h		\
	plugin-domevents.cpp		\
	plugin-domevents.h		\
	plugin-downloader.cpp		\
	plugin-downloader.h		\
	plugin-entry.cpp 		\
	plugin-glue.cpp 		\
	plugin-class.cpp 		\
	plugin-class.g.cpp 		\
	plugin-class.h			\
	plugin-debug.cpp		\
	plugin-debug.h			\
	plugin-utils.cpp		\
	plugin-spinner.h		\
	windowless.cpp			\
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

if PLUGIN_INSTALL
user-plugin: libmoonloaderxpi.la libmoonpluginxpi.la
	$(MAKE) $(AM_MAKEFLAGS) -C install
else
user-plugin:
	@echo "Error: You must configure with Firefox 2 or 3 support"; exit 1
endif

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so

EXTRA_DIST=moon.config silverlight-versions.h.in

DISTCLEANFILES= silverlight-version.h

dist-hook:
	mkdir $(distdir)/moz-sdk
	-cp $(srcdir)/moz-sdk/*.h $(distdir)/moz-sdk

inst-%: $(OUTPUT_DIR)/%
	-mkdir -p $(DESTDIR)$(pkglibdir)/plugin/
	$(INSTALL) -c -m 755  $(OUTPUT_DIR)/$* $(DESTDIR)$(pkglibdir)/plugin/$*
	test ! -f $(OUTPUT_DIR)/$*.mdb || $(INSTALL) -c -m 755  $(OUTPUT_DIR)/$*.mdb $(DESTDIR)$(pkglibdir)/plugin/$*.mdb


# gross hack needed because of the way the bridges are located
all-local:
	@mkdir -p .libs
	test -L .libs/libmoonplugin-ff2bridge.so || (cd .libs && ln -s ../firefox/ff2/.libs/libmoonplugin-ff2bridge.so)
	test -L .libs/libmoonplugin-ff3bridge.so || (cd .libs && ln -s ../firefox/ff3/.libs/libmoonplugin-ff3bridge.so)
	test -L .libs/libmoonplugin-ff36bridge.so || (cd .libs && ln -s ../firefox/ff3/.libs/libmoonplugin-ff36bridge.so)
	test -L .libs/libmoonplugin-curlbridge.so || (cd .libs && ln -s ../firefox/ff3/.libs/libmoonplugin-curlbridge.so)
