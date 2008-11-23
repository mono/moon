desklet = ruler.xap
noinst_DATA = default.xaml $(desklet)

EXTRA_DIST = ruler.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

ruler.xap: default.xaml ruler.cs
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.Ruler.App

CLEANFILES = 		\
	$(desklet)	\
	ruler.dll	\
	ruler.dll.mdb	\
	ruler.html	\
	ruler.g.resources \
	App.xaml.g.cs	\
	default.xaml.g.cs \
	AppManifest.xaml 
