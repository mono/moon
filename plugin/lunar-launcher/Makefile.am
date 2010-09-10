bin_PROGRAMS = lunar-launcher
bindir = $(DESTDIR)$(pkglibdir)/plugin/

INCLUDES = $(LUNAR_LAUNCHER_CFLAGS) \
	-DG_LOG_DOMAIN=\"LunarLauncher\" \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/pal \
	-I$(top_srcdir)/src/zip \
	-I$(top_srcdir)/plugin	\
	-I/usr/include/X11 \
	-Wall

lunar_launcher_SOURCES = 	\
	lunar-launcher.cpp

lunar_launcher_LDADD = $(top_builddir)/plugin/libmoonplugin.la $(LUNAR_LAUNCHER_LIBS)
