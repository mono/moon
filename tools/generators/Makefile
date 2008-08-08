generate: generator ../../src/*.cpp ../../src/*.h
	mono --debug generator.exe

generator: *.cs
	gmcs *.cs -r:System.dll,System.Xml.dll -debug+ -out:generator.exe
