default: moon-unit.sources
	$(MAKE) -C ../Microsoft.Silverlight.Testing
	cp ../Microsoft.Silverlight.Testing/*.dll* .
	mxap --debug --generate-html- --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=moon-unit.sources
	cp *.xap site/ClientBin

test: default
	firefox site/index.html

moon-unit.sources: *.cs */*.cs
	ls *.cs */*.cs > moon-unit.sources
