DIST_SUBDIRS = test samples ff2 ff3 install
SUBDIRS = . test samples
if HAVE_GECKO_1_8
SUBDIRS += ff2
endif
if HAVE_GECKO_1_9
SUBDIRS += ff3
endif

if PLUGIN_INSTALL
SUBDIRS += install
endif

CLEANFILES =

SMCS = MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe $(CSCFLAGS)

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la

pluginlib_DATA=README

if INCLUDE_MONO_RUNTIME
pluginlib_DATA += moonlight.exe moonlight.exe.mdb
endif

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(srcdir)/moz-sdk \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS) \
	-Wall

libmoonloader_la_LDFLAGS = \
	-avoid-version

libmoonloader_la_LIBADD = $(PLUGIN_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

libmoonplugin_la_LDFLAGS = \
	-avoid-version

if !PLUGIN_INSTALL
MAYBE_LIBMOON = $(top_builddir)/src/libmoon.la
endif

libmoonplugin_la_LIBADD = $(MAYBE_LIBMOON) $(PLUGIN_LIBS)

libmoonplugin_la_SOURCES = \
	browser-bridge.h	\
	moonlight.h		\
	mono.cpp		\
	moon-mono.h		\
	plugin.cpp		\
	plugin.h		\
	plugin-downloader.cpp	\
	plugin-downloader.h	\
	plugin-entry.cpp 	\
	plugin-glue.cpp 	\
	plugin-class.cpp 	\
	plugin-class.h		\
	plugin-debug.cpp	\
	mmsh-state.h		\
	mmsh-state.cpp		\
	plugin-debug.h

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/moonlight.exe ~/.mozilla/plugins/libmoonloader.so
	cp .libs/libmoonloader.so ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs moon.config
DISTCLEANFILES=moonlight.exe moonlight.exe.mdb

dist-hook:
	mkdir $(distdir)/moz-sdk
	-cp $(srcdir)/moz-sdk/*.h $(distdir)/moz-sdk

moonlight_CS=  $(srcdir)/moonlight.cs

if INCLUDE_MONO_RUNTIME
moonlight.exe moonlight.exe.mdb: $(moonlight_CS) $(top_builddir)/class/lib/2.1/Mono.Moonlight.dll $(top_builddir)/class/lib/2.1/System.Windows.dll
	$(SMCS) -debug -out:moonlight.exe $(moonlight_CS) -r:$(top_builddir)/class/lib/2.1/Mono.Moonlight.dll -r:$(top_builddir)/class/lib/2.1/System.Windows.dll
CLEANFILES += moonlight.exe moonlight.exe.mdb
endif
