assembly = ControlsExtended.Test

SDK_SRCDIR=$(srcdir)/../../../../class/Microsoft.SilverlightControls/SDKControls/Extended/Test

CLEANFILES = \
	App.xaml.g.cs	\
	$(assembly).dll	\
	$(assembly).dll.mdb	\
	$(assembly).xap	\
	$(assembly).g.resources	\
	System.Windows.Controls.Extended.Test.Resource.resources

REFERENCES = \
	-r:$(srcdir)/../Microsoft.Silverlight.Testing.dll 								\
	-r:$(srcdir)/../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	-r:$(srcdir)/../Controls.Test.Common/Controls.Test.Common.dll							\
	-r:$(srcdir)/../System.Windows.Controls.dll	\
	-r:$(srcdir)/../System.Windows.Controls.Data.dll	\
	-r:$(srcdir)/../Mono.Moonlight.UnitTesting.dll	\
	-r:System.Core	\
	-r:System	\
	-r:System.Xml	\
	-r:System.Net	\
	-r:System.Windows

CSCFLAGS = /codepage:65001 -d:SILVERLIGHT -d:MOONLIGHT -target:library -debug+ -noconfig $(REFERENCES)

SMCS = MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe  $(CSCFLAGS)

FILES = $(shell cat $(srcdir)/ControlsExtended.Test.sources)
SOURCES = $(foreach file,$(FILES),$(SDK_SRCDIR)/$(file))
SOURCES += $(srcdir)/App.xaml.g.cs

KEYFILE = $(SDK_SRCDIR)/../../MixControls.snk
MANIFEST = $(SDK_SRCDIR)/AppManifest.xaml

$(assembly).sources: $(SOURCES)
	ls $(SOURCES) | grep -v aspx > $@

App.xaml.g.cs:
	$(MONO) --debug $(top_builddir)/tools/xamlg/xamlg.exe $(SDK_SRCDIR)/App.xaml,$(srcdir)/App.xaml.g.cs

$(assembly).g.resources:
	$(MONO) --debug $(top_builddir)/tools/respack/respack.exe $(assembly).g.resources $(SDK_SRCDIR)/App.xaml,app.xaml

System.Windows.Controls.Extended.Test.Resource.resources:
	resgen2 $(SDK_SRCDIR)/Resource.resx System.Windows.Controls.Extended.Test.Resource.resources

$(assembly).dll: System.Windows.Controls.Extended.Test.Resource.resources $(assembly).g.resources $(SOURCES)
	$(SMCS) -out:$(assembly).dll $(SOURCES) -keyfile:$(KEYFILE) -res:$(assembly).g.resources -res:System.Windows.Controls.Extended.Test.Resource.resources

$(assembly).xap: $(assembly).dll
	rm -f $(assembly).xap
	zip -j $(assembly).xap \
		$(assembly).dll* \
		$(MANIFEST)	\
		$(srcdir)/../Controls.Test.Common/Controls.Test.Common.dll*	\
		$(srcdir)/../Microsoft.Silverlight.Testing.dll*	\
		$(srcdir)/../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll*	\
		$(srcdir)/../System.Windows.Controls.dll*	\
		$(srcdir)/../System.Windows.Controls.Data.dll*	\
		$(srcdir)/../System.Xml.Linq.dll*	\
		$(srcdir)/../Mono.Moonlight.UnitTesting.dll*
	
all-local: $(assembly).xap
