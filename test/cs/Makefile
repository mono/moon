demo.exe: demo.cs 
	gmcs -debug demo.cs  \
	tests/ITest.cs \
	tests/LineAnimationTest.cs \
	tests/LinearBrushTest.cs \
	tests/RadialBrushTest.cs \
	tests/LineColorTest.cs \
	-pkg:gtk-sharp-2.0 -pkg:silver -r:../gtksilver.dll

run: demo.exe
	MONO_PATH=.. mono --debug demo.exe

