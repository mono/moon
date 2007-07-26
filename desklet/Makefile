SUBDIRS = calculator calendar clock cpumonitor glassycalendar glassyclock glassymonitor glassyslides picasa simpleclock

all: docs/Mono.Desklets.xml desklets.dll 
	for i in $(SUBDIRS); do (cd $$i && make); done

clean:
	for i in $(SUBDIRS); do (cd $$i && make clean); done
	-rm desklets.tree desklets.zip desklets.dll desklets.xml

desklets.dll desklets.xml: desklets.cs
	gmcs -debug -doc:desklets.xml -target:library desklets.cs -pkg:silver -pkg:gtk-sharp-2.0 -pkg:gconf-sharp-2.0

docs/Mono.Desklets.xml: desklets.xml desklets.dll
	monodocer -importslashdoc:desklets.xml -path:docs -assembly:desklets.dll

desklets.tree desklets.zip: docs/Mono.Desklets.xml
	mdassembler --ecma docs/ --out desklets

install: desklets.tree desklets.zip desklets.dll
	cp desklets.tree desklets.zip desklets.source `pkg-config monodoc --variable sourcesdir`
	echo TODO: install desklets.dll into the gac
	echo TODO: create and install desklets.pc into the pkgconfig thing

