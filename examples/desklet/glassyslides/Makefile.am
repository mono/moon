bin_SCRIPTS = example-glassyslides
deskletdir = $(libdir)/desklets/example-glassyslides

desklet_DATA = default.xaml slideshow.dll

EXTRA_DIST = default.xaml slideshow.cs

example-glassyslides: Makefile
	echo "#!/bin/sh" > example-glassyslides
	echo "mopen -d $(deskletdir)" >> example-glassyslides
	chmod +x example-glassyslides

slideshow.dll: slideshow.cs
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll slideshow.cs


