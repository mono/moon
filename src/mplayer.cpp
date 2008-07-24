/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mplayer.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdlib.h>

#include <poll.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <asoundlib.h>

#include "clock.h"
#include "mplayer.h"
#include "pipeline.h"
#include "runtime.h"
#include "list.h"
#include "media.h"

#define DEBUG_ADVANCEFRAME 0
#define LOG_MEDIAPLAYER(...)// printf (__VA_ARGS__);
// This one prints out spew on every frame
#define LOG_MEDIAPLAYER_EX(...)// printf (__VA_ARGS__);
#define LOG_AUDIO(...)// printf (__VA_ARGS__);
// This one prints out spew on every sample
#define LOG_AUDIO_EX(...)// printf (__VA_ARGS__);

/*
 * Packet
 */

class Packet : public List::Node {
public:
	MediaFrame *frame;
	
	Packet (MediaFrame *frame)
	{
		this->frame = frame;
	}
	virtual ~Packet ()
	{
		delete frame;
	}
};

/*
 * Audio
 */

Audio::Audio ()
{	
	balance = 0.0f;
	volume = 1.0f;
	muted = false;
	stream_count = 0;
	stream = NULL;
	pts_per_frame = 0;
}

/*
 * Video
 */

Video::Video ()
{	
	stream = NULL;
	surface = NULL;
	rgb_buffer = NULL;
}

/*
 * MediaPlayer
 */

MediaPlayer::MediaPlayer (MediaElement *el)
{
	LOG_MEDIAPLAYER ("MediaPlayer::MediaPlayer (%p, id=%i), id=%i\n", el, GET_OBJ_ID (el), GET_OBJ_ID (this));

	element = el;
	pthread_mutex_init (&target_pts_lock, NULL);

	media = NULL;
	
	Initialize ();
}

MediaPlayer::~MediaPlayer ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::~MediaPlayer (), id=%i\n", GET_OBJ_ID (this));
	Close (true);
	
	pthread_mutex_destroy (&target_pts_lock);
}

void
MediaPlayer::SetSurface (Surface *s)
{
	if (!SetSurfaceLock ())
		return;
	
	EventObject::SetSurface (s);
	
	SetSurfaceUnlock ();
}

void
MediaPlayer::EnqueueVideoFrameCallback (EventObject *user_data)
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::EnqueueVideoFrameCallback ()\n");
	MediaPlayer *mplayer = (MediaPlayer *) user_data;
	mplayer->EnqueueFrames (0, 1);
}

void
MediaPlayer::EnqueueAudioFrameCallback (EventObject *user_data)
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::EnqueueAudioFrameCallback ()\n");
	MediaPlayer *mplayer = (MediaPlayer *) user_data;
	mplayer->EnqueueFrames (1, 0);
}

void
MediaPlayer::LoadFrameCallback (EventObject *user_data)
{
	bool result = false;

	LOG_MEDIAPLAYER ("MediaPlayer::LoadFrameCallback ()\n");

	MediaPlayer *player = (MediaPlayer*) user_data;
	if (player != NULL) {
		// Check again if LoadFrame is still pending.
 		if (player->IsLoadFramePending ())
			result = player->LoadVideoFrame ();
	}		
}

static const char *media_type_name[] = {
	"None", "Video", "Audio", "Marker"
};

MediaResult
MediaPlayer::FrameCallback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *player = element->GetMediaPlayer ();
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame ? frame->stream : NULL;
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::FrameCallback (closure=%p)\n"
			    "\tstate: %d, frame: %p, pts: %llu = %llu, type: %s\n"
			    "\taudio packets: %d, video packets: %d\n",
			    closure, player->state, closure->frame, frame ? frame->pts : 0,
			    frame ? MilliSeconds_FromPts (frame->pts) : 0, media_type_name[stream->GetType ()],
			    player->audio.queue.Length (), player->video.queue.Length ());
	
	if (player->GetBit (MediaPlayer::Seeking)) {
		// We don't want any frames while we're waiting for a seek.
		return MEDIA_SUCCESS;
	}
	
	if (closure->frame == NULL)
		return MEDIA_SUCCESS;
	
	closure->frame = NULL;
	
	if (element->IsLive ()) {
		if (player->first_live_pts == G_MAXULONG) {
			player->first_live_pts = frame->pts;
		} else if (player->first_live_pts > frame->pts) {
			//printf ("\tMediaPlayer::FrameCallback (): Found a frame with lower pts (%llu) than a previous frame (%llu).\n", frame->pts, player->first_live_pts);
			player->first_live_pts = frame->pts;
		}
	}
	
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		player->video.queue.Push (new Packet (frame));
		if (player->IsLoadFramePending ()) {
			// We need to call LoadVideoFrame on the main thread
			player->AddTickCallSafe (LoadFrameCallback);
		}
		return MEDIA_SUCCESS;
	case MediaTypeAudio: 
		player->audio.queue.Push (new Packet (frame));
		AudioPlayer::WakeUp ();
		return MEDIA_SUCCESS;
	default:
		return MEDIA_SUCCESS;
	}
}

void
MediaPlayer::AudioFinishedCallback (EventObject *user_data)
{
	LOG_MEDIAPLAYER ("MediaPlayer::AudioFinishedCallback ()\n");

	MediaPlayer *mplayer = (MediaPlayer *) user_data;
	mplayer->AudioFinished ();
}

void
MediaPlayer::AudioFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::AudioFinished ()\n");

	if (!Surface::InMainThread ()) {
		AddTickCallSafe (AudioFinishedCallback);
		return;
	}

	if (HasVideo ())
		return;

	if (!HasAudio ())
		return;

	if (element) {
		Stop ();
		element->AudioFinished ();
	}
}

void
MediaPlayer::EnqueueFramesAsync (int audio_frames, int video_frames)
{
	for (int i = 0; i < audio_frames; i++)
		AddTickCallSafe (EnqueueAudioFrameCallback);
	
	for (int i = 0; i < video_frames; i++)
		AddTickCallSafe (EnqueueVideoFrameCallback);
}

void
MediaPlayer::EnqueueFrames (int audio_frames, int video_frames)
{
	MediaClosure *closure;
	int states;

	LOG_MEDIAPLAYER_EX ("MediaPlayer::EnqueueFrames (%i, %i)\n", audio_frames, video_frames);
	
	if (element == NULL || GetBit (Seeking))
		return;

	if (HasAudio ()) {
		for (int i = 0; i < audio_frames; i++) {
			closure = new MediaClosure (FrameCallback);
			closure->SetContext (element);
			
			// To decode on the main thread comment out FRAME_DECODED and FRAME_COPY_DECODED_DATA.
			states = FRAME_DEMUXED | FRAME_DECODED | FRAME_COPY_DECODED_DATA;
			
			media->GetNextFrameAsync (closure, audio.stream, states);
		}
	}
	
	if (HasVideo ()) {
		for (int i = 0; i < video_frames; i++) {
			closure = new MediaClosure (FrameCallback);
			closure->SetContext (element);
			
			// To decode on the main thread comment out FRAME_DECODED and FRAME_COPY_DECODED_DATA.
			states = FRAME_DEMUXED | FRAME_DECODED | FRAME_COPY_DECODED_DATA;
				
			media->GetNextFrameAsync (closure, video.stream, states);
		}
	}
}

bool
MediaPlayer::Open (Media *media)
{
	int stride;
	IMediaDecoder *encoding;
	IMediaStream *stream;
	guint64 asx_duration;
	
	LOG_MEDIAPLAYER ("MediaPlayer::Open (%p), current media: %p\n", media, this->media);
	
	Close (false);

	if (media == NULL) {
		printf ("MediaPlayer::Open (): media is NULL.\n");
		return false;
	}
	
	if (!media->IsOpened ()) {
		printf ("MediaPlayer::Open (): media isn't opened.\n");
		return false;
	}
	
	this->media = media;
	this->media->ref ();
	SetState (Opened);
	
	// Find audio/video streams
	IMediaDemuxer *demuxer = media->GetDemuxer ();
	VideoStream *vstream;
	AudioStream *astream;
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);
		encoding = stream->GetDecoder (); //stream->codec;
		
		if (encoding == NULL)
			continue; // No encoding was found for the stream.
		
		switch (stream->GetType ()) {
		case MediaTypeAudio:
			astream = (AudioStream *) stream;
			audio.stream_count++;			

			if (audio.stream != NULL && astream->GetBitRate () < audio.stream->GetBitRate ())
				break;

			audio.stream = astream;
			audio.stream->SetSelected (true);
			break;
		case MediaTypeVideo: 
			vstream = (VideoStream *) stream;

			if (video.stream != NULL && vstream->GetBitRate () < video.stream->GetBitRate ())
				break;

			video.stream = vstream;
			video.stream->SetSelected (true);
			
			height = video.stream->height;
			width = video.stream->width;

			stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
			if (stride % 64) {
				int remain = stride % 64;
				stride += 64 - remain;
			}
			
			// for conversion to rgb32 format needed for rendering with 16 byte alignment
			if (posix_memalign ((void **)(&video.rgb_buffer), 16, height * stride)) {
				g_warning ("Could not allocate memory for video RGB buffer");
				return false;
			}
			
			memset (video.rgb_buffer, 0, height * stride);
			
			// rendering surface
			video.surface = cairo_image_surface_create_for_data (
				video.rgb_buffer, CAIRO_FORMAT_ARGB32,
				width, height, stride);
			
			// printf ("video size: %i, %i\n", video.stream->width, video.stream->height);
			break;
		case MediaTypeMarker:
			LOG_MEDIAPLAYER ("MediaPlayer::Open (): Found a marker stream, selecting it.\n");
			stream->SetSelected (true);
		default:
			break;
		}
	}

	if (audio.stream != NULL) {
		AudioPlayer::Add (this);
		// TODO: Detect when we can't play audio and deselect the audio stream,
		// otherwise we'll be decoding audio nobody will hear.
		//if (!AudioPlayer::Add (this)) {
		//	// Can't play audio
		//	audio.stream->SetSelected (false);
		//	audio.stream = NULL;
		//}
	}
	
	current_pts = 0;
	target_pts = 0;
	start_pts = 0;
	PlaylistEntry *entry = element->GetPlaylist ()->GetCurrentPlaylistEntry ();
	if (entry != NULL) {
		start_pts =  TimeSpan_ToPts (entry->GetStartTime ());
		LOG_MEDIAPLAYER ("MediaPlayer::Open (), setting start_pts to: %llu (%llu ms).\n", start_pts, MilliSeconds_FromPts (start_pts));
		if (start_pts > 0) {
			SeekInternal (start_pts);
		}
	}
	duration = media->GetDemuxer ()->GetDuration ();
	if (start_pts >= duration + MilliSeconds_ToPts (6000) /* This random value (6000) is as close as I could get without spending hours testing */) {
		element->MediaFailed (new ErrorEventArgs (MediaError, 1001, "AG_E_UNKNOWN_ERROR"));
		return false;
	}

	if (entry != NULL && entry->HasDuration ()) {
		asx_duration = TimeSpan_ToPts (entry->GetDuration ());
		if (asx_duration < duration || element->IsLive ()) {
			duration = asx_duration;
			SetBit (FixedDuration);
		}
	}

	if (start_pts <= duration)
		duration -= start_pts;
	else
		duration = 0;
	
	if (HasVideo ()) {
		SetBit (LoadFramePending);
		EnqueueFrames (0, 1);
	}
	
	return true;
}

void 
MediaPlayer::Initialize ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Initialize ()\n");

	state = (PlayerState) 0;
	SetState (Stopped);
	SetBit (SeekSynched);
	SetBit (CanSeek);
	SetBit (CanPause);
	
	start_time = 0;
	start_pts = 0;
	current_pts = 0;
	target_pts = 0;
	first_live_pts = G_MAXULONG;
	
	height = 0;
	width = 0;
}

void
MediaPlayer::Close (bool dtor)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Close ()\n");

	AudioPlayer::Remove (this);

	Stop (false);
	
	// Reset state back to what it was at instantiation

	if (video.rgb_buffer != NULL) {
		free (video.rgb_buffer);
		video.rgb_buffer = NULL;
	}

	if (video.surface != NULL) {
		cairo_surface_destroy (video.surface);
		video.surface = NULL;
	}
	video.stream = NULL;
	
	audio.stream_count = 0;
	audio.stream = NULL;
	audio.pts_per_frame = 0;
	
	if (media)
		media->unref ();
	media = NULL;

	if (dtor) {
		// To avoid circular references we don't keep a ref to the media element.
		element = NULL;
	}

	Initialize ();
}

//
// Puts the data into our rgb buffer.
// If necessary converts the data from its source format to rgb first.
//

void
MediaPlayer::RenderFrame (MediaFrame *frame)
{
	VideoStream *stream = (VideoStream *) frame->stream;

	LOG_MEDIAPLAYER_EX ("MediaPlayer::RenderFrame (%p)\n", frame);
	
	if (!frame->IsDecoded ()) {
		fprintf (stderr, "MediaPlayer::RenderFrame (): Trying to render a frame which hasn't been decoded yet.\n");
		return;
	}
	
	if (!frame->IsPlanar ()) {
		// Just copy the data
		memcpy (video.rgb_buffer, frame->buffer, MIN (frame->buflen, (guint32) (cairo_image_surface_get_stride (video.surface) * height)));
		SetBit (RenderedFrame);
		return;
	}
	
	if (frame->data_stride == NULL || 
		frame->data_stride[1] == NULL || 
		frame->data_stride[2] == NULL) {
		return;
	}
	
	guint8 *rgb_dest [3] = { video.rgb_buffer, NULL, NULL };
	int rgb_stride [3] = { cairo_image_surface_get_stride (video.surface), 0, 0 };
	
	stream->converter->Convert (frame->data_stride, frame->srcStride, frame->srcSlideY,
				    frame->srcSlideH, rgb_dest, rgb_stride);
	
	SetBit (RenderedFrame);
}


bool
MediaPlayer::AdvanceFrame ()
{
	Packet *pkt = NULL;
	MediaFrame *frame = NULL;
	IMediaStream *stream;
	guint64 target_pts = 0;
	guint64 target_pts_start = 0;
	guint64 target_pts_end = 0;
	guint64 target_pts_delta = MilliSeconds_ToPts (100);
	bool update = false;
	
#if DEBUG_ADVANCEFRAME
	static int frames_per_second = 0;
	static int skipped_per_second = 0;
	static guint64 last_second_pts = 0;
	int skipped = 0;
#endif
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame () state: %i, current_pts = %llu, IsPaused: %i, IsSeeking: %i, GetEof: %i, HasVideo: %i, HasAudio: %i\n", 
		state, current_pts, IsPaused (), IsSeeking (), GetEof (), HasVideo (), HasAudio ());

	RemoveBit (LoadFramePending);
	
	if (IsPaused ())
		return false;
	
	if (IsSeeking ())
		return false;
	
	if (GetEof ())
		return false;

	if (!HasVideo ())
		return false;

	if (HasAudio ()) {
		// use target_pts as set by audio thread
		target_pts = GetTargetPts ();	
	} else {
		// no audio to sync to
		guint64 now = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
		guint64 elapsed_pts = now - start_time;
		
		target_pts = elapsed_pts;
		
		this->target_pts = target_pts;
		/*
		printf ("MediaPlayer::AdvanceFrame (): determined target_pts to be: %llu = %llu ms, elapsed_pts: %llu = %llu ms, start_time: %llu = %llu ms\n",
			target_pts, MilliSeconds_FromPts (target_pts), elapsed_pts, MilliSeconds_FromPts (elapsed_pts), start_time, MilliSeconds_FromPts (start_time));
		*/
	}
	
	target_pts_start = target_pts_delta > target_pts ? 0 : target_pts - target_pts_delta;
	target_pts_end = target_pts + target_pts_delta;
	
	if (current_pts >= target_pts_end && GetBit (SeekSynched)) {
#if DEBUG_ADVANCEFRAME
		printf ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %llu, target_pts: %llu, delta: %llu, diff: %lld (%lld ms)).\n",
			current_pts, target_pts, target_pts_delta, current_pts - target_pts, MilliSeconds_FromPts (current_pts - target_pts));
#endif
		return false;
	}
	
	//printf ("MediaPlayer::AdvanceFrame (): target pts: %llu = %llu ms\n", target_pts, MilliSeconds_FromPts (target_pts));
		
	while ((pkt = (Packet *) video.queue.Pop ())) {
		if (pkt->frame->event == FrameEventEOF) {
			if (!HasAudio ()) {
				// Set the target pts to the last pts we showed, since target_pts is what's reported as our current position.
				this->target_pts = current_pts;
			}
			delete pkt;
			SetEof (true);
			return false;
		}
		
		// always decode the frame or we get glitches in the screen
		frame = pkt->frame;
		stream = frame->stream;
		current_pts = frame->pts;
		update = true;
		
		//printf ("MediaPlayer::AdvanceFrame (): current_pts: %llu = %llu ms, duration: %llu = %llu ms\n",
		//		current_pts, MilliSeconds_FromPts (current_pts),
		//		duration, MilliSeconds_FromPts (duration));
		
		if (GetBit (FixedDuration)) {
/*
			printf ("MediaPlayer::AdvanceFrame (): (fixed duration, live: %i) current_pts: %llu = %llu ms, duration: %llu = %llu ms, first_live_pts: %llu = %llu ms\n",
				element->IsLive (),
				current_pts, MilliSeconds_FromPts (current_pts),
				duration, MilliSeconds_FromPts (duration),
				first_live_pts, MilliSeconds_FromPts (first_live_pts),
				current_pts - first_live_pts, MilliSeconds_FromPts (current_pts - first_live_pts));
*/
			if (element->IsLive ()) {
				if (current_pts - first_live_pts > duration)
					SetEof (true);
			} else {
				if (current_pts > duration)
					SetEof (true);
			}
			if (GetEof ()) {
				//printf ("MediaPlayer::AdvanceFrame (): Reached end of duration.\n");
				update = false;
				break;
			}
		}
		
		EnqueueFrames (0, 1);	
		
		if (!frame->IsDecoded ()) {
			//printf ("MediaPlayer::AdvanceFrame (): decoding on main thread.\n");
			MediaResult result = stream->decoder->DecodeFrame (frame);
			
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("MediaPlayer::AdvanceFrame (): Couldn't decode frame (%i)\n", result);
				update = false;
			}
		}
		
		if (update && current_pts >= target_pts_start) {
			if (!GetBit (SeekSynched)) {
				LOG_MEDIAPLAYER ("MediaPlayer::AdvanceFrame (): We have now successfully synched with the audio after the seek, current_pts: %llu\n", current_pts);
			}
			SetBit (SeekSynched);
			// we are in sync (or ahead) of audio playback
			break;
		}
		
		if (video.queue.IsEmpty ()) {
			// no more packets in queue, this frame is the most recent we have available
			EnqueueFrames (0, 1);
			break;
		}
		
		// we are lagging behind, drop this frame
#if DEBUG_ADVANCEFRAME
		//printf ("MediaPlayer::AdvanceFrame (): skipped frame with pts %llu, target pts: %llu, diff: %lld, milliseconds: %lld\n", frame->pts, target_pts, target_pts - frame->pts, MilliSeconds_FromPts ((target_pts - frame->pts)));
		skipped++;
#endif
		frame = NULL;
		delete pkt;
	}
	
	if (update && frame && GetBit (SeekSynched)) {
#if DEBUG_ADVANCEFRAME
		int fps = 0, sps = 0;
		guint64 ms = 0;
		frames_per_second++;
		skipped_per_second += skipped;
		if (MilliSeconds_FromPts (target_pts - last_second_pts) > 1000) {
			fps = frames_per_second;
			sps = skipped_per_second;
			frames_per_second = 0;
			skipped_per_second = 0;
			ms = MilliSeconds_FromPts (target_pts - last_second_pts);
			last_second_pts = target_pts;
		}
		if (fps > 0)
			printf ("MediaPlayer::AdvanceFrame (): rendering pts %llu (target pts: %llu, current pts: %llu, skipped frames: %i, fps: %i, sps: %i, ms: %llu)\n", frame->pts, target_pts, current_pts, skipped, fps, sps, ms);
#endif
		RenderFrame (frame);
		delete pkt;
		
		return true;
	}
	
	delete pkt;
		
	return !GetEof ();
}

bool
MediaPlayer::LoadVideoFrame ()
{
	Packet *packet;
	bool cont = false;
	
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), HasVideo: %i, LoadFramePending: %i, queue size: %i\n", HasVideo (), state & LoadFramePending, video.queue.Length ());

	if (!HasVideo ())
		return false;
	
	if (!IsLoadFramePending ())
		return false;
	
	packet = (Packet*) video.queue.Pop ();

	if (packet != NULL && packet->frame->event == FrameEventEOF)
		return false;

	EnqueueFrames (0, 1);
	
	if (packet == NULL)
		return false;
	
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), packet pts: %llu, target pts: %llu\n", packet->frame->pts, GetTargetPts ());

	if (packet->frame->pts >= GetTargetPts ()) {
		RemoveBit (LoadFramePending);
		RenderFrame (packet->frame);
		element->Invalidate ();
	} else {
		cont = true;
	}
	
	delete packet;
	
	return cont;
}

bool
MediaPlayer::MediaEnded ()
{
	return GetEof ();
}

void
MediaPlayer::SetEof (bool value)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetEof (%i), state: %i\n", value, state);

	if (value) {
		SetBit (Eof);
	} else {
		RemoveBit (Eof);
	}
}

void
MediaPlayer::Play ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i, IsPlaying: %i, IsSeeking: %i\n", state, IsPlaying (), IsSeeking ());

	if (IsPlaying () && !IsSeeking ())
		return;
	
	SetState (Playing);
	start_time = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
	start_time -= target_pts;

	AudioPlayer::Play (this);
	
	// Request 10 audio packets from the start, the audio thread will in some cases
	// be able to send more audio samples to the hardware in one call than what's
	// in one single frame, but this depends of course on having more than one 
	// decoded frame available.
	EnqueueFrames (3, 1);

	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i [Done]\n", state);
}

gint32
MediaPlayer::GetTimeoutInterval ()
{
	if (HasVideo ()) {
		// TODO: Calculate correct framerate (in the pipeline)
		return MAX (video.stream->msec_per_frame, 1000 / 60);
	} else {
		return 33;
	}
}

bool
MediaPlayer::GetCanPause ()
{
	// FIXME: should return false if it is streaming media
	return GetBit (CanPause);
}

void
MediaPlayer::SetCanPause (bool value)
{
	SetBitTo (CanPause, value);
}

void
MediaPlayer::Pause ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i\n", state);

	if (IsPaused ())
		return;
	
	SetState (Paused);
	SetBit (SeekSynched);
	AudioPlayer::Pause (this, true);

	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i [Done]\n", state);	
}

guint64
MediaPlayer::GetTargetPts ()
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::GetTargetPts (): target_pts: %llu\n", target_pts);

	guint64 result;
	
	pthread_mutex_lock (&target_pts_lock);
	result = target_pts;
	pthread_mutex_unlock (&target_pts_lock);
	
	return result;
}

void
MediaPlayer::SetTargetPts (guint64 pts)
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::SetTargetPts (%llu = %llu ms), current_pts: %llu, IsSeeking (): %i\n", pts, MilliSeconds_FromPts (pts), current_pts, IsSeeking ());

	if (IsSeeking () || IsPaused ())
		return;

	pthread_mutex_lock (&target_pts_lock);
	target_pts = pts;
	pthread_mutex_unlock (&target_pts_lock);
}

void
MediaPlayer::SeekCallback ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekCallback ()\n");

	element->SetPreviousPosition (GetTargetPts ());

	// Clear all queues.
	audio.queue.Clear (true);
	video.queue.Clear (true);
	
	RemoveBit (Seeking);
	current_pts = 0;
	
	EnqueueFrames (1, 1);
}

void
MediaPlayer::SeekCallback (EventObject *mplayer)
{
	((MediaPlayer *) mplayer)->SeekCallback ();
}

MediaResult
MediaPlayer::SeekCallback (MediaClosure *closure)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekCallback (%p)\n", closure);

	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *mplayer = element->GetMediaPlayer ();
	mplayer->AddTickCallSafe (SeekCallback);
	
	return MEDIA_SUCCESS;
}

void
MediaPlayer::SeekInternal (guint64 pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekInternal (%llu = %llu ms), media: %p, state: %i, Position (): %llu\n", pts, MilliSeconds_FromPts (pts), media, state, GetPosition ());

	if (media == NULL)
		return;

	SetBit (Seeking);
	RemoveBit (SeekSynched);

	MediaClosure *closure = new MediaClosure (SeekCallback);
	closure->SetContext (element);
	media->ClearQueue ();
	media->SeekAsync (pts, closure);
}

void
MediaPlayer::Seek (guint64 pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%llu = %llu ms), media: %p, state: %i, current_pts: %llu, IsPlaying (): %i\n", pts, MilliSeconds_FromPts (pts), media, state, current_pts, IsPlaying ());

	guint64 duration = GetDuration ();
	bool resume = IsPlaying ();
	
	if (!GetCanSeek ())
		return;
	
	if (pts > start_pts + duration)
		pts = start_pts + duration;
	
	if (pts < start_pts)
		pts = start_pts;

	if (pts == current_pts)
		return;
	
	// Clear all queues.
	audio.queue.Clear (true);
	video.queue.Clear (true);
	
	// Stop the audio
	AudioPlayer::Stop (this);

	SetBit (Seeking);
	RemoveBit (Eof);
	if (HasVideo () && !resume)
		SetBit (LoadFramePending);

	start_time = 0;	
	current_pts = pts;
	target_pts = pts;
	
	SeekInternal (pts);
	
	if (resume)
		Play ();

	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%llu = %llu ms), media: %p, state: %i, current_pts: %llu, resume: %i [END]\n", pts, MilliSeconds_FromPts (pts), media, state, current_pts, resume);
}

bool
MediaPlayer::GetCanSeek ()
{
	return GetBit (CanSeek);
}

void
MediaPlayer::SetCanSeek (bool value)
{
	SetBitTo (CanSeek, value);
}

void
MediaPlayer::Stop (bool seek_to_start)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Stop (), state: %i\n", state);

	AudioPlayer::Stop (this);

	audio.queue.Clear (true);
	video.queue.Clear (true);
		
	start_time = 0;
	current_pts = 0;
	target_pts = 0;
	SetState (Stopped);
	RemoveBit (Eof);

	// If we're closing the media player, there is no need to 
	// seek to the beginning, it may even cause crashes if we're
	// closing because the MediaElement is being destructed.
	if (seek_to_start)
		SeekInternal (0);
}

void
MediaPlayer::SetBalance (double balance)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetBalance (%f)\n", balance);

	if (balance < -1.0)
		balance = -1.0;
	else if (balance > 1.0)
		balance = 1.0;
	
	audio.balance = balance;
}

void
MediaPlayer::SetVolume (double volume)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetVolume (%f)\n", volume);

	if (volume < -1.0)
		volume = -1.0;
	else if (volume > 1.0)
		volume = 1.0;
	
	audio.volume = volume;
}

/*
 * AudioPlayer
 */

pthread_mutex_t AudioPlayer::instance_mutex = PTHREAD_MUTEX_INITIALIZER;
AudioPlayer *AudioPlayer::instance = NULL;

void
AudioPlayer::Shutdown ()
{
	LOG_AUDIO ("AudioPlayer::Shutdown ()\n");
	
	int result;
	bool join = false;
	pthread_t audio_thread;
	pthread_mutex_lock (&instance_mutex);
	
	if (instance) {
		audio_thread = instance->audio_thread;
		
		instance->AddWork (NULL, ActionShutdown);
		
		// The instance might get deleted at any moment from now on, 
		// don't use it anymore.
		instance = NULL;
	
		join = true;
	}
	
	// We must unlock the mutex before joining the thread.
	pthread_mutex_unlock (&instance_mutex);
	
	if (join) {		
		// Wait for the audio thread to finsih
		result = pthread_join (audio_thread, NULL);
		if (result != 0) {
			fprintf (stderr, "AudioPlayer::Shutdown (): failed to join the audio thread (error code: %i).\n", result);
		}
	}
	
	LOG_AUDIO ("AudioPlayer::Shutdown (): Done\n");
}

AudioPlayer::AudioPlayer ()
{
	int result;
	
	LOG_AUDIO ("AudioPlayer::AudioPlayer ()\n");
	
	list = NULL;
	list_count = 0;
	list_size = 0;

	udfs = NULL;
	ndfs = 0;

	fds [0] = -1;
	fds [1] = -1;
	if (pipe (fds) != 0) {
		fprintf (stderr, "AudioPlayer::AudioPlayer (): Unable to create pipe (%s).\n", strerror (errno));
		return;
	}

	// Make the writer pipe non-blocking.
	fcntl (fds [1], F_SETFL, fcntl (fds [1], F_GETFL) | O_NONBLOCK);

	ndfs = 1;
	udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
	udfs [0].fd = fds [0];
	udfs [0].events = POLLIN;
	
	result = pthread_create (&audio_thread, NULL, Loop, this);
	if (result != 0)
		fprintf (stderr, "AudioPlayer::AudioPlayer (): could not create audio thread (error code: %i = '%s').\n", result, strerror (result));
	
	LOG_AUDIO ("AudioPlayer::AudioPlayer (): the audio player has been initialized.\n");
}

AudioPlayer::~AudioPlayer ()
{
	LOG_AUDIO ("AudioPlayer::~AudioPlayer ()\n");
}

void
AudioPlayer::ShutdownInternal ()
{
	LOG_AUDIO ("AudioPlayer::ShutdownInternal ().\n");
	
	if (list != NULL) {	
		for (guint32 i = 0; i < list_count; i++)
			RemoveInternal (list [i]->GetMediaPlayer ());
		g_free (list);
		list = NULL;
	}

	close (fds [0]);
	close (fds [1]);
	
	g_free (udfs);
	udfs = NULL;
	
	LOG_AUDIO ("AudioPlayer::ShutdownInternal (): the audio player has been shut down.\n");
}

void
AudioPlayer::AddWork (MediaPlayer *mplayer, AudioAction action)
{
	LOG_AUDIO ("AudioPlayer::AddWork (%p, %i)\n", mplayer, action);
	
	if (mplayer != NULL && mplayer->GetRefCount () == 0) {
		// MediaPlayer might end up calling us when it is in the destructor
		// Since we ref the MediaPlayer, we are guaranteed to not be playing
		// that MediaPlayer. Optimize this case to not do anything (besides,
		// since we ref the MediaPlayer it'll cause us to abort due to reffing
		// an object which is being destructed).
		LOG_AUDIO ("AudioPlayer::AddWork (): Not adding work since MediaPlayer is being destructed. Action: %i\n", action);
		return;
	}
	
	work.Push (new AudioListNode (mplayer, action));
	WakeUp ();
}

void
AudioPlayer::Add (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	
	pthread_mutex_lock (&instance_mutex);
	
	if (instance == NULL)
		instance = new AudioPlayer ();

	instance->AddWork (mplayer, ActionAdd);
	
	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::AddInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::AddInternal (%p)\n", mplayer);

	AudioNode *node = new AudioNode (mplayer);
	AudioNode **new_list = NULL;

	if (!node->Initialize ()) {
		delete node;
		return;
	}
	
	list_count++;
	if (list_count > list_size) {
		// Grow the list a bit
		list_size = (list_size == 0) ? 1 : list_size << 1;
		new_list = (AudioNode**) g_malloc0 (sizeof (AudioNode*) * (list_size + 1));
		if (list != NULL) {
			for (guint32 i = 0; i < list_count - 1; i++)
				new_list [i] = list [i];
			g_free (list);
		}
		list = new_list;
	}
	list [list_count - 1] = node;
	
	UpdatePollList ();
}

void
AudioPlayer::Remove (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Remove (%p)\n", mplayer);
	
	pthread_mutex_lock (&instance_mutex);
	
	// If there's no audio player, there's nothing to remove either
	if (instance != NULL)
		instance->AddWork (mplayer, ActionRemove);
		
	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::RemoveInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::RemoveInternal (%p)\n", mplayer);
	
	for (guint32 i = 0; i < list_count; i++) {
		if (list [i]->GetMediaPlayer () == mplayer) {
			// printf ("AudioPlayer::Remove (%p): removing... (node: %p)\n", list [i]->mplayer, list [i]);
			delete list [i];

			for (guint32 k = i + 1; k < list_count; k++)
				list [k - 1] = list [k];
			
			list [list_count - 1] = NULL;
			list_count--;

			break;
		}
	}

	UpdatePollList ();

	// We don't stop the audio thread when we reach 0 audio nodes since
	// this may cause a lot of threads being created if a lot of
	// mediaelements/media are added and removed.
	// TODO: Add a timeout, 10 seconds for instance, 
	// after which the audio thread is stopped if no more audio node have been created.
}

void
AudioPlayer::Play (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Play (%p)\n", mplayer);
	
	pthread_mutex_lock (&instance_mutex);
	
	if (instance == NULL)
		instance = new AudioPlayer ();

	instance->AddWork (mplayer, ActionPlay);
	
	pthread_mutex_unlock (&instance_mutex);
}


void
AudioPlayer::PlayInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::PlayInternal (%p)\n", mplayer);

	AudioNode *node;

	node = Find (mplayer);
	
	if (node != NULL) {
		LOG_AUDIO  ("AudioPlayer::PlayInternal (): Playing %p\n", node);
		node->state = Playing;
		UpdatePollList ();
	}
}

void
AudioPlayer::WaitForData (AudioNode *node)
{
	LOG_AUDIO ("AudioPlayer::WaitForData (%p)\n", node);

	// This method should be called with the lock held in the worker loop.
	node->state = WaitingForData;
	UpdatePollList ();
}

AudioPlayer::AudioNode*
AudioPlayer::Find (MediaPlayer *mplayer)
{
	AudioNode *result = NULL;
	
	for (guint32 i = 0; i < list_count; i++) {
		if (list [i]->GetMediaPlayer () == mplayer) {
			result = list [i];
			break;
		}	
	}
	
	return result;
}

void
AudioPlayer::UpdatePollList ()
{
	int current;

	/*
	 * We need to update the list of file descriptors we poll on
	 * to only include audio nodes which are playing.
	 */
	
	ndfs = 1;
	for (guint32 i = 0; i < list_count; i++) {
		if (list [i]->state == Playing)
			ndfs += list [i]->ndfs;
	}
	
	g_free (udfs);
	udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
	udfs [0].fd = fds [0];
	udfs [0].events = POLLIN;

	current = 1;
	for (guint32 i = 0; i < list_count; i++) {
		if (list [i]->state == Playing) {
			memcpy (&udfs [current], list [i]->udfs, list [i]->ndfs * sizeof (pollfd));
			current += list[i]->ndfs;
		}
	}
}

void*
AudioPlayer::Loop (void *data)
{
	((AudioPlayer *) data)->Loop ();
	return NULL;
}

void
AudioPlayer::Loop ()
{
	AudioListNode *work_node = NULL;
	AudioNode *current = NULL;
	guint32 current_index = 0;
	bool shutdown = false;

	// Keep track of how many of the audio nodes actually played something
	// If none of the nodes played anything, then we poll until something happens.
	int pc = 0; // The number of consecutive nodes which haven't played anything
	int lc = 0; // Save a copy of the list count for ourselves to avoid some locking.
	
	LOG_AUDIO ("AudioPlayer: entering audio loop.\n");

	while (true) {
		while ((work_node = (AudioListNode *) work.Pop ()) != NULL) {
			switch (work_node->action) {
			case ActionShutdown:
				// Shutdown is always the last item in the work list
				ShutdownInternal ();
				shutdown = true;
				delete this;
				break;
			case ActionPlay:
				PlayInternal (work_node->mplayer);
				break;
			case ActionPause:
				PauseInternal (work_node->mplayer, true);
				break;
			case ActionRestart:
				PauseInternal (work_node->mplayer, false);
				break;
			case ActionStop:
				StopInternal (work_node->mplayer);
				break;
			case ActionDrain:
				DrainInternal (work_node->mplayer);
				break;
			case ActionRemove:
				RemoveInternal (work_node->mplayer);
				break;
			case ActionAdd:
				AddInternal (work_node->mplayer);
				break;
			default:
				g_warning ("AudioPlayer::Loop () Unhandled action: %i\n", work_node->action);
				break;
			}
			delete work_node;
			pc = 0;
			
			if (shutdown)
				break;
		}

		if (shutdown)
			break;

		current = NULL;
		lc = list_count;
		if (list_count > 0) {
			// Get the next node in the list
			if (current_index >= list_count)
				current_index = 0;
			current = list [current_index];
			current_index++;
		}

		pc++;

		// Play something from the node we got
		if (current != NULL) {
			// Check if we're waiting for data and there's already more data available.
			if (current->state == WaitingForData && current->GetMediaPlayer ()->audio.queue.LinkedList ()->First () != NULL) {
				current->state = Playing;
				UpdatePollList ();
			}

			if (current->state == Playing) {
				if (current->Play ())
					pc = 0;
			}
		}
		
		
		if (lc < pc) {
			// None of the audio nodes in the list played anything
			// (or there are no audio nodes), so wait for something
			// to happen. We handle spurious wakeups correctly, so 
			// there is no find out exactly what happened.

			int result;
			int buffer;

			do {
				pc = 0;
				udfs [0].events = POLLIN;
				udfs [0].revents = 0;
				
				LOG_AUDIO_EX ("AudioPlayer::Loop (): polling... (lc: %i, pc: %i)\n", lc, pc);
				result = poll (udfs, ndfs, 10000); // Have a timeout of 10 seconds, just in case something goes wrong.
				LOG_AUDIO_EX ("AudioPlayer::Loop (): poll result: %i, fd: %i, fd [0].revents: %i, errno: %i, err: %s, ndfs = %i, shutdown: %i\n", result, udfs [0].fd, (int) udfs [0].revents, errno, strerror (errno), ndfs, shutdown);
	
				if (result == 0) { // Timed out
					LOG_AUDIO_EX ("AudioPlayer::Loop (): poll timed out.\n");
				} else if (result < 0) { // Some error poll exit condition
					// Doesn't matter what happened (happens quite often due to interrupts)
					LOG_AUDIO_EX ("AudioPlayer::Loop (): poll failed: %i (%s)\n", errno, strerror (errno));
				} else { // Something woke up the poll
					if (udfs [0].revents & POLLIN) {
						// We were asked to wake up by the audio player
						// Read whatever was written into the pipe so that the pipe doesn't fill up.
						read (udfs [0].fd, &buffer, sizeof (int));
						LOG_AUDIO_EX ("AudioPlayer::Loop (): woken up by ourselves.\n");
					} else {
						// Something happened on any of the audio streams
					}
				}
			} while (result == -1 && errno == EINTR);
		}
	}
			
	LOG_AUDIO ("AudioPlayer: exiting audio loop.\n");
}

void
AudioPlayer::Drain (MediaPlayer *mplayer)
{
	pthread_mutex_lock (&instance_mutex);
	
	if (instance != NULL)
		instance->AddWork (mplayer, ActionDrain);
	
	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::DrainInternal (MediaPlayer *mplayer)
{
	AudioNode *node;

	LOG_AUDIO ("AudioPlayer::DrainInternal (%p)\n", mplayer);
	
	node = Find (mplayer);
	
	if (node == NULL)
		return;
	
	if (node->first_buffer) {
		g_free (node->first_buffer);
		node->first_buffer = NULL;
	}

	node->first_used = 0;
	node->first_size = 0;
	node->first_pts = 0;
}

void
AudioPlayer::Pause (MediaPlayer *mplayer, bool value)
{
	pthread_mutex_lock (&instance_mutex);
	
	if (instance != NULL)
		instance->AddWork (mplayer, value ? ActionPause : ActionRestart);
	
	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::PauseInternal (MediaPlayer *mplayer, bool value)
{
	AudioNode *node;
	int err = 0;
	
	LOG_AUDIO ("AudioPlayer::PauseInternal (%p, %i)\n", mplayer, value);
	
	node = Find (mplayer);
	
	if (node == NULL)
		return;
	
	if (value != (node->state == Paused)) {
		if (value) {
			LOG_AUDIO  ("AudioPlayer::PauseInternal (), setting node %p's state to Paused\n", node);
			node->state = Paused;
			// We need to stop the pcm
			if (snd_pcm_state (node->pcm) == SND_PCM_STATE_RUNNING) {
				// FIXME:
				// Alsa provides a pause method, but it's hardware
				// dependent (and it doesn't work on my hardware),
				// so just drop all the data we've put into the pcm
				// device. This means that for the moment when we resume
				// we will skip all the data we drop here.
				err = snd_pcm_drop (node->pcm);
				if (err < 0) {
					fprintf (stderr, "AudioPlayer::Pause (%p, %s): Could not stop/drain pcm: %s\n", mplayer, value ? "true" : "false", snd_strerror (err)); 
				}
			}
		} else {
			node->state = Playing;
		}

	}

	UpdatePollList ();
}

void
AudioPlayer::Stop (MediaPlayer *mplayer)
{
	pthread_mutex_lock (&instance_mutex);
	
	if (instance != NULL)
		instance->AddWork (mplayer, ActionStop);
	
	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::StopInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::StopInternal (%p)\n", mplayer);
	
	PauseInternal (mplayer, true);
	DrainInternal (mplayer);
}

void
AudioPlayer::WakeUp ()
{
	int result;
		
	LOG_AUDIO_EX ("AudioPlayer::WakeUp ().\n");
	
	if (instance == NULL) {
		printf ("AudioPlayer::WakeUp (): Nothing to wake up.\n");
		return;
	}
	
	// Write until something has been written.
	// This should method should only be executed on the main thread.
	do {
		result = write (instance->fds [1], "c", 1);
	} while (result == 0);
	
	if (result == -1)
		fprintf (stderr, "AudioPlayer::WakeUp (): Could not wake up audio thread: %s\n", strerror (errno));
		
	LOG_AUDIO_EX ("AudioPlayer::WakeUp (): thread should now wake up (or have woken up already).\n");
	
}

/*
 * AudioPlayer::AudioListNode
 */

AudioPlayer::AudioListNode::AudioListNode (MediaPlayer *mplayer, AudioAction action)
{
	if (mplayer)
		mplayer->ref ();
	this->mplayer = mplayer;
	this->action = action;
}

AudioPlayer::AudioListNode::~AudioListNode ()
{
	if (mplayer)
		mplayer->unref ();
}
 

/*
 * AudioPlayer::AudioNode
 */

#define AUDIO_HW_DEBUG 0
#define MOON_AUDIO_FORMAT SND_PCM_FORMAT_S16

bool
AudioPlayer::AudioNode::SetupHW ()
{
	bool result = false;
	bool rw_available = false;
	bool mmap_available = false;
	
	snd_pcm_hw_params_t *params = NULL;
	guint32 buffer_time = 500000; // request 0.5 seconds of buffer time.
	int err = 0;
	int dir = 0;
	unsigned int rate = sample_rate;
	unsigned int actual_rate = rate;

#if AUDIO_HW_DEBUG
	snd_output_t *output = NULL;
	err = snd_output_stdio_attach (&output, stdout, 0);
	if (err < 0) {
		fprintf(stderr, "AudioNode::SetupHW (): Could not create alsa output: %s\n", snd_strerror (err));
	}
#endif

	err = snd_pcm_hw_params_malloc (&params);
	if (err < 0) {
		fprintf(stderr, "AudioNode::SetupHW (): Audio HW setup failed (malloc): %s\n", snd_strerror (err));
		return false;
	}

	// choose all parameters
	err = snd_pcm_hw_params_any (pcm, params);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (no configurations available): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
#if AUDIO_HW_DEBUG
	if (output != NULL) {
		printf ("AudioNode::SetupHW (): hw configurations:\n");
		snd_pcm_hw_params_dump (params, output);
	}
#endif
	
	// enable software resampling
	err = snd_pcm_hw_params_set_rate_resample (pcm, params, 1);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (could not enable resampling): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
	// test for available transfer modes
	if (!(moonlight_flags & RUNTIME_INIT_AUDIO_NO_MMAP)) {
		err = snd_pcm_hw_params_test_access (pcm, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
		if (err < 0) {
			fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup: MMAP access not supported (%s), will check if RW access is supported\n", snd_strerror (err));
			} else {
			mmap_available = true;
		}
	} else {
		LOG_AUDIO ("AudioNode::SetupHW (): Not checking for MMAP access, disabled with environment variable.\n");
	}
	
	err = snd_pcm_hw_params_test_access (pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup: RW access not supported (%s), don't know any other access modes to try.\n", snd_strerror (err));			
	} else {
		rw_available = true;
	}
	
	if (mmap_available) {
		mmap = true;
	} else if (rw_available) {
		mmap = false;
	} else {
		goto cleanup;
	}

#if DEBUG
	printf ("AudioNode::SetupHW (): Audio HW setup: using %s access mode.\n", mmap ? "MMAP" : "RW");
#endif

	// set transfer mode (mmap or rw in our case)
	err = snd_pcm_hw_params_set_access (pcm, params, mmap ? SND_PCM_ACCESS_MMAP_INTERLEAVED : SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (access type not available for playback): %s\n", snd_strerror (err));
		goto cleanup;
	}

	// set audio format
	err = snd_pcm_hw_params_set_format (pcm, params, MOON_AUDIO_FORMAT);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (sample format not available for playback): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
	// set channel count
	err = snd_pcm_hw_params_set_channels (pcm, params, channels);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (channels count %i not available for playback): %s\n", channels, snd_strerror (err));
		goto cleanup;
	}
	
	// set sample rate
	err = snd_pcm_hw_params_set_rate_near (pcm, params, &actual_rate, 0);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (sample rate %i Hz not available for playback): %s\n", rate, snd_strerror (err));
		goto cleanup;
	} else if (actual_rate != rate) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (sample rate %i Hz not available for playback, only got %i Hz).\n", rate, actual_rate);
		goto cleanup;
	}
	
	// set the buffer time
	err = snd_pcm_hw_params_set_buffer_time_near (pcm, params, &buffer_time, &dir);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror (err));
		goto cleanup;
	}

	// write the parameters to device
	err = snd_pcm_hw_params (pcm, params);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (unable to set hw params for playback: %s)\n", snd_strerror (err));
		goto cleanup;
	}
	
#if AUDIO_HW_DEBUG
	printf ("AudioNode::SetupHW (): succeeded\n");
	if (output != NULL) {
		snd_pcm_hw_params_dump (params, output);
	}
#endif

	LOG_AUDIO ("AudioNode::SetupHW (): hardware pause support: %s\n", snd_pcm_hw_params_can_pause (params) == 0 ? "no" : "yes"); 

	result = true;
	
cleanup:
	snd_pcm_hw_params_free (params);
	
	return result;
}

bool
AudioPlayer::AudioNode::XrunRecovery (int err)
{
	switch (err) {
	case -EPIPE: // under-run
		err = snd_pcm_prepare (pcm);
		if (err < 0)
			fprintf (stderr, "AudioPlayer: Can't recover from underrun, prepare failed: %s.\n", snd_strerror (err));
		break;
	case -ESTRPIPE:
		while ((err = snd_pcm_resume (pcm)) == -EAGAIN) {
			//printf ("XrunRecovery: waiting for resume\n");
			sleep (1); // wait until the suspend flag is released
		}
		if (err >= 0)
			break;

		err = snd_pcm_prepare (pcm);
		if (err < 0)
			fprintf (stderr, "AudioPlayer: Can't recover from suspend, prepare failed: %s.\n", snd_strerror (err));

		break;
	default:
		fprintf (stderr, "AudioPlayer: Can't recover from underrun: %s\n", snd_strerror (err));
		break;
	}
	
	return err >= 0;
}

bool
AudioPlayer::AudioNode::PreparePcm (snd_pcm_sframes_t *avail)
{
	snd_pcm_state_t state = snd_pcm_state (pcm);
	snd_pcm_sframes_t pending_frames;
	gint32 period_size = sample_size;
	int err;
	
	switch (state) {
	case SND_PCM_STATE_XRUN:
		LOG_AUDIO ("AudioNode::PreparePcm (): SND_PCM_STATE_XRUN.\n");

		if (!XrunRecovery (-EPIPE))
			return false;

		started = false;
		break;
	case SND_PCM_STATE_SUSPENDED:
		if (!XrunRecovery (-ESTRPIPE))
			return false;
		break;
	case SND_PCM_STATE_SETUP:
		if (!XrunRecovery (-EPIPE))
			return false;

		started = false;
		break;
	case SND_PCM_STATE_RUNNING:
		started = true; // We might have gotten started automatically after writing a certain number of samples.
	case SND_PCM_STATE_PREPARED:
		break;
	case SND_PCM_STATE_PAUSED:
	case SND_PCM_STATE_DRAINING:
	default:
		LOG_AUDIO ("AudioNode::PreparePcm (): state: %s (prepare failed)\n", snd_pcm_state_name (state));
		return false;
	}
	
	if (!mmap) {
		err = snd_pcm_delay (pcm, &pending_frames);
		if (err < 0) {
			//fprintf (stderr, "AudioPlayer: could not get delay from audio hw: %s\n", snd_strerror (err));
			return false;
		}
		if (pending_frames < 0) {
			*avail = -EPIPE; // underrun
		} else if ((snd_pcm_sframes_t) buffer_size < pending_frames) {
			*avail = 0;
		} else {
			*avail = buffer_size - pending_frames;
		}
	} else {
		*avail = snd_pcm_avail_update (pcm);
	}

	if (*avail < 0) {
		if (!XrunRecovery (*avail))
			return false;

		started = false;
		return false;
	}

	if (*avail < period_size) {
		if (!started) {
			LOG_AUDIO ("AudioPlayer::PreparePcm (): starting pcm (period size: %i, available: %li)\n", period_size, *avail);
			err = snd_pcm_start (pcm);
			if (err < 0) {
				fprintf (stderr, "AudioPlayer: Could not start pcm: %s\n", snd_strerror (err));
				return false;
			}
			started = true;
		} else {
			return false;
		}
		return false;
	}

	LOG_AUDIO ("AudioPlayer::PreparePcm (): Prepared, avail: %li, started: %i\n", *avail, (int) started);

	return true;
}

bool
AudioPlayer::AudioNode::Play ()
{
	LOG_AUDIO_EX ("AudioPlayer::AudioNode::Play () state: %i\n", state);

	bool result = false;	
	Audio *audio = &mplayer->audio;
	snd_pcm_channel_area_t *areas = NULL;
	snd_pcm_uframes_t offset = 0, frames, size;
	snd_pcm_sframes_t avail, commitres;
	int err = 0;
	
	gint32 bpf = snd_pcm_format_width (MOON_AUDIO_FORMAT) / 8 * channels; // bytes per frame
	gint32 steps [channels];
	gint32 count;
	gint16 *samples [channels];
	gint16 *outptr;
	
	if (state != Playing)
		return false;

	if (!PreparePcm (&avail))
		return false;
	
	LOG_AUDIO_EX ("AudioPlayer::AudioNode::Play (): entering play loop, avail: %lld, sample size: %i\n", (gint64) avail, (int) sample_size);

	// Set the volume
	gint32 volume = audio->volume * 8192;
	gint32 volumes [channels]; // channel #0 = left, #1 = right 
	
	// FIXME: Can we get audio with channels != 2?

	if (audio->muted) {
		volumes [0] = volumes [1] = 0;
	} else 	if (audio->balance < 0.0) {
		volumes [0] = volume;
		volumes [1] = (1.0 + audio->balance) * volume;
	} else if (audio->balance > 0.0) {
		volumes [0] = (1.0 - audio->balance) * volume;
		volumes [1] = volume;
	} else {
		volumes [0] = volumes [1] = volume;
	}
	
	size = sample_size;
	
	while (size > 0 && state == Playing) {
		frames = size;
		
		if (mmap) {
			err = snd_pcm_mmap_begin (pcm, (const snd_pcm_channel_area_t** ) &areas, &offset, &frames);
			if (err < 0) {
				if (!XrunRecovery (err)) {
					fprintf (stderr, "AudioPlayer: could not get mmapped memory: %s\n", snd_strerror (err));
					return result;
				}
				started = false;
			}
		} else {
			// Fake the mmap api for the rest of the code
			frames = avail;
			offset = 0;
			areas = (snd_pcm_channel_area_t *) g_malloc0 (sizeof (snd_pcm_channel_area_t) * 2);
			areas [0].addr = g_malloc (frames * bpf);
			areas [1].addr = ((guint8 *) areas [0].addr) + (bpf / 2);
			areas [0].first = areas [1].first = 0;
			areas [0].step = areas [1].step = bpf * 8;
		}

		count = frames;
		
		for (guint32 channel = 0; channel < channels; channel++) {
			// number of 16bit samples between each sample
			steps [channel] = areas [channel].step / 16;
			// pointer to the first sample to write to
			samples [channel] = ((gint16*) areas [channel].addr) + (areas [channel].first / 16);
			samples [channel] += offset * steps [channel];
		}

#if DEBUG
		//printf ("after mmap begin: size: %i, sample_size: %i, offset: %i, frames: %i, err: %i, bpf: %i\n", size, sample_size, offset, frames, err, bpf);
/*
		for (int i = 0; i < channels; i++) {
			printf ("  %i: addr: %p, first: %u, step: %u\n", i, areas [i].addr, areas [i].first, areas [i].step);
			printf ("  steps [%i] = %i, samples [%i] = %p\n", i, steps [i], i, samples [i]); 
			printf ("  volume [%i] = %i\n", i, volumes [i]); 
		}
*/
		/* fill the channel areas */
		//printf ("play: writing %i samples @ %i Hz = %.2f seconds (%.2f pts per sample)\n", count, mplayer->audio->stream->sample_rate, 
		//	count / (double) mplayer->audio->stream->sample_rate, 10000000.0 / mplayer->audio->stream->sample_rate);
#endif
		
		bool update_target_pts = false;
		while (count-- > 0) {
			if (first_size == 0 || first_used + bpf > first_size) {
				// FIXME: if the buffer doesn't have a size which is a multiple of the bits per frame, we currently drop the extra bits.
				GetNextBuffer ();
				if (first_size == 0 || first_used + bpf > first_size) {
					if ((gint32) frames <= count + 1) {
						if (mmap) {
							commitres = snd_pcm_mmap_commit (pcm, offset, 0);
						} else {
							g_free (areas [0].addr);
							g_free (areas);
						}
						return result; // We haven't read anything
					}
					
					// Set 'frames' to how many frames we got
					frames = frames - (count + 1);
					break;
				}
				//printf ("play: sent_pts = %llu (from frame, old pts: %llu, diff: %lld, time: %lld milliseconds), samples sent: %i\n", first_pts, 
				//	sent_pts, first_pts - sent_pts, (gint64) MilliSeconds_FromPts ((gint64) first_pts - (gint64) sent_pts), sent_samples);
				sent_pts = first_pts;
				sent_samples = 0;
				update_target_pts = true;
			} else {
				sent_pts = first_pts + sent_samples * 10000000 / sample_rate;
			}

			outptr = (gint16*) &first_buffer [first_used];
			first_used += bpf;
			sent_samples++;
			for (guint32 channel = 0; channel < channels; channel++) {
				gint32 value = (outptr [channel] * volumes [channel]) >> 13;
				*(samples[channel]) = (gint16) CLAMP (value, -32768, 32767);
				samples[channel] += steps[channel];
			}
		}
		if (mmap) {
			commitres = snd_pcm_mmap_commit (pcm, offset, frames);
			if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
				if (!XrunRecovery (commitres >= 0 ? -EPIPE : commitres)) {
					fprintf (stderr, "AudioPlayer: could not commit mmapped memory: %s\n", snd_strerror(err));
					return result;
				}
				started = false;
			}
		} else {
			// TODO: We should use non-blocking mode and detect when the write fails due to 
			// a full buffer (in order to play nicely when we're playing more than one audio source
			// at the same time).
			commitres = snd_pcm_writei (pcm, areas [0].addr, frames);
			g_free (areas [0].addr);
			g_free (areas);
			if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
				if (!XrunRecovery (commitres >= 0 ? -EPIPE : commitres)) {
					fprintf (stderr, "AudioPlayer: could not write audio data: %s\n", snd_strerror(err));
					return result;
				}
				started = false;
			}
		}
		size -= frames;
		
		if (update_target_pts || (sent_pts - updated_pts) > 10000) {
			snd_pcm_sframes_t delay;
			guint64 pts = sent_pts;
			guint64 delay_pts;
			err = snd_pcm_delay (pcm, &delay);
			if (err >= 0) {
				delay_pts = delay * (guint64) 10000000 / sample_rate;
				if (delay_pts > pts)
					pts = 0;
				else
					pts -= delay_pts;
			}
			mplayer->SetTargetPts (pts);
			updated_pts = pts;
		}
		
		result = true;
		if (!mmap)
			break;
	}
	
	return result;
}

bool
AudioPlayer::AudioNode::Initialize ()
{
	int result;
	AudioStream *stream;
	
	LOG_AUDIO ("AudioNode::Initialize (%p)\n", this);
	
	if (moonlight_flags & RUNTIME_INIT_DISABLE_AUDIO) {
		LOG_AUDIO ("Moonlight: audio is disabled.\n");
		return false;
	}
	
	if (pcm != NULL) {
		fprintf (stderr, "AudioNode::Initialize (): trying to initialize an audio stream more than once.\n");
		return false;
	}
	
	stream = mplayer->audio.stream;
	
	if (stream == NULL) {
		// Shouldn't really happen, but handle this case anyway.
		fprintf (stderr, "AudioNode::Initialize (): trying to initialize an audio device, but there's no audio to play.\n");
		return false;
	}
	
	sample_rate = stream->sample_rate;
	channels = stream->channels;
	
	// Open a pcm device
	result = snd_pcm_open (&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (result != 0) {
		fprintf (stderr, "AudioNode::Initialize (): cannot open audio device: %s\n", snd_strerror (result));
		pcm = NULL;
		return false;
	}

	// Configure the hardware
	if (!SetupHW ()) {
		fprintf (stderr, "AudioNode::Initialize (): could not configure hardware for audio playback\n");
		Close ();
		return false;
	}
	
	result = snd_pcm_get_params (pcm, &buffer_size, &sample_size);
	if (result != 0) {
		fprintf (stderr, "AudioNode::Initialize (): error while getting parameters: %s\n", snd_strerror (result));
		Close ();
		return false;
	}

	// Get the file descriptors to poll on
	ndfs = snd_pcm_poll_descriptors_count (pcm);
	if (ndfs <= 0) {
		fprintf (stderr, "AudioNode::Initialize(): Unable to initialize audio for playback (could not get poll descriptor count).\n");
		Close ();
		return false;
	}

	udfs = (pollfd *) g_malloc0 (sizeof (pollfd) * ndfs);
	if (snd_pcm_poll_descriptors (pcm, udfs, ndfs) < 0) {
		fprintf (stderr, "AudioNode::Initialize (): Unable to initialize audio for playback (could not get poll descriptors).\n");
		g_free (udfs);
		udfs = NULL;
		Close ();
		return false;
	}
	
	LOG_AUDIO ("AudioNode::Initialize (%p): Succeeded. Buffer size: %lu, sample size (period size): %lu\n", this, buffer_size, sample_size);
	
	return true;
}

AudioPlayer::AudioNode::AudioNode (MediaPlayer *mplayer)
{
	this->mplayer = mplayer;
	this->mplayer->ref ();
	pcm = NULL;
	sample_size = 0;
	buffer_size = 0;
	state = Stopped;
	
	first_buffer = NULL;
	first_used = 0;
	first_size = 0;
	first_pts = 0;

	sent_pts = 0;
	updated_pts = 0;
	started = false;

	udfs = NULL;
	ndfs = 0;
	
	mmap = false;
}

AudioPlayer::AudioNode::~AudioNode ()
{
	LOG_AUDIO ("AudioNode::~AudioNode ()\n");

	Close ();
	
	mplayer->unref ();
}

bool
AudioPlayer::AudioNode::GetNextBuffer ()
{
	MediaFrame *frame = NULL;
	Packet *packet = NULL;
	
	if (mplayer->GetEof ())
		return false;

	if (first_buffer) {
		g_free (first_buffer);
		first_buffer = NULL;
	}

	first_used = 0;
	first_size = 0;
	first_pts = 0;

	packet = (Packet*) mplayer->audio.queue.Pop ();
	
	if (packet == NULL) {
		pthread_mutex_lock (&instance_mutex);
		if (instance)
			instance->WaitForData (this);
		pthread_mutex_unlock (&instance_mutex);
		return false;
	}
	
	frame = packet->frame;
	
	if (frame->event == FrameEventEOF) {
		if (!started) {
			LOG_AUDIO ("AudioPlayer::GetNextBuffer (): starting pcm as we're out of data but never started");
			int err = snd_pcm_start (pcm);
			if (err < 0) {
				fprintf (stderr, "AudioPlayer: Could not start pcm: %s\n", snd_strerror (err));
			}
			started = true;
		}
		mplayer->AudioFinished ();
		mplayer->SetEof (true);
		return false;
	}
	
	if (!frame->IsDecoded ())
		frame->stream->decoder->DecodeFrame (frame);
		
	mplayer->EnqueueFramesAsync (1, 0);
				
	first_used = 0;
	first_buffer = frame->buffer;
	first_size = frame->buflen;
	first_pts = frame->pts;
	frame->buffer = NULL;
	
	delete packet;
	
	return true;
}

void
AudioPlayer::AudioNode::Close ()
{
	LOG_AUDIO ("AudioNode::Close () %p\n", this);
		
	if (pcm != NULL) {
		snd_pcm_close (pcm);
		pcm = NULL;
	}
	
	g_free (udfs);
	udfs = NULL;
	
	g_free (first_buffer);
	first_buffer = NULL;
}
