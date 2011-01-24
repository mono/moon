SMCS = $(prefix)/bin/mono $(prefix)/lib/mono/2.0/mcs.exe -nostdlib -lib:../../../class/lib/2.1 -r:mscorlib.dll $(CSCFLAGS)

noinst_DATA= Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll

CLEANFILES=Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
	   Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb

EXTRA_DIST =			\
	ChangeLog		\
	Microsoft.VisualStudio.TestTools.UnitTesting	\
	Microsoft.VisualStudio.TestTools.UnitTesting.Web	\
	System.Data	\
	System.Data.Common

KEYFILE=$(srcdir)/../moonlight.snk

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll: $(wildcard */*.cs)
	$(SMCS) -recurse:$(srcdir)/*.cs -target:library -out:Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
		-debug:full -delaysign+ -keyfile:$(KEYFILE) 
	sn -q -R $@ $(KEYFILE)
