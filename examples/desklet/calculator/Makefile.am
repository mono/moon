bin_SCRIPTS = example-calculator
deskletdir = $(libdir)/desklets/example-calculator

desklet_DATA = default.xaml calculator.dll

EXTRA_DIST = default.xaml calculator.cs

example-calculator: Makefile
	echo "#!/bin/sh" > example-calculator
	echo "mopen -d $(deskletdir)" >> example-calculator
	chmod +x example-calculator

calculator.dll: calculator.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll calculator.cs


