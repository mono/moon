/*
 * media.cpp: 
 *
 * Authors:
 *   Jeffrey Stedfast <fejj@novell.com>
 *   Jb Evain <jbevain@novell.com>
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "runtime.h"
#include "media.h"
#include "error.h"
#include "downloader.h"
#include "playlist.h"
#include "geometry.h"
#include "pipeline.h"

#define d(x)

// still too ugly to be exposed in the header files ;-)
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform, Transform *relative_transform);


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
MediaBase::SetSourceAsyncCallback ()
{
	if (!source.downloader)
		return;

	if (GetSurface () == NULL)
		return;
	
	SetSourceInternal (source.downloader, source.part_name);
	source.downloader->unref ();
	source.downloader = NULL;
	source.part_name = NULL;
}

void
MediaBase::SetSourceInternal (Downloader *downloader, char *PartName)
{
	this->downloader = downloader;
	part_name = PartName;
	
	if (downloader)
		downloader->ref ();
}

static gboolean
set_source_async (void *user_data)
{
	MediaBase *media = (MediaBase *) user_data;
	
	media->SetSourceAsyncCallback ();
	media->unref ();
	
	return false;
}

void
MediaBase::SetSource (Downloader *downloader, const char *PartName)
{
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
	
	ref ();
	TimeManager::InvokeOnMainThread (set_source_async, this);
}


MediaBase *
media_base_new (void)
{
	return new MediaBase ();
}

char *
media_base_get_source (MediaBase *media)
{
	Value *value = media->GetValue (MediaBase::SourceProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
media_base_set_source (MediaBase *media, const char *value)
{
	media->SetValue (MediaBase::SourceProperty, Value (value));
}

Stretch
media_base_get_stretch (MediaBase *media)
{
	return (Stretch) media->GetValue (MediaBase::StretchProperty)->AsInt32 ();
}

void
media_base_set_stretch (MediaBase *media, Stretch value)
{
	media->SetValue (MediaBase::StretchProperty, Value (value));
}

void
media_base_set_download_progress (MediaBase *media, double progress)
{
	media->SetValue (MediaBase::DownloadProgressProperty, Value (progress));
}

double
media_base_get_download_progress (MediaBase *media)
{
	return media->GetValue (MediaBase::DownloadProgressProperty)->AsDouble ();
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
	Loaded            = (1 << 0),  // set once OnLoaded has been called
	TryOpenOnLoaded   = (1 << 1),  // set if OnLoaded should call TryOpen
	PlayRequested     = (1 << 2),  // set if Play() has been requested prior to being ready
	BufferingFailed   = (1 << 3),  // set if TryOpen failed to buffer the media.
	DisableBuffering  = (1 << 4),  // set if we cannot give useful buffering progress
	DownloadComplete  = (1 << 5),  // set if the download is complete
	UpdatingPosition  = (1 << 6),  // set if we are updating the PositionProperty as opposed to someone else
	RecalculateMatrix = (1 << 7),  // set if the patern matrix needs to be recomputed
	WaitingForOpen    = (1 << 8)   // set if we've called OpenAsync on a media and we're waiting for the result	
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
	MediaMarker *marker = closure->marker;
	
	if (marker == NULL)
		return MEDIA_FAIL;
	
	uint64_t pts = marker->Pts ();
	
	TimelineMarker *tl_marker = new TimelineMarker ();
	tl_marker->SetValue (TimelineMarker::TextProperty, marker->Text ());
	tl_marker->SetValue (TimelineMarker::TypeProperty, marker->Type ());
	tl_marker->SetValue (TimelineMarker::TimeProperty, Value (pts, Type::TIMESPAN));
	element->AddStreamedMarker (tl_marker);
	tl_marker->unref ();
	
	return MEDIA_SUCCESS;
}

void
MediaElement::AddStreamedMarker (TimelineMarker *marker)
{
	if (streamed_markers == NULL)
		streamed_markers = new TimelineMarkerCollection ();
	streamed_markers->Add (marker);
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
			MediaClosure *closure = new MediaClosure (marker_callback);
			closure->SetContext (this);
			closure->SetMedia (media);
			stream->SetCallback (closure);
			break;
		}
	}
	
	TimelineMarkerCollection *col = NULL;
	MediaMarker::Node *current = (MediaMarker::Node *) media->GetMarkers ()->First ();
	
	if (current == NULL) {
		//printf ("MediaElement::ReadMarkers (): no markers.\n");
		return;
	}
	
	col = new TimelineMarkerCollection ();
	while (current != NULL) {
		MediaMarker *marker = current->marker;
		TimelineMarker *new_marker = new TimelineMarker ();
		new_marker->SetValue (TimelineMarker::TextProperty, marker->Text ());
		new_marker->SetValue (TimelineMarker::TypeProperty, marker->Type ());
		new_marker->SetValue (TimelineMarker::TimeProperty, Value (TimeSpan_FromPts (marker->Pts ()), Type::TIMESPAN));
		col->Add (new_marker);
		new_marker->unref ();
		
		current = (MediaMarker::Node *) current->next;
	}
	
	// Docs says we overwrite whatever's been loaded already.
	//printf ("MediaElement::ReadMarkers (): setting %d markers.\n", collection_count (col));
	SetValue (MarkersProperty, col);
	col->unref ();
}

void
MediaElement::CheckMarkers (uint64_t from, uint64_t to)
{
	if (from == to)
		return;
	
	Value *val = GetValue (MediaElement::MarkersProperty);
	
	if (val == NULL)
		return;
		
	CheckMarkers (from, to, val->AsTimelineMarkerCollection (), false);
	CheckMarkers (from, to, streamed_markers, true);	
}

void
MediaElement::CheckMarkers (uint64_t from, uint64_t to, TimelineMarkerCollection *col, bool remove)
{
	Collection::Node *node, *next;
	TimelineMarker *marker;
	Value *val = NULL;
	uint64_t pts;
	
	if (col == NULL)
		return;
	
	// We might want to use a more intelligent algorithm here, 
	// this code only loops through all markers on every frame.
	
	node = (Collection::Node *) col->list->First ();
	while (node != NULL) {
		if (!(marker = (TimelineMarker *) node->obj))
			return;
		
		if (!(val = marker->GetValue (TimelineMarker::TimeProperty)))
			return;
		
		pts = (uint64_t) val->AsTimeSpan ();
		
		//printf ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu\n", from, to, pts);
		
		if (pts >= from && pts <= to) {
			Emit (MarkerReachedEvent, new MarkerReachedEventArgs (marker));
		}
		
		next = (Collection::Node *) node->next;
		
		if (remove && pts <= to) {
			// Also delete markers we've passed by already
			col->list->Remove (node);
		}
		
		node = next;
	}
}

void
MediaElement::AudioFinished ()
{
	SetState (Stopped);
	Emit (MediaElement::MediaEndedEvent);
}

bool
MediaElement::AdvanceFrame ()
{
	uint64_t position; // pts
	bool advanced;
	
	d(printf ("MediaElement::AdvanceFrame (), IsPlaying: %i, HasVideo: %i, HasAudio: %i\n",
		  IsPlaying (), mplayer->HasVideo (), mplayer->HasAudio ()));
	
	if (!IsPlaying ())
		return false;
	
	if (!mplayer->HasVideo ())
		return false;
	
	advanced = mplayer->AdvanceFrame ();
	position = mplayer->GetPosition ();
	
	if (advanced) {
		//printf ("MediaElement::AdvanceFrame (): advanced, setting position to: %lld\n", position);
		flags |= UpdatingPosition;
		media_element_set_position (this, TimeSpan_FromPts (position));
		flags &= ~UpdatingPosition;
		last_played_pts = position;
	}
	
	CheckMarkers (previous_position, position);
	
	// Add 1 to avoid the same position to be able to be both
	// beginning and end of a range (otherwise the same marker
	// might raise two events).
	previous_position = position + 1;
	
	if (!advanced && mplayer->GetEof ()) {
		mplayer->Stop ();
		SetState (Stopped);
		Emit (MediaEndedEvent);
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
	downloaded_file = NULL;
	playlist = NULL;
	mplayer = NULL;
	media = NULL;
	closure = NULL;
	flags = 0;
	
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
	
	pthread_mutex_destroy (&open_mutex);
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
	
	UIElement::SetSurface (s);
}

void
MediaElement::Reinitialize (bool dtor)
{
	Value *val;
	
	if (mplayer)
		mplayer->Close (dtor);
	
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
		SetValue (MediaElement::CurrentStateProperty, Value ("Closed"));
	prev_state = Closed;
	state = Closed;
	
	DownloaderAbort ();
	
	if (downloaded_file) {
		downloaded_file->unref ();
		downloaded_file = NULL;
	}
	
	if (!dtor) {
		flags |= UpdatingPosition;
		SetValue (MediaElement::PositionProperty, Value (0, Type::TIMESPAN));
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
	
	if (streamed_markers)
		streamed_markers->unref ();
	streamed_markers = NULL;
	previous_position = 0;
	
	val = GetValue (MediaElement::MarkersProperty);
	if (val != NULL && val->AsCollection () != NULL)
		val->AsCollection ()->Clear ();
	
	val = GetValue (MediaElement::AttributesProperty);
	if (val != NULL && val->AsCollection () != NULL)
		val->AsCollection ()->Clear ();
	
	if (!dtor)
		SetValue (PositionProperty, Value (0, Type::TIMESPAN));
}

void
MediaElement::SetMedia (Media *media)
{
        d(printf ("MediaElement::SetMedia (%p), current media: %p\n", media, this->media));
	
	if (this->media == media)
		return;	

	this->media = media;
	if (!mplayer->Open (media))
		return;
	
	ReadMarkers ();
	
	SetValue (MediaElement::CanSeekProperty, Value (mplayer->CanSeek ()));
	SetValue (MediaElement::CanPauseProperty, Value (mplayer->CanPause ()));
	SetValue (MediaElement::AudioStreamCountProperty, Value (mplayer->GetAudioStreamCount ()));
	SetValue (MediaElement::NaturalDurationProperty, Value (Duration (TimeSpan_FromPts (mplayer->GetDuration ()))));
	SetValue (MediaElement::NaturalVideoHeightProperty, Value ((double) mplayer->GetHeight ()));
	SetValue (MediaElement::NaturalVideoWidthProperty, Value ((double) mplayer->GetWidth ()));
	
	mplayer->SetMuted (GetValue (MediaElement::IsMutedProperty)->AsBool ());
	mplayer->SetVolume (GetValue (MediaElement::VolumeProperty)->AsDouble ());
	mplayer->SetBalance (GetValue (MediaElement::BalanceProperty)->AsDouble ());
	
	UpdatePlayerPosition (GetValue (MediaElement::PositionProperty));
	
	ComputeBounds ();
}
	
bool
MediaElement::MediaOpened (Media *media)
{
	const char *demux_name = media->GetDemuxer ()->GetName ();
	
	d(printf ("MediaElement::MediaOpened (%p), demuxer name: %s\n", media, demux_name));
	
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
		media->unref ();
		return false;
	} else {
		if (playlist != NULL) {	
			playlist->GetCurrentEntry ()->SetMedia (media);
		} else {
			playlist = new Playlist (this, media);
		}
		playlist->GetCurrentEntry ()->PopulateMediaAttributes ();
		SetMedia (media);
		
		Emit (MediaOpenedEvent);
		return true;
	}
}

void
MediaElement::MediaFailed (ErrorEventArgs *args)
{
	d(printf ("MediaElement::MediaFailed (%p)\n", args));
	
	if (state == MediaElement::Error)
		return;
	
	SetValue (MediaElement::CanSeekProperty, Value (false));
	SetValue (MediaElement::CanPauseProperty, Value (false));
	SetValue (MediaElement::AudioStreamCountProperty, Value (0));
	SetValue (MediaElement::NaturalDurationProperty, Value (Duration::FromSeconds (0)));
	SetValue (MediaElement::NaturalVideoHeightProperty, Value (0.0));
	SetValue (MediaElement::NaturalVideoWidthProperty, Value (0.0));
	
	SetState (MediaElement::Error);
	Emit (MediaFailedEvent, args);
}

void
MediaElement::ComputeBounds ()
{
	double h = GetValue (HeightProperty)->AsDouble ();
	double w = GetValue (WidthProperty)->AsDouble ();
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetHeight ();
		w = (double) mplayer->GetWidth ();
	}
	
	Rect box = Rect (0, 0, w, h);
	
	bounds = IntersectBoundsWithClipPath (box, false).Transform (&absolute_xform); 
}

Point
MediaElement::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	double h = GetValue (HeightProperty)->AsDouble ();
	double w = GetValue (WidthProperty)->AsDouble ();
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetHeight ();
		w = (double) mplayer->GetWidth ();
	}
	
	return Point (user_xform_origin.x * w, user_xform_origin.y * h);
}

void
MediaElement::Render (cairo_t *cr, Region *region)
{
	Stretch stretch = (Stretch) GetValue (MediaBase::StretchProperty)->AsInt32 ();
	double h = GetValue (FrameworkElement::HeightProperty)->AsDouble ();
	double w = GetValue (FrameworkElement::WidthProperty)->AsDouble ();
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (!(surface = mplayer->GetCairoSurface ()))
		return;
	
	if (downloader == NULL)
		return;
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->GetHeight ();
		w = (double) mplayer->GetWidth ();
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
		image_brush_compute_pattern_matrix (&matrix, w, h, mplayer->GetWidth (), mplayer->GetHeight (), stretch,
						    AlignmentXCenter, AlignmentYCenter, NULL, NULL);
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

void
MediaElement::UpdateProgress ()
{
	double progress, current;
	bool emit = false;
	uint64_t currently_available_pts, current_pts, buffer_pts;

	d(printf ("MediaElement::UpdateProgress (). Current state: %s\n", GetStateName (state)));
	
	if (state & WaitingForOpen)
		return;
	
	if (downloaded_file != NULL && !IsBuffering () && downloaded_file->IsWaiting ()) {
		// We're waiting for more data, switch to the 'Buffering' state.
		d(printf ("MediaElement::UpdateProgress (): Switching to 'Buffering', previous_position: "
			  "%llu = %llu ms, mplayer->GetPosition (): %llu = %llu ms, last available pts: %llu\n", 
			  previous_position, MilliSeconds_FromPts (previous_position), mplayer->GetPosition (),
			  MilliSeconds_FromPts (mplayer->GetPosition ()),
			  media ? media->GetDemuxer ()->GetLastAvailablePts () : 0));
		
		SetValue (MediaElement::BufferingProgressProperty, Value (0.0));
		SetState (Buffering);
		mplayer->Pause ();
		emit = true;
	}
	
	// CHECK: if buffering, will DownloadCompletedEvent be emitted?
	
	if (IsBuffering ()) {
		if (media && media->GetDemuxer ()) {
			currently_available_pts = media->GetDemuxer ()->GetLastAvailablePts ();
		} else {
			currently_available_pts = 0;
		}
		current_pts = mplayer->GetPosition ();
		buffer_pts = TimeSpan_ToPts (GetValue (MediaElement::BufferingTimeProperty)->AsTimeSpan ());

		// Check that we don't cause any div/0.		
		if (current_pts - last_played_pts + buffer_pts == currently_available_pts - last_played_pts) {
			progress = 1.0;
		} else {
			progress = (double) (currently_available_pts - last_played_pts) / (double) (current_pts - last_played_pts + buffer_pts);
		}

		current = GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
		
		d(printf ("MediaElement::UpdateProgress (), buf start: %llu = %llu ms, buf end: %llu = %llu ms, "
			  "buf time: %llu = %llu ms, last_available_pts: %llu = %llu ms, current: %.2f, progress: %.2f\n",
			  buffering_start, MilliSeconds_FromPts (buffering_start), buffering_end,
			  MilliSeconds_FromPts (buffering_end), buffering_time, MilliSeconds_FromPts (buffering_time),
			  last_available_pts, MilliSeconds_FromPts (last_available_pts), current, progress));
		
		if (progress < 0.0)
			progress = 0.0;
		else if (progress > 1.0)
			progress = 1.0;

		if (current > progress) {
			// Somebody might have seeked further away after the first change to Buffering,
			// in which case the progress goes down. Don't emit any events in this case.
			emit = false;
		}
		
		// Emit the event if it's 100%, or a change of at least 0.05%
		if (emit || progress == 1.0 || (progress - current) >= 0.0005) {
			SetValue (MediaElement::BufferingProgressProperty, Value (progress));
			Emit (BufferingProgressChangedEvent);
		}
		
		if (progress == 1.0)
			BufferingComplete ();
	} else { 
		// FIXME: Do we emit DownloadProgressChangedEvent if we're playing the media?
		progress = downloader->GetValue (Downloader::DownloadProgressProperty)->AsDouble ();
		current = GetValue (MediaElement::DownloadProgressProperty)->AsDouble ();
		
		// Emit the event if it's 100%, or a change of at least 0.05%
		if (progress == 1.0 || (progress - current) >= 0.0005) {
			SetValue (MediaElement::DownloadProgressProperty, Value (progress));
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
	
	SetValue (MediaElement::CurrentStateProperty, Value (name));
}

void 
MediaElement::DataWrite (void *buf, int32_t offset, int32_t n)
{
	//printf ("MediaElement::DataWrite (%p, %d, %d)\n", buf, offset, n);
	
	if (downloaded_file != NULL) {
		downloaded_file->Write (buf, (int64_t) offset, n);
		
 		// FIXME: How much do we actually have to download in order to try to open the file?
		if (!(flags & BufferingFailed) && IsOpening () && offset > 1024 && (part_name == NULL || part_name[0] == 0))
			TryOpen ();
	}
	
	// Delay the propogating progress 1.0 until
	// the downloader has notified us it is done.
	double progress = downloader->GetValue (Downloader::DownloadProgressProperty)->AsDouble ();
	
	if (progress < 1.0)
		UpdateProgress ();
}

void
MediaElement::DataRequestPosition (int64_t *position)
{
       if (downloaded_file != NULL)
               downloaded_file->RequestPosition (position);
}

void 
MediaElement::data_write (void *buf, int32_t offset, int32_t n, gpointer data)
{
	((MediaElement *) data)->DataWrite (buf, offset, n);
}

void 
MediaElement::data_request_position (int64_t *position, gpointer data)
{
       ((MediaElement *) data)->DataRequestPosition (position);
}

void
MediaElement::size_notify (int64_t size, gpointer data)
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
		if ((flags & PlayRequested) || GetValue (AutoPlayProperty)->AsBool ())
			Play ();
		else
			Pause ();
		return;
	case Playing: // Restart playback
		Play ();
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
	MediaElement *element = (MediaElement*) closure->GetContext ();
	if (element != NULL) {
		// the closure will be deleted when we return from this function,
		// so make a copy of the data.
		pthread_mutex_lock (&element->open_mutex);
		if (element->closure)
			delete element->closure;
		element->closure = closure->Clone ();
		pthread_mutex_unlock (&element->open_mutex);
		// We need to call TryOpenFinished on the main thread, so 
		TimeManager::InvokeOnMainThread (MediaElement::TryOpenFinished, element);
	}
	return MEDIA_SUCCESS;
}

gboolean
MediaElement::TryOpenFinished (void *user_data)
{
	d(printf ("MediaElement::TryOpenFinished ()\n"));
	
	// No locking should be necessary here, since we can't have another open request pending.
	MediaElement *element = (MediaElement*) user_data;
	MediaClosure *closure = element->closure;
	element->closure = NULL;
	element->flags &= ~WaitingForOpen;
	
	if (MEDIA_SUCCEEDED (closure->result)) {
		d(printf ("MediaElement::TryOpen (): download is not complete, but media was "
			  "opened successfully and we'll now start buffering.\n"));
		element->last_played_pts = 0;
		element->SetState (Buffering);
		element->MediaOpened (closure->GetMedia ());
	} else {
		element->flags |=  BufferingFailed;
		// Seek back to the beginning of the file
		element->downloaded_file->Seek (0, SEEK_SET);
	}
	delete closure;
	return false;
}

void
MediaElement::TryOpen ()
{
	switch (state) {
	case Closed:
	case Error:
		d(printf ("MediaElement::TryOpen (): Current state (%s) is invalid.\n", GetStateName (state))); 
		// Should not happen
		return;
	case Playing:
	case Paused:
	case Stopped:
	case Buffering:
		// I don't think this should happen either
		d(printf ("MediaElement::TryOpen (): Current state (%s) was unexpected.\n", GetStateName (state)));
		// Media is already open.
		// There's nothing to do here.
		return;
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
	
	d(printf ("MediaElement::TryOpen ()\n"));
	
	if (flags & DownloadComplete) {
		IMediaSource *current_downloaded_file = downloaded_file;
		char *filename = downloader_get_response_file (downloader, part_name);
		Media *media = new Media (this);
		IMediaSource *source;
		
		if (current_downloaded_file)
			current_downloaded_file->ref ();

		source = new FileSource (media, filename);
		g_free (filename);
		
		if (MEDIA_SUCCEEDED (source->Initialize ()) && MEDIA_SUCCEEDED (media->Open (source))) {
			if (MediaOpened (media)) {
				SetState (Buffering);
				
				if ((flags & PlayRequested) || GetValue (AutoPlayProperty)->AsBool ())
					Play ();
				else
					Pause ();
				
				Invalidate ();
			}
		} else {
			MediaFailed ();
			media->unref ();
			media = NULL;
		}
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
		
		Media *media = new Media (this);
		
		MediaClosure *closure = new MediaClosure (media_element_open_callback);
		closure->SetContext (this);
		media->OpenAsync (downloaded_file, closure);
	}
	
	// FIXME: specify which audio stream index the player should use
}

void
MediaElement::DownloaderComplete ()
{
	flags |= DownloadComplete;
	
	UpdateProgress ();
	
	switch (state) {
	case Closed:
	case Error:
		// Should not happen
		d(printf ("MediaElement::DownloaderComplete (): Current state (%d) is invalid.\n", state));
		return;
	case Playing:
	case Paused:
	case Stopped:
		// Media was opened, buffered, and then played/paused/stopped
		// There's nothing to do here
		return;
	case Buffering:
	 	// Media finished downloading before the buffering time was reached.
		// Play it.
		if ((flags & PlayRequested) || prev_state == Playing || GetValue (AutoPlayProperty)->AsBool ())
			Play ();
		else
			Pause ();
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
	const char *uri = downloader ? downloader->GetValue (Downloader::UriProperty)->AsString () : NULL;
	bool is_live = uri ? g_str_has_prefix (uri, "mms:") : false;
	
	d(printf ("MediaElement::SetSourceInternal (%p, '%s'), uri: %s\n", downloader, PartName, uri));
	
	Reinitialize (false);
	
	MediaBase::SetSourceInternal (downloader, PartName);
	
	if (downloader) {
		SetState (Opening);
		
		if (downloader->Started ()) {
			flags |= DisableBuffering;
			
			if (downloader->Completed ())
				flags |= DownloadComplete;
			
			TryOpen ();
		} else {
			downloaded_file = new ProgressiveSource (mplayer->GetMedia (), is_live);
			
			// FIXME: error check Initialize()
			downloaded_file->Initialize ();
			
			downloader->SetWriteFunc (data_write, size_notify, this);
			if (is_live)
				downloader->SetRequestPositionFunc (data_request_position);
		}
		
		if (!(flags & DownloadComplete))
			downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
		
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
	
	MediaBase::SetSource (downloader, PartName);
}

void
MediaElement::Pause ()
{
	d(printf ("MediaElement::Pause (): current state: %s\n", GetStateName (state)));
	
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
		if (mplayer->CanPause ()) {
			if (playlist && playlist->Pause ())
				SetState (Paused);
		}
		break;
	}
}

void
MediaElement::Play ()
{
	d(printf ("MediaElement::Play (): current state: %s\n", GetStateName (state)));
	
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
}

void
MediaElement::Stop ()
{
	d(printf ("MediaElement::Stop (): current state: %s\n", GetStateName (state)));
	
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
		
		if (use_mplayer) {
			uint64_t position = mplayer->GetPosition ();
			Value v = Value (TimeSpan_FromPts (position), Type::TIMESPAN);
			
			flags |= UpdatingPosition;
			SetValue (prop, &v);
			flags &= ~UpdatingPosition;
		}
	}
	
	return MediaBase::GetValue (prop);
}

TimeSpan
MediaElement::UpdatePlayerPosition (Value *value)
{
	Duration *duration = GetValue (NaturalDurationProperty)->AsDuration ();
	TimeSpan position = value->AsTimeSpan ();
	
	if (duration->HasTimeSpan () && position > duration->GetTimeSpan ())
		position = duration->GetTimeSpan ();
	else if (position < 0)
		position = 0;
	
	if (position == (TimeSpan) mplayer->GetPosition ())
		return position;
	
	// position is a timespan, while mplayer expects time pts
	mplayer->Seek (TimeSpan_ToPts (position));
	Invalidate ();
	
	d(printf ("MediaElement::UpdatePlayerPosition (%p), buffering_start: %llu = %llu ms, "
		  "buffering_end: %llu = %llu ms, position: %llu = %llu ms, "
		  "mplayer->GetPosition (): %llu = %llu ms\n", value, buffering_start,
		  MilliSeconds_FromPts (buffering_start), buffering_end,
		  MilliSeconds_FromPts (buffering_end), position, MilliSeconds_FromPts (position),
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
MediaElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == MediaBase::SourceProperty) {
		char *uri = args->new_value ? args->new_value->AsString() : NULL;
		
		if (uri && *uri) {
			Downloader *dl = Surface::CreateDownloader (this);
			downloader_open (dl, "GET", uri);
			SetSource (dl, "");
			dl->unref ();
		} else {
			DownloaderAbort ();
			Invalidate ();
		}
		
		flags |= RecalculateMatrix;
	} else if (args->property == MediaElement::AudioStreamIndexProperty) {
		// FIXME: set the audio stream index
	} else if (args->property == MediaElement::AutoPlayProperty) {
		// no state to change
	} else if (args->property == MediaElement::BalanceProperty) {
		mplayer->SetBalance (args->new_value->AsDouble ());
	} else if (args->property == MediaElement::BufferingProgressProperty) {
		Emit (BufferingProgressChangedEvent);
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
				position = UpdatePlayerPosition (args->new_value);
				if (state == Stopped)
					SetState (Paused);
				
				if (position != args->new_value->AsTimeSpan ()) {
					Value v = Value (position, Type::TIMESPAN);
					SetValue (args->property, &v);
					return;
				}
				
				break;
			}
		} else if (IsPlaying() && mplayer->HasVideo ()) {
			Invalidate ();
		}
	} else if (args->property == MediaElement::VolumeProperty) {
		mplayer->SetVolume (args->new_value->AsDouble ());
	}
	
	if (args->property->type == Type::MEDIAELEMENT) {
		NotifyListenersOfPropertyChange (args);
	} else {
		// propagate to parent class
		MediaBase::OnPropertyChanged (args);
		flags |= RecalculateMatrix;
	}
}

bool 
MediaElement::EnableAntiAlias (void)
{
	return !(absolute_xform.xx == absolute_xform.yy &&		/* no rotation */
		 (absolute_xform.yx == 0 && absolute_xform.xy == 0));	/* no skew */

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
	Value *value = media->GetValue (MediaElement::AttributesProperty);
	
	return value ? (MediaAttributeCollection *) value->AsMediaAttributeCollection () : NULL;
}

void
media_element_set_attributes (MediaElement *media, MediaAttributeCollection *value)
{
	media->SetValue (MediaElement::AttributesProperty, Value (value));
}

int
media_element_get_audio_stream_count (MediaElement *media)
{
	return (int) media->GetValue (MediaElement::AudioStreamCountProperty)->AsInt32 ();
}

void
media_element_set_audio_stream_count (MediaElement *media, int value)
{
	media->SetValue (MediaElement::AudioStreamCountProperty, Value (value));
}

int
media_element_get_audio_stream_index (MediaElement *media)
{
	Value *value = media->GetValue (MediaElement::AudioStreamIndexProperty);
	int index = -1;
	
	if (value && value->AsNullableInt32 ())
		index = *value->AsNullableInt32 ();
	
	return index;
}

void
media_element_set_audio_stream_index (MediaElement *media, int value)
{
	if (value >= 0)
		media->SetValue (MediaElement::AudioStreamIndexProperty, Value (value));
	else
		media->SetValue (MediaElement::AudioStreamIndexProperty, NULL);
}

bool
media_element_get_auto_play (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::AutoPlayProperty)->AsBool ();
}

void
media_element_set_auto_play (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::AutoPlayProperty, Value (value));
}

double
media_element_get_balance (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BalanceProperty)->AsDouble ();
}

void
media_element_set_balance (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BalanceProperty, Value (value));
}

double
media_element_get_buffering_progress (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
}

void
media_element_set_buffering_progress (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BufferingProgressProperty, Value (value));
}

TimeSpan
media_element_get_buffering_time (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::BufferingTimeProperty)->AsTimeSpan ();
}

void
media_element_set_buffering_time (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::BufferingTimeProperty, Value (value, Type::TIMESPAN));
}

bool
media_element_get_can_pause (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::CanPauseProperty)->AsBool ();
}

void
media_element_set_can_pause (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::CanPauseProperty, Value (value));
}

bool
media_element_get_can_seek (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::CanSeekProperty)->AsBool ();
}

void
media_element_set_can_seek (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::CanSeekProperty, Value (value));
}

char *
media_element_get_current_state (MediaElement *media)
{
	Value *value = media->GetValue (MediaElement::CurrentStateProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
media_element_set_current_state (MediaElement *media, const char *value)
{
	media->SetValue (MediaElement::CurrentStateProperty, Value (value));
}

bool
media_element_get_is_muted (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::IsMutedProperty)->AsBool ();
}

void
media_element_set_is_muted (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::IsMutedProperty, Value (value));
}

TimelineMarkerCollection *
media_element_get_markers (MediaElement *media)
{
	Value *value = media->GetValue (MediaElement::MarkersProperty);
	
	return value ? (TimelineMarkerCollection *) value->AsTimelineMarkerCollection () : NULL;
}

void
media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value)
{
	media->SetValue (MediaElement::MarkersProperty, Value (value));
}

Duration *
media_element_get_natural_duration (MediaElement *media)
{
	return media->GetValue (MediaElement::NaturalDurationProperty)->AsDuration ();
}

void
media_element_set_natural_duration (MediaElement *media, Duration value)
{
	media->SetValue (MediaElement::NaturalDurationProperty, Value (value));
}

double
media_element_get_natural_video_height (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoHeightProperty)->AsDouble ();
}

void
media_element_set_natural_video_height (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoHeightProperty, Value (value));
}

double
media_element_get_natural_video_width (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoWidthProperty)->AsDouble ();
}

void
media_element_set_natural_video_width (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoWidthProperty, Value (value));
}

TimeSpan
media_element_get_position (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::PositionProperty)->AsTimeSpan ();
}

void
media_element_set_position (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::PositionProperty, Value (value, Type::TIMESPAN));
}

double
media_element_get_volume (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::VolumeProperty)->AsDouble ();
}

void
media_element_set_volume (MediaElement *media, double value)
{
	media->SetValue (MediaElement::VolumeProperty, Value (value));
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
		surface->ref_cnt --;
		if (surface->ref_cnt == 0) {
			g_hash_table_remove (surface_cache, surface->fname);
			g_free (surface->fname);
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
	double progress = downloader->GetValue (Downloader::DownloadProgressProperty)->AsDouble ();
	double current = GetValue (Image::DownloadProgressProperty)->AsDouble ();
	
	SetValue (Image::DownloadProgressProperty, Value (progress));
	
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
	MediaBase::SetSource (downloader, PartName);
}

void
Image::PixbufWrite (void *buf, int32_t offset, int32_t n)
{
	UpdateProgress ();
}

void
Image::DownloaderComplete ()
{
	Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);
	Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);
	char *filename = downloader->GetDownloadedFilePart (part_name);
	
	CleanupSurface ();
	
	if (!filename) {
		/* the download was aborted */
		/* FIXME: should this emit ImageFailed? */
		Invalidate ();
		return;
	}
	
	if (!CreateSurface (filename)) {
		g_free (filename);
		Invalidate ();
		return;
	}
	
	g_free (filename);
	
	if (width == NULL && height == NULL) {
		SetValue (FrameworkElement::WidthProperty, (double) surface->width);
		SetValue (FrameworkElement::HeightProperty, (double) surface->height);
	}
	
	if (width == NULL && height != NULL)
		SetValue (FrameworkElement::WidthProperty, (double) surface->width * height->AsDouble () / (double)surface->height);
	
	if (width != NULL && height == NULL)
		SetValue (FrameworkElement::HeightProperty, (double) surface->height * width->AsDouble () / (double)surface->width);
	
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
Image::CreateSurface (const char *fname)
{
	if (surface) {
		// image surface already created
		return true;
	}

	CleanupPattern ();

	if (!surface_cache)
		surface_cache = g_hash_table_new (g_str_hash, g_str_equal);
	
	if (!(surface = (CachedSurface*)g_hash_table_lookup (surface_cache, fname))) {
		GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
		GdkPixbuf *pixbuf = NULL;
		GError *err = NULL;
		guchar buf[4096];
		ssize_t n;
		char *msg;
		int fd;
		
		if ((fd = open (fname, O_RDONLY)) == -1) {
			msg = g_strdup_printf ("Failed to load image %s: %s", fname, g_strerror (errno));
			Emit (ImageFailedEvent, new ImageErrorEventArgs (msg));
			return false;
		}
		
		do {
			do {
				n = read (fd, buf, sizeof (buf));
			} while (n == -1 && errno == EINTR);
			
			if (n == -1)
				break;
			
			gdk_pixbuf_loader_write (GDK_PIXBUF_LOADER (loader), buf, n, &err);
		} while (n > 0 && !err);
		
		gdk_pixbuf_loader_close (GDK_PIXBUF_LOADER (loader), err ? NULL : &err);
		close (fd);
		
		if (!(pixbuf = gdk_pixbuf_loader_get_pixbuf (GDK_PIXBUF_LOADER (loader)))) {
			if (err && err->message)
				msg = g_strdup_printf ("Failed to load image %s: %s", fname, err->message);
			else
				msg = g_strdup_printf ("Failed to load image %s", fname);
			
			Emit (ImageFailedEvent, new ImageErrorEventArgs (msg));
			
			if (err)
				g_error_free (err);
			
			return false;
		} else if (err) {
			g_error_free (err);
		}
		
		surface = g_new0 (CachedSurface, 1);
		
		surface->ref_cnt = 1;
		surface->fname = g_strdup (fname);
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
		g_hash_table_insert (surface_cache, surface->fname, surface);
	} else {
		surface->ref_cnt++;
	}

	return true;
}

void
Image::size_notify (int64_t size, gpointer data)
{
	// Do something with it?
	// if size == -1, we do not know the size of the file, can happen
	// if the server does not return a Content-Length header
	//printf ("The image size is %lld\n", size);
}

void
Image::pixbuf_write (void *buf, int32_t offset, int32_t n, gpointer data)
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

	Stretch stretch = (Stretch) GetValue (StretchProperty)->AsInt32 ();
	double w = GetValue (WidthProperty)->AsDouble ();
	double h = GetValue (HeightProperty)->AsDouble ();
	
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

void
Image::ComputeBounds ()
{
	Rect box = Rect (0,0,
			 GetValue (WidthProperty)->AsDouble (),
			 GetValue (HeightProperty)->AsDouble ());
								   
	bounds = IntersectBoundsWithClipPath (box, false).Transform (&absolute_xform);
						     
}

Point
Image::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (GetValue (WidthProperty)->AsDouble () * user_xform_origin.x, 
		      GetValue (HeightProperty)->AsDouble () * user_xform_origin.y);
}

cairo_surface_t *
Image::GetCairoSurface ()
{
	return surface ? surface->cairo : NULL;
}

void
Image::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == MediaBase::SourceProperty) {
		DownloaderAbort ();
		
		char *source = args->new_value ? args->new_value->AsString() : NULL;
		
		Downloader *dl = Surface::CreateDownloader (this);
		downloader_open (dl, "GET", source);
		SetSource (dl, "");
		dl->unref ();
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

void
media_init (void)
{
	/* MediaAttribute */
	MediaAttribute::ValueProperty = DependencyObject::Register (Type::MEDIAATTRIBUTE, "Value", Type::STRING);
	
	/* MediaBase */
	MediaBase::SourceProperty = DependencyObject::Register (Type::MEDIABASE, "Source", Type::STRING);
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
