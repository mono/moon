bin_SCRIPTS = example-glassyclock
deskletdir = $(libdir)/desklets/example-glassyclock

desklet_DATA = default.xaml clock.dll

EXTRA_DIST = default.xaml clock.cs

example-glassyclock: Makefile
	echo "#!/bin/sh" > example-glassyclock
	echo "mopen -d $(deskletdir)" >> example-glassyclock
	chmod +x example-glassyclock

clock.dll: clock.cs
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll clock.cs


