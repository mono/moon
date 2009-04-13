moondir = $(pkglibdir)

bin_SCRIPTS = mxap

moon_DATA = mxap.exe

EXTRA_DIST = \
	$(mxap_exe_CS)

CLEANFILES = 		\
	mxap 		\
	mxap.exe	\
	mxap.exe.mdb

mxap_exe_CS = 		\
	$(srcdir)/mxap.cs	\
	$(srcdir)/../Options.cs

mxap.exe: $(mxap_exe_CS) 
	gmcs -debug+ -out:$@ $(mxap_exe_CS) -r:System -r:System.Xml

mxap: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > mxap
	chmod +x mxap

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],mxap.exe,g'

run:
	mono --debug mxap.exe
