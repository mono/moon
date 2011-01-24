assembly = System.Windows.Browser

if INCLUDE_BROWSER_MANAGED_CODE
assemblysl = ../lib/2.1/$(assembly).dll
assemblysl_raw = ../lib/2.1_raw/$(assembly).dll
sl_secattr = ../tuning/SecurityAttributes/$(assembly).secattr
endif

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/moonlight/$(assembly).dll
endif

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -d:AGCLR -unsafe
if DEBUG
CSCFLAGS += -d:DEBUG
endif
if SANITY
CSCFLAGS += -d:SANITY
endif

GMCS = gmcs $(CSCFLAGS) -lib:../lib/moonlight
SMCS = $(prefix)/bin/mono $(prefix)/lib/mono/2.0/mcs.exe -nostdlib -lib:../lib/2.1 -r:mscorlib.dll $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib
monoloc = `pkg-config --variable=libdir mono`/mono/moonlight
nunit = -r:$(monoloc)/nunit.framework.dll -r:$(monoloc)/nunit.core.dll -r:$(monoloc)/nunit.util.dll 

system_windows_browser_sources =					\
	$(srcdir)/Assembly/AssemblyInfo.cs					\
	$(srcdir)/System.Windows.Browser/BrowserInformation.cs			\
	$(srcdir)/System.Windows.Browser/HostServices.cs			\
	$(srcdir)/System.Windows.Browser/HtmlDocument.cs			\
	$(srcdir)/System.Windows.Browser/HtmlElement.cs				\
	$(srcdir)/System.Windows.Browser/HtmlEventArgs.cs			\
	$(srcdir)/System.Windows.Browser/HtmlObject.cs				\
	$(srcdir)/System.Windows.Browser/HtmlPage.cs				\
	$(srcdir)/System.Windows.Browser/HtmlPopupWindowOptions.cs		\
	$(srcdir)/System.Windows.Browser/HtmlWindow.cs				\
	$(srcdir)/System.Windows.Browser/HttpUtility.cs				\
	$(srcdir)/System.Windows.Browser/ManagedObject.cs			\
	$(srcdir)/System.Windows.Browser/MouseButtons.cs			\
	$(srcdir)/System.Windows.Browser/ScriptableMemberAttribute.cs		\
	$(srcdir)/System.Windows.Browser/ScriptableTypeAttribute.cs		\
	$(srcdir)/System.Windows.Browser/ScriptEventHandler.cs			\
	$(srcdir)/System.Windows.Browser/ScriptObject.cs			\
	$(srcdir)/System.Windows.Browser/ScriptObjectCollection.cs		\
	$(srcdir)/Mono/JSFriendlyMethodBinder.cs				\
	$(srcdir)/Mono/ScriptObjectHelper.cs					\
	$(srcdir)/Mono/ScriptObjectToggleRef.cs					\
	$(srcdir)/Mono/ScriptObjectEventInfo.cs

test_sources = \
	$(srcdir)/Test/System.Windows/WebApplicationTest.cs


JS_RESOURCES = $(srcdir)/System.Windows.Browser/json.js,json.js
JS_RESOURCES_DEPS = $(srcdir)/System.Windows.Browser/json.js

EXTRA_DIST = $(system_windows_browser_sources) 				\
	$(test_sources)							\
	Assembly/ChangeLog						\
	System.Windows.Browser/ChangeLog				\
	System.Windows.Browser.Net/ChangeLog				\
	System.Windows.Hosting/ChangeLog				\
	ChangeLog

CLEANFILES =			\
	TestResult-net_3_0.log	\
	TestResult-net_3_0.xml

$(assembly).g.resources: $(JS_RESOURCES_DEPS)
	$(AM_V_GEN)mono $(top_builddir)/tools/respack/respack.exe $@ $(JS_RESOURCES)

$(assemblysl_raw): $(system_windows_browser_sources) Makefile $(top_builddir)/class/lib/2.1-bootstrap/System.Windows.dll $(assembly).g.resources
	$(AM_V_GEN)$(SMCS) -target:library -out:$@ $(system_windows_browser_sources) -d:NET_2_1 -r:$(top_builddir)/class/lib/2.1-bootstrap/System.Windows.dll -r:System.Core -r:System.Net -r:System.Xml -r:System.ServiceModel.Web \
		-keyfile:$(srcdir)/../silverlight.pub -delaysign+ \
		-resource:$(assembly).g.resources
	$(AM_V_at)sn -q -R $@ $(srcdir)/../mono.snk

LINKER = ../lib/tuner/monolinker.exe

LINKER_STEPS =	\
	-s ResolveFromAssemblyStep:Mono.Tuner.MoonlightAssemblyStep,Mono.Tuner

LINKER_FLAGS =	\
	-o ../lib/2.1/	\
	-d ../lib/2.1/ -d ../lib/2.1-bootstrap \
	-a $(assemblysl_raw)	\
	-b true	\
	-m secattrs ../tuning/SecurityAttributes

$(assemblysl): $(assemblysl_raw) $(sl_secattr)
	$(AM_V_GEN)MONO_PATH="../lib/tuner:$$MONO_PATH" mono --debug $(LINKER) $(LINKER_FLAGS) $(LINKER_STEPS)
	$(AM_V_at)sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_browser_sources) Makefile $(top_builddir)/class/lib/moonlight-bootstrap/System.Windows.dll $(assembly).g.resources
	$(AM_V_GEN)$(GMCS) -target:library -out:$@ $(system_windows_browser_sources) -d:DESKTOP -r:$(top_builddir)/class/lib/moonlight-bootstrap/System.Windows.dll -r:System.ServiceModel.Web \
		-r:System.Runtime.Serialization \
		-resource:$(assembly).g.resources \
		-keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) 
	-rm -rf *.dll *.mdb tester.cs
	-rm -rf $(assembly).g.resources

if INCLUDE_BROWSER_MANAGED_CODE
install-sl:
	$(INSTALL) -c -m 755  $(assemblysl) $(DESTDIR)$(pkglibdir)/plugin/$(assembly).dll
	test ! -f $(assemblysl).mdb || $(INSTALL) -c -m 755 $(assemblysl).mdb $(DESTDIR)$(pkglibdir)/plugin/$(assembly).dll.mdb
else
install-sl:
endif

if INCLUDE_DESKTOP_MANAGED_CODE
install-desktop:
	$(GACUTIL) /i $(assemblydesktop) /f /package moonlight
else
install-desktop:
endif

install-data-local: install-sl install-desktop

uninstall-local:
	$(GACUTIL) /u $(assembly)
