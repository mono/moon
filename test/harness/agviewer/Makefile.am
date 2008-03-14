
bin_PROGRAMS = agviewer

INCLUDES = $(AGVIEWER_CFLAGS) -Wall

agviewer_SOURCES = 	\
	agviewer.cpp

agviewer_LDADD = $(AGVIEWER_LIBS)

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

