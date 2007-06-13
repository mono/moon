bin_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS)

libmoon_la_SOURCES = 	\
	animation.cpp	\
	animation.h	\
	brush.cpp	\
	clock.cpp	\
	clock.h		\
	cutil.cpp	\
	cutil.h		\
	ffvideo.cpp	\
	geometry.cpp	\
	geometry.h	\
	media.cpp	\
	media.h		\
	rsvg.cpp	\
	rsvg.h		\
	runtime.cpp	\
	runtime.h	\
	shape.cpp	\
	shape.h		\
	text.cpp	\
	transform.cpp	\
	transform.h	\
	type.cpp	\
	value.cpp	\
	value.h		\
	video.cpp	\
	xaml.cpp

demo_SOURCES = \
	demo.cpp

libmoon_la_LIBADD = $(MOON_LIBS)

demo_LDADD = -lmoon 

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo

valgrind:
	LD_LIBRARY_PATH=.libs valgrind .libs/demo
