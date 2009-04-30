
SUBDIRS=shocker

install-test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C shocker install-test-plugin

dist-hook:
	cp -a $(srcdir)/masters $(distdir)/masters
	find $(distdir)/masters -name .svn | xargs rm -rf
