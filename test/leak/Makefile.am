XAML_FILES = App.xaml Page.xaml
SOURCE_FILES = App.xaml.cs Page.xaml.cs
BUILD_FILES = $(XAML_FILES) $(SOURCE_FILES)
CLEANFILES = AppManifest.xaml  App.xaml.g.cs  Page.xaml.g.cs \
	$(wildcard */*.xap) $(wildcard */*.xaml.cs) $(wildcard */*.g.cs) \
	$(wildcard */*.xaml) $(wildcard */*.g.resources) $(wildcard */*.dll) \
	$(wildcard */*.mdb) $(wildcard */*.html)
LEAK_DIRS = $(wildcard */)
XAPS = $(addsuffix Leak.xap,$(LEAK_DIRS))

noinst_DATA = $(XAPS)

all-local: $(XAPS)

%/Leak.xap: $(BUILD_FILES) %/Test.cs
	@echo "Making $@"
	@rm -f $*/Leak.*
	@cp $(BUILD_FILES) $*
	@(cd $* && $(SYSTEM_MONO) ../../../tools/mxap/mxap.exe --application-name=Leak --generate-html=no)
	@cp Leak.html $*
