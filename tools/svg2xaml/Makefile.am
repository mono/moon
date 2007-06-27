moondir = $(prefix)/lib/moon

bin_SCRIPTS = svg2xaml svg2xaml-gui

moon_DATA = svg2xaml.exe svg2xaml-gui.exe

EXTRA_DIST = svg2xaml.in svg2xaml-gui.in

CLEANFILES = svg2xaml svg2xaml.exe

svg2xaml_exe_CS = svg2xaml.cs

svg2xaml.exe: $(svg2xaml_exe_CS) 
	gmcs -debug svg2xaml.cs -r:System -r:System.Xml

svg2xaml_gui_exe_CS = 		\
	Main.cs			\
	MainWindow.cs		\
	gtk-gui/MainWindow.cs	\
	gtk-gui/generated.cs

svg2xaml: svg2xaml.in
	$(do_subst) < $(srcdir)/svg2xaml.in > svg2xaml
	chmod +x svg2xaml

svg2xaml-gui: svg2xaml-gui.in
	$(do_subst) < $(srcdir)/svg2xaml-gui.in > svg2xaml-gui
	chmod +x svg2xaml-gui

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

svg2xaml-gui.exe: $(svg2xaml_gui_exe_CS) ../../gtk/gtksilver.dll
	gmcs -debug -out:svg2xaml-gui.exe $(svg2xaml_gui_exe_CS) -pkg:gtk-sharp-2.0 -pkg:rsvg-sharp-2.0 -pkg:silver -r:../../gtk/gtksilver.dll -r:Mono.Posix -r:System -r:System.Xml

run:
	mono --debug svg2xaml.exe