all: clock.dll

clock.dll: clock.cs
	gmcs -debug -target:library clock.cs -pkg:silver

run: clock.dll
	MONO_OPTIONS=--debug mopen default.xaml