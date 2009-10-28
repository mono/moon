moondir = $(pkglibdir)

bin_PROGRAMS = mopen1
mopen1_SOURCES = $(srcdir)/mopen1.cpp
INCLUDES = $(MOON_PROG_CFLAGS) -Wall -I$(top_srcdir)/src/asf
LIBS = $(MOON_PROG_LIBS)

if INCLUDE_DESKTOP_MANAGED_CODE
moon_DATA = mopen.exe mopen.exe.mdb mopen.exe.config

bin_SCRIPTS = mopen
endif

EXTRA_DIST = \
	mopen.exe.config \
	$(mopen_exe_CS)

CLEANFILES = mopen mopen.exe mopen.exe.mdb
DISTCLEANFILES=$(CLEANFILES)

mopen_exe_CS = 	$(srcdir)/mopen.cs		\
		$(srcdir)/CompositeHelper.cs	\
		$(srcdir)/../Options.cs

mopen.exe mopen.exe.mdb: $(mopen_exe_CS) $(top_builddir)/class/lib/moonlight/System.Windows.dll $(top_builddir)/class/lib/moonlight/Moon.Windows.Desktop.dll $(top_builddir)/gtk/Moonlight.Gtk.dll
	gmcs -debug -out:mopen.exe -target:exe $(mopen_exe_CS) -r:$(top_builddir)/class/lib/moonlight/System.Windows.dll -r:$(top_builddir)/class/lib/moonlight/Moon.Windows.Desktop.dll -r:$(top_builddir)/gtk/Moonlight.Gtk.dll -pkg:gtk-sharp-2.0 -pkg:mono-cairo -pkg:wnck-sharp-1.0

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],mopen.exe,g'

mopen: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > mopen
	chmod +x mopen

