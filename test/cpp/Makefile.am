noinst_PROGRAMS = unit

GUNIT_DIR = gtest-1.3.0

unit_SOURCES = \
	main.cpp	\
	utils.cpp	\
	mms.cpp

unit_LDADD = $(MOON_PROG_LIBS)
unit_LDFLAGS = -static $(shell $(GUNIT_DIR)/scripts/gtest-config --ldflags --libs)
unit_CPPFLAGS = -static $(MOON_PROG_CFLAGS) -I$(top_srcdir)/src/ -I$(top_srcdir)/src/asf $(shell $(GUNIT_DIR)/scripts/gtest-config --cppflags --cxxflags)

# there is a flaw here: $(GUNIT_DIR) does not exist in svn, it's downloaded on demand.
# However $(GUNIT_DIR)/scripts/gtest-config is required when executing make and then make fails to execute the $(GUNIT_DIR) rule.
# do: "make gtest-1.3.0 && touch Makefile.am && make"

run: $(GUNIT_DIR) all
	unit

valgrind: $(GUNIT_DIR) all
	valgrind --error-exitcode=1 --leak-check=full unit

gdb: $(GUNIT_DIR) all
	gdb --eval-command=run unit

$(GUNIT_DIR):
	wget http://googletest.googlecode.com/files/$(GUNIT_DIR).tar.gz
	tar zvfx $(GUNIT_DIR).tar.gz
	cd $(GUNIT_DIR) && configure && make