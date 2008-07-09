
SUBDIRS=harness parsertest sizes

MOZ_PATH = `pwd`/$(top_builddir)/test/harness/shocker/.libs:`pwd`/$(top_builddir)/plugin/.libs
if HAVE_GECKO_1_9 
LD_PATH = $(top_builddir)/plugin/.libs:$(top_builddir)/src/.libs:`pkg-config --variable=sdkdir mozilla-gtkmozembed`/lib
else
LD_PATH = $(top_builddir)/plugin/.libs:$(top_builddir)/src/.libs:`pkg-config --variable=libdir xulrunner-gtkmozembed`
endif

MOON_PLUGIN_DIR = `pwd`/$(top_builddir)/plugin/.libs

if INCLUDE_TESTING
TESTING = true
else
TESTING = echo ERROR: Test harness is disabled by configuration; false
endif

export MS_DRTLIST
XEPHYR_DEFAULT=2

check-testing:
	$(TESTING)

install-test-plugin: check-testing
	$(MAKE) $(AM_MAKEFLAGS) -C harness/shocker install-test-plugin

run-server: all check-testing
	mono harness/test-runner/test-runner.exe -drtlist $(MS_DRTLIST) -runner-server

run-logging-server: all check-testing
	mono harness/test-runner/logging-server.exe

run-gdb:
	HARNESS_FLAGS="-gdb" $(MAKE) $(MAKEFLAGS) run-drtlist

run-valgrind:
	HARNESS_FLAGS="-valgrind" $(MAKE) $(MAKEFLAGS) run-drtlist

run-xephyr:
	(test -z "$$XEPHYR" && XEPHYR=$(XEPHYR_DEFAULT); export XEPHYR; echo Running Xephyr in display $$XEPHYR; \
	test -z "$$DRT_TARGET" && DRT_TARGET="run-drtlist"; export DRT_TARGET; echo Executing target $$DRT_TARGET; \
	Xephyr -screen 1200x960 -ac -noreset :$$XEPHYR & xephyr_pid=$$!; \
	sleep 1; \
	DISPLAY=:$$XEPHYR xsetroot -solid '#ffffff'; \
	DISPLAY=:$$XEPHYR $(MAKE) $(MAKEFLAGS) $$DRT_TARGET; \
	kill $$xephyr_pid)

run-drtlist: all check-testing
	GNOME_DISABLE_CRASH_DIALOG=1 MOON_PLUGIN_DIR=$(MOON_PLUGIN_DIR) MOZ_PLUGIN_PATH=$(MOZ_PATH) LD_LIBRARY_PATH=$(LD_PATH):$(LD_LIBRARY_PATH) mono --debug harness/test-runner/test-runner.exe -drtlist $(DRTLIST) -agviewer $(top_srcdir)/test/harness/agviewer/agviewer -v -v -v -html-report -run-known-failures $(HARNESS_FLAGS)

run-tests:
	DRTLIST=$(top_builddir)/test/xaml/drtlist.xml $(MAKE) run-drtlist

run-ms-tests:
	DRTLIST=$(MS_DRTLIST) $(MAKE) run-drtlist

gtksilver.dll: ../gtk/gtksilver.dll
	cp ../gtk/gtksilver.dll .

control.exe: control.cs 
	gmcs -debug control.cs -pkg:gtk-sharp-2.0 -pkg:silverdesktop -r:gtksilver.dll

download.exe: download.cs gtksilver.dll test-custom-element.dll
	gmcs -debug download.cs -pkg:gtk-sharp-2.0 -pkg:silverdesktop -r:gtksilver.dll

demo.exe: demo.cs gtksilver.dll 
	gmcs -debug demo.cs -pkg:gtk-sharp-2.0 -pkg:silverdesktop -r:gtksilver.dll

test-custom-element.dll: test-custom-element.cs
	gmcs -pkg:silverdesktop /target:library test-custom-element.cs

run: demo.exe
	mono --debug demo.exe

test: demo.exe
	mono --debug demo.exe xaml/*xaml

dist-hook:
	cp -a $(srcdir)/xaml $(distdir)/xaml
	find $(distdir)/xaml -name .svn | xargs rm -rf
