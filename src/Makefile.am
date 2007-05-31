bin_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

libmoon_la_SOURCES = 	\
	brush.cpp	\
	runtime.cpp	\
	runtime.h	\
	shape.cpp	\
	video.cpp	\
	videoff.cpp

demo_SOURCES = \
	demo.cpp

demo_LDADD = -lmoon

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo