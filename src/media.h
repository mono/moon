#ifndef __MEDIA_H__
#define __MEDIA_H__

G_BEGIN_DECLS

#include <string.h>

#include "clock.h"
#include "value.h"


class MediaAttribute : public DependencyObject {
 public:
	static DependencyProperty *ValueProperty;
	
	MediaAttribute () { }
	virtual Value::Kind GetObjectType () { return Value::MEDIAATTRIBUTE; };
};

MediaAttribute *media_attribute_new ();


class MediaBase : public FrameworkElement {
public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	
	MediaBase () { }
	virtual Value::Kind GetObjectType () { return Value::MEDIABASE; };
};

char *media_base_get_source (MediaBase *media);
void media_base_set_source (MediaBase *media, char *value);

Stretch media_base_get_stretch (MediaBase *media);
void    media_base_set_stretch (MediaBase *media, Stretch value);


class Image : public MediaBase {
 public:
	static DependencyProperty *DownloadProgressProperty;
	
	Image ();
	virtual Value::Kind GetObjectType () { return Value::IMAGE; };
	
	virtual void render (Surface *surface, int x, int y, int width, int height);
	virtual void getbounds ();

	void SetSource (DependencyObject *Downloader, char *PartName);

	virtual void OnPropertyChanged (DependencyProperty *prop);

 private:
	void PixbufWrite (guchar *bug, gsize offset, gsize count);
	void LoaderSizePrepared (int width, int height);
	static void pixbuf_write (guchar *buf, gsize offset, gsize count, gpointer data);
	static void loader_size_prepared (GdkPixbufLoader *loader, int width, int height, gpointer data);
	GdkPixbufLoader *loader;
	Downloader *downloader;
	cairo_surface_t *xlib_surface;
	GdkPixmap *pixmap;
	int pixbuf_width;
	int pixbuf_height;
};

Image* image_new ();
void   image_set_download_progress (Image *img, double progress);
double image_get_download_progress (Image *img);
void   image_set_source (DependencyObject *Downloader, char *PartName);



class MediaElement : public MediaBase {
public:
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
	
	MediaElement () { }
	virtual Value::Kind GetObjectType () { return Value::MEDIAELEMENT; };
	
	void SetSource (DependencyObject *Downloader, char *PartName);
	
	void Pause ();
	void Play ();
	void Stop ();
};

MediaElement *media_element_new ();

bool media_element_get_auto_play (MediaElement *media);
void media_element_set_auto_play (MediaElement *media, bool value);

double media_element_get_balance (MediaElement *media);
void media_element_set_balance (MediaElement *media, double value);

double media_element_get_buffering_progress (MediaElement *media);
void media_element_set_buffering_progress (MediaElement *media, double value);

TimeSpan media_element_get_buffering_time (MediaElement *media);
void media_element_set_buffering_time (MediaElement *media, TimeSpan value);

bool media_element_get_can_seek (MediaElement *media);

char *media_element_get_current_state (MediaElement *media);

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

G_END_DECLS

#endif /* __MEDIA_H__ */
