#
# This is the makefile fragment with default rules for building desklets in 
# Moonlight. If you need to edit this file, that's a bug, email Everaldo
# <ecanuto@novell.com> about it.
#

ASSEMBLY = bin/$(DESKLET).dll
DSOURCES = $(SOURCES)
LAUNCHER = mopen -d
LOPTIONS = --debug

all: $(ASSEMBLY) postcompile

$(ASSEMBLY): $(DSOURCES)
	mkdir -p bin
	gmcs -debug -target:library -pkg:silverdesktop -out:$(ASSEMBLY) $(foreach a,$(EXTRA_ASSEMBLIES),-r:$(a)) $(DSOURCES)

clean:
	rm -f $(ASSEMBLY)
	rm -f $(ASSEMBLY).mdb

run: $(ASSEMBLY)
	MONO_OPTIONS=$(LOPTIONS) $(LAUNCHER) default.xaml

dist: $(ASSEMBLY)
	# put here code to pack desklet (zip? tgz?)

postcompile:

.PHONY: postcompile

