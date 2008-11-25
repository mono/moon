default: moon-unit.sources
	$(MAKE) -C ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight
	$(MAKE) -C ../Microsoft.Silverlight.Testing
	cp ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll .
	cp ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll .
	mxap --debug --generate-html- --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=moon-unit.sources
	cp *.xap site/ClientBin

test: default
	firefox site/index.html

moon-unit.sources: *.cs */*.cs
	ls *.cs */*.cs > moon-unit.sources

# TODO: Fix this to detect changes in the sources of any of the following two libraries.
Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	cd ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight && $(MAKE)
	cp ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll .

Microsoft.Silverlight.Testing.dll:
	cd ../Microsoft.Silverlight.Testing && $(MAKE)
	cp ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll .
