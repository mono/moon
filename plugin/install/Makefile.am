CLEANFILES = novell-moonlight.xpi

noinst_DATA = novell-moonlight.xpi

mono_libdir = `pkg-config --variable=libdir mono`
mono_dlldir = `pkg-config --variable=prefix mono`/lib
mono_sysconfdir = `pkg-config --variable=sysconfdir mono`

novell_moonlight_xpi_CONTENTS =	$(srcdir)/install.rdf \
				$(srcdir)/icon.png \
				$(srcdir)/chrome.manifest \
				../.libs/libmoonloader.so \
				../.libs/libmoonplugin.so \
				$(top_builddir)/src/.libs/libmoon.so

if HAVE_GECKO_1_8
novell_moonlight_xpi_CONTENTS += ../firefox/ff2/.libs/libmoonplugin-ff2bridge.so
endif HAVE_GECKO_1_8

if HAVE_GECKO_1_9
novell_moonlight_xpi_CONTENTS += ../firefox/ff3/.libs/libmoonplugin-ff3bridge.so
endif HAVE_GECKO_1_9

if INCLUDE_FFMPEG
novell_moonlight_xpi_CONTENTS += $(avutil_libdir)/libavutil.so \
				$(avcodec_libdir)/libavcodec.so

endif INCLUDE_FFMPEG

if INCLUDE_BROWSER_MANAGED_CODE
novell_moonlight_xpi_CONTENTS += $(srcdir)/../moon.config
endif INCLUDE_BROWSER_MANAGED_CODE

novell-moonlight.xpi: $(novell_moonlight_xpi_CONTENTS) Makefile
	-rm -f $@
	-rm -rf .xpinstall/
	mkdir -p .xpinstall/plugins/moonlight/
	cp -L $(srcdir)/install.rdf .xpinstall/
	cp -L $(srcdir)/chrome.manifest .xpinstall/
	mkdir -p .xpinstall/skin/
	cp -L $(srcdir)/icon.png .xpinstall/skin/
	cp -L ../.libs/libmoonloader.so .xpinstall/plugins/
	cp -L ../.libs/libmoonplugin.so .xpinstall/plugins/moonlight/
	cp -L $(top_builddir)/src/.libs/libmoon.so .xpinstall/plugins/moonlight/
if HAVE_GECKO_1_8
	cp -L ../firefox/ff2/.libs/libmoonplugin-ff2bridge.so .xpinstall/plugins/moonlight/
endif HAVE_GECKO_1_8
if HAVE_GECKO_1_9
	cp -L ../firefox/ff3/.libs/libmoonplugin-ff3bridge.so .xpinstall/plugins/moonlight/
endif HAVE_GECKO_1_9
if INCLUDE_FFMPEG
	cp -L $(avutil_libdir)/libavutil.so .xpinstall/plugins/moonlight/
	cp -L $(avcodec_libdir)/libavcodec.so .xpinstall/plugins/moonlight/
endif INCLUDE_FFMPEG

if INCLUDE_BROWSER_MANAGED_CODE
	cp -L $(mono_libdir)/libmono.so .xpinstall/plugins/moonlight/
#	mkdir -p .xpinstall/plugins/moonlight/mono/2.1/
#	cp -L $(mono_dlldir)/mono/2.1/mscorlib.dll .xpinstall/plugins/moonlight/mono/2.1/
#	find $(mono_dlldir)/mono/2.1 -name \*.dll -and -not -name mscorlib.dll -exec gacutil -i '{}' -package 2.1 -root .xpinstall/plugins/moonlight ';'
#	find $(top_builddir)/class/lib/2.1 -name \*.dll -exec gacutil -i '{}' -package 2.1 -root .xpinstall/plugins/moonlight ';'
#	cp -L $(mono_dlldir)/mono/2.1/*.dll .xpinstall/plugins/moonlight
	cp -L $(top_builddir)/class/lib/2.1/*.dll .xpinstall/plugins/moonlight
#	find .xpinstall -name \*.mdb -exec rm '{}' ';'

	mkdir -p .xpinstall/plugins/moonlight/mono/2.0/
#	cp -L $(srcdir)/../moon.config .xpinstall/plugins/moonlight/mono/config
	cp -L $(mono_sysconfdir)/mono/2.0/machine.config .xpinstall/plugins/moonlight/mono/2.0/
endif INCLUDE_BROWSER_MANAGED_CODE
	find .xpinstall -name \*.so -exec strip '{}' ';'  # strip symbols
	cd .xpinstall/ && zip -r9 ../$@ *

clean-local:
	-rm -rf .xpinstall

EXTRA_DIST = install.rdf.in icon.png chrome.manifest
DISTCLEANFILES = novell-moonlight.xpi
