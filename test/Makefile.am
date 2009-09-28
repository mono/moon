
SUBDIRS=harness parsertest sizes
DIST_SUBDIRS = $(SUBDIRS) cpp templates

if INCLUDE_BROWSER_MANAGED_CODE
SUBDIRS += 2.0
endif

if INCLUDE_TESTING
TESTING = true
else
TESTING = echo ERROR: Test harness is disabled by configuration; false
endif

check-testing:
	$(TESTING)

install-test-plugin: check-testing
	$(MAKE) $(AM_MAKEFLAGS) -C harness/shocker install-test-plugin

run-unit-tests:
	$(MAKE) -C 2.0/moon-unit test

run-unit-tests-harness:
	$(MAKE) -C 2.0/moon-unit
	MOONLIGHT_UNIT_HARNESS_LOG_DIRECTORY=$(srcdir)/test-run-data DRTLIST=2.0/moon-unit/site/drtlist.xml $(MAKE) run-xephyr

dist-hook:
	cp -a $(srcdir)/xaml $(distdir)/xaml
	find $(distdir)/xaml -name .svn | xargs rm -rf

noinst_PROGRAMS = uris

uris_SOURCES= uri-test.cpp

uris_LDADD = $(MOON_PROG_LIBS)

uris_CPPFLAGS = $(MOON_PROG_CFLAGS) -I$(top_srcdir)/src/ -I$(top_srcdir)/src/asf

