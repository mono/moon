moondir = $(prefix)/lib/moon

bin_SCRIPTS = munxap

moon_DATA = munxap.exe

EXTRA_DIST = \
	$(munxap_exe_CS)

CLEANFILES = 		\
	munxap 		\
	munxap.exe	\
	munxap.exe.mdb

munxap_exe_CS = 		\
	$(srcdir)/munxap.cs	\
	$(srcdir)/../Options.cs	\
	$(srcdir)/Zip.cs	\
	$(srcdir)/View.cs

munxap.exe: $(munxap_exe_CS) 
	gmcs -debug+ -out:$@ $(munxap_exe_CS) -r:System -r:System.Xml -pkg:gtk-sharp-2.0

munxap: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > munxap
	chmod +x munxap

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],munxap.exe,g'

run: munxap.exe
	mono --debug munxap.exe
