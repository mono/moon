all: svg2xaml.exe svg2xaml-gui.exe

gtksilver.dll: ../../gtk/gtksilver.dll
	cp ../../gtk/gtksilver.dll .

svg2xaml.exe: svg2xaml.cs
	gmcs -debug svg2xaml.cs -r:System -r:System.Xml

svg2xaml-gui.exe: Main.cs MainWindow.cs gtk-gui/MainWindow.cs gtk-gui/generated.cs gtksilver.dll 
	gmcs -debug -out:svg2xaml-gui.exe Main.cs MainWindow.cs gtk-gui/MainWindow.cs gtk-gui/generated.cs -pkg:gtk-sharp-2.0 -pkg:rsvg-sharp-2.0 -pkg:silver -r:gtksilver.dll -r:Mono.Posix -r:System -r:System.Xml

run: svg2xaml.exe
	mono --debug svg2xaml.exe
	
clean:
	-rm *.exe
	-rm *.mdb

