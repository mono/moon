xapname=moon-unit

all-local: copy-xap-to-site

noinst_DATA= $(xapname).xap

CLEANFILES=	$(xapname).xap 		\
		$(xapname).dll		\
		$(xapname).dll.mdb	\
		$(xapname).sources	\
		*.xaml.g.cs		\
		AppManifest.xaml 	\
		$(xapname).g.resources	\
		Microsoft.Silverlight.Testing.dll \
		Microsoft.Silverlight.Testing.dll.mdb \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb \
		MoonAtkBridge.dll \
		System.Windows.Controls.dll	\
		System.Windows.Controls.dll.mdb	\
		System.Xml.Linq.dll	\
		System.Xml.Linq.dll.mdb	\
		Mono.Moonlight.UnitTesting.dll	\
		Mono.Moonlight.UnitTesting.dll.mdb	\
		site/ClientBin/moon-unit.xap

BROWSER_ASSEMBLIES=-r:$(top_builddir)/class/lib/2.1/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.Windows.Browser.dll

XAML_RESOURCES= \
	-res:misc/Events/Event_0.xaml,misc/Events/Event_0.xaml \
	-res:misc/Events/Event_1.xaml,misc/Events/Event_1.xaml \
	-res:misc/Events/Event_3.xaml,misc/Events/Event_3.xaml \
	-res:misc/Events/Event_Basic.xaml,misc/Events/Event_Basic.xaml \
	-res:misc/Events/Event_CaseMismatch.xaml,misc/Events/Event_CaseMismatch.xaml \
	-res:misc/Events/Event_DO.xaml,misc/Events/Event_DO.xaml \
	-res:misc/Events/Event_Exact.xaml,misc/Events/Event_Exact.xaml \
	-res:misc/Events/Event_ExactCaseMismatch.xaml,misc/Events/Event_ExactCaseMismatch.xaml \
	-res:misc/Events/Event_Ex_Derived2.xaml,misc/Events/Event_Ex_Derived2.xaml \
	-res:misc/Events/Event_Ex_Derived3.xaml,misc/Events/Event_Ex_Derived3.xaml \
	-res:misc/Events/Event_Ex_Derived.xaml,misc/Events/Event_Ex_Derived.xaml \
	-res:misc/Events/Event_InexistentResource.xaml,misc/Events/Event_InexistentResource.xaml \
	-res:misc/Events/Event_Internal.xaml,misc/Events/Event_Internal.xaml \
	-res:misc/Events/Event_MouseArgs.xaml,misc/Events/Event_MouseArgs.xaml \
	-res:misc/Events/Event_NonVoid.xaml,misc/Events/Event_NonVoid.xaml \
	-res:misc/Events/Event_ObjectObject.xaml,misc/Events/Event_ObjectObject.xaml \
	-res:misc/Events/Event_OverloadedInvalid.xaml,misc/Events/Event_OverloadedInvalid.xaml \
	-res:misc/Events/Event_OverloadedValid.xaml,misc/Events/Event_OverloadedValid.xaml \
	-res:misc/Events/Event_Private.xaml,misc/Events/Event_Private.xaml \
	-res:misc/Events/Event_Protected.xaml,misc/Events/Event_Protected.xaml \
	-res:misc/Events/Event_RoutedEventArgs.xaml,misc/Events/Event_RoutedEventArgs.xaml	\
	-res:misc/Parsing/MiscParsingAnimation.xaml,misc/Parsing/MiscParsingAnimation.xaml	\
	-res:misc/Parsing/MiscParsingCanvas.xaml,misc/Parsing/MiscParsingCanvas.xaml	\
	-res:misc/Parsing/MiscParsingEvent.xaml,misc/Parsing/MiscParsingEvent.xaml	\
	-res:misc/Parsing/MiscParsingManagedAttachedProp.xaml,misc/Parsing/MiscParsingManagedAttachedProp.xaml	\
	-res:misc/Parsing/MiscParsingPrivateControl.xaml,misc/Parsing/MiscParsingPrivateControl.xaml	\
	-res:misc/Parsing/MiscParsingTestPage.xaml,misc/Parsing/MiscParsingTestPage.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_A1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_A2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_A3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A3.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_B1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_B2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_B3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B3.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_C1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_C2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_C3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C3.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_D1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_D2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_D3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D3.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_D4.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D4.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_E1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_E1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_E2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_E2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_F1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F1.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_F2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F2.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_F3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F3.xaml	\
	-res:System.Windows/DependencyPropertyTest_ManagedTest_F4.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F4.xaml	\
	-res:System.Windows/ResourceDictionarySourcePropertyTest.xaml,System.Windows/ResourceDictionarySourcePropertyTest.xaml		\
	-cres:System.Windows/ResourceDictionarySourcePropertyTest.xaml,System.Windows/ResourceDictionarySourcePropertyTest.xaml		\
	-res:System.Windows/VSMControl.xaml,System.Windows/VSMControl.xaml		\
	-res:System.Windows.Data/BindingConverter.xaml	\
	-res:System.Windows.Controls/ControlTemplateTest.xaml,System.Windows.Controls/ControlTemplateTest.xaml

SOURCES= \
	$(wildcard $(srcdir)/*.xaml.cs)	\
	$(wildcard $(srcdir)/*/*.cs)	\
	$(wildcard $(srcdir)/*/*/*.cs)	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/BaseDomainPolicy.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/ClientAccessPolicy.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/ClientAccessPolicyParser.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/CrossDomainPolicyManager.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/FlashCrossDomainPolicy.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/FlashCrossDomainPolicyParser.cs	\
	../../../class/System.Windows.Browser/System.Windows.Browser.Net/ICrossDomainPolicy.cs

REFERENCES = \
	-r:$(top_builddir)/test/2.0/moon-unit/System.Windows.Controls.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/Microsoft.Silverlight.Testing.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	-r:$(top_builddir)/test/2.0/moon-unit/Mono.Moonlight.UnitTesting.dll

moon-unit.xap: moon-unit.sources Microsoft.Silverlight.Testing.dll Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll System.Xml.Linq.dll Mono.Moonlight.UnitTesting.dll System.Windows.Controls.dll MoonAtkBridge.dll
	$(MONO) --debug $(top_builddir)/tools/mxap/mxap.exe -d:TEST -d:NET_2_1 --generate-html=false --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=$(top_builddir)/test/2.0/moon-unit/moon-unit.sources --builddirhack=$(top_builddir) $(BROWSER_ASSEMBLIES) $(REFERENCES)  $(srcdir)/ $(XAML_RESOURCES)

MoonAtkBridge.dll : ../FakeA11yBridge/MoonAtkBridge.dll
	cp  ../FakeA11yBridge/MoonAtkBridge.dll* .

System.Windows.Controls.dll : ../Microsoft.SilverlightControls/System.Windows.Controls.dll
	cp ../Microsoft.SilverlightControls/System.Windows.Controls.dll* .

Microsoft.Silverlight.Testing.dll: ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll
	cp ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll* .

System.Xml.Linq.dll: ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll
	cp ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll* .

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll: ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll
	cp ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll* .

Mono.Moonlight.UnitTesting.dll: ../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll
	cp ../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll* .

../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll:
	$(MAKE) -C ../Microsoft.Silverlight.Testing

../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	$(MAKE) -C ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight

../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll:
	$(MAKE) -C ../Mono.Moonlight.UnitTesting

../FakeA11yBridge/MoonAtkBridge.dll:
	$(MAKE) -C ../FakeA11yBridge

moon-unit.sources: $(SOURCES)
	ls $(SOURCES) | grep -v aspx > $@

EXTRA_DIST = 			\
	$(srcdir)/*xaml.cs	\
	$(srcdir)/*xaml		\
	$(srcdir)/*/*xaml	\
	$(srcdir)/*/*/*xaml	\
	$(srcdir)/*/*.cs

test: copy-xap-to-site
	MONO_PATH=$(top_builddir)/class/lib/2.1:$(MONO_PATH) LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(top_builddir)/plugin/.libs firefox site/index.html -P MoonTest --no-remote

copy-xap-to-site: site/ClientBin/moon-unit.xap
	cp -p $(srcdir)/../Mono.Moonlight.UnitTesting/site/* site/

site/ClientBin/moon-unit.xap: moon-unit.xap
	@mkdir -p site/ClientBin
	@cp moon-unit.xap site/ClientBin

install:

.PHONY: copy-xap-to-site
