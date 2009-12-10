assembly = Moon.Windows.Desktop

NET_2_1_ASSEMBLY_DEPS=-r:System.Runtime.Serialization.dll
NET_3_0_ASSEMBLY_DEPS=

sl_secattr = ../tuning/SecurityAttributes/$(assembly).secattr

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/moonlight/$(assembly).dll

FLAGS_DESKTOP= -r:../lib/moonlight/System.Windows.dll
endif

assemblies = $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:SANITY -d:NET_1_1 -d:NET_2_0 -d:MOONLIGHT -debug+ -noconfig -r:System -r:System.Core -r:System.Xml -d:AGCLR -unsafe

GMCS = MONO_PATH="../lib/moonlight:$$MONO_PATH" gmcs $(CSCFLAGS) -lib:../lib/moonlight -d:NET_3_0
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib

system_windows_sources =							\
	$(srcdir)/Assembly/AssemblyInfo.cs					\
	$(srcdir)/Assembly/MonoTODOAttribute.cs					\
	$(srcdir)/../Consts.cs							\
	$(srcdir)/Moon.Windows.Desktop/Window.cs

XAML_RESOURCES = $(srcdir)/themes/generic.xaml,themes/generic.xaml
XAML_RESOURCES_DEPS = $(srcdir)/themes/generic.xaml

EXTRA_DIST = $(system_windows_sources) 	\
	Assembly/ChangeLog			\
	Moon.Windows.Desktop/ChangeLog

$(assembly).g.resources: $(XAML_RESOURCES_DEPS)
	mono $(top_builddir)/tools/respack/respack.exe $@ $(XAML_RESOURCES)

$(assemblydesktop): $(system_windows_sources) Makefile $(assembly).g.resources
	$(GMCS) $(NET_3_0_ASSEMBLY_DEPS) $(FLAGS_DESKTOP) -target:library -out:$@ $(system_windows_sources) \
	        -resource:$(assembly).g.resources \
	        -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) check-static check-dynamic
	-rm -rf *.exe *.mdb tester.cs
	-rm -rf $(assembly).g.resources

if INCLUDE_DESKTOP_MANAGED_CODE
install-desktop:
	$(GACUTIL) /i $(assemblydesktop) /f /package moonlight
else
install-desktop:
endif

install-data-local: install-desktop

uninstall-local:
	$(GACUTIL) /u $(assembly)

dist-hook:
	-mkdir $(distdir)/themes
	-cp $(srcdir)/themes/*.xaml $(distdir)/themes

check-code: check-static check-dynamic

check-static:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/moonlight/Moon.Windows.Desktop.dll -out:check-static.exe
	MONO_PATH=../lib/moonlight:$$MONO_PATH mono --debug check-static.exe
	> check-static

check-dynamic:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/moonlight/Moon.Windows.Desktop.dll -out:check-static.exe
	mono --debug check-static.exe generate-test-code > tester.cs
	gmcs tester.cs -r:../lib/moonlight/System.Windows.dll -debug:full -nowarn:219
	MONO_PATH=../lib/moonlight:$$MONO_PATH mono --debug tester.exe
