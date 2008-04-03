bin_SCRIPTS = example-calculator
deskletdir = $(libdir)/desklets/example-calculator

desklet_DATA = default.xaml calculator.dll

EXTRA_DIST = default.xaml calculator.cs

example-calculator: Makefile
	echo "#!/bin/sh" > example-calculator
	echo "mopen -d $(deskletdir)" >> example-calculator
	chmod +x example-calculator

calculator.dll: calculator.cs
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll calculator.cs


