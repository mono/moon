SUBDIRS = .
if INCLUDE_BROWSER_MANAGED_CODE
SUBDIRS += tuning
endif
SUBDIRS += System.Windows.Browser System.Windows System.Windows.Controls System.Windows.Controls.Data
if INCLUDE_DESKTOP_MANAGED_CODE
SUBDIRS += Moon.Windows.Desktop
endif

if INCLUDE_BROWSER_MANAGED_CODE
NET_1_1_ASSEMBLIES = Mono.Cecil.dll Mono.Cecil.Mdb.dll
NET_2_0_ASSEMBLIES = monolinker.exe Mono.Tuner.dll
TUNER_ASSEMBLIES =  $(NET_1_1_ASSEMBLIES) $(NET_2_0_ASSEMBLIES)
LOCAL_ASSEMBLIES = $(TUNER_ASSEMBLIES:%=lib/tuner/%)
MCS_ASSEMBLIES = $(NET_1_1_ASSEMBLIES:%=$(MCS_PATH)/class/lib/net_1_1/%) $(NET_2_0_ASSEMBLIES:%=$(MCS_PATH)/class/lib/net_2_0/%)
MCS_MDBS = $(MCS_ASSEMBLIES:%=%.mdb)

endif

EXTRA_DIST = mono.pub winfx3.pub msfinal.pub mono.snk silverlight.pub

# this is gross, but we have a circular dependency between System.Windows.dll and System.Windows.Browser.dll.
#
all-local: copy-mcs-assemblies
	@echo Bootstrapping circular build;
if INCLUDE_BROWSER_MANAGED_CODE
	$(MAKE) -C tuning
endif
	$(MAKE) -C System.Windows bootstrap

make-lib-directories:
	@mkdir -p lib/2.1
	@mkdir -p lib/2.1_raw
	@mkdir -p lib/2.1-bootstrap
	@mkdir -p lib/2.1_raw-bootstrap
	@mkdir -p lib/moonlight
	@mkdir -p lib/moonlight-bootstrap
	@mkdir -p lib/tuner

if INCLUDE_BROWSER_MANAGED_CODE
copy-mcs-assemblies: make-lib-directories $(MCS_ASSEMBLIES)
	@cp $(MCS_ASSEMBLIES) $(MCS_MDBS) lib/tuner
else
copy-mcs-assemblies: make-lib-directories
endif

clean-local:
	-rm -rf lib

DISTCLEANFILES=	Microsoft.SilverlightControls/Makefile \
		Microsoft.SilverlightControls/Controls/Makefile

dist-hook:
	-mkdir $(distdir)/Microsoft.SilverlightControls
	-cp -a $(srcdir)/Microsoft.SilverlightControls/* $(distdir)/Microsoft.SilverlightControls
	find $(distdir)/Microsoft.SilverlightControls -name .svn | xargs rm -rf


.PHONY: make-lib-directories copy-mcs-assemblies
