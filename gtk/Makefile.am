gtksilverdir = $(prefix)/lib/mono/moon
pkgconfigdir = $(prefix)/lib/pkgconfig
monodocdir   = $(prefix)/lib/monodoc/sources

gtksilver_DATA = gtksilver.dll 
pkgconfig_DATA = gtksilver.pc 

EXTRA_DIST= $(gtksilver_dll_CS) gtksilver.snk gtksilver.source 

CLEANFILES=gtksilver.dll gtksilver.dll.mdb gtksilver.tree gtksilver.zip 
DISTCLEANFILES=$(CLEANFILES)

gtksilver_dll_CS = $(srcdir)/gtk.cs \
		   $(srcdir)/desklets.cs \
                   $(srcdir)/ManagedDownloader.cs \
                   $(srcdir)/Assembly.cs

gtksilver_SNK = $(srcdir)/gtksilver.snk

monodoc_DATA = gtksilver.zip gtksilver.tree gtksilver.source

gtksilver.dll gtksilver.xml: $(gtksilver_dll_CS) $(top_builddir)/class/lib/3.0/Mono.Moonlight.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -doc:gtksilver.xml -out:gtksilver.dll -target:library $(gtksilver_dll_CS) -keyfile:$(gtksilver_SNK) -pkg:gtk-sharp-2.0 -r:$(top_builddir)/class/lib/3.0/System.Windows.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.Browser.dll -r:$(top_builddir)/class/lib/3.0/Mono.Moonlight.dll

MONODOCER = MONO_PATH="../class/lib/3.0:$$MONO_PATH" monodocer
gtksilver.tree gtksilver.zip: gtksilver.xml gtksilver.dll $(srcdir)/docs/index.xml
	$(MONODOCER) -importslashdoc:gtksilver.xml -path:$(top_builddir)/gtk/docs -assembly:gtksilver.dll
	mdassembler --ecma $(top_builddir)/gtk/docs/ --out gtksilver

install-data-local:
	gacutil -gacdir $(DESTDIR)$(prefix) -i gtksilver.dll

uninstall-local:
	gacutil -gacdir $(DESTDIR)$(prefix) -u gtksilver

NAMESPACES=Gtk.Moonlight
dist-hook:
	mkdir -p $(distdir)/docs
	cp $(srcdir)/docs/*.xml $(distdir)/docs/.
#	for i in $(NAMESPACES); do                                      \
		mkdir -p $(distdir)/docs/$$i;                           \
		cp $(srcdir)/docs/$$i/*.xml $(distdir)/docs/$$i;        \
	done

 
push:
	scp -v gtksilver.tree gtksilver.zip gtksilver.source root@www.go-mono.com:/usr/lib/monodoc/sources/
