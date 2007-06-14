demo.exe: demo.cs gtksilver.dll
	gmcs -debug demo.cs -pkg:gtk-sharp-2.0 -pkg:silver -r:gtksilver.dll

gtksilver.dll: gtk.cs ManagedDownloader.cs
	gmcs -debug -target:library -out:gtksilver.dll gtk.cs ManagedDownloader.cs -pkg:gtk-sharp-2.0 -pkg:silver

run: demo.exe
	mono --debug demo.exe

test: demo.exe
	mono --debug demo.exe xaml/*xaml
