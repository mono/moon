SUBDIRS = src gtk plugin 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=moon.pc

EXTRA_DIST=moon.pc.in

# custom install rule
install-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C plugin install-plugin
