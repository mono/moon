SMCS = $(MONO) --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe

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
	MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" $(SMCS) -recurse:$(srcdir)/*.cs -target:library -out:Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll -debug:full -delaysign+ -keyfile:$(KEYFILE)
	sn -q -R $@ $(KEYFILE)
