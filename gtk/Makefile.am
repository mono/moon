gtksilverdir = $(prefix)/lib/mono/gtksilver
pkgconfigdir = $(prefix)/lib/pkgconfig

gtksilver_DATA = gtksilver.dll

gtksilver.dll: gtk.cs ManagedDownloader.cs Assembly.cs
	gmcs -debug -out:gtksilver.dll -target:library gtk.cs ManagedDownloader.cs Assembly.cs -pkg:gtk-sharp-2.0 -pkg:silver 

install-data-local:
	gacutil -i gtksilver.dll

pkgconfig_DATA = gtksilver.pc