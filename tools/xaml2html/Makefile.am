moondir = $(prefix)/lib/moon

bin_SCRIPTS = xaml2html

moon_DATA = xaml2html.exe

EXTRA_DIST = \
	xaml2html.in \
	$(xaml2html_exe_CS)

CLEANFILES = 		\
	xaml2html 	\
	xaml2html.exe	\
	xaml2html.exe.mdb

xaml2html_exe_CS = 	\
	$(srcdir)/xaml2html.cs

xaml2html.exe: $(xaml2html_exe_CS) 
	gmcs -debug -out:$@ $(xaml2html_exe_CS)

xaml2html: xaml2html.in
	$(do_subst) < $(srcdir)/xaml2html.in > xaml2html
	chmod +x xaml2html

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' 

run:
	mono --debug xaml2html.exe
