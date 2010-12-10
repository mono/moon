BUILD_FILES = App.xaml App.xaml.cs Page.xaml Page.xaml.cs
CLEAN_FILES = ${BUILD_FILES} AppManifest.xaml  App.xaml.g.cs  Page.xaml.g.cs Leak* xdb.*
SUB_DIRS = ${shell find -mindepth 1 -type d -print}

all: clean
	$(foreach subdir,$(SUB_DIRS), \
		$(foreach f,$(BUILD_FILES), \
			cp ${f} ${subdir}; \
		) \
		cd ${subdir}; \
		mxap --application-name=Leak; \
		cd ..; \
		cp Leak.html ${subdir}; \
	)

clean:
	$(foreach subdir,$(SUB_DIRS), \
		$(foreach f,$(CLEAN_FILES), \
			rm -f ${subdir}/${f}; \
		) \
	)
