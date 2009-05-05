desktopdir = $(prefix)/lib/mono/$(PACKAGE)
silverdir = $(prefix)/lib/mono/2.1

pkgconfigdir = $(prefix)/lib/pkgconfig

pkgconfig_DATA = silverdesktop.pc

EXTRA_DIST=silverdesktop.pc.in

if INCLUDE_BROWSER_MANAGED_CODE
pkgconfig_DATA += moon-browser-redist-assemblies.pc silver.pc
EXTRA_DIST += moon-browser-redist-assemblies.pc silver.pc.in
endif


silver.pc: $(srcdir)/silver.pc.in Makefile
	sed -e "s,@silverdir@,$(silverdir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/silver.pc.in > $@.tmp
	mv $@.tmp $@

silverdesktop.pc: $(srcdir)/silverdesktop.pc.in Makefile
	sed -e "s,@desktopdir@,$(desktopdir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/silverdesktop.pc.in > $@.tmp
	mv $@.tmp $@

CLEANFILES =			\
	silver.pc		\
	silverdesktop.pc
