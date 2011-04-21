XAML_FILES = App.xaml Page.xaml
SOURCE_FILES = App.xaml.cs Page.xaml.cs
BUILD_FILES = AppManifest.xaml $(XAML_FILES) $(SOURCE_FILES)
CLEANFILES = App.xaml.g.cs  Page.xaml.g.cs \
	$(wildcard */*.xap) $(wildcard */*.xaml.cs) $(wildcard */*.g.cs) \
	$(wildcard */*.xaml) $(wildcard */*.g.resources) $(wildcard */*.dll) \
	$(wildcard */*.mdb) $(wildcard */*.html)
LEAK_DIRS = $(wildcard */)
XAPS = $(addsuffix Leak.xap,$(LEAK_DIRS))

noinst_DATA = $(XAPS)

$(XAPS): %Leak.xap: $(BUILD_FILES) %Test.cs
	@echo "Making $@"
	@rm -f $*/Leak.*
	@cp $(BUILD_FILES) $*
	@-(cd $* && $(SYSTEM_MONO) $(abs_top_builddir)/tools/xamlg/xamlg.exe -sl2app:Leak $(XAML_FILES))
	@-(cd $* && $(SYSTEM_MONO) $(abs_top_builddir)/tools/respack/respack.exe Leak.g.resources $(XAML_FILES))
	@-(cd $* && $(SMCS) -debug+ -r:System.Windows.dll -r:System.Windows.Browser.dll -target:library -out:Leak.dll -resource:Leak.g.resources -warnaserror+ $(SOURCE_FILES) Test.cs $(addsuffix .g.cs,$(XAML_FILES)))
	@-(cd $* && zip -q Leak.xap Leak.dll Leak.dll.mdb AppManifest.xaml Leak.dll Leak.dll.mdb)
	@cp Leak.html $*

clean:
	@-rm -f $(CLEANFILES)

