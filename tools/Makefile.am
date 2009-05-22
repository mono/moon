SUBDIRS = mopen generators
if INCLUDE_MANAGED_CODE
#don't include respack because the toplevel includes that already
SUBDIRS += xamlg xaml2html mxap munxap unsign sockpol
endif

DIST_SUBDIRS = $(SUBDIRS) moonvisi respack  # added as dist_subdir so as not to break make dist

EXTRA_DIST = launcher.in Options.cs README
