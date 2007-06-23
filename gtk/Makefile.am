gtksilverdir = $(prefix)/lib/mono/gtksilver
pkgconfigdir = $(prefix)/lib/pkgconfig

bin_SCRIPTS = mopen

gtksilver_DATA = gtksilver.dll mopen.exe

gtksilver.dll: gtk.cs ManagedDownloader.cs Assembly.cs
	gmcs -debug -out:gtksilver.dll -target:library gtk.cs ManagedDownloader.cs Assembly.cs -pkg:gtk-sharp-2.0 -pkg:silver 

mopen.exe: mopen.cs gtksilver.dll 
	gmcs -debug -out:mopen.exe -target:exe mopen.cs -r:gtksilver.dll -pkg:gtk-sharp-2.0 -pkg:silver

install-data-local:
	gacutil -i gtksilver.dll

pkgconfig_DATA = gtksilver.pc

do_subst = sed -e 's,[@]gtksilverdir[@],$(gtksilverdir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

mopen: mopen.in Makefile
	$(do_subst) < $(srcdir)/mopen.in > mopen
	chmod +x mopen