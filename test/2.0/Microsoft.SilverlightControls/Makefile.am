SUBDIRS = \
	.						\
	Controls.Test.Common	\
	Controls.Data.Test		\
	Controls.Extended.Test	\
	site

CLEANFILES = \
	Microsoft.Silverlight.Testing.dll	\
	Microsoft.Silverlight.Testing.dll.mdb	\
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll	\
	Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb	\
	Mono.Moonlight.UnitTesting.dll	\
	Mono.Moonlight.UnitTesting.dll.mdb	\
	System.Xml.Linq.dll	\
	System.Xml.Linq.dll.mdb	\
	System.Windows.Controls.dll	\
	System.Windows.Controls.dll.mdb	\
	System.Windows.Controls.Data.dll	\
	System.Windows.Controls.Data.dll.mdb

SYSTEM_XML_LINQ_DLL = $(top_builddir)/class/lib/2.1/System.Xml.Linq.dll
SYSTEM_WINDOWS_CONTROLS_DLL = $(top_builddir)/class/lib/2.1/System.Windows.Controls.dll
SYSTEM_WINDOWS_CONTROLS_DATA_DLL = $(top_builddir)/class/lib/2.1/System.Windows.Controls.Data.dll

all-local: Microsoft.Silverlight.Testing.dll Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll System.Xml.Linq.dll System.Windows.Controls.dll System.Windows.Controls.Data.dll Mono.Moonlight.UnitTesting.dll
	
Microsoft.Silverlight.Testing.dll:
	-cp $(srcdir)/../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll* .

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	-cp $(srcdir)/../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll* .

Mono.Moonlight.UnitTesting.dll:
	-cp $(srcdir)/../Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll* .

System.Xml.Linq.dll:
	RESIGN_DLL=$(SYSTEM_XML_LINQ_DLL) $(MAKE) resign

System.Windows.Controls.dll: $(SYSTEM_WINDOWS_CONTROLS_DLL)
	RESIGN_DLL=$(SYSTEM_WINDOWS_CONTROLS_DLL) $(MAKE) resign

System.Windows.Controls.Data.dll: $(SYSTEM_WINDOWS_CONTROLS_DATA_DLL)
	RESIGN_DLL=$(SYSTEM_WINDOWS_CONTROLS_DATA_DLL) $(MAKE) resign

resign:
	$(MONO) --debug $(top_builddir)/tools/unsign/unsign.exe $(RESIGN_DLL) --resign $(srcdir)/../moonlight.snk
	mv `basename $(RESIGN_DLL)`.Unsigned `basename $(RESIGN_DLL)`
	-cp $(RESIGN_DLL).mdb .

install:
