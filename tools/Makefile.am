SUBDIRS = 
if GTK_PAL
SUBDIRS += mopen
endif

if INCLUDE_MANAGED_CODE
#don't include respack because the toplevel includes that already
SUBDIRS += xamlg xaml2html mxap unsign sockpol
endif

if HAVE_GTKSHARP
SUBDIRS += munxap
endif

DIST_SUBDIRS = $(SUBDIRS) moonvisi respack  # added as dist_subdir so as not to break make dist

EXTRA_DIST = launcher.in Options.cs README
