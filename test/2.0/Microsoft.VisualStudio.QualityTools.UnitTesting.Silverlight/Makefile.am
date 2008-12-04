noinst_DATA= Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll

CLEANFILES=Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
	   Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll: $(wildcard */*.cs)
	smcs -recurse:*.cs -target:library -out:Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll -debug:full
