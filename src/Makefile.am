libmoon_includedir=$(includedir)/libmoon

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS) -Wall -DUSE_OPT_RGB24=1 -DG_LOG_DOMAIN=\"Moonlight\"

libmoon_include_HEADERS = \
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
	dependencyproperty.h \
	dirty.h		\
	downloader.h	\
	enums.h		\
	eventargs.h	\
	error.h		\
	file-downloader.h	\
	font.h		\
	frameworkelement.h \
	fullscreen.h	\
	geometry.h	\
	http-streaming.h	\
	internal-downloader.h	\
	layout.h	\
	list.h		\
	media.h		\
	mms-downloader.h	\
	moon-path.h	\
	moon-curves.h	\
	mp3.h		\
	mplayer.h	\
	namescope.h	\
	openfile.h	\
	panel.h		\
	pipeline.h	\
	playlist.h	\
	point.h		\
	rect.h		\
	region.h	\
	runtime.h	\
	shape.h		\
	size.h		\
	stylus.h	\
	text.h		\
	transform.h	\
	trigger.h	\
	type.h		\
	uielement.h	\
	uri.h		\
	utils.h		\
	value.h		\
	visual.h	\
	window.h	\
	window-gtk.h	\
	xaml.h		\
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
	dependencyproperty.cpp	\
	dirty.h	\
	dirty.cpp	\
	downloader.cpp	\
	eventargs.cpp	\
	file-downloader.cpp	\
	font.cpp	\
	frameworkelement.cpp	\
	geometry.cpp	\
	http-streaming.cpp	\
	layout.cpp	\
	list.cpp	\
	media.cpp	\
	mms-downloader.cpp	\
	moon-path.c	\
	moon-curves.c	\
	enums.c		\
	mp3.cpp		\
	mplayer.cpp	\
	namescope.cpp	\
	openfile.cpp	\
	panel.cpp	\
	pipeline.cpp	\
	point.cpp	\
	playlist.cpp	\
	rect.cpp	\
	region.cpp	\
	runtime.cpp	\
	shape.cpp	\
	stylus.cpp	\
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
	window-gtk.cpp	\
	xaml.cpp	\
	yuv-converter.cpp	\
	zip/ioapi.c	\
	zip/ioapi.h	\
	zip/unzip.c	\
	zip/unzip.h	\
	zip/crypt.h

if INCLUDE_MONO_RUNTIME
libmoon_include_HEADERS += \
	deployment.h	\
	grid.h		\
	usercontrol.h	\
	xap.h

libmoon_la_SOURCES += \
	deployment.cpp	\
	grid.cpp	\
	usercontrol.cpp \
	xap.cpp
endif

if INCLUDE_FFMPEG
libmoon_la_SOURCES += pipeline-ffmpeg.cpp pipeline-ffmpeg.h
endif

EXTRA_DIST = \
	alpha-premul-table.inc \
	zip/README \
	zip/ChangeLogUnzip \
	$(libmooninclude_headers)

libmoon_la_LIBADD = $(MOON_LIBS)

