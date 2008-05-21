bin_PROGRAMS = agviewer

INCLUDES = $(AGVIEWER_CFLAGS) -Wall  -DG_LOG_DOMAIN=\"Agviewer\"

agviewer_SOURCES = 	\
	agviewer.cpp	\
	agserver.h	\
	AgserverObject.h

agviewer_LDADD = $(AGVIEWER_LIBS)

AgserverObject.h:
	dbus-binding-tool --mode=glib-server --prefix=agserver AgserverObject.xml --output=AgserverObject.h

debug: all
	LD_LIBRARY_PATH=.libs gdb .libs/agviewer

valgrind: all
	LD_LIBRARY_PATH=.libs valgrind .libs/agviewer

setup-test-dir: all
	cp agviewer $(TEST_DIR)/.

setup-moon-test-dir: all
	cp agviewer $(MOON_TEST_DIR)/.

cleanup-test-dir:
	rm -rf $(TEST_DIR)/agviewer

