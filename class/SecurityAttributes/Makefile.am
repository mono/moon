securitymodel_sources = \
	$(srcdir)/CecilRocks.cs			\
	$(srcdir)/PlatformCode.cs

CECIL=-r:`pkg-config --variable=Libraries cecil`
LIBS=$(CECIL) -r:Moonlight.SecurityModel.dll
MONO_PLATFORM_CODE=$(MCS_PATH)/class/lib/net_2_1/
MOONLIGHT_PLATFORM_CODE=$(pkglibdir)/plugin
SILVERLIGHT_PLATFORM_CODE="/cygdrive/c/Program\ Files/Microsoft\ Silverlight/2.0.31005.0/"

SECURITY_ATTRIBUTES_MANUAL_INFO= \
	overrides/mscorlib.manual \
	overrides/System.manual \
	overrides/System.Core.manual \
	overrides/System.Net.manual \
	overrides/System.Runtime.Serialization.manual \
	overrides/System.ServiceModel.manual \
	overrides/System.ServiceModel.Web.manual \
	overrides/System.Windows.Browser.manual \
	overrides/System.Windows.manual \
	overrides/System.Xml.manual \
	overrides/Microsoft.VisualBasic.manual

SECURITY_ATTRIBUTES_COMPATIBILITY_INFO= \
	compatibility/mscorlib.compat.sc \
	compatibility/System.compat.sc \
	compatibility/System.Core.compat.sc \
	compatibility/System.Net.compat.sc \
	compatibility/System.Runtime.Serialization.compat.sc \
	compatibility/System.ServiceModel.compat.sc \
	compatibility/System.ServiceModel.Web.compat.sc \
	compatibility/System.Windows.Browser.compat.sc \
	compatibility/System.Windows.compat.sc \
	compatibility/System.Xml.compat.sc \
	compatibility/Microsoft.VisualBasic.compat.sc

SECATTRS = $(SECURITY_ATTRIBUTES_MANUAL_INFO) $(SECURITY_ATTRIBUTES_COMPATIBILITY_INFO)

EXTRA_DIST = $(SECATTRS) $(securitymodel_sources) find-sc.cs detect.cs audit.cs ivt-check.cs ivt-detect.cs ivt-compat.cs

MERGED_SECURITY_ATTRIBUTES_INFO = \
	mscorlib.secattr \
	System.secattr \
	System.Core.secattr \
	System.Net.secattr \
	System.Runtime.Serialization.secattr \
	System.ServiceModel.secattr \
	System.ServiceModel.Web.secattr \
	System.Windows.Browser.secattr \
	System.Windows.secattr \
	System.Xml.secattr \
	Microsoft.VisualBasic.secattr

LINKER = monolinker

LINKER_STEPS =	\
	-s ResolveFromAssemblyStep:Mono.Tuner.MoonlightAssemblyStep,Mono.Tuner

LINKER_FLAGS =	\
	-o ../lib/2.1/	\
	-d ../lib/2.1/	\
	-b true	\
	-m secattrs .

all: ivt-check detect.exe
	mono --debug detect.exe -v -v -v -a ../lib/2.1/ .
	for i in $(TRUSTED_ASSEMBLIES); do \
		MONO_PATH="../lib/tuner:$$MONO_PATH" $(LINKER) -a $$i $(LINKER_FLAGS) $(LINKER_STEPS); \
		sn -q -R $$i $(srcdir)/../mono.snk; \
	done

Moonlight.SecurityModel.dll: $(securitymodel_sources)
	mcs $(CECIL) -debug -r:System.Core.dll -t:library -out:$@ $(securitymodel_sources)

find-sc.exe: find-sc.cs Moonlight.SecurityModel.dll
	mcs find-sc.cs -debug -out:$@ $(LIBS)

compat: find-sc.exe
	mkdir -p compatibility
	mono find-sc.exe $(SILVERLIGHT_PLATFORM_CODE) compatibility

detect.exe: detect.cs Moonlight.SecurityModel.dll
	mcs detect.cs -debug -out:$@ $(LIBS)
 
$(MERGED_SECURITY_ATTRIBUTES_INFO): ivt-check detect.exe $(SECURITY_ATTRIBUTES_MANUAL_INFO) $(SECURITY_ATTRIBUTES_COMPATIBILITY_INFO) $(TRUSTED_ASSEMBLIES)

validate: find-sc.exe
	mkdir -p moonlight
	mono find-sc.exe $(MOONLIGHT_PLATFORM_CODE) moonlight
	diff -u compatibility moonlight

audit.exe: $(srcdir)/audit.cs Moonlight.SecurityModel.dll
	mcs $(srcdir)/audit.cs -debug -out:$@ $(LIBS)

ivt-check.exe: $(srcdir)/ivt-check.cs Moonlight.SecurityModel.dll
	mcs $(srcdir)/ivt-check.cs -debug -out:$@ $(LIBS)

ivt-detect.exe: $(srcdir)/ivt-detect.cs Moonlight.SecurityModel.dll
	mcs $(srcdir)/ivt-detect.cs -debug -out:$@ $(LIBS)

ivt-compat.exe: $(srcdir)/ivt-compat.cs Moonlight.SecurityModel.dll
	mcs $(srcdir)/ivt-compat.cs -debug -out:$@ $(LIBS)

# everything that we install in the plugin directory is (somewhat) trusted - even non-platform assemblies
TRUSTED_ASSEMBLIES= \
	../lib/2.1/mscorlib.dll \
	../lib/2.1/System.dll \
	../lib/2.1/System.Core.dll \
	../lib/2.1/System.Net.dll \
	../lib/2.1/System.Runtime.Serialization.dll \
	../lib/2.1/System.ServiceModel.dll \
	../lib/2.1/System.ServiceModel.Web.dll \
	../lib/2.1/System.Windows.dll \
	../lib/2.1/System.Windows.Browser.dll \
	../lib/2.1/System.Xml.dll
	

if HAVE_MONO_BASIC
TRUSTED_ASSEMBLIES += ../lib/2.1/Microsoft.VisualBasic.dll
endif

ivt-check: ivt-check.exe
	MONO_PATH="../lib/tuner:$$MONO_PATH" mono ivt-check.exe $(TRUSTED_ASSEMBLIES)

ivt-compat: ivt-compat.exe
	MONO_PATH="../lib/tuner:$$MONO_PATH" mono --debug ivt-compat.exe $(MOONLIGHT_PLATFORM_CODE) sl-4-sdk.internals

clean-data:
	rm -f *.secattr

clean: clean-data
	rm -f find-sc.exe detect.exe audit.exe ivt-check.exe Moonlight.SecurityModel.dll *.mdb
