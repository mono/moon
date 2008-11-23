desklet=glassyslides.xap

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = slideshow.cs

glassyslides.xap: slideshow.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.GlassySlides.App

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

CLEANFILES =			\
	glassyslides.dll	\
	glassyslides.dll.mdb	\
	glassyslides.xap	\
	glassyslides.html	\
	glassyslides.g.resources\
        default.xaml.g.cs       \
        App.xaml.g.cs           \
        AppManifest.xaml
