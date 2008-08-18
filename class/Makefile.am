SUBDIRS = . Mono.Moonlight System.Windows System.Windows.Browser

if INCLUDE_BROWSER_MANAGED_CODE
noinst_SCRIPTS = lib/2.1/smcs.exe
endif

EXTRA_DIST = mono.pub winfx3.pub msfinal.pub mono.snk silverlight.pub

if INCLUDE_DESKTOP_MANAGED_CODE
all-local:
	-mkdir -p lib/3.0
endif

if INCLUDE_BROWSER_MANAGED_CODE
lib/2.1/smcs.exe: $(SMCS_ASM)
	-mkdir -p lib/2.1
	cp $(SMCS_ASM) lib/2.1
endif

clean-local:
	-rm -rf lib

dist-hook:
	-mkdir $(distdir)/Microsoft.SilverlightControls
	cp -a $(srcdir)/Microsoft.SilverlightControls/* $(distdir)/Microsoft.SilverlightControls
	find $(distdir)/Microsoft.SilverlightControls -name .svn | xargs rm -rf

