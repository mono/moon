SDK_ROOT=$(exec_prefix)/lib/moonlight

desktopdir = $(DESTDIR)$(libdir)/mono/$(PACKAGE)
webdir = $(DESTDIR)$(SDK_ROOT)/2.0

pkgconfigdir = $(datadir)/pkgconfig

pkgconfig_DATA=

if INCLUDE_DESKTOP_MANAGED_CODE
pkgconfig_DATA += moonlight-desktop-2.0.pc
endif

if INCLUDE_BROWSER_MANAGED_CODE
pkgconfig_DATA += moonlight-web-2.0.pc
endif


moonlight-web-2.0.pc: $(srcdir)/moonlight-web-2.0.pc.in Makefile
	sed -e "s,@webdir@,$(webdir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/moonlight-web-2.0.pc.in > $@.tmp
	mv $@.tmp $@

moonlight-desktop-2.0.pc: $(srcdir)/moonlight-desktop-2.0.pc.in Makefile
	sed -e "s,@desktopdir@,$(desktopdir),g" -e "s,\@VERSION\@,$(VERSION),g" < $(srcdir)/moonlight-desktop-2.0.pc.in > $@.tmp
	mv $@.tmp $@

EXTRA_DIST=				\
	moonlight-web-2.0.pc.in		\
	moonlight-desktop-2.0.pc.in

CLEANFILES =				\
	moonlight-web-2.0.pc		\
	moonlight-desktop-2.0.pc
