SUBDIRS = .
if INCLUDE_BROWSER_MANAGED_CODE
SUBDIRS += tuning
endif
SUBDIRS += System.Windows System.Windows.Browser System.Windows.Controls System.Windows.Controls.Data

if INCLUDE_BROWSER_MANAGED_CODE
NET_1_1_ASSEMBLIES = Mono.Cecil.dll Mono.Cecil.Mdb.dll
NET_2_0_ASSEMBLIES = monolinker.exe Mono.Tuner.dll
TUNER_ASSEMBLIES =  $(NET_1_1_ASSEMBLIES) $(NET_2_0_ASSEMBLIES)
LOCAL_ASSEMBLIES = $(TUNER_ASSEMBLIES:%=lib/tuner/%)
MCS_ASSEMBLIES = $(NET_1_1_ASSEMBLIES:%=$(MCS_PATH)/class/lib/net_1_1/%) $(NET_2_0_ASSEMBLIES:%=$(MCS_PATH)/class/lib/net_2_0/%)

noinst_SCRIPTS = $(LOCAL_ASSEMBLIES)

endif

EXTRA_DIST = mono.pub winfx3.pub msfinal.pub mono.snk silverlight.pub

if INCLUDE_DESKTOP_MANAGED_CODE
all-local:
	-mkdir -p lib/moonlight
endif

if INCLUDE_BROWSER_MANAGED_CODE
$(LOCAL_ASSEMBLIES): $(MCS_ASSEMBLIES)
	-mkdir -p lib/2.1
	-mkdir -p lib/tuner
	for i in $(MCS_ASSEMBLIES); do cp $$i $$i.mdb lib/tuner/; done
endif

clean-local:
	-rm -rf lib

DISTCLEANFILES=	Microsoft.SilverlightControls/Makefile \
		Microsoft.SilverlightControls/Controls/Makefile

dist-hook:
	-mkdir $(distdir)/Microsoft.SilverlightControls
	-cp -a $(srcdir)/Microsoft.SilverlightControls/* $(distdir)/Microsoft.SilverlightControls
	find $(distdir)/Microsoft.SilverlightControls -name .svn | xargs rm -rf

