/*
 * media.h:
 *
 * Authors:
 *   Jeffrey Stedfast <fejj@novell.com>
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MEDIA_H__
#define __MEDIA_H__

G_BEGIN_DECLS

#include <string.h>

#include <gdk/gdkpixbuf.h>

#include "mplayer.h"
#include "enums.h"
#include "clock.h"
#include "value.h"
#include "brush.h"
#include "frameworkelement.h"
#include "error.h"

class MediaAttribute : public DependencyObject {
 protected:
	virtual ~MediaAttribute () {}

 public:
	static DependencyProperty *ValueProperty;
	
	MediaAttribute () { }
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE; };
};

MediaAttribute *media_attribute_new (void);
const char *media_attribute_get_value (MediaAttribute *attribute);
void media_attribute_set_value (MediaAttribute *attribute, const char *value);


class MediaBase : public FrameworkElement {
 protected:
	virtual ~MediaBase () {}

public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	static DependencyProperty *DownloadProgressProperty;
	
	static int DownloadProgressChangedEvent;
	
	MediaBase ();
	virtual Type::Kind GetObjectType () { return Type::MEDIABASE; };
};

MediaBase *media_base_new (void);
char *media_base_get_source (MediaBase *media);
void media_base_set_source (MediaBase *media, const char *value);

Stretch media_base_get_stretch (MediaBase *media);
void    media_base_set_stretch (MediaBase *media, Stretch value);

void   media_base_set_download_progress (MediaBase *media, double progress);
double media_base_get_download_progress (MediaBase *media);


class Image : public MediaBase {
private:
	bool create_xlib_surface;
	
	bool CreateSurface (const char *fname);
	void CleanupSurface ();
	void CleanupPattern ();
	void DownloaderAbort ();
	
	// downloader callbacks
	void PixbufWrite (void *buf, int32_t offset, int32_t n);
	void DownloaderComplete ();
	void DownloaderFailed (ErrorEventArgs *args);
	void UpdateProgress ();
	
	static void pixbuf_write (void *buf, int32_t offset, int32_t n, gpointer data);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void size_notify (int64_t size, gpointer data);
	
	Downloader *downloader;
	
	char *part_name;
	
	// pattern caching
	cairo_pattern_t *pattern;

protected:
	virtual ~Image ();

public:
	Image ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGE; };
	
	virtual void Render (cairo_t *cr, Region *region);
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	
	cairo_surface_t *GetCairoSurface ();
	
	void SetSource (DependencyObject *Downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	int GetHeight () { return surface ? surface->height : 0; };
	int GetWidth  () { return surface ? surface->width : 0; };

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	ImageBrush *brush;

	struct CachedSurface {
		int ref_cnt;
		
		char *fname;
		cairo_surface_t *cairo;
		bool xlib_surface_created;
		GdkPixbuf *backing_pixbuf;
		guchar *backing_data;
		
		bool has_alpha;
		int width;
		int height;
	};
	

	CachedSurface *surface;
	
	static GHashTable *surface_cache;
	
	static int ImageFailedEvent;
};

Image *image_new (void);
void   image_set_source (Image *img, DependencyObject *Downloader, const char *PartName);


class MediaSource {
protected:
	MediaElement *element;
	char *source_name;
	IMediaSource *source;
	
	MediaSource (MediaElement *element, const char *source_name, IMediaSource *source);
	
	MediaPlayer *GetMediaPlayer ();
	
	virtual bool OpenInternal () = 0;
	
public:
	virtual ~MediaSource ();
	
	const char *GetSourceName ();
	IMediaSource *GetSource ();
	
	virtual bool Open ();
	virtual void Play () = 0;
	virtual void Pause () = 0;
	virtual void Stop (bool media_ended) = 0;
	virtual void Close () = 0;
	
	static MediaSource *CreateSource (MediaElement *element, const char *source_name, IMediaSource *source);
};


class SingleMedia : public MediaSource {
	guint advance_frame_timeout_id;
	
	void ClearTimeout ();
	
protected:
	virtual bool OpenInternal ();
	
public:
	SingleMedia (MediaElement *element, const char *source_name, IMediaSource *source);
	virtual ~SingleMedia ();
	
	virtual void Play ();
	virtual void Pause ();
	virtual void Stop (bool media_ended);
	virtual void Close ();
};


class MediaElement : public MediaBase {
 protected:
	virtual ~MediaElement ();

public:
	enum MediaElementState {
		Closed,
		Opening,
		Buffering,
		Playing,
		Paused,
		Stopped,
		Error
	};

private:
	
	// The current state of the media element.
	MediaElementState state;
	
	// this is used to know what to do after a Buffering state finishes
	MediaElementState prev_state;
	
	uint32_t flags;
	
	
	bool recalculate_matrix;
	cairo_matrix_t matrix;
	
	TimelineMarkerCollection *streamed_markers;
	
	// When checking if a marker has been reached, we need to 
	// know the last time the check was made, to see if 
	// the marker's pts hit the region.
	uint64_t previous_position;
	
	// downloader methods/data
	ProgressiveSource *downloaded_file;
	Downloader *downloader;
	char *part_name;
	uint64_t buffering_start; // The last write position in the file when we started buffering
	uint64_t buffering_pts; // The pts we're waiting for
	Media *media;
	
	int advance_frame_timeout_id;
	
	void DataWrite (void *data, int32_t offset, int32_t n);
	void DownloaderAbort ();
	void DownloaderComplete ();
	void BufferingComplete ();
	void UpdateProgress ();
	
	void Reinitialize ();
	virtual void OnLoaded ();
	
	// Try to open the media (i.e. read the headers).
	void TryOpen ();
	
	// Fill in all information from the opened media and raise MediaOpenedEvent. Does not change any state.
	void MediaOpened (Media *media);
	
	// Reset all information to defaults, set state to 'Error' and raise MediaFailedEvent
	void MediaFailed ();
	
	static void data_write (void *data, int32_t offset, int32_t n, void *closure);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void size_notify (int64_t size, gpointer data);
	
	void ReadMarkers ();
	void CheckMarkers (uint64_t from, uint64_t to);
	void CheckMarkers (uint64_t from, uint64_t to, TimelineMarkerCollection *col, bool remove);
	TimeSpan UpdatePlayerPosition (Value *value);
	
public:
	// properties
	static DependencyProperty *AttributesProperty;
	static DependencyProperty *AudioStreamCountProperty;
	static DependencyProperty *AudioStreamIndexProperty;
	static DependencyProperty *AutoPlayProperty;
	static DependencyProperty *BalanceProperty;
	static DependencyProperty *BufferingProgressProperty;
	static DependencyProperty *BufferingTimeProperty;
	static DependencyProperty *CanPauseProperty;
	static DependencyProperty *CanSeekProperty;
	static DependencyProperty *CurrentStateProperty;
	static DependencyProperty *IsMutedProperty;
	static DependencyProperty *MarkersProperty;
	static DependencyProperty *NaturalDurationProperty;
	static DependencyProperty *NaturalVideoHeightProperty;
	static DependencyProperty *NaturalVideoWidthProperty;
	static DependencyProperty *PositionProperty;
	static DependencyProperty *VolumeProperty;
	
	// events
	static int BufferingProgressChangedEvent;
	static int CurrentStateChangedEvent;
	static int MarkerReachedEvent;
	static int MediaEndedEvent;
	static int MediaFailedEvent;
	static int MediaOpenedEvent;
	
	MediaElement ();
	virtual Type::Kind GetObjectType () { return Type::MEDIAELEMENT; };

	virtual void SetSurface (Surface *surface);
	
	bool AdvanceFrame ();
	MediaPlayer *mplayer;
	
	// overrides
	virtual void Render (cairo_t *cr, Region *region);
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	
	virtual Value *GetValue (DependencyProperty *prop);
	virtual void SetValue (DependencyProperty *prop, Value value);
	virtual void SetValue (DependencyProperty *prop, Value *value);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	void SetSource (DependencyObject *Downloader, const char *PartName);
	
	void Pause ();
	void Play ();
	void Stop ();
	
	// State methods
	bool IsClosed () { return state == Closed; }
	bool IsOpening () { return state == Opening; }
	bool IsBuffering () { return state == Buffering; }
	bool IsPlaying () { return state == Playing; }
	bool IsPaused () { return state == Paused; }
	bool IsStopped () { return state == Stopped; }
	
	pthread_mutex_t open_mutex; // Used when accessing closure.
	MediaClosure *closure;
	static gboolean TryOpenFinished (void *user_data);
	
	static const char *GetStateName (MediaElementState state);
	
	MediaElementState GetState () { return state; }
	void SetState (MediaElementState state);
	
	virtual bool EnableAntiAlias ()
	{
		return !(absolute_xform.xx == absolute_xform.yy /* no rotation */
			 && (absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew */);
	}
	
	void AddStreamedMarker (TimelineMarker *marker);
};

MediaElement *media_element_new (void);

void media_element_pause (MediaElement *media);
void media_element_play (MediaElement *media);
void media_element_stop (MediaElement *media);
void media_element_set_source (MediaElement *media, DependencyObject *Downloader, const char *PartName);

gboolean media_element_advance_frame (gpointer data);

MediaAttributeCollection *media_element_get_attributes (MediaElement *media);
void media_element_set_attributes (MediaElement *media, MediaAttributeCollection *value);

int media_element_get_audio_stream_count (MediaElement *media);
void media_element_set_audio_stream_count (MediaElement *media, int value);

int media_element_get_audio_stream_index (MediaElement *media);
void media_element_set_audio_stream_index (MediaElement *media, int value);

bool media_element_get_auto_play (MediaElement *media);
void media_element_set_auto_play (MediaElement *media, bool value);

double media_element_get_balance (MediaElement *media);
void media_element_set_balance (MediaElement *media, double value);

double media_element_get_buffering_progress (MediaElement *media);
void media_element_set_buffering_progress (MediaElement *media, double value);

TimeSpan media_element_get_buffering_time (MediaElement *media);
void media_element_set_buffering_time (MediaElement *media, TimeSpan value);

bool media_element_get_can_pause (MediaElement *media);
void media_element_set_can_pause (MediaElement *media, bool value);

bool media_element_get_can_seek (MediaElement *media);
void media_element_set_can_seek (MediaElement *media, bool value);

char *media_element_get_current_state (MediaElement *media);
void media_element_set_current_state (MediaElement *media, const char *value);

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
