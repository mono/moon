/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mediaelement.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_MEDIAELEMENT_H__
#define __MOON_MEDIAELEMENT_H__

#include <glib.h>
#include <gdk/gdkpixbuf.h>

#include "value.h"
#include "frameworkelement.h"
#include "pipeline.h"
#include "downloader.h"
#include "mutex.h"
#include "enums.h"

/* @Namespace=System.Windows.Controls */
class MOON_API MediaElement : public FrameworkElement {
 friend class MediaElementPropertyValueProvider;	
 private:	
	Mutex mutex;
	
	List *streamed_markers_queue; // Thread-safe: Accesses to this field needs to use the mutex.
	TimelineMarkerCollection *streamed_markers; // Main thread only.
	ErrorEventArgs *error_args; // Thread-safe: Accesses to this field needs to use the mutex.
	MediaMarkerFoundClosure *marker_closure;
	cairo_matrix_t matrix;
	int quality_level; // higher number = better quality, starts out at 0.
	guint64 last_quality_level_change_position; // the pts of the position the last time the quality changed. Used to not change quality too often.
	
	MediaPlayer *mplayer;
	PlaylistRoot *playlist;
	
	// 
	guint32 marker_timeout;
	// When checking if a marker has been reached, we need to 
	// know the last time the check was made, to see if 
	// the marker's pts hit the region.
	guint64 previous_position;
	// When the position is changed by the client, we store the requested position
	// here and do the actual seeking async. Note that we might get several seek requests
	// before the actual seek is done, currently we just seek to the last position requested,
	// the previous requests are ignored. -1 denotes that there are no pending seeks.
	TimeSpan seek_to_position;
	// This is the last seeked to position. Used to never ever return a Position below this value.
	guint64 seeked_to_position;
	// This is the position when Pause is called. Since the actually Pause is done async, we must report
	// this value as the current Position.
	guint64 paused_position;
	
	guint64 first_pts; // the first pts, starts off at GUINT_MAX
	int buffering_mode; // if we're in [3] or not: 0 = unknown, 1 = [1], etc.
	
	// this is used to know what to do after a Buffering state finishes
	MediaState prev_state;
	
	// The current state of the media element.
	MediaState state;
	
	guint32 flags;
		
	void Reinitialize (); // not thread-safe
	
	void SetMarkerTimeout (bool start); // not thread-safe
	static gboolean MarkerTimeout (gpointer context); // not thread-safe
	
	// Media event handlers	
		
	EVENTHANDLER (MediaElement, Opening, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, OpenCompleted,  PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, Seeking, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, SeekCompleted, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, Seek,    PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, CurrentStateChanged, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, MediaError,   PlaylistRoot, ErrorEventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, MediaEnded, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, DownloadProgressChanged, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, BufferingProgressChanged, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, Play, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, Pause, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, Stop, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, BufferUnderflow, PlaylistRoot, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaElement, EntryChanged, PlaylistRoot, EventArgs); // Not thread-safe
	
	EVENTHANDLER (MediaElement, ShuttingDown, Deployment, EventArgs); // Not thread-safe
	
	// Fill in information to/from the media, mediaplayer, etc.
	// Does not change any state
	void SetProperties (Media *media);
	
	void EmitMediaEnded ();
	void EmitStateChangedAsync ();
	static void EmitStateChanged (EventObject *obj);
	static void ReportErrorOccurredCallback (EventObject *obj);
	
	void AddStreamedMarker (TimelineMarker *marker); // Thread-safe
	void AddStreamedMarker (MediaMarker *marker); // Thread-safe
	static MediaResult AddStreamedMarkerCallback (MediaClosure *closure); // Thread-safe
	void CheckMarkers (guint64 from, guint64 to, TimelineMarkerCollection *col, bool remove); // Not thread-safe
	void CheckMarkers (guint64 from, guint64 to); // Not thread-safe
	void CheckMarkers (); // Not thread-safe
	void ReadMarkers (Media *media, IMediaDemuxer *demuxer); // Not thread-safe
	
	//
	// Private Property Accessors
	//
	void SetAudioStreamCount (int count);
	
	void SetBufferingProgress (double progress);
	
	void SetCanPause (bool set);
	void SetCanSeek (bool set);

	void SetNaturalVideoHeight (int height);
	void SetNaturalVideoWidth (int width);
	
	void PlayOrStop (); // Not thread-safe. To the right thing if we can pause, if we have to autoplay, etc.
		
	void CreatePlaylist ();
	void SetPlaylist (PlaylistRoot *playlist); // Adds/removes event handlers

 protected:
	virtual ~MediaElement () {}
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaElement ();
	virtual void Dispose ();
	
	// properties
 	/* @PropertyType=MediaAttributeCollection,ManagedPropertyType=Dictionary<string\,string>,AutoCreateValue,ManagedSetterAccess=Internal,GenerateAccessors,Validator=MediaAttributeCollectionValidator */
	const static int AttributesProperty;
 	/* @PropertyType=gint32,DefaultValue=0,ReadOnly,GenerateAccessors */
	const static int AudioStreamCountProperty;
 	/* @PropertyType=gint32,Nullable,GenerateAccessors,Validator=AudioStreamIndexValidator */
	const static int AudioStreamIndexProperty;
 	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int AutoPlayProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,Validator=BalanceValidator */
	const static int BalanceProperty;
 	/* @PropertyType=double,DefaultValue=0.0,ReadOnly,GenerateAccessors */
	const static int BufferingProgressProperty;
 	/* @PropertyType=TimeSpan,GenerateAccessors,Validator=BufferingTimeValidator */
	const static int BufferingTimeProperty;
 	/* @PropertyType=bool,DefaultValue=false,ReadOnly,GenerateAccessors */
	const static int CanPauseProperty;
 	/* @PropertyType=bool,DefaultValue=false,ReadOnly,GenerateAccessors */
	const static int CanSeekProperty;
 	/* @PropertyType=double,ReadOnly,DefaultValue=0.0,GenerateAccessors */
	const static int DownloadProgressProperty;
 	/* @PropertyType=MediaState,ReadOnly,ManagedPropertyType=MediaElementState,DefaultValue=MediaStateClosed,GenerateAccessors */
	const static int CurrentStateProperty;
 	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsMutedProperty;
 	/* @PropertyType=TimelineMarkerCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int MarkersProperty;
 	/* @PropertyType=Duration,DefaultValue=Duration::FromSeconds (0),ReadOnly,GenerateAccessors */
	const static int NaturalDurationProperty;
 	/* @PropertyType=gint32,DefaultValue=0,ReadOnly,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	const static int NaturalVideoHeightProperty;
 	/* @PropertyType=gint32,DefaultValue=0,ReadOnly,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	const static int NaturalVideoWidthProperty;
 	/* @PropertyType=TimeSpan,AlwaysChange,GenerateAccessors */
	const static int PositionProperty;
	/* @PropertyType=Uri,AlwaysChange,ManagedPropertyType=Uri,Nullable,GenerateAccessors */
	const static int SourceProperty;
 	/* @PropertyType=Stretch,DefaultValue=StretchUniform,GenerateAccessors */
	const static int StretchProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors,Validator=VolumeValidator */
	const static int VolumeProperty;

 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,ReadOnly */
	const static int DownloadProgressOffsetProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,ReadOnly */
	const static int DroppedFramesPerSecondProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,ReadOnly */
	const static int RenderedFramesPerSecondProperty;
	
	// events
	/* @DelegateType=RoutedEventHandler */
	const static int BufferingProgressChangedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int CurrentStateChangedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int DownloadProgressChangedEvent;
	/* @DelegateType=TimelineMarkerRoutedEventHandler */
	const static int MarkerReachedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int MediaEndedEvent;
	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int MediaFailedEvent;
	// MediaOpened is raised when media is ready to play (we've already started playing, or, if AutoPlay is false, paused).
	/* @DelegateType=RoutedEventHandler */
	const static int MediaOpenedEvent;
	/* @GenerateManagedEvent=false */
	const static int MediaInvalidatedEvent;
	/* @DelegateType=LogReadyRoutedEventHandler */
	const static int LogReadyEvent;
	
	virtual void SetSurface (Surface *surface);
	
	MediaPlayer *GetMediaPlayer () { return mplayer; }
	
	// overrides
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual Point GetTransformOrigin ();

	virtual Rect GetCoverageBounds ();
	virtual Size ComputeActualSize ();
	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	void MediaInvalidate ();

	void SetSource (Downloader *downloader, const char *PartName);
	void SetUriSource (Uri *uri); // This is called from OnPropertyChanged
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	void SetStreamSource (ManagedStreamCallbacks *stream);
	/* @GenerateCBinding,GeneratePInvoke */
	IMediaDemuxer *SetDemuxerSource (void *context, CloseDemuxerCallback close_demuxer, GetDiagnosticAsyncCallback get_diagnostic, GetFrameAsyncCallback get_sample, OpenDemuxerAsyncCallback open_demuxer, SeekAsyncCallback seek, SwitchMediaStreamAsyncCallback switch_media_stream);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Pause (); // Not thread-safe
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Play (); // Not thread-safe
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Stop (); // Not thread-safe

	void Seek (TimeSpan to, bool force); // Not thread-safe. 
	
	void ReportErrorOccurred (ErrorEventArgs *args); // Thread safe
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportErrorOccurred (const char *args); // Thread safe
	
	// State methods
	bool IsClosed () { return state == MediaStateClosed; }
	bool IsOpening () { return state == MediaStateOpening; }
	bool IsBuffering () { return state == MediaStateBuffering; }
	bool IsPlaying () { return state == MediaStatePlaying; }
	bool IsPaused () { return state == MediaStatePaused; }
	bool IsStopped () { return state == MediaStateStopped; }
	
	bool IsMissingCodecs (); // Not thread-safe
	
	void SetPlayRequested (); // Not thread-safe
	
	static const char *GetStateName (MediaState state); // Thread-safe
	
	MediaState GetState () { return state; } // Thread-safe
	void SetState (MediaState state); // Thread-safe
	
	virtual bool EnableAntiAlias ();
	int GetQualityLevel (int min, int max); /* returns a quality level between min and max */
	
	//
	// Public Property Accessors
	//
	void SetAttributes (MediaAttributeCollection *attrs);
	MediaAttributeCollection *GetAttributes ();
	
	int GetAudioStreamCount ();
	
	void SetAudioStreamIndex (gint32 index);
	void SetAudioStreamIndex (gint32* index);
	gint32* GetAudioStreamIndex ();
	
	void SetAutoPlay (bool set);
	bool GetAutoPlay ();
	
	void SetBalance (double balance);
	double GetBalance ();
	
	double GetBufferingProgress ();
	
	void SetBufferingTime (TimeSpan time);
	TimeSpan GetBufferingTime ();
	
	bool GetCanPause ();
	bool GetCanSeek ();
	
	void SetCurrentState (MediaState state);
	MediaState GetCurrentState ();
	
	void SetIsMuted (bool set);
	bool GetIsMuted ();
	
	void SetMarkers (TimelineMarkerCollection *markers);
	TimelineMarkerCollection *GetMarkers ();
	
	void SetNaturalDuration (Duration *duration);
	Duration *GetNaturalDuration ();

	int GetNaturalVideoHeight ();
	int GetNaturalVideoWidth ();
	
	void SetPosition (TimeSpan position);
	TimeSpan GetPosition ();
	
	void SetVolume (double volume);
	double GetVolume ();

	void SetDownloadProgressOffset (double value);
	double GetDownloadProgressOffset ();

	void SetRenderedFramesPerSecond (double value);
	double GetRenderedFramesPerSecond ();

	void SetDroppedFramesPerSecond (double value);
	double GetDroppedFramesPerSecond ();
	
	double GetDownloadProgress ();
	void SetDownloadProgress (double progress);
	
	void SetSource (Uri *uri);
	void SetSource (Uri uri);
	Uri *GetSource ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
};

/*
 * MediaElementPropertyValueProvider
 */
 
class MediaElementPropertyValueProvider : public FrameworkElementProvider {
 private:
 	Value *position;
 	Value *current_state;
	Value *rendered_frames_per_second;
	Value *dropped_frames_per_second;

	Value *GetPosition ();
	Value *GetCurrentState ();
	Value *GetRenderedFramesPerSecond ();
	Value *GetDroppedFramesPerSecond ();
 public:
	MediaElementPropertyValueProvider (MediaElement *obj, PropertyPrecedence precedence);
	virtual ~MediaElementPropertyValueProvider ();
	virtual Value *GetPropertyValue (DependencyProperty *property);
};

#endif /* __MEDIAELEMENT_H__ */
