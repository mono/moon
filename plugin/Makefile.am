SUBDIRS = test samples

lib_LTLIBRARIES = libmoonplugin.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS)

libmoonplugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-L$(top_srcdir)/src/.libs

libmoonplugin_la_LIBADD = -lmoon $(MOON_LIBS) $(PLUGIN_LIBS)

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
	downloader.cpp 	\
	downloader.h

# custom install rule
install-plugin: libmoonplugin.la moonlight.exe
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so
	cp .libs/libmoonplugin.so ~/.mozilla/plugins
	cp moonlight.exe ~/.mozilla/plugins

uninstall-plugin:
	rm -rf ~/.mozilla/plugins/libmoonplugin.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs

moonlight.exe: moonlight.cs
	gmcs -debug moonlight.cs -pkg:silver
