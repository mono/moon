assembly = System.Windows.Browser
assemblysl = ../lib/2.1/$(assembly).dll
assemblydesktop = ../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/3.0
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib
monoloc = `pkg-config --variable=libdir mono`/mono/3.0
nunit = -r:$(monoloc)/nunit.framework.dll -r:$(monoloc)/nunit.core.dll -r:$(monoloc)/nunit.util.dll 

system_windows_browser_sources =					\
	$(srcdir)/../System.Windows/Assembly/MonoTODOAttribute.cs			\
	$(srcdir)/../Consts.cs							\
	$(srcdir)/Assembly/AssemblyInfo.cs					\
	$(srcdir)/System.Windows.Browser.Net/BrowserHttpWebRequest.cs		\
	$(srcdir)/System.Windows.Browser.Net/BrowserHttpWebResponse.cs		\
	$(srcdir)/System.Windows.Browser.Net/BrowserHttpWebAsyncResult.cs		\
	$(srcdir)/System.Windows.Browser.Net/SoapHttpClientProtocol.cs		\
	$(srcdir)/System.Windows.Browser.Serialization/JavaScriptConverter.cs	\
	$(srcdir)/System.Windows.Browser.Serialization/JavaScriptSerializer.cs	\
	$(srcdir)/System.Windows.Browser.Serialization/JavaScriptTypeResolver.cs	\
	$(srcdir)/System.Windows.Browser.Serialization/ScriptIgnoreAttribute.cs	\
	$(srcdir)/System.Windows.Browser.Serialization/SimpleTypeResolver.cs	\
	$(srcdir)/System.Windows.Browser/BrowserInformation.cs			\
	$(srcdir)/System.Windows.Browser/BrowserRuntimeSettings.cs		\
	$(srcdir)/System.Windows.Browser/HtmlDocument.cs				\
	$(srcdir)/System.Windows.Browser/HtmlElement.cs				\
	$(srcdir)/System.Windows.Browser/HtmlElementCollection.cs			\
	$(srcdir)/System.Windows.Browser/HtmlEventArgs.cs				\
	$(srcdir)/System.Windows.Browser/HtmlObject.cs				\
	$(srcdir)/System.Windows.Browser/HtmlPage.cs				\
	$(srcdir)/System.Windows.Browser/HtmlTimer.cs				\
	$(srcdir)/System.Windows.Browser/HtmlWindow.cs				\
	$(srcdir)/System.Windows.Browser/HttpUtility.cs				\
	$(srcdir)/System.Windows.Browser/ScriptableAttribute.cs			\
	$(srcdir)/System.Windows.Browser/ScriptableObject.cs			\
	$(srcdir)/System.Windows.Controls/DialogResult.cs				\
	$(srcdir)/System.Windows.Controls/FileDialogFileInfo.cs			\
	$(srcdir)/System.Windows.Controls/OpenFileDialog.cs			\
	$(srcdir)/System.Windows.Hosting/Callback.cs				\
	$(srcdir)/System.Windows/ScriptableObjectGenerator.cs			\
	$(srcdir)/System.Windows/WebApplication.cs

test_sources = \
	$(srcdir)/Test/System.Windows.Browser.Serialization/JavaScriptSerializerTest.cs		\
	$(srcdir)/Test/System.Windows.Browser.Serialization/SimpleTypeResolverTest.cs		\
	$(srcdir)/Test/System.Windows/WebApplicationTest.cs

EXTRA_DIST = $(system_windows_browser_sources) 				\
	$(test_sources)							\
	Assembly/ChangeLog						\
	System.Windows.Browser/ChangeLog				\
	System.Windows.Browser.Net/ChangeLog				\
	System.Windows.Browser.Serialization/ChangeLog			\
	System.Windows/ChangeLog					\
	System.Windows.Controls/ChangeLog				\
	System.Windows.Hosting/ChangeLog				\
	ChangeLog

CLEANFILES =			\
	TestResult-net_3_0.log	\
	TestResult-net_3_0.xml

$(assemblysl): $(system_windows_browser_sources) Makefile
	$(SMCS) -target:library -out:$@ $(system_windows_browser_sources) -d:NET_2_1 -r:System.Net -keyfile:$(srcdir)/../silverlight.pub -delaysign+
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_browser_sources) Makefile
	$(GMCS) -target:library -out:$@ $(system_windows_browser_sources) -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) 
	-rm -rf *.dll *.mdb tester.cs

install-data-local:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1

uninstall-local:
	$(GACUTIL) /u $(assembly)

check: $(assemblydesktop) Makefile
	$(GMCS) -out:test.dll -target:library -r:$(assemblydesktop) $(nunit) $(test_sources)
	MONO_PATH="../lib/3.0:$$MONO_PATH" mono --debug $(monoloc)/nunit-console.exe /exclude:NotWorking,ValueAdd,CAS,InetAccess /output:TestResult-net_3_0.log /xml:TestResult-net_3_0.xml test.dll 
