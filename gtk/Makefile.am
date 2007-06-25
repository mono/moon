gtksilverdir = $(prefix)/lib/mono/gtksilver
pkgconfigdir = $(prefix)/lib/pkgconfig

bin_SCRIPTS = mopen
gtksilver_DATA = gtksilver.dll mopen.exe mopen.exe.config
pkgconfig_DATA = gtksilver.pc

EXTRA_DIST= $(gtksilver_dll_CS) $(mopen_exe_CS) gtksilver.snk mopen.in
DISTCLEANFILES=mopen gtksilver.dll gtksilver.dll.mdb mopen.exe mopen.exe.mdb

gtksilver_dll_CS = $(srcdir)/gtk.cs \
                   $(srcdir)/ManagedDownloader.cs \
                   $(srcdir)/Assembly.cs

mopen_exe_CS = $(srcdir)/mopen.cs		\
               $(srcdir)/CompositeHelper.cs

gtksilver_SNK = $(srcdir)/gtksilver.snk

gtksilver.dll: $(gtksilver_dll_CS)
	gmcs -debug -out:gtksilver.dll -target:library $(gtksilver_dll_CS) -keyfile:$(gtksilver_SNK) -pkg:gtk-sharp-2.0 -pkg:silver

mopen.exe: $(mopen_exe_CS) gtksilver.dll 
	gmcs -debug -out:mopen.exe -target:exe $(mopen_exe_CS) -r:gtksilver.dll -pkg:gtk-sharp-2.0 -pkg:silver -pkg:mono-cairo

do_subst = sed -e 's,[@]gtksilverdir[@],$(gtksilverdir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

mopen: $(srcdir)/mopen.in Makefile
	$(do_subst) < $(srcdir)/mopen.in > mopen
	chmod +x mopen

install-data-local:
	gacutil -i gtksilver.dll

