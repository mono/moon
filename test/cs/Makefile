demo.exe: demo.cs gtk.cs
	gmcs -debug demo.cs gtk.cs \
	tests/ITest.cs \
	tests/LineAnimationTest.cs \
	tests/LinearBrushTest.cs \
	tests/RadialBrushTest.cs \
	tests/LineColorTest.cs \
	-pkg:gtk-sharp-2.0 -pkg:silver

run: demo.exe
	mono --debug demo.exe

