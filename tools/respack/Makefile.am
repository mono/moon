moondir = $(prefix)/lib/moon

bin_SCRIPTS = respack

moon_DATA = respack.exe

EXTRA_DIST = \
	respack.in \
	respack.cs

CLEANFILES = 		\
	respack 	\
	respack.exe	\
	respack.exe.mdb

respack.exe: $(srcdir)/respack.cs
	gmcs -d:LINQ -debug -out:$@ $(srcdir)/respack.cs $(srcdir)/../Options.cs

respack: $(srcdir)/../launcher.in
	$(do_subst) < $(srcdir)/../launcher.in > respack
	chmod +x respack

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],respack,g'

run:
	mono --debug xaml2html.exe
