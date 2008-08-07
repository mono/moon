generate: generator ../../src/*.cpp ../../src/*.h
	mono --debug generator.exe

generator: *.cs
	gmcs typegen.cs common.cs -r:System.dll,System.Xml.dll -debug+ -out:generator.exe

