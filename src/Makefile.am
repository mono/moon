
libmoonincludedir=$(includedir)/libmoon

noinst_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS)

libmooninclude_HEADERS = \
	animation.h	\
	brush.h		\
	clock.h		\
	control.h	\
	cutil.h		\
	downloader.h	\
	geometry.h	\
	media.h		\
	list.h		\
	mplayer.h	\
	rsvg.h		\
	runtime.h	\
	shape.h		\
	stylus.h	\
	text.h		\
	transform.h	\
	type.h		\
	value.h		\
	xaml.h


libmoon_la_SOURCES = 	\
	runtime.cpp	\
	animation.cpp	\
	animation.h	\
	brush.cpp	\
	clock.cpp	\
	control.cpp	\
	cutil.cpp	\
	downloader.cpp	\
	geometry.cpp	\
	list.cpp	\
	media.cpp	\
	mplayer.cpp	\
	rsvg.cpp	\
	shape.cpp	\
	stylus.cpp	\
	text.cpp	\
	transform.cpp	\
	transform.h	\
	type.cpp	\
	value.cpp	\
	xaml.cpp

EXTRA_DIST=alpha-premul-table.inc

demo_SOURCES = \
	demo.cpp

libmoon_la_LIBADD = $(MOON_LIBS)

demo_LDADD = -lmoon 

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo

valgrind:
	LD_LIBRARY_PATH=.libs valgrind .libs/demo
