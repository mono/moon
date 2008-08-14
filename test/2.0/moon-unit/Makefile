default: moon-unit.sources
	mxap --debug --generate-html- --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=moon-unit.sources
	cp *.xap site/ClientBin

test: default
	firefox site/index.html

moon-unit.sources: *.cs */*.cs
	ls *.cs */*.cs > moon-unit.sources
