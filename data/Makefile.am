olivedir  = $(prefix)/lib/mono/3.0
silverdir = $(prefix)/lib/mono/2.1

pkgconfigdir = $(prefix)/lib/pkgconfig

pkgconfig_DATA = agmono.pc silverdesktop.pc silver.pc

EXTRA_DIST=agmono.pc.in silverdesktop.pc.in silver.pc.in

silver.pc: silver.pc.in Makefile
	sed -e "s,@silverdir@,$(silverdir),g" -e "s,\@VERSION\@,$(VERSION),g" < silver.pc.in > $@.tmp
	mv $@.tmp $@

silverdesktop.pc: silverdesktop.pc.in Makefile
	sed -e "s,@olivedir@,$(olivedir),g" -e "s,\@VERSION\@,$(VERSION),g" < silverdesktop.pc.in > $@.tmp
	mv $@.tmp $@

agmono.pc: agmono.pc.in Makefile
	sed -e "s,@olivedir@,$(olivedir),g" -e "s,\@VERSION\@,$(VERSION),g" < agmono.pc.in > $@.tmp
	mv $@.tmp $@
