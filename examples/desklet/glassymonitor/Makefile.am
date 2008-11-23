desklet = glassymonitor.xap
noinst_DATA = default.xaml $(desklet)

EXTRA_DIST = monitor.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)
	
glassymonitor.xap: monitor.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.GlassyMonitor.App
	
CLEANFILES =				\
	$(desklet)			\
	glassymonitor.dll		\
	glassymonitor.dll.mdb		\
	glassymonitor.g.resources	\
	default.xaml.g.cs       	\
        App.xaml.g.cs           	\
        AppManifest.xaml        	\
	glassymonitor.html
	
