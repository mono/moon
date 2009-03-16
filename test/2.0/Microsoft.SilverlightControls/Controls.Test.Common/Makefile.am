assembly = Controls.Test.Common

SDK_SRCDIR=$(srcdir)/../../../../class/Microsoft.SilverlightControls/SDKControls/Test/Controls.Test.Common

CLEANFILES = \
	$(assembly).dll	\
	$(assembly).dll.mdb

REFERENCES = \
	-r:$(srcdir)/../Microsoft.Silverlight.Testing.dll	\
	-r:$(srcdir)/../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	-r:System.Net	\
	-r:System	\
	-r:System.Core	\
	-r:System.Xml	\
	-r:System.Windows

CSCFLAGS = /codepage:65001  -d:SILVERLIGHT -debug+ -noconfig $(REFERENCES)

SMCS = MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe  $(CSCFLAGS)

sources =							\
	$(SDK_SRCDIR)/Common.cs				\
	$(SDK_SRCDIR)/LiveReference.cs		\
	$(SDK_SRCDIR)/OverriddenMethod.cs	\
	$(SDK_SRCDIR)/SilverlightControlTest.cs		\
	$(SDK_SRCDIR)/SilverlightTestExtensions.cs	\
	$(SDK_SRCDIR)/Properties/AssemblyInfo.cs

KEYFILE = $(SDK_SRCDIR)/../../MixControls.snk

$(assembly).dll: $(sources)
	$(SMCS) -target:library -out:$@ $(sources) -keyfile:$(KEYFILE)

all-local: $(assembly).dll
