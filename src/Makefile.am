
libmoonincludedir=$(includedir)/libmoon

noinst_PROGRAMS = demo sizes

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS) -Wall

libmooninclude_headers = \
	animation.h	\
	array.h		\
	brush.h		\
	canvas.h	\
	clock.h		\
	collection.h	\
	color.h		\
	control.h	\
	debug.h		\
	dependencyobject.h \
	downloader.h	\
	enums.h		\
	error.h		\
	font.h		\
	frameworkelement.h \
	fullscreen.h	\
	garray-ext.h	\
	geometry.h	\
	list.h		\
	mango.h		\
	media.h		\
	moon-path.h	\
	mplayer.h	\
	namescope.h	\
	openfile.h	\
	panel.h		\
	playlist.h	\
	point.h		\
	rect.h		\
	rsvg.h		\
	runtime.h	\
	shape.h		\
	stylus.h	\
	text.h		\
	transform.h	\
	trigger.h	\
	type.h		\
	uielement.h	\
	uri.h		\
	value.h		\
	visual.h	\
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
	debug.cpp	\
	dependencyobject.cpp	\
	dirty.h	\
	dirty.cpp	\
	downloader.cpp	\
	font.cpp	\
	frameworkelement.cpp	\
	garray-ext.cpp	\
	geometry.cpp	\
	list.cpp	\
	mango.cpp	\
	media.cpp	\
	moon-path.c	\
	mplayer.cpp	\
	namescope.cpp	\
	openfile.cpp	\
	panel.cpp	\
	point.cpp	\
	playlist.cpp	\
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
	uri.cpp		\
	value.cpp	\
	visual.cpp	\
	xaml.cpp	\
	zip/ioapi.c	\
	zip/ioapi.h	\
	zip/unzip.c	\
	zip/unzip.h	\
	zip/crypt.h

EXTRA_DIST = \
	alpha-premul-table.inc \
	zip/README \
	zip/ChangeLogUnzip \
	$(libmooninclude_headers)

demo_SOURCES = \
	demo.cpp

sizes_SOURCES = \
	sizes.cpp

libmoon_la_LIBADD = $(MOON_LIBS)

demo_LDADD = -lmoon $(MOON_PROG_LIBS)

sizes_LDADD = -lmoon $(MOON_PROG_LIBS)

debug:
	LD_LIBRARY_PATH=.libs gdb .libs/demo

valgrind:
	LD_LIBRARY_PATH=.libs valgrind .libs/demo
