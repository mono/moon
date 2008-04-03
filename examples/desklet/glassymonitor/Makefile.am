bin_SCRIPTS = example-glassymonitor
deskletdir = $(libdir)/desklets/example-glassymonitor

desklet_DATA = default.xaml monitor.dll

EXTRA_DIST = default.xaml monitor.cs

example-glassymonitor: Makefile
	echo "#!/bin/sh" > example-glassymonitor
	echo "mopen -d $(deskletdir)" >> example-glassymonitor
	chmod +x example-glassymonitor

monitor.dll: monitor.cs
	gmcs -debug -target:library -pkg:silverdesktop -pkg:gtksilver monitor.cs


