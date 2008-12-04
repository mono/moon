generate: generator.exe ../../src/*.cpp ../../src/*.h
	$(MONO) --debug generator.exe

generator.exe: *.cs
	gmcs *.cs -r:System.dll,System.Xml.dll -debug+ -out:generator.exe

CLEANFILES= generator.exe generator.exe.mdb
