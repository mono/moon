libmoon_includedir=$(includedir)/libmoon

lib_LTLIBRARIES = libmoon.la

INCLUDES = $(MOON_CFLAGS) -Wall -DUSE_OPT_RGB24=1 -DG_LOG_DOMAIN=\"Moonlight\" -I$(srcdir)/asf

libmoon_include_headers = 	\
	animation2.h		\
	animation.h		\
	applier.h		\
	asf/asf-debug.h		\
	asf/asf-generated.h	\
	asf/asf-guids.h		\
	asf/asf.h		\
	asf/asf-structures.h	\
	audio.h			\
	binding.h		\
	bitmapimage.h		\
	border.h		\
	brush.h			\
	canvas.h		\
	cbinding.h		\
	clock.h			\
	codec-url.h		\
	collection.h		\
	color.h			\
	contentcontrol.h	\
	control.h		\
	cornerradius.h		\
	debug.h			\
	deepzoomimagetilesource.h \
	dependencyobject.h	\
	dependencyproperty.h	\
	deployment.h		\
	dirty.h			\
	downloader.h		\
	enums.h			\
	error.h			\
	eventargs.h		\
	expression.h		\
	file-downloader.h	\
	font.h			\
	frameworkelement.h 	\
	fullscreen.h		\
	geometry.h		\
	grid.h			\
	http-streaming.h	\
	internal-downloader.h	\
	layout.h		\
	libmoon.h		\
	list.h			\
	media.h			\
	mediaelement.h		\
	mediaplayer.h		\
	mms-downloader.h	\
	moon-curves.h		\
	moonlightconfiguration.h\
	moon-path.h		\
	mp3.h			\
	multiscaleimage.h	\
	multiscalesubimage.h	\
	namescope.h		\
	openfile.h		\
	panel.h			\
	pipeline.h		\
	pipeline-asf.h		\
	pipeline-ui.h		\
	pipeline-nocodec-ui.h	\
	playlist.h		\
	point.h			\
	rect.h			\
	region.h		\
	resources.h		\
	runtime.h		\
	shape.h			\
	size.h			\
	stackpanel.h		\
	style.h			\
	stylus.h		\
	template.h		\
	textbox.h		\
	text.h			\
	thickness.h		\
	tilesource.h		\
	transform.h		\
	trigger.h		\
	type.h			\
	uielement.h		\
	uri.h			\
	usercontrol.h		\
	utils.h			\
	value.h			\
	window-gtk.h		\
	window.h		\
	xaml.h			\
	xap.h			\
	yuv-converter.h

libmoon_la_SOURCES = 		\
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
	mediaelement.cpp	\
	mediaplayer.cpp		\
	mms-downloader.cpp	\
	moon-path.c		\
	moon-curves.c		\
	moonlightconfiguration.cpp	\
	applier.cpp		\
	enums.cpp		\
	mp3.cpp			\
	namescope.cpp		\
	openfile.cpp		\
	panel.cpp		\
	pipeline.cpp		\
	pipeline-asf.cpp	\
	pipeline-logo.cpp	\
	point.cpp		\
	playlist.cpp		\
	rect.cpp		\
	region.cpp		\
	resources.cpp		\
	runtime.cpp		\
	shape.cpp		\
	size.cpp		\
	stackpanel.cpp		\
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

libmoon_la_sources_2 =		\
	binding.cpp		\
	border.cpp		\
	contentcontrol.cpp	\
	control.cpp		\
	cornerradius.cpp	\
	deepzoomimagetilesource.cpp \
	deployment.cpp		\
	grid.cpp		\
	multiscaleimage.cpp	\
	style.cpp		\
	template.cpp		\
	textbox.cpp		\
	tilesource.cpp		\
	thickness.cpp		\
	usercontrol.cpp 	\
	xap.cpp

if CODECS_SUPPORTED
libmoon_la_SOURCES += pipeline-ui.cpp
else
libmoon_la_SOURCES += pipeline-nocodec-ui.cpp
endif

if INCLUDE_MANAGED_CODE
libmoon_include_headers += $(libmoon_include_headers_2)
libmoon_la_SOURCES      += $(libmoon_la_sources_2)
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
	$(libmoon_include_headers)

libmoon_la_LIBADD = $(MOON_LIBS)

