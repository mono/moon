moondir = $(prefix)/lib/moon

bin_SCRIPTS = svg2xaml svg2xaml-gui

moon_DATA = svg2xaml.exe svg2xaml-gui.exe

svg2xaml_exe_CS = 		\
	$(srcdir)/svg2xaml.cs

EXTRA_DIST = \
	svg2xaml.in \
	svg2xaml-gui.in \
	$(svg2xaml_exe_CS) \
	$(svg2xaml_gui_exe_CS) \
	svg2xaml.xslt

CLEANFILES = svg2xaml svg2xaml.exe

svg2xaml.exe: $(svg2xaml_exe_CS) 
	gmcs -out:$@ -debug $(svg2xaml_exe_CS) -r:System -r:System.Xml -resource:$(srcdir)/svg2xaml.xslt

svg2xaml_gui_exe_CS = 		\
	$(srcdir)/Main.cs			\
	$(srcdir)/MainWindow.cs		\
	$(srcdir)/gtk-gui/MainWindow.cs	\
	$(srcdir)/gtk-gui/generated.cs

svg2xaml: svg2xaml.in
	$(do_subst) < $(srcdir)/svg2xaml.in > svg2xaml
	chmod +x svg2xaml

svg2xaml-gui: svg2xaml-gui.in
	$(do_subst) < $(srcdir)/svg2xaml-gui.in > svg2xaml-gui
	chmod +x svg2xaml-gui

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

svg2xaml-gui.exe: $(svg2xaml_gui_exe_CS) $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -out:svg2xaml-gui.exe $(svg2xaml_gui_exe_CS) -pkg:gtk-sharp-2.0 -pkg:@RSVG_SHARP@ -r:$(top_builddir)/class/lib/3.0/System.Windows.dll -r:$(top_builddir)/gtk/gtksilver.dll -r:Mono.Posix -r:System -r:System.Xml -resource:$(srcdir)/svg2xaml.xslt

run:
	mono --debug svg2xaml.exe
