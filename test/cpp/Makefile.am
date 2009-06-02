noinst_PROGRAMS = unit

GUNIT_DIR = gtest-1.3.0
GUNIT_SO = $(GUNIT_DIR)/lib/.libs/libgtest.so

unit_SOURCES = \
	main.cpp	\
	utils.cpp	\
	mms.cpp

unit_LDADD = $(MOON_PROG_LIBS)
unit_LDFLAGS = -static $(shell $(GUNIT_DIR)/scripts/gtest-config --ldflags --libs)
unit_CPPFLAGS = -static $(MOON_PROG_CFLAGS) -I$(top_srcdir)/src/ -I$(top_srcdir)/src/asf $(shell $(GUNIT_DIR)/scripts/gtest-config --cppflags --cxxflags)

$(GUNIT_SO): $(GUNIT_DIR)
	cd $(GUNIT_DIR) && configure && make

run: $(GUNIT_SO) all
	unit

valgrind: $(GUNIT_SO) all
	valgrind --error-exitcode=1 --leak-check=full unit

gdb: $(GUNIT_SO) all
	gdb --eval-command=run unit

$(GUNIT_DIR):
	wget http://googletest.googlecode.com/files/$(GUNIT_DIR).tar.gz
	tar zvfx $(GUNIT_DIR).tar.gz