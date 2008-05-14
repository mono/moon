if INCLUDE_MONO_RUNTIME
MONO_SUBDIRS = class
endif

if !PLUGIN_INSTALL
if INCLUDE_MONO_RUNTIME
MONO_SUBDIRS += gtk examples man data
endif INCLUDE_MONO_RUNTIME

if INCLUDE_TESTING
TEST_SUBDIR = test
endif INCLUDE_TESTING

TOOLS_SUBDIR = tools
endif !PLUGIN_INSTALL

SUBDIRS = $(subdirs) src $(MONO_SUBDIRS) $(TOOLS_SUBDIR) plugin $(TEST_SUBDIR)

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
