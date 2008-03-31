
SUBDIRS=shocker agviewer test-runner

install-test-plugin:
	$(MAKE) $(AM_MAKEFLAGS) -C shocker install-test-plugin

dist-hook:
	cp -a $(srcdir)/masters $(distdir)/masters
	cp -a $(srcdir)/baselines $(distdir)/baselines
	find $(distdir)/masters $(distdir)/baselines -name .svn | xargs rm -rf
