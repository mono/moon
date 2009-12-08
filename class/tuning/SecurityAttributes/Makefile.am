securitymodel_sources = \
	$(srcdir)/CecilRocks.cs			\
	$(srcdir)/PlatformCode.cs

CECIL=-r:$(MCS_PATH)/class/lib/net_1_1/Mono.Cecil.dll
LIBS=$(CECIL) -r:Moonlight.SecurityModel.dll
MONO_PLATFORM_CODE=$(MCS_PATH)/class/lib/net_2_1/
MOONLIGHT_PLATFORM_CODE=$(pkglibdir)/plugin
SILVERLIGHT_PLATFORM_CODE="/cygdrive/c/Program\ Files/Microsoft\ Silverlight/2.0.31005.0/"

SECURITY_ATTRIBUTES_AUTOMATIC_INFO= \
	automatic/mscorlib.auto.sc \
	automatic/mscorlib.auto.ssc \
	automatic/System.auto.sc \
	automatic/System.auto.ssc \
	automatic/System.Core.auto.sc \
	automatic/System.Core.auto.ssc \
	automatic/System.Net.auto.sc \
	automatic/System.Net.auto.ssc \
	automatic/System.Runtime.Serialization.auto.sc \
	automatic/System.Runtime.Serialization.auto.ssc \
	automatic/System.ServiceModel.auto.sc \
	automatic/System.ServiceModel.auto.ssc \
	automatic/System.ServiceModel.Web.auto.sc \
	automatic/System.ServiceModel.Web.auto.ssc \
	automatic/System.Windows.Browser.auto.sc \
	automatic/System.Windows.Browser.auto.ssc \
	automatic/System.Windows.auto.sc \
	automatic/System.Windows.auto.ssc \
	automatic/System.Xml.auto.sc \
	automatic/System.Xml.auto.ssc \
	automatic/Microsoft.VisualBasic.auto.sc \
	automatic/Microsoft.VisualBasic.auto.ssc

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

SECATTRS = $(SECURITY_ATTRIBUTES_AUTOMATIC_INFO) $(SECURITY_ATTRIBUTES_MANUAL_INFO) $(SECURITY_ATTRIBUTES_COMPATIBILITY_INFO)

EXTRA_DIST = $(SECATTRS) $(securitymodel_sources) find-sc.cs detect-sc.cs detect-ssc.cs merge.cs audit.cs ivt-check.cs

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

all: merge.exe $(MERGED_SECURITY_ATTRIBUTES_INFO) audit.exe ivt-check.exe

Moonlight.SecurityModel.dll: $(securitymodel_sources)
	gmcs $(CECIL) -debug -r:System.Core.dll -t:library -out:$@ $(securitymodel_sources)

find-sc.exe: find-sc.cs Moonlight.SecurityModel.dll
	gmcs find-sc.cs -debug -out:$@ $(LIBS)

compat: find-sc.exe
	mkdir -p compatibility
	mono find-sc.exe $(SILVERLIGHT_PLATFORM_CODE) compatibility

detect-sc.exe: detect-sc.cs Moonlight.SecurityModel.dll
	gmcs detect-sc.cs -debug -out:$@ $(LIBS)

detect-ssc.exe: detect-ssc.cs Moonlight.SecurityModel.dll
	gmcs detect-ssc.cs -debug -out:$@ $(LIBS)

detect: detect-sc.exe detect-ssc.exe
	mkdir -p automatic
	mono detect-sc.exe $(MOONLIGHT_PLATFORM_CODE) automatic
	mono detect-ssc.exe $(MOONLIGHT_PLATFORM_CODE) automatic

merge.exe: $(srcdir)/merge.cs Moonlight.SecurityModel.dll
	gmcs $(srcdir)/merge.cs -debug -out:$@ $(LIBS)

%.secattr: automatic/%.auto.sc automatic/%.auto.ssc overrides/%.manual compatibility/%.compat.sc merge.exe
	mono merge.exe "$(srcdir)" . $@

validate: find-sc.exe
	mkdir -p moonlight
	mono find-sc.exe $(MOONLIGHT_PLATFORM_CODE) moonlight
	diff -u compatibility moonlight

audit.exe: $(srcdir)/audit.cs Moonlight.SecurityModel.dll
	gmcs $(srcdir)/audit.cs -debug -out:$@ $(LIBS)

ivt-check.exe: $(srcdir)/ivt-check.cs Moonlight.SecurityModel.dll
	gmcs $(srcdir)/ivt-check.cs -debug -out:$@ $(LIBS)

# everything that we install in the plugin directory is (somewhat) trusted - even non-platform assemblies
TRUSTED_ASSEMBLIES= \
	../../lib/2.1/mscorlib.dll \
	../../lib/2.1/System.dll \
	../../lib/2.1/System.Core.dll \
	../../lib/2.1/System.Net.dll \
	../../lib/2.1/System.Runtime.Serialization.dll \
	../../lib/2.1/System.ServiceModel.dll \
	../../lib/2.1/System.ServiceModel.Web.dll \
	../../lib/2.1/System.Windows.dll \
	../../lib/2.1/System.Windows.Browser.dll \
	../../lib/2.1/System.Xml.dll
	

if HAVE_MONO_BASIC
TRUSTED_ASSEMBLIES += ../../lib/2.1/Microsoft.VisualBasic.dll
endif

ivt-check: ivt-check.exe
	MONO_PATH="../../lib/tuner:$$MONO_PATH" mono ivt-check.exe $(TRUSTED_ASSEMBLIES)

install: ivt-check

clean-data:
	rm -f *.secattr

clean: clean-data
	rm -f find-sc.exe detect-sc.exe detect-ssc.exe merge.exe audit.exe ivt-check.exe Moonlight.SecurityModel.dll *.mdb
