bin_SCRIPTS = example-glassycalendar
deskletdir = $(libdir)/desklets/example-glassycalendar

desklet_DATA = default.xaml calendar.dll

EXTRA_DIST = default.xaml calendar.cs

example-glassycalendar: Makefile
	echo "#!/bin/sh" > example-glassycalendar
	echo "mopen -d $(deskletdir)" >> example-glassycalendar
	chmod +x example-glassycalendar

calendar.dll: calendar.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll calendar.cs


