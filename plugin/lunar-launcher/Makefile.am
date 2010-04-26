bin_PROGRAMS = lunar-launcher

INCLUDES = $(LUNAR_LAUNCHER_CFLAGS) \
	-DG_LOG_DOMAIN=\"LunarLauncher\" \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/pal \
	-I$(top_srcdir)/src/zip \
	-I/usr/include/X11 \
	-Wall

lunar_launcher_SOURCES = 	\
	lunar-launcher.cpp	\
	getopts.cpp		\
	getopts.h

lunar_launcher_LDADD = $(top_builddir)/plugin/libmoonplugin.la $(LUNAR_LAUNCHER_LIBS)
