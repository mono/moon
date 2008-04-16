SUBDIRS = test samples
CLEANFILES =

SMCS = MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe $(CSCFLAGS)

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
	-I$(srcdir)/moz-sdk \
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
	plugin-downloader.cpp	\
	plugin-downloader.h	\
	plugin-entry.cpp 	\
	plugin-glue.cpp 	\
	plugin-class.cpp 	\
	plugin-class.h		\
	plugin-debug.cpp	\
	plugin-debug.h		\
	browser-http.h		\
	browser-http.cpp	\
	browser-mmsh.h		\
	browser-mmsh.cpp

if PLUGIN_INSTALL
novell_moonlight_xpi_CONTENTS =	$(srcdir)/install.rdf \
				.libs/libmoonloader.so \
				.libs/libmoonplugin.so \
				$(top_builddir)/src/.libs/libmoon.so

if INCLUDE_FFMPEG
novell_moonlight_xpi_CONTENTS += $(avutil_libdir)/libavutil.so \
				$(avcodec_libdir)/libavcodec.so

if INCLUDE_SWSCALE
novell_moonlight_xpi_CONTENTS += $(swscale_libdir)/libswscale.so
endif INCLUDE_SWSCALE

endif INCLUDE_FFMPEG

if INCLUDE_MONO_RUNTIME
novell_moonlight_xpi_CONTENTS += moonlight.exe \
				$(srcdir)/moon.config
endif INCLUDE_MONO_RUNTIME

novell-moonlight.xpi: $(novell_moonlight_xpi_CONTENTS)
	-rm -f $@
	-rm -rf .xpinstall/
	mkdir -p .xpinstall/plugins/moonlight/
if INCLUDE_FFMPEG
	cp -L $(avutil_libdir)/libavutil.so .xpinstall/plugins/moonlight/
if INCLUDE_SWSCALE
	cp -L $(swscale_libdir)/libswscale.so .xpinstall/plugins/moonlight/
endif INCLUDE_SWSCALE
	cp -L $(avcodec_libdir)/libavcodec.so .xpinstall/plugins/moonlight/
	find .xpinstall -name \*.so -exec strip '{}' ';'
endif INCLUDE_FFMPEG
	cp -L $(srcdir)/install.rdf .xpinstall/
	cp -L .libs/libmoonloader.so .xpinstall/plugins/
	cp -L .libs/libmoonplugin.so .xpinstall/plugins/moonlight/
	cp -L $(top_builddir)/src/.libs/libmoon.so .xpinstall/plugins/moonlight/
if INCLUDE_MONO_RUNTIME
	cp -L $(mono_libdir)/libmono.so .xpinstall/plugins/moonlight/
	cp -L moonlight.exe .xpinstall/plugins/moonlight/
	mkdir -p .xpinstall/plugins/moonlight/mono/2.1/
	cp -L $(mono_dlldir)/mono/2.1/mscorlib.dll .xpinstall/plugins/moonlight/mono/2.1/
	find $(mono_dlldir)/mono/2.1 -name \*.dll -and -not -name mscorlib.dll -exec gacutil -i '{}' -root .xpinstall/plugins/moonlight ';'
	find $(mono_dlldir)/mono/gac -path \*/2.1.\*.dll -exec gacutil -i '{}' -root .xpinstall/plugins/moonlight ';'
	find .xpinstall -name \*.mdb -exec rm '{}' ';'
	mkdir -p .xpinstall/plugins/moonlight/mono/2.0/
	cp -L $(srcdir)/moon.config .xpinstall/plugins/moonlight/mono/config
	cp -L $(mono_sysconfdir)/mono/2.0/machine.config .xpinstall/plugins/moonlight/mono/2.0/
endif INCLUDE_MONO_RUNTIME
	cd .xpinstall/ && zip -r9 ../$@ *

CLEANFILES += novell-moonlight.xpi

clean-local:
	-rm -rf .xpinstall
endif PLUGIN_INSTALL

# custom install rule
test-plugin: libmoonloader.la libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libmoon.so ~/.mozilla/plugins/libmoonplugin.so ~/.mozilla/plugins/moonlight.exe ~/.mozilla/plugins/libmoonloader.so
	cp .libs/libmoonloader.so ~/.mozilla/plugins

untest-plugin:
	-rm -rf ~/.mozilla/plugins/libmoonplugin.so
	-rm -rf ~/.mozilla/plugins/libmoonloader.so
	rm -rf ~/.mozilla/plugin/moonlight.exe

EXTRA_DIST = moonlight.cs install.rdf.in moon.config
DISTCLEANFILES=moonlight.exe moonlight.exe.mdb novell-moonlight.xpi

dist-hook:
	mkdir $(distdir)/moz-sdk
	-cp $(srcdir)/moz-sdk/*.h $(distdir)/moz-sdk

moonlight_CS=  $(srcdir)/moonlight.cs

if INCLUDE_MONO_RUNTIME
moonlight.exe: $(moonlight_CS) $(top_builddir)/class/lib/2.1/Mono.Moonlight.dll $(top_builddir)/class/lib/2.1/System.Windows.dll
	$(SMCS) -debug -out:moonlight.exe $(moonlight_CS) -r:$(top_builddir)/class/lib/2.1/Mono.Moonlight.dll -r:$(top_builddir)/class/lib/2.1/System.Windows.dll
CLEANFILES += moonlight.exe moonlight.exe.mdb
endif
