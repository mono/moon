SUBDIRS = test samples

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la

pluginlib_DATA=README moonlight.exe

INCLUDES = \
	-I$(top_srcdir)/src \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS)

libmoonloader_la_LDFLAGS = \
	-avoid-version -module

libmoonloader_la_LIBADD = $(PLUGIN_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

libmoonplugin_la_LDFLAGS = \
	-avoid-version \
	-module

libmoonplugin_la_LIBADD = $(top_builddir)/src/libmoon.la $(MOON_LIBS) $(PLUGIN_LIBS)

libmoonplugin_la_SOURCES = \
	moonlight.h		\
	mono.cpp		\
	moon-mono.h		\
	plugin.cpp		\
	plugin.h		\
	plugin-entry.cpp 	\
	plugin-glue.cpp 	\
	plugin-class.cpp 	\
	plugin-class.h 	\
	downloader.cpp

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la moonlight.exe
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so
	cp .libs/libmoonplugin.so ~/.mozilla/plugins
	cp .libs/libmoonloader.so ~/.mozilla/plugins
	cp moonlight.exe ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs
DISTCLEANFILES=moonlight.exe moonlight.exe.mdb

moonlight_CS=  $(srcdir)/moonlight.cs

moonlight.exe: $(moonlight_CS)
	gmcs -debug -out:moonlight.exe $(moonlight_CS) -pkg:silver
