default: cgen methodgen typegen typeandkind

cgen: cgen.cs ../../src/*.cpp ../../src/*.h
	MOON_GENERATOR=cgen $(MAKE) generate

typegen: typegen.cs ../../src/*.cpp ../../src/*.h
	MOON_GENERATOR=typegen $(MAKE) generate

methodgen: methodgen.cs ../../src/*.cpp ../../src/*.h
	MOON_GENERATOR=methodgen $(MAKE) generate

typeandkind: typeandkindgen.cs  ../../src/*.cpp ../../src/*.h ../../class/System.Windows/*/*.cs
	MOON_GENERATOR=typeandkindgen $(MAKE) generate
	
generate:
	gmcs $(MOON_GENERATOR).cs common.cs -r:System.dll,System.Xml.dll -debug+ -out:$(MOON_GENERATOR).exe && \
	mono --debug $(MOON_GENERATOR).exe && \
	rm -f *.exe*
	
