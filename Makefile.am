if INCLUDE_MONO_RUNTIME
MONO_SUBDIRS = class
if !PLUGIN_INSTALL
MONO_SUBDIRS += gtk examples man data
endif !PLUGIN_INSTALL
endif INCLUDE_MONO_RUNTIME

if INCLUDE_TESTING
TEST_SUBDIR=test
endif

SUBDIRS = $(subdirs) src $(MONO_SUBDIRS) tools plugin $(TEST_SUBDIR)

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
