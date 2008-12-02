desklet=simpleclock
entrypoint= Desklets.Clock.App

noinst_DATA = default.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: default.xaml $(desklet).cs
	PKG_CONFIG_PATH=$(top_builddir)/gtk:$(top_builddir)/data:$(PKG_CONFIG_PATH) mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint)

CLEANFILES =			\
	$(desklet).xap		\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).g.resources	\
	$(desklet).html		\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml
