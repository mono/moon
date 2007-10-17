if INCLUDE_MONO_RUNTIME
MONO_SUBDIRS = gtk tools
endif

DIST_SUBDIRS = $(subdirs) src gtk tools plugin man test/harness
SUBDIRS = $(subdirs) src $(MONO_SUBDIRS) plugin man test/harness

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=moon.pc

EXTRA_DIST=moon.pc.in

# custom install rule
test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C plugin test-plugin
