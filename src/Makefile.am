
libmoonincludedir=$(includedir)/libmoon

noinst_PROGRAMS = demo

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS)

libmooninclude_headers = \
	animation.h	\
	array.h		\
	brush.h		\
	canvas.h	\
	clock.h		\
	collection.h	\
	control.h	\
	cutil.h		\
	downloader.h	\
	geometry.h	\
	mango.h		\
	media.h		\
	list.h		\
	mplayer.h	\
	namescope.h	\
	point.h		\
	rect.h		\
	rsvg.h		\
	runtime.h	\
	shape.h		\
	stylus.h	\
	text.h		\
	transform.h	\
	type.h		\
	uielement.h	\
	value.h		\
	xaml.h		\
	libmoon.h


libmoon_la_SOURCES = 	\
	$(libmoon_la_headers) \
	animation.cpp	\
	array.cpp	\
	brush.cpp	\
	canvas.cpp	\
	color.cpp	\
	clock.cpp	\
	collection.cpp	\
	control.cpp	\
	cutil.cpp	\
	dependencyobject.cpp	\
	downloader.cpp	\
	frameworkelement.cpp	\
	geometry.cpp	\
	list.cpp	\
	mango.cpp	\
	media.cpp	\
	mplayer.cpp	\
	namescope.cpp	\
	panel.cpp	\
	point.cpp	\
	rect.cpp	\
	rsvg.cpp	\
	runtime.cpp	\
	shape.cpp	\
	stylus.cpp	\
	text.cpp	\
	transform.cpp	\
	trigger.cpp	\
	type.cpp	\
	uielement.cpp	\
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
