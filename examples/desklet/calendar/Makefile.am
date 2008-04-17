bin_SCRIPTS = example-calendar
deskletdir = $(libdir)/desklets/example-calendar

desklet_DATA = default.xaml calendar.dll

EXTRA_DIST = default.xaml calendar.cs

example-calendar: Makefile
	echo "#!/bin/sh" > example-calendar
	echo "mopen -d $(deskletdir)" >> example-calendar
	chmod +x example-calendar

calendar.dll: calendar.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/calendar.cs


