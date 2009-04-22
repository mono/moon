SMCS = $(MONO) --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe

noinst_DATA= Mono.Moonlight.UnitTesting.dll

SOURCES = \
	$(srcdir)/AssemblyInfo.cs										\
	$(srcdir)/Mono.Moonlight.UnitTesting/MoonlightBugAttribute.cs	\
	$(srcdir)/Mono.Moonlight.UnitTesting/MoonLogProvider.cs			\
	$(srcdir)/Mono.Moonlight.UnitTesting/SilverlightBugAttribute.cs	\
	$(srcdir)/Mono.Moonlight.UnitTesting/Testing.cs	\
	$(srcdir)/Mono.Moonlight.UnitTesting/TestLogger.cs	\
	$(srcdir)/Mono.Moonlight.UnitTesting/TestPlugin.cs
	
EXTRA_DIST = \
	$(SOURCES) \
	$(srcdir)/site/MoonLogProvider.aspx \
	$(srcdir)/site/MoonLogProvider.aspx.cs \
	$(srcdir)/site/moon-unit-log.xsl        


CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)
	
KEYFILE = $(srcdir)/../moonlight.snk

REFERENCES = \
	-r:$(top_builddir)/test/2.0/Mono.Moonlight.UnitTesting/Microsoft.Silverlight.Testing.dll							\
	-r:$(top_builddir)/test/2.0/Mono.Moonlight.UnitTesting/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight	

Mono.Moonlight.UnitTesting.dll: $(SOURCES) Makefile.am Microsoft.Silverlight.Testing.dll Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll
	MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" $(SMCS) $(SOURCES) $(REFERENCES) -delaysign+ -keyfile:$(KEYFILE) -target:library -out:$@ -debug:full
	sn -q -R $@ $(KEYFILE)

Microsoft.Silverlight.Testing.dll: ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll
	cp ../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll* .

System.Xml.Linq.dll: ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll
	cp ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll* .

Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll: ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll
	cp ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll* .

../Microsoft.Silverlight.Testing/Microsoft.Silverlight.Testing.dll ../Microsoft.Silverlight.Testing/System.Xml.Linq.dll:
	$(MAKE) -C ../Microsoft.Silverlight.Testing

../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll:
	$(MAKE) -C ../Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight

all: Mono.Moonlight.UnitTesting.dll
