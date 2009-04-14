bin_SCRIPTS = example-glassycalendar
deskletdir = $(libdir)/desklets/example-glassycalendar

desklet_DATA = default.xaml calendar.dll

EXTRA_DIST = default.xaml calendar.cs

example-glassycalendar: Makefile
	echo "#!/bin/sh" > example-glassycalendar
	echo "mopen -d $(deskletdir)" >> example-glassycalendar
	chmod +x example-glassycalendar

calendar.dll: calendar.cs $(top_builddir)/gtk/Moonlight.Gtk.dll $(top_builddir)/class/lib/moonlight/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/Moonlight.Gtk.dll -r:$(top_builddir)/class/lib/moonlight/System.Windows.dll $(srcdir)/calendar.cs

CLEANFILES =			\
	example-glassycalendar	\
	calendar.dll		\
	calendar.dll.mdb


