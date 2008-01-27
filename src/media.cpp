/*
 * media.cpp: 
 *
 * Authors:
 *   Jeffrey Stedfast <fejj@novell.com>
 *   Jb Evain <jbevain@novell.com>
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

// still too ugly to be exposed in the header files ;-)
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform, Transform *relative_transform);


// MediaBase

DependencyProperty *MediaBase::SourceProperty;
DependencyProperty *MediaBase::StretchProperty;
DependencyProperty *MediaBase::DownloadProgressProperty;

int MediaBase::DownloadProgressChangedEvent = -1;

MediaBase::MediaBase ()
{
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

// MediaSource

MediaSource::MediaSource (MediaElement *element, const char *source_name, IMediaSource *source)
{
	this->element = element;
	this->source_name = g_strdup (source_name);
	this->source = source;
}

MediaSource::~MediaSource ()
{
	g_free (source_name);
}

const char *
MediaSource::GetSourceName ()
{
	return source_name;
}

IMediaSource *
MediaSource::GetSource ()
{
	return source;
}

MediaPlayer *
MediaSource::GetMediaPlayer ()
{
	g_assert (element != NULL);
	return element->mplayer;
}

bool
MediaSource::Open ()
{
/*	
	if (!OpenInternal ()) {
		media_element_set_can_seek (element, false);
		media_element_set_can_pause (element, false);
		media_element_set_audio_stream_count (element, 0);
		media_element_set_natural_duration (element, Duration (0));
		media_element_set_natural_video_height (element, 0);
		media_element_set_natural_video_width (element, 0);
		
		media_element_set_current_state (element, "Error");
		return false;
	}

	MediaPlayer *mplayer = GetMediaPlayer ();

	media_element_set_can_seek (element, mplayer->CanSeek ());
	media_element_set_can_pause (element, mplayer->CanPause ());
	media_element_set_audio_stream_count (element, mplayer->GetAudioStreamCount ());
	media_element_set_natural_duration (element, Duration (mplayer->Duration () * TIMESPANTICKS_IN_SECOND / 1000));
	media_element_set_natural_video_height (element, mplayer->height);
	media_element_set_natural_video_width (element, mplayer->width);
	
	return true;
*/
	return false;
}

MediaSource *
MediaSource::CreateSource (MediaElement *element, const char *source_name, IMediaSource *source)
{
	if (Playlist::IsPlaylistFile (source)) {
		if (source->GetType () == MediaSourceTypeFile)
			return new Playlist (element, source_name, (FileSource *) source);
		else
			return NULL;
	}
	
	return new SingleMedia (element, source_name, source);
}

// SingleMedia

SingleMedia::SingleMedia (MediaElement *element, const char *source_name, IMediaSource *source)
	: MediaSource (element, source_name, source)
{
	advance_frame_timeout_id = 0;
}

SingleMedia::~SingleMedia ()
{
	Close ();
}

bool
SingleMedia::OpenInternal ()
{
	//if ((element->mplayer->media == NULL || element->mplayer->media->GetSource () == NULL) && !element->mplayer->Initialize (file_name))
	//	return false;
	//return element->mplayer->Open ();
	return false;
}

void
SingleMedia::ClearTimeout ()
{
	if (advance_frame_timeout_id == 0)
		return;

	TimeManager::Instance()->RemoveTimeout (advance_frame_timeout_id);
	advance_frame_timeout_id = 0;
}

void
SingleMedia::Play ()
{
	advance_frame_timeout_id = element->mplayer->Play (media_element_advance_frame, element);
	element->SetState (MediaElement::Playing);
	element->Invalidate ();
}

void
SingleMedia::Pause ()
{
	element->mplayer->Pause ();
	element->SetState (MediaElement::Paused);
	ClearTimeout ();
}

void
SingleMedia::Stop (bool media_ended)
{
	element->mplayer->Stop ();
	element->SetState (MediaElement::Stopped);
	ClearTimeout ();
}

void
SingleMedia::Close ()
{
	element->mplayer->Stop ();
	element->SetState (MediaElement::Closed);
	ClearTimeout ();
}

// MediaElement

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

int MediaElement::BufferingProgressChangedEvent = -1;
int MediaElement::CurrentStateChangedEvent = -1;
int MediaElement::MarkerReachedEvent = -1;
int MediaElement::MediaEndedEvent = -1;
int MediaElement::MediaFailedEvent = -1;
int MediaElement::MediaOpenedEvent = -1;


enum MediaElementFlags {
	Loaded            = (1 << 0),  // set once OnLoaded has been called
	TryOpenOnLoaded   = (1 << 1),  // set if OnLoaded should call TryOpen
	PlayRequested     = (1 << 2),  // set if Play() has been requested prior to being ready
	BufferingMedia    = (1 << 3),  // set if TryOpen() succeeded and we are now buffering
	DisableBuffering  = (1 << 4),  // set if we cannot give useful buffering progress
	DownloadComplete  = (1 << 5),  // set if the download is complete
	UpdatingPosition  = (1 << 6),  // set if we are updating the PositionProperty as opposed to someone else
	RecalculateMatrix = (1 << 7)   // set if the patern matrix needs to be recomputed
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
	MediaElement *element = (MediaElement *) closure->context;
	MediaMarker *marker = closure->marker;
	
	if (marker == NULL)
		return MEDIA_FAIL;
	
	uint64_t pts = (marker->Pts () - closure->media->GetStartTime ()) * 10000;
	
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
	
	if (mplayer == NULL || mplayer->media == NULL || mplayer->media->GetDemuxer () == NULL)
		return;

	media = mplayer->media;
	demuxer = media->GetDemuxer ();

	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		if (demuxer->GetStream (i)->GetType () == MediaTypeMarker) {
			MarkerStream *stream = (MarkerStream *) demuxer->GetStream (i);
			MediaClosure *closure = new MediaClosure ();
			closure->callback = marker_callback;
			closure->context = this;
			closure->media = media;
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
		new_marker->SetValue (TimelineMarker::TimeProperty, Value ((marker->Pts () - media->GetStartTime ()) * 10000, Type::TIMESPAN));
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
			marker->ref ();
			Emit (MarkerReachedEvent, marker);
			marker->unref ();
		}
		
		next = (Collection::Node *) node->next;
		
		if (remove && pts <= to) {
			// Also delete markers we've passed by already
			col->list->Remove (node);
		}
		
		node = next;
	}
}

bool
MediaElement::AdvanceFrame ()
{
	uint64_t position;
	bool advanced;
	
	if (!IsPlaying ())
		return false;
	
	advanced = mplayer->AdvanceFrame ();
	
	position = mplayer->Position () * (TIMESPANTICKS_IN_SECOND / 1000);
	
	if (advanced) {
		//printf ("MediaElement::AdvanceFrame (): advanced, setting position to: %lld\n", position);
		flags |= UpdatingPosition;
		media_element_set_position (this, position);
		flags &= ~UpdatingPosition;
	}
	
	CheckMarkers (previous_position, position);
	
	// Add 1 to avoid the same position to be able to be both
	// beginning and end of a range (otherwise the same marker
	// might raise two events).
	previous_position = position + 1;
	
	if (!advanced) {
		mplayer->Stop ();
		SetState (Stopped);
		Emit (MediaEndedEvent);
	}
	
	return advanced;
}

gboolean
media_element_advance_frame (void *user_data)
{
	MediaElement *media = (MediaElement *) user_data;
	
	return (gboolean) media->AdvanceFrame ();
}

MediaElement::MediaElement ()
{
	streamed_markers = NULL;
	downloaded_file = NULL;
	downloader = NULL;
	part_name = NULL;
	mplayer = NULL;
	media = NULL;
	flags = 0;
	
	Reinitialize ();
	
	mplayer = new MediaPlayer ();
	
	SetValue (MediaElement::AttributesProperty, Value::CreateUnref (new MediaAttributeCollection ()));		
	SetValue (MediaElement::MarkersProperty, Value::CreateUnref (new TimelineMarkerCollection ()));
}

MediaElement::~MediaElement ()
{
	Reinitialize ();
	
	delete mplayer;
}

void
MediaElement::Reinitialize ()
{
	Value *val;
	
	if (mplayer)
		mplayer->Close ();
	
	if (media != NULL) {
		// Media will delete its source upon destruction
		// so clear out our reference if they're the same.
		if (media->GetSource () == downloaded_file)
			downloaded_file = NULL;
		
		delete media;
		media = NULL;
	}
	
	flags = (flags & (Loaded | PlayRequested)) | RecalculateMatrix;
	prev_state = Closed;
	state = Closed;
	
	DownloaderAbort ();
	g_free (part_name);
	part_name = NULL;
	
	if (downloaded_file) {
		delete downloaded_file;
		downloaded_file = NULL;
	}
	
	buffering_start = 0;
	
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
}

void
MediaElement::DownloaderAbort ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
		downloader = NULL;
	}
}

void
MediaElement::MediaOpened (Media *media)
{
	mplayer->Open (media);
	this->media = media;
	
	ReadMarkers ();
	
	media_element_set_can_seek (this, mplayer->CanSeek ());
	media_element_set_can_pause (this, mplayer->CanPause ());
	media_element_set_audio_stream_count (this, mplayer->GetAudioStreamCount ());
	media_element_set_natural_duration (this, Duration (mplayer->Duration () * TIMESPANTICKS_IN_SECOND / 1000));
	media_element_set_natural_video_height (this, mplayer->height);
	media_element_set_natural_video_width (this, mplayer->width);
	
	mplayer->SetMuted (media_element_get_is_muted (this));
	mplayer->SetVolume (media_element_get_volume (this));
	mplayer->SetBalance (media_element_get_balance (this));

	Emit (MediaElement::MediaOpenedEvent);
}

void
MediaElement::MediaFailed ()
{
	media_element_set_can_seek (this, false);
	media_element_set_can_pause (this, false);
	media_element_set_audio_stream_count (this, 0);
	media_element_set_natural_duration (this, Duration (0));
	media_element_set_natural_video_height (this, 0);
	media_element_set_natural_video_width (this, 0);
	
	SetState (MediaElement::Error);
	Emit (MediaElement::MediaFailedEvent);
}

void
MediaElement::ComputeBounds ()
{
	double h = framework_element_get_height (this);
	double w = framework_element_get_width (this);
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->height;
		w = (double) mplayer->width;
	}
	
	Rect box = Rect (0, 0, w, h);
	
	bounds = bounding_rect_for_transformed_rect (&absolute_xform,
						     IntersectBoundsWithClipPath (box, false));
}

Point
MediaElement::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	double h = framework_element_get_height (this);
	double w = framework_element_get_width (this);
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->height;
		w = (double) mplayer->width;
	}
	
	return Point (user_xform_origin.x * w, user_xform_origin.y * h);
}

void
MediaElement::Render (cairo_t *cr, Region *region)
{
	Stretch stretch = media_base_get_stretch (this);
	double h = framework_element_get_height (this);
	double w = framework_element_get_width (this);
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (!(surface = mplayer->GetSurface ()))
		return;
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->height;
		w = (double) mplayer->width;
	}
	
	cairo_save (cr);
	if (!EnableAntiAlias ())
		cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	
	pattern = cairo_pattern_create_for_surface (surface);	
	
	if (flags & RecalculateMatrix) {
		image_brush_compute_pattern_matrix (&matrix, w, h, mplayer->width, mplayer->height, stretch,
						    AlignmentXCenter, AlignmentYCenter, NULL, NULL);
		flags &= ~RecalculateMatrix;
	}
	
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	
	cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);

	cairo_set_matrix (cr, &absolute_xform);

	cairo_new_path (cr);
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_fill (cr);

	cairo_restore (cr);
}


// TODO: Honor BufferTimeProperty and calculate the buffering size from that.
#define BUFFERING_SIZE (5 * 1024 * 1024)


void
MediaElement::UpdateProgress ()
{
	double progress, current;
	bool emit = false;
	
	//printf ("MediaElement::UpdateProgress (). Current state: %s\n", GetStateName (state));
	
	if (downloaded_file != NULL && !IsBuffering () && downloaded_file->IsWaiting ()) {
		// We're waiting for more data, switch to the 'Buffering' state.
		//printf ("MediaElement::UpdateProgress (): Switching to 'Buffering'.\n");
		SetValue (MediaElement::BufferingProgressProperty, Value (0.0));
		buffering_start = downloaded_file->GetPosition ();
		SetState (Buffering);
		mplayer->Pause ();
		emit = true;
	}
	
	// CHECK: if buffering, will DownloadCompletedEvent be emitted?
	
	if (IsBuffering ()) {
		int64_t wait_pos = downloaded_file->GetWaitPosition ();
		int64_t pos = downloaded_file->GetWritePosition ();
		int64_t size = downloaded_file->GetTotalSize ();
		int64_t buffer_size = BUFFERING_SIZE;
		
		if ((buffering_start + buffer_size) < wait_pos) {
			// If the position the stream is waiting for is beyond the default
			// buffer size, use that position to calculate the buffer size.
			buffer_size = wait_pos - buffering_start;
		} else if (size != -1 && (buffering_start + buffer_size) > size) {
			// there's less than BUFFERING_SIZE data left to buffer
			buffer_size = size - buffering_start;
		}
		
		current = GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
		if ((progress = ((double) (pos - buffering_start)) / buffer_size) > 1.0)
			progress = 1.0;
		
		// Emit the event if it's 100%, or a change of at least 0.05%
		if (emit || progress == 1.0 || (progress - current) >= 0.0005) {
			SetValue (MediaElement::BufferingProgressProperty, Value (progress));
			Emit (MediaElement::BufferingProgressChangedEvent);
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
			Emit (MediaBase::DownloadProgressChangedEvent);
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
		printf ("MediaElement::SetState (%d) state is not valid.\n", state);
		return;
	}
	
	//printf ("MediaElement::SetState (%d): New state: %s\n", state, state_name);
	
	prev_state = this->state;
	this->state = state;
	
	media_element_set_current_state (this, name);		
}

void 
MediaElement::DataWrite (void *buf, int32_t offset, int32_t n)
{
	//printf ("MediaElement::DataWrite (%p, %d, %d)\n", buf, offset, n);
	
	if (downloaded_file != NULL) {
		downloaded_file->Write (buf, (int64_t) offset, n);
		
 		// FIXME: How much do we actually have to download in order to try to open the file?
		if (!(flags & BufferingMedia) && offset > 16384 && (part_name == NULL || part_name[0] == 0))
			TryOpen ();
	}
	
	// Delay the propogating progress 1.0 until
	// the downloader has notified us it is done.
	double progress = downloader->GetValue (Downloader::DownloadProgressProperty)->AsDouble ();
	
	if (progress < 1.0)
		UpdateProgress ();
}

void 
MediaElement::data_write (void *buf, int32_t offset, int32_t n, gpointer data)
{
	((MediaElement *) data)->DataWrite (buf, offset, n);
}

void
MediaElement::size_notify (int64_t size, gpointer data)
{
	MediaElement *element = (MediaElement *) data;
	
	if (element->downloaded_file != NULL)
		element->downloaded_file->NotifySize (size);
}

void
MediaElement::downloader_complete (EventObject *sender, gpointer calldata, gpointer closure)
{
	((MediaElement *) closure)->DownloaderComplete ();
}

void
MediaElement::BufferingComplete ()
{
	if (state != Buffering) {
		printf ("MediaElement::BufferingComplete (): current state is invalid ('%s'), should only be 'Buffering'\n",
			GetStateName (state));
		return;
	}
	
	switch (prev_state) {
	case Opening: // Start playback
		if ((flags & PlayRequested) || media_element_get_auto_play (this))
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
		printf ("MediaElement::BufferingComplete (): previous state is invalid ('%s').\n", GetStateName (prev_state));
		return;
	}
}

void
MediaElement::TryOpen ()
{
	switch (state) {
	case Closed:
	case Error:
		printf ("MediaElement::TryOpen (): Current state (%s) is invalid.\n", GetStateName (state)); 
		// Should not happen
		return;
	case Playing:
	case Paused:
	case Stopped:
	case Buffering:
		// I don't think this should happen either
		printf ("MediaElement::TryOpen (): Current state (%s) was unexpected.\n", GetStateName (state));
		// Media is already open.
		// There's nothing to do here.
		return;
	case Opening:
		// Try to open it now
		break;
	default:
		printf ("MediaElement::TryOpen (): Unknown state: %d\n", state);
		return;
	}
	
	if (!(flags & Loaded)) {
		//printf ("MediaElement::TryOpen (): We're not loaded, so wait until then.\n");
		flags |= TryOpenOnLoaded;
		return;
	}
	
	if (flags & DownloadComplete) {
		char *filename = downloader_get_response_file (downloader, part_name);
		Media *media = new Media ();
		IMediaSource *source;
		
		source = new FileSource (media, filename);
		g_free (filename);
		
		if (MEDIA_SUCCEEDED (source->Initialize ())) {
			if (MEDIA_SUCCEEDED (media->Open (source))) {
				MediaOpened (media);
				SetState (Buffering);
				
				if ((flags & PlayRequested) || media_element_get_auto_play (this))
					Play ();
				else
					Pause ();
				
				Invalidate ();
			} else {
				MediaFailed ();
				delete source;
				delete media;
			}
		} else {
			MediaFailed ();
			delete source;
			delete media;
		}
		
		// If we have a downloaded file ourselves, delete it, we no longer need it.
		if (downloaded_file) {
			delete downloaded_file;
			downloaded_file = NULL;
		}
	} else if (part_name != NULL && part_name[0] != 0) {
		// PartName is set, we can't buffer, download the entire file.
	} else if (!(flags & BufferingMedia)) {
		Media *media = new Media ();
		
		if (MEDIA_SUCCEEDED (media->Open (downloaded_file))) {
			//printf ("MediaElement::TryOpen (): download is not complete, but media was "
			//	"opened successfully and we'll now start buffering.\n");
			flags |= BufferingMedia;
			MediaOpened (media);
			SetState (Buffering);
		} else {
			// Seek back to the beginning of the file
			downloaded_file->Seek (0, SEEK_SET);
			delete media;
		}
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
		printf ("MediaElement::DownloaderComplete (): Current state (%d) is invalid.\n", state);
		// Should not happen
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
		if ((flags & PlayRequested) || media_element_get_auto_play (this))
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
		printf ("MediaElement::DownloaderComplete (): Unknown state: %d\n", state);
		return;
	}
	
}

void
MediaElement::SetSource (DependencyObject *dl, const char *PartName)
{
	g_return_if_fail (dl->GetObjectType () == Type::DOWNLOADER);
	
	Reinitialize ();
	
	downloader = (Downloader *) dl;
	downloader->ref ();
	part_name = g_strdup (PartName);
	
	SetState (Opening);
	
	if (downloader->Started ()) {
		flags |= DisableBuffering;
		
		if (downloader->Completed ())
			flags |= DownloadComplete;
		
		TryOpen ();
	} else {
		downloaded_file = new ProgressiveSource (mplayer->media);
		
		// FIXME: error check Initialize()
		downloaded_file->Initialize ();
		
		downloader->SetWriteFunc (data_write, size_notify, this);
	}
	
	if (!(flags & DownloadComplete))
		downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
	
	// This is what actually triggers the download
	if (downloaded_file != NULL)
		downloader->Send ();
	
	Invalidate ();
}

void
MediaElement::Pause ()
{
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
			mplayer->Pause ();
			SetState (Paused);
		}
		break;
	}
}

void
MediaElement::Play ()
{
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
		mplayer->Play (media_element_advance_frame, this);
		flags &= ~PlayRequested;
		SetState (Playing);
		break;
	}
}

void
MediaElement::Stop ()
{
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
		mplayer->Stop ();
		SetState (Stopped);
		Invalidate ();
		break;
	}
}

Value *
MediaElement::GetValue (DependencyProperty *prop)
{
	if (prop == MediaElement::PositionProperty) {
		uint64_t position = mplayer->Position ();
		Value v = Value (position * TIMESPANTICKS_IN_SECOND / 1000, Type::TIMESPAN);
		
		flags |= UpdatingPosition;
		SetValue (prop, &v);
		flags &= ~UpdatingPosition;
	}
	
	return MediaBase::GetValue (prop);
}

void
MediaElement::SetValue (DependencyProperty *prop, Value value)
{
	MediaBase::SetValue (prop, value);
}

void
MediaElement::SetValue (DependencyProperty *prop, Value *value)
{
	Value v;
	
	if (prop == MediaElement::PositionProperty && !(flags & UpdatingPosition)) {
		// Some outside source is updating the Position
		// property which means we need to Seek
		
		switch (state) {
		case Opening:
		case Closed:
		case Error:
			return;
		case Buffering:
		case Playing: // Play
			break;
		case Paused:
		case Stopped: // Paused
			break;
		}
		
		// Buffering, Playing: playing
		// Paused, Stopped: paused
		
		Duration *duration = media_element_get_natural_duration (this);
		TimeSpan position = value->AsTimeSpan ();
		
		if (duration->HasTimeSpan () && position > duration->GetTimeSpan ())
			position = duration->GetTimeSpan ();
		else if (position < 0)
			position = 0;
		
		// position is in ticks, while mplayer expects time in milliseconds.
		mplayer->Seek (position * 1000 / TIMESPANTICKS_IN_SECOND);
		Invalidate ();
		
		if (IsStopped ())
			SetState (Paused);
		
		if (position != value->AsTimeSpan ()) {
			v = Value (position, Type::TIMESPAN);
			MediaBase::SetValue (prop, &v);
			return;
		}
	}
	
	MediaBase::SetValue (prop, value);
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
MediaElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == MediaBase::SourceProperty) {
		char *uri = media_base_get_source (this);
		
		if (uri && *uri) {
			Downloader *dl = Surface::CreateDownloader (this);
			downloader_open (dl, "GET", uri);
			SetSource (dl, "");
			dl->unref ();
		} else {
			//printf ("trying to set media source to empty\n");
			DownloaderAbort ();
		}
		
		flags |= RecalculateMatrix;
	} else if (prop == MediaElement::AudioStreamCountProperty) {
		// read-only property
	} else if (prop == MediaElement::AudioStreamIndexProperty) {
		// FIXME: set the audio stream index
	} else if (prop == MediaElement::AutoPlayProperty) {
		// no state to change
		//printf ("AutoPlay set to %s\n", media_element_get_auto_play (this) ? "true" : "false");
	} else if (prop == MediaElement::BalanceProperty) {
		mplayer->SetBalance (media_element_get_balance (this));
	} else if (prop == MediaElement::BufferingProgressProperty) {
		Emit (BufferingProgressChangedEvent);
	} else if (prop == MediaElement::BufferingTimeProperty) {
		// FIXME: set the buffering time
	} else if (prop == MediaElement::CanPauseProperty) {
		// read-only property
	} else if (prop == MediaElement::CanSeekProperty) {
		// read-only property
	} else if (prop == MediaElement::CurrentStateProperty) {
		Emit (CurrentStateChangedEvent);
	} else if (prop == MediaElement::IsMutedProperty) {
		mplayer->SetMuted (media_element_get_is_muted (this));
	} else if (prop == MediaElement::MarkersProperty) {
		// FIXME: keep refs to these?
	} else if (prop == MediaElement::NaturalDurationProperty) {
		// read-only property
	} else if (prop == MediaElement::NaturalVideoHeightProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (prop == MediaElement::NaturalVideoWidthProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (prop == MediaElement::PositionProperty) {
		if (IsPlaying() && mplayer->HasVideo ())
			Invalidate ();
	} else if (prop == MediaElement::VolumeProperty) {
		mplayer->SetVolume (media_element_get_volume (this));
	}
	
	if (prop->type == Type::MEDIAELEMENT) {
		NotifyAttachersOfPropertyChange (prop);
	} else {
		// propagate to parent class
		MediaBase::OnPropertyChanged (prop);
		flags |= RecalculateMatrix;
	}
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
media_element_set_source (MediaElement *media, DependencyObject *Downloader, const char *PartName)
{
	media->SetSource (Downloader, PartName);
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
	int idx = (int) media->GetValue (MediaElement::AudioStreamIndexProperty)->AsInt32 ();
	
	return idx < 0 ? -1 : idx;
}

void
media_element_set_audio_stream_index (MediaElement *media, int value)
{
	if (value >= 0)
		media->SetValue (MediaElement::AudioStreamIndexProperty, Value (value));
	else
		media->SetValue (MediaElement::AudioStreamIndexProperty, Value (-1));
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

int Image::ImageFailedEvent = -1;

Image::Image ()
  : create_xlib_surface (true),
    downloader (NULL),
    surface (NULL),
    part_name(NULL),
    pattern (NULL),
    brush (NULL)
{
}

Image::~Image ()
{
	DownloaderAbort ();
	CleanupSurface ();
	g_free (part_name);
}

void
Image::DownloaderAbort ()
{
	if (downloader){
		downloader_abort (downloader);
		downloader->unref ();
		downloader = NULL;
	}
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
		Emit (MediaBase::DownloadProgressChangedEvent);
}

void
Image::SetSource (DependencyObject *dl, const char *PartName)
{
	g_return_if_fail (dl->GetObjectType() == Type::DOWNLOADER);

	dl->ref ();

	if (downloader)
		downloader->unref ();

	part_name = g_strdup (PartName);

	CleanupSurface ();
	Invalidate (); 

	downloader = (Downloader*) dl;

	downloader->AddHandler (Downloader::CompletedEvent, downloader_complete, this);
	downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, this);

	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
		
		UpdateProgress ();
	} else {
		downloader->SetWriteFunc (pixbuf_write, size_notify, this);
		
		// This is what actually triggers the download
		downloader->Send ();
	}
}

void
Image::PixbufWrite (void *buf, int32_t offset, int32_t n)
{
	UpdateProgress ();
}

void
Image::DownloaderComplete ()
{
	char *file = downloader_get_response_file (downloader, part_name);
	/* the download was aborted */
	if (!file) {
		/* XXX should this emit ImageFailed? */
		return;
	}

	bool ok = CreateSurface (file);
	g_free (file);
	if (!ok)
		return;

	Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);

	if (width == NULL && height == NULL) {
		SetValue (FrameworkElement::WidthProperty, (double) surface->width);
		SetValue (FrameworkElement::HeightProperty, (double) surface->height);
	}
	if (width == NULL && height != NULL)
		SetValue (FrameworkElement::WidthProperty, (double) surface->width * height->AsDouble () / (double)surface->height);
	if (width != NULL && height == NULL)
		SetValue (FrameworkElement::HeightProperty, (double) surface->height * width->AsDouble () / (double)surface->width);
		
	if (brush)
		brush->OnPropertyChanged (ImageBrush::DownloadProgressProperty);
	else 
		Invalidate ();
}

void
Image::DownloaderFailed (const char *msg)
{
	ImageErrorEventArgs *ea = new ImageErrorEventArgs (msg);

	Emit (ImageFailedEvent, ea);

	delete ea;
}


#ifdef WORDS_BIGENDIAN
#define set_pixel_bgra(pixel,index,b,g,r,a) do { \
		((unsigned char *)(pixel))[index]   = a; \
		((unsigned char *)(pixel))[index+1] = r; \
		((unsigned char *)(pixel))[index+2] = g; \
		((unsigned char *)(pixel))[index+3] = b; \
	} while (0)
#else
#define set_pixel_bgra(pixel,index,b,g,r,a) do { \
		((unsigned char *)(pixel))[index]   = b; \
		((unsigned char *)(pixel))[index+1] = g; \
		((unsigned char *)(pixel))[index+2] = r; \
		((unsigned char *)(pixel))[index+3] = a; \
	} while (0)
#endif
#define get_pixel_bgra(color, b, g, r, a) do { \
		a = ((color & 0xff000000) >> 24); \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} while(0)

#include "alpha-premul-table.inc"


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

	surface = (CachedSurface*)g_hash_table_lookup (surface_cache, fname);
	if (surface) {
		surface->ref_cnt ++;
	}
	else {
		GError *error = NULL;

		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (fname, &error);
		if (!pixbuf){
			char *msg;

			if (error && error->message)
				msg = g_strdup_printf ("Failed to load image %s: %s", fname, error->message);
			else
				msg = g_strdup_printf ("Failed to load image %s", fname);
		  
			ImageErrorEventArgs *ea = new ImageErrorEventArgs (msg);

			Emit (ImageFailedEvent, ea);

			delete ea;
			return false;
		}

		surface = g_new0 (CachedSurface, 1);

		surface->ref_cnt = 1;
		surface->fname = g_strdup (fname);
		surface->height = gdk_pixbuf_get_height (pixbuf);
		surface->width = gdk_pixbuf_get_width (pixbuf);
		
		bool has_alpha = gdk_pixbuf_get_n_channels (pixbuf) == 4;

		if (has_alpha) {
			g_object_ref (pixbuf);
		} else {
			/* gdk-pixbuf packs its pixel data into 24 bits for
			   rgb, instead of 32 with an unused byte for alpha,
			   like cairo expects */

			GdkPixbuf *pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, surface->width, surface->height);
			gdk_pixbuf_copy_area (pixbuf,
					      0, 0, surface->width, surface->height,
					      pb,
					      0, 0);
			g_object_unref (pixbuf);
			pixbuf = pb;
		}

		guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
		guchar *p;
		for (int y = 0; y < surface->height; y ++) {
			p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
			for (int x = 0; x < surface->width; x ++) {
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

		surface->backing_pixbuf = pixbuf;
		surface->cairo = cairo_image_surface_create_for_data (pb_pixels,
#if USE_OPT_RGB24
								      has_alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
#else
								      CAIRO_FORMAT_ARGB32,
#endif
								      surface->width,
								      surface->height,
								      gdk_pixbuf_get_rowstride (pixbuf));

		g_hash_table_insert (surface_cache, surface->fname, surface);
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
Image::downloader_complete (EventObject *sender, gpointer calldata, gpointer closure)
{
	Image *image = (Image*)closure;
	image->DownloaderComplete ();
}

void
Image::downloader_failed (EventObject *sender, gpointer calldata, gpointer closure)
{
	((Image*)closure)->DownloaderFailed ((const char *)calldata);
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
		g_object_unref (surface->backing_pixbuf);
		surface->backing_pixbuf = NULL;
		surface->cairo = xlib_surface;
	}

	cairo_save (cr);

	Stretch stretch = media_base_get_stretch (this);
	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

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
			 framework_element_get_width (this),
			 framework_element_get_height (this));
								   
	bounds = bounding_rect_for_transformed_rect (&absolute_xform,
						     IntersectBoundsWithClipPath (box, false));
}

Point
Image::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		      framework_element_get_height (this) * user_xform_origin.y);
}

cairo_surface_t *
Image::GetCairoSurface ()
{
	return surface ? surface->cairo : NULL;
}

void
Image::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == MediaBase::SourceProperty) {
		DownloaderAbort ();
		
		char *source = media_base_get_source (this);
		
		Downloader *dl = Surface::CreateDownloader (this);
		downloader_open (dl, "GET", source);
		SetSource (dl, "");
		dl->unref ();
	}

	if (prop->type != Type::IMAGE) {
		MediaBase::OnPropertyChanged (prop);
		return;
	}

	// we need to notify attachees if our DownloadProgress changed.
	NotifyAttachersOfPropertyChange (prop);
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
image_set_source (Image *img, DependencyObject *Downloader, const char *PartName)
{
	img->SetSource (Downloader, PartName);
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
	MediaElement::AudioStreamCountProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AudioStreamCount", new Value (0));
	MediaElement::AudioStreamIndexProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AudioStreamIndex", new Value (-1));
	MediaElement::AutoPlayProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AutoPlay", new Value (true));
	MediaElement::BalanceProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Balance", new Value (0.0));
	MediaElement::BufferingProgressProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingProgress", new Value (0.0));
	MediaElement::BufferingTimeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingTime", Type::TIMESPAN);
	MediaElement::CanPauseProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CanPause", new Value (false));
	MediaElement::CanSeekProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CanSeek", new Value (false));
	MediaElement::CurrentStateProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CurrentState", Type::STRING);
	MediaElement::IsMutedProperty = DependencyObject::Register (Type::MEDIAELEMENT, "IsMuted", new Value (false));
	MediaElement::MarkersProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Markers", Type::TIMELINEMARKER_COLLECTION);
	MediaElement::NaturalDurationProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalDuration", new Value(Duration::FromSeconds (0)));
	MediaElement::NaturalVideoHeightProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoHeight", new Value (0.0));
	MediaElement::NaturalVideoWidthProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoWidth", new Value (0.0));
	MediaElement::PositionProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Position", Type::TIMESPAN);
	MediaElement::VolumeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Volume", new Value (0.5));

	/* lookup events */
	Type *t = Type::Find (Type::MEDIABASE);
	MediaBase::DownloadProgressChangedEvent = t->LookupEvent ("DownloadProgressChanged");
	
	t = Type::Find (Type::MEDIAELEMENT);
	MediaElement::BufferingProgressChangedEvent = t->LookupEvent ("BufferingProgressChanged");
	MediaElement::CurrentStateChangedEvent = t->LookupEvent ("CurrentStateChanged");
	MediaElement::MarkerReachedEvent = t->LookupEvent ("MarkerReached");
	MediaElement::MediaEndedEvent = t->LookupEvent ("MediaEnded");
	MediaElement::MediaFailedEvent = t->LookupEvent ("MediaFailed");
	MediaElement::MediaOpenedEvent = t->LookupEvent ("MediaOpened");

	t = Type::Find (Type::IMAGE);
	Image::ImageFailedEvent = t->LookupEvent ("ImageFailed");
	
	Media::Initialize ();
}
