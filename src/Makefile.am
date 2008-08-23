libmoon_includedir=$(includedir)/libmoon

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS) -Wall -DUSE_OPT_RGB24=1 -DG_LOG_DOMAIN=\"Moonlight\"

libmoon_include_HEADERS = 	\
	animation.h		\
	animation2.h		\
	asf/asf.h		\
	asf/asf-generated.h	\
	asf/asf-guids.h		\
	asf/asf-structures.h	\
	asf/asf-debug.h		\
	audio.h			\
	brush.h			\
	canvas.h		\
	cbinding.h		\
	clock.h			\
	collection.h		\
	color.h			\
	debug.h			\
	dependencyobject.h	\
	dependencyproperty.h	\
	dirty.h			\
	downloader.h		\
	enums.h			\
	eventargs.h		\
	error.h			\
	file-downloader.h	\
	font.h			\
	frameworkelement.h 	\
	fullscreen.h		\
	geometry.h		\
	http-streaming.h	\
	internal-downloader.h	\
	layout.h		\
	list.h			\
	media.h			\
	mms-downloader.h	\
	moon-path.h		\
	moon-curves.h		\
	mp3.h			\
	mplayer.h		\
	namescope.h		\
	openfile.h		\
	panel.h			\
	pipeline.h		\
	playlist.h		\
	point.h			\
	rect.h			\
	region.h		\
	runtime.h		\
	shape.h			\
	size.h			\
	stylus.h		\
	text.h			\
	transform.h		\
	trigger.h		\
	type.h			\
	uielement.h		\
	uri.h			\
	utils.h			\
	value.h			\
	window.h		\
	window-gtk.h		\
	xaml.h			\
	yuv-converter.h		\
	libmoon.h


libmoon_la_SOURCES = 		\
	$(libmoon_la_headers) 	\
	animation.cpp		\
	asf/asf.cpp		\
	asf/asf-generated.cpp	\
	asf/asf-guids.cpp	\
	asf/asf-structures.cpp	\
	asf/asf-debug.cpp	\
	audio.cpp		\
	brush.cpp		\
	canvas.cpp		\
	cbinding.cpp		\
	clock.cpp		\
	collection.cpp		\
	color.cpp		\
	debug.cpp		\
	dependencyobject.cpp	\
	dependencyproperty.cpp	\
	dependencyproperty.g.cpp\
	dirty.h			\
	dirty.cpp		\
	downloader.cpp		\
	error.cpp		\
	eventargs.cpp		\
	file-downloader.cpp	\
	font.cpp		\
	frameworkelement.cpp	\
	geometry.cpp		\
	http-streaming.cpp	\
	layout.cpp		\
	list.cpp		\
	media.cpp		\
	mms-downloader.cpp	\
	moon-path.c		\
	moon-curves.c		\
	enums.cpp		\
	mp3.cpp			\
	mplayer.cpp		\
	namescope.cpp		\
	openfile.cpp		\
	panel.cpp		\
	pipeline.cpp		\
	point.cpp		\
	playlist.cpp		\
	rect.cpp		\
	region.cpp		\
	runtime.cpp		\
	shape.cpp		\
	size.cpp		\
	stylus.cpp		\
	text.cpp		\
	transform.cpp		\
	trigger.cpp		\
	type.cpp		\
	type-generated.cpp	\
	uielement.cpp		\
	uri.cpp			\
	utils.cpp		\
	value.cpp		\
	window-gtk.cpp		\
	xaml.cpp		\
	yuv-converter.cpp	\
	zip/ioapi.c		\
	zip/ioapi.h		\
	zip/unzip.c		\
	zip/unzip.h		\
	zip/crypt.h

if INCLUDE_MANAGED_CODE
libmoon_include_HEADERS += 	\
	border.h		\
	contentcontrol.h	\
	control.h		\
	cornerradius.h		\
	deployment.h		\
	grid.h			\
	stackpanel.h		\
	style.h			\
	template.h		\
	textbox.h		\
	thickness.h		\
	tilesource.h		\
	usercontrol.h		\
	xap.h

libmoon_la_SOURCES += 		\
	border.cpp		\
	contentcontrol.cpp	\
	control.cpp		\
	cornerradius.cpp	\
	deployment.cpp		\
	grid.cpp		\
	style.cpp		\
	template.cpp		\
	textbox.cpp		\
	thickness.cpp		\
	usercontrol.cpp 	\
	xap.cpp
endif

if INCLUDE_FFMPEG
libmoon_la_SOURCES += pipeline-ffmpeg.cpp pipeline-ffmpeg.h
endif

if INCLUDE_ALSA
libmoon_la_SOURCES += audio-alsa.cpp audio-alsa.h
endif

if INCLUDE_PULSEAUDIO
libmoon_la_SOURCES += audio-pulse.cpp audio-pulse.h
endif

EXTRA_DIST = \
	alpha-premul-table.inc \
	zip/README \
	zip/ChangeLogUnzip \
	$(libmooninclude_headers)

libmoon_la_LIBADD = $(MOON_LIBS)

