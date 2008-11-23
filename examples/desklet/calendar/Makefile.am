desklet = calendar.xap

noinst_DATA = default.xaml App.xaml $(desklet)

EXTRA_DIST = calendar.cs

run: $(desklet)
	mono ../../../tools/mopen/mopen.exe -d $(desklet)

$(desklet): calendar.cs default.xaml App.xaml
	mono ../../../tools/mxap/mxap.exe --desktop --entry-point-type=Desklet.CalendarPanel.App

CLEANFILES =			\
	$(desklet)		\
	calendar.g.resources	\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml	\
	calendar.dll		\
	calendar.dll.mdb	\
	calendar.html
