desklet = glassymonitor
entrypoint=Desklets.GlassyMonitor.App

DESKTOP_ASSEMBLIES=-r:$(top_builddir)/gtk/gtksilver.dll \
		   -r:$(top_builddir)/class/lib/3.0/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/3.0/System.Windows.Browser.dll \
		   -r:$(top_builddir)/class/lib/3.0/Mono.Moonlight.dll

noinst_DATA = default.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: $(desklet).cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint) --builddirhack=$(top_builddir) $(DESKTOP_ASSEMBLIES)

CLEANFILES =				\
	default.xaml.g.cs       	\
        App.xaml.g.cs           	\
        AppManifest.xaml        	\
	$(desklet).xap			\
	$(desklet).dll			\
	$(desklet).dll.mdb		\
	$(desklet).g.resources		\
	$(desklet).html
