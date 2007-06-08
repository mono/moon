bin_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS)

libmoon_la_SOURCES = 	\
	cutil.cpp	\
	brush.cpp	\
	animation.cpp	\
	animation.h	\
	geometry.cpp	\
	geometry.h	\
	runtime.cpp	\
	runtime.h	\
	transform.cpp	\
	transform.h	\
	shape.h		\
	shape.cpp	\
	video.cpp	\
	ffvideo.cpp	\
	xaml.cpp

demo_SOURCES = \
	demo.cpp

libmoon_la_LIBADD = $(MOON_LIBS)

demo_LDADD = -lmoon 

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo
