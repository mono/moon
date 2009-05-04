bin_SCRIPTS = sockpol

moondir = $(pkglibdir)
moon_DATA = sockpol.exe

CLEANFILES = sockpol.exe \
	     sockpol.exe.mdb

EXTRA_DIST = $(sockpol_exe_CS)

sockpol_exe_CS = $(srcdir)/sockpol.cs

sockpol.exe: $(sockpol_exe_CS) 
	gmcs $(sockpol_exe_CS) -target:exe -out:$@ -debug

sockpol: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > sockpol
	chmod +x sockpol

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],sockpol.exe,g'

