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

#include "geometry.h"
#include "runtime.h"
#include "media.h"
#include "downloader.h"
#include "playlist.h"
#include "pipeline.h"
#include "pipeline-asf.h"
#include "pipeline-ui.h"
#include "mediaelement.h"
#include "debug.h"
#include "deployment.h"
#include "mediaplayer.h"
#include "timeline.h"
#include "timemanager.h"

/*
 * MarkerNode
 */
class MarkerNode : public List::Node {
 public:
	TimelineMarker *marker;
	
	MarkerNode (TimelineMarker *marker) { this->marker = marker; marker->ref (); }
	virtual ~MarkerNode () { marker->unref (); }
};

/*
 * MediaElementFlags
 */
 
enum MediaElementFlags {
	PlayRequested       = (1 << 2),  // set if Play() has been requested prior to being ready
	BufferingFailed     = (1 << 3),  // set if TryOpen failed to buffer the media.
	DisableBuffering    = (1 << 4),  // set if we cannot give useful buffering progress
	RecalculateMatrix   = (1 << 7),  // set if the patern matrix needs to be recomputed
	MediaOpenedEmitted  = (1 << 9),  // set if MediaOpened has been emitted.
	MissingCodecs       = (1 << 11), // set if we have no video codecs
	AutoPlayed          = (1 << 12), // set if we've autoplayed
	UpdatingSizeFromMedia = (1 << 13),
	UseMediaHeight =        (1 << 14),
	UseMediaWidth =         (1 << 15),
};

/*
 * MediaElement
 */

MediaElement::MediaElement ()
{
	SetObjectType (Type::MEDIAELEMENT);
	
	streamed_markers = NULL;
	marker_closure = NULL;
	mplayer = NULL;
	playlist = NULL;
	flags = UseMediaWidth | UseMediaHeight;
		
	marker_timeout = 0;
	mplayer = NULL;
	
	Reinitialize ();
	
	providers [PropertyPrecedence_DynamicValue] = new MediaElementPropertyValueProvider (this);
	
	// Note: BufferingTime and Position need to be set in the ctor
	// so that ReadLocalValue() will get these values.
	SetValue (MediaElement::BufferingTimeProperty, Value (TimeSpan_FromSeconds (5), Type::TIMESPAN));
	SetValue (MediaElement::PositionProperty, Value (TimeSpan_FromSeconds (0), Type::TIMESPAN));
	
	GetDeployment ()->AddHandler (Deployment::ShuttingDownEvent, ShuttingDownCallback, this);
}

void
MediaElement::Dispose ()
{
	LOG_MEDIAELEMENT ("MediaElement::Dispose ()\n");
	VERIFY_MAIN_THREAD;
	
	GetDeployment ()->RemoveHandler (Deployment::ShuttingDownEvent, ShuttingDownCallback, this);

	Reinitialize ();
	FrameworkElement::Dispose ();
}

void
MediaElement::ShuttingDownHandler (Deployment *sender, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::ShuttingDownHandler ()\n");
	
	Reinitialize ();
}

const char *
MediaElement::GetStateName (MediaState state)
{
	return enums_int_to_str ("MediaState", state);
}

MediaResult
MediaElement::AddStreamedMarkerCallback (MediaClosure *c)
{
	MediaMarkerFoundClosure *closure = (MediaMarkerFoundClosure *) c;
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaMarker *mmarker = closure->GetMarker ();
	
	// Thread-safe
	
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

void
MediaElement::AddStreamedMarker (TimelineMarker *marker)
{	
	LOG_MEDIAELEMENT ("MediaElement::AddStreamedMarker (): got marker %s, %s, %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms\n",
			  marker->GetText (), marker->GetType (), marker->GetTime (),
			  MilliSeconds_FromPts (marker->GetTime ()));
	
	// thread-safe
	
	mutex.Lock ();
	if (streamed_markers == NULL)
		streamed_markers = new TimelineMarkerCollection ();
	streamed_markers->Add (marker);
	mutex.Unlock ();
}

void
MediaElement::ReadMarkers (Media *media, IMediaDemuxer *demuxer)
{
	LOG_MEDIAELEMENT ("MediaElement::ReadMarkers ()\n");
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (demuxer != NULL);
	g_return_if_fail (media != NULL);
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		if (demuxer->GetStream (i)->GetType () == MediaTypeMarker) {
			MarkerStream *stream = (MarkerStream *) demuxer->GetStream (i);
			
			if (marker_closure == NULL)
				marker_closure = new MediaMarkerFoundClosure (media, AddStreamedMarkerCallback, this);
			
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

gboolean
MediaElement::MarkerTimeout (gpointer context)
{
	VERIFY_MAIN_THREAD;
	((MediaElement *) context)->SetCurrentDeployment ();
	((MediaElement *) context)->CheckMarkers ();
	return TRUE;
}

void
MediaElement::SetMarkerTimeout (bool start)
{
	TimeManager *tm;
	Surface *surface;
	
	VERIFY_MAIN_THREAD;
	
	surface = GetDeployment ()->GetSurface ();
	
	if (surface == NULL)
		return;
	
	tm = surface->GetTimeManager ();
	
	g_return_if_fail (tm != NULL);
	
	if (start) {
		if (marker_timeout == 0) {
			marker_timeout = tm->AddTimeout (G_PRIORITY_DEFAULT, 33, MarkerTimeout, this);
			ref (); // add a ref to self
		}
	} else { // stop
		if (marker_timeout != 0) {
			tm->RemoveTimeout (marker_timeout);
			marker_timeout = 0;
			unref (); // unref self
		}
	}
}

void
MediaElement::CheckMarkers ()
{
	guint64 current_position = GetPosition ();
	
	LOG_MARKERS_EX ("MediaElement::CheckMarkers () current position: %" G_GUINT64_FORMAT ", previous position: %" G_GUINT64_FORMAT ")\n", current_position, previous_position);
	VERIFY_MAIN_THREAD;
	
	if (current_position > previous_position && seek_to_position == -1) {
		guint64 tmp = previous_position;
		// We need to set previous_position before calling CheckMarkers, 
		// as CheckMarkers may end up emitting events, causing seeks
		// which will change previous_position.
		previous_position = current_position; 
		CheckMarkers (tmp, current_position);
	}
}

void
MediaElement::CheckMarkers (guint64 from, guint64 to)
{
	TimelineMarkerCollection *markers;
	
	LOG_MARKERS_EX ("MediaElement::CheckMarkers (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", from, to);
	VERIFY_MAIN_THREAD;
	
	if (from == to) {
		LOG_MARKERS ("MediaElement::CheckMarkers (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT "). from == to\n", from, to);
		return;
	}
	
	if (!(markers = GetMarkers ())) {
		LOG_MARKERS ("MediaElement::CheckMarkers (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT "). No markers\n", from, to);
		return;
	}
	
	if (from > to) {
		// if from > to we've seeked backwards (last played position is after this one)
		LOG_MARKERS ("MediaElement::CheckMarkers (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT "). from > to (diff: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms).\n", from, to, from - to, MilliSeconds_FromPts (from - to));
		return;
	}
	
	CheckMarkers (from, to, markers, false);
	CheckMarkers (from, to, streamed_markers, true);	
}

void
MediaElement::CheckMarkers (guint64 from, guint64 to, TimelineMarkerCollection *markers, bool remove)
{
	TimelineMarker *marker;
	ArrayList emit_list;
	Value *val = NULL;
	guint64 pts;
	bool emit;
	
	LOG_MARKERS ("MediaElement::CheckMarkers (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ", %p, %i). count: %i\n", from, to, markers, remove, markers ? markers->GetCount () : -1);
	VERIFY_MAIN_THREAD;
	
	// We might want to use a more intelligent algorithm here, 
	// this code only loops through all markers on every frame.
	
	mutex.Lock (); // We lock here since markers might be streamed_markers, which require locking
	
	if (markers != NULL) {
		for (int i = 0; i < markers->GetCount (); i++) {
			marker = markers->GetValueAt (i)->AsTimelineMarker ();
			
			if (!(val = marker->GetValue (TimelineMarker::TimeProperty)))
				break;
			
			pts = (guint64) val->AsTimeSpan ();
			
			LOG_MARKERS_EX ("MediaElement::CheckMarkers (%llu, %llu): Checking pts: %llu, enqueued %i elements\n", from, to, pts, emit_list.GetCount ());
			
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
				marker->ref ();
				emit_list.Add (marker);
				
				LOG_MARKERS ("MediaElement::CheckMarkers (%llu, %llu): Emitting: Text = %s, Type = %s, Time = %llu = %llu ms, count: %in",
					     from, to, marker->GetText (), marker->GetType (), marker->GetTime (), MilliSeconds_FromPts (marker->GetTime ()), emit_list.GetCount ());
			}
			
			if (remove && (pts <= to || emit)) {
				// Also delete markers we've passed by already
				markers->RemoveAt (i);
				i--;
			}
		}
	}
	
	mutex.Unlock ();
	
	// We need to emit with the mutex unlocked.
	
	for (int i = 0; i < emit_list.GetCount (); i++) {
		marker = (TimelineMarker *) emit_list [i];
		Emit (MarkerReachedEvent, new MarkerReachedEventArgs (marker));
		marker->unref ();
	}
}

void
MediaElement::SetSurface (Surface *s)
{
	VERIFY_MAIN_THREAD;
	
	if (GetSurface() == s)
		return;
	
	if (mplayer)
		mplayer->SetSurface (s);
	
	if (!SetSurfaceLock ())
		return;
	FrameworkElement::SetSurface (s);
	SetSurfaceUnlock ();
}

void
MediaElement::Reinitialize ()
{
	TimelineMarkerCollection *markers;
	MediaAttributeCollection *attrs;
	
	LOG_MEDIAELEMENT ("MediaElement::Reinitialize ()\n");
	VERIFY_MAIN_THREAD;
	
	if (mplayer) {
		mplayer->Dispose ();
		mplayer->unref ();
		mplayer = NULL;
	}
		
	if (marker_closure) {
		marker_closure->Dispose ();
		marker_closure->unref ();
		marker_closure = NULL;
	}

	if (playlist != NULL) {
		playlist->Dispose ();
		playlist->unref ();
		playlist = NULL;
	}
	
	flags &= (PlayRequested | UseMediaHeight | UseMediaWidth);
	flags |= RecalculateMatrix;
	
	prev_state = MediaStateClosed;
	state = MediaStateClosed;
	
	last_played_pts = 0;
	first_pts = G_MAXUINT64;
	seek_to_position = -1;
	buffering_mode = 0;
	
	mutex.Lock ();
	if (streamed_markers) {
		streamed_markers->unref ();
		streamed_markers = NULL;
	}
	mutex.Unlock ();
	
	previous_position = 0;
	
	SetMarkerTimeout (false);
	if ((markers = GetMarkers ()))
		markers->Clear ();
	
	if ((attrs = GetAttributes ()))
		attrs->Clear ();
	
	cairo_matrix_init_identity (&matrix);
}

bool
MediaElement::IsMissingCodecs ()
{
	VERIFY_MAIN_THREAD;
	return flags & MissingCodecs;
}

Point
MediaElement::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	double h = GetActualHeight();
	double w = GetActualWidth ();
	
	if (w == 0.0 && h == 0.0 && mplayer != NULL) {
		h = (double) mplayer->GetVideoHeight ();
		w = (double) mplayer->GetVideoWidth ();
	}
	
	return Point (user_xform_origin->x * w, user_xform_origin->y * h);
}

Size
MediaElement::MeasureOverride (Size availableSize)
{
	Size desired = availableSize;
	Rect shape_bounds = Rect ();
	double sx = 0.0;
	double sy = 0.0;

	if (mplayer)
		shape_bounds = Rect (0,0,
				     mplayer->GetVideoWidth (),
				     mplayer->GetVideoHeight ());

	if (GetStretch () == StretchNone)
		return desired.Min (shape_bounds.width, shape_bounds.height);

	/* don't stretch to infinite size */
	if (isinf (desired.width))
		desired.width = shape_bounds.width;
	if (isinf (desired.height))
		desired.height = shape_bounds.height;
	
	/* compute the scaling */
	if (shape_bounds.width > 0)
		sx = desired.width / shape_bounds.width;
	if (shape_bounds.height > 0)
		sy = desired.height / shape_bounds.height;

	switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	default:
		break;
	}

	desired = desired.Min (shape_bounds.width * sx, shape_bounds.height * sy);

	return desired;
}

Size
MediaElement::ArrangeOverride (Size finalSize)
{
	Size arranged = finalSize;
	Rect shape_bounds = Rect ();
	double sx = 1.0;
	double sy = 1.0;


	if (mplayer)
		shape_bounds = Rect (0, 0, 
				     mplayer->GetVideoWidth (), 
				     mplayer->GetVideoHeight ());

	if (GetStretch () == StretchNone) {
	        arranged = Size (shape_bounds.x + shape_bounds.width,
				 shape_bounds.y + shape_bounds.height);

		if (GetHorizontalAlignment () == HorizontalAlignmentStretch)
			arranged.width = MAX (arranged.width, finalSize.width);

		if (GetVerticalAlignment () == VerticalAlignmentStretch)
			arranged.height = MAX (arranged.height, finalSize.height);

		return arranged;
	}


	/* compute the scaling */
	if (shape_bounds.width == 0)
		shape_bounds.width = arranged.width;
	if (shape_bounds.height == 0)
		shape_bounds.height = arranged.height;

	if (shape_bounds.width != arranged.width)
		sx = arranged.width / shape_bounds.width;
	if (shape_bounds.height != arranged.height)
		sy = arranged.height / shape_bounds.height;

	switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	default:
		break;
	}

	arranged = Size (shape_bounds.width * sx, shape_bounds.height * sy);

	return arranged;
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
MediaElement::Render (cairo_t *cr, Region *region, bool path_only)
{
	Stretch stretch = GetStretch ();
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (!mplayer || !(surface = mplayer->GetCairoSurface ()))
		return;
	
	cairo_save (cr);
	
	cairo_set_matrix (cr, &absolute_xform);
	
	// if we're opaque, we can likely do this and hopefully get a
	// speed up since the server won't have to blend.
	//cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_new_path (cr);
	
	Rect paint = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	Rect video = Rect (0, 0, mplayer->GetVideoWidth (), mplayer->GetVideoHeight ());

	/*
	if (absolute_xform.xy == 0 && absolute_xform.yx == 0) {
		//cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		cairo_matrix_t inv = absolute_xform;
		cairo_matrix_invert (&inv);
		paint = paint.Transform (&absolute_xform);
		paint = paint.RoundIn ();
		paint = paint.Transform (&inv);
	}
	*/

	image_brush_compute_pattern_matrix (&matrix, 
					    paint.width, paint.height, 
					    video.width, video.height,
					    stretch, AlignmentXCenter, AlignmentYCenter, NULL, NULL);
	
	
	pattern = cairo_pattern_create_for_surface (surface);	
	
	
	cairo_pattern_set_matrix (pattern, &matrix);
		
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	
	if (IsPlaying ())
		cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);

	Geometry *clip = LayoutInformation::GetLayoutClip (this);
	if (clip) {
		clip->Draw (cr);
		cairo_clip (cr);
	}	

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
	Media *media;
	
	VERIFY_MAIN_THREAD;

	buffering_time = TimeSpan_ToPts (GetBufferingTime ());

	if (buffering_time == 0)
		return 1.0;

	g_return_val_if_fail (playlist != NULL, 0.0);

	media = playlist->GetMedia ();

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
	Media *media;
	
	VERIFY_MAIN_THREAD;

	buffering_time = TimeSpan_ToPts (GetBufferingTime ());
	position_pts = TimeSpan_ToPts (GetPosition ());
	
	if (buffering_time == 0)
		return 1.0;

	g_return_val_if_fail (playlist != NULL, 0.0);

	media = playlist->GetMedia ();
	
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

	LOG_MEDIAELEMENT_EX ("MediaElement::CalculateBufferingProgress () buffering mode: %i, result: %.2f, buffering time: %" G_GUINT64_FORMAT " ms, position: %" G_GUINT64_FORMAT " ms, last available pts: %" G_GUINT64_FORMAT " ms\n",
			     buffering_mode, result, MilliSeconds_FromPts (buffering_time), MilliSeconds_FromPts (position_pts), MilliSeconds_FromPts (last_available_pts));

	return result;
}

void
MediaElement::UpdateProgress ()
{
	double progress, current;
	bool emit = false;
	
	LOG_MEDIAELEMENT_EX ("MediaElement::UpdateProgress (). Current state: %s\n", GetStateName (state));
	VERIFY_MAIN_THREAD;
	
	if (IsPlaying () && mplayer->IsBufferUnderflow () && GetBufferedSize () == 0.0) {
		// We're waiting for more data, switch to the 'Buffering' state.
		LOG_MEDIAELEMENT ("MediaElement::UpdateProgress (): Switching to 'Buffering', previous_position: "
				  "%" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, mplayer->GetPosition (): %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, buffered size: %.2f\n", 
				  previous_position, MilliSeconds_FromPts (previous_position), mplayer->GetPosition (),
				  MilliSeconds_FromPts (mplayer->GetPosition ()),
				  GetBufferedSize ());
		
		flags |= PlayRequested;
		SetBufferingProgress (0.0);
		Emit (BufferingProgressChangedEvent);
		SetState (MediaStateBuffering);
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
		if (emit || (progress == 1.0 && current != 1.0) || (progress - current) >= 0.05) {
			SetBufferingProgress (progress);
			//printf ("MediaElement::UpdateProgress (): Emitting BufferingProgressChanged: progress: %.3f, current: %.3f\n", progress, current);
			Emit (BufferingProgressChangedEvent);
		}
		
		if (progress == 1.0)
			BufferingComplete ();
	}
}

void
MediaElement::SetState (MediaState state)
{
	bool emit = false;
	
	LOG_MEDIAELEMENT ("MediaElement::SetState (%d): New state: %s, old state: %s\n",
			  state, GetStateName (state), GetStateName (this->state));	

	// thread-safe
	
	mutex.Lock ();
	if (this->state != state) {
		prev_state = this->state;
		this->state = state;
		emit = true;
	}
	mutex.Unlock ();
	
	if (emit) // Don't emit with mutex locked.
		Emit (CurrentStateChangedEvent);
}

void
MediaElement::BufferingComplete ()
{
	LOG_MEDIAELEMENT ("MediaElement::BufferingCompleted ()\n");
	VERIFY_MAIN_THREAD;
	
	buffering_mode = 0;

	if (state != MediaStateBuffering) {
		LOG_MEDIAELEMENT ("MediaElement::BufferingComplete (): current state is invalid ('%s'), should only be 'Buffering'\n",
				  GetStateName (state));
		return;
	}
	
	switch (prev_state) {
	case MediaStateOpening: // Start playback
		Play ();
		return;
	case MediaStatePlaying: // Restart playback
		Play ();
		return;
	case MediaStatePaused: // Do nothing
		// TODO: Should we show the first (new) frame here?
		return;
	case MediaStateError:
	case MediaStateBuffering:
	case MediaStateClosed:
	case MediaStateStopped: // This should not happen.
		LOG_MEDIAELEMENT ("MediaElement::BufferingComplete (): previous state is invalid ('%s').\n",
				  GetStateName (prev_state));
		return;
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElement: Invalid state.");
		return;
	}
}

void
MediaElement::CreatePlaylist ()
{
	g_return_if_fail (mplayer == NULL);
	
	mplayer = new MediaPlayer (this);
	SetPlaylist (new PlaylistRoot (this));
}

void
MediaElement::SetPlaylist (PlaylistRoot *value)
{
	// if playlist is something, then value must be null (and vice versa)
	g_return_if_fail ((playlist == NULL) != (value == NULL));
	
	VERIFY_MAIN_THREAD;
	
	if (playlist != NULL) {
		playlist->RemoveAllHandlers (this);
		playlist->Dispose ();
		playlist->unref ();
		playlist = NULL;
	} else {
		playlist = value; // We assume the caller gives us a reference to the playlist
		playlist->AddHandler (PlaylistRoot::OpeningEvent, OpeningCallback, this);
		playlist->AddHandler (PlaylistRoot::OpenCompletedEvent, OpenCompletedCallback, this);
		playlist->AddHandler (PlaylistRoot::SeekingEvent, SeekingCallback, this);
		playlist->AddHandler (PlaylistRoot::SeekCompletedEvent, SeekCompletedCallback, this);
		playlist->AddHandler (PlaylistRoot::CurrentStateChangedEvent, CurrentStateChangedCallback, this);
		playlist->AddHandler (PlaylistRoot::MediaErrorEvent, MediaErrorCallback, this);
		playlist->AddHandler (PlaylistRoot::MediaEndedEvent, MediaEndedCallback, this);
		playlist->AddHandler (PlaylistRoot::DownloadProgressChangedEvent, DownloadProgressChangedCallback, this);
		playlist->AddHandler (PlaylistRoot::BufferingProgressChangedEvent, BufferingProgressChangedCallback, this);
		playlist->AddHandler (PlaylistRoot::PlayEvent, PlayCallback, this);
		playlist->AddHandler (PlaylistRoot::PauseEvent, PauseCallback, this);
		playlist->AddHandler (PlaylistRoot::StopEvent, StopCallback, this);
	}
}

void
MediaElement::OpeningHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::OpeningHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	SetState (MediaStateOpening);
}

void
MediaElement::OpenCompletedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	IMediaDemuxer *demuxer;
	const char *demuxer_name;
	PlaylistEntry *entry;
	Media *media;
	
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (playlist != NULL);
	g_return_if_fail (mplayer != NULL);
	
	entry = playlist->GetCurrentPlaylistEntry ();
	
	g_return_if_fail (entry != NULL);
	
	media = entry->GetMedia ();
	
	g_return_if_fail (media != NULL);
	
	demuxer = media->GetDemuxer ();
	demuxer_name = demuxer->GetName ();
	
	LOG_MEDIAELEMENT ("MediaElement::OpenCompletedHandler (%p), demuxer name: %s\n", media, demuxer_name);
	
	// Try to figure out if we're missing codecs	
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

	SetProperties (media);
	
	PlayOrStop ();
	
	flags |= MediaOpenedEmitted;
	Emit (MediaOpenedEvent);
}

void
MediaElement::SetProperties (Media *media)
{
	IMediaDemuxer *demuxer;
	PlaylistEntry *entry;
	Duration *natural_duration;
	bool can_seek = true;
	
	LOG_MEDIAELEMENT ("MediaElement::SetProperties (%p)\n", media);
	
	g_return_if_fail (media != NULL);
	g_return_if_fail (playlist != NULL);
	
	demuxer = media->GetDemuxer ();
	entry = playlist->GetCurrentPlaylistEntry ();
	
	g_return_if_fail (demuxer != NULL);
	g_return_if_fail (entry != NULL);
	
	ReadMarkers (media, demuxer);
	
	
	if (entry->GetIsLive ()) {
		natural_duration = new Duration (TimeSpan_FromPts (0));
	} else {
		natural_duration = new Duration (TimeSpan_FromPts (mplayer->GetDuration ()));
	}
	
	can_seek = entry->GetClientSkip ();
	
	SetCanPause (true);
	SetCanSeek (can_seek);
	SetNaturalDuration (natural_duration);
	SetNaturalVideoHeight ((double) mplayer->GetVideoHeight ());
	SetNaturalVideoWidth ((double) mplayer->GetVideoWidth ());
	SetAudioStreamCount (mplayer->GetAudioStreamCount ());

	mplayer->SetMuted (GetIsMuted ());
	mplayer->SetVolume (GetVolume ());

	/* XXX FIXME horrible hack to keep old world charm until canvas logic is updated */
	if (GetVisualParent () && GetVisualParent ()->Is (Type::CANVAS)) {
		flags |= UpdatingSizeFromMedia;

		if (flags & UseMediaWidth) {
			Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);
			
			if (!(flags & UseMediaHeight))
				SetWidth ((double) mplayer->GetVideoWidth() * height->AsDouble () / (double) mplayer->GetVideoHeight());
			else
				SetWidth ((double) mplayer->GetVideoWidth());
		}
		
		if (flags & UseMediaHeight) {
			Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);
			
			if (!(flags & UseMediaWidth))
				SetHeight ((double) mplayer->GetVideoHeight() * width->AsDouble () / (double) mplayer->GetVideoWidth());
			else
				SetHeight ((double) mplayer->GetVideoHeight());
		}
		
		flags &= ~UpdatingSizeFromMedia;
	}
	InvalidateMeasure ();
}

void
MediaElement::SeekingHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::SeekingHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	SetMarkerTimeout (false);
}

void
MediaElement::SeekCompletedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::SeekCompletedHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	if (state == MediaStatePlaying)
		playlist->PlayAsync ();
	
	seek_to_position = -1;
	SetMarkerTimeout (true);
}

void
MediaElement::PlayHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::PlayHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	SetMarkerTimeout (true);
	
	SetState (MediaStatePlaying);
	Emit (CurrentStateChangedEvent);
}

void
MediaElement::PauseHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::PauseHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	SetMarkerTimeout (false);
	
	SetState (MediaStatePaused);
	Emit (CurrentStateChangedEvent);
}

void
MediaElement::StopHandler (PlaylistRoot *playlist, EventArgs *args)
{
	PlaylistEntry *entry;
	
	LOG_MEDIAELEMENT ("MediaElement::StopHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (playlist != NULL);
	
	entry = playlist->GetCurrentPlaylistEntry ();
	
	g_return_if_fail (entry != NULL);
		
	SetProperties (entry->GetMedia ());
	
	SetMarkerTimeout (false);
	
	SetState (MediaStateStopped);
	Emit (CurrentStateChangedEvent);
}

void
MediaElement::CurrentStateChangedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::CurrentStateChangedHandler ()\n");
	VERIFY_MAIN_THREAD;
}

void
MediaElement::MediaErrorHandler (PlaylistRoot *playlist, ErrorEventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::MediaErrorHandler (). State: %s\n", GetStateName (state));
	VERIFY_MAIN_THREAD;
	
	if (state == MediaStateError)
		return;
	
	// TODO: Should ClearValue be called on these instead?
	SetAudioStreamCount (0);
	SetNaturalVideoHeight (0);
	SetNaturalVideoWidth (0);
	SetNaturalDuration (0);
	SetCanPause (false);
	SetCanSeek (false);
	SetDownloadProgress (0);
	SetDownloadProgressOffset (0);
	SetRenderedFramesPerSecond (0);
	SetDroppedFramesPerSecond (0);
	SetState (MediaStateError);
	
	if (args)
		args->ref ();
	Emit (MediaFailedEvent, args);
}

void
MediaElement::MediaEndedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::MediaEndedHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	CheckMarkers ();
	SetState (MediaStatePaused);
	Emit (MediaEndedEvent);
}

void
MediaElement::DownloadProgressChangedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	Media *media;
	
	LOG_MEDIAELEMENT ("MediaElement::DownloadProgressChangedHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (playlist != NULL);
	
	media = playlist->GetCurrentMedia ();
	if (media != NULL)
		SetDownloadProgress (media->GetDownloadProgress ());
		
	Emit (DownloadProgressChangedEvent);
}

void
MediaElement::BufferingProgressChangedHandler (PlaylistRoot *playlist, EventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::BufferingProgressChangedHandler ()\n");
	VERIFY_MAIN_THREAD;
	
	Emit (BufferingProgressChangedEvent);
}

void
MediaElement::SetUriSource (Uri *uri)
{
	LOG_MEDIAELEMENT ("MediaElement::SetUriSource ('%s')\n", uri->ToString ());
	VERIFY_MAIN_THREAD;
	
	Reinitialize ();
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (playlist == NULL);
	
	CreatePlaylist ();
	char *str = uri->ToString ();
	playlist->GetCurrentEntry ()->InitializeWithUri (str);
	g_free (str);
}

void
MediaElement::SetSource (Downloader *downloader, const char *PartName)
{
	LOG_MEDIAELEMENT ("MediaElement::SetSource (%p, '%s')\n", downloader, PartName);
	VERIFY_MAIN_THREAD;

	Reinitialize ();
	
	g_return_if_fail (downloader != NULL);
	g_return_if_fail (playlist == NULL);
	
	CreatePlaylist ();
	playlist->GetCurrentEntry ()->InitializeWithDownloader (downloader, PartName);
}

void
MediaElement::SetStreamSource (ManagedStreamCallbacks *callbacks)
{
	LOG_MEDIAELEMENT ("MediaElement::SetStreamSource (%p)\n", callbacks);
	VERIFY_MAIN_THREAD;

	Reinitialize ();
	
	g_return_if_fail (callbacks != NULL);
	g_return_if_fail (playlist == NULL);
	
	CreatePlaylist ();
	playlist->GetCurrentEntry ()->InitializeWithStream (callbacks);
}

IMediaDemuxer *
MediaElement::SetDemuxerSource (void *context, CloseDemuxerCallback close_demuxer, GetDiagnosticAsyncCallback get_diagnostic, GetFrameAsyncCallback get_sample,
	OpenDemuxerAsyncCallback open_demuxer, SeekAsyncCallback seek, SwitchMediaStreamAsyncCallback switch_media_stream)
{
	ExternalDemuxer *demuxer;
	Media *media;
	
	LOG_MEDIAELEMENT ("MediaElement::SetDemuxerSource (%p)\n", demuxer);
	VERIFY_MAIN_THREAD;

	Reinitialize ();
	
	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (close_demuxer != NULL && get_diagnostic != NULL && get_sample != NULL && open_demuxer != NULL && seek != NULL && switch_media_stream != NULL, NULL);
	g_return_val_if_fail (playlist == NULL, NULL);
	
	CreatePlaylist ();
	media = new Media (playlist);
	demuxer = new ExternalDemuxer (media, context, close_demuxer, get_diagnostic, get_sample, open_demuxer, seek, switch_media_stream);
	playlist->GetCurrentEntry ()->InitializeWithDemuxer (demuxer);
	media->unref ();
	
	return demuxer;
}

void
MediaElement::SetPlayRequested ()
{
	LOG_MEDIAELEMENT ("MediaElement::SetPlayRequested ()\n");
	VERIFY_MAIN_THREAD;
	
	flags |= PlayRequested;
}

void
MediaElement::PlayOrStop ()
{
	LOG_MEDIAELEMENT ("MediaElement::PlayOrPause (): GetCanPause (): %s, PlayRequested: %s, GetAutoPlay: %s, AutoPlayed: %s\n",
			  GetCanPause () ? "true" : "false", (flags & PlayRequested) ? "true" : "false",
			  GetAutoPlay () ? "true" : "false", (flags & AutoPlayed) ? "true" : "false");
	VERIFY_MAIN_THREAD;
	
	if (!GetCanPause ()) {
		// If we can't pause, we play
		playlist->PlayAsync ();
	} else if (flags & PlayRequested) {
		// A Play has already been requested.
		playlist->PlayAsync ();
	} else if (GetAutoPlay () && !(flags & AutoPlayed)) {
		// Autoplay us.
		flags |= AutoPlayed;
		playlist->PlayAsync ();
	} else {
		SetState (MediaStatePlaying);
		SetState (MediaStateStopped);
	}
}

void
MediaElement::Pause ()
{
	LOG_MEDIAELEMENT ("MediaElement::Pause (): current state: %s\n", GetStateName (state));
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (playlist != NULL);
	
	switch (state) {
	case MediaStateOpening:// docs: No specified behaviour
		flags &= ~PlayRequested;
		return;
	case MediaStateClosed: // docs: No specified behaviour
	case MediaStateError:  // docs: ? (says nothing)
	case MediaStatePaused:// docs: no-op
		return;
	case MediaStateBuffering:
	case MediaStatePlaying:
	case MediaStateStopped: // docs: pause
		playlist->PauseAsync ();
		break;
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElement: Invalid state.");
		return;
	}
}

void
MediaElement::Play ()
{
	LOG_MEDIAELEMENT ("MediaElement::Play (): current state: %s\n", GetStateName (state));
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (playlist != NULL);
	
	switch (state) {
	case MediaStateClosed: // docs: No specified behaviour
	case MediaStateOpening:// docs: No specified behaviour
		flags |= PlayRequested;
		break;
	case MediaStateError:  // docs: ? (says nothing)
	case MediaStatePlaying:// docs: no-op
		return;
	case MediaStateBuffering:
	case MediaStatePaused:
	case MediaStateStopped:
		playlist->PlayAsync ();
		break;
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElement: Invalid state.");
		return;
	}
}

void
MediaElement::Stop ()
{
	LOG_MEDIAELEMENT ("MediaElement::Stop (): current state: %s\n", GetStateName (state));
	VERIFY_MAIN_THREAD;
	
	if (GetSurface () == NULL)
		return;

	switch (state) {
	case MediaStateOpening:// docs: No specified behaviour
		flags &= ~PlayRequested;
		return;
	case MediaStateClosed: // docs: No specified behaviour
	case MediaStateError:  // docs: ? (says nothing)
	case MediaStateStopped:// docs: no-op
		return;
	case MediaStateBuffering:
	case MediaStatePlaying:
	case MediaStatePaused: // docs: stop
		playlist->StopAsync ();
		break;
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElement: Invalid state.");
		return;
	}
}

void
MediaElement::Seek (TimeSpan to)
{
	LOG_MEDIAELEMENT ("MediaElement::Seek (%llu = %llu ms)\n", to, MilliSeconds_FromPts (to));
	VERIFY_MAIN_THREAD;

	if (GetSurface () == NULL)
		return;
	
	switch (state) {
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElement:Seek (): Invalid state %s\n", GetStateName (state));
		// Fall through
	case MediaStateOpening:
	case MediaStateClosed:
	case MediaStateError:
		break;
	case MediaStateBuffering:
	case MediaStatePlaying:
	case MediaStatePaused:
	case MediaStateStopped:
		Duration *duration = GetNaturalDuration ();
		
		if (duration->HasTimeSpan () && to > duration->GetTimeSpan ())
			to = duration->GetTimeSpan ();
		else if (to < 0)
			to = 0;
		
		if (to == TimeSpan_FromPts (mplayer->GetPosition ()))
			return;
		
		previous_position = to;
		seek_to_position = to;
		
		mplayer->NotifySeek (TimeSpan_ToPts (to));
		playlist->SeekAsync (to);
		Invalidate ();
		
		LOG_MEDIAELEMENT ("MediaElement::Seek (%llu = %llu ms) previous position: %llu\n", to, MilliSeconds_FromPts (to), previous_position);
		
		break;
	}
}

void
MediaElement::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MediaElement::SourceProperty) {
		flags |= RecalculateMatrix;
		SetUriSource (GetSource ());		
	} else if (args->GetId () == MediaElement::AudioStreamIndexProperty) {
		if (mplayer)
			mplayer->SetAudioStreamIndex (args->GetNewValue()->AsInt32 ());
	} else if (args->GetId () == MediaElement::AutoPlayProperty) {
		// no state to change
	} else if (args->GetId () == MediaElement::BalanceProperty) {
		if (mplayer)
			mplayer->SetBalance (args->GetNewValue()->AsDouble ());
	} else if (args->GetId () == MediaElement::BufferingProgressProperty) {
		// read-only property
	} else if (args->GetId () == MediaElement::BufferingTimeProperty) {
		// TODO
		// Not quite sure what to do here, we could:
		// a) store the buffering time on the mediaplayer and let the media request it whenever it needs it 
		// b) let the media request the buffering time from mediaelement
		// c) always keep the current media up to date (this is harder to do right when we get multiple media downloading at the same time)
		// note that thread-safety is easier with a) (media will do the request from another thread) or c)
		/*
		if (media)
			media->SetBufferingTime (TimeSpan_ToPts (GetBufferingTime ()));
		*/
	} else if (args->GetId () == MediaElement::CurrentStateProperty) {
		// read-only property
		// This should really not happen, we use a property provider for this property.
	} else if (args->GetId () == MediaElement::IsMutedProperty) {
		if (mplayer)
			mplayer->SetMuted (args->GetNewValue()->AsBool ());
	} else if (args->GetId () == MediaElement::MarkersProperty) {
		// 
	} else if (args->GetId () == MediaElement::NaturalVideoHeightProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (args->GetId () == MediaElement::NaturalVideoWidthProperty) {
		// read-only property
		flags |= RecalculateMatrix;
	} else if (args->GetId () == MediaElement::PositionProperty) {
		Seek (args->GetNewValue()->AsTimeSpan ());
		ClearValue (MediaElement::PositionProperty, false); // We need this, otherwise our property system will return the seeked-to position forever (MediaElementPropertyValueProvider isn't called).
	} else if (args->GetId () == MediaElement::VolumeProperty) {
		if (mplayer)
			mplayer->SetVolume (args->GetNewValue()->AsDouble ());
	} else if (args->GetId () == FrameworkElement::HeightProperty) {
		if (!(flags & UpdatingSizeFromMedia)) {
			if (args->GetNewValue() == NULL)
				flags |= UseMediaHeight;
			else
				flags &= ~UseMediaHeight;
		}
	} else if (args->GetId () == FrameworkElement::WidthProperty) {
		if (!(flags & UpdatingSizeFromMedia)) {
			if (args->GetNewValue() == NULL)
				flags |= UseMediaWidth;
			else
				flags &= ~UseMediaWidth;
		}
	}
	
	if (args->GetProperty ()->GetOwnerType() != Type::MEDIAELEMENT) {
		// propagate to parent class
		FrameworkElement::OnPropertyChanged (args, error);
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
MediaElement::ReportErrorOccurred (ErrorEventArgs *args)
{
	LOG_MEDIAELEMENT ("MediaElement::ReportErrorOccurred (%p)\n", args);
	VERIFY_MAIN_THREAD;
	
	MediaErrorHandler (NULL, args);
}

void
MediaElement::ReportErrorOccurred (const char *args)
{
	LOG_MEDIAELEMENT ("MediaElement::ReportErrorOccurred ('%s')\n", args);
	VERIFY_MAIN_THREAD;
	
	MediaErrorHandler (NULL, new ErrorEventArgs (MediaError, 3001, args));
}

/*
 * MediaElementPropertyValueProvider
 */

MediaElementPropertyValueProvider::MediaElementPropertyValueProvider (MediaElement *element)
	: PropertyValueProvider (element)
{
	position = NULL;
	current_state = NULL;
}

MediaElementPropertyValueProvider::~MediaElementPropertyValueProvider ()
{
	delete position;
	delete current_state;
}

Value *
MediaElementPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	// We verify main thread here too in case some object in the pipeline happens to want a property on the media element
	VERIFY_MAIN_THREAD;
	
	if (property->GetId () == MediaElement::PositionProperty)
		return GetPosition ();
		
	if (property->GetId () == MediaElement::CurrentStateProperty)
		return GetCurrentState ();
	
	return NULL;
}

Value *
MediaElementPropertyValueProvider::GetCurrentState ()
{
	MediaElement *element = (MediaElement *) obj;

	delete current_state;
	current_state = new Value (element->state);
	
	return current_state;
}

Value *
MediaElementPropertyValueProvider::GetPosition ()
{
	bool use_mplayer;
	MediaElement *element = (MediaElement *) obj;
	guint64 position = TimeSpan_ToPts (element->seek_to_position);
	
	delete this->position;
	this->position = NULL;
	
	switch (element->state) {
	case MediaStateIndividualizing:
	case MediaStateAcquiringLicense:
		g_warning ("MediaElementPropertyValueProvider::GetPosition (): Invalid state.\n");
		// Fall through
	case MediaStateOpening:
	case MediaStateClosed:
	case MediaStateError:
		use_mplayer = false;
		break;
	case MediaStateStopped:
	case MediaStateBuffering:
	case MediaStatePlaying:
	case MediaStatePaused:
		use_mplayer = true;
		break;
	}
	
	// If a seek is pending, we need to return that position.
	
	if (use_mplayer && (TimeSpan_FromPts (position) == -1))
		position = element->mplayer->GetPosition ();
	
	if (TimeSpan_FromPts (position) != -1) {
		this->position = new Value (TimeSpan_FromPts (position), Type::TIMESPAN);
	} else {
		this->position = new Value (0, Type::TIMESPAN);
	}
		
	return this->position;
}
