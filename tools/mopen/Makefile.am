moondir = $(prefix)/lib/moon

bin_PROGRAMS = mopen1
mopen1_SOURCES = $(srcdir)/mopen1.cpp
INCLUDES = $(MOON_CFLAGS) -Wall  -I../../src
LIBS = $(PLUGIN_LIBS) $(MOON_LIBS) -lmoon
LDFLAGS = -L../../src/.libs

moon_DATA = mopen.exe mopen.exe.mdb mopen.exe.config

bin_SCRIPTS = mopen

EXTRA_DIST = \
	mopen.in \
	mopen.exe.config \
	$(mopen_exe_CS)

CLEANFILES = mopen mopen.exe mopen.exe.mdb
DISTCLEANFILES=$(CLEANFILES)

mopen_exe_CS = $(srcdir)/mopen.cs		\
               $(srcdir)/CompositeHelper.cs

mopen.exe: $(mopen_exe_CS) ../../class/lib/3.0/System.Windows.dll ../../class/lib/3.0/Mono.Moonlight.dll ../../gtk/gtksilver.dll
	gmcs -debug -out:mopen.exe -target:exe $(mopen_exe_CS) -r:../../class/lib/3.0/System.Windows.dll -r:../../class/lib/3.0/Mono.Moonlight.dll -r:../../gtk/gtksilver.dll -pkg:gtk-sharp-2.0 -pkg:mono-cairo

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

mopen: $(srcdir)/mopen.in Makefile
	$(do_subst) < $(srcdir)/mopen.in > mopen
	chmod +x mopen

