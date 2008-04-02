bin_SCRIPTS = example-glassycalendar
deskletdir = $(libdir)/desklets/example-glassycalendar

desklet_DATA = default.xaml calendar.dll

EXTRA_DIST = default.xaml calendar.cs

example-glassycalendar: Makefile
	echo "#!/bin/sh" > example-glassycalendar
	echo "mopen -d $(deskletdir)" >> example-glassycalendar
	chmod +x example-glassycalendar

calendar.dll: calendar.cs
	gmcs -debug -target:library -pkg:silverdesktop -pkg:gtksilver calendar.cs


