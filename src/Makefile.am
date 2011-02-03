libmoon_includedir=$(includedir)/libmoon

lib_LTLIBRARIES = libmoon.la
noinst_LTLIBRARIES = libmoonxpi.la
noinst_SCRIPTS = xaml-to-header.exe

INCLUDES = $(MOON_CFLAGS) -Wall -DUSE_OPT_RGB24=1 -DG_LOG_DOMAIN=\"Moonlight\" -I$(top_srcdir)/plugin -I$(srcdir)/asf -fvisibility=hidden -DBUILDING_MOONLIGHT=1

libmoon_include_headers = 	\
	animation.h		\
	application.h		\
	applier.h		\
	asf/asf-debug.h		\
	asf/asf-generated.h	\
	asf/asf-guids.h		\
	asf/asf.h		\
	asf/asf-structures.h	\
	audio.h			\
	bitmapcache.h		\
	bitmapimage.h		\
	bitmapsource.h		\
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
	dependencyobject.h	\
	dependencyproperty.h	\
	deployment.h		\
	deepzoomimagetilesource.h	\
	dirty.h			\
	downloader.h		\
	easing.h		\
	enums.h			\
	effect.h		\
	error.h			\
	eventargs.h		\
	file-downloader.h	\
	fontmanager.h		\
	fonts.h			\
	fontfamily.h		\
	fontsource.h		\
	fontstretch.h		\
	fontstyle.h		\
	fontweight.h		\
	frameworkelement.h 	\
	geometry.h		\
	glyphs.h		\
	grid.h			\
	http-streaming.h	\
	imagesource.h		\
	internal-downloader.h	\
	keyboard.h		\
	layout.h		\
	layoutinformation.h	\
	libmoon.h		\
	list.h			\
	managedtypeinfo.h	\
	media.h			\
	mediaelement.h		\
	mediaplayer.h		\
	messagebox.h		\
	mms-downloader.h	\
	moon-curves.h		\
	moonlightconfiguration.h\
	moon-path.h		\
	mp3.h			\
	multiscaleimage.h	\
	multiscalesubimage.h	\
	mutex.h			\
	namescope.h		\
	openfile.h		\
	panel.h			\
	pipeline.h		\
	pipeline-asf.h		\
	pipeline-ui.h		\
	pipeline-nocodec-ui.h	\
	playlist.h		\
	point.h			\
	popup.h			\
	projection.h		\
	propertypath.h		\
	provider.h		\
	ptr.h			\
	rect.h			\
	region.h		\
	resources.h		\
	runtime.h		\
	security.h		\
	shape.h			\
	size.h			\
	style.h			\
	stylus.h		\
	tabnavigationwalker.h	\
	template.h		\
	textblock.h		\
	textbox.h		\
	textoptions.h		\
	thickness.h		\
	textoptions.h		\
	tilesource.h		\
	timeline.h		\
	timemanager.h		\
	timesource.h		\
	transform.h		\
	trigger.h		\
	type.h			\
	uielement.h		\
	uri.h			\
	usercontrol.h		\
	utils.h			\
	validators.h		\
	value.h			\
	window-gtk.h		\
	window.h		\
	writeablebitmap.h	\
	xaml.h			\
	xap.h			\
	yuv-converter.h

libmoon_la_SOURCES = 		\
	animation.cpp		\
	application.cpp		\
	applier.cpp		\
	asf/asf.cpp		\
	asf/asf-debug.cpp	\
	asf/asf-generated.cpp	\
	asf/asf-guids.cpp	\
	asf/asf-structures.cpp	\
	audio.cpp		\
	bitmapcache.cpp		\
	bitmapimage.cpp		\
	bitmapsource.cpp	\
	border.cpp		\
	brush.cpp		\
	canvas.cpp		\
	cbinding.cpp		\
	clock.cpp		\
	collection.cpp		\
	color.cpp		\
	contentcontrol.cpp	\
	control.cpp		\
	cornerradius.cpp	\
	debug.cpp		\
	deepzoomimagetilesource.cpp \
	dependencyobject.cpp	\
	dependencyproperty.cpp	\
	dependencyproperty.g.cpp\
	deployment.cpp		\
	dirty.cpp		\
	dirty.h			\
	downloader.cpp		\
	easing.cpp		\
	effect.cpp		\
	enums.cpp		\
	error.cpp		\
	eventargs.cpp		\
	file-downloader.cpp	\
	fontmanager.cpp		\
	fonts.cpp		\
	frameworkelement.cpp	\
	geometry.cpp		\
	glyphs.cpp		\
	grid.cpp		\
	http-streaming.cpp	\
	imagesource.cpp		\
	keyboard.cpp		\
	layout.cpp		\
	layoutinformation.cpp	\
	list.cpp		\
	managedtypeinfo.cpp	\
	media.cpp		\
	mediaelement.cpp	\
	mediaplayer.cpp		\
	messagebox.cpp		\
	mms-downloader.cpp	\
	moon-curves.c		\
	moonlightconfiguration.cpp	\
	moon-path.c		\
	mp3.cpp			\
	multiscaleimage.cpp	\
	multiscalesubimage.cpp	\
	namescope.cpp		\
	openfile.cpp		\
	panel.cpp		\
	pipeline-asf.cpp	\
	pipeline.cpp		\
	pipeline-logo.cpp	\
	playlist.cpp		\
	point.cpp		\
	popup.cpp		\
	projection.cpp		\
	provider.cpp		\
	rect.cpp		\
	region.cpp		\
	resources.cpp		\
	runtime.cpp		\
	security.c		\
	shape.cpp		\
	size.cpp		\
	style.cpp		\
	stylus.cpp		\
	tabnavigationwalker.cpp	\
	template.cpp		\
	textblock.cpp		\
	textbox.cpp		\
	thickness.cpp		\
	tilesource.cpp		\
	timeline.cpp		\
	timemanager.cpp		\
	timesource.cpp		\
	transform.cpp		\
	trigger.cpp		\
	type.cpp		\
	type-generated.cpp	\
	uielement.cpp		\
	uri.cpp			\
	usercontrol.cpp 	\
	utils.cpp		\
	validators.cpp		\
	value.cpp		\
	window.cpp		\
	window-gtk.cpp		\
	writeablebitmap.cpp	\
	xaml.cpp		\
	xap.cpp			\
	yuv-converter.cpp	\
	zip/crypt.h		\
	zip/ioapi.c		\
	zip/ioapi.h		\
	zip/unzip.c		\
	zip/unzip.h

if CODECS_SUPPORTED
libmoon_la_SOURCES += pipeline-ui.cpp
else
libmoon_la_SOURCES += pipeline-nocodec-ui.cpp
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
	morton-layout-table.inc \
	fullscreen.xaml \
	incomplete-support.xaml \
	drm.xaml \
	xaml-to-header.cs \
	zip/README \
	zip/ChangeLogUnzip \
	$(libmoon_include_headers)

libmoon_la_LIBADD = $(MOON_LIBS)

libmoonxpi_la_LDFLAGS = -rpath /nowhere

libmoonxpi_la_LIBADD = $(MOON_XPI_LIBS)

libmoonxpi_la_SOURCES = $(libmoon_la_SOURCES)

runtime.lo: fullscreen.h incomplete-support.h drm.h

fullscreen.h: fullscreen.xaml xaml-to-header.exe
	$(MONO) xaml-to-header.exe fullscreen.xaml FULLSCREEN_MESSAGE

incomplete-support.h: incomplete-support.xaml xaml-to-header.exe
	$(MONO) xaml-to-header.exe incomplete-support.xaml INCOMPLETE_SUPPORT_MESSAGE

drm.h: drm.xaml xaml-to-header.exe
	$(MONO) xaml-to-header.exe drm.xaml DRM_MESSAGE

xaml-to-header.exe: xaml-to-header.cs
	gmcs xaml-to-header.cs
