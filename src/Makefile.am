libmoonincludedir=$(includedir)/libmoon

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS) -Wall -DUSE_OPT_RGB24=1 -DG_LOG_DOMAIN=\"Moonlight\"

libmooninclude_headers = \
	animation.h	\
	array.h		\
	asf/asf.h		\
	asf/asf-generated.h	\
	asf/asf-guids.h	\
	asf/asf-structures.h	\
	asf/asf-debug.h	\
	brush.h		\
	canvas.h	\
	clock.h		\
	collection.h	\
	color.h		\
	control.h	\
	debug.h		\
	dependencyobject.h \
	deployment.h	\
	downloader.h	\
	enums.h		\
	eventargs.h	\
	error.h		\
	font.h		\
	frameworkelement.h \
	fullscreen.h	\
	geometry.h	\
	http-streaming.h	\
	layout.h	\
	list.h		\
	mango.h		\
	media.h		\
	moon-path.h	\
	mplayer.h	\
	namescope.h	\
	openfile.h	\
	panel.h		\
	pipeline.h	\
	pipeline-ffmpeg.h \
	playlist.h	\
	point.h		\
	rect.h		\
	region.h	\
	runtime.h	\
	shape.h		\
	stylus.h	\
	swscale-converter.h	\
	text.h		\
	transform.h	\
	trigger.h	\
	type.h		\
	uielement.h	\
	uri.h		\
	utils.h		\
	value.h		\
	visual.h	\
	xaml.h		\
	xap.h		\
	yuv-converter.h	\
	libmoon.h


libmoon_la_SOURCES = 	\
	$(libmoon_la_headers) \
	animation.cpp	\
	array.cpp	\
	asf/asf.cpp		\
	asf/asf-generated.cpp	\
	asf/asf-guids.cpp	\
	asf/asf-structures.cpp	\
	asf/asf-debug.cpp	\
	brush.cpp	\
	canvas.cpp	\
	color.cpp	\
	clock.cpp	\
	collection.cpp	\
	control.cpp	\
	debug.cpp	\
	dependencyobject.cpp	\
	deployment.cpp	\
	dirty.h	\
	dirty.cpp	\
	downloader.cpp	\
	eventargs.cpp	\
	font.cpp	\
	frameworkelement.cpp	\
	geometry.cpp	\
	http-streaming.cpp	\
	layout.cpp	\
	list.cpp	\
	mango.cpp	\
	media.cpp	\
	moon-path.c	\
	enums.c		\
	mplayer.cpp	\
	namescope.cpp	\
	openfile.cpp	\
	panel.cpp	\
	pipeline.cpp \
	pipeline-ffmpeg.cpp	\
	point.cpp	\
	playlist.cpp	\
	rect.cpp	\
	region.cpp	\
	runtime.cpp	\
	shape.cpp	\
	stylus.cpp	\
	swscale-converter.cpp	\
	text.cpp	\
	transform.cpp	\
	trigger.cpp	\
	type.cpp	\
	type-generated.cpp	\
	uielement.cpp	\
	uri.cpp		\
	utils.cpp	\
	value.cpp	\
	visual.cpp	\
	xaml.cpp	\
	xap.cpp		\
	yuv-converter.cpp	\
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

libmoon_la_LIBADD = $(MOON_LIBS)

