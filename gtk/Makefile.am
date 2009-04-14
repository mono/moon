moonlight_gtkdir = $(prefix)/lib/mono/moon
pkgconfigdir     = $(prefix)/lib/pkgconfig
monodocdir       = $(prefix)/lib/monodoc/sources

moonlight_gtk_DATA = Moonlight.Gtk.dll 
pkgconfig_DATA = moonlight-gtk.pc 
monodoc_DATA = \
	moonlight-gtk.zip \
	moonlight-gtk.tree \
	moonlight-gtk.source

moonlight_gtk_dll_CS = \
	Moonlight.Gtk/MoonlightRuntime.cs	\
	Moonlight.Gtk/MoonlightHost.cs 		\
	Moonlight.Gtk/Desklets.cs 		\
	Moonlight.Gtk/ManagedDownloader.cs 	\
	Moonlight.Gtk/AssemblyInfo.cs

EXTRA_DIST = \
	$(moonlight_gtk_dll_CS) \
	moonlight-gtk.pc.in \
	moonlight-gtk.snk \
	moonlight-gtk.source

CLEANFILES = \
	Moonlight.Gtk.dll \
	Moonlight.Gtk.dll.mdb \
	moonlight-gtk.zip \
	moonlight-gtk.xml \
	moonlight-gtk.tree

clean-local:
	-rm -rf $(builddir)/docs

moonlight_gtk_SNK = $(srcdir)/moonlight-gtk.snk

Moonlight.Gtk.dll: $(moonlight_gtk_dll_CS) $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:$@ -target:library -unsafe -doc:moonlight-gtk.xml \
		-keyfile:$(moonlight_gtk_SNK) -pkg:gtk-sharp-2.0 \
		-r:$(top_builddir)/class/lib/3.0/System.Windows.dll \
		$(moonlight_gtk_dll_CS)

MONODOCER = MONO_PATH="../class/lib/3.0:$$MONO_PATH" monodocer
moonlight-gtk.xml: Moonlight.Gtk.dll
moonlight-gtk.tree: moonlight-gtk.zip
moonlight-gtk.zip: Moonlight.Gtk.dll
	rm -f $(top_builddir)/gtk/docs/ns-Moonlight.Gtk.xml
	$(MONODOCER) -importslashdoc:moonlight-gtk.xml -path:$(top_builddir)/gtk/docs -assembly:Moonlight.Gtk.dll
	mdassembler --ecma $(top_builddir)/gtk/docs/ --out moonlight-gtk

install-data-local:
	gacutil -gacdir $(DESTDIR)$(prefix) -i Moonlight.Gtk.dll

uninstall-local:
	gacutil -gacdir $(DESTDIR)$(prefix) -u Moonlight.Gtk

#NAMESPACES=Moonlight.Gtk
#dist-hook:
#	mkdir -p $(distdir)/docs
#	cp -f $(srcdir)/docs/*.xml $(distdir)/docs/.
##	for i in $(NAMESPACES); do                                      \
#		mkdir -p $(distdir)/docs/$$i;                           \
#		cp $(srcdir)/docs/$$i/*.xml $(distdir)/docs/$$i;        \
#	done

 
push:
	scp -v moonlight-gtk.tree moonlight-gtk.zip moonlight-gtk.source root@www.go-mono.com:/usr/lib/monodoc/sources/
