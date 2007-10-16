DIST_SUBDIRS = $(subdirs) src gtk plugin tools man test/harness
SUBDIRS = $(subdirs) src

if INCLUDE_MONO_RUNTIME
	SUBDIRS += gtk tools
endif

SUBDIRS += plugin man test/harness

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=moon.pc

EXTRA_DIST=moon.pc.in

# custom install rule
test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C plugin test-plugin
