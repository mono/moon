bin_SCRIPTS = example-simpleclock
deskletdir = $(libdir)/desklets/example-simpleclock

desklet_DATA = default.xaml clock.dll

EXTRA_DIST = default.xaml clock.cs

example-simpleclock: Makefile
	echo "#!/bin/sh" > example-simpleclock
	echo "mopen -d $(deskletdir)" >> example-simpleclock
	chmod +x example-simpleclock

clock.dll: clock.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/clock.cs


