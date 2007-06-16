#ifndef __MEDIA_H__
#define __MEDIA_H__

G_BEGIN_DECLS

#include <string.h>

#include "clock.h"
#include "value.h"
#include "brush.h"

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
	virtual ~Image ();

	virtual Value::Kind GetObjectType () { return Value::IMAGE; };
	
	virtual void render (Surface *surface, int x, int y, int width, int height);
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
	void CreateSurface ();
	void CleanupSurface ();
	void StopLoader ();

	// downloader callbacks
	void PixbufWrite (guchar *bug, gsize offset, gsize count);
	void DownloaderEvent (int kind);
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
};

Image *image_new (void);
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

MediaElement *media_element_new (void);

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
