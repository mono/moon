bin_SCRIPTS = example-calendar
deskletdir = $(libdir)/desklets/example-calendar

desklet_DATA = default.xaml calendar.dll

EXTRA_DIST = default.xaml calendar.cs

example-calendar: Makefile
	echo "#!/bin/sh" > example-calendar
	echo "mopen -d $(deskletdir)" >> example-calendar
	chmod +x example-calendar

calendar.dll: calendar.cs
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll calendar.cs


