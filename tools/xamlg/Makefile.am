moondir = $(pkglibdir)

bin_SCRIPTS = xamlg

moon_DATA = xamlg.exe

EXTRA_DIST = \
	$(xamlg_exe_CS)

CLEANFILES = 		\
	xamlg 		\
	xamlg.exe	\
	xamlg.exe.mdb

xamlg_exe_CS = 			\
	$(srcdir)/xamlg.cs	\
	$(srcdir)/../Options.cs

xamlg.exe: $(xamlg_exe_CS) 
	gmcs -debug -out:$@ $(xamlg_exe_CS) -r:System -r:System.Xml

xamlg: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > xamlg
	chmod +x xamlg

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],xamlg.exe,g'

run:
	mono --debug xamlg.exe
