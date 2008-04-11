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
	struct {
		Downloader *downloader;
		char *part_name;
	} source;
	
	Downloader *downloader;
	char *part_name;
	
	virtual ~MediaBase ();
	
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void DownloaderAbort ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	
 public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	static DependencyProperty *DownloadProgressProperty;
	
	static int DownloadProgressChangedEvent;
	
	MediaBase ();
	virtual Type::Kind GetObjectType () { return Type::MEDIABASE; };
	
	
	
	void SetSourceAsyncCallback ();
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
};

MediaBase *media_base_new (void);
char *media_base_get_source (MediaBase *media);
void media_base_set_source (MediaBase *media, const char *value);

Stretch media_base_get_stretch (MediaBase *media);
void    media_base_set_stretch (MediaBase *media, Stretch value);

void   media_base_set_download_progress (MediaBase *media, double progress);
double media_base_get_download_progress (MediaBase *media);


class Image : public MediaBase {
	bool create_xlib_surface;
	
	bool CreateSurface (const char *fname);
	void CleanupSurface ();
	void CleanupPattern ();
	
	// downloader callbacks
	void PixbufWrite (void *buf, int32_t offset, int32_t n);
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void UpdateProgress ();
	
	static void pixbuf_write (void *buf, int32_t offset, int32_t n, gpointer data);
	static void size_notify (int64_t size, gpointer data);
	
	// pattern caching
	cairo_pattern_t *pattern;

 protected:
	virtual ~Image ();
	
 public:
	static GHashTable *surface_cache;
	
	static int ImageFailedEvent;
	
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
	ImageBrush *brush;
	
	Image ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGE; };
	
	virtual void Render (cairo_t *cr, Region *region);
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	
	cairo_surface_t *GetCairoSurface ();
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	int GetHeight () { return surface ? surface->height : 0; };
	int GetWidth  () { return surface ? surface->width : 0; };

	virtual bool InsideObject (cairo_t *cr, double x, double y);
};

Image *image_new (void);
void   image_set_source (Image *img, Downloader *downloader, const char *PartName);

class MediaElement : public MediaBase {
 public:
	enum MediaElementState {
		Closed,
		Opening,
		Buffering,
		Playing,
		Paused,
		Stopped,
		Error,
	};
	
 private:
	static void data_write (void *data, int32_t offset, int32_t n, void *closure);
	static void data_request_position (int64_t *pos, void *closure);
	static void size_notify (int64_t size, gpointer data);
	
	TimelineMarkerCollection *streamed_markers;
	int advance_frame_timeout_id;
	bool recalculate_matrix;
	cairo_matrix_t matrix;
	MediaPlayer *mplayer;
	Playlist *playlist;
	Media *media;
	
	// When checking if a marker has been reached, we need to 
	// know the last time the check was made, to see if 
	// the marker's pts hit the region.
	uint64_t previous_position;
	
	// Buffering can be caused by:
	//   * When the media is opened, we automatically buffer an amount equal to BufferingTime.
	//     - In this case the buffering progress is calculated as:
	//       "currently available pts" / BufferingTime, which equals
	//       ("currently available pts" - "last played pts") / ("current pts" - "last played pts" + BufferingTime)
	//
	//   * When during playback we realize that we don't have enough data.
	//     - In this case the buffering progress is calculated as:
	//       ("currently available pts" - "last played pts") / BufferingTime, which equals
	//       ("currently available pts" - "last played pts") / ("current pts" - "last played pts" + BufferingTime)
	//
	//   * When we seek, and realize that we don't have enough data.
	//     - In this case the buffering progress is calculated as:
	//       ("currently available pts" - "last played pts") / ("seeked to pts" - "last played pts" + BufferingTime)
	//
	//   So the general formula turns out to be:
	//     ("currently available pts" - last_played_pts) / (mplayer->GetPosition () - last_played_pts + BufferingTime)
	uint64_t last_played_pts;
	
	// this is used to know what to do after a Buffering state finishes
	MediaElementState prev_state;
	
	// The current state of the media element.
	MediaElementState state;
	
	uint32_t flags;
	
	// downloader methods/data
	ProgressiveSource *downloaded_file;
	
	void DataWrite (void *data, int32_t offset, int32_t n);
	void DataRequestPosition (int64_t *pos);
	virtual void DownloaderComplete ();
	void BufferingComplete ();
	void UpdateProgress ();
	
	void Reinitialize (bool dtor); // dtor is true if we're calling from the destructor.
	virtual void OnLoaded ();
	
	// Try to open the media (i.e. read the headers).
	void TryOpen ();
	
	// Checks if the media was actually a playlist, in which case false is returned.
	// Fill in all information from the opened media and raise MediaOpenedEvent. Does not change any state.
	bool MediaOpened (Media *media);
	
	void CheckMarkers (uint64_t from, uint64_t to, TimelineMarkerCollection *col, bool remove);
	void CheckMarkers (uint64_t from, uint64_t to);
	void ReadMarkers ();
	
	TimeSpan UpdatePlayerPosition (Value *value);
	
 protected:
	virtual ~MediaElement ();
	
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
	void AudioFinished (); // Called by MediaPlayer when the audio reaches its end. Only called if we have no video.
	
	MediaPlayer *GetMediaPlayer () { return mplayer;  }
	
	// overrides
	virtual void Render (cairo_t *cr, Region *region);
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	
	virtual Value *GetValue (DependencyProperty *prop);
	virtual void SetValue (DependencyProperty *prop, Value value);
	virtual void SetValue (DependencyProperty *prop, Value *value);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	void Pause ();
	void Play ();
	void Stop ();
	
	// These methods are the ones that actually call the appropiate methods on the MediaPlayer
	void PlayInternal ();
	//void PauseInternal ();
	//void StopInternal ();
	
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
	
	// Reset all information to defaults, set state to 'Error' and raise MediaFailedEvent
	void MediaFailed (ErrorEventArgs *args = NULL);
	
	static const char *GetStateName (MediaElementState state);
	
	MediaElementState GetState () { return state; }
	void SetState (MediaElementState state);
	
	Playlist *GetPlaylist () { return playlist;  }

	virtual bool EnableAntiAlias ();
	
	void AddStreamedMarker (TimelineMarker *marker);
	void SetMedia (Media *media);
};

MediaElement *media_element_new (void);

void media_element_pause (MediaElement *media);
void media_element_play (MediaElement *media);
void media_element_stop (MediaElement *media);
void media_element_set_source (MediaElement *media, Downloader *downloader, const char *PartName);

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
