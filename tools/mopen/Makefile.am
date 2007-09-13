moondir = $(prefix)/lib/moon

moon_DATA = mopen.exe mopen.exe.mdb mopen.exe.config

bin_SCRIPTS = mopen

EXTRA_DIST = mopen.in

CLEANFILES = mopen mopen.exe mopen.exe.mdb
DISTCLEANFILES=$(CLEANFILES)

mopen_exe_CS = $(srcdir)/mopen.cs		\
               $(srcdir)/CompositeHelper.cs

mopen.exe: $(mopen_exe_CS) ../../gtk/gtksilver.dll
	gmcs -debug -out:mopen.exe -target:exe $(mopen_exe_CS) -r:../../gtk/gtksilver.dll -pkg:gtk-sharp-2.0 -pkg:mono-cairo -pkg:silverdesktop

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

mopen: $(srcdir)/mopen.in Makefile
	$(do_subst) < $(srcdir)/mopen.in > mopen
	chmod +x mopen

