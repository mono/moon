TESTS= \
	tests/ITest.cs \
	tests/LineAnimationTest.cs \
	tests/LinearBrushTest.cs \
	tests/RadialBrushTest.cs \
	tests/LineColorTest.cs \
	tests/ZIndexChangeTest.cs

demo.exe: demo.cs $(TESTS)
	gmcs -debug demo.cs  \
	$(TESTS) \
	-pkg:gtk-sharp-2.0 -pkg:silverdesktop -pkg:gtksilver

run: demo.exe
	MONO_PATH=.. mono --debug demo.exe

