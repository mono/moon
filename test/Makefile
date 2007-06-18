all: demo.exe download.exe control.exe

gtksilver.dll: ../gtk/gtksilver.dll
	cp ../gtk/gtksilver.dll .

control.exe: control.cs 
	gmcs -debug control.cs -pkg:gtk-sharp-2.0 -pkg:silver -r:gtksilver.dll

download.exe: download.cs gtksilver.dll test-custom-element.dll
	gmcs -debug download.cs -pkg:gtk-sharp-2.0 -pkg:silver -r:gtksilver.dll

demo.exe: demo.cs gtksilver.dll 
	gmcs -debug demo.cs -pkg:gtk-sharp-2.0 -pkg:silver -r:gtksilver.dll

test-custom-element.dll: test-custom-element.cs
	gmcs -pkg:silver /target:library test-custom-element.cs

run: demo.exe
	mono --debug demo.exe

test: demo.exe
	mono --debug demo.exe xaml/*xaml
