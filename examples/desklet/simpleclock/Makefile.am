bin_SCRIPTS = example-simpleclock
deskletdir = $(libdir)/desklets/example-simpleclock

desklet_DATA = default.xaml clock.dll

EXTRA_DIST = default.xaml clock.cs

example-simpleclock: Makefile
	echo "#!/bin/sh" > example-simpleclock
	echo "mopen -d $(deskletdir)" >> example-simpleclock
	chmod +x example-simpleclock

clock.dll: clock.cs
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll clock.cs


