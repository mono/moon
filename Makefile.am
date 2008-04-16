if INCLUDE_MONO_RUNTIME
MONO_SUBDIRS = gtk tools man examples class
endif

if INCLUDE_TESTING
TEST_SUBDIR=test
endif

DIST_SUBDIRS = $(subdirs) src gtk tools plugin man test examples class
SUBDIRS = $(subdirs) src $(MONO_SUBDIRS) plugin $(TEST_SUBDIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=moon.pc

EXTRA_DIST=moon.pc.in

# custom install rule
test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C plugin test-plugin


run-tests:
	$(MAKE) $(AM_MAKEFLAGS) -C test run-tests

run-ms-tests:
	$(MAKE) $(AM_MAKEFLAGS) -C test run-ms-tests
