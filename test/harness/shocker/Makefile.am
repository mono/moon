

lib_LTLIBRARIES = libshocker.la

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/plugin/moz-sdk $(SHOCKER_CFLAGS) -Wall -DDBUS_ENABLED

libshocker_la_SOURCES = 	\
	debug.cpp		\
	debug.h			\
	clipboard.cpp		\
	clipboard.h		\
	shocker.cpp  		\
	shocker.h		\
	shocker-plugin.h	\
	shocker-plugin.cpp	\
	browser.h		\
	browser.cpp		\
	netscape.h		\
	netscape.cpp		\
	logging.h		\
	logging.cpp		\
	input.h			\
	input.cpp		\
	image-capture.h		\
	image-capture.cpp	\
	harness.h		\
	harness.cpp		\
	shutdown-manager.h	\
	shutdown-manager.cpp	\
	texthelpers.h		\
	texthelpers.cpp		\
	$(top_srcdir)/src/moonlightconfiguration.cpp

libshocker_la_LDFLAGS = \
	-avoid-version -module

libshocker_la_LIBADD = \
	$(PULSEAUDIO_SIMPLE_LIBS) \
	$(SHOCKER_LIBS) \
	$(top_builddir)/plugin/libmoonplugin.la


debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo

valgrind:
	LD_LIBRARY_PATH=.libs valgrind .libs/demo

test-plugin install-test-plugin: libshocker.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libshocker.so
	cp .libs/libshocker.so ~/.mozilla/plugins
