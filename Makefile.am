MONO_SUBDIRS=

if INCLUDE_MANAGED_CODE
MONO_SUBDIRS += class
endif

if !PLUGIN_INSTALL
if INCLUDE_DESKTOP_MANAGED_CODE
MONO_SUBDIRS += gtk man data
MONO_EXAMPLES = examples
endif

if INCLUDE_TESTING
TEST_SUBDIR = test
endif INCLUDE_TESTING

if INCLUDE_PERFORMANCE
PERF_SUBDIR = perf
endif INCLUDE_PERFORMANCE

TOOLS_SUBDIR = tools
endif !PLUGIN_INSTALL

SUBDIRS = $(subdirs) src $(MONO_SUBDIRS) $(TOOLS_SUBDIR) plugin $(TEST_SUBDIR) $(PERF_SUBDIR) $(MONO_EXAMPLES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=moon.pc

EXTRA_DIST=moon.pc.in LICENSE NOTES

# this should be handled by dolt.m4
DISTCLEANFILES=	doltcompile \
		doltlibtool

# custom install rule
test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C plugin test-plugin

run-tests:
	$(MAKE) $(AM_MAKEFLAGS) -C test run-tests

run-ms-tests:
	$(MAKE) $(AM_MAKEFLAGS) -C test run-ms-tests

run-perf: 
	$(MAKE) $(AM_MAKEFLAGS) -C perf run-perf

