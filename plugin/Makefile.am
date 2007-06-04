
XPIDL=`pkg-config --variable=libdir xulrunner-xpcom`/xpidl
XPFLAGS=-I `pkg-config --variable=includedir xulrunner-xpcom`

lib_LTLIBRARIES = libmoonplugin.la

INCLUDES = \
	-I$(top_srcdir)/src \
	`pkg-config --cflags gtk+-2.0` \
	`pkg-config --cflags nspr` \
	`pkg-config --cflags xulrunner-xpcom` \
	-I/usr/include/X11

libmoonplugin_la_LDFLAGS = \
	`pkg-config --libs gtk+-2.0` \
	`pkg-config --libs nspr` \
	-L$(top_srcdir)/src/.libs

libmoonplugin_la_LIBADD = -lmoon

libmoonplugin_la_SOURCES = \
	moon-plugin.cpp \
	moon-plugin.h \
	moon-plugin-glue.cpp \
	nsScriptablePeer.cpp \
	nsScriptablePeer.h

# custom install rule
idl:
	$(XPIDL) -m header $(XPFLAGS) nsIScriptableMoonPlugin.idl
	$(XPIDL) -m typelib $(XPFLAGS) nsIScriptableMoonPlugin.idl

install-plugin: libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	cp .libs/libmoonplugin.so ~/.mozilla/plugins
	cp $(top_srcdir)/src/.libs/libmoon.so ~/.mozilla/plugins
