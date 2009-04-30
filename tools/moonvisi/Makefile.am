#SUBDIRS = viewer
moondir = $(pkglibdir)

bin_SCRIPTS = moonvisi
moon_DATA = moonvisi.exe

EXTRA_DIST = \
	$(moonvisi_exe_CS)

CLEANFILES = 		\
	moonvisi 		\
	moonvisi.exe	\
	moonvisi.exe.mdb

moonvisi_exe_CS = 		\
	$(srcdir)/moonvisi.cs	\
	$(srcdir)/../Options.cs

moonvisi.exe: $(moonvisi_exe_CS) 
	gmcs -debug+ -out:$@ $(moonvisi_exe_CS) -r:System -r:Mono.Posix

moonvisi: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > moonvisi
	chmod +x moonvisi

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],moonvisi.exe,g'
