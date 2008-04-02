bin_SCRIPTS = example-ruler
deskletdir = $(libdir)/desklets/example-ruler

desklet_DATA = default.xaml ruler.dll

EXTRA_DIST = default.xaml ruler.cs

example-ruler: Makefile
	echo "#!/bin/sh" > example-ruler
	echo "mopen -d $(deskletdir)" >> example-ruler
	chmod +x example-ruler

ruler.dll: ruler.cs
	gmcs -debug -target:library -pkg:silverdesktop -pkg:gtksilver ruler.cs


