desklet=simpleclock
entrypoint= Desklets.Clock.App

DESKTOP_ASSEMBLIES=-r:$(top_builddir)/gtk/Moonlight.Gtk.dll \
		   -r:$(top_builddir)/class/lib/moonlight/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/moonlight/System.Windows.Browser.dll

noinst_DATA = default.xaml $(desklet).xap

EXTRA_DIST =		\
	$(desklet).cs	\
	default.xaml	\
	App.xaml

run: $(desklet).xap
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) MONO_PATH=$(top_builddir)/gtk:$(top_builddir)/class/lib/moonlight:$(MONO_PATH) mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: $(srcdir)/$(desklet).cs $(srcdir)/default.xaml $(srcdir)/App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint) --builddirhack=$(top_builddir) $(DESKTOP_ASSEMBLIES) $(srcdir)

CLEANFILES =			\
	$(desklet).xap		\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).g.resources	\
	$(desklet).html		\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml
