DESKLET = monitor
SOURCES = monitor.cs

# you dont need to change below lines

ASSEMBLY = $(DESKLET).dll
DSOURCES = $(SOURCES)
LAUNCHER = mopen
LOPTIONS = --debug

all: $(ASSEMBLY)

$(ASSEMBLY): $(DSOURCES)
	gmcs -debug -target:library -pkg:silver -r:../desklets.dll $(DSOURCES)
	cp -a ../desklets.dll .

run: $(ASSEMBLY)
	MONO_OPTIONS=$(LOPTIONS) $(LAUNCHER) default.xaml
