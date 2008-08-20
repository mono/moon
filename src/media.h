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
#include <gdk/gdkpixbuf.h>

#include "mplayer.h"
#include "value.h"
#include "brush.h"
#include "frameworkelement.h"
#include "error.h"
#include "pipeline.h"


/* @Namespace=None */
class MediaErrorEventArgs : public ErrorEventArgs {
 protected:
	virtual ~MediaErrorEventArgs () {}

 public:
	MediaErrorEventArgs (MediaResult result, const char *msg)
		: ErrorEventArgs (MediaError, (int) result, msg)
	{
		
	}
	
	virtual Type::Kind GetObjectType () { return Type::MEDIAERROREVENTARGS; }

	MediaResult GetMediaResult () { return (MediaResult) error_code; }
};


/* @Namespace=None */
/* @ManagedDependencyProperties=None */
class MediaAttribute : public DependencyObject {
 protected:
	virtual ~MediaAttribute () {}

 public:
 	/* @PropertyType=string */
	static DependencyProperty *ValueProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttribute () { }
	
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE; }
};


/* @Namespace=None */
class MediaAttributeCollection : public DependencyObjectCollection {
 protected:
	virtual ~MediaAttributeCollection () {}

 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttributeCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::MEDIAATTRIBUTE; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttribute *GetItemByName (const char *name);
};


/*
 * This collection is always sorted by the time value of the markers.
 * We override AddToList to add the node where it's supposed to be, keeping the
 * collection sorted at all times.
 * We also override Insert to ignore the index and behave just like Add.
 */
/* @Namespace=System.Windows.Media */
class TimelineMarkerCollection : public DependencyObjectCollection {
 protected:
	virtual ~TimelineMarkerCollection () {}
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	TimelineMarkerCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::TIMELINEMARKER; }
	
	virtual int Add (Value *value);
	virtual bool Insert (int index, Value *value);
};


/* @Namespace=None */
class MarkerReachedEventArgs : public EventArgs {
	TimelineMarker *marker;
	
 protected:
	virtual ~MarkerReachedEventArgs ();
	
 public:
	MarkerReachedEventArgs (TimelineMarker *marker);
	virtual Type::Kind GetObjectType () { return Type::MARKERREACHEDEVENTARGS; }

	TimelineMarker *GetMarker () { return marker; }
};


/* @Namespace=None */
/* @ManagedDependencyProperties=None */
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
 	/* @PropertyType=string,AlwaysChange */
	static DependencyProperty *SourceProperty;
 	/* @PropertyType=gint32,DefaultValue=StretchUniform */
	static DependencyProperty *StretchProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *DownloadProgressProperty;
	
	const static int DownloadProgressChangedEvent;
	
	/* @GenerateCBinding,GeneratePInvoke */
	MediaBase ();
	virtual Type::Kind GetObjectType () { return Type::MEDIABASE; }
	
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


/* @Namespace=System.Windows.Controls */
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
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Image ();
	
	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void SetStreamSource (ManagedStreamCallbacks *stream);
	
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


/* @Namespace=System.Windows.Controls */
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
	guint64 first_pts; // the first pts, starts off at GUINT_MAX
	
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
 	/* @PropertyType=MediaAttributeCollection,ManagedPropertyType=Dictionary<string\,string>,ManagedSetterAccess=Internal */
	static DependencyProperty *AttributesProperty;
 	/* @PropertyType=gint32,DefaultValue=0,ReadOnly */
	static DependencyProperty *AudioStreamCountProperty;
 	/* @PropertyType=gint32,Nullable */
	static DependencyProperty *AudioStreamIndexProperty;
 	/* @PropertyType=bool,DefaultValue=true */
	static DependencyProperty *AutoPlayProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *BalanceProperty;
 	/* @PropertyType=double,DefaultValue=0.0,ReadOnly */
	static DependencyProperty *BufferingProgressProperty;
 	/* @PropertyType=TimeSpan,DefaultValue="TimeSpan_FromSeconds (5)\,Type::TIMESPAN" */
	static DependencyProperty *BufferingTimeProperty;
 	/* @PropertyType=bool,DefaultValue=false,ReadOnly */
	static DependencyProperty *CanPauseProperty;
 	/* @PropertyType=bool,DefaultValue=false,ReadOnly */
	static DependencyProperty *CanSeekProperty;
 	/* @PropertyType=string,ReadOnly,ManagedPropertyType=MediaElementState */
	static DependencyProperty *CurrentStateProperty;
 	/* @PropertyType=bool,DefaultValue=false */
	static DependencyProperty *IsMutedProperty;
 	/* @PropertyType=TimelineMarkerCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal */
	static DependencyProperty *MarkersProperty;
 	/* @PropertyType=Duration,DefaultValue=Duration::FromSeconds (0),ReadOnly */
	static DependencyProperty *NaturalDurationProperty;
 	/* @PropertyType=double,DefaultValue=0.0,ReadOnly,ManagedPropertyType=int */
	static DependencyProperty *NaturalVideoHeightProperty;
 	/* @PropertyType=double,DefaultValue=0.0,ReadOnly,ManagedPropertyType=int */
	static DependencyProperty *NaturalVideoWidthProperty;
 	/* @PropertyType=TimeSpan */
	static DependencyProperty *PositionProperty;
 	/* @PropertyType=double,DefaultValue=0.5 */
	static DependencyProperty *VolumeProperty;

	
	// events
	const static int BufferingProgressChangedEvent;
	const static int CurrentStateChangedEvent;
	const static int MarkerReachedEvent;
	const static int MediaEndedEvent;
	const static int MediaFailedEvent;
	// MediaOpened is raised when media is ready to play (we've already started playing, or, if AutoPlay is false, paused).
	const static int MediaOpenedEvent;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaElement ();
	virtual Type::Kind GetObjectType () { return Type::MEDIAELEMENT; }
	
	virtual void SetSurface (Surface *surface);
	
	bool AdvanceFrame ();
	void AudioFinished (); // Called by MediaPlayer when the audio reaches its end. Only called if we have no video.
	
	MediaPlayer *GetMediaPlayer () { return mplayer; }
	
	// overrides
	virtual void Render (cairo_t *cr, Region *region);
	virtual Point GetTransformOrigin ();
	
	virtual Value *GetValue (DependencyProperty *prop);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void SetStreamSource (ManagedStreamCallbacks *stream);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Pause ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Play ();
	
	/* @GenerateCBinding,GeneratePInvoke */
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

#endif /* __MEDIA_H__ */
