desklet=cpumonitor.xap

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = monitor.cs

run: cpumonitor.xap
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

cpumonitor.xap: monitor.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklets.CpuMonitorPanel.App

CLEANFILES =			\
	$(desklet)		\
        default.xaml.g.cs       \
        App.xaml.g.cs           \
        AppManifest.xaml        \
	cpumonitor.g.resources	\
	cpumonitor.dll		\
	cpumonitor.dll.mdb	\
	cpumonitor.html
