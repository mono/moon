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

#include <stdlib.h>
#include <glib.h>

#include "clock.h"
#include "mplayer.h"
#include "pipeline.h"
#include "runtime.h"
#include "list.h"
#include "media.h"

#define DEBUG_ADVANCEFRAME 0
#define LOG_MEDIAPLAYER(...)// printf (__VA_ARGS__);

// This one prints out spew on every frame
//#define VERBOSE_DEBUG
#ifdef VERBOSE_DEBUG
#define LOG_MEDIAPLAYER_EX(...) printf (__VA_ARGS__);
#else
#define LOG_MEDIAPLAYER_EX(...)
#endif


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

	media = NULL;
	audio = NULL;
	
	Initialize ();
}

MediaPlayer::~MediaPlayer ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::~MediaPlayer (), id=%i\n", GET_OBJ_ID (this));
	Close (true);
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

#ifdef VERBOSE_DEBUG
static const char *media_type_name[] = {
	"None", "Video", "Audio", "Marker"
};
#endif

MediaResult
MediaPlayer::FrameCallback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *player = element->GetMediaPlayer ();
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame ? frame->stream : NULL;
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::FrameCallback (closure=%p)\n"
			    "\tstate: %d, frame: %p, pts: %llu = %llu, type: %s\n"
			    "\taudio packets: ?, video packets: %d\n",
			    closure, player->state, closure->frame, frame ? frame->pts : 0,
			    frame ? MilliSeconds_FromPts (frame->pts) : 0, media_type_name[stream->GetType ()],
			    player->video.queue.Length ());
	
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
		if (player->audio != NULL) {
			player->audio->AppendFrame (frame);
		} else {
			//fprintf (stderr, "MediaPlayer::FrameCallback (): Got audio frame, but there is no audio player.\n");
		}
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

	SetBit (AudioEnded);
	CheckFinished ();
}

void
MediaPlayer::VideoFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::VideoFinished ()\n");
	
	SetBit (VideoEnded);
	CheckFinished ();
}

void
MediaPlayer::NotifyFinishedCallback (EventObject *player)
{
	((MediaPlayer *) player)->NotifyFinished ();
}

void
MediaPlayer::NotifyFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::NotifyFinished (): Element: %p\n", element);
	
	Stop ();
	if (element)
		element->MediaFinished ();
}

void
MediaPlayer::CheckFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::CheckFinished (), HasVideo: %i, VideoEnded: %i, HasAudio: %i, AudioEnded: %i\n",
		HasVideo (), GetBit (VideoEnded), HasAudio (), GetBit (AudioEnded));
		
	if (HasVideo () && !GetBit (VideoEnded))
		return;
		
	if (HasAudio () && !GetBit (AudioEnded))
		return;
	
	AddTickCallSafe (NotifyFinishedCallback);
}

void
MediaPlayer::AudioFailed (AudioSource *source)
{
	if (this->audio == source) {
		AudioPlayer::Remove (this->audio);
		this->audio = NULL;
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
			
			media->GetNextFrameAsync (closure, audio->GetStream (), states);
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
	VideoStream *vstream = NULL;
	AudioStream *astream = NULL, *astream2 = NULL;
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);
		encoding = stream->GetDecoder (); //stream->codec;
		
		if (encoding == NULL)
			continue; // No encoding was found for the stream.
		
		switch (stream->GetType ()) {
		case MediaTypeAudio:
			audio_stream_count++;
			astream2 = (AudioStream *) stream;

			if (astream == NULL || astream->GetBitRate () < astream2->GetBitRate ())
				astream = astream2;

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

	if (astream != NULL) {
		audio = AudioPlayer::Add (this, astream);
		if (audio != NULL) {
			// Only select the audio stream if we can actually play it
			astream->SetSelected (true);
			audio->ref ();
		}
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

	// Clear out any state, bits, etc
	state = (PlayerState) 0;
	// Set initial states and bits
	SetState (Stopped);
	SetBit (SeekSynched);
	SetBit (CanSeek);
	SetBit (CanPause);
	
	start_time = 0;
	start_pts = 0;
	current_pts = 0;
	target_pts = 0;
	first_live_pts = G_MAXULONG;
	
	audio_stream_count = 0;
	height = 0;
	width = 0;
}

void
MediaPlayer::Close (bool dtor)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Close ()\n");

	if (audio) {
		AudioPlayer::Remove (audio);
		audio->unref ();
		audio = NULL;
	}

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

	LOG_MEDIAPLAYER_EX ("MediaPlayer::RenderFrame (%p), buflen: %i, buffer: %p, IsPlanar: %i\n", frame, frame->buflen, frame->buffer, frame->IsPlanar ());
	
	if (!frame->IsDecoded ()) {
		fprintf (stderr, "MediaPlayer::RenderFrame (): Trying to render a frame which hasn't been decoded yet.\n");
		return;
	}
	
	if (!frame->IsPlanar ()) {
		// Just copy the data
		guint32 stride = cairo_image_surface_get_stride (video.surface);
		for (int i = 0; i < height; i++)
			memcpy (video.rgb_buffer + stride * i, frame->buffer + i * width * 4, width * 4);
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
	bool result = false;
	
#if DEBUG_ADVANCEFRAME
	static int frames_per_second = 0;
	static int skipped_per_second = 0;
	static guint64 last_second_pts = 0;
	int skipped = 0;
	
	printf ("MediaPlayer::AdvanceFrame () state: %i, current_pts = %llu, IsPaused: %i, IsSeeking: %i, VideoEnded: %i, AudioEnded: %i, HasVideo: %i, HasAudio: %i\n", 
		state, current_pts, IsPaused (), IsSeeking (), GetBit (VideoEnded), GetBit (AudioEnded), HasVideo (), HasAudio ());
#endif

	RemoveBit (LoadFramePending);
	
	if (IsPaused ())
		return false;
	
	if (IsSeeking ())
		return false;
	
	if (GetBit (VideoEnded))
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
		
		/*
		printf ("MediaPlayer::AdvanceFrame (): determined target_pts to be: %llu = %llu ms, elapsed_pts: %llu = %llu ms, start_time: %llu = %llu ms\n",
			target_pts, MilliSeconds_FromPts (target_pts), elapsed_pts, MilliSeconds_FromPts (elapsed_pts), start_time, MilliSeconds_FromPts (start_time));
		*/
	}
	
	this->target_pts = target_pts;
		
	target_pts_start = target_pts_delta > target_pts ? 0 : target_pts - target_pts_delta;
	target_pts_end = target_pts + target_pts_delta;
	
	if (current_pts >= target_pts_end && GetBit (SeekSynched) && !(HasAudio () && GetBit (AudioEnded))) {
#if DEBUG_ADVANCEFRAME
		printf ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %llu, target_pts: %llu, delta: %llu, diff: %lld (%lld ms)).\n",
			current_pts, target_pts, target_pts_delta, current_pts - target_pts, MilliSeconds_FromPts (current_pts - target_pts));
#endif
		return false;
	}

#if DEBUG_ADVANCEFRAME
	printf ("MediaPlayer::AdvanceFrame (): target pts: %llu = %llu ms\n", target_pts, MilliSeconds_FromPts (target_pts));
#endif

	while (true) {
		pkt = (Packet *) video.queue.Pop ();
		if (pkt == NULL) {
			if (!HasAudio ())
				SetBufferUnderflow ();
			// If we have audio, we keep playing (and loosing frames) until the audio playing stops due to buffer underflow
			break;
		}
		
		if (pkt->frame->event == FrameEventEOF) {
			if (!HasAudio ()) {
				// Set the target pts to the last pts we showed, since target_pts is what's reported as our current position.
				this->target_pts = current_pts;
			}
			delete pkt;
			VideoFinished ();
			return false;
		}
		
		// always decode the frame or we get glitches in the screen
		frame = pkt->frame;
		stream = frame->stream;
		current_pts = frame->pts;
		update = true;
		result = true;
		
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
				if (current_pts - first_live_pts > duration) {
					AudioFinished ();
					VideoFinished ();
				}
			} else {
				if (current_pts > duration) {
					AudioFinished ();
					VideoFinished ();
				}
			}
			if (GetBit (VideoEnded)) {
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
		
	return result;
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

	if (packet->frame->pts >= GetTargetPts () && packet->frame->buflen > 0) {
		RemoveBit (LoadFramePending);
		RenderFrame (packet->frame);
		element->Invalidate ();
	} else {
		cont = true;
	}
	
	delete packet;
	
	return cont;
}

void
MediaPlayer::Play ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i, IsPlaying: %i, IsSeeking: %i\n", state, IsPlaying (), IsSeeking ());

	if (IsPlaying () && !IsSeeking ())
		return;
	
	SetState (Playing);
	RemoveBit (BufferUnderflow);
	start_time = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
	start_time -= target_pts;

	if (audio)
		audio->Play ();
	
	// Request 10 audio packets from the start, the audio thread will in some cases
	// be able to send more audio samples to the hardware in one call than what's
	// in one single frame, but this depends of course on having more than one 
	// decoded frame available.
	EnqueueFrames (10, 1);

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
	
	if (audio)
		audio->Pause ();

	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i [Done]\n", state);	
}

guint64
MediaPlayer::GetTargetPts ()
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::GetTargetPts (): target_pts: %llu, HasAudio (): %i, audio->GetCurrentPts (): %llu\n", target_pts, HasAudio (), HasAudio () ? audio->GetCurrentPts () : 0);

	guint64 result;
	
	if (HasAudio ())
		result = audio->GetCurrentPts ();
	else
		result = target_pts;
		
	return result;
}

void
MediaPlayer::SeekCallback ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekCallback ()\n");

	// Clear all queues.
	video.queue.Clear (true);
	if (audio != NULL)
		audio->ClearFrames ();
	
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
	video.queue.Clear (true);
	
	// Stop the audio
	if (audio)
		audio->Stop ();

	SetBit (Seeking);
	RemoveBit (AudioEnded);
	RemoveBit (VideoEnded);
		
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

	if (audio)
		audio->Stop ();

	video.queue.Clear (true);
		
	start_time = 0;
	current_pts = 0;
	target_pts = 0;
	SetState (Stopped);
	RemoveBit (AudioEnded);
	RemoveBit (VideoEnded);

	// If we're closing the media player, there is no need to 
	// seek to the beginning, it may even cause crashes if we're
	// closing because the MediaElement is being destructed.
	if (seek_to_start)
		SeekInternal (0);
}

double
MediaPlayer::GetBalance ()
{
	if (audio) {
		return audio->GetBalance ();
	} else {
		fprintf (stderr, "MediaPlayer::GetBalance (): There's no audio source to get the balance from\n");
		return 0.0;
	}
}

void
MediaPlayer::SetBalance (double balance)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetBalance (%f)\n", balance);

	if (balance < -1.0)
		balance = -1.0;
	else if (balance > 1.0)
		balance = 1.0;
	
	if (audio) {
		audio->SetBalance (balance);
	} else {
		//fprintf (stderr, "MediaPlayer::SetBalance (%f): There's no audio source to set the balance\n", balance);
	}
}

double
MediaPlayer::GetVolume ()
{
	if (audio) {
		return audio->GetVolume ();
	} else {
		fprintf (stderr, "MediaPlayer::GetVolume (): There's no audio source to get the volume from\n");
		return 0.0;
	}
}

void
MediaPlayer::SetVolume (double volume)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetVolume (%f)\n", volume);

	if (volume < -1.0)
		volume = -1.0;
	else if (volume > 1.0)
		volume = 1.0;
	
	if (audio) {
		audio->SetVolume (volume);
	} else {
		//fprintf (stderr, "MediaPlayer::SetVolume (%f): There's no audio source to set the volume\n", volume);
	}		
}
	
bool
MediaPlayer::GetMuted ()
{
	if (audio) {
		return audio->GetMuted ();
	} else {
		fprintf (stderr, "MediaPlayer::GetMuted (): There's no audio.\n");
		return false;
	}
}

void
MediaPlayer::SetMuted (bool muted)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetMuted (%i)\n", muted);
	
	if (audio) {
		audio->SetMuted (true);
	} else {
		//fprintf (stderr, "MediaPlayer::SetMuted (%i): There's no audio to mute.\n", muted);
	}
}
