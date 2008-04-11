
SUBDIRS=harness parsertest sizes

DIST_SUBDIRS=harness parsertest sizes

MOZ_PATH = `pwd`/$(top_builddir)/test/harness/shocker/.libs:`pwd`/$(top_builddir)/plugin/.libs
LD_PATH = $(top_builddir)/plugin/.libs:$(top_builddir)/src/.libs:`pkg-config --variable=libdir xulrunner-gtkmozembed`

install-test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C harness/shocker install-test-plugin

run-tests: all
	GNOME_DISABLE_CRASH_DIALOG=1 MOZ_PLUGIN_PATH=$(MOZ_PATH) LD_LIBRARY_PATH=$(LD_PATH):$(LD_LIBRARY_PATH) mono harness/test-runner/test-runner.exe -drtlist $(top_builddir)/test/xaml/drtlist.xml -agviewer $(top_srcdir)/test/harness/agviewer/agviewer -v -v -html-report

run-ms-tests: all
	GNOME_DISABLE_CRASH_DIALOG=1 MOZ_PLUGIN_PATH=$(MOZ_PATH) LD_LIBRARY_PATH=$(LD_PATH):$(LD_LIBRARY_PATH) mono harness/test-runner/test-runner.exe -drtlist $(MS_DRTLIST) -agviewer $(top_srcdir)/test/harness/agviewer/agviewer -v -v  -html-report -run-known-failures




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
