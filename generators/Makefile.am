FILES = $(wildcard ../src/*.cpp) $(wildcard ../src/*.h) $(wildcard ../plugin/*.h)

stamp: generator.exe $(FILES)
	$(MONO) --debug generator.exe
	$(MONO) --debug generator.exe
	touch stamp

install all: stamp

SRC = $(wildcard *.cs)
generator.exe: $(SRC)
	gmcs $(SRC) -r:System.dll,System.Xml.dll -debug+ -out:generator.exe

CLEANFILES= generator.exe generator.exe.mdb
