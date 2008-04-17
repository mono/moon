bin_SCRIPTS = example-ruler
deskletdir = $(libdir)/desklets/example-ruler

desklet_DATA = default.xaml ruler.dll

EXTRA_DIST = default.xaml ruler.cs

example-ruler: Makefile
	echo "#!/bin/sh" > example-ruler
	echo "mopen -d $(deskletdir)" >> example-ruler
	chmod +x example-ruler

ruler.dll: ruler.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/ruler.cs


