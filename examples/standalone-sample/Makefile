all: standalone-sample.exe

clean:
	rm -f standalone-sample.exe
	rm -f MyCanvas.xaml.g.cs
	rm -f *.mdb

MyCanvas.xaml.g.cs:
	xamlg MyCanvas.xaml

standalone-sample.exe: Main.cs MyCanvas.cs MyCanvas.xaml.g.cs
	gmcs -debug -pkg:gtksilver -pkg:silverdesktop -pkg:gtk-sharp-2.0 -out:standalone-sample.exe Main.cs MyCanvas.cs MyCanvas.xaml.g.cs

run:
	mono standalone-sample.exe


