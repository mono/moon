.NOTPARALLEL:

SUBDIRS = . System.Windows.Browser System.Windows 
if INCLUDE_DESKTOP_MANAGED_CODE
SUBDIRS += Moon.Windows.Desktop
endif
if INCLUDE_BROWSER_MANAGED_CODE
SUBDIRS += tuning
endif
SUBDIRS += System.Windows.Controls System.Windows.Controls.Data

EXTRA_DIST = mono.pub winfx3.pub mono.snk silverlight.pub

# this is gross, but we have a circular dependency between System.Windows.dll and System.Windows.Browser.dll.
#
all-local: make-lib-directories
	@echo Bootstrapping circular build;
	cp $(MONO_PATH)/mcs/class/lib/moonlight/* lib/2.1_raw/
	$(MAKE) -C System.Windows bootstrap

make-lib-directories:
	@mkdir -p lib/2.1
	@mkdir -p lib/2.1_raw
	@mkdir -p lib/2.1-bootstrap
	@mkdir -p lib/2.1_raw-bootstrap
	@mkdir -p lib/moonlight
	@mkdir -p lib/moonlight-bootstrap
	@mkdir -p lib/tuner

clean-local:
	-rm -rf lib

DISTCLEANFILES=	Microsoft.SilverlightControls/Makefile \
		Microsoft.SilverlightControls/Controls/Makefile

dist-hook:
	-mkdir $(distdir)/Microsoft.SilverlightControls
	-cp -a $(srcdir)/Microsoft.SilverlightControls/* $(distdir)/Microsoft.SilverlightControls
	find $(distdir)/Microsoft.SilverlightControls -name .svn | xargs rm -rf


.PHONY: make-lib-directories
