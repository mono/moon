xapname=moon-unit

all-local: copy-xap-to-site

noinst_DATA= $(xapname).xap

CLEANFILES=	$(xapname).xap 		\
		$(xapname).dll		\
		$(xapname).dll.mdb	\
		$(xapname).sources	\
		*.xaml.g.cs		\
		AppManifest.xaml 	\
		$(xapname).g.resources	\
		Microsoft.Silverlight.Testing.dll \
		Microsoft.Silverlight.Testing.dll.mdb \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll \
		Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll.mdb

BROWSER_ASSEMBLIES=-r:$(top_builddir)/class/lib/2.1/System.Windows.dll \
		   -r:$(top_builddir)/class/lib/2.1/System.Windows.Browser.dll \
		   -r:$(top_builddir)/class/lib/2.1/Mono.Moonlight.dll

moon-unit.xap: $(builddir)/moon-unit.sources Microsoft.Silverlight.Testing.dll Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll System.Xml.Linq.dll
	$(MONO) --debug $(top_builddir)/tools/mxap/mxap.exe --generate-html- --entry-point-type=Mono.Moonlight.UnitTesting.App --cs-sources=$(builddir)/moon-unit.sources --builddirhack=$(top_builddir) $(BROWSER_ASSEMBLIES) -r:$(builddir)/Microsoft.Silverlight.Testing.dll -r:$(builddir)/Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll  $(srcdir)/

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

moon-unit.sources: $(srcdir)/*.cs $(srcdir)/*/*.cs
	ls $(srcdir)/*.cs $(srcdir)/*/*.cs | grep -v aspx > $@

EXTRA_DIST = 			\
	$(srcdir)/*xaml.cs	\
	$(srcdir)/*xaml		\
	$(srcdir)/*/*.cs

test: copy-xap-to-site
	MONO_PATH=$(top_builddir)/class/lib/2.1:$(MONO_PATH) LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(top_builddir)/plugin/.libs firefox site/index.html

copy-xap-to-site: site/ClientBin/moon-unit.xap

site/ClientBin/moon-unit.xap: moon-unit.xap
	@mkdir -p site/ClientBin
	@cp moon-unit.xap site/ClientBin

.PHONY: copy-xap-to-site
