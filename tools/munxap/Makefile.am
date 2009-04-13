moondir = $(pkglibdir)

bin_SCRIPTS = munxap

moon_DATA = munxap.exe

EXTRA_DIST = \
	$(munxap_exe_CS) \
	$(munxap_resources)

CLEANFILES = 		\
	munxap 		\
	munxap.exe	\
	munxap.exe.mdb

munxap_exe_CS = 		\
	$(srcdir)/munxap.cs	\
	$(srcdir)/../Options.cs	\
	$(srcdir)/Zip.cs	\
	$(srcdir)/View.cs

munxap_resources = 			\
	$(srcdir)/cm/c.gif			\
	$(srcdir)/cm/d.gif			\
	$(srcdir)/cm/e.gif			\
	$(srcdir)/cm/en.gif			\
	$(srcdir)/cm/f.gif			\
	$(srcdir)/cm/i.gif			\
	$(srcdir)/cm/m.gif			\
	$(srcdir)/cm/n.gif			\
	$(srcdir)/cm/p.gif			\
	$(srcdir)/cm/r.gif			\
	$(srcdir)/cm/s.gif			\
	$(srcdir)/cm/sc.gif			\
	$(srcdir)/cm/se.gif			\
	$(srcdir)/cm/sm.gif			\
	$(srcdir)/cm/st.gif			\
	$(srcdir)/cm/sx.gif			\
	$(srcdir)/cm/tb.gif			\
	$(srcdir)/cm/tm.gif			\
	$(srcdir)/cm/tp.gif			\
	$(srcdir)/cm/w.gif			\
	$(srcdir)/cm/y.gif			\
	$(srcdir)/cm/mn.png

ress= $(foreach res,$(munxap_resources), $(addprefix -resource:,$(res)),$(notdir $(res)))

munxap.exe: $(munxap_exe_CS) 
	gmcs -debug+ -out:$@ $(munxap_exe_CS) -r:System -r:System.Xml -pkg:gtk-sharp-2.0 -r:`pkg-config --variable=Libraries cecil` $(ress)

munxap: $(srcdir)/../launcher.in Makefile
	$(do_subst) < $(srcdir)/../launcher.in > munxap
	chmod +x munxap

do_subst = sed -e 's,[@]moondir[@],$(moondir),g' -e 's,[@]VERSION[@],$(VERSION),g' -e 's,[@]TOOL[@],munxap.exe,g'

run: munxap.exe
	mono --debug munxap.exe
