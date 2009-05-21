SMCS = $(MONO) --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe

SOURCES = \
    $(srcdir)/UnitTestHarness/AsynchronousAttribute.cs\
    $(srcdir)/UnitTestHarness/BugAttribute.cs\
    $(srcdir)/UnitTestHarness/CodeCoverage.cs\
    $(srcdir)/UnitTestHarness/CustomFrameworkUnitTest.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/CodeCoverageProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/EnvironmentProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/Extensions.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/IsolatedStorageSettingsProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/ProviderBase.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/ServiceResult.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/ServiceType.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/SettingsProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/SilverlightTestService.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/SilverlightTestService.ServiceVerifier.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/TestReportingProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/TestServiceFeature.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebCodeCoverageProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebEnvironmentProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebServiceResult.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebSettingsProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebTestReportingProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebTestService.CrossThreadState.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebTestService.cs\
    $(srcdir)/UnitTestHarness/Harness/Service/WebTestService.RequestData.cs\
    $(srcdir)/UnitTestHarness/Harness/TestServiceProvider.cs\
    $(srcdir)/UnitTestHarness/TagAttribute.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/HarnessExtensions.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/IProvideDynamicTestMethods.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/RetryTestRunFilter.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TagManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TagManager.ExpressionEvaluator.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TagManager.Tags.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TagTestRunFilter.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestAssemblyHelper.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestClassHelper.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestMethodHelper.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestMethodManager.Silverlight.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestRunFilter.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestLogicFactory.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/VisualStudioLogProvider.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/VisualStudioLogProvider.Writer.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/TextFailuresLogProvider.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/BrowserInformation.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/HtmlPropertyGrid.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/ResultInspector.Details.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/ResultInspector.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/ResultInspector.ExceptionDetails.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/TestPage.xaml.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/TestRunProgress.cs\
    $(srcdir)/UnitTestHarness/Properties/UnitTestMessage.Designer.cs\
    $(srcdir)/UnitTestHarness/UnitTestSystem.Silverlight.cs\
    $(srcdir)/UnitTestHarness/WorkItemTest.cs\
    $(srcdir)/UnitTestHarness/ExclusiveAttribute.cs\
    $(srcdir)/UnitTestHarness/GlobalSuppressions.cs\
    $(srcdir)/UnitTestHarness/IWorkItem.cs\
    $(srcdir)/UnitTestHarness/PresentationTest.cs\
    $(srcdir)/UnitTestHarness/SilverlightTest.cs\
    $(srcdir)/UnitTestHarness/WorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestLogMessageWriter.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestHarness.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestLogDecorator.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestMessageConditional.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/LazyMethodAssemblyInfo.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IAssembly.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IExpectedException.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IHostType.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IPriority.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/ITestClass.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/ITestMethod.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/ITestProperty.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IUnitTestProvider.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/IWorkItemMetadata.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/StringEventArgs.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/UnitTestProviderCapabilities.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/SleepWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/BrowserUtility.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/FailureEntry.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/FailureSummaryControl.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/TestHtmlElementManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/ExpectedException.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/Priority.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/ProviderAttributes.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/TestClass.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/TestMethod.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/TestProperty.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/UnitTestContext.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/UnitTestFrameworkAssembly.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/VsttProvider.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/VisualStudio/WorkItemMetadata.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/FailureControl.cs\
    $(srcdir)/UnitTestHarness/UnitTestSystem.cs\
    $(srcdir)/UnitTestHarness/Properties/AssemblyInfo.cs\
    $(srcdir)/UnitTestHarness/UnitTestSettings.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/CurrentTestStatus.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/TestPanelManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/UI/UnitTestWebpageLog.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/ScenarioResult.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/AssemblyManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/ICompositeWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/LazyMethodInfo.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/MethodContainer.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestClassInstanceDictionary.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestClassManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/WorkItemsManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/TestMethodManager.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/CompositeWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestMethodContainer.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Harness/UnitTestCompositeWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/CallbackWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/ConditionalWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/MethodInvokeWorkItem.cs\
    $(srcdir)/UnitTestHarness/UnitTesting/Metadata/UnitTestProviders.cs\
    $(srcdir)/UnitTestHarness/Harness/DecoratorDictionary.cs\
    $(srcdir)/UnitTestHarness/Harness/ResourceUtility.cs\
    $(srcdir)/UnitTestHarness/Harness/LogMessageFactory.cs\
    $(srcdir)/UnitTestHarness/Harness/LogMessageWriter.cs\
    $(srcdir)/UnitTestHarness/Harness/LogDecorator.cs\
    $(srcdir)/UnitTestHarness/Harness/TestOutcome.cs\
    $(srcdir)/UnitTestHarness/Harness/TestStage.cs\
    $(srcdir)/UnitTestHarness/Harness/LogProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/TestHarness.cs\
    $(srcdir)/UnitTestHarness/Harness/GlobalExceptionHandler.cs\
    $(srcdir)/UnitTestHarness/Harness/LogMessage.cs\
    $(srcdir)/UnitTestHarness/Harness/TestGranularity.cs\
    $(srcdir)/UnitTestHarness/Harness/DebugOutputProvider.cs\
    $(srcdir)/UnitTestHarness/Harness/ITestHarness.cs\
    $(srcdir)/UnitTestHarness/Harness/LogMessageType.cs\
    $(srcdir)/UnitTestHarness/Harness/ITestSettingsLogProvider.cs\
    $(srcdir)/UnitTestHarness/UI/WebpageHeaderLogProvider.cs\
    $(srcdir)/UnitTestHarness/UI/WebpageLogProvider.cs\
    $(srcdir)/UnitTestHarness/UI/WebpageLogProviderBase.cs\
    $(srcdir)/UnitTestHarness/Harness/TestHarnessCompletedEventArgs.cs\
    $(srcdir)/UnitTestHarness/Harness/RunDispatcher.cs\
    $(srcdir)/UnitTestHarness/Harness/TestHarnessSettings.cs\
    $(srcdir)/UnitTestHarness/UI/HtmlTestColumn.cs\
    $(srcdir)/UnitTestHarness/Html/CssPosition.cs\
    $(srcdir)/UnitTestHarness/Html/EventAttachment.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlControlBase.cs\
    $(srcdir)/UnitTestHarness/Html/BorderStyle.cs\
    $(srcdir)/UnitTestHarness/Html/CssDisplay.cs\
    $(srcdir)/UnitTestHarness/Html/CssSize.cs\
    $(srcdir)/UnitTestHarness/Html/CssAttribute.cs\
    $(srcdir)/UnitTestHarness/Html/CssMargin.cs\
    $(srcdir)/UnitTestHarness/Html/CssPadding.cs\
    $(srcdir)/UnitTestHarness/Html/CssBox.cs\
    $(srcdir)/UnitTestHarness/Html/FontInfo.cs\
    $(srcdir)/UnitTestHarness/Html/FontNamesConverter.cs\
    $(srcdir)/UnitTestHarness/Html/FontSize.cs\
    $(srcdir)/UnitTestHarness/Html/FontUnit.cs\
    $(srcdir)/UnitTestHarness/Html/FontUnitConverter.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlAnchor.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlAttribute.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlButton.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlContainerControl.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlControlCollection.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlControlManager.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlDiv.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlElementWrapperBag.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlElementWrapper.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlEvent.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlHeaderControl.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlInputControl.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlInputText.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlLineBreak.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlPageControl.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlProperty.cs\
    $(srcdir)/UnitTestHarness/Html/Box.cs\
    $(srcdir)/UnitTestHarness/Html/CssBorder.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlControl.cs\
    $(srcdir)/UnitTestHarness/Html/Extensions.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlSpan.cs\
    $(srcdir)/UnitTestHarness/Html/HtmlTag.cs\
    $(srcdir)/UnitTestHarness/UI/Color.cs\
    $(srcdir)/UnitTestHarness/Html/BrowserScreenInformation.cs\
    $(srcdir)/UnitTestHarness/Html/Paragraph.cs\
    $(srcdir)/UnitTestHarness/Html/Size.cs\
    $(srcdir)/UnitTestHarness/Html/StyleProvider.cs\
    $(srcdir)/UnitTestHarness/Html/Unit.cs\
    $(srcdir)/UnitTestHarness/Html/UnitConverter.cs\
    $(srcdir)/UnitTestHarness/Html/UnitType.cs\
    $(srcdir)/UnitTestHarness/UI/ResultIndicatorColor.cs\
    $(srcdir)/UnitTestHarness/UI/TestColumnHeader.cs\
    $(srcdir)/UnitTestHarness/UI/TestStyleProvider.cs\
    $(srcdir)/UnitTestHarness/UI/WebBrowserTestPage.cs\
    $(srcdir)/UnitTestHarness/UI/WebBrowserTick.cs\
    $(srcdir)/UnitTestHarness/Harness/TestHarnessState.cs\
    $(srcdir)/UnitTestHarness/Harness/ReflectionUtility.cs

GENERATED_SOURCES =	\
	$(srcdir)/UnitTestHarness/UnitTesting/UI/TestPage.xaml.g.cs

$(srcdir)/UnitTestHarness/UnitTesting/UI/TestPage.xaml.g.cs: $(srcdir)/UnitTestHarness/UnitTesting/UI/TestPage.xaml
	$(MONO) --debug $(top_builddir)/tools/xamlg/xamlg.exe UnitTestHarness/UnitTesting/UI/TestPage.xaml,UnitTestHarness/UnitTesting/UI/TestPage.xaml.g.cs -sl2app:Microsoft.Silverlight.Testing -root:UnitTestHarness

EXTRA_DIST =			\
	$(SOURCES)		\
	$(srcdir)/UnitTestHarness/UnitTesting/UI/TestPage.xaml \
	$(srcdir)/UnitTestHarness/Properties/UnitTestMessage.resources	\
	$(srcdir)/UnitTestHarness/Properties/UnitTestMessage.resx

RESOURCES = \
    -res:$(srcdir)/UnitTestHarness/Properties/UnitTestMessage.resources,Microsoft.Silverlight.Testing.Properties.UnitTestMessage.resources \
	-res:$(srcdir)/Microsoft.Silverlight.Testing.g.resources,Microsoft.Silverlight.Testing.g.resources

# System.Xml.Linq.dll requires full path, otherwise the compiler references the one in the gac 
REFERENCES = \
	-r:System \
	-r:System.Net \
	-r:System.Xml \
	-r:$(CURDIR)/System.Xml.Linq.dll \
	-r:System.Core \
	-r:$(top_builddir)/test/2.0/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll

BROWSER_ASSEMBLIES=-r:$(top_builddir)/class/lib/2.1/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.Windows.Browser.dll

noinst_DATA=	Microsoft.Silverlight.Testing.dll

CLEANFILES= 			\
	System.Xml.Linq.dll	\
	System.Xml.Linq.dll.mdb	\
	System.Xml.Linq.dll.Unsigned Microsoft.Silverlight.Testing.dll	\
	Microsoft.Silverlight.Testing.dll.mdb \
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb

KEYFILE=$(srcdir)/../moonlight.snk

Microsoft.Silverlight.Testing.dll: ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll UnitTestHarness/Properties/UnitTestMessage.resources $(SOURCES) System.Xml.Linq.dll Microsoft.Silverlight.Testing.g.resources $(GENERATED_SOURCES)
	MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" $(SMCS) $(SOURCES) $(GENERATED_SOURCES) $(RESOURCES) $(REFERENCES) $(BROWSER_ASSEMBLIES) -noconfig -define:SILVERLIGHT -target:library -out:Microsoft.Silverlight.Testing.dll -debug:full -delaysign+ -keyfile:$(KEYFILE)
	sn -q -R $@ $(KEYFILE)

Microsoft.Silverlight.Testing.g.resources: UnitTestHarness/UnitTesting/UI/TestPage.xaml
	$(MONO) --debug $(top_builddir)/tools/respack/respack.exe Microsoft.Silverlight.Testing.g.resources UnitTestHarness/UnitTesting/UI/TestPage.xaml,unittesting/ui/testpage.xaml

UnitTestHarness/Properties/UnitTestMessage.resources: $(srcdir)/UnitTestHarness/Properties/UnitTestMessage.resx
	resgen2 $(srcdir)/UnitTestHarness/Properties/UnitTestMessage.resx

SYSTEM_XML_LINQ_DLL = $(top_builddir)/class/lib/2.1/System.Xml.Linq.dll

System.Xml.Linq.dll: $(SYSTEM_XML_LINQ_DLL)
	$(MONO) --debug $(top_builddir)/tools/unsign/unsign.exe $(SYSTEM_XML_LINQ_DLL) --resign $(KEYFILE)
	mv System.Xml.Linq.dll.Unsigned System.Xml.Linq.dll
	-cp $(SYSTEM_XML_LINQ_DLL).mdb .

../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	$(MAKE) -C ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/
