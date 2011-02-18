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

mxap: mxap.in Makefile
	$(do_subst) < mxap.in > mxap
	chmod +x mxap

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],mxap.exe,g' -e 's,[@]PREFIX[@],$(prefix),g'

run:
	mono --debug mxap.exe
