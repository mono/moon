assembly = System.Windows.Browser
assemblysl = ../lib/2.1/$(assembly).dll
assemblydesktop = ../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/3.0
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(prefix)/lib /root $(prefix)/lib
monoloc = @mono_libdir@/mono/3.0
nunit = -r:$(monoloc)/nunit.framework.dll -r:$(monoloc)/nunit.core.dll -r:$(monoloc)/nunit.util.dll 

system_windows_browser_sources =					\
	../System.Windows/Assembly/MonoTODOAttribute.cs			\
	../Consts.cs							\
	Assembly/AssemblyInfo.cs					\
	System.Windows.Browser.Net/BrowserHttpWebRequest.cs		\
	System.Windows.Browser.Net/BrowserHttpWebResponse.cs		\
	System.Windows.Browser.Net/BrowserHttpWebAsyncResult.cs		\
	System.Windows.Browser.Net/SoapHttpClientProtocol.cs		\
	System.Windows.Browser.Serialization/JavaScriptConverter.cs	\
	System.Windows.Browser.Serialization/JavaScriptSerializer.cs	\
	System.Windows.Browser.Serialization/JavaScriptTypeResolver.cs	\
	System.Windows.Browser.Serialization/ScriptIgnoreAttribute.cs	\
	System.Windows.Browser.Serialization/SimpleTypeResolver.cs	\
	System.Windows.Browser/BrowserInformation.cs			\
	System.Windows.Browser/BrowserRuntimeSettings.cs		\
	System.Windows.Browser/HtmlDocument.cs				\
	System.Windows.Browser/HtmlElement.cs				\
	System.Windows.Browser/HtmlElementCollection.cs			\
	System.Windows.Browser/HtmlEventArgs.cs				\
	System.Windows.Browser/HtmlObject.cs				\
	System.Windows.Browser/HtmlPage.cs				\
	System.Windows.Browser/HtmlTimer.cs				\
	System.Windows.Browser/HttpUtility.cs				\
	System.Windows.Browser/ScriptableAttribute.cs			\
	System.Windows.Browser/ScriptableObject.cs			\
	System.Windows.Controls/DialogResult.cs				\
	System.Windows.Controls/FileDialogFileInfo.cs			\
	System.Windows.Controls/OpenFileDialog.cs			\
	System.Windows.Hosting/Callback.cs				\
	System.Windows/ApplicationUnhandledExceptionEventArgs.cs	\
	System.Windows/ScriptableObjectGenerator.cs			\
	System.Windows/WebApplication.cs

test_sources = \
	Test/System.Windows.Browser.Serialization/JavaScriptSerializerTest.cs		\
	Test/System.Windows.Browser.Serialization/SimpleTypeResolverTest.cs		\
	Test/System.Windows/WebApplicationTest.cs

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

$(assemblysl): $(system_windows_browser_sources) Makefile
	$(SMCS) -target:library -out:$@ $(system_windows_browser_sources) -d:NET_2_1 -r:System.Net
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_browser_sources) Makefile
	$(GMCS) -target:library -out:$@ $(system_windows_browser_sources)
	sn -q -R $@ $(srcdir)/../mono.snk

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