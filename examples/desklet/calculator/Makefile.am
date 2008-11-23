desklet = calculator
entrypoint= Desklet.Calculator.App

noinst_DATA = default.xaml App.xaml $(desklet).xap

EXTRA_DIST = $(desklet).cs

run: $(desklet).xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet).xap

$(desklet).xap: $(desklet).cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=$(entrypoint)

CLEANFILES =			\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml	\
	$(desklet).xap		\
	$(desklet).g.resources	\
	$(desklet).dll		\
	$(desklet).dll.mdb	\
	$(desklet).html
