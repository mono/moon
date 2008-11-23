desklet=glassyslides
entrypoint= Desklets.GlassySlides.App

noinst_DATA = default.xaml App.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

$(desklet).xap: $(desklet).cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint)

CLEANFILES =			\
	$(desklet).xap		\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).html		\
	$(desklet).g.resources	\
        default.xaml.g.cs       \
        App.xaml.g.cs           \
        AppManifest.xaml
