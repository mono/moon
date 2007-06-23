gtksilverdir = $(prefix)/lib/mono/gtksilver
pkgconfigdir = $(prefix)/lib/pkgconfig

bin_SCRIPTS = mopen
gtksilver_DATA = gtksilver.dll mopen.exe
pkgconfig_DATA = gtksilver.pc

EXTRA_DIST= $(gtksilver_CS) gtksilver.snk

gtksilver_dll_CS = $(srcdir)/gtk.cs \
                   $(srcdir)/ManagedDownloader.cs \
                   $(srcdir)/Assembly.cs

mopen_exe_CS = $(srcdir)/mopen.cs

gtksilver_SNK = $(srcdir)/gtksilver.snk

gtksilver.dll: $(gtksilver_dll_CS)
	gmcs -debug -out:gtksilver.dll -target:library $(gtksilver_CS) -keyfile:$(gtksilver_SNK) -pkg:gtk-sharp-2.0 -pkg:silver 

mopen.exe: $(mopen_exe_CS) gtksilver.dll 
	gmcs -debug -out:mopen.exe -target:exe mopen.cs -r:gtksilver.dll -pkg:gtk-sharp-2.0 -pkg:silver

do_subst = sed -e 's,[@]gtksilverdir[@],$(gtksilverdir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

mopen: $(srcdir)/mopen.in Makefile
	$(do_subst) < $(srcdir)/mopen.in > mopen
	chmod +x mopen

install-data-local:
	gacutil -i gtksilver.dll

