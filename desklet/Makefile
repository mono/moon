all: docs/Mono.Desklets.xml desklets.dll

desklets.dll desklets.xml: desklets.cs
	gmcs -debug -doc:desklets.xml -target:library desklets.cs -pkg:silver -pkg:gtk-sharp-2.0

docs/Mono.Desklets.xml: desklets.xml desklets.dll
	monodocer -importsslashdoc:desklets.xml -path:docs -assembly:desklets.dll

desklets.tree desklets.zip: docs/Mono.Desklets.xml
	mdassembler --ecma docs/ --out desklets

install:
	echo TODO: install desklets.dll into the gac
	echo TODO: create and install desklets.pc into the pkgconfig thing
	echo TODO: install documentation
