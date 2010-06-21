CLEANFILES = novell-moonlight.xpi

noinst_DATA = novell-moonlight.xpi

mono_libdir = `pkg-config --variable=libdir mono`
mono_dlldir = `pkg-config --variable=prefix mono`/lib
mono_sysconfdir = `pkg-config --variable=sysconfdir mono`

novell_moonlight_xpi_CONTENTS =		\
				$(srcdir)/install.rdf \
				$(srcdir)/icon.png \
				$(srcdir)/chrome.manifest \
				$(top_builddir)/plugin/.libs/libmoonloaderxpi.so \
				$(top_builddir)/plugin/.libs/libmoonpluginxpi.so \
				$(top_builddir)/src/.libs/libmoonxpi.so

if INCLUDE_BROWSER_MANAGED_CODE
novell_moonlight_xpi_CONTENTS +=	\
				$(top_builddir)/class/lib/2.1/mscorlib.dll \
				$(top_builddir)/class/lib/2.1/System.Core.dll \
				$(top_builddir)/class/lib/2.1/System.dll \
				$(top_builddir)/class/lib/2.1/System.Net.dll \
				$(top_builddir)/class/lib/2.1/System.Runtime.Serialization.dll \
				$(top_builddir)/class/lib/2.1/System.ServiceModel.dll \
				$(top_builddir)/class/lib/2.1/System.ServiceModel.Web.dll \
				$(top_builddir)/class/lib/2.1/System.Windows.Browser.dll \
				$(top_builddir)/class/lib/2.1/System.Windows.dll \
				$(top_builddir)/class/lib/2.1/System.Xml.dll
if HAVE_MONO_BASIC
novell_moonlight_xpi_CONTENTS += $(top_builddir)/class/lib/2.1/Microsoft.VisualBasic.dll
endif
endif INCLUDE_BROWSER_MANAGED_CODE


if HAVE_GECKO_1_8
novell_moonlight_xpi_CONTENTS += ../firefox/ff2/.libs/libmoonpluginxpi-ff2bridge.so
endif HAVE_GECKO_1_8

if HAVE_GECKO_1_9
novell_moonlight_xpi_CONTENTS += ../firefox/ff3/.libs/libmoonpluginxpi-ff3bridge.so
endif HAVE_GECKO_1_9

if INCLUDE_FFMPEG
novell_moonlight_xpi_CONTENTS += $(avutil_libdir)/libavutil.so \
				$(avcodec_libdir)/libavcodec.so
endif INCLUDE_FFMPEG

# Mono.CompilerServices.SymbolWriter.dll is useless for the plugin but should be shipped with our own SDK (for smcs)
SDK_DLLS = \
	Mono.CompilerServices.SymbolWriter.dll	\
	System.Data.Services.Client.dll         \
	System.Json.dll                         \
	System.Runtime.Serialization.Json.dll   \
	System.ServiceModel.PollingDuplex.dll   \
	System.ServiceModel.Syndication.dll     \
	System.Windows.Controls.Data.Design.dll \
	System.Windows.Controls.Data.dll        \
	System.Windows.Controls.Design.dll      \
	System.Windows.Controls.dll             \
	System.Windows.VisualStudio.Design.dll  \
	System.Xml.Serialization.dll            \
	System.Xml.Utils.dll			\
	System.Xml.Linq.dll

novell-moonlight.xpi: $(novell_moonlight_xpi_CONTENTS) Makefile
	rm -f $@
	rm -rf .xpinstall/
	rm -f novell-moonlight.xpi

	mkdir -p .xpinstall/plugins/moonlight/
	cp -L $(srcdir)/install.rdf .xpinstall/
	cp -L $(srcdir)/chrome.manifest .xpinstall/
	mkdir -p .xpinstall/skin/
	cp -L $(srcdir)/icon.png .xpinstall/skin/
	cp -L ../.libs/libmoonloaderxpi.so .xpinstall/plugins/
	cp -L ../.libs/libmoonpluginxpi.so .xpinstall/plugins/moonlight/
	cp -L $(top_builddir)/src/.libs/libmoonxpi.so .xpinstall/plugins/moonlight/
if HAVE_GECKO_1_8
# Rename the bridges to the original names
	cp -L ../firefox/ff2/.libs/libmoonpluginxpi-ff2bridge.so .xpinstall/plugins/moonlight/libmoonplugin-ff2bridge.so
endif HAVE_GECKO_1_8
if HAVE_GECKO_1_9
# Rename the bridges to the original names
	cp -L ../firefox/ff3/.libs/libmoonpluginxpi-ff3bridge.so .xpinstall/plugins/moonlight/libmoonplugin-ff3bridge.so
endif HAVE_GECKO_1_9
if HAVE_CURL
# Rename the bridges to the original names
	cp -L ../curl/.libs/libmoonpluginxpi-curlbridge.so .xpinstall/plugins/moonlight/libmoonplugin-curlbridge.so
endif HAVE_CURL
if INCLUDE_FFMPEG
	cp -L $(avutil_libdir)/libavutil.so .xpinstall/plugins/moonlight/
	cp -L $(avcodec_libdir)/libavcodec.so .xpinstall/plugins/moonlight/
endif INCLUDE_FFMPEG

if INCLUDE_BROWSER_MANAGED_CODE
	cp -L $(mono_libdir)/libmono.so .xpinstall/plugins/moonlight/
	cp -L $(top_builddir)/class/lib/2.1/*.dll .xpinstall/plugins/moonlight

	cd .xpinstall/plugins/moonlight && rm -f $(SDK_DLLS)
#	find .xpinstall -name \*.mdb -exec rm '{}' ';'

#	mkdir -p .xpinstall/plugins/moonlight/mono/2.0/
#	cp -L $(srcdir)/../moon.config .xpinstall/plugins/moonlight/mono/config
#	cp -L $(mono_sysconfdir)/mono/2.0/machine.config .xpinstall/plugins/moonlight/mono/2.0/
endif INCLUDE_BROWSER_MANAGED_CODE
#	find .xpinstall -name \*.so -exec strip '{}' ';'  # strip symbols
	cd .xpinstall/ && zip -r9 ../$@ *
	./check_xpi.sh $@

clean-local:
	-rm -rf .xpinstall
	-rm -f *.xpi
	-rm -rf tmp*

EXTRA_DIST = install.rdf.in icon.png chrome.manifest check_xpi.sh
DISTCLEANFILES = novell-moonlight.xpi
