moondir = $(pkglibdir)

bin_SCRIPTS = respack unrespack

moon_DATA = respack.exe

EXTRA_DIST = \
	respack.cs

CLEANFILES = 		\
	respack 	\
	respack.exe	\
	respack.exe.mdb \
	unrespack

respack.exe: $(srcdir)/respack.cs
	gmcs -d:LINQ -debug -out:$@ $(srcdir)/respack.cs $(srcdir)/../Options.cs

respack: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > respack
	chmod +x respack

unrespack: $(srcdir)/../launcher.in Makefile
	$(do_subst_un) < $(srcdir)/../launcher.in > unrespack
	chmod +x unrespack

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],respack.exe,g'

do_subst_un = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],respack.exe -u,g'
