DESKLET = calendar
SOURCES = calendar.cs

# you dont need to change below lines

ASSEMBLY = $(DESKLET).dll
DSOURCES = $(SOURCES)
LAUNCHER = mopen
LOPTIONS = --debug

all: $(ASSEMBLY)

$(ASSEMBLY): $(DSOURCES)
	gmcs -debug -target:library -pkg:silver $(DSOURCES)

run: $(ASSEMBLY)
	MONO_OPTIONS=$(LOPTIONS) $(LAUNCHER) default.xaml
