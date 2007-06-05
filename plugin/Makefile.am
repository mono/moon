lib_LTLIBRARIES = libmoonplugin.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I/usr/include/X11 \
	$(PLUGIN_CFLAGS)

libmoonplugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-L$(top_srcdir)/src/.libs \
	$(PLUGIN_LIBS)

libmoonplugin_la_LIBADD = -lpthread -lmoon

libmoonplugin_la_SOURCES = \
	moon-plugin.cpp \
	moon-plugin.h \
	moon-plugin-glue.cpp \
	nsScriptablePeer.cpp \
	nsScriptablePeer.h

# custom install rule
idl:
	$(XPIDL) -m header $(XPFLAGS) nsIScriptableMoonPlugin.idl

install-plugin: libmoonplugin.la
	mkdir -p ~/.mozilla/plugins
	cp .libs/libmoonplugin.so ~/.mozilla/plugins
	cp $(top_srcdir)/src/.libs/libmoon.so ~/.mozilla/plugins
	$(XPIDL) -m typelib $(XPFLAGS) -o ~/.mozilla/plugins/libmoonplugin nsIScriptableMoonPlugin.idl
