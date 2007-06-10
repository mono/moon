demo.exe: demo.cs gtk.cs
	gmcs -debug demo.cs gtk.cs -pkg:gtk-sharp-2.0 -pkg:silver

run: demo.exe
	mono --debug demo.exe

test: demo.exe
	mono --debug demo.exe xaml/*xaml
