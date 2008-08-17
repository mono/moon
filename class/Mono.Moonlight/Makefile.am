assembly = Mono.Moonlight

if INCLUDE_BROWSER_MANAGED_CODE
assemblysl = ../lib/2.1/$(assembly).dll
endif

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/3.0/$(assembly).dll
endif

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS)
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib

mono_moonlight_sources =				\
	$(srcdir)/../Consts.cs				\
	$(srcdir)/Assembly/AssemblyInfo.cs		\
	$(srcdir)/Mono/Kind.cs				\
	$(srcdir)/Mono/GeneratedPInvokes.cs			\
	$(srcdir)/Mono/NativeMethods.cs			\
	$(srcdir)/Mono/Value.cs				\
	$(srcdir)/Mono/StreamCallbacks.cs						\
	$(srcdir)/Mono/ManagedType.cs			\
	$(srcdir)/Mono/MoonError.cs			\
	$(srcdir)/Mono/Moonlight.cs			\
	$(srcdir)/Mono/Helper.cs			\
	$(srcdir)/Mono/Surface.cs						\
	$(srcdir)/Mono/Types.cs						\
	$(srcdir)/Mono/Types.g.cs						\
	$(srcdir)/Mono/XamlLoaderCallbacks.cs		\
	$(srcdir)/Mono/XapHack.cs			\
	$(srcdir)/System.IO/SimpleUnmanagedMemoryStream.cs\
	$(srcdir)/System.Windows.Interop/PluginHost.cs	

EXTRA_DIST = $(mono_moonlight_sources)

$(assemblysl): $(mono_moonlight_sources) Makefile
	$(SMCS) -target:library -out:$@ $(mono_moonlight_sources) -d:NET_2_1 -keyfile:$(srcdir)/../mono.snk

$(assemblydesktop): $(mono_moonlight_sources) Makefile
	$(GMCS) -target:library -out:$@ $(mono_moonlight_sources) -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies)

if INCLUDE_BROWSER_MANAGED_CODE
install-sl:
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1
else
install-sl:
endif

if INCLUDE_DESKTOP_MANAGED_CODE
install-desktop:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
else
install-desktop:
endif

install-data-local: install-sl install-desktop

uninstall-local:
	$(GACUTIL) /u $(assembly)
