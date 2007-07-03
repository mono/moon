moondir = $(prefix)/lib/moon

bin_SCRIPTS = xamlg

man_MANS = xamlg.1

moon_DATA = xamlg.exe

EXTRA_DIST = xamlg.in

CLEANFILES = xamlg xamlg.exe

xamlg_exe_CS = xamlg.cs

xamlg.exe: $(xamlg_exe_CS) 
	gmcs -debug xamlg.cs -r:System -r:System.Xml

xamlg: xamlg.in
	$(do_subst) < $(srcdir)/xamlg.in > xamlg
	chmod +x xamlg

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

run:
	mono --debug xamlg.exe