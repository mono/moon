bin_SCRIPTS = example-glassyclock
deskletdir = $(libdir)/desklets/example-glassyclock

desklet_DATA = default.xaml clock.dll

EXTRA_DIST = default.xaml clock.cs

example-glassyclock: Makefile
	echo "#!/bin/sh" > example-glassyclock
	echo "mopen -d $(deskletdir)" >> example-glassyclock
	chmod +x example-glassyclock

clock.dll: clock.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/clock.cs


