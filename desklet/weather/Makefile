DESKLET = weather
SOURCES = Locations.cs LocationsGenerated.cs Metar.cs default.xaml.cs

# you dont need to change below lines

ASSEMBLY = bin/$(DESKLET).dll
DSOURCES = $(SOURCES)
LAUNCHER = mopen -d
LOPTIONS = --debug

STATION_COMPILER = tools/StationCompiler.exe
STATION_COMPILER_SOURCES = tools/StationCompiler.cs

all: $(ASSEMBLY)

$(ASSEMBLY): $(DSOURCES)
	mkdir -p bin
	gmcs -debug -target:library -pkg:silver -out:$@ $^

clean:
	rm -f $(ASSEMBLY)

run: $(ASSEMBLY)
	MONO_OPTIONS=$(LOPTIONS) $(LAUNCHER) default.xaml

locations: $(STATION_COMPILER)
	bunzip2 -c data/Locations.xml.bz2 > tools/Locations.xml
	mono --debug $(STATION_COMPILER) tools/Locations.xml LocationsGenerated.cs
	rm -f tools/Locations.xml

$(STATION_COMPILER): $(STATION_COMPILER_SOURCES)
	gmcs -debug:full -out:$@ $^

dist: $(ASSEMBLY)
	# put here code to pack desklet (zip? tgz?)
