all: photo-browser.exe

photo-browser.exe: PhotoBrowser.boo
	booc PhotoBrowser.boo -o:photo-browser.exe -pkg:gtk-sharp-2.0 -pkg:Moonlight.Gtk

run: photo-browser.exe
	mono photo-browser.exe

clean:
	rm -f *.exe
	rm -f *.mdb
