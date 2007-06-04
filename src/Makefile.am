bin_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

libmoon_la_SOURCES = 	\
	brush.cpp	\
	runtime.cpp	\
	runtime.h	\
	shape.h		\
	shape.cpp	\
	video.cpp	\
	ffvideo.cpp	\
	xaml.cpp

demo_SOURCES = \
	demo.cpp

demo_LDADD = -lmoon

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo
