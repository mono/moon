DESKLET = clock
SOURCES = clock.cs

# you dont need to change below lines

ASSEMBLY = bin/$(DESKLET).dll
DSOURCES = $(SOURCES)
LAUNCHER = mopen -t
LOPTIONS = --debug

all: $(ASSEMBLY)

$(ASSEMBLY): $(DSOURCES)
	mkdir -p bin
	gmcs -debug -target:library -pkg:silver -out:$(ASSEMBLY) $(DSOURCES)

clean:
	rm -f $(ASSEMBLY)

run: $(ASSEMBLY)
	MONO_OPTIONS=$(LOPTIONS) $(LAUNCHER) default.xaml
