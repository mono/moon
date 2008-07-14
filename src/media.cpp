/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * media.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "runtime.h"
#include "media.h"
#include "error.h"
#include "downloader.h"
#include "playlist.h"
#include "geometry.h"
#include "pipeline.h"

#define d(x)
#define e(x)

// still too ugly to be exposed in the header files ;-)
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform,
					 Transform *relative_transform);


/*
 * MediaBase
 */

DependencyProperty *MediaBase::SourceProperty;
DependencyProperty *MediaBase::StretchProperty;
DependencyProperty *MediaBase::DownloadProgressProperty;

MediaBase::MediaBase ()
{
	source.downloader = NULL;
	source.part_name = NULL;
	downloader = NULL;
	part_name = NULL;
	updating_size_from_media = false;
	use_media_height = true;
	use_media_width = true;
	source_changed = false;
}

MediaBase::~MediaBase ()
{
	DownloaderAbort ();
}

void
MediaBase::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MediaBase *media = (MediaBase *) closure;
	
	media->DownloaderComplete ();
}

void
MediaBase::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MediaBase *media = (MediaBase *) closure;
	
	media->DownloaderFailed (calldata);
}

void
MediaBase::DownloaderComplete ()
{
	// Nothing for MediaBase to do...
}

void
MediaBase::DownloaderFailed (EventArgs *args)
{
	// Nothing for MediaBase to do...
}

void
MediaBase::DownloaderAbort ()
{
	if (downloader) {
		downloader->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->SetWriteFunc (NULL, NULL, NULL);
		downloader->SetRequestPositionFunc (NULL);
		downloader->Abort ();
		downloader->unref ();
		g_free (part_name);
		downloader = NULL;
		part_name = NULL;
	}
}

void
MediaBase::SetSurface (Surface *surface)
{
	const char *uri;
	Downloader *dl;
	
	FrameworkElement::SetSurface (surface);
	
	if (!source_changed || !surface)
		return;
	
	source_changed = false;
	
	if ((uri = GetSource ()) && *uri) {
		if (!(dl = surface->CreateDownloader ())) {
			// we're shutting down
			return;
		}
		
		dl->Open ("GET", uri);
		SetSource (dl, "");
		dl->unref ();
	}
}

void
MediaBase::SetSourceAsyncCallback ()
{
	Downloader *downloader;
	char *part_name;
	
	if (!source.downloader)
		return;
	
	if (GetSurface () == NULL)
		return;
	
	downloader = source.downloader;
	part_name = source.part_name;
	source.downloader = NULL;
	source.part_name = NULL;
	
	SetSourceInternal (downloader, part_name);
	
	downloader->unref ();
}

void
MediaBase::SetSourceInternal (Downloader *downloader, char *PartName)
{
	this->downloader = downloader;
	part_name = PartName;
	
	if (downloader)
		downloader->ref ();
}

static void
set_source_async (EventObject *user_data)
{
	MediaBase *media = (MediaBase *) user_data;
	
	media->SetSourceAsyncCallback ();
}

void
MediaBase::SetSource (Downloader *downloader, const char *PartName)
{
	source_changed = false;
	
	DownloaderAbort ();
	
	if (source.downloader) {
		source.downloader->unref ();
		g_free (source.part_name);
		source.downloader = NULL;
		source.part_name = NULL;
	}
	
	if (!downloader) {
		SetSourceInternal (NULL, NULL);
		return;
	}
	
	source.part_name = g_strdup (PartName);
	source.downloader = downloader;
	downloader->ref ();
	
	AddTickCall (set_source_async);
}

void
MediaBase::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == MediaBase::SourceProperty) {
		const char *uri = args->new_value ? args->new_value->AsString () : NULL;
		
		if (uri && *uri) {
			Surface *surface = GetSurface ();
			Downloader *dl;
			
			if (surface) {
				if ((dl = surface->CreateDownloader ())) {
					dl->Open ("GET", uri);
					SetSource (dl, "");
					dl->unref ();
				} else {
					// we're shutting down
				}
			} else {
				source_changed = true;
			}
		} else {
			DownloaderAbort ();
			OnEmptySource ();
			Invalidate ();
		}
	}
	
	if (args->property->type != Type::MEDIABASE) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
MediaBase::SetDownloadProgress (double progress)
{
	SetValue (MediaBase::DownloadProgressProperty, Value (progress));
}

double
MediaBase::GetDownloadProgress ()
{
	return GetValue (MediaBase::DownloadProgressProperty)->AsDouble ();
}

const char *
MediaBase::GetSource ()
{
	Value *value = GetValue (MediaBase::SourceProperty);
	
	return value ? value->AsString () : NULL;
}

void
MediaBase::SetStretch (Stretch stretch)
{
	SetValue (MediaBase::StretchProperty, Value (stretch));
}

Stretch
MediaBase::GetStretch ()
{
	return (Stretch) GetValue (MediaBase::StretchProperty)->AsInt32 ();
}

void
MediaBase::ComputeBounds ()
{
	Rect box = Rect (0,0, GetWidth (), GetHeight ());
	
	bounds = IntersectBoundsWithClipPath (box, false).Transform (&absolute_xform);
}



MediaBase *
media_base_new (void)
{
	return new MediaBase ();
}

const char *
media_base_get_source (MediaBase *media)
{
	Value *value = media->GetValue (MediaBase::SourceProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
media_base_set_source (MediaBase *media, const char *source)
{
	media->SetValue (MediaBase::SourceProperty, Value (source));
}

Stretch
media_base_get_stretch (MediaBase *media)
{
	return media->GetStretch ();
}

void
media_base_set_stretch (MediaBase *media, Stretch stretch)
{
	media->SetStretch (stretch);
}

double
media_base_get_download_progress (MediaBase *media)
{
	return media->GetDownloadProgress ();
}

/*
 * MediaElement
 */

DependencyProperty *MediaElement::AttributesProperty;
DependencyProperty *MediaElement::AudioStreamCountProperty;
DependencyProperty *MediaElement::AudioStreamIndexProperty;
DependencyProperty *MediaElement::AutoPlayProperty;
DependencyProperty *MediaElement::BalanceProperty;
DependencyProperty *MediaElement::BufferingProgressProperty;
DependencyProperty *MediaElement::BufferingTimeProperty;
DependencyProperty *MediaElement::CanPauseProperty;
DependencyProperty *MediaElement::CanSeekProperty;
DependencyProperty *MediaElement::CurrentStateProperty;
DependencyProperty *MediaElement::IsMutedProperty;
DependencyProperty *MediaElement::MarkersProperty;
DependencyProperty *MediaElement::NaturalDurationProperty;
DependencyProperty *MediaElement::NaturalVideoHeightProperty;
DependencyProperty *MediaElement::NaturalVideoWidthProperty;
DependencyProperty *MediaElement::PositionProperty;
DependencyProperty *MediaElement::VolumeProperty;

enum MediaElementFlags {
	Loaded              = (1 << 0),  // set once OnLoaded has been called
	TryOpenOnLoaded     = (1 << 1),  // set if OnLoaded should call TryOpen
	PlayRequested       = (1 << 2),  // set if Play() has been requested prior to being ready
	BufferingFailed     = (1 << 3),  // set if TryOpen failed to buffer the media.
	DisableBuffering    = (1 << 4),  // set if we cannot give useful buffering progress
	DownloadComplete    = (1 << 5),  // set if the download is complete
	UpdatingPosition    = (1 << 6),  // set if we are updating the PositionProperty as opposed to someone else
	RecalculateMatrix   = (1 << 7),  // set if the patern matrix needs to be recomputed
	WaitingForOpen      = (1 << 8),  // set if we've called OpenAsync on a media and we're waiting for the result	
	MediaOpenedEmitted  = (1 << 9),  // set if MediaOpened has been emitted.
	Broadcast           = (1 << 10), // set if we have a live stream as source
};


const char *media_element_states[] = { "Closed", "Opening", "Buffering", "Playing", "Paused", "Stopped", "Error" };

const char *
MediaElement::GetStateName (MediaElementState state)
{
	int i = (int) state;
	
	if (i >= 0 && i < (int) G_N_ELEMENTS (media_element_states))
		return media_element_states[i];
	
	return NULL;
}

static MediaResult
marker_callback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaMarker *mmarker = closure->marker;
	
	if (mmarker == NULL)
		return MEDIA_FAIL;
	
	guint64 pts = mmarker->Pts ();
	
	TimelineMarker *marker = new TimelineMarker ();
	marker->SetText (mmarker->Text ());
	marker->SetType (mmarker->Type ());
	marker->SetTime (pts);
	
	element->AddStreamedMarker (marker);
	marker->unref ();
	
	return MEDIA_SUCCESS;
}

class MarkerNode : public List::Node {
 public:
	TimelineMarker *marker;
	MarkerNode (TimelineMarker *marker) { this->marker = marker; this->marker->ref (); }
	virtual ~MarkerNode () { this->marker->unref (); }
};

void
MediaElement::AddStreamedMarker (TimelineMarker *marker)
{	
	d(printf ("MediaElement::AddStreamedMarker (): got marker %s, %s, %llu = %llu ms\n",
		  marker->GetText (), marker->GetType (), marker->GetTime (),
		  MilliSeconds_FromPts (marker->GetTime ())));
	
	pending_streamed_markers->Push (new MarkerNode (marker));
	
	AddTickCallSafe (AddStreamedMarkersCallback);
}

void
MediaElement::AddStreamedMarkersCallback (EventObject *obj)
{
	((MediaElement *) obj)->AddStreamedMarkers ();
}

void
MediaElement::AddStreamedMarkers ()
{
	MarkerNode *node;
	
	d(printf ("MediaElement::AddStreamedMarkers ()\n"));
	
	if (streamed_markers == NULL)
		streamed_markers = new TimelineMarkerCollection ();

	while ((node = (MarkerNode *) pending_streamed_markers->Pop ()) != NULL) {
		streamed_markers->Add (node->marker);
		delete node;
	}
}

void
MediaElement::ReadMarkers ()
{
	IMediaDemuxer *demuxer;
	Media *media;
	
	if (mplayer == NULL || mplayer->GetMedia () == NULL || mplayer->GetMedia ()->GetDemuxer () == NULL)
		return;
	
	media = mplayer->GetMedia ();
	demuxer = media->GetDemuxer ();
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		if (demuxer->GetStream (i)->GetType () == MediaTypeMarker) {
			MarkerStream *stream = (MarkerStream *) demuxer->GetStream (i);
			
			if (marker_closure == NULL) {
				marker_closure = new MediaClosure (marker_callback);
				marker_closure->SetContextUnsafe (this);
				marker_closure->SetMedia (media);
			}
			
			stream->SetCallback (marker_closure);
			break;
		}
	}
	
	TimelineMarkerCollection *markers = NULL;
	MediaMarker::Node *current = (MediaMarker::Node *) media->GetMarkers ()->First ();
	
	if (current == NULL) {
		//printf ("MediaElement::ReadMarkers (): no markers.\n");
		return;
	}
	
	markers = new TimelineMarkerCollection ();
	while (current != NULL) {
		TimelineMarker *new_marker = new TimelineMarker ();
		MediaMarker *marker = current->marker;
		
		new_marker->SetText (marker->Text ());
		new_marker->SetType (marker->Type ());
		new_marker->SetTime (TimeSpan_FromPts (marker->Pts ()));
		markers->Add (new_marker);
		new_marker->unref ();
		
		current = (MediaMarker::Node *) current->next;
	}
	
	// Docs says we overwrite whatever's been loaded already.
	d (printf ("MediaElement::ReadMarkers (): setting %d markers.\n", markers->GetCount ()));
	SetMarkers (markers);
	markers->unref ();
}

#define LOG_MARKERS(...)// printf (__VA_ARGS__);
#define LOG_MARKERS_EX(...) //printf (__VA_ARGS__);

void
MediaElement::CheckMarkers (guint64 from, guint64 to)
{
	TimelineMarkerCollection *markers;
	
	if (from == to) {
		LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu). from == to\n", from, to);
		return;
	}
	
	if (!(markers = GetMarkers ())) {
		LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu). No markers\n", from, to);
		return;
	}
	
	if (from > to) {
		// if from > to we've seeked backwards (last played position is after this one)
		LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu). from > to (diff: %llu = %llu ms).\n", from, to, from - to, MilliSeconds_FromPts (from - to));
		return;
	}
	
	CheckMarkers (from, to, markers, false);
	CheckMarkers (from, to, streamed_markers, true);	
}

void
MediaElement::CheckMarkers (guint64 from, guint64 to, TimelineMarkerCollection *markers, bool remove)
{
	Collection::Node *node, *next;
	TimelineMarker *marker;
	Value *val = NULL;
	guint64 pts;
	bool emit;
	
	LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu, %p, %i). count: %i\n", from, to, markers, remove, markers ? markers->GetCount () : -1);
	
	if (markers == NULL)
		return;
	
	// We might want to use a more intelligent algorithm here, 
	// this code only loops through all markers on every frame.
	
	node = (Collection::Node *) markers->list->First ();
	while (node != NULL) {
		if (!(marker = (TimelineMarker *) node->obj))
			return;
		
		if (!(val = marker->GetValue (TimelineMarker::TimeProperty)))
			return;
		
		pts = (guint64) val->AsTimeSpan ();
		
		LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu\n", from, to, pts);
		
		emit = false;
		if (remove) {
			// Streamed markers. Emit these even if we passed them with up to 1 s.
			if (from <= MilliSeconds_ToPts (1000)) {
				emit = pts >= 0 && pts <= to;
			} else {
				emit = pts >= (from - MilliSeconds_ToPts (1000)) && pts <= to;
			}
			
			LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu in marker with Text = %s, Type = %s (removed from from)\n", from - MilliSeconds_ToPts (100), to, pts, marker->GetText (), marker->GetType ());
		} else {
			// Normal markers.
			emit = pts >= from && pts <= to;
			LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu in marker with Text = %s, Type = %s\n", from, to, pts, marker->GetText (), marker->GetType ());
		}
		
		if (emit) {
			LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu): Emitting: Text = %s, Type = %s, Time = %llu = %llu ms\n", from, to, marker->GetText (), marker->GetType (), marker->GetTime (), MilliSeconds_FromPts (marker->GetTime ()));
			Emit (MarkerReachedEvent, new MarkerReachedEventArgs (marker));
		}

		next = (Collection::Node *) node->next;
		
		if (remove && (pts <= to || emit)) {
			// Also delete markers we've passed by already
			markers->list->Remove (node);
		}
		
		node = next;
	}
}

void
MediaElement::AudioFinished ()
{
	d (printf ("MediaElement::AudioFinished ()\n"));
	
	SetState (Stopped);
	Emit (MediaElement::MediaEndedEvent);
}

bool
MediaElement::AdvanceFrame ()
{
	guint64 position; // pts
	bool advanced;
	
	e(printf ("MediaElement::AdvanceFrame (), IsPlaying: %i, HasVideo: %i, HasAudio: %i\n",
		  IsPlaying (), mplayer->HasVideo (), mplayer->HasAudio ()));
	
	if (!IsPlaying ())
		return false;
	
	if (!mplayer->HasVideo ())
		return false;
	
	advanced = mplayer->AdvanceFrame ();
	position = mplayer->GetPosition ();
	
	if (advanced) {
		d (printf ("MediaElement::AdvanceFrame (): advanced, setting position to: %llu = %llu ms\n", position, MilliSeconds_FromPts (position)));
		flags |= UpdatingPosition;
		SetPosition (TimeSpan_FromPts (position));
		flags &= ~UpdatingPosition;
		last_played_pts = position;
	}
	
	if (advanced || !mplayer->IsSeeking ()) {
		e (printf ("MediaElement::AdvanceFrame () previous_position: %llu = %llu ms, position: %llu = %llu ms, advanced: %i\n", 
			previous_position, MilliSeconds_FromPts (previous_position), position, MilliSeconds_FromPts (position), advanced));
			
		CheckMarkers (previous_position, position);
	}
	
	// Add 1 to avoid the same position to be able to be both
	// beginning and end of a range (otherwise the same marker
	// might raise two events).
	previous_position = position + 1;
	
	if (!advanced && mplayer->GetEof ()) {	
		mplayer->Stop ();
		SetState (Stopped);
		EmitMediaEnded ();
	}
	
	return !IsStopped ();
}

gboolean
media_element_advance_frame (void *user_data)
{
	MediaElement *media = (MediaElement *) user_data;
	
	return (gboolean) media->AdvanceFrame ();
}

MediaElement::MediaElement ()
{
	pthread_mutex_init (&open_mutex, NULL);
	
	advance_frame_timeout_id = 0;
	streamed_markers = NULL;
	marker_closure = NULL;
	downloaded_file = NULL;
	playlist = NULL;
	mplayer = NULL;
	media = NULL;
	closure = NULL;
	flags = 0;
	pending_streamed_markers = new Queue ();
	
	Reinitialize (false);
	
	mplayer = new MediaPlayer (this);
	
	SetValue (MediaElement::AttributesProperty, Value::CreateUnref (new MediaAttributeCollection ()));		
	SetValue (MediaElement::MarkersProperty, Value::CreateUnref (new TimelineMarkerCollection ()));
}

MediaElement::~MediaElement ()
{
	Reinitialize (true);
	
	if (mplayer)
		mplayer->unref ();
	
	if (playlist)
		playlist->unref ();
	
	delete pending_streamed_markers;
	
	pthread_mutex_destroy (&open_mutex);
}

void 
MediaElement::SetPreviousPosition (guint64 pos)
{
	d(printf ("MediaElement::SetPreviousPosition (%llu)\n", pos));
	
	previous_position = pos;
}

void
MediaElement::SetSurface (Surface *s)
{
	// if we previously had a surface and are losing it, we need
	// to remove our timeout (if we had one)
	if (s == NULL && GetSurface ()) {
		if (advance_frame_timeout_id != 0) {
			GetTimeManager()->RemoveTimeout (advance_frame_timeout_id);
			advance_frame_timeout_id = 0;
		}
	}
	
	mplayer->SetSurface (s);
	
	if (!SetSurfaceLock ())
		return;
	MediaBase::SetSurface (s);
	SetSurfaceUnlock ();
}

void
MediaElement::Reinitialize (bool dtor)
{
	TimelineMarkerCollection *markers;
	MediaAttributeCollection *attrs;
	IMediaDemuxer *demuxer = NULL;
	
	d(printf ("MediaElement::Reinitialize (%i)\n", dtor));
	
	if (mplayer)
		mplayer->Close (dtor);
	
	if (media != NULL) {
		demuxer = media->GetDemuxer ();
	
		if (demuxer != NULL) {								
			for (int i = 0; i < demuxer->GetStreamCount (); i++) {
				if (demuxer->GetStream (i)->GetType () != MediaTypeMarker)
					continue;
					
				((MarkerStream *) demuxer->GetStream (i))->SetCallback (NULL);
				break;
			}
		}
	}
	
	if (marker_closure) {
		delete marker_closure;
		marker_closure = NULL;
	}
	
	if (media != NULL) {
		media->unref ();
		media = NULL;
	}
	
	if (closure) {
		delete closure;
		closure = NULL;
	}
	
	if (advance_frame_timeout_id != 0) {
		GetTimeManager()->RemoveTimeout (advance_frame_timeout_id);
		advance_frame_timeout_id = 0;
	}
	
	flags = (flags & (Loaded | PlayRequested)) | RecalculateMatrix;
	if (!dtor)
		SetCurrentState ("Closed");
	
	prev_state = Closed;
	state = Closed;
	
	DownloaderAbort ();
	
	if (downloaded_file) {
		downloaded_file->unref ();
		downloaded_file = NULL;
	}
	
	if (!dtor) {
		flags |= UpdatingPosition;
		SetPosition (0);
		flags &= ~UpdatingPosition;
	}
	
	// We can't delete the playlist here,
	// because a playlist item will call SetSource
	// which will end up here, causing us to 
	// delete the playlist and then the playlist item
	// which we were about to open.
	//
	// playlist->unref ();
	
	last_played_pts = 0;
	seek_to_position = -1;
	
	if (streamed_markers) {
		streamed_markers->unref ();
		streamed_markers = NULL;
	}
	
	pending_streamed_markers->Clear (true);
	
	previous_position = 0;
	
	if ((markers = GetMarkers ()))
		markers->Clear ();
	
	if ((attrs = GetAttributes ()))
		attrs->Clear ();
	
	if (!dtor)
		SetPosition (0);
}

bool
MediaElement::IsLive ()
{
	return flags & Broadcast;
}

void
MediaElement::SetMedia (Media *media)
{
	bool broadcast = false, seekable = true;
	
	d(printf ("MediaElement::SetMedia (%p), current media: %p\n", media, this->media));
	
	if (this->media == media)
		return;	
	
	if (this->media)
		this->media->unref ();
	this->media = media;
	if (this->media)
		this->media->ref ();
	
	if (downloader != NULL && downloader->GetHttpStreamingFeatures () != 0) {
		broadcast = downloader->GetHttpStreamingFeatures () & HttpStreamingBroadcast;
		seekable = downloader->GetHttpStreamingFeatures () & HttpStreamingSeekable;
		
		d(printf ("MediaElement::SetMedia () setting features %d to broadcast (%d) and seekable (%d)\n",
			  downloader->GetHttpStreamingFeatures (), broadcast, seekable));
		
		SetCanPause (!broadcast);
		SetCanSeek (seekable);
		
		if (broadcast)
			flags |= Broadcast;
		
	}
	
	if (!mplayer->Open (media))
		return;
	
	ReadMarkers ();
	
	SetNaturalDuration (broadcast ? 0 : TimeSpan_FromPts (mplayer->GetDuration ()));
	SetNaturalVideoHeight ((double) mplayer->GetVideoHeight ());
	SetNaturalVideoWidth ((double) mplayer->GetVideoWidth ());
	SetAudioStreamCount (mplayer->GetAudioStreamCount ());
	
	mplayer->SetMuted (GetIsMuted ());
	mplayer->SetVolume (GetVolume ());
	mplayer->SetBalance (GetBalance ());
	
	
	if (playlist != NULL && playlist->GetCurrentPlaylistEntry () != NULL) {
		if (!playlist->GetCurrentPlaylistEntry ()->GetClientSkip ()) {
			SetCanSeek (false);
		}
	}
	
	mplayer->SetCanPause (GetCanPause ());
	mplayer->SetCanSeek (GetCanSeek ());
	
	UpdatePlayerPosition (GetPosition ());

	updating_size_from_media = true;

	if (use_media_width) {
		Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);

		if (!use_media_height)
			SetWidth ((double) mplayer->GetVideoWidth() * height->AsDouble () / (double) mplayer->GetVideoHeight());
		else
			SetWidth ((double) mplayer->GetVideoWidth());
	}
	
	if (use_media_height) {
		Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);

		if (!use_media_width)
			SetHeight ((double) mplayer->GetVideoHeight() * width->AsDouble () / (double) mplayer->GetVideoWidth());
		else
			SetHeight ((double) mplayer->GetVideoHeight());
	}
	
	updating_size_from_media = false;
}

bool
MediaElement::MediaOpened (Media *media)
{
	const char *demux_name = media->GetDemuxer ()->GetName ();
	
	d(printf ("MediaElement::MediaOpened (%p), demuxer name: %s, download complete: %i\n", media, demux_name, flags & DownloadComplete));
	
	if (demux_name != NULL && strcmp (demux_name, "ASXDemuxer") == 0) {
		Playlist *pl = ((ASXDemuxer *) media->GetDemuxer ())->GetPlaylist ();
		if (playlist == NULL) {
			playlist = pl;
			playlist->ref ();
			playlist->Open ();
		} else {
			if (playlist->ReplaceCurrentEntry (pl))
				pl->Open ();
		}
		return false;
	} else {
		if (playlist != NULL) {	
			playlist->GetCurrentPlaylistEntry ()->SetMedia (media);
		} else {
			playlist = new Playlist (this, media);
		}
		
		playlist->GetCurrentEntry ()->PopulateMediaAttributes ();
		SetMedia (media);
		
		if (flags & DownloadComplete) {
			SetState (Buffering);
			PlayOrPauseNow ();
			Invalidate ();
			EmitMediaOpened ();
		}
		
		return true;
	}
}

void
MediaElement::EmitMediaOpened ()
{
	d (printf ("MediaElement::EmitMediaOpened (): already emitted: %s, current state: %s\n", flags & MediaOpenedEmitted ? "true" : "false", GetStateName (state)));

	if (flags & MediaOpenedEmitted)
		return;

	flags |= MediaOpenedEmitted;
	
	Emit (MediaOpenedEvent);

}

void
MediaElement::EmitMediaEnded ()
{
	if (playlist == NULL || playlist->IsCurrentEntryLastEntry ())
		Emit (MediaEndedEvent);
		
	if (playlist)
		playlist->OnEntryEnded ();
}

void
MediaElement::MediaFailed (ErrorEventArgs *args)
{
	d(printf ("MediaElement::MediaFailed (%p)\n", args));
	
	if (state == MediaElement::Error)
		return;
	
	SetAudioStreamCount (0);
	SetNaturalVideoHeight (0);
	SetNaturalVideoWidth (0);
	SetNaturalDuration (0);
	SetCanPause (false);
	SetCanSeek (false);
	SetDownloadProgress (0);
	
	SetState (MediaElement::Error);
	
	DownloaderAbort ();
	
	Emit (MediaFailedEvent, args);
}

Point
MediaElement::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	double h = GetHeight ();
	double w = GetWidth ();
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetVideoHeight ();
		w = (double) mplayer->GetVideoWidth ();
	}
	
	return Point (user_xform_origin.x * w, user_xform_origin.y * h);
}

void
MediaElement::Render (cairo_t *cr, Region *region)
{
	Stretch stretch = GetStretch ();
	double h = GetHeight ();
	double w = GetWidth ();
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (!(surface = mplayer->GetCairoSurface ()))
		return;
	
	if (downloader == NULL)
		return;
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetVideoHeight ();
		w = (double) mplayer->GetVideoWidth ();
	}
	
	cairo_save (cr);
	
	cairo_set_matrix (cr, &absolute_xform);
	
	// if we're opaque, we can likely do this and hopefully get a
	// speed up since the server won't have to blend.
	//cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_new_path (cr);
	
	double x, y, x2, y2;
	
	x = y = 0;
	x2 = w;
	y2 = h;
	
	if (absolute_xform.xy == 0 && absolute_xform.yx == 0) {
		cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		
		cairo_user_to_device (cr, &x, &y);
		cairo_user_to_device (cr, &x2, &y2);
		
		// effectively RoundIn.  not sure if this is what we want..
		x = floor (x);
		y = floor (y);
		
		x2 = ceil (x2);
		y2 = ceil (y2);
		
		cairo_device_to_user (cr, &x, &y);
		cairo_device_to_user (cr, &x2, &y2);
	}
	
	w = x2 - x;
	h = y2 - y;
	
	if (flags & RecalculateMatrix) {
		image_brush_compute_pattern_matrix (&matrix, w, h, mplayer->GetVideoWidth (), mplayer->GetVideoHeight (),
						    stretch, AlignmentXCenter, AlignmentYCenter, NULL, NULL);
		flags &= ~RecalculateMatrix;
	}
	
	pattern = cairo_pattern_create_for_surface (surface);	
	
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	
	cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);
	
	cairo_rectangle (cr, x, y, w, h);
	
	cairo_fill (cr);
	
	cairo_restore (cr);
}

double
MediaElement::GetBufferedSize ()
{
	double progress;
	guint64 current_pts;
	guint64 buffer_pts;
	IMediaDemuxer *demuxer;
	guint64 currently_available_pts;
	
	current_pts = mplayer->GetPosition ();
	buffer_pts = TimeSpan_ToPts (GetValue (MediaElement::BufferingTimeProperty)->AsTimeSpan ());
	demuxer = media ? media->GetDemuxer () : NULL;
	currently_available_pts = demuxer ? demuxer->GetLastAvailablePts () : 0;
		
	// Check that we don't cause any div/0.
	if (currently_available_pts == 0) {
		progress = 0.0;
	} else if (currently_available_pts < last_played_pts) {
		last_played_pts = currently_available_pts;
		progress = 0.0;
	} else if (current_pts - last_played_pts + buffer_pts == currently_available_pts - last_played_pts) {
		progress = 1.0;
	} else {
		progress = (double) (currently_available_pts - last_played_pts) / (double) (current_pts - last_played_pts + buffer_pts);
	}

	e(printf ("MediaElement::GetBufferedSize (), "
			"buffer_pts: %llu = %llu ms, "
			"last_played_pts: %llu = %llu ms, "
			"current_pts: %llu = %llu ms, "
			"currently_available_pts: %llu = %llu ms, current: %.2f, progress: %.2f\n",
			buffer_pts, MilliSeconds_FromPts (buffer_pts),
			last_played_pts, MilliSeconds_FromPts (last_played_pts),
			current_pts, MilliSeconds_FromPts (current_pts),
			currently_available_pts, MilliSeconds_FromPts (currently_available_pts), 
			GetValue (MediaElement::BufferingProgressProperty)->AsDouble (), progress));
			
	if (progress < 0.0)
		progress = 0.0;
	else if (progress > 1.0)
		progress = 1.0;
		
	return progress;
}

void
MediaElement::UpdateProgress ()
{
	double progress, current;
	bool emit = false;
	
	e(printf ("MediaElement::UpdateProgress (). Current state: %s\n", GetStateName (state)));
	
	if (state & WaitingForOpen)
		return;
	
	if (downloaded_file != NULL && IsPlaying () && downloaded_file->IsWaiting ()) {
		// We're waiting for more data, switch to the 'Buffering' state.
		d(printf ("MediaElement::UpdateProgress (): Switching to 'Buffering', previous_position: "
			  "%llu = %llu ms, mplayer->GetPosition (): %llu = %llu ms, last available pts: %llu\n", 
			  previous_position, MilliSeconds_FromPts (previous_position), mplayer->GetPosition (),
			  MilliSeconds_FromPts (mplayer->GetPosition ()),
			  media ? media->GetDemuxer ()->GetLastAvailablePts () : 0));
		
		flags |= PlayRequested;
		SetBufferingProgress (0.0);
		Emit (BufferingProgressChangedEvent);
		SetState (Buffering);
		mplayer->Pause ();
		emit = true;
	}
	
	if (IsBuffering ()) {
		progress = GetBufferedSize ();
		current = GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
		
		if (current > progress) {
			// Somebody might have seeked further away after the first change to Buffering,
			// in which case the progress goes down. Don't emit any events in this case.
			emit = false;
		}
		
		// Emit the event if it's 100%, or a change of at least 0.05%
		if (emit || (progress == 1.0 && current != 1.0) || (progress - current) >= 0.0005) {
			SetBufferingProgress (progress);
			//printf ("MediaElement::UpdateProgress (): Emitting BufferingProgressChanged: progress: %.3f, current: %.3f\n", progress, current);
			Emit (BufferingProgressChangedEvent);
		}
		
		// Don't call BufferingComplete until the pipeline isn't waiting for anything anymore,
		// since otherwise we'll jump back to the Buffering state on the next call to UpdateProgress.
		if (progress == 1.0 && (downloaded_file == NULL || !downloaded_file->IsWaiting ()))
			BufferingComplete ();
	}
	
	if (downloader) {
		progress = downloader->GetDownloadProgress ();
		current = GetDownloadProgress ();
		
		// Emit the event if it's 100%, or a change of at least 0.05%
		if (progress == 1.0 || (progress - current) >= 0.0005) {
			SetDownloadProgress (progress);
			Emit (DownloadProgressChangedEvent);
		}
	}
}

void
MediaElement::SetState (MediaElementState state)
{
	const char *name;
	
	if (this->state == state)
		return;
	
	if (!(name = GetStateName (state))) {
		d(printf ("MediaElement::SetState (%d) state is not valid.\n", state));
		return;
	}
	
	d(printf ("MediaElement::SetState (%d): New state: %s, old state: %s\n",
		  state, GetStateName (state), GetStateName (this->state)));
	
	prev_state = this->state;
	this->state = state;
	
	SetCurrentState (name);
}

void 
MediaElement::DataWrite (void *buf, gint32 offset, gint32 n)
{
	//printf ("MediaElement::DataWrite (%p, %d, %d), size: %llu, source: %s\n", buf, offset, n, downloaded_file ? downloaded_file->GetSize () : 0, downloader ? downloader->GetUri () : NULL);
	
	if (downloaded_file != NULL) {
		downloaded_file->Write (buf, (gint64) offset, n);
		
 		// FIXME: How much do we actually have to download in order to try to open the file?
		if (!(flags & BufferingFailed) && IsOpening () && offset > 1024 && (part_name == NULL || part_name[0] == 0))
			TryOpen ();
	}
	
	// Delay the propogating progress 1.0 until
	// the downloader has notified us it is done.
	double progress = downloader->GetDownloadProgress ();
	
	if (progress < 1.0)
		UpdateProgress ();
}

void
MediaElement::DataRequestPosition (gint64 *position)
{
       if (downloaded_file != NULL)
               ((MemoryQueueSource*)downloaded_file)->RequestPosition (position);
}

void 
MediaElement::data_write (void *buf, gint32 offset, gint32 n, gpointer data)
{
	((MediaElement *) data)->DataWrite (buf, offset, n);
}

void 
MediaElement::data_request_position (gint64 *position, gpointer data)
{
       ((MediaElement *) data)->DataRequestPosition (position);
}

void
MediaElement::size_notify (gint64 size, gpointer data)
{
	MediaElement *element = (MediaElement *) data;
	
	if (element->downloaded_file != NULL)
		element->downloaded_file->NotifySize (size);
}

void
MediaElement::BufferingComplete ()
{
	if (state != Buffering) {
		d(printf ("MediaElement::BufferingComplete (): current state is invalid ('%s'), should only be 'Buffering'\n",
			  GetStateName (state)));
		return;
	}
	
	switch (prev_state) {
	case Opening: // Start playback
		PlayOrPauseNow ();
		return;
	case Playing: // Restart playback
		PlayNow ();
		return;
	case Paused: // Do nothing
		// TODO: Should we show the first (new) frame here?
		return;
	case Error:
	case Buffering:
	case Closed:
	case Stopped: // This should not happen.
		d(printf ("MediaElement::BufferingComplete (): previous state is invalid ('%s').\n",
			  GetStateName (prev_state)));
		return;
	}
}

MediaResult
media_element_open_callback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	
	if (element != NULL) {
		// the closure will be deleted when we return from this function,
		// so make a copy of the data.
		pthread_mutex_lock (&element->open_mutex);
		if (element->closure)
			delete element->closure;
		element->closure = closure->Clone ();
		pthread_mutex_unlock (&element->open_mutex);
		// We need to call TryOpenFinished on the main thread, so 
		element->AddTickCallSafe (MediaElement::TryOpenFinished);
	}
	
	return MEDIA_SUCCESS;
}

void
MediaElement::TryOpenFinished (EventObject *user_data)
{
	d(printf ("MediaElement::TryOpenFinished ()\n"));
	
	// No locking should be necessary here, since we can't have another open request pending.
	MediaElement *element = (MediaElement *) user_data;
	MediaClosure *closure = element->closure;
	element->closure = NULL;
	element->flags &= ~WaitingForOpen;
	
	if (!closure)
		return;
	
	if (MEDIA_SUCCEEDED (closure->result)) {
		d(printf ("MediaElement::TryOpen (): download is not complete, but media was "
			  "opened successfully and we'll now start buffering.\n"));
		element->last_played_pts = 0;
		element->SetState (Buffering);
		element->MediaOpened (closure->GetMedia ());
	} else {
		element->flags |= BufferingFailed;
		// Seek back to the beginning of the file
		element->downloaded_file->Seek (0, SEEK_SET);
		element->MediaFailed (new ErrorEventArgs (MediaError, 3001, "AG_E_INVALID_FILE_FORMAT"));
	}
	
	delete closure;
}

void
MediaElement::TryOpen ()
{
	MediaResult result; 
	
	d(printf ("MediaElement::TryOpen (), state: %s, flags: %i, Loaded: %i, WaitingForOpen: %i, DownloadComplete: %i\n", GetStateName (state), flags, flags & Loaded, flags & WaitingForOpen, flags & DownloadComplete));
	
	switch (state) {
	case Closed:
	case Error:
		d(printf ("MediaElement::TryOpen (): Current state (%s) is invalid.\n", GetStateName (state))); 
		// Should not happen
		return;
	case Playing:
	case Paused:
	case Buffering:
		// I don't think this should happen either
		d(printf ("MediaElement::TryOpen (): Current state (%s) was unexpected.\n", GetStateName (state)));
		// Media is already open.
		// There's nothing to do here.
		return;
	case Stopped:
		// This may happen if we stop a playlist (and we're not playing the first item in the list).
		// We must reload the first item, but the current state remains as stopped. 
	case Opening:
		// Try to open it now
		break;
	default:
		d(printf ("MediaElement::TryOpen (): Unknown state: %d\n", state));
		return;
	}
	
	if (!(flags & Loaded)) {
		//printf ("MediaElement::TryOpen (): We're not loaded, so wait until then.\n");
		flags |= TryOpenOnLoaded;
		return;
	}
	
	if (flags & WaitingForOpen)
		return;
	
	if (flags & DownloadComplete) {
		IMediaSource *current_downloaded_file = downloaded_file;
		char *filename = downloader->GetDownloadedFilename (part_name);
		Media *media = new Media (this, downloader);
		IMediaSource *source;
		
		if (current_downloaded_file)
			current_downloaded_file->ref ();
		
		if (filename == NULL && current_downloaded_file != NULL) {
			source = current_downloaded_file;
			source->ref ();
		} else {
			source = new FileSource (media, filename);
			g_free (filename);
		}
		
		if (!MEDIA_SUCCEEDED (result = source->Initialize ())) {
			MediaFailed ();
		} else if (!MEDIA_SUCCEEDED (result = media->Open (source))) {
			MediaFailed (new ErrorEventArgs (MediaError, 3001, "AG_E_INVALID_FILE_FORMAT"));
		} else {
			MediaOpened (media);
		}
		
		media->unref ();
		media = NULL;
	
		source->unref ();
		source = NULL;
		
		// If we have a downloaded file ourselves, delete it, we no longer need it.
		if (current_downloaded_file) {
			current_downloaded_file->unref ();
			current_downloaded_file = NULL;
		}
	} else if (part_name != NULL && part_name[0] != 0) {
		// PartName is set, we can't buffer, download the entire file.
	} else if (!(flags & BufferingFailed)) {
		flags |= WaitingForOpen;
		
		Media *media = new Media (this, downloader);
		
		MediaClosure *closure = new MediaClosure (media_element_open_callback);
		closure->SetContext (this);
		media->OpenAsync (downloaded_file, closure);
	}
	
	// FIXME: specify which audio stream index the player should use
}

void
MediaElement::DownloaderFailed (EventArgs *args)
{
	const char *protocols[] = { "mms://", "rtsp://", "rtspt://" };	
	const char *uri = downloader ? downloader->GetUri () : NULL;
	Downloader *dl;
	char *new_uri;
	size_t n;
	guint i;
	
	for (i = 0; uri && i < G_N_ELEMENTS (protocols); i++) {
		n = strlen (protocols[i]);
		if (!strncmp (uri, protocols[i], n)) {
			new_uri = g_strdup_printf ("http://%s", uri + n);
			dl = Surface::CreateDownloader (this);
			
			if (dl == NULL)
				return;
				
			dl->Open ("GET", new_uri);
			SetSource (dl, "");
			g_free (new_uri);
			dl->unref ();
			return;
		}
	}
	
	MediaFailed (new ErrorEventArgs (MediaError, 1001, "AG_E_UNKNOWN_ERROR"));
}

void
MediaElement::DownloaderComplete ()
{
	d(printf ("MediaElement::DownloaderComplete (), downloader: %d, state: %s, previous state: %s\n",
		  GET_OBJ_ID (downloader), GetStateName (state), GetStateName (prev_state)));
	
	flags |= DownloadComplete;
	
	if (GetSurface ()) {
		SetDownloadProgress (1.0);
		Emit (DownloadProgressChangedEvent);
	}
	
	if (downloaded_file != NULL)
		downloaded_file->NotifyFinished ();
	
	UpdateProgress ();
	
	switch (state) {
	case Closed:
	case Error:
		// Should not happen
		d(printf ("MediaElement::DownloaderComplete (): Current state (%d) is invalid.\n", state));
		return;
	case Playing:
	case Paused:
		// Media was opened, buffered, and then played/paused
		// There's nothing to do here
		return;
	case Stopped:
		if (!(flags & MediaOpenedEmitted)) {
			// We're a stopped playlist, and we're now reloading the first media
			TryOpen ();
		}
		return;
	case Buffering:
	 	// Media finished downloading before the buffering time was reached.
		// Play it.
		PlayOrPauseNow ();
		EmitMediaOpened ();
		break;
	case Opening:
		// The media couldn't be buffered for some reason
		// Try to open it now
		TryOpen ();
		break;
	default:
		d(printf ("MediaElement::DownloaderComplete (): Unknown state: %d\n", state));
		return;
	}
}

void
MediaElement::SetSourceInternal (Downloader *downloader, char *PartName)
{
	const char *uri = downloader ? downloader->GetUri () : NULL;
	bool is_streaming = uri ? g_str_has_prefix (uri, "mms:") : false;
	
	d(printf ("MediaElement::SetSourceInternal (%p, '%s'), uri: %s\n", downloader, PartName, uri));
	
	Reinitialize (false);
	
	SetCanPause (!is_streaming);
	SetCanSeek (!is_streaming);
	
	MediaBase::SetSourceInternal (downloader, PartName);
	
	if (downloader) {
		SetState (Opening);
		
		if (downloader->Started ()) {
			flags |= DisableBuffering;
			
			if (downloader->Completed ())
				flags |= DownloadComplete;
			
			TryOpen ();
		} else {
			if (is_streaming)
				downloaded_file = new MemoryQueueSource (mplayer->GetMedia() );
			 else 
				downloaded_file = new ProgressiveSource (mplayer->GetMedia (), false);
			
			// FIXME: error check Initialize()
			downloaded_file->Initialize ();
			
			if (is_streaming) {
				downloader->SetRequestPositionFunc (data_request_position);
			}
			downloader->SetWriteFunc (data_write, size_notify, this);
		}
		
		if (!(flags & DownloadComplete)) {
			downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
			downloader->AddHandler (downloader->DownloadFailedEvent, downloader_failed, this);
		}
		
		if (downloaded_file != NULL) {
			// MediaElement::SetSource() is already async, so we don't need another
			// layer of asyncronicity... it is safe to call SendNow() here.
			downloader->SendNow ();
		}
	} else {
		Invalidate ();
	}
}

void
MediaElement::SetSource (Downloader *downloader, const char *PartName)
{
	// Remove our playlist.
	// When the playlist changes media, it will call
	// SetSourceInternal to avoid ending up deleting itself.
	//
	if (playlist) {
		playlist->unref ();
		playlist = NULL;
	}
	
	Reinitialize (false);
	
	MediaBase::SetSource (downloader, PartName);
}

void
MediaElement::SetPlayRequested ()
{
	flags |= PlayRequested;
}

void
MediaElement::PlayOrPauseNow ()
{
	d (printf ("MediaElement::PlayOrPause (): GetCanPause (): %i, PlayRequested: %i, GetAutoPlay: %i, AutoPlayed: %i\n", GetCanPause (), flags & PlayRequested, GetAutoPlay (), playlist->GetAutoPlayed ()));

	if (!GetCanPause ()) {
		// If we can't pause, we play
		PlayNow ();
	} else if (flags & PlayRequested) {
		// A Play has already been requested.
		PlayNow ();
	} else if (GetAutoPlay () && !playlist->GetAutoPlayed ()) {
		// Autoplay us.
		playlist->SetAutoPlayed (true);
		PlayNow ();
	} else {
		PauseNow ();
	}
}

void
MediaElement::Pause ()
{
	d(printf ("MediaElement::Pause (): current state: %s\n", GetStateName (state)));
	
	AddTickCall (MediaElement::PauseNow);
}

void
MediaElement::PauseNow (EventObject *data)
{
	((MediaElement *) data)->PauseNow ();
}

void
MediaElement::PauseNow ()
{
	d(printf ("MediaElement::PauseNow (): current state: %s\n", GetStateName (state)));
	
	if (GetSurface () == NULL)
		return;
	
	switch (state) {
	case Opening:// docs: No specified behaviour
		flags &= ~PlayRequested;
		return;
	case Closed: // docs: No specified behaviour
	case Error:  // docs: ? (says nothing)
	case Paused:// docs: no-op
		return;
	case Buffering:
	case Playing:
	case Stopped: // docs: pause
		if (mplayer->GetCanPause ()) {
			if (playlist && playlist->Pause ())
				SetState (Paused);
		}
		EmitMediaOpened ();	
		break;
	}
}

void
MediaElement::Play ()
{
	d(printf ("MediaElement::Play (): current state: %s\n", GetStateName (state)));
	
	AddTickCall (MediaElement::PlayNow);
}

void
MediaElement::PlayNow (EventObject *data)
{
	((MediaElement *) data)->PlayNow ();
}

void
MediaElement::PlayNow ()
{
	d(printf ("MediaElement::PlayNow (): current state: %s\n", GetStateName (state)));
	
	if (GetSurface () == NULL)
		return;
		
	switch (state) {
	case Closed: // docs: No specified behaviour
	case Opening:// docs: No specified behaviour
		flags |= PlayRequested;
		return;
	case Error:  // docs: ? (says nothing)
	case Playing:// docs: no-op
		return;
	case Buffering:
	case Paused:
	case Stopped: // docs: start playing
		flags |= PlayRequested;
		playlist->Play ();
		break;
	}
}

void
MediaElement::PlayInternal ()
{
	d(printf ("MediaElement::PlayInternal (), state = %s, timeout_id: %i\n",
		  GetStateName (state), advance_frame_timeout_id));
	
	flags &= ~PlayRequested;
	SetState (Playing);
	mplayer->Play ();
	
	// Reinitialize our AdvanceFrame timeout
	if (advance_frame_timeout_id != 0) {
		GetTimeManager()->RemoveTimeout (advance_frame_timeout_id);
		advance_frame_timeout_id = 0;
	}
	
	advance_frame_timeout_id = GetTimeManager ()->AddTimeout (mplayer->GetTimeoutInterval (),
								  media_element_advance_frame, this);
	
	d(printf ("MediaElement::PlayInternal (), state = %s, timeout_id: %i, interval: %i [Done]\n",
		  GetStateName (state), advance_frame_timeout_id, mplayer->GetTimeoutInterval ()));
		  
	EmitMediaOpened ();
}

void
MediaElement::Stop ()
{
	d(printf ("MediaElement::Stop (): current state: %s\n", GetStateName (state)));
	
	AddTickCall (MediaElement::StopNow);
}

void
MediaElement::StopNow (EventObject *data)
{
	((MediaElement *) data)->StopNow ();
}

void
MediaElement::StopNow ()
{
	d(printf ("MediaElement::StopNow (): current state: %s\n", GetStateName (state)));
	
	if (GetSurface () == NULL)
		return;

	switch (state) {
	case Opening:// docs: No specified behaviour
		flags &= ~PlayRequested;
		return;
	case Closed: // docs: No specified behaviour
	case Error:  // docs: ? (says nothing)
	case Stopped:// docs: no-op
		return;
	case Buffering:
	case Playing:
	case Paused: // docs: stop
		playlist->Stop ();
		SetState (Stopped);
		Invalidate ();
		break;
	}
}

Value *
MediaElement::GetValue (DependencyProperty *prop)
{
	if (prop == MediaElement::PositionProperty) {
		bool use_mplayer;
		guint64 position = TimeSpan_ToPts (seek_to_position);
		
		switch (state) {
		case Opening:
		case Closed:
		case Error:
			use_mplayer = false;
			break;
		case Stopped:
		case Buffering:
		case Playing:
		case Paused:
		default:
			use_mplayer = true;
			break;
		}
		
		// If a seek is pending, we need to return that position.
		
		if (use_mplayer && (TimeSpan_FromPts (position) == -1))
			position = mplayer->GetPosition ();
		
		if (TimeSpan_FromPts (position) != -1) {
			Value v = Value (TimeSpan_FromPts (position), Type::TIMESPAN);
			
			flags |= UpdatingPosition;
			SetValue (prop, &v);
			flags &= ~UpdatingPosition;
		}
	}
	
	return MediaBase::GetValue (prop);
}

TimeSpan
MediaElement::UpdatePlayerPosition (TimeSpan position)
{
	Duration *duration = GetNaturalDuration ();
	
	if (duration->HasTimeSpan () && position > duration->GetTimeSpan ())
		position = duration->GetTimeSpan ();
	else if (position < 0)
		position = 0;
	
	if (position == (TimeSpan) mplayer->GetPosition ())
		return position;
	
	// position is a timespan, while mplayer expects time pts
	mplayer->Seek (TimeSpan_ToPts (position));
	Invalidate ();
	
	d(printf ("MediaElement::UpdatePlayerPosition (%llu = %llu ms, "
		  "mplayer->GetPosition (): %llu = %llu ms\n", position, MilliSeconds_FromPts (position),
		  mplayer->GetPosition (), MilliSeconds_FromPts (mplayer->GetPosition ())));
	
	return position;
}

void
MediaElement::OnLoaded ()
{
	if (!(flags & Loaded)) {
		flags |= Loaded;
		
		if (flags & TryOpenOnLoaded)
			TryOpen ();
	}
	
	MediaBase::OnLoaded ();
}

void
MediaElement::SeekNow (EventObject *data)
{
	((MediaElement *) data)->SeekNow ();
}

void
MediaElement::SeekNow ()
{
	d (printf ("MediaElement::SeekNow (), position: %llu = %llu ms\n", seek_to_position, MilliSeconds_FromPts (seek_to_position)));

	if (GetSurface () == NULL)
		return;

	if (seek_to_position == -1) {
		// This may happen if we get two seek requests in a row,
		// we handle the first one, and when we get to the second
		// seek_to_position is -1.
		return;
	}
				
	if (!(flags & UpdatingPosition)) {
		// Some outside source is updating the Position
		// property which means we need to Seek
		TimeSpan position;
		
		switch (state) {
		case Opening:
		case Closed:
		case Error:
		default:
			break;
		case Buffering:
		case Playing:
		case Paused:
		case Stopped:
			position = UpdatePlayerPosition (seek_to_position);
			seek_to_position = -1;
			
			if (position != seek_to_position) {
				flags |= UpdatingPosition;
				SetPosition (TimeSpan_FromPts (position));
				flags &= ~UpdatingPosition;
			}
			
			break;
		}
	}
}

void
MediaElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == MediaBase::SourceProperty) {
		// MediaBase will handle the rest
		flags |= RecalculateMatrix;
	} else if (args->property == MediaElement::AudioStreamIndexProperty) {
		// FIXME: set the audio stream index
	} else if (args->property == MediaElement::AutoPlayProperty) {
		// no state to change
	} else if (args->property == MediaElement::BalanceProperty) {
		mplayer->SetBalance (args->new_value->AsDouble ());
	} else if (args->property == MediaElement::BufferingProgressProperty) {
		// read-only property
	} else if (args->property == MediaElement::CurrentStateProperty) {
		Emit (CurrentStateChangedEvent);
	} else if (args->property == MediaElement::IsMutedProperty) {
		mplayer->SetMuted (args->new_value->AsBool ());
	} else if (args->property == MediaElement::MarkersProperty) {
		// FIXME: keep refs to these?
	} else if (args->property == MediaElement::NaturalVideoHeightProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (args->property == MediaElement::NaturalVideoWidthProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (args->property == MediaElement::PositionProperty) {
		if (!(flags & UpdatingPosition)) {
			seek_to_position = args->new_value->AsTimeSpan ();
			AddTickCall (MediaElement::SeekNow);
		} else if (IsPlaying() && mplayer->HasVideo ()) {
			Invalidate ();
		}
	} else if (args->property == MediaElement::VolumeProperty) {
		mplayer->SetVolume (args->new_value->AsDouble ());
	} else if (args->property == FrameworkElement::HeightProperty) {
		if (!updating_size_from_media)
			use_media_height = args->new_value == NULL;
	} else if (args->property == FrameworkElement::WidthProperty) {
		if (!updating_size_from_media)
			use_media_width = args->new_value == NULL;
	}
	
	if (args->property->type != Type::MEDIAELEMENT) {
		// propagate to parent class
		MediaBase::OnPropertyChanged (args);
		flags |= RecalculateMatrix;
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

bool 
MediaElement::EnableAntiAlias (void)
{
	return !(absolute_xform.xx == absolute_xform.yy &&		/* no rotation */
		 (absolute_xform.yx == 0 && absolute_xform.xy == 0));	/* no skew */

}

void
MediaElement::SetAttributes (MediaAttributeCollection *attrs)
{
	SetValue (MediaElement::AttributesProperty, Value (attrs));
}

MediaAttributeCollection *
MediaElement::GetAttributes ()
{
	Value *value = GetValue (MediaElement::AttributesProperty);
	
	return value ? value->AsMediaAttributeCollection () : NULL;
}

void
MediaElement::SetAudioStreamCount (int count)
{
	SetValue (MediaElement::AudioStreamCountProperty, Value (count));
}

int
MediaElement::GetAudioStreamCount ()
{
	return GetValue (MediaElement::AudioStreamCountProperty)->AsInt32 ();
}

void
MediaElement::SetAudioStreamIndex (int index)
{
	if (index >= 0)
		SetValue (MediaElement::AudioStreamIndexProperty, Value (index));
	else
		SetValue (MediaElement::AudioStreamIndexProperty, NULL);
}

int
MediaElement::GetAudioStreamIndex ()
{
	Value *value = GetValue (MediaElement::AudioStreamIndexProperty);
	int index = -1;
	
	if (value && value->AsNullableInt32 ())
		index = *value->AsNullableInt32 ();
	
	return index;
}

void
MediaElement::SetAutoPlay (bool set)
{
	SetValue (MediaElement::AutoPlayProperty, Value (set));
}

bool
MediaElement::GetAutoPlay ()
{
	return GetValue (MediaElement::AutoPlayProperty)->AsBool ();
}

void
MediaElement::SetBalance (double balance)
{
	SetValue (MediaElement::BalanceProperty, Value (balance));
}

double
MediaElement::GetBalance ()
{
	return GetValue (MediaElement::BalanceProperty)->AsDouble ();
}

void
MediaElement::SetBufferingProgress (double progress)
{
	SetValue (MediaElement::BufferingProgressProperty, Value (progress));
}

double
MediaElement::GetBufferingProgress ()
{
	return GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
}

void
MediaElement::SetBufferingTime (TimeSpan time)
{
	SetValue (MediaElement::BufferingTimeProperty, Value (time, Type::TIMESPAN));
}

TimeSpan
MediaElement::GetBufferingTime ()
{
	return (TimeSpan) GetValue (MediaElement::BufferingTimeProperty)->AsTimeSpan ();
}

void
MediaElement::SetCanPause (bool set)
{
	SetValue (MediaElement::CanPauseProperty, Value (set));
}

bool
MediaElement::GetCanPause ()
{
	return GetValue (MediaElement::CanPauseProperty)->AsBool ();
}

void
MediaElement::SetCanSeek (bool set)
{
	SetValue (MediaElement::CanSeekProperty, Value (set));
}

bool
MediaElement::GetCanSeek ()
{
	return GetValue (MediaElement::CanSeekProperty)->AsBool ();
}

void
MediaElement::SetCurrentState (const char *state)
{
	SetValue (MediaElement::CurrentStateProperty, Value (state));
}

const char *
MediaElement::GetCurrentState ()
{
	Value *value = GetValue (MediaElement::CurrentStateProperty);
	
	return value ? value->AsString () : NULL;
}

void
MediaElement::SetIsMuted (bool set)
{
	SetValue (MediaElement::IsMutedProperty, Value (set));
}

bool
MediaElement::GetIsMuted ()
{
	return GetValue (MediaElement::IsMutedProperty)->AsBool ();
}

void
MediaElement::SetMarkers (TimelineMarkerCollection *markers)
{
	SetValue (MediaElement::MarkersProperty, Value (markers));
}

TimelineMarkerCollection *
MediaElement::GetMarkers ()
{
	Value *value = GetValue (MediaElement::MarkersProperty);
	
	return value ? value->AsTimelineMarkerCollection () : NULL;
}

void
MediaElement::SetNaturalDuration (TimeSpan duration)
{
	SetValue (MediaElement::NaturalDurationProperty, Value (Duration (duration)));
}

Duration *
MediaElement::GetNaturalDuration ()
{
	return GetValue (MediaElement::NaturalDurationProperty)->AsDuration ();
}

void
MediaElement::SetNaturalVideoHeight (double height)
{
	SetValue (MediaElement::NaturalVideoHeightProperty, Value (height));
}

double
MediaElement::GetNaturalVideoHeight ()
{
	return GetValue (MediaElement::NaturalVideoHeightProperty)->AsDouble ();
}

void
MediaElement::SetNaturalVideoWidth (double width)
{
	SetValue (MediaElement::NaturalVideoWidthProperty, Value (width));
}

double
MediaElement::GetNaturalVideoWidth ()
{
	return GetValue (MediaElement::NaturalVideoWidthProperty)->AsDouble ();
}

void
MediaElement::SetPosition (TimeSpan position)
{
	SetValue (MediaElement::PositionProperty, Value (position, Type::TIMESPAN));
}

TimeSpan
MediaElement::GetPosition ()
{
	return (TimeSpan) GetValue (MediaElement::PositionProperty)->AsTimeSpan ();
}

void
MediaElement::SetVolume (double volume)
{
	SetValue (MediaElement::VolumeProperty, Value (volume));
}

double
MediaElement::GetVolume ()
{
	return GetValue (MediaElement::VolumeProperty)->AsDouble ();
}


MediaElement *
media_element_new (void)
{
	return new MediaElement ();
}

void
media_element_stop (MediaElement *media)
{
	media->Stop ();
}

void 
media_element_play (MediaElement *media)
{
	media->Play ();
}

void
media_element_pause (MediaElement *media)
{
	media->Pause ();
}

void
media_element_set_source (MediaElement *media, Downloader *downloader, const char *PartName)
{
	media->SetSource (downloader, PartName);
}

MediaAttributeCollection *
media_element_get_attributes (MediaElement *media)
{
	return media->GetAttributes ();
}

void
media_element_set_attributes (MediaElement *media, MediaAttributeCollection *attrs)
{
	media->SetAttributes (attrs);
}

int
media_element_get_audio_stream_count (MediaElement *media)
{
	return media->GetAudioStreamCount ();
}

int
media_element_get_audio_stream_index (MediaElement *media)
{
	return media->GetAudioStreamIndex ();
}

void
media_element_set_audio_stream_index (MediaElement *media, int index)
{
	media->SetAudioStreamIndex (index);
}

bool
media_element_get_auto_play (MediaElement *media)
{
	return media->GetAutoPlay ();
}

void
media_element_set_auto_play (MediaElement *media, bool set)
{
	media->SetAutoPlay (set);
}

double
media_element_get_balance (MediaElement *media)
{
	return media->GetBalance ();
}

void
media_element_set_balance (MediaElement *media, double balance)
{
	media->SetBalance (balance);
}

double
media_element_get_buffering_progress (MediaElement *media)
{
	return media->GetBufferingProgress ();
}

TimeSpan
media_element_get_buffering_time (MediaElement *media)
{
	return media->GetBufferingTime ();
}

void
media_element_set_buffering_time (MediaElement *media, TimeSpan time)
{
	media->SetBufferingTime (time);
}

bool
media_element_get_can_pause (MediaElement *media)
{
	return media->GetCanPause ();
}

bool
media_element_get_can_seek (MediaElement *media)
{
	return media->GetCanSeek ();
}

const char *
media_element_get_current_state (MediaElement *media)
{
	return media->GetCurrentState ();
}

void
media_element_set_current_state (MediaElement *media, const char *state)
{
	media->SetCurrentState (state);
}

bool
media_element_get_is_muted (MediaElement *media)
{
	return media->GetIsMuted ();
}

void
media_element_set_is_muted (MediaElement *media, bool set)
{
	media->SetIsMuted (set);
}

TimelineMarkerCollection *
media_element_get_markers (MediaElement *media)
{
	return media->GetMarkers ();
}

void
media_element_set_markers (MediaElement *media, TimelineMarkerCollection *markers)
{
	media->SetMarkers (markers);
}

Duration *
media_element_get_natural_duration (MediaElement *media)
{
	return media->GetNaturalDuration ();
}

double
media_element_get_natural_video_height (MediaElement *media)
{
	return media->GetNaturalVideoHeight ();
}

double
media_element_get_natural_video_width (MediaElement *media)
{
	return media->GetNaturalVideoWidth ();
}

TimeSpan
media_element_get_position (MediaElement *media)
{
	return media->GetPosition ();
}

void
media_element_set_position (MediaElement *media, TimeSpan position)
{
	media->SetPosition (position);
}

double
media_element_get_volume (MediaElement *media)
{
	return media->GetVolume ();
}

void
media_element_set_volume (MediaElement *media, double volume)
{
	media->SetVolume (volume);
}

//
// Image
//
GHashTable *Image::surface_cache = NULL;

Image::Image ()
{
	create_xlib_surface = true;
	
	pattern = NULL;
	brush = NULL;
	surface = NULL;
	loader = NULL;
	loader_err = NULL;
}

Image::~Image ()
{
	CleanupSurface ();
}

void
Image::CleanupPattern ()
{
	if (pattern) {
		cairo_pattern_destroy (pattern);
		pattern = NULL;
	}
}

void
Image::CleanupSurface ()
{
	CleanupPattern ();

	if (surface) {
		surface->ref_count--;
		if (surface->ref_count == 0) {
			g_hash_table_remove (surface_cache, surface->filename);
			g_free (surface->filename);
			cairo_surface_destroy (surface->cairo);
			if (surface->backing_pixbuf)
				g_object_unref (surface->backing_pixbuf);
			if (surface->backing_data)
				g_free (surface->backing_data);
			g_free (surface);
		}
		
		surface = NULL;
	}
}

void
Image::UpdateProgress ()
{
	double progress = downloader->GetDownloadProgress ();
	double current = GetDownloadProgress ();
	
	SetDownloadProgress (progress);
	
	/* only emit an event if the delta is >= 0.05% */
	if (progress == 1.0 || (progress - current) > 0.0005)
		Emit (DownloadProgressChangedEvent);
}

void
Image::SetSourceInternal (Downloader *downloader, char *PartName)
{
	MediaBase::SetSourceInternal (downloader, PartName);
	
	if (downloader) {
		downloader->AddHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
		
		if (downloader->Started () || downloader->Completed ()) {
			if (downloader->Completed ())
				DownloaderComplete ();
			
			UpdateProgress ();
		} else {
			downloader->SetWriteFunc (pixbuf_write, size_notify, this);
			
			// Image::SetSource() is already async, so we don't need another
			// layer of asyncronicity... it is safe to call SendNow() here.
			downloader->SendNow ();
		}
	} else {
		CleanupSurface ();
		Invalidate ();
	}
}

void
Image::SetSource (Downloader *downloader, const char *PartName)
{
	loader = NULL;
	MediaBase::SetSource (downloader, PartName);
}

void
Image::PixbufWrite (void *buf, gint32 offset, gint32 n)
{
	UpdateProgress ();
	if (loader == NULL)
		loader = gdk_pixbuf_loader_new ();

	if (!loader_err) {
		gdk_pixbuf_loader_write (GDK_PIXBUF_LOADER (loader), (const guchar *)buf, n, &loader_err);

		if (loader_err)
			gdk_pixbuf_loader_close (GDK_PIXBUF_LOADER (loader), NULL);
	}
}

void
Image::DownloaderComplete ()
{
	char *uri;

	if (strcmp (part_name, "") == 0)
		uri = g_strdup (downloader->GetUri ());
	else
		uri = g_strdup (downloader->GetDownloadedFilename (part_name));
	
	CleanupSurface ();
	
	if (!CreateSurface (uri)) {
		g_free (uri);
		Invalidate ();
		return;
	}
	
	g_free (uri);
	
	updating_size_from_media = true;
	
	if (use_media_width) {
		Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);

		if (!use_media_height)
			SetWidth ((double) surface->width * height->AsDouble () / (double) surface->height);
		else
			SetWidth ((double) surface->width);
	}
	
	if (use_media_height) {
		Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);

		if (!use_media_width)
			SetHeight ((double) surface->height * width->AsDouble () / (double) surface->width);
		else
			SetHeight ((double) surface->height);
	}
	
	updating_size_from_media = false;
	
	if (brush) {
		// FIXME: this is wrong, we probably need to set the
		// property, or use some other mechanism, but this is
		// gross.
		PropertyChangedEventArgs args (ImageBrush::DownloadProgressProperty, NULL, 
					       brush->GetValue (ImageBrush::DownloadProgressProperty));
		
		brush->OnPropertyChanged (&args);
	} else
		Invalidate ();
}

void
Image::DownloaderFailed (EventArgs *args)
{
	ErrorEventArgs *err = NULL;
	
	if (args && args->GetObjectType () == Type::ERROREVENTARGS)
		err = (ErrorEventArgs *) args;
	
	Emit (ImageFailedEvent, new ImageErrorEventArgs (err ? err->error_message : NULL));
	
	Invalidate ();
}


#ifdef WORDS_BIGENDIAN
#define set_pixel_bgra(pixel,index,b,g,r,a) \
	G_STMT_START { \
		((unsigned char *)(pixel))[index]   = a; \
		((unsigned char *)(pixel))[index+1] = r; \
		((unsigned char *)(pixel))[index+2] = g; \
		((unsigned char *)(pixel))[index+3] = b; \
	} G_STMT_END
#define get_pixel_bgr_p(p,b,g,r) \
	G_STMT_START { \
		r = *(p);   \
		g = *(p+1); \
		b = *(p+2); \
	} G_STMT_END
#else
#define set_pixel_bgra(pixel,index,b,g,r,a) \
	G_STMT_START { \
		((unsigned char *)(pixel))[index]   = b; \
		((unsigned char *)(pixel))[index+1] = g; \
		((unsigned char *)(pixel))[index+2] = r; \
		((unsigned char *)(pixel))[index+3] = a; \
	} G_STMT_END
#define get_pixel_bgr_p(p,b,g,r) \
	G_STMT_START { \
		b = *(p);   \
		g = *(p+1); \
		r = *(p+2); \
	} G_STMT_END
#endif
#define get_pixel_bgra(color, b, g, r, a) \
	G_STMT_START { \
		a = ((color & 0xff000000) >> 24); \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} G_STMT_END
#define get_pixel_bgr(color, b, g, r) \
	G_STMT_START { \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} G_STMT_END
#include "alpha-premul-table.inc"

//
// Expands RGB to ARGB allocating new buffer for it.
//
static guchar*
expand_rgb_to_argb (GdkPixbuf *pixbuf, int *stride)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	*stride = w * 4;
	guchar *data = (guchar *) g_malloc (*stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		out = data + y * (*stride);
		if (false && gdk_pixbuf_get_rowstride (pixbuf) % 4 == 0) {
			for (int x = 0; x < w; x ++) {
				guint32 color = *(guint32*)p;
				guchar r, g, b;

				get_pixel_bgr (color, b, g, r);
				set_pixel_bgra (out, 0, r, g, b, 255);

				p += 3;
				out += 4;
			}
		}
		else {
			for (int x = 0; x < w; x ++) {
				guchar r, g, b;

				get_pixel_bgr_p (p, b, g, r);
				set_pixel_bgra (out, 0, r, g, b, 255);

				p += 3;
				out += 4;
			}
		}
	}

	return data;
}

//
// Converts RGBA unmultiplied alpha to ARGB pre-multiplied alpha.
//
static void
unmultiply_rgba_in_place (GdkPixbuf *pixbuf)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		for (int x = 0; x < w; x ++) {
			guint32 color = *(guint32*)p;
			guchar r, g, b, a;

			get_pixel_bgra (color, b, g, r, a);

			/* pre-multipled alpha */
			if (a == 0) {
				r = g = b = 0;
			}
			else if (a < 255) {
				r = pre_multiplied_table [r][a];
				g = pre_multiplied_table [g][a];
				b = pre_multiplied_table [b][a];
			}

			/* store it back, swapping red and blue */
			set_pixel_bgra (p, 0, r, g, b, a);

			p += 4;
		}
	}
}

bool
Image::CreateSurface (const char *uri)
{
	if (surface) {
		// image surface already created
		return true;
	}

	CleanupPattern ();

	if (!surface_cache)
		surface_cache = g_hash_table_new (g_str_hash, g_str_equal);
	
	if (!(surface = (CachedSurface *) g_hash_table_lookup (surface_cache, uri))) {
		GdkPixbuf *pixbuf = NULL;
		char *msg;
		
		if (loader == NULL) {
			guchar buf[4096];
			ssize_t n;
			char *msg;
			char *filename;
			int fd;
                
			filename = downloader->GetDownloadedFilename (part_name);

			loader = gdk_pixbuf_loader_new ();

			if ((fd = open (filename, O_RDONLY)) == -1) {
				msg = g_strdup_printf ("Failed to load image %s: %s", filename, g_strerror (errno));
				Emit (ImageFailedEvent, new ImageErrorEventArgs (msg));
				return false;
			}

			do {
				do {
					n = read (fd, buf, sizeof (buf));
				} while (n == -1 && errno == EINTR);

				if (n == -1)
					break;

				gdk_pixbuf_loader_write (GDK_PIXBUF_LOADER (loader), buf, n, &loader_err);
			} while (n > 0 && !loader_err);

			close (fd);
		}

		gdk_pixbuf_loader_close (GDK_PIXBUF_LOADER (loader), loader_err ? NULL : &loader_err);
		
		if (!(pixbuf = gdk_pixbuf_loader_get_pixbuf (GDK_PIXBUF_LOADER (loader)))) {
			if (loader_err && loader_err->message)
				msg = g_strdup_printf ("Failed to load image %s: %s", uri, loader_err->message);
			else
				msg = g_strdup_printf ("Failed to load image %s", uri);
			
			Emit (ImageFailedEvent, new ImageErrorEventArgs (msg));
			
			if (loader_err)
				g_error_free (loader_err);
			
			return false;
		} else if (loader_err) {
			g_error_free (loader_err);
		}
		
		surface = g_new0 (CachedSurface, 1);
		
		surface->ref_count = 1;
		surface->filename = g_strdup (uri);
		surface->height = gdk_pixbuf_get_height (pixbuf);
		surface->width = gdk_pixbuf_get_width (pixbuf);
		
		bool has_alpha = gdk_pixbuf_get_n_channels (pixbuf) == 4;
		guchar *data;
		int stride;
		
		if (has_alpha) {
			surface->backing_pixbuf = pixbuf;
			surface->backing_data = NULL;
			unmultiply_rgba_in_place (pixbuf);
			stride = gdk_pixbuf_get_rowstride (pixbuf);
			data = gdk_pixbuf_get_pixels (pixbuf);
		} else {
			surface->backing_pixbuf = NULL;
			surface->backing_data = expand_rgb_to_argb (pixbuf, &stride);
			data = surface->backing_data;
			g_object_unref (pixbuf);
		}

		surface->cairo = cairo_image_surface_create_for_data (data,
#if USE_OPT_RGB24
								      has_alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
#else
								      CAIRO_FORMAT_ARGB32,
#endif
								      surface->width,
								      surface->height,
								      stride);

#if USE_OPT_RGB24
		surface->has_alpha = has_alpha;
#endif
		g_hash_table_insert (surface_cache, surface->filename, surface);
	} else {
		surface->ref_count++;
	}

	return true;
}

void
Image::size_notify (gint64 size, gpointer data)
{
	// Do something with it?
	// if size == -1, we do not know the size of the file, can happen
	// if the server does not return a Content-Length header
	//printf ("The image size is %lld\n", size);
}

void
Image::pixbuf_write (void *buf, gint32 offset, gint32 n, gpointer data)
{
	((Image *) data)->PixbufWrite (buf, offset, n);
}

void
Image::Render (cairo_t *cr, Region *region)
{
	if (!surface)
		return;
	
	if (create_xlib_surface && !surface->xlib_surface_created) {
		surface->xlib_surface_created = true;
		
		cairo_surface_t *xlib_surface = image_brush_create_similar (cr, surface->width, surface->height);
		cairo_t *cr = cairo_create (xlib_surface);

		cairo_set_source_surface (cr, surface->cairo, 0, 0);

		//cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);

		cairo_paint (cr);
		cairo_destroy (cr);

		cairo_surface_destroy (surface->cairo);

		if (surface->backing_pixbuf) {
			g_object_unref (surface->backing_pixbuf);
			surface->backing_pixbuf = NULL;
		}

		if (surface->backing_data) {
			g_free (surface->backing_data);
			surface->backing_data =NULL;
		}

		surface->cairo = xlib_surface;
	}

	cairo_save (cr);

	Stretch stretch = GetStretch ();
	double h = GetHeight ();
	double w = GetWidth ();
	
	if (!pattern)
		pattern = cairo_pattern_create_for_surface (surface->cairo);
	
	cairo_matrix_t matrix;
	
	image_brush_compute_pattern_matrix (&matrix, w, h, surface->width, surface->height, stretch, 
					    AlignmentXCenter, AlignmentYCenter, NULL, NULL);
	
	cairo_pattern_set_matrix (pattern, &matrix);
	cairo_set_source (cr, pattern);

	cairo_set_matrix (cr, &absolute_xform);
	
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_fill (cr);

	cairo_restore (cr);
}

Point
Image::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	
	return Point (GetWidth () * user_xform_origin.x, 
		      GetHeight () * user_xform_origin.y);
}

cairo_surface_t *
Image::GetCairoSurface ()
{
	return surface ? surface->cairo : NULL;
}

void
Image::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == FrameworkElement::HeightProperty) {
		if (!updating_size_from_media)
			use_media_height = args->new_value == NULL;
	} else if (args->property == FrameworkElement::WidthProperty) {
		if (!updating_size_from_media)
			use_media_width = args->new_value == NULL;
	}
	
	if (args->property->type != Type::IMAGE) {
		MediaBase::OnPropertyChanged (args);
		return;
	}
	
	// we need to notify attachees if our DownloadProgress changed.
	NotifyListenersOfPropertyChange (args);
}

bool
Image::InsideObject (cairo_t *cr, double x, double y)
{
	if (!surface)
		return false;

	return FrameworkElement::InsideObject (cr, x, y);
}

Image *
image_new (void)
{
	return new Image ();
}

void
image_set_source (Image *img, Downloader *downloader, const char *PartName)
{
	img->SetSource (downloader, PartName);
}

//
// MediaAttribute
//

DependencyProperty *MediaAttribute::ValueProperty;

MediaAttribute *
media_attribute_new (void)
{
	return new MediaAttribute ();
}

const char *
media_attribute_get_value (MediaAttribute *attribute)
{
	Value *value = attribute->GetValue (MediaAttribute::ValueProperty);
	return value ? value->AsString () : NULL;
}

void
media_attribute_set_value (MediaAttribute *attribute, const char *value)
{
	attribute->SetValue (MediaAttribute::ValueProperty, Value (value));
}

//
// MediaAttributeCollection
//

static bool
media_attribute_by_name_finder (List::Node *node, void *data)
{
	Collection::Node *cn = (Collection::Node *) node;
	MediaAttribute *attribute = (MediaAttribute *) cn->obj;
	const char *name = (const char *) data;

	Value *value = attribute->GetValue (DependencyObject::NameProperty);
	if (!value)
		return false;

	return !strcmp (name, value->AsString ());
}

MediaAttribute *
MediaAttributeCollection::GetItemByName (const char *name)
{
	Collection::Node *cn = (Collection::Node *) list->Find (media_attribute_by_name_finder, (char *) name);
	if (!cn)
		return NULL;

	return (MediaAttribute *) cn->obj;
}

MediaAttributeCollection *
media_attribute_collection_new (void)
{
	return new MediaAttributeCollection ();
}

MediaAttribute *
media_attribute_collection_get_item_by_name (MediaAttributeCollection *collection, const char *name)
{
	return collection->GetItemByName (name);
}

//
// TimelineMarkerCollection
// 
bool
TimelineMarkerCollection::Insert (int index, DependencyObject *data)
{
	return Add (data) != -1;
}

int
TimelineMarkerCollection::AddToList (Collection::Node *node)
{
	TimelineMarker *added_marker = (TimelineMarker *) node->obj;
	TimelineMarker *current_marker;
	Collection::Node *current;
	int counter = 0;
		
	current = (Collection::Node *) list->First ();
	while (current != NULL) {
		current_marker = (TimelineMarker *) current->obj;
		
		if (current_marker->GetTime () >= added_marker->GetTime ()) {
			list->InsertBefore (node, current);
			return counter;
		}
		
		current = (Collection::Node *) current->next;
		counter++;
	}
	
	list->Append (node);
	return counter;
}

TimelineMarkerCollection *
timeline_marker_collection_new (void)
{
	return new TimelineMarkerCollection ();
}

//
// MarkerReachedEventArgs
//

MarkerReachedEventArgs::MarkerReachedEventArgs (TimelineMarker *marker)
{
	this->marker = marker;
	this->marker->ref ();
}

MarkerReachedEventArgs::~MarkerReachedEventArgs ()
{
	marker->unref ();
}



void
media_init (void)
{
	/* MediaAttribute */
	MediaAttribute::ValueProperty = DependencyObject::Register (Type::MEDIAATTRIBUTE, "Value", Type::STRING);
	
	/* MediaBase */
	MediaBase::SourceProperty = DependencyObject::RegisterFull (Type::MEDIABASE, "Source", NULL, Type::STRING, false, false, true);
	MediaBase::StretchProperty = DependencyObject::Register (Type::MEDIABASE, "Stretch", new Value (StretchUniform));
	MediaBase::DownloadProgressProperty = DependencyObject::Register (Type::MEDIABASE, "DownloadProgress", new Value (0.0));
	
	/* MediaElement */
	MediaElement::AttributesProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Attributes", Type::MEDIAATTRIBUTE_COLLECTION);
	MediaElement::AudioStreamCountProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "AudioStreamCount", new Value (0), Type::INT32, false, true);
	MediaElement::AudioStreamIndexProperty = DependencyObject::RegisterNullable (Type::MEDIAELEMENT, "AudioStreamIndex", Type::INT32);
	MediaElement::AutoPlayProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AutoPlay", new Value (true));
	MediaElement::BalanceProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Balance", new Value (0.0));
	MediaElement::BufferingProgressProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "BufferingProgress", new Value (0.0), Type::DOUBLE, false, true);
	MediaElement::BufferingTimeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingTime", new Value (TimeSpan_FromSeconds (5), Type::TIMESPAN));
	MediaElement::CanPauseProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "CanPause", new Value (false), Type::BOOL, false, true);
	MediaElement::CanSeekProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "CanSeek", new Value (false), Type::BOOL, false, true);
	MediaElement::CurrentStateProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "CurrentState", NULL, Type::STRING, false, true);
	MediaElement::IsMutedProperty = DependencyObject::Register (Type::MEDIAELEMENT, "IsMuted", new Value (false));
	MediaElement::MarkersProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Markers", Type::TIMELINEMARKER_COLLECTION);
	MediaElement::NaturalDurationProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "NaturalDuration", new Value (Duration::FromSeconds (0)), Type::DURATION, false, true);
	MediaElement::NaturalVideoHeightProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "NaturalVideoHeight", new Value (0.0), Type::DOUBLE, false, true);
	MediaElement::NaturalVideoWidthProperty = DependencyObject::RegisterFull (Type::MEDIAELEMENT, "NaturalVideoWidth", new Value (0.0), Type::DOUBLE, false, true);
	MediaElement::PositionProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Position", Type::TIMESPAN);
	MediaElement::VolumeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Volume", new Value (0.5));
 	
	Media::Initialize ();
}
