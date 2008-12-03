desklet=cpumonitor
entrypoint=Desklets.CpuMonitorPanel.App

DESKTOP_ASSEMBLIES=-r:$(top_builddir)/gtk/gtksilver.dll \
		   -r:$(top_builddir)/class/lib/3.0/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/3.0/System.Windows.Browser.dll \
		   -r:$(top_builddir)/class/lib/3.0/Mono.Moonlight.dll

noinst_DATA = default.xaml App.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) MONO_PATH=$(top_builddir)/gtk:$(top_builddir)/class/lib/3.0:$(MONO_PATH) mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: $(desklet).cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint) --builddirhack=$(top_builddir) $(DESKTOP_ASSEMBLIES)

CLEANFILES =			\
        default.xaml.g.cs       \
        App.xaml.g.cs           \
        AppManifest.xaml        \
	$(desklet).xap		\
	$(desklet).g.resources	\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).html
