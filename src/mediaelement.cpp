/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mediaelement.cpp: 
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
#include "downloader.h"
#include "playlist.h"
#include "pipeline.h"
#include "pipeline-asf.h"
#include "pipeline-ui.h"
#include "mediaelement.h"
#include "debug.h"
#include "validators.h"


// still too ugly to be exposed in the header files ;-)
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform,
					 Transform *relative_transform);
//
// MediaElement
//

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
	MissingCodecs       = (1 << 11), // set if we have no video codecs
};


static const char *media_element_states[] = { "Closed", "Opening", "Buffering", "Playing", "Paused", "Stopped", "Error" };

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
	
	MarkerNode (TimelineMarker *marker) { this->marker = marker; marker->ref (); }
	virtual ~MarkerNode () { marker->unref (); }
};

void
MediaElement::AddStreamedMarker (TimelineMarker *marker)
{	
	LOG_MEDIAELEMENT ("MediaElement::AddStreamedMarker (): got marker %s, %s, %llu = %llu ms\n",
			  marker->GetText (), marker->GetType (), marker->GetTime (),
			  MilliSeconds_FromPts (marker->GetTime ()));
	
	pending_streamed_markers->Push (new MarkerNode (marker));
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
	
	LOG_MEDIAELEMENT ("MediaElement::AddStreamedMarkers ()\n");
	
	if (streamed_markers == NULL)
		streamed_markers = new TimelineMarkerCollection ();

	while ((node = (MarkerNode *) pending_streamed_markers->Pop ()) != NULL) {
		Value v(node->marker);
		streamed_markers->Add (&v);
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

		Value v(new_marker);
		markers->Add (&v);

		new_marker->unref ();
		
		current = (MediaMarker::Node *) current->next;
	}
	
	// Docs says we overwrite whatever's been loaded already.
	LOG_MEDIAELEMENT ("MediaElement::ReadMarkers (): setting %d markers.\n", markers->GetCount ());
	SetMarkers (markers);
	markers->unref ();
}

void
MediaElement::CheckMarkers (guint64 from, guint64 to)
{
	TimelineMarkerCollection *markers;
	
	LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu)\n", from, to);
	
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
	TimelineMarker *marker;
	Value *val = NULL;
	guint64 pts;
	bool emit;
	
	LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu, %p, %i). count: %i\n", from, to, markers, remove, markers ? markers->GetCount () : -1);
	
	if (markers == NULL)
		return;
	
	// We might want to use a more intelligent algorithm here, 
	// this code only loops through all markers on every frame.
	
	for (int i = 0; i < markers->GetCount (); i++) {
		marker = markers->GetValueAt (i)->AsTimelineMarker ();
		
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
			
			LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): emit: %i, Checking pts: %llu in marker with Text = %s, Type = %s (removed from from)\n",
					from <= MilliSeconds_ToPts (1000) ? 0 : from - MilliSeconds_ToPts (1000), to, emit, pts, marker->GetText (), marker->GetType ());
		} else {
			// Normal markers.
			emit = pts >= from && pts <= to;
			LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu in marker with Text = %s, Type = %s\n",
					from, to, pts, marker->GetText (), marker->GetType ());
		}
		
		if (emit) {
			LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu): Emitting: Text = %s, Type = %s, Time = %llu = %llu ms\n",
				     from, to, marker->GetText (), marker->GetType (), marker->GetTime (), MilliSeconds_FromPts (marker->GetTime ()));
			Emit (MarkerReachedEvent, new MarkerReachedEventArgs (marker));
		}
		
		if (remove && (pts <= to || emit)) {
			// Also delete markers we've passed by already
			markers->RemoveAt (i);
			i--;
		}
	}
}

void
MediaElement::MediaFinished ()
{
	LOG_MEDIAELEMENT ("MediaElement::MediaFinished ()\n");
	
	SetState (Paused);
	EmitMediaEnded ();
}

bool
MediaElement::AdvanceFrame ()
{
	guint64 position; // pts
	bool advanced;
	
	LOG_MEDIAELEMENT_EX ("MediaElement::AdvanceFrame (), IsPlaying: %i, HasVideo: %i, HasAudio: %i, IsSeeking: %i\n",
			     IsPlaying (), mplayer->HasVideo (), mplayer->HasAudio (), mplayer->IsSeeking ());
	
	if (!IsPlaying ())
		return false;
	
	if (!mplayer->HasVideo ())
		return false;
	
	advanced = mplayer->AdvanceFrame ();
	position = mplayer->GetPosition ();
	
	if (advanced && position != G_MAXUINT64) {
		LOG_MEDIAELEMENT ("MediaElement::AdvanceFrame (): advanced, setting position to: %llu = %llu ms\n", position, MilliSeconds_FromPts (position));
		flags |= UpdatingPosition;
		SetPosition (TimeSpan_FromPts (position));
		flags &= ~UpdatingPosition;
		last_played_pts = position;
		
		if (first_pts == G_MAXUINT64)
			first_pts = position;
	}
	
	if (advanced || !mplayer->IsSeeking ()) {
		LOG_MEDIAELEMENT_EX ("MediaElement::AdvanceFrame () previous_position: %llu = %llu ms, "
				     "position: %llu = %llu ms, advanced: %i\n", 
				     previous_position, MilliSeconds_FromPts (previous_position), position,
				     MilliSeconds_FromPts (position), advanced);
			
		AddStreamedMarkers ();
		if (position != G_MAXUINT64)
			CheckMarkers (previous_position, position);
	}
	
	if (!mplayer->IsSeeking () && position > previous_position && position != G_MAXUINT64) {
		// Add 1 to avoid the same position to be able to be both
		// beginning and end of a range (otherwise the same marker
		// might raise two events).
		previous_position = position + 1;
	}
	
	return !IsStopped ();
}

MediaElement::MediaElement ()
{
	static bool init = true;
	if (init) {
		init = false;
		MediaElement::NaturalVideoHeightProperty->SetValueValidator (Validators::IntGreaterThanZeroValidator);
		MediaElement::NaturalVideoWidthProperty->SetValueValidator (Validators::IntGreaterThanZeroValidator);
		MediaElement::AttributesProperty->SetValueValidator (Validators::MediaAttributeCollectionValidator);
	}
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
	
	if (playlist) {
		playlist->Dispose ();
		playlist->unref ();
		playlist = NULL;
	}
	
	delete pending_streamed_markers;
	
	pthread_mutex_destroy (&open_mutex);
}

void
MediaElement::SetSurface (Surface *s)
{
	if (GetSurface() == s)
		return;

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
	
	LOG_MEDIAELEMENT ("MediaElement::Reinitialize (%i)\n", dtor);
	
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
		if (dtor)
			downloaded_file->Dispose ();
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
	first_pts = G_MAXUINT64;
	seek_to_position = -1;
	buffering_mode = 0;
	
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

bool
MediaElement::IsMissingCodecs ()
{
	return flags & MissingCodecs;
}

DownloaderAccessPolicy
MediaElement::GetDownloaderPolicy (const char *uri)
{
	if (!g_ascii_strncasecmp (uri, "mms://", 6))
		return StreamingPolicy;
	
	return MediaPolicy;
}

void
MediaElement::SetMedia (Media *media)
{
	bool broadcast = false, seekable = true;
	
	LOG_MEDIAELEMENT ("MediaElement::SetMedia (%p), current media: %p\n", media, this->media);
	
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
		
		LOG_MEDIAELEMENT ("MediaElement::SetMedia () setting features %d to broadcast (%d) and seekable (%d)\n",
				  downloader->GetHttpStreamingFeatures (), broadcast, seekable);
		
		SetCanPause (!broadcast);
		SetCanSeek (seekable);
		
		if (broadcast)
			flags |= Broadcast;
		
	}
	
	if (!mplayer->Open (media))
		return;
	
	ReadMarkers ();
	media->SetBufferingEnabled (true);
	
	SetNaturalDuration (broadcast ? 0 : TimeSpan_FromPts (mplayer->GetDuration ()));
	SetNaturalVideoHeight ((double) mplayer->GetVideoHeight ());
	SetNaturalVideoWidth ((double) mplayer->GetVideoWidth ());
	SetAudioStreamCount (mplayer->GetAudioStreamCount ());
	
	if (mplayer->HasAudio ()) {
		mplayer->SetMuted (GetIsMuted ());
		mplayer->SetVolume (GetVolume ());
		mplayer->SetBalance (GetBalance ());
	}
	
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
	IMediaDemuxer *demuxer = media->GetDemuxer ();
	const char *demux_name = demuxer->GetName ();
	
	LOG_MEDIAELEMENT ("MediaElement::MediaOpened (%p), demuxer name: %s, download complete: %i\n", media, demux_name, flags & DownloadComplete);
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		IMediaDecoder *decoder = stream->GetDecoder ();
		const char *decoder_name = decoder ? decoder->GetName () : NULL;
		if (decoder_name != NULL && strcmp (decoder_name, "NullDecoder") == 0) {
			flags |= MissingCodecs;
			break;
		}
	}
	if (flags & MissingCodecs)
		CodecDownloader::ShowUI (GetSurface ());

	if (demux_name != NULL && strcmp (demux_name, "ASXDemuxer") == 0) {
		Playlist *pl = ((ASXDemuxer *) media->GetDemuxer ())->GetPlaylist ();
		if (playlist == NULL) {
			playlist = pl;
			playlist->ref ();
			playlist->SetMedia (media, false);
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
			PlayOrStopNow ();
			Invalidate ();
			EmitMediaOpened ();
		}
		
		return true;
	}
	if (downloaded_file != NULL)
		downloaded_file->SetMedia (media);
}

void
MediaElement::EmitMediaOpened ()
{
	LOG_MEDIAELEMENT ("MediaElement::EmitMediaOpened (): already emitted: %s, current state: %s\n", flags & MediaOpenedEmitted ? "true" : "false", GetStateName (state));

	if (flags & MediaOpenedEmitted)
		return;

	flags |= MediaOpenedEmitted;
	
	Emit (MediaOpenedEvent);

}

void
MediaElement::EmitMediaEnded ()
{
	LOG_MEDIAELEMENT_EX ("MediaElement::EmitMediaEnded (), playlist: %p, isCurrentLastEntry: %i\n", playlist, playlist ? playlist->IsCurrentEntryLastEntry () : -1);
	
	if (playlist == NULL || playlist->IsCurrentEntryLastEntry ())
		Emit (MediaEndedEvent);
		
	if (playlist)
		playlist->OnEntryEnded ();
}

void
MediaElement::MediaFailed (ErrorEventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::MediaFailed (%p)\n", args);
	
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

	if (playlist)
		playlist->OnEntryFailed ();
}

Point
MediaElement::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	double h = GetHeight ();
	double w = GetWidth ();
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetVideoHeight ();
		w = (double) mplayer->GetVideoWidth ();
	}
	
	return Point (user_xform_origin->x * w, user_xform_origin->y * h);
}

Rect
MediaElement::GetCoverageBounds ()
{
	MediaPlayer *mplayer = GetMediaPlayer ();
	Stretch stretch = GetStretch ();

	if  (IsClosed () || !mplayer || !mplayer->HasRenderedFrame ())
		return Rect ();

	if (stretch == StretchFill || stretch == StretchUniformToFill)
		return bounds;

	Rect video = Rect (0, 0, mplayer->GetVideoWidth (), mplayer->GetVideoHeight ());
	cairo_matrix_t brush_xform = matrix;

	cairo_matrix_invert (&brush_xform);
	cairo_matrix_multiply (&brush_xform, &brush_xform, &absolute_xform);

	video = video.Transform (&brush_xform);
	video = video.Intersection (bounds);

	return video;
}

void
MediaElement::Render (cairo_t *cr, Region *region)
{
	Stretch stretch = GetStretch ();
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (!(surface = mplayer->GetCairoSurface ()))
		return;
	
	if (downloader == NULL)
		return;
	
	cairo_save (cr);
	
	cairo_set_matrix (cr, &absolute_xform);
	
	// if we're opaque, we can likely do this and hopefully get a
	// speed up since the server won't have to blend.
	//cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_new_path (cr);
	
	Rect paint = Rect (0, 0, GetWidth (), GetHeight ());
	Rect video = Rect (0, 0, mplayer->GetVideoWidth (), mplayer->GetVideoHeight ());

	/* FIXME NaN */
	if (paint.width == 0.0 && paint.height == 0.0)
		paint = video;
	
	/* snap paint rect to device space */
	if (absolute_xform.xy == 0 && absolute_xform.yx == 0) {
		//cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		cairo_matrix_t inv = absolute_xform;
		cairo_matrix_invert (&inv);
		paint = paint.Transform (&absolute_xform);
		paint = paint.RoundIn ();
		paint = paint.Transform (&inv);
	}
	
	if (flags & RecalculateMatrix) {
		image_brush_compute_pattern_matrix (&matrix, 
						    paint.width, paint.height, 
						    video.width, video.height,
						    stretch, AlignmentXCenter, AlignmentYCenter, NULL, NULL);

		flags &= ~RecalculateMatrix;
	}
	
	pattern = cairo_pattern_create_for_surface (surface);	
	
	
	cairo_pattern_set_matrix (pattern, &matrix);
		
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	
	if (IsPlaying ())
		cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);
	
	paint.Draw (cr);
	cairo_fill (cr);
	
	cairo_restore (cr);
}

double
MediaElement::GetBufferedSize ()
{
	double result = 0.0;
	guint64 buffering_time = G_MAXUINT64;
	guint64 buffered_time = G_MAXUINT64;
	IMediaDemuxer *demuxer;

	buffering_time = TimeSpan_ToPts (GetBufferingTime ());

	if (buffering_time == 0)
		return 1.0;

	if (!media)
		return 0.0;
		
	demuxer = media->GetDemuxer ();

	if (!demuxer)
		return 0.0;

	buffered_time = demuxer->GetBufferedSize ();

	if (buffered_time >= buffering_time)
		return 1.0;
		
	result = (double) buffered_time / (double) buffering_time;
	
	return result;
}

double
MediaElement::CalculateBufferingProgress ()
{
	double result = 0.0;
	guint64 buffering_time = G_MAXUINT64;
	guint64 last_available_pts = G_MAXUINT64;
	guint64 position_pts = G_MAXUINT64;
	IMediaDemuxer *demuxer;

	buffering_time = TimeSpan_ToPts (GetBufferingTime ());
	position_pts = TimeSpan_ToPts (GetPosition ());
	
	if (buffering_time == 0)
		return 1.0;

	if (!media)
		return 0.0;
		
	demuxer = media->GetDemuxer ();

	if (!demuxer)
		return 0.0;

	last_available_pts = demuxer->GetLastAvailablePts ();
	
	if (buffering_mode == 0) {
		if (position_pts == 0) {
			buffering_mode = 1;
		} else if (demuxer->GetSource ()->CanSeekToPts ()) {
			buffering_mode = 2;
		} else if (position_pts + buffering_time > last_available_pts) {
			buffering_mode = 3;
		} else {
			buffering_mode = 2;
		}
	}

	switch (buffering_mode) {
	case 1:
	case 2: {
		result = GetBufferedSize ();
		break;
	}
	case 3: {
//      ("last available pts" - "last played pts") / ("seeked to pts" - "last played pts" + BufferingTime)
		double a = ((double) last_available_pts - (double) last_played_pts);
		double b = ((double) position_pts - (double) last_played_pts + (double) buffering_time);

		if (a < 0.0 || b < 0.0) {
			result = 0.0;
		} else {
			// check for /0
			result = b == 0 ? 1.0 : a / b;
			// ensure 0.0 <= result <= 1.0
			result = result < 0.0 ? 0.0 : (result > 1.0 ? 1.0 : result);
		}
		// The pipeline might stop buffering because it determines it has buffered enough,
		// while this calculation only gets us to 99% (and it will never get to 100% since
		// the pipeline has stopped reading more media).
		if (last_available_pts > position_pts && result != 1.0 && GetBufferedSize () == 1.0)
			result = 1.0;

		break;
	}
	default:
		fprintf (stderr, "Moonlight: MediaElement got an unexpected buffering mode (%i).\n", buffering_mode);
		result = 0.0;
		break;
	}

	LOG_MEDIAELEMENT_EX ("MediaElement::CalculateBufferingProgress () buffering mode: %i, result: %.2f, buffering time: %llu ms, position: %llu ms, last available pts: %llu ms\n",
			     buffering_mode, result, MilliSeconds_FromPts (buffering_time), MilliSeconds_FromPts (position_pts), MilliSeconds_FromPts (last_available_pts));

	return result;
}

void
MediaElement::UpdateProgress ()
{
	double progress, current;
	bool emit = false;
	
	LOG_MEDIAELEMENT_EX ("MediaElement::UpdateProgress (). Current state: %s\n", GetStateName (state));
	
	if (state & WaitingForOpen)
		return;
	
	if (downloaded_file != NULL && IsPlaying () && mplayer->IsBufferUnderflow () && GetBufferedSize () == 0.0) {
		// We're waiting for more data, switch to the 'Buffering' state.
		LOG_MEDIAELEMENT ("MediaElement::UpdateProgress (): Switching to 'Buffering', previous_position: "
				  "%llu = %llu ms, mplayer->GetPosition (): %llu = %llu ms, buffered size: %llu, "
				  "buffering progress: %.2f\n", 
				  previous_position, MilliSeconds_FromPts (previous_position), mplayer->GetPosition (),
				  MilliSeconds_FromPts (mplayer->GetPosition ()),
				  media ? media->GetDemuxer ()->GetBufferedSize () : 0, GetBufferedSize ());
		
		flags |= PlayRequested;
		SetBufferingProgress (0.0);
		Emit (BufferingProgressChangedEvent);
		SetState (Buffering);
		mplayer->Pause ();
		emit = true;
	}
	
	//if (media && media->GetDemuxer ())
	//	media->GetDemuxer ()->PrintBufferInformation ();
			
	if (IsBuffering ()) {
		progress = CalculateBufferingProgress ();
		current = GetBufferingProgress ();
		
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
		
		if (progress == 1.0)
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
		LOG_MEDIAELEMENT ("MediaElement::SetState (%d) state is not valid.\n", state);
		return;
	}
	
	LOG_MEDIAELEMENT ("MediaElement::SetState (%d): New state: %s, old state: %s\n",
			  state, GetStateName (state), GetStateName (this->state));	

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
		if (!(flags & BufferingFailed) && IsOpening () && offset > 4096 && (part_name == NULL || part_name[0] == 0))
			TryOpen ();
	}
	
	// Delay the propogating progress 1.0 until
	// the downloader has notified us it is done.
	double progress = downloader->GetDownloadProgress ();
	
	if (progress < 1.0)
		UpdateProgress ();
}

void 
MediaElement::data_write (void *buf, gint32 offset, gint32 n, gpointer data)
{
	((MediaElement *) data)->DataWrite (buf, offset, n);
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
	buffering_mode = 0;

	if (state != Buffering) {
		LOG_MEDIAELEMENT ("MediaElement::BufferingComplete (): current state is invalid ('%s'), should only be 'Buffering'\n",
				  GetStateName (state));
		return;
	}
	
	switch (prev_state) {
	case Opening: // Start playback
		PlayOrStopNow ();
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
		LOG_MEDIAELEMENT ("MediaElement::BufferingComplete (): previous state is invalid ('%s').\n",
				  GetStateName (prev_state));
		return;
	}
}

static MediaResult
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
	LOG_MEDIAELEMENT ("MediaElement::TryOpenFinished ()\n");
	
	// No locking should be necessary here, since we can't have another open request pending.
	MediaElement *element = (MediaElement *) user_data;
	MediaClosure *closure = element->closure;
	element->closure = NULL;
	element->flags &= ~WaitingForOpen;
	
	if (!closure)
		return;
	
	if (MEDIA_SUCCEEDED (closure->result)) {
		LOG_MEDIAELEMENT ("MediaElement::TryOpen (): download is not complete, but media was "
				  "opened successfully and we'll now start buffering.\n");
		element->last_played_pts = 0;
		element->SetState (Buffering);
		element->MediaOpened (closure->GetMedia ());
	} else if (closure->result == MEDIA_NOT_ENOUGH_DATA) {
		if (element->flags & DownloadComplete) {
			// Try again, the download completed after the media failed to open it due to not having
			// enough data.
			element->TryOpen ();
		} else {
			// do nothing, we neither failed nor succeeded.
			// clearing the WaitingForOpen flag above causes us to
			// try to open it again upon next write
		}
	} else {
		element->flags |= BufferingFailed;
		element->MediaFailed (new ErrorEventArgs (MediaError, 3001, "AG_E_INVALID_FILE_FORMAT"));
	}
	
	delete closure;
}

void
MediaElement::TryOpen ()
{
	MediaResult result; 
	
	LOG_MEDIAELEMENT ("MediaElement::TryOpen (), state: %s, flags: %i, Loaded: %i, WaitingForOpen: %i, DownloadComplete: %i\n", GetStateName (state), flags, flags & Loaded, flags & WaitingForOpen, flags & DownloadComplete);
	
	switch (state) {
	case Closed:
	case Error:
		LOG_MEDIAELEMENT ("MediaElement::TryOpen (): Current state (%s) is invalid.\n", GetStateName (state)); 
		// Should not happen
		return;
	case Playing:
	case Paused:
	case Buffering:
		// I don't think this should happen either
		LOG_MEDIAELEMENT ("MediaElement::TryOpen (): Current state (%s) was unexpected.\n", GetStateName (state));
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
		LOG_MEDIAELEMENT ("MediaElement::TryOpen (): Unknown state: %d\n", state);
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

		media->SetBufferingTime (TimeSpan_ToPts (GetBufferingTime ()));
		
		if (current_downloaded_file) {
			current_downloaded_file->ref ();
			current_downloaded_file->SetMedia (media);
		}
		
		if (filename == NULL && current_downloaded_file != NULL) {
			source = current_downloaded_file;
			source->ref ();
		} else {
			source = new FileSource (media, filename);
			g_free (filename);
		}
		
		if (!MEDIA_SUCCEEDED (result = source->Initialize ())) {
			MediaFailed ();
			media->Dispose ();
		} else if (!MEDIA_SUCCEEDED (result = media->Open (source))) {
			MediaFailed (new ErrorEventArgs (MediaError, 3001, "AG_E_INVALID_FILE_FORMAT"));
			media->Dispose ();
		} else {
			MediaOpened (media);
		}
		
		media->unref ();
		media = NULL;
	
		source->unref ();
		source = NULL;
		
		// If we have a downloaded file ourselves, delete it, we no longer need it.
		if (current_downloaded_file) {
			current_downloaded_file->Dispose ();
			current_downloaded_file->unref ();
			current_downloaded_file = NULL;
		}
	} else if (part_name != NULL && part_name[0] != 0) {
		// PartName is set, we can't buffer, download the entire file.
	} else if (!(flags & BufferingFailed) && (downloaded_file != NULL)) {
		flags |= WaitingForOpen;
		
		Media *media = new Media (this, downloader);
		
		media->SetBufferingTime (TimeSpan_ToPts (GetBufferingTime ()));
		MediaClosure *closure = new MediaClosure (media_element_open_callback);
		closure->SetContext (this);
		closure->SetMedia (media);
		media->OpenAsync (downloaded_file, closure);

		if (downloaded_file)
			downloaded_file->SetMedia (media);

		media->unref ();
		media = NULL;
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
			
			dl->Open ("GET", new_uri, MediaPolicy);
			SetSource (dl, "");
			g_free (new_uri);
			dl->unref ();
			return;
		}
	}
	
	MediaFailed (new ErrorEventArgs (MediaError, 4001, "AG_E_NETWORK_ERROR"));
}

void
MediaElement::DownloaderComplete ()
{
	LOG_MEDIAELEMENT ("MediaElement::DownloaderComplete (), downloader: %d, state: %s, previous state: %s\n",
			  GET_OBJ_ID (downloader), GetStateName (state), GetStateName (prev_state));
	
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
		LOG_MEDIAELEMENT ("MediaElement::DownloaderComplete (): Current state (%d) is invalid.\n", state);
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
		PlayOrStopNow ();
		EmitMediaOpened ();
		break;
	case Opening:
		// The media couldn't be buffered for some reason
		// Try to open it now
		TryOpen ();
		break;
	default:
		LOG_MEDIAELEMENT ("MediaElement::DownloaderComplete (): Unknown state: %d\n", state);
		return;
	}
}

void
MediaElement::SetSourceInternal (Downloader *downloader, char *PartName)
{
	const char *uri = downloader ? downloader->GetUri () : NULL;
	bool is_streaming = uri ? g_str_has_prefix (uri, "mms:") : false;
	
	LOG_MEDIAELEMENT ("MediaElement::SetSourceInternal (%p, '%s'), uri: %s\n", downloader, PartName, uri);
	
	Reinitialize (false);
	
	SetCanPause (!is_streaming);
	SetCanSeek (!is_streaming);
	SetBufferingProgress (0.0);
	
	MediaBase::SetSourceInternal (downloader, PartName);
	
	if (downloader) {
		SetState (Opening);
		
		if (downloader->Started ()) {
			flags |= DisableBuffering;
			
			if (downloader->Completed ())
				flags |= DownloadComplete;
			
			TryOpen ();
		} else {
			if (is_streaming) {
				downloaded_file = new MemoryQueueSource (media);
			} else {
				downloaded_file = new ProgressiveSource (media);
			}
			
			// FIXME: error check Initialize()
			downloaded_file->Initialize ();
			
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
		playlist->Dispose ();
		playlist->unref ();
		playlist = NULL;
	}
	
	Reinitialize (false);
	
	MediaBase::SetSource (downloader, PartName);
}

void
MediaElement::SetStreamSource (ManagedStreamCallbacks *callbacks)
{
	// This is a big hack just to get things working
	downloaded_file = new ManagedStreamSource (media, callbacks);
	SetState (Opening);
	flags |= Loaded;
	TryOpen ();
}

void
MediaElement::SetPlayRequested ()
{
	flags |= PlayRequested;
}

void
MediaElement::PlayOrStopNow ()
{
	LOG_MEDIAELEMENT ("MediaElement::PlayOrPause (): GetCanPause (): %s, PlayRequested: %s, GetAutoPlay: %s, AutoPlayed: %s\n",
			  GetCanPause () ? "true" : "false", (flags & PlayRequested) ? "true" : "false",
			  GetAutoPlay () ? "true" : "false", playlist->GetAutoPlayed () ? "true" : "false");
	
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
		SetState (Playing);
		SetState (Stopped);
	}
}

void
MediaElement::Pause ()
{
	LOG_MEDIAELEMENT ("MediaElement::Pause (): current state: %s\n", GetStateName (state));
	
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
	LOG_MEDIAELEMENT ("MediaElement::PauseNow (): current state: %s\n", GetStateName (state));
	
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
	LOG_MEDIAELEMENT ("MediaElement::Play (): current state: %s\n", GetStateName (state));
	
	switch (state) {
	case Opening:
	case Buffering:
	case Paused:
	case Playing:
	case Stopped:
		AddTickCall (MediaElement::PlayNow);
		break;
	default:
		// do nothing
		break;
	}
}

void
MediaElement::PlayNow (EventObject *data)
{
	((MediaElement *) data)->PlayNow ();
}

void
MediaElement::PlayNow ()
{
	LOG_MEDIAELEMENT ("MediaElement::PlayNow (): current state: %s\n", GetStateName (state));
	
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

static gboolean
media_element_advance_frame (void *user_data)
{
	return (gboolean) ((MediaElement *) user_data)->AdvanceFrame ();
}

void
MediaElement::PlayInternal ()
{
	LOG_MEDIAELEMENT ("MediaElement::PlayInternal (), state = %s, timeout_id: %i\n",
			  GetStateName (state), advance_frame_timeout_id);
	
	flags &= ~PlayRequested;
	SetState (Playing);
	mplayer->Play ();
	
	// Reinitialize our AdvanceFrame timeout
	if (advance_frame_timeout_id != 0) {
		GetTimeManager()->RemoveTimeout (advance_frame_timeout_id);
		advance_frame_timeout_id = 0;
	}
	
	advance_frame_timeout_id = GetTimeManager ()->AddTimeout (G_PRIORITY_DEFAULT - 10,
								  mplayer->GetTimeoutInterval (),
								  media_element_advance_frame, this);
	
	LOG_MEDIAELEMENT ("MediaElement::PlayInternal (), state = %s, timeout_id: %i, interval: %i [Done]\n",
			  GetStateName (state), advance_frame_timeout_id, mplayer->GetTimeoutInterval ());
		  
	EmitMediaOpened ();
}

void
MediaElement::Stop ()
{
	LOG_MEDIAELEMENT ("MediaElement::Stop (): current state: %s\n", GetStateName (state));
	
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
	LOG_MEDIAELEMENT ("MediaElement::StopNow (): current state: %s\n", GetStateName (state));
	
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
			Value v (TimeSpan_FromPts (position), Type::TIMESPAN);
			
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
	
	LOG_MEDIAELEMENT ("MediaElement::UpdatePlayerPosition (%llu = %llu ms, "
			  "mplayer->GetPosition (): %llu = %llu ms\n", position, MilliSeconds_FromPts (position),
			  mplayer->GetPosition (), MilliSeconds_FromPts (mplayer->GetPosition ()));
	
	previous_position = position;
	
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
	LOG_MEDIAELEMENT ("MediaElement::SeekNow (), position: %llu = %llu ms\n", seek_to_position, MilliSeconds_FromPts (seek_to_position));

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
		mplayer->SetAudioStreamIndex (args->new_value->AsInt32 ());
	} else if (args->property == MediaElement::AutoPlayProperty) {
		// no state to change
	} else if (args->property == MediaElement::BalanceProperty) {
		mplayer->SetBalance (args->new_value->AsDouble ());
	} else if (args->property == MediaElement::BufferingProgressProperty) {
		// read-only property
	} else if (args->property == MediaElement::BufferingTimeProperty) {
		if (media)
			media->SetBufferingTime (TimeSpan_ToPts (GetBufferingTime ()));
	} else if (args->property == MediaElement::CurrentStateProperty) {
		Emit (CurrentStateChangedEvent);
		Invalidate ();
	} else if (args->property == MediaElement::IsMutedProperty) {
		mplayer->SetMuted (args->new_value->AsBool ());
	} else if (args->property == MediaElement::MarkersProperty) {
		// 
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
		} else if (IsPlaying() && mplayer->HasVideo () && !IsMissingCodecs ()) {
			Invalidate (GetCoverageBounds ());
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
	
	if (args->property->GetOwnerType() != Type::MEDIAELEMENT) {
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
MediaElement::SetNaturalDuration (TimeSpan duration)
{
	SetValue (MediaElement::NaturalDurationProperty, Value (Duration (duration)));
}
