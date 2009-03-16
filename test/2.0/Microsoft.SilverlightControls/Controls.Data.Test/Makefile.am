assembly = Controls.Data.Test

SDK_SRCDIR=$(srcdir)/../../../../class/Microsoft.SilverlightControls/SDKControls/Data/test

CLEANFILES = \
	$(assembly).xap	\
	test.dll	\
	test.dll.mdb	\
	$(assembly).sources	\
	test.g.resources

REFERENCES = \
	-r:$(srcdir)/../Microsoft.Silverlight.Testing.dll 								\
	-r:$(srcdir)/../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	-r:$(srcdir)/../Controls.Test.Common/Controls.Test.Common.dll							\
	-r:System.Windows.Controls	\
	-r:System.Core	\
	-r:System	\
	-r:System.Xml	\
	-r:System.Net	\
	-r:System.Windows	\
	-r:System.Windows.Controls.Data

CSCFLAGS = /codepage:65001 -d:SILVERLIGHT -target:library -debug+ -noconfig $(REFERENCES)

SMCS = MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe  $(CSCFLAGS)

SOURCES = \
	$(wildcard $(SDK_SRCDIR)/*.xaml.cs)	\
	$(wildcard $(SDK_SRCDIR)/*/*.cs)	\
	$(wildcard $(SDK_SRCDIR)/*/*/*.cs) 

KEYFILE = $(SDK_SRCDIR)/../../MixControls.snk
MANIFEST = $(SDK_SRCDIR)/AppManifest.xaml

$(assembly).sources: $(SOURCES)
	ls $(SOURCES) | grep -v aspx > $@

test.g.resources:
	$(MONO) --debug $(top_builddir)/tools/respack/respack.exe test.g.resources $(SDK_SRCDIR)/App.xaml,app.xaml

$(assembly).dll: test.g.resources
	$(SMCS) -out:test.dll $(SOURCES) -keyfile:$(KEYFILE) -res:test.g.resources

$(assembly).xap: $(assembly).dll
	rm -f $(assembly).xap
	zip -j -MM $(assembly).xap \
		test.dll \
		$(MANIFEST)	\
		../Controls.Test.Common/Controls.Test.Common.dll	\
		../Microsoft.Silverlight.Testing.dll	\
		../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
		$(top_builddir)/class/lib/2.1/System.Windows.Controls.Data.dll	\
		$(top_builddir)/class/lib/2.1/System.Xml.Linq.dll
	
test: $(assembly).xap