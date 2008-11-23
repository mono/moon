desklet = ruler
entrypoint=Desklets.Ruler.App

noinst_DATA = default.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: default.xaml $(desklet).cs
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint)

CLEANFILES = 			\
	App.xaml.g.cs		\
	default.xaml.g.cs	\
	AppManifest.xaml 	\
	$(desklet).xap		\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).html		\
	$(desklet).g.resources
