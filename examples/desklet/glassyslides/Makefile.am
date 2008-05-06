bin_SCRIPTS = example-glassyslides
deskletdir = $(libdir)/desklets/example-glassyslides

desklet_DATA = default.xaml slideshow.dll

EXTRA_DIST = default.xaml slideshow.cs

example-glassyslides: Makefile
	echo "#!/bin/sh" > example-glassyslides
	echo "mopen -d $(deskletdir)" >> example-glassyslides
	chmod +x example-glassyslides

slideshow.dll: slideshow.cs $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll $(srcdir)/slideshow.cs

CLEANFILES =	\
	example-glassyslides	\
	slideshow.dll		\
	slideshow.dll.mdb
