SUBDIRS = test samples
CLEANFILES =

pluginlibdir=$(pkglibdir)/plugin
pluginlib_LTLIBRARIES = libmoonplugin.la libmoonloader.la

pluginlib_DATA=README

if PLUGIN_INSTALL
noinst_DATA = novell-moonlight.xpi
endif

if INCLUDE_MONO_RUNTIME
pluginlib_DATA += moonlight.exe moonlight.exe.mdb
endif

INCLUDES = \
	-I$(top_srcdir)/src \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS) \
	-Wall

libmoonloader_la_LDFLAGS = \
	-avoid-version -module

libmoonloader_la_LIBADD = $(PLUGIN_LIBS)

libmoonloader_la_SOURCES = \
	plugin-proxy.cpp

libmoonplugin_la_LDFLAGS = \
	-avoid-version \
	-module

if !PLUGIN_INSTALL
MAYBE_LIBMOON = $(top_builddir)/src/libmoon.la
endif

libmoonplugin_la_LIBADD = $(MAYBE_LIBMOON) $(MOON_LIBS) $(PLUGIN_LIBS)

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
	browser-http.h	\
	browser-http.cpp	\
	downloader.cpp

if PLUGIN_INSTALL
novell_moonlight_xpi_CONTENTS =	$(srcdir)/install.js \
				$(builddir)/.libs/libmoonloader.so \
				$(builddir)/.libs/libmoonplugin.so \
				$(top_builddir)/src/.libs/libmoon.so \
				$(avutil_libdir)/libavutil.so \
				$(swscale_libdir)/libswscale.so \
				$(avcodec_libdir)/libavcodec.so \
				$(avformat_libdir)/libavformat.so

novell-moonlight.xpi: $(novell_moonlight_xpi_CONTENTS)
	-rm -f $@
	zip -j -9 $@ $(novell_moonlight_xpi_CONTENTS)

CLEANFILES += novell-moonlight.xpi
endif

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la moonlight.exe
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/moonlight.exe
	cp .libs/libmoonloader.so ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs install.js
DISTCLEANFILES=moonlight.exe moonlight.exe.mdb novell-moonlight.xpi

moonlight_CS=  $(srcdir)/moonlight.cs

if INCLUDE_MONO_RUNTIME
moonlight.exe: $(moonlight_CS)
	smcs -debug -out:moonlight.exe $(moonlight_CS) -pkg:silver -pkg:agmonosilver
CLEANFILES += moonlight.exe moonlight.exe.mdb
endif
