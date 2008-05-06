bin_SCRIPTS = example-glassymonitor
deskletdir = $(libdir)/desklets/example-glassymonitor

desklet_DATA = default.xaml monitor.dll

EXTRA_DIST = default.xaml monitor.cs

example-glassymonitor: Makefile
	echo "#!/bin/sh" > example-glassymonitor
	echo "mopen -d $(deskletdir)" >> example-glassymonitor
	chmod +x example-glassymonitor

monitor.dll: monitor.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/monitor.cs

CLEANFILES =			\
	example-glassymonitor	\
	monitor.dll		\
	monitor.dll.mdb
