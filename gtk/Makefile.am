gtksilverdir = $(prefix)/lib/mono/moon
pkgconfigdir = $(prefix)/lib/pkgconfig
monodocdir   = $(prefix)/lib/monodoc/sources

gtksilver_DATA = gtksilver.dll 
pkgconfig_DATA = gtksilver.pc

EXTRA_DIST= $(gtksilver_dll_CS) $(mopen_exe_CS) gtksilver.snk gtksilver.source

CLEANFILES=gtksilver.dll gtksilver.dll.mdb gtksilver.tree gtksilver.zip
DISTCLEANFILES=$(CLEANFILES)

gtksilver_dll_CS = $(srcdir)/gtk.cs \
                   $(srcdir)/ManagedDownloader.cs \
                   $(srcdir)/Assembly.cs

gtksilver_SNK = $(srcdir)/gtksilver.snk

monodoc_DATA = gtksilver.zip gtksilver.tree gtksilver.source

gtksilver.dll gtksilver.xml: $(gtksilver_dll_CS)
	gmcs -debug -doc:gtksilver.xml -out:gtksilver.dll -target:library $(gtksilver_dll_CS) -keyfile:$(gtksilver_SNK) -pkg:gtk-sharp-2.0 -pkg:silverdesktop

gtksilver.tree gtksilver.zip: gtksilver.xml gtksilver.dll docs/Gtk.Moonlight.xml docs/index.xml
	monodocer -importslashdoc:gtksilver.xml -path:docs -assembly:gtksilver.dll
	mdassembler --ecma docs/ --out gtksilver

install-data-local:
	gacutil -gacdir $(DESTDIR)$(prefix) -i gtksilver.dll

dist-hook:
	mkdir $(distdir)/docs
	-cp $(srcdir)/docs/Gtk.Moonlight.xml $(srcdir)/docs/index.xml $(distdir)/docs

push:
	scp -v gtksilver.tree gtksilver.zip gtksilver.source root@www.go-mono.com:/usr/lib/monodoc/sources/
