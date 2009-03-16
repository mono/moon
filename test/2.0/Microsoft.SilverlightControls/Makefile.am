SUBDIRS = \
	.						\
	Controls.Test.Common	\
	Controls.Data.Test		\
	Controls.Extended.Test	\
	site

IGNITE_DIR=$(srcdir)/../../../class/Microsoft.SilverlightControls/SDKControls/Test/Ignite

CLEANFILES = \
	Microsoft.Silverlight.Testing.dll	\
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll

all-local: Microsoft.Silverlight.Testing.dll Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll
	
Microsoft.Silverlight.Testing.dll:
	-cp $(IGNITE_DIR)/Microsoft.Silverlight.Testing.dll* .

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	-cp $(IGNITE_DIR)/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll* .

install: