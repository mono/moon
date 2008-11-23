desklet=glassyclock.xap

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = clock.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

glassyclock.xap: clock.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.Clock.App

CLEANFILES =			\
	$(desklet)		\
	glassyclock.dll		\
	glassyclock.dll.mdb	\
	glassyclock.g.resources \
	default.xaml.g.cs       \
        App.xaml.g.cs           \
        AppManifest.xaml        \
	glassyclock.xap		\
	glassyclock.html

