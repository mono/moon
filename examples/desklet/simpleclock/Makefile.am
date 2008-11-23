desklet=simpleclock.xap

noinst_DATA = default.xaml simpleclock.xap $(desklet)

EXTRA_DIST = clock.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d simpleclock.xap

simpleclock.xap:
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.Clock.App

CLEANFILES =			\
	$(desklet)		\
	simpleclock.dll		\
	simpleclock.dll.mdb	\
	simpleclock.g.resources \
	simpleclock.html	\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml
	
