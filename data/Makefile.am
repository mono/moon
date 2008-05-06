olivedir  = $(prefix)/lib/mono/3.0
silverdir = $(prefix)/lib/mono/2.1

pkgconfigdir = $(prefix)/lib/pkgconfig

pkgconfig_DATA = agmono.pc silverdesktop.pc silver.pc

EXTRA_DIST=agmono.pc.in silverdesktop.pc.in silver.pc.in

silver.pc: $(srcdir)/silver.pc.in Makefile
	sed -e "s,@silverdir@,$(silverdir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/silver.pc.in > $@.tmp
	mv $@.tmp $@

silverdesktop.pc: $(srcdir)/silverdesktop.pc.in Makefile
	sed -e "s,@olivedir@,$(olivedir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/silverdesktop.pc.in > $@.tmp
	mv $@.tmp $@

agmono.pc: $(srcdir)/agmono.pc.in Makefile
	sed -e "s,@olivedir@,$(olivedir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/agmono.pc.in > $@.tmp
	mv $@.tmp $@

CLEANFILES =			\
	silver.pc		\
	silverdesktop.pc	\
	agmono.pc
