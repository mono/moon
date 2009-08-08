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

# this is gross, but we have a circular dependency between System.Windows.dll and System.Windows.Browser.dll.
#
all-local: make-moonlight-dir
#	@(test ! -e lib/2.1/System.Windows.dll && bootstrap=1; \
#	 if [ "x$${bootstrap}" = "x1" ]; then \
#		echo Bootstrapping circular build; \
#		$(MAKE) -C tuning && \
#		BOOTSTRAP_FLAGS_2_1="-d:BOOTSTRAP" BOOTSTRAP_FLAGS_DESKTOP="-d:BOOTSTRAP" $(MAKE) -C System.Windows && \
#		$(MAKE) -C System.Windows.Browser && \
#		rm -f lib/2.1_raw/System.Windows.dll && \
#		rm -f lib/moonlight/System.Windows.dll && \
#		$(MAKE) -C System.Windows; \
#	 fi)

if INCLUDE_DESKTOP_MANAGED_CODE
make-moonlight-dir:
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


.PHONY: make-moonlight-dir