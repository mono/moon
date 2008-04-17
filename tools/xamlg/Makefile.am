moondir = $(prefix)/lib/moon

bin_SCRIPTS = xamlg

moon_DATA = xamlg.exe

EXTRA_DIST = \
	xamlg.in \
	$(xamlg_exe_CS)

CLEANFILES = xamlg xamlg.exe

xamlg_exe_CS = 		\
	$(srcdir)/xamlg.cs

xamlg.exe: $(xamlg_exe_CS) 
	gmcs -debug -out:$@ $(xamlg_exe_CS) -r:System -r:System.Xml

xamlg: xamlg.in
	$(do_subst) < $(srcdir)/xamlg.in > xamlg
	chmod +x xamlg

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

run:
	mono --debug xamlg.exe
