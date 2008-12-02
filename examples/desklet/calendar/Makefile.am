desklet = calendar.xap
entrypoint= Desklet.CalendarPanel.App

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = calendar.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

$(desklet).xap: $(desklet).cs default.xaml App.xaml
	PKG_CONFIG_PATH=$(top_builddir)/gtk:$(top_builddir)/data:$(PKG_CONFIG_PATH) mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint)

CLEANFILES =			\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml	\
	$(desklet).xap		\
	$(desklet).g.resources	\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).html
