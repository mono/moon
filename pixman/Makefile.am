SUBDIRS = pixman test

#pkgconfigdir=$(libdir)/pkgconfig
noinst_DATA=pixman-1.pc

$(pkgconfig_DATA): pixman-1.pc.in

snapshot:
	distdir="$(distdir)-`date '+%Y%m%d'`"; \
	test -d "$(srcdir)/.git" && distdir=$$distdir-`cd "$(srcdir)" && git-rev-parse HEAD | cut -c 1-6`; \
	$(MAKE) $(AM_MAKEFLAGS) distdir="$$distdir" dist

RELEASE_OR_SNAPSHOT = $$(if test "x$(CAIRO_VERSION_MINOR)" = "x$$(echo "$(CAIRO_VERSION_MINOR)/2*2" | bc)" ; then echo release; else echo snapshot; fi)
RELEASE_CAIRO_HOST =	cairographics.org
RELEASE_CAIRO_DIR =	/srv/cairo.freedesktop.org/www/releases
RELEASE_CAIRO_URL = 	http://cairographics.org/releases
RELEASE_XORG_HOST =	xorg.freedesktop.org
RELEASE_XORG_DIR =	/srv/xorg.freedesktop.org/archive/individual/lib
RELEASE_ANNOUNCE_LIST = cairo-announce@cairographics.org

tar_gz = $(PACKAGE)-$(VERSION).tar.gz
tar_bz2 = $(PACKAGE)-$(VERSION).tar.bz2
sha1_file = $(tar_gz).sha1
gpg_file = $(sha1_file).asc

$(sha1_file): $(tar_gz)
	sha1sum $^ > $@

$(gpg_file): $(sha1_file)
	@echo "Please enter your GPG password to sign the checksum."
	gpg --armor --sign $^ 

release-verify-newer:
	@echo -n "Checking that no $(VERSION) release already exists..."
	@ssh $(RELEASE_CAIRO_HOST) test ! -e $(RELEASE_CAIRO_DIR)/$(tar_gz) \
		|| (echo "Ouch." && echo "Found: $(RELEASE_CAIRO_HOST):$(RELEASE_CAIRO_DIR)/$(tar_gz)" \
		&& echo "Refusing to try to generate a new release of the same name." \
		&& false)
	@echo "Good."

release-remove-old:
	$(RM) $(tar_gz) $(tar_bz2) $(sha1_file) $(gpg_file)

release-check: release-verify-newer release-remove-old distcheck

release-upload: release-check $(tar_gz) $(tar_bz2) $(sha1_file) $(gpg_file)
	mkdir -p releases
	scp $(tar_gz) $(sha1_file) $(gpg_file) $(RELEASE_CAIRO_HOST):$(RELEASE_CAIRO_DIR)
	scp $(tar_gz) $(tar_bz2) $(sha1_file) $(gpg_file) $(RELEASE_XORG_HOST):$(RELEASE_XORG_DIR)
	mv $(tar_gz) $(tar_bz2) $(sha1_file) $(gpg_file) releases
	ssh $(RELEASE_CAIRO_HOST) "rm -f $(RELEASE_CAIRO_DIR)/LATEST-$(PACKAGE)-[0-9]* && ln -s $(tar_gz) $(RELEASE_CAIRO_DIR)/LATEST-$(PACKAGE)-$(VERSION)"
	git-tag -s -m "$(PACKAGE) $(VERSION) release" $(PACKAGE)-$(VERSION)

release-publish-message: releases/$(sha1_file)
	@echo "Please follow the instructions in RELEASING to push stuff out and"
	@echo "send out the announcement mails.  Here is the excerpt you need:"
	@echo ""
	@echo "Subject: $(PACKAGE) release $(VERSION) now available"
	@echo ""
	@echo "============================== CUT HERE =============================="
	@echo "A new $(PACKAGE) release $(VERSION) is now available from:"
	@echo ""
	@echo "	$(RELEASE_CAIRO_URL)/$(tar_gz)"
	@echo ""
	@echo "    which can be verified with:"
	@echo ""
	@echo "	$(RELEASE_CAIRO_URL)/$(sha1_file)"
	@echo -n "	"
	@cat releases/$(sha1_file)
	@echo ""
	@echo "	$(RELEASE_CAIRO_URL)/$(gpg_file)"
	@echo "	(signed by `getent passwd "$$USER" | cut -d: -f 5 | cut -d, -f 1`)"
	@echo ""
	@echo "  Additionally, a git clone of the source tree:"
	@echo ""
	@echo "	git clone git://git.freedesktop.org/git/pixman"
	@echo ""
	@echo "    will include a signed $(PACKAGE)-$(VERSION) tag which points to a commit named:"
	@echo "	`git cat-file tag $(PACKAGE)-$(VERSION) | grep ^object | sed -e 's,object ,,'`"
	@echo ""
	@echo "    which can be verified with:"
	@echo "	git verify-tag $(PACKAGE)-$(VERSION)"
	@echo ""
	@echo "    and can be checked out with a command such as:"
	@echo "	git checkout -b build $(PACKAGE)-$(VERSION)"
	@echo ""
	@echo "============================== CUT HERE =============================="

release-publish: release-upload release-publish-message

.PHONY: release-upload release-publish release-publish-message
