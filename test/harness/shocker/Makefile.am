

lib_LTLIBRARIES = libshocker.la

INCLUDES = -I$(top_srcdir)/plugin/moz-sdk $(SHOCKER_CFLAGS) -Wall -DDBUS_ENABLED

libshocker_la_SOURCES = 	\
	shocker.cpp  		\
	shocker.h		\
	plugin.h		\
	plugin.cpp		\
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
	harness.h			\
	harness.cpp			\
	shutdown-manager.h	\
	shutdown-manager.cpp

libshocker_la_LDFLAGS = \
	-avoid-version -module

libshocker_la_LIBADD = $(SHOCKER_LIBS)

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo

valgrind:
	LD_LIBRARY_PATH=.libs valgrind .libs/demo

install-test-plugin: libshocker.la
	mkdir -p ~/.mozilla/plugins
	-rm -f ~/.mozilla/plugins/libshocker.so
	cp .libs/libshocker.so ~/.mozilla/plugins
