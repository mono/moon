
INCLUDES=@MOON_CFLAGS@ -I$(srcdir)/../../src

noinst_PROGRAMS=xaml2png
xaml2png_SOURCES=xaml2png.cpp
xaml2png_LDADD=$(top_builddir)/src/libmoon.la

MOON_TESTS=../xaml/test-zindex.xaml ../xaml/test-render-xorigin.xaml ../xaml/test-rtol.xaml

baseline:
	@for t in $(MOON_TESTS); do \
		./run-test.sh baseline $$t `basename $$t`; \
	done	
check:
	@for t in $(MOON_TESTS); do \
		./run-test.sh test $$t `basename $$t`; \
	done

clean:
	rm -f *.jpg