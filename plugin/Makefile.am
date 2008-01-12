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

libmoonplugin_la_LIBADD = $(MAYBE_LIBMOON) $(PLUGIN_LIBS)

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
				.libs/libmoonloader.so \
				.libs/libmoonplugin.so \
				$(top_builddir)/src/.libs/libmoon.so

if INCLUDE_FFMPEG
novell_moonlight_xpi_CONTENTS += $(avformat_libdir)/libavformat.so
				$(avutil_libdir)/libavutil.so \
				$(swscale_libdir)/libswscale.so \
				$(avcodec_libdir)/libavcodec.so
endif INCLUDE_FFMPEG

if INCLUDE_MONO_RUNTIME
novell_moonlight_xpi_CONTENTS += moonlight.exe \
				$(srcdir)/moon.config
endif INCLUDE_MONO_RUNTIME

novell-moonlight.xpi: $(novell_moonlight_xpi_CONTENTS)
	-rm -f $@
	-rm -rf .xpinstall/
	mkdir -p .xpinstall/moonlight/
if INCLUDE_FFMPEG
	cp -L $(avutil_libdir)/libavutil.so .xpinstall/moonlight/
	cp -L $(swscale_libdir)/libswscale.so .xpinstall/moonlight/
	cp -L $(avcodec_libdir)/libavcodec.so .xpinstall/moonlight/
	cp -L $(avformat_libdir)/libavformat.so .xpinstall/moonlight/
	find .xpinstall -name \*.so -exec strip '{}' ';'
endif INCLUDE_FFMPEG
	cp -L $(srcdir)/install.js .xpinstall/
	cp -L .libs/libmoonloader.so .xpinstall/
	cp -L .libs/libmoonplugin.so .xpinstall/moonlight/
	cp -L $(top_builddir)/src/.libs/libmoon.so .xpinstall/moonlight/
if INCLUDE_MONO_RUNTIME
	cp -L $(mono_libdir)/libmono.so .xpinstall/moonlight/
	cp -L moonlight.exe .xpinstall/moonlight/
	mkdir -p .xpinstall/moonlight/mono/2.1/
	cp -L $(mono_dlldir)/mono/2.1/mscorlib.dll .xpinstall/moonlight/mono/2.1/
	find $(mono_dlldir)/mono/2.1 -name \*.dll -and -not -name mscorlib.dll -exec gacutil -i '{}' -root .xpinstall/moonlight ';'
	find $(mono_dlldir)/mono/gac -path \*/2.1.\*.dll -exec gacutil -i '{}' -root .xpinstall/moonlight ';'
	find .xpinstall -name \*.mdb -exec rm '{}' ';'
	mkdir -p .xpinstall/moonlight/mono/2.0/
	cp -L $(srcdir)/moon.config .xpinstall/moonlight/mono/config
	cp -L $(mono_sysconfdir)/mono/2.0/machine.config .xpinstall/moonlight/mono/2.0/
endif INCLUDE_MONO_RUNTIME
	cd .xpinstall/ && zip -r9 ../$@ *

CLEANFILES += novell-moonlight.xpi

clean-local:
	-rm -rf .xpinstall
endif PLUGIN_INSTALL

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la moonlight.exe
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/moonlight.exe
	cp .libs/libmoonloader.so ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs install.js.in moon.config
DISTCLEANFILES=moonlight.exe moonlight.exe.mdb novell-moonlight.xpi

moonlight_CS=  $(srcdir)/moonlight.cs

if INCLUDE_MONO_RUNTIME
moonlight.exe: $(moonlight_CS)
	smcs -debug -out:moonlight.exe $(moonlight_CS) -pkg:silver -pkg:agmonosilver
CLEANFILES += moonlight.exe moonlight.exe.mdb
endif
