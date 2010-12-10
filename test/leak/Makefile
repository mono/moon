BUILD_FILES = App.xaml App.xaml.cs Page.xaml Page.xaml.cs
CLEAN_FILES = ${BUILD_FILES} AppManifest.xaml  App.xaml.g.cs  Page.xaml.g.cs Leak* xdb.*
SUBDIRS := $(wildcard */)
OBJECTS := $(addsuffix Leak.dll,$(SUBDIRS))

all: $(OBJECTS)

$(OBJECTS): %Leak.dll : %Test.cs $(BUILD_FILES)
	@echo "Making $@"
	@-rm -f $(patsubst %,$*%,$(CLEAN_FILES))
	@$(foreach f,$(BUILD_FILES), \
		cp ${f} $*; \
	)
	@(cd $* && mxap --application-name=Leak)
	@cp Leak.html $*

clean:
	@$(foreach subdir,$(SUBDIRS), \
		$(foreach f,$(CLEAN_FILES), \
			rm -f ${subdir}/${f}; \
		) \
	)
	
install:

