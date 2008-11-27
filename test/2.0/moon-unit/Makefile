default: moon-unit.sources
	$(MAKE) -C ../Microsoft.Silverlight.Testing
	cp ../Microsoft.Silverlight.Testing/*.dll* .
	mxap --debug --generate-html- --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=moon-unit.sources
	mkdir -p site/ClientBin
	cp *.xap site/ClientBin

test: default
	firefox site/index.html

moon-unit.sources: *.cs */*.cs
	ls *.cs */*.cs | grep -v aspx > moon-unit.sources
