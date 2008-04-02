bin_SCRIPTS = example-cpumonitor
deskletdir = $(libdir)/desklets/example-cpumonitor

desklet_DATA = default.xaml monitor.dll

EXTRA_DIST = default.xaml monitor.cs

example-cpumonitor: Makefile
	echo "#!/bin/sh" > example-cpumonitor
	echo "mopen -d $(deskletdir)" >> example-cpumonitor
	chmod +x example-cpumonitor

monitor.dll: monitor.cs
	gmcs -debug -target:library -pkg:silverdesktop -pkg:gtksilver monitor.cs


