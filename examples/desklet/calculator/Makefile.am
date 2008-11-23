desklet = calculator.xap

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = calculator.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

calculator.xap: calculator.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklet.Calculator.App

CLEANFILES =			\
	$(desklet)		\
	example-calculator	\
	calculator.g.resources	\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml	\
	calculator.dll		\
	calculator.dll.mdb	\
	calculator.html
