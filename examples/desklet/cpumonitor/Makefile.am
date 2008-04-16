bin_SCRIPTS = example-cpumonitor
deskletdir = $(libdir)/desklets/example-cpumonitor

desklet_DATA = default.xaml monitor.dll

EXTRA_DIST = default.xaml monitor.cs

example-cpumonitor: Makefile
	echo "#!/bin/sh" > example-cpumonitor
	echo "mopen -d $(deskletdir)" >> example-cpumonitor
	chmod +x example-cpumonitor

monitor.dll: monitor.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll monitor.cs


