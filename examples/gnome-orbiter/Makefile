all: gnome-orbiter.exe

gnome-orbiter.exe: Main.cs Entry.cs Window.cs Background.cs Throbber.cs Entry.cs EntryData.cs PlanetFetcher.cs Button.cs LayoutFu.cs 
	gmcs -g -out:gnome-orbiter.exe Main.cs EntryData.cs Entry.cs Window.cs Background.cs PlanetFetcher.cs Button.cs LayoutFu.cs Throbber.cs -pkg:gtk-sharp-2.0 -pkg:silverdesktop -pkg:Moonlight.Gtk -pkg:mono-cairo

run: gnome-orbiter.exe
	mono --debug gnome-orbiter.exe

clean:
	rm -f *.exe
	rm -f *.mdb
