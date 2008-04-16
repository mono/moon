assembly = Mono.Moonlight
assemblysl = ../lib/2.1/$(assembly).dll
assemblydesktop = ../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS)
SMCS = MONO_PATH="../lib/2_1:$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(prefix)/lib /root $(prefix)/lib

mono_moonlight_sources =			\
	../Consts.cs				\
	Assembly/AssemblyInfo.cs		\
	Mono/Kind.cs				\
	Mono/NativeMethods.cs			\
	Mono/Value.cs				\
	Mono/Moonlight.cs			\
	Mono/Helper.cs				\
	Mono/XamlLoaderCallbacks.cs		\
	System.IO/SimpleUnmanagedMemoryStream.cs\
	System.Windows.Interop/PluginHost.cs	

EXTRA_DIST = $(mono_moonlight_sources)

$(assemblysl): $(mono_moonlight_sources) Makefile
	$(SMCS) -target:library -out:$@ $(mono_moonlight_sources) -d:NET_2_1
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(mono_moonlight_sources) Makefile
	$(GMCS) -target:library -out:$@ $(mono_moonlight_sources)
	sn -q -R $@ $(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies)

install-data-local:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1

uninstall-local:
	$(GACUTIL) /u $(assembly)
