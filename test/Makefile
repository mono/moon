demo.exe: demo.cs gtk.cs
	gmcs -g demo.cs gtk.cs -pkg:gtk-sharp-2.0 -pkg:silver

run: demo.exe
	mono --debug demo.exe