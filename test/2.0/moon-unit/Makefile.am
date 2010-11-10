.NOTPARALLEL:

xapname=moon-unit
BROWSER_NAME=firefox
BROWSER_OPTS=-P MoonTest --no-remote

all-local: copy-xaps-to-site

xaps= $(xapname)2.xap \
      $(xapname)3.xap \
      $(xapname)4.xap

clientbin_xaps= $(xaps:%=site/ClientBin/%)

noinst_DATA= $(xaps)

CLEANFILES=	$(xaps) 		\
		moon-unit.dll		\
		moon-unit.dll.mdb	\
		moon-unit.g.resources	\
		moon-unit.sources	\
		$(CODE_BEHINDS)		\
		AppManifest.xaml 	\
		Microsoft.Silverlight.Testing.dll \
		Microsoft.Silverlight.Testing.dll.mdb \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb \
		MoonAtkBridge.dll \
		System.Windows.Controls.dll	\
		System.Windows.Controls.dll.mdb	\
		System.Windows.Controls.Data.dll	\
		System.Windows.Controls.Data.dll.mdb	\
		System.Xml.Linq.dll	\
		System.Xml.Linq.dll.mdb	\
		Mono.Moonlight.UnitTesting.dll	\
		Mono.Moonlight.UnitTesting.dll.mdb	\
		moon-unit.xap				\
		$(clientbin_xaps)

BROWSER_ASSEMBLIES=-r:$(top_builddir)/class/lib/2.1/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.Windows.Browser.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.Runtime.Serialization.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.ServiceModel.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.ServiceModel.Web.dll

moon_unit_dll_RESOURCES= \
	$(srcdir)/../../media/video/timecode-long-with-audio.wmv,timecode-long-with-audio.wmv \
	misc/Events/Event_0.xaml,misc/Events/Event_0.xaml \
	misc/Events/Event_1.xaml,misc/Events/Event_1.xaml \
	misc/Events/Event_3.xaml,misc/Events/Event_3.xaml \
	misc/Events/Event_Basic.xaml,misc/Events/Event_Basic.xaml \
	misc/Events/Event_CaseMismatch.xaml,misc/Events/Event_CaseMismatch.xaml \
	misc/Events/Event_DO.xaml,misc/Events/Event_DO.xaml \
	misc/Events/Event_Exact.xaml,misc/Events/Event_Exact.xaml \
	misc/Events/Event_ExactCaseMismatch.xaml,misc/Events/Event_ExactCaseMismatch.xaml \
	misc/Events/Event_Ex_Derived2.xaml,misc/Events/Event_Ex_Derived2.xaml \
	misc/Events/Event_Ex_Derived3.xaml,misc/Events/Event_Ex_Derived3.xaml \
	misc/Events/Event_Ex_Derived.xaml,misc/Events/Event_Ex_Derived.xaml \
	misc/Events/Event_InexistentResource.xaml,misc/Events/Event_InexistentResource.xaml \
	misc/Events/Event_Internal.xaml,misc/Events/Event_Internal.xaml \
	misc/Events/Event_MouseArgs.xaml,misc/Events/Event_MouseArgs.xaml \
	misc/Events/Event_NonVoid.xaml,misc/Events/Event_NonVoid.xaml \
	misc/Events/Event_ObjectObject.xaml,misc/Events/Event_ObjectObject.xaml \
	misc/Events/Event_OverloadedInvalid.xaml,misc/Events/Event_OverloadedInvalid.xaml \
	misc/Events/Event_OverloadedValid.xaml,misc/Events/Event_OverloadedValid.xaml \
	misc/Events/Event_Private.xaml,misc/Events/Event_Private.xaml \
	misc/Events/Event_Protected.xaml,misc/Events/Event_Protected.xaml \
	misc/Events/Event_RoutedEventArgs.xaml,misc/Events/Event_RoutedEventArgs.xaml	\
	misc/Parsing/MiscParsingAnimation.xaml,misc/Parsing/MiscParsingAnimation.xaml	\
	misc/Parsing/MiscParsingCanvas.xaml,misc/Parsing/MiscParsingCanvas.xaml	\
	misc/Parsing/MiscParsingEvent.xaml,misc/Parsing/MiscParsingEvent.xaml	\
	misc/Parsing/MiscParsingManagedAttachedProp.xaml,misc/Parsing/MiscParsingManagedAttachedProp.xaml	\
	misc/Parsing/MiscParsingPrivateControl.xaml,misc/Parsing/MiscParsingPrivateControl.xaml	\
	misc/Parsing/MiscParsingTestPage.xaml,misc/Parsing/MiscParsingTestPage.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_A1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_A2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_A3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_A3.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_B1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_B2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_B3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_B3.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_C1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_C2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_C3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_C3.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_D1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_D2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_D3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D3.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_D4.xaml,System.Windows/DependencyPropertyTest_ManagedTest_D4.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_E1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_E1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_E2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_E2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_F1.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F1.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_F2.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F2.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_F3.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F3.xaml	\
	System.Windows/DependencyPropertyTest_ManagedTest_F4.xaml,System.Windows/DependencyPropertyTest_ManagedTest_F4.xaml	\
	System.Windows/ResourceDictionarySourcePropertyTest.xaml,System.Windows/ResourceDictionarySourcePropertyTest.xaml		\
	System.Windows/ResourceDictionarySourcePropertyTest_LackingDefaultXmlns.xaml,System.Windows/ResourceDictionarySourcePropertyTest_LackingDefaultXmlns.xaml		\
\
	System.Windows/VSMControl.xaml,System.Windows/VSMControl.xaml		\
	System.Windows.Data/BindingConverter.xaml	\
	System.Windows.Controls/ControlTemplateTest.xaml,System.Windows.Controls/ControlTemplateTest.xaml


moon_unit_xap_RESOURCES = \
	System.Windows/ResourceDictionarySourcePropertyTest.xaml		\
	System.Windows/ResourceDictionarySourcePropertyTest_LackingDefaultXmlns.xaml		

XAML_FILES=$(filter-out AppManifest.xaml,$(wildcard *.xaml))
CODE_BEHINDS=$(XAML_FILES:%.xaml=%.xaml.g.cs)

SOURCES= \
	$(wildcard $(srcdir)/*.xaml.cs)	\
	$(wildcard $(srcdir)/*/*.cs)	\
	$(wildcard $(srcdir)/*/*/*.cs)	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/BaseDomainPolicy.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/ClientAccessPolicy.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/ClientAccessPolicyParser.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/CrossDomainPolicyManager.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/FlashCrossDomainPolicy.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/FlashCrossDomainPolicyParser.cs	\
	$(MCS_PATH)/class/System.Net/System.Net.Policy/ICrossDomainPolicy.cs

REFERENCES = \
	-r:$(top_builddir)/test/2.0/moon-unit/System.Windows.Controls.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/System.Windows.Controls.Data.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/Microsoft.Silverlight.Testing.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	-r:$(top_builddir)/test/2.0/moon-unit/Mono.Moonlight.UnitTesting.dll \
	-r:$(top_builddir)/test/2.0/moon-unit/MoonAtkBridge.dll

DLLS_TO_EMBED = \
	Microsoft.Silverlight.Testing.dll \
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
	System.Xml.Linq.dll \
	Mono.Moonlight.UnitTesting.dll \
	System.Windows.Controls.dll \
	System.Windows.Controls.Data.dll \
	MoonAtkBridge.dll \
	moon-unit.dll

appmanifest_ASSEMBLYPARTS= \
	$(foreach dll,$(DLLS_TO_EMBED),\<AssemblyPart x:Name=\"$(basename $(dll))\" Source=\"$(dll)\"/\>)

appmanifest_NAMESPACES=xmlns=\"http://schemas.microsoft.com/client/2007/deployment\" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\"

appmanifest_contents=$(subst @RUNTIME_VERSION@,$(1),\<Deployment  $(appmanifest_NAMESPACES) EntryPointAssembly=\"moon-unit\" EntryPointType=\"Mono.Moonlight.UnitTesting.App\" RuntimeVersion=\"@RUNTIME_VERSION@\"\>\<Deployment.Parts\>$(appmanifest_ASSEMBLYPARTS)\</Deployment.Parts\>\</Deployment\>)

$(CODE_BEHINDS): $(XAML_FILES)
	@echo Generating code-behinds
	@$(SYSTEM_MONO) ../../../tools/xamlg/xamlg.exe  -sl2app:$(xap_name)  -root:$(srcdir)  $(XAML_FILES)

moon-unit.g.resources:
	@echo Generating moon-unit.g.resources
	@$(SYSTEM_MONO) $(top_builddir)/tools/respack/respack.exe moon-unit.g.resources $(moon_unit_dll_RESOURCES)

# we include moon-unit.dll in the DLLS_TO_EMBED so we don't have to make the appmanifest_contents function even larger,
# but it means we need to filter it out here or else we have a circular dependency (which gmake complains about and drops)
moon-unit.dll: moon-unit.sources moon-unit.g.resources $(CODE_BEHINDS) $(filter-out moon-unit.dll,$(DLLS_TO_EMBED))
	@echo Compiling moon-unit.dll
	@MONO_PATH=$(top_builddir)/class/lib/2.1 $(MOONLIGHT_MONO) --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe -d:TEST -d:NET_2_1 -debug+ -target:library -out:moon-unit.dll $(BROWSER_ASSEMBLIES) $(REFERENCES) @$(top_builddir)/test/2.0/moon-unit/moon-unit.sources $(CODE_BEHINDS) -resource:moon-unit.g.resources -warnaserror+


define XAP_TEMPLATE
$(1): Makefile.am moon-unit.sources $(DLLS_TO_EMBED) moon-unit.dll
	 @echo Packaging $(1)
	 @echo $(call appmanifest_contents,$(2)) > AppManifest.xaml
	 @$(ZIP) -q $(1) AppManifest.xaml moon-unit.dll $(DLLS_TO_EMBED) $(moon_unit_xap_RESOURCES)
	 @rm -f AppManifest.xaml
endef

$(eval $(call XAP_TEMPLATE,moon-unit2.xap,$(SILVERLIGHT_2_0_TARGETRUNTIME)))
$(eval $(call XAP_TEMPLATE,moon-unit3.xap,$(SILVERLIGHT_3_0_TARGETRUNTIME)))
$(eval $(call XAP_TEMPLATE,moon-unit4.xap,$(SILVERLIGHT_4_0_TARGETRUNTIME)))



# rules to copy DLLS_TO_EMBED into this directory

MoonAtkBridge.dll : ../FakeA11yBridge/MoonAtkBridge.dll
	@cp  ../FakeA11yBridge/MoonAtkBridge.dll* .

System.Windows.Controls.dll : ../Microsoft.SilverlightControls/System.Windows.Controls.dll
	@cp ../Microsoft.SilverlightControls/System.Windows.Controls.dll* .

System.Windows.Controls.Data.dll : ../Microsoft.SilverlightControls/System.Windows.Controls.Data.dll
	@cp ../Microsoft.SilverlightControls/System.Windows.Controls.Data.dll* .

Microsoft.Silverlight.Testing.dll: ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll
	@cp ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll* .

System.Xml.Linq.dll: ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll
	@cp ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll* .

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll: ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll
	@cp ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll* .

Mono.Moonlight.UnitTesting.dll: ../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll
	@cp ../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll* .

../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll:
	$(MAKE) -C ../Microsoft.Silverlight.Testing

../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	$(MAKE) -C ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight

../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll:
	$(MAKE) -C ../Mono.Moonlight.UnitTesting

../FakeA11yBridge/MoonAtkBridge.dll:
	$(MAKE) -C ../FakeA11yBridge

moon-unit.sources: $(SOURCES)
	@ls $(SOURCES) | grep -v aspx | grep -v obj/Debug > $@

EXTRA_DIST = 			\
	$(srcdir)/*xaml.cs	\
	$(srcdir)/*xaml		\
	$(srcdir)/*/*xaml	\
	$(srcdir)/*/*/*xaml	\
	$(srcdir)/*/*.cs	\
	$(srcdir)/mono-logo.png

test: copy-xaps-to-site
	MONO_PATH=$(top_builddir)/class/lib/2.1:$(MONO_PATH) \
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(top_builddir)/plugin/.libs \
	$(BROWSER_NAME) $(BROWSER_OPTS) "file://`pwd`/site/index4$(RUNTIME_VERSION).html?$(TEST_NAME)"

test-all: copy-xaps-to-site
	MONO_PATH=$(top_builddir)/class/lib/2.1:$(MONO_PATH) \
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(top_builddir)/plugin/.libs \
	$(BROWSER_NAME) $(BROWSER_OPTS) "file://`pwd`/site/index4.html?$(TEST_NAME)" "file://`pwd`/site/index3.html?$(TEST_NAME)" "file://`pwd`/site/index2.html?$(TEST_NAME)"

debug: copy-xaps-to-site
	MONO_XDEBUG=1 MONO_PATH=$(top_builddir)/class/lib/2.1:$(MONO_PATH) \
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(top_builddir)/plugin/.libs \
	$(BROWSER_NAME) $(BROWSER_OPTS) -d gdb "file://`pwd`/site/index4.html?$(TEST_NAME)"

AUTOMATION_NAMESPACES="MoonTest.System.Windows.Automation+MoonTest.System.Windows.Automation.Peers"

test-automation:
	TEST_NAME=$(AUTOMATION_NAMESPACES) $(MAKE) test

test-moonlight:
	TEST_NAME="!($(AUTOMATION_NAMESPACES))" $(MAKE) test

test-all-moonlight:
	TEST_NAME="!($(AUTOMATION_NAMESPACES))" $(MAKE) test-all

copy-xaps-to-site: $(clientbin_xaps) site/ClientBin/timecode-long.wmv site/ClientBin/images/mono-logo.png
	cp -p $(srcdir)/../Mono.Moonlight.UnitTesting/site/* site/

$(clientbin_xaps): $(xaps)
	@mkdir -p site/ClientBin
	@cp $(xaps) site/ClientBin/
	@cp $(srcdir)/../../media/video/timecode-long.wmv site/ClientBin/

site/ClientBin/timecode-long.wmv: $(srcdir)/../../media/video/timecode-long.wmv
	@cp $(srcdir)/../../media/video/timecode-long.wmv site/ClientBin/

site/ClientBin/images/mono-logo.png: $(srcdir)/mono-logo.png
	@mkdir -p site/ClientBin/images/
	@cp $(srcdir)/mono-logo.png site/ClientBin/images/

install:

.PHONY: copy-xap-to-site
