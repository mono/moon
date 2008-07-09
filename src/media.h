/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * media.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <glib.h>

G_BEGIN_DECLS

#include <gdk/gdkpixbuf.h>

#include "mplayer.h"
#include "enums.h"
#include "clock.h"
#include "value.h"
#include "brush.h"
#include "frameworkelement.h"
#include "error.h"


class MediaErrorEventArgs : public ErrorEventArgs {
 protected:
	virtual ~MediaErrorEventArgs () {}

 public:
	MediaErrorEventArgs (MediaResult result, const char *msg)
		: ErrorEventArgs (MediaError, (int) result, msg)
	{
		
	}
	
	virtual Type::Kind GetObjectType () { return Type::MEDIAERROREVENTARGS; };

	MediaResult GetMediaResult () { return (MediaResult) error_code; }
};

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


class MediaAttributeCollection : public Collection {
 protected:
	virtual ~MediaAttributeCollection () {}

 public:
	MediaAttributeCollection () {}
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::MEDIAATTRIBUTE; }

	MediaAttribute *GetItemByName (const char *name);
};

MediaAttributeCollection *media_attribute_collection_new (void);
MediaAttribute *media_attribute_collection_get_item_by_name (MediaAttributeCollection *collection, const char *name);


/*
 * This collection is always sorted by the time value of the markers.
 * We override AddToList to add the node where it's supposed to be, keeping the
 * collection sorted at all times.
 * We also override Insert to ignore the index and behave just like Add.
 */
class TimelineMarkerCollection : public Collection {
 protected:
	virtual ~TimelineMarkerCollection () {}
	virtual int AddToList (Collection::Node *node);

 public:
	TimelineMarkerCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::TIMELINEMARKER; }
	
	virtual bool Insert (int index, DependencyObject *data);
};

TimelineMarkerCollection *timeline_marker_collection_new (void);

class MarkerReachedEventArgs : public EventArgs {
	TimelineMarker *marker;
	
 protected:
	virtual ~MarkerReachedEventArgs ();
	
 public:
	MarkerReachedEventArgs (TimelineMarker *marker);
	virtual Type::Kind GetObjectType () { return Type::MARKERREACHEDEVENTARGS; };

	TimelineMarker *GetMarker () { return marker; }
};


class MediaBase : public FrameworkElement {
 protected:
	struct {
		Downloader *downloader;
		char *part_name;
	} source;
	
	Downloader *downloader;
	char *part_name;
	
	int updating_size_from_media:1;
	int use_media_height:1;
	int use_media_width:1;
	int source_changed:1;
	
	virtual ~MediaBase ();
	
	virtual void ComputeBounds ();
	
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void DownloaderAbort ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	virtual void OnEmptySource () { }
	
	void SetDownloadProgress (double progress);
	
 public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	static DependencyProperty *DownloadProgressProperty;
	
	const static int DownloadProgressChangedEvent;
	
	MediaBase ();
	virtual Type::Kind GetObjectType () { return Type::MEDIABASE; };
	
	void SetSourceAsyncCallback ();
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void SetSurface (Surface *surface);
	
	//
	// Property Accessors
	//
	double GetDownloadProgress ();
	
	const char *GetSource ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
};

MediaBase *media_base_new (void);

const char *media_base_get_source (MediaBase *media);
void media_base_set_source (MediaBase *media, const char *value);

Stretch media_base_get_stretch (MediaBase *media);
void    media_base_set_stretch (MediaBase *media, Stretch stretch);

double media_base_get_download_progress (MediaBase *media);


class Image : public MediaBase {
	int create_xlib_surface:1;
	
	bool CreateSurface (const char *filename);
	void CleanupSurface ();
	void CleanupPattern ();
	
	// downloader callbacks
	void PixbufWrite (void *buf, gint32 offset, gint32 n);
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void UpdateProgress ();
	
	static void pixbuf_write (void *buf, gint32 offset, gint32 n, gpointer data);
	static void size_notify (gint64 size, gpointer data);
	
	// pattern caching
	cairo_pattern_t *pattern;

	// pixbuf loading
	 GdkPixbufLoader *loader;
	 GError *loader_err;
	
 protected:
	virtual ~Image ();
	
	virtual void OnEmptySource () { CleanupSurface (); }
	
 public:
	static GHashTable *surface_cache;
	
	const static int ImageFailedEvent;
	
	struct CachedSurface {
		int xlib_surface_created:1;
		int ref_count:30;
		int has_alpha:1;
		
		GdkPixbuf *backing_pixbuf;
		cairo_surface_t *cairo;
		guchar *backing_data;
		char *filename;
		
		int height;
		int width;
	};
	
	CachedSurface *surface;
	ImageBrush *brush;
	
	Image ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGE; };
	
	virtual void Render (cairo_t *cr, Region *region);
	virtual Point GetTransformOrigin ();
	
	cairo_surface_t *GetCairoSurface ();
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	int GetImageHeight () { return surface ? surface->height : 0; };
	int GetImageWidth  () { return surface ? surface->width : 0; };
	
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
	static void data_write (void *data, gint32 offset, gint32 n, void *closure);
	static void data_request_position (gint64 *pos, void *closure);
	static void size_notify (gint64 size, gpointer data);
	
	TimelineMarkerCollection *streamed_markers;
	Queue *pending_streamed_markers;
	MediaClosure *marker_closure;
	int advance_frame_timeout_id;
	bool recalculate_matrix;
	cairo_matrix_t matrix;
	MediaPlayer *mplayer;
	Playlist *playlist;
	Media *media;
	
	// When checking if a marker has been reached, we need to 
	// know the last time the check was made, to see if 
	// the marker's pts hit the region.
	guint64 previous_position;
	// When the position is changed by the client, we store the requested position
	// here and do the actual seeking async. Note that we might get several seek requests
	// before the actual seek is done, currently we just seek to the last position requested,
	// the previous requests are ignored. -1 denotes that there are no pending seeks.
	TimeSpan seek_to_position;
	
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
	guint64 last_played_pts;
	
	// this is used to know what to do after a Buffering state finishes
	MediaElementState prev_state;
	
	// The current state of the media element.
	MediaElementState state;
	
	guint32 flags;
	
	// downloader methods/data
	IMediaSource *downloaded_file;
	
	void DataWrite (void *data, gint32 offset, gint32 n);
	void DataRequestPosition (gint64 *pos);
	virtual void DownloaderComplete ();
	virtual void DownloaderFailed (EventArgs *args);
	void BufferingComplete ();
	double GetBufferedSize ();
	void UpdateProgress ();
	
	virtual void OnLoaded ();
	
	// Try to open the media (i.e. read the headers).
	void TryOpen ();
	
	// Checks if the media was actually a playlist, in which case false is returned.
	// Fill in all information from the opened media and raise MediaOpenedEvent. Does not change any state.
	bool MediaOpened (Media *media);
	void EmitMediaEnded ();
	
	void CheckMarkers (guint64 from, guint64 to, TimelineMarkerCollection *col, bool remove);
	void CheckMarkers (guint64 from, guint64 to);
	void ReadMarkers ();
	
	TimeSpan UpdatePlayerPosition (TimeSpan position);
	
	//
	// Private Property Accessors
	//
	void SetAudioStreamCount (int count);
	
	void SetBufferingProgress (double progress);
	
	void SetCanPause (bool set);
	void SetCanSeek (bool set);
	
	void SetNaturalDuration (TimeSpan duration);
	void SetNaturalVideoHeight (double height);
	void SetNaturalVideoWidth (double width);
	
	void PlayOrPauseNow ();
	void PauseNow ();
	void PlayNow ();
	void StopNow ();
	void SeekNow ();
	static void PauseNow (EventObject *value);
	static void PlayNow (EventObject *value);
	static void StopNow (EventObject *value);
	static void SeekNow (EventObject *value);
	
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
	const static int BufferingProgressChangedEvent;
	const static int CurrentStateChangedEvent;
	const static int MarkerReachedEvent;
	const static int MediaEndedEvent;
	const static int MediaFailedEvent;
	// MediaOpened is raised when media is ready to play (we've already started playing, or, if AutoPlay is false, paused).
	const static int MediaOpenedEvent;
	
	MediaElement ();
	virtual Type::Kind GetObjectType () { return Type::MEDIAELEMENT; };
	
	virtual void SetSurface (Surface *surface);
	void SetPreviousPosition (guint64 pos);
	
	bool AdvanceFrame ();
	void AudioFinished (); // Called by MediaPlayer when the audio reaches its end. Only called if we have no video.
	
	MediaPlayer *GetMediaPlayer () { return mplayer;  }
	
	// overrides
	virtual void Render (cairo_t *cr, Region *region);
	virtual Point GetTransformOrigin ();
	
	virtual Value *GetValue (DependencyProperty *prop);
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
	
	bool IsLive ();
	void EmitMediaOpened ();
	void Reinitialize (bool dtor); // dtor is true if we're calling from the destructor.
	
	pthread_mutex_t open_mutex; // Used when accessing closure.
	MediaClosure *closure;
	static void TryOpenFinished (EventObject *user_data);
	void SetPlayRequested ();
	
	// Reset all information to defaults, set state to 'Error' and raise MediaFailedEvent
	void MediaFailed (ErrorEventArgs *args = NULL);
	
	static const char *GetStateName (MediaElementState state);
	
	MediaElementState GetState () { return state; }
	void SetState (MediaElementState state);
	
	Playlist *GetPlaylist () { return playlist;  }
	
	virtual bool EnableAntiAlias ();
	
	void AddStreamedMarker (TimelineMarker *marker);
	static void AddStreamedMarkersCallback (EventObject *obj);
	void AddStreamedMarkers ();
	void SetMedia (Media *media);
	
	//
	// Public Property Accessors
	//
	void SetAttributes (MediaAttributeCollection *attrs);
	MediaAttributeCollection *GetAttributes ();
	
	int GetAudioStreamCount ();
	
	void SetAudioStreamIndex (int index);
	int GetAudioStreamIndex ();
	
	void SetAutoPlay (bool set);
	bool GetAutoPlay ();
	
	void SetBalance (double balance);
	double GetBalance ();
	
	double GetBufferingProgress ();
	
	void SetBufferingTime (TimeSpan time);
	TimeSpan GetBufferingTime ();
	
	bool GetCanPause ();
	bool GetCanSeek ();
	
	void SetCurrentState (const char *state);
	const char *GetCurrentState ();
	
	void SetIsMuted (bool set);
	bool GetIsMuted ();
	
	void SetMarkers (TimelineMarkerCollection *markers);
	TimelineMarkerCollection *GetMarkers ();
	
	Duration *GetNaturalDuration ();
	double GetNaturalVideoHeight ();
	double GetNaturalVideoWidth ();
	
	void SetPosition (TimeSpan position);
	TimeSpan GetPosition ();
	
	void SetVolume (double volume);
	double GetVolume ();
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

int media_element_get_audio_stream_index (MediaElement *media);
void media_element_set_audio_stream_index (MediaElement *media, int value);

bool media_element_get_auto_play (MediaElement *media);
void media_element_set_auto_play (MediaElement *media, bool value);

double media_element_get_balance (MediaElement *media);
void media_element_set_balance (MediaElement *media, double value);

double media_element_get_buffering_progress (MediaElement *media);

TimeSpan media_element_get_buffering_time (MediaElement *media);
void media_element_set_buffering_time (MediaElement *media, TimeSpan value);

bool media_element_get_can_pause (MediaElement *media);
bool media_element_get_can_seek (MediaElement *media);

const char *media_element_get_current_state (MediaElement *media);
void media_element_set_current_state (MediaElement *media, const char *value);

bool media_element_get_is_muted (MediaElement *media);
void media_element_set_is_muted (MediaElement *media, bool value);

TimelineMarkerCollection *media_element_get_markers (MediaElement *media);
void media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value);

Duration *media_element_get_natural_duration (MediaElement *media);
double media_element_get_natural_video_height (MediaElement *media);
double media_element_get_natural_video_width (MediaElement *media);

TimeSpan media_element_get_position (MediaElement *media);
void media_element_set_position (MediaElement *media, TimeSpan value);

double media_element_get_volume (MediaElement *media);
void media_element_set_volume (MediaElement *media, double value);

void media_init (void);

G_END_DECLS

#endif /* __MEDIA_H__ */
