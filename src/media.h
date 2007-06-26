/*
 * media.h:
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MEDIA_H__
#define __MEDIA_H__

G_BEGIN_DECLS

#include <string.h>

#include "mplayer.h"
#include "enums.h"
#include "clock.h"
#include "value.h"
#include "brush.h"
#include "frameworkelement.h"

class MediaAttribute : public DependencyObject {
 public:
	static DependencyProperty *ValueProperty;
	
	MediaAttribute () { }
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE; };
};

MediaAttribute *media_attribute_new ();


class MediaBase : public FrameworkElement {
public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	
	MediaBase () { }
	virtual Type::Kind GetObjectType () { return Type::MEDIABASE; };
};

MediaBase* media_base_new ();
char *media_base_get_source (MediaBase *media);
void media_base_set_source (MediaBase *media, char *value);

Stretch media_base_get_stretch (MediaBase *media);
void    media_base_set_stretch (MediaBase *media, Stretch value);


class Image : public MediaBase {
 public:
	static DependencyProperty *DownloadProgressProperty;
	
	Image ();
	virtual ~Image ();

	virtual Type::Kind GetObjectType () { return Type::IMAGE; };
	
	virtual void render (cairo_t *cr, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual Point getxformorigin ();
	
	cairo_surface_t *GetSurface ();

	void SetSource (DependencyObject *Downloader, char *PartName);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	int GetHeight () { return pixbuf_height; };
	int GetWidth  () { return pixbuf_width; };

	ImageBrush *brush;

	bool render_progressive; /* true if we want the onscreen image
				    updated as we download */

 private:
	bool create_xlib_surface;

	void CreateSurface ();
	void CleanupSurface ();
	void CleanupPattern ();
	void StopLoader ();

	// downloader callbacks
	void PixbufWrite (guchar *bug, gsize offset, gsize count);
	void DownloaderEvent (int kind);
	void UpdateProgress ();
	static void pixbuf_write (guchar *buf, gsize offset, gsize count, gpointer data);
	static void downloader_event (int kind, gpointer data);
	static void size_notify (int64_t size, gpointer data);


	// pixbuf callbacks
	void LoaderSizePrepared (int width, int height);
	void LoaderAreaPrepared ();
	void LoaderAreaUpdated (int x, int y, int width, int height);
	static void loader_size_prepared (GdkPixbufLoader *loader, int width, int height, gpointer data);
	static void loader_area_prepared (GdkPixbufLoader *loader, gpointer data);
	static void loader_area_updated (GdkPixbufLoader *loader, int x, int y, int width, int height, gpointer data);
	
	GdkPixbufLoader *loader;
	Downloader *downloader;
	GdkPixbuf *pixbuf;
	cairo_surface_t *surface;
	int pixbuf_width;
	int pixbuf_height;

	// pattern caching
	cairo_pattern_t *pattern;
	double pattern_opacity;
};

Image *image_new (void);
void   image_set_download_progress (Image *img, double progress);
double image_get_download_progress (Image *img);
void   image_set_source (Image *img, DependencyObject *Downloader, char *PartName);



class MediaElement : public MediaBase {
public:
	static DependencyProperty *AttributesProperty;
	static DependencyProperty *AutoPlayProperty;
	static DependencyProperty *BalanceProperty;
	static DependencyProperty *BufferingProgressProperty;
	static DependencyProperty *BufferingTimeProperty;
	static DependencyProperty *CanSeekProperty;
	static DependencyProperty *CurrentStateProperty;
	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *IsMutedProperty;
	static DependencyProperty *MarkersProperty;
	static DependencyProperty *NaturalDurationProperty;
	static DependencyProperty *NaturalVideoHeightProperty;
	static DependencyProperty *NaturalVideoWidthProperty;
	static DependencyProperty *PositionProperty;
	static DependencyProperty *VolumeProperty;
	
	// technically private
	MediaPlayer *mplayer;
	guint timeout_id;
	
	MediaElement ();
	~MediaElement ();
	virtual Type::Kind GetObjectType () { return Type::MEDIAELEMENT; };
	
	// overrides
	virtual void render (cairo_t *cr, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual Point getxformorigin ();
	
	virtual void OnPropertyChanged (DependencyProperty *prop);
	
	void SetSource (DependencyObject *Downloader, char *PartName);
	
	void Pause ();
	void Play ();
	void Stop ();
};

MediaElement *media_element_new (void);

void media_element_pause (MediaElement *media);
void media_element_play (MediaElement *media);
void media_element_stop (MediaElement *media);
void media_element_setsource (MediaElement *media, DependencyObject *Downloader, char *PartName);

bool media_element_get_auto_play (MediaElement *media);
void media_element_set_auto_play (MediaElement *media, bool value);

double media_element_get_balance (MediaElement *media);
void media_element_set_balance (MediaElement *media, double value);

double media_element_get_buffering_progress (MediaElement *media);
void media_element_set_buffering_progress (MediaElement *media, double value);

TimeSpan media_element_get_buffering_time (MediaElement *media);
void media_element_set_buffering_time (MediaElement *media, TimeSpan value);

bool media_element_get_can_seek (MediaElement *media);
void media_element_set_can_seek (MediaElement *media, bool value);

char *media_element_get_current_state (MediaElement *media);
void media_element_set_current_state (MediaElement *media, char *value);

double media_element_get_download_progress (MediaElement *media);
void media_element_set_download_progress (MediaElement *media, double value);

bool media_element_get_is_muted (MediaElement *media);
void media_element_set_is_muted (MediaElement *media, bool value);

TimelineMarkerCollection *media_element_get_markers (MediaElement *media);
void media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value);

Duration *media_element_get_natural_duration (MediaElement *media);
void media_element_set_natural_duration (MediaElement *media, Duration value);

double media_element_get_natural_video_height (MediaElement *media);
void media_element_set_natural_video_height (MediaElement *media, double value);

double media_element_get_natural_video_width (MediaElement *media);
void media_element_set_natural_video_width (MediaElement *media, double value);

TimeSpan media_element_get_position (MediaElement *media);
void media_element_set_position (MediaElement *media, TimeSpan value);

double media_element_get_volume (MediaElement *media);
void media_element_set_volume (MediaElement *media, double value);

void media_init (void);

G_END_DECLS

#endif /* __MEDIA_H__ */
