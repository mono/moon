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

#include <config.h>

#include <stdlib.h>
#include <glib.h>

#include "timesource.h"
#include "timemanager.h"
#include "mediaplayer.h"
#include "pipeline.h"
#include "runtime.h"
#include "list.h"
#include "media.h"
#include "mediaelement.h"
#include "debug.h"
#include "playlist.h"

/*
 * MediaPlayer
 */

MediaPlayer::MediaPlayer (MediaElement *el)
	: EventObject (Type::MEDIAPLAYER)
{
	LOG_MEDIAPLAYER ("MediaPlayer::MediaPlayer (%p, id=%i), id=%i\n", el, GET_OBJ_ID (el), GET_OBJ_ID (this));

	VERIFY_MAIN_THREAD;
	
	element = el;

	video_stream = NULL;
	surface = NULL;
	rgb_buffer = NULL;
	buffer_width = 0;
	buffer_height = 0;
	format = MoonPixelFormatRGB32;
	advance_frame_timeout_id = 0;
	seeks = 0;

	media = NULL;
	audio_unlocked = NULL;

	Initialize ();
}

MediaPlayer::~MediaPlayer ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::~MediaPlayer (), id=%i\n", GET_OBJ_ID (this));
	VERIFY_MAIN_THREAD;
}

void
MediaPlayer::Dispose ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Dispose (), id=%i\n", GET_OBJ_ID (this));
	
	VERIFY_MAIN_THREAD;
	
	Close ();
	
	element = NULL;
	
	EventObject::Dispose ();
}

AudioSource *
MediaPlayer::GetAudio ()
{
	AudioSource *result = NULL;

	// Thread-safe
	
	mutex.Lock ();
	if (audio_unlocked != NULL) {
		result = audio_unlocked;
		result->ref ();
	}	
	mutex.Unlock ();

	return result;
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
MediaPlayer::AudioFinishedCallback (EventObject *user_data)
{
	LOG_MEDIAPLAYER ("MediaPlayer::AudioFinishedCallback ()\n");
	VERIFY_MAIN_THREAD;

	MediaPlayer *mplayer = (MediaPlayer *) user_data;
	mplayer->AudioFinished ();
}

void
MediaPlayer::AudioFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::AudioFinished () VideoEnded: %i, AudioEnded: %i AudioSource id: %i\n", GetBit (VideoEnded), GetBit (AudioEnded), GET_OBJ_ID (audio_unlocked));

	// This method must be thread-safe

	if (!Surface::InMainThread ()) {
		AddTickCallSafe (AudioFinishedCallback);
		return;
	}
	
	VERIFY_MAIN_THREAD;
	if (!GetBit (AudioEnded)) {
		SetBit (AudioEnded);
		CheckFinished ();
	}
}

void
MediaPlayer::VideoFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::VideoFinished () VideoEnded: %i, AudioEnded: %i\n", GetBit (VideoEnded), GetBit (AudioEnded));
	VERIFY_MAIN_THREAD;
	
	if (!GetBit (VideoEnded)) {
		SetBit (VideoEnded);
		CheckFinished ();
	}
}

void
MediaPlayer::CheckFinished ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::CheckFinished (), HasVideo: %i, VideoEnded: %i, HasAudio: %i, AudioEnded: %i\n",
		HasVideo (), GetBit (VideoEnded), HasAudio (), GetBit (AudioEnded));
	VERIFY_MAIN_THREAD;
		
	if (HasVideo () && !GetBit (VideoEnded))
		return;
		
	if (HasAudio () && !GetBit (AudioEnded))
		return;
	
	Emit (MediaEndedEvent);
}

void
MediaPlayer::AudioFailed (AudioSource *source)
{
	// This method must be thread-safe
	
	mutex.Lock ();
	if (this->audio_unlocked == source) {
		AudioPlayer::Remove (this->audio_unlocked);
		this->audio_unlocked->unref ();
		this->audio_unlocked = NULL;
	}
	mutex.Unlock ();
}

bool
MediaPlayer::Open (Media *media, PlaylistEntry *entry)
{
	IMediaDecoder *encoding;
	IMediaStream *stream;
	guint64 asx_duration;
	gint32 *audio_stream_index = NULL;
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::Open (%p), current media: %p\n", media, this->media);
	VERIFY_MAIN_THREAD;
	
	Close ();

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
	IMediaDemuxer *demuxer = media->GetDemuxerReffed ();
	VideoStream *vstream = NULL;
	AudioStream *astream = NULL, *astream2 = NULL;
	
	if (demuxer == NULL) {
		fprintf (stderr, "MediaPlayer::Open (): media doesn't have a demuxer.\n");
		return false;
	}

	audio_stream_index = element->GetAudioStreamIndex ();

	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);
		encoding = stream->GetDecoder (); //stream->codec;
		
		if (encoding == NULL)
			continue; // No encoding was found for the stream.
		
		switch (stream->GetType ()) {
		case MediaTypeAudio:
			audio_stream_count++;
			if (audio_stream_index != NULL){
				if (*audio_stream_index == audio_stream_count - 1) {
					astream = (AudioStream *) stream;
				}
			} else {
				astream2 = (AudioStream *) stream;
	
				if (astream == NULL || astream->GetBitRate () < astream2->GetBitRate ())
					astream = astream2;
			}

			break;
		case MediaTypeVideo: 
			vstream = (VideoStream *) stream;

			if (video_stream != NULL && vstream->GetBitRate () < video_stream->GetBitRate ())
				break;

			video_stream = vstream;
			
			height = video_stream->height;
			width = video_stream->width;

			SetVideoBufferSize (width, height);
			
			// printf ("video size: %i, %i\n", video_stream->width, video_stream->height);
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
			LOG_MEDIAPLAYER ("MediaPlayer::Open(): Selected audio stream (%d) properties:\n"
					 "\tchannels: Input: %d Output: %d\n"
					 "\tsample_rate: Input: %d Output: %d\n"
					 "\tbit_rate: Input: %d Output: %d\n"
					 "\tblock_align: Input: %d Output: %d\n"
					 "\tbits_per_sample: Input: %d Output: %d\n"
					 "\tcodec_id: 0x%x\n"
					 "\tduration: %" G_GUINT64_FORMAT "\n"
					 "\textra data size: %d\n",
					 astream->index, astream->GetChannels (), astream->GetOutputChannels (), 
					 astream->GetSampleRate (), astream->GetOutputSampleRate (),
					 astream->GetBitRate (), astream->GetOutputBitRate (),
					 astream->GetBlockAlign (), astream->GetOutputBlockAlign (),
					 astream->GetBitsPerSample (), astream->GetOutputBitsPerSample (),
					 astream->GetCodecId (), astream->GetDuration (), astream->GetExtraDataSize ());
			if (astream->extra_data_size > 0) {
				int n;
				LOG_MEDIAPLAYER ("\textra data: ");
				for (n = 0; n < astream->extra_data_size; n++)
					LOG_MEDIAPLAYER ("[0x%x] ", ((gint8*)astream->extra_data)[n]);
				LOG_MEDIAPLAYER ("\n"); 
			}
			mutex.Lock ();
			this->audio_unlocked = audio;
			mutex.Unlock ();
		}
	}
	if (video_stream != NULL) {
		LOG_MEDIAPLAYER ("MediaPlayer::Open(): Selected Video stream (%d) properties:\n"
					  "\twidth: %d\n"
					  "\theight: %d\n"
					  "\tbits_per_sample: %d\n"
					  "\tbit_rate: %d\n"
					  "\tcodec_id: 0x%x\n"
					  "\tpts_per_frame: %" G_GUINT64_FORMAT "\n"
					  "\tduration: %" G_GUINT64_FORMAT "\n"
					  "\textra data size: %d\n",
					  video_stream->index, video_stream->width, video_stream->height, video_stream->bits_per_sample,
					  video_stream->bit_rate, video_stream->codec_id, video_stream->pts_per_frame,
					  video_stream->duration, video_stream->extra_data_size);
		video_stream->SetSelected (true);
		video_stream->ref ();
			if (video_stream->extra_data_size > 0) {
				int n;
				LOG_MEDIAPLAYER ("\textra data: ");
				for (n = 0; n < video_stream->extra_data_size; n++)
					LOG_MEDIAPLAYER ("[0x%x] ", ((gint8*)video_stream->extra_data)[n]);
				LOG_MEDIAPLAYER ("\n");
			}
	}

	current_pts = 0;
	target_pts = 0;
	start_pts = 0;

	if (entry != NULL) {
		start_pts =  TimeSpan_ToPts (entry->GetStartTime ());
		LOG_MEDIAPLAYER ("MediaPlayer::Open (), setting start_pts to: %" G_GUINT64_FORMAT " (%" G_GUINT64_FORMAT " ms).\n", start_pts, MilliSeconds_FromPts (start_pts));
		// note that we might be re-opening a media which is not at position 0,
		// so it is not possible to optimize away the case where start_pts = 0.
		element->Seek (start_pts, true);

		if (entry->GetIsLive ())		
			SetBit (IsLive);
	}
	
	duration = demuxer->GetDuration ();

	if (entry != NULL && entry->HasInheritedDuration () && entry->GetInheritedDuration ()->HasTimeSpan ()) {
		asx_duration = TimeSpan_ToPts (entry->GetInheritedDuration ()->GetTimeSpan ());
		if (asx_duration < duration || GetBit (IsLive)) {
			duration = asx_duration;
			SetBit (FixedDuration);
		}
	}

	SetBit (LoadFramePending);
	
	media->AddSafeHandler (Media::SeekCompletedEvent, SeekCompletedCallback, this);
	media->SetBufferingTime (element->GetBufferingTime ());
	
	if (HasVideo ()) {
		video_stream->AddSafeHandler (IMediaStream::FirstFrameEnqueuedEvent, FirstFrameEnqueuedCallback, this);
		// We may attach the handler above after the first frame has been queued, 
		// so just execute LoadVideoFrame once right away
		LoadVideoFrame ();
	}
	
	demuxer->unref ();
	
	return true;
}

void
MediaPlayer::SetVideoBufferSize (gint32 width, gint32 height)
{
	gint32 stride;
	
	LOG_MEDIAPLAYER ("MediaPlayer::SetVideoBufferSize (%i, %i). buffer_width: %i, buffer_height: %i\n", width, height, buffer_width, buffer_height);
	VERIFY_MAIN_THREAD;
		
	if (surface) {
		cairo_surface_destroy (surface);
		surface = NULL;
	}
		
	/* NOTE: We only accept RGB32 or RGBA32 data here */
	stride = cairo_format_stride_for_width (format == MoonPixelFormatRGB32 ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32, MAX (width, buffer_width));
	
	if (stride % 64) {
		int remain = stride % 64;
		stride += 64 - remain;
	}
		
	if (width > buffer_width || height > buffer_height) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetVideoBufferSize (): creating new buffer.\n");
		free (rgb_buffer);
		// for conversion to rgb32 format needed for rendering with 16 byte alignment
		if (posix_memalign ((void **)(&rgb_buffer), 16, height * stride)) {
			rgb_buffer = NULL;
			g_warning ("Could not allocate memory for video RGB buffer");
			return;
		}	
		memset (rgb_buffer, 0, height * stride);
		
		buffer_width = width;
		buffer_height = height;
	}
		
	// rendering surface
	LOG_MEDIAPLAYER ("MediaPlayer::SetVideoBufferSize (): creating new surface, width: %i, height: %i, stride: %i\n", width, height, stride);
	/* NOTE: We only accept RGB32 or RGBA32 data here */
	surface = cairo_image_surface_create_for_data (rgb_buffer, format == MoonPixelFormatRGB32 ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32, width, height, stride);
}

void 
MediaPlayer::Initialize ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Initialize ()\n");
	VERIFY_MAIN_THREAD;

	// Clear out any state, bits, etc
	state_unlocked = (PlayerState) 0;
	// Set initial states and bits
	SetState (Stopped);
	SetBit (SeekSynched);
	SetBit (CanSeek);
	SetBit (CanPause);
	
	seeks = 0;
	start_time = 0;
	duration = 0;
	start_pts = 0;
	current_pts = 0;
	target_pts = 0;
	first_live_pts = G_MAXUINT64;
	
	audio_stream_count = 0;
	height = 0;
	width = 0;

	frames_update_timestamp = 0;
	rendered_frames = 0;
	dropped_frames = 0;
	rendered_frames_per_second = 0.0;
	dropped_frames_per_second = 0.0;
}

void
MediaPlayer::Close ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Close ()\n");
	VERIFY_MAIN_THREAD;

	mutex.Lock ();
	if (audio_unlocked) {
		AudioPlayer::Remove (audio_unlocked);
		audio_unlocked->Dispose ();
		audio_unlocked->unref ();
		audio_unlocked = NULL;
	}
	mutex.Unlock ();

	Stop ();
	
	// Reset state back to what it was at instantiation

	if (rgb_buffer != NULL) {
		free (rgb_buffer);
		rgb_buffer = NULL;
	}
	buffer_width = 0;
	buffer_height = 0;

	if (surface != NULL) {
		cairo_surface_destroy (surface);
		surface = NULL;
	}
	
	if (video_stream) {
		video_stream->RemoveSafeHandlers (this);
		video_stream->unref ();
		video_stream = NULL;
	}
	
	if (media) {
		media->unref ();
		media = NULL;
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

	LOG_MEDIAPLAYER_EX ("MediaPlayer::RenderFrame (%p), pts: %" G_GUINT64_FORMAT " ms, buflen: %i, buffer: %p, IsPlanar: %i\n", frame, MilliSeconds_FromPts (frame->pts), frame->buflen, frame->buffer, frame->IsPlanar ());
	VERIFY_MAIN_THREAD;
	
	if (!frame->IsDecoded ()) {
		fprintf (stderr, "MediaPlayer::RenderFrame (): Trying to render a frame which hasn't been decoded yet.\n");
		return;
	}
	
	if ((frame->width > 0 && frame->width != width) || (frame->height > 0 && frame->height != height) || (format != stream->GetDecoder ()->GetPixelFormat ())) {
		LOG_MEDIAPLAYER ("MediaPlayer::RenderFrame () frame width: %i, frame height: %i, stream width: %i, stream height: %i, previous frame width: %i, previous frame height: %i\n",
			frame->width, frame->height, video_stream->width, video_stream->height, width, height);

		if (frame->width > 0)
			width = frame->width;
		if (frame->height > 0)
			height = frame->height;

		format = stream->GetDecoder ()->GetPixelFormat ();

		SetVideoBufferSize (width, height);
	}
	
	if (!frame->IsPlanar ()) {
		// Just copy the data
		guint32 stride = cairo_image_surface_get_stride (surface);
		for (int i = 0; i < height; i++)
			memcpy (rgb_buffer + stride * i, frame->buffer + i * width * 4, width * 4);
		SetBit (RenderedFrame);
		element->MediaInvalidate ();
		return;
	}
	
	if (frame->data_stride == NULL || 
		frame->data_stride[1] == NULL || 
		frame->data_stride[2] == NULL) {
		return;
	}
	
	guint8 *rgb_dest [3] = { rgb_buffer, NULL, NULL };
	int rgb_stride [3] = { cairo_image_surface_get_stride (surface), 0, 0 };
	
	stream->converter->Convert (frame->data_stride, frame->srcStride, frame->srcSlideY,
				    frame->srcSlideH, rgb_dest, rgb_stride);
	
	SetBit (RenderedFrame);
	element->MediaInvalidate ();
}

#define LOG_RS(x)  \
		printf ("MediaPlayer::AdvanceFrame (), %10s frame pts: %6llu ms, target pts: %6llu ms, diff: %+5lld, rendered fps: %5.2f, dropped fps: %5.2f, total: %5.2f\n", x, \
			MilliSeconds_FromPts (frame->pts), \
			MilliSeconds_FromPts (target_pts), \
			(gint64) MilliSeconds_FromPts (frame->pts) - (gint64) MilliSeconds_FromPts (target_pts), \
			rendered_frames_per_second, \
			dropped_frames_per_second, \
			dropped_frames_per_second + rendered_frames_per_second);

void
MediaPlayer::AdvanceFrame ()
{
	MediaFrame *frame = NULL;
	IMediaStream *stream;
	guint64 target_pts = 0;
	guint64 target_pts_start = 0;
	guint64 target_pts_end = 0;
	guint64 target_pts_delta = MilliSeconds_ToPts (100);
	bool update = false;
	double dropped_frames_per_second = -1;
	double rendered_frames_per_second = -1;
	AudioSource *audio;
	
	guint64 now = 0;
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame () state: %i, current_pts = %" G_GUINT64_FORMAT ", IsPaused: %i, IsSeeking: %i, VideoEnded: %i, AudioEnded: %i, HasVideo: %i, HasAudio: %i\n", 
		state_unlocked, current_pts, IsPaused (), IsSeeking (), GetBit (VideoEnded), GetBit (AudioEnded), HasVideo (), HasAudio ());
	VERIFY_MAIN_THREAD;

	RemoveBit (LoadFramePending);
	
	if (IsPaused ())
		return;
	
	if (IsSeeking ())
		return;
	
	if (GetBit (VideoEnded))
		return;

	if (!HasVideo ())
		return;

	// If the audio isn't playing, there might be slight length-difference between
	// audio and video streams (the audio is shorted and finished earlier for instance)
	// Treat this case as if there's no audio at all.
	audio = GetAudio ();
	if (audio != NULL && audio->GetState () == AudioPlaying) {
		// use target_pts as set by audio thread
		target_pts = GetTargetPts ();	
		if (target_pts == G_MAXUINT64) {
			// This might happen if we've called Play on the audio source, but it hasn't actually played anything yet.
			LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame (): invalid target pts from the audio stream.\n");
			audio->unref ();
			return;
		}
	} else {
		// no audio to sync to
		guint64 now = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
		guint64 elapsed_pts = now - start_time;
		
		target_pts = elapsed_pts;
		
		/*
		printf ("MediaPlayer::AdvanceFrame (): determined target_pts to be: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, elapsed_pts: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, start_time: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms\n",
			target_pts, MilliSeconds_FromPts (target_pts), elapsed_pts, MilliSeconds_FromPts (elapsed_pts), start_time, MilliSeconds_FromPts (start_time));
		*/
	}
	if (audio != NULL) {
		audio->unref ();
		audio = NULL;
	}
	
	this->target_pts = target_pts;
	media->SetTargetPts (target_pts);
		
	target_pts_start = target_pts_delta > target_pts ? 0 : target_pts - target_pts_delta;
	target_pts_end = target_pts + target_pts_delta;
	
	if (current_pts >= target_pts_end && GetBit (SeekSynched) && !(HasAudio () && GetBit (AudioEnded))) {
		LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %" G_GUINT64_FORMAT " ms, target_pts: %" G_GUINT64_FORMAT " ms, delta: %" G_GUINT64_FORMAT " ms, diff: %" G_GINT64_FORMAT " (%" G_GINT64_FORMAT " ms)).\n",
			MilliSeconds_FromPts (current_pts), MilliSeconds_FromPts (target_pts), MilliSeconds_FromPts (target_pts_delta), current_pts - target_pts, MilliSeconds_FromPts (current_pts - target_pts));
		return;
	}

	LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame (): target pts: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms\n", target_pts, MilliSeconds_FromPts (target_pts));

	while (true) {
		frame = video_stream->PopFrame ();
		if (frame == NULL) {
			if (video_stream->GetOutputEnded ()) {
				if (!HasAudio ()) {
					// Set the target pts to the last pts we showed, since target_pts is what's reported as our current position.
					this->target_pts = current_pts;
				}
				VideoFinished ();
				return;
			}
			if (!HasAudio ())
				SetBufferUnderflow ();
			// If we have audio, we keep playing (and loosing frames) until the audio playing stops due to buffer underflow
			// TODO: determine if we don't have video due to not having enough data (in which case we should start buffering),
			// or if it is because the decoder can't keep up (in which case we should drop frames).
			break;
		}
		
		stream = frame->stream;
		current_pts = frame->pts;
		update = true;
		
		//printf ("MediaPlayer::AdvanceFrame (): current_pts: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, duration: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms\n",
		//		current_pts, MilliSeconds_FromPts (current_pts),
		//		duration, MilliSeconds_FromPts (duration));
		
		if (GetBit (IsLive)) {
			first_live_pts = MIN (current_pts, first_live_pts);
		}

		if (GetBit (FixedDuration)) {
			/*
			printf ("MediaPlayer::AdvanceFrame (): (fixed duration, live: %i) current_pts: %" G_GUINT64_FORMAT " ms, duration: %" G_GUINT64_FORMAT " ms, first_live_pts: %" G_GUINT64_FORMAT " ms, diff: %"  G_GUINT64_FORMAT "ms\n",
				GetBit (IsLive), MilliSeconds_FromPts (current_pts), MilliSeconds_FromPts (duration), MilliSeconds_FromPts (first_live_pts), MilliSeconds_FromPts (current_pts - first_live_pts));
			*/
			if (GetBit (IsLive)) {
				if (current_pts - first_live_pts > duration) {
					// TODO: Move this out of AdvanceFrame, here it only works for media which has video, not for audio-only media.
					StopAudio ();
					AudioFinished ();
					VideoFinished ();
				}
			} else {
				if (current_pts > duration) {
					StopAudio ();
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
		
		if (!frame->IsDecoded ()) {
			printf ("MediaPlayer::AdvanceFrame (): Got a non-decoded frame.\n");
			update = false;
		}
		
		if (update && current_pts >= target_pts_start) {
			if (!GetBit (SeekSynched)) {
				SetBit (SeekSynched);
				LOG_MEDIAPLAYER ("MediaPlayer::AdvanceFrame (): We have now successfully synched with the audio after the seek, current_pts: %" G_GUINT64_FORMAT ", target_pts_start: %" G_GUINT64_FORMAT "\n", MilliSeconds_FromPts (current_pts), MilliSeconds_FromPts (target_pts_start));
			}
			// we are in sync (or ahead) of audio playback
			break;
		}
		
		if (video_stream->IsQueueEmpty ()) {
			// no more packets in queue, this frame is the most recent we have available
			break;
		}
		
		// we are lagging behind, drop this frame
		dropped_frames++;
	
		//LOG_RS ("[SKIPPED]");
		media->DisposeObject (frame);
		frame->unref ();
		frame = NULL;
	}
	
	if (update && frame && GetBit (SeekSynched)) {
		rendered_frames++;
		//LOG_RS ("[RENDER]");

		RenderFrame (frame);
	}
	
	if (frame) {
		media->DisposeObject (frame);
		frame->unref ();
		frame = NULL;
	}

	now = get_now ();
	if (frames_update_timestamp == 0) {
		frames_update_timestamp = now;
	} else if ((now - frames_update_timestamp) > TIMESPANTICKS_IN_SECOND) {
		double time_elapsed = (double) (now - frames_update_timestamp) / (double) TIMESPANTICKS_IN_SECOND;
		dropped_frames_per_second = (double) dropped_frames / time_elapsed;
		rendered_frames_per_second = (double) rendered_frames / time_elapsed;
		dropped_frames = rendered_frames = 0;
		frames_update_timestamp = now;

		this->dropped_frames_per_second = dropped_frames_per_second;
		this->rendered_frames_per_second = rendered_frames_per_second;
	}
	
	return;
}

void
MediaPlayer::FirstFrameEnqueuedHandler (EventObject *sender, EventArgs *args)
{
	LoadVideoFrame ();	
}

void
MediaPlayer::LoadVideoFrameCallback (EventObject *object)
{
	((MediaPlayer *) object)->LoadVideoFrame ();
}

void
MediaPlayer::LoadVideoFrame ()
{
	guint64 target_pts;
	MediaFrame *frame;
	
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), HasVideo: %i, LoadFramePending: %i\n", HasVideo (), state_unlocked & LoadFramePending);
	VERIFY_MAIN_THREAD;

	if (!HasVideo ())
		return;
	
	if (!IsLoadFramePending ())
		return;
	
	frame = video_stream->PopFrame ();

	if (frame == NULL)
		return;
	
	target_pts = GetTargetPts ();

	if (target_pts == G_MAXUINT64)
		target_pts = 0;

	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), packet pts: %" G_GUINT64_FORMAT ", target pts: %" G_GUINT64_FORMAT ", pts_per_frame: %" G_GUINT64_FORMAT ", buflen: %i\n", frame->pts, GetTargetPts (), video_stream->pts_per_frame, frame->buflen);

	if (frame->pts + video_stream->pts_per_frame >= target_pts) {
		LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (): rendering.\n");
		RemoveBit (LoadFramePending);
		RenderFrame (frame);
		element->MediaInvalidate ();
	} else {
		AddTickCallSafe (LoadVideoFrameCallback);
	}
	
	media->DisposeObject (frame);
	frame->unref ();
	
	return;
}

gboolean
MediaPlayer::AdvanceFrameCallback (void *user_data)
{
	MediaPlayer *mplayer = (MediaPlayer *) user_data;
	mplayer->SetCurrentDeployment ();
	mplayer->AdvanceFrame ();
#if SANITY
	Deployment::SetCurrent (NULL);
#endif
	return true;
}

void
MediaPlayer::SetTimeout (gint32 timeout /* set to 0 to clear */)
{
	TimeManager *time_manager = element ? element->GetTimeManager () : NULL;
	bool clear = timeout == 0 || advance_frame_timeout_id != 0;
	
	LOG_MEDIAPLAYER ("MediaPlayer::SetTimeout (%i) time_manager: %p id: %i\n", timeout, time_manager, GET_OBJ_ID (time_manager));

	if (clear && advance_frame_timeout_id != 0) {
		if (time_manager != NULL) {
			time_manager->RemoveTimeout (advance_frame_timeout_id);
		} else {
			g_warning ("MediaPlayer::SetTimeout (): Could not clear timeout. Leaking ourselves to not crash.\n");
			ref (); // This will prevent us from getting destroyed.
		}
		advance_frame_timeout_id = 0;
	}
	
	if (timeout != 0) {
		if (time_manager == NULL) {
			g_warning ("MediaPlayer::SetTimeout (): Could not set timeout (no time manager).\n");
		} else {
			advance_frame_timeout_id = time_manager->AddTimeout (MOON_PRIORITY_DEFAULT, timeout, AdvanceFrameCallback, this);
		}
	}
}

void
MediaPlayer::Play ()
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i, IsPlaying: %i, IsSeeking: %i\n", state_unlocked, IsPlaying (), IsSeeking ());
	VERIFY_MAIN_THREAD;
	
	if (IsSeeking ())
		return;
	
	SetState (Playing);
	RemoveBit (BufferUnderflow);
	start_time = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
	start_time -= target_pts;

	audio = GetAudio ();
	if (audio) {
		audio->Play ();
		audio->unref ();
	}
	
	SetTimeout (GetTimeoutInterval ());

	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i [Done]\n", state_unlocked);
}

gint32
MediaPlayer::GetTimeoutInterval ()
{
	gint32 result; // ms between timeouts
	guint64 pts_per_frame = 0;
	
	VERIFY_MAIN_THREAD;
	
	if (HasVideo ()) {
		pts_per_frame = video_stream->pts_per_frame;
		// there are 10000 pts in a millisecond, anything less than that will result in 0 (and an endless loop)
		if (pts_per_frame < PTS_PER_MILLISECOND || pts_per_frame >= (guint64) G_MAXINT32) {
			// If the stream doesn't know its frame rate, use a default of 60 fps
			result = (gint32) (1000.0 / 60.0);
		} else {
			result = (gint32) MilliSeconds_FromPts (pts_per_frame);
		}
	} else {
		result = 33;
	}

	LOG_MEDIAPLAYER ("MediaPlayer::GetTimeoutInterval (): %i ms between frames gives fps: %.1f, pts_per_frame: %" G_GUINT64_FORMAT ", exact fps: %f\n", result, 1000.0 / result, pts_per_frame, TIMESPANTICKS_IN_SECOND / (double) pts_per_frame);

	return result;
}

void
MediaPlayer::SetAudioStreamIndex (gint32 index)
{
	IMediaDemuxer *demuxer = NULL;
	IMediaStream *stream = NULL;
	AudioStream *next_stream = NULL;
	AudioStream *prev_stream = NULL;
	gint32 audio_streams_found = 0;
	AudioSource *audio;

	LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i).\n", index);
	VERIFY_MAIN_THREAD;
	
	if (index < 0 || index >= audio_stream_count) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i): Invalid audio stream index.\n", index);
		return;
	}

	if (media == NULL) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i): No media.\n", index);
		return;
	}

	audio = GetAudio ();
	if (audio == NULL) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i): No audio source.\n", index);
		return;
	}

	demuxer = media->GetDemuxerReffed ();

	if (demuxer == NULL) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i): Media doesn't have a demuxer.\n", index);
		return;
	}

	prev_stream = audio->GetAudioStream ();

	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);

		if (stream->GetType () != MediaTypeAudio)
			continue;

		if (audio_streams_found == index) {
			next_stream = (AudioStream *) stream;
			break;
		}
		
		audio_streams_found++;
	}

	if (next_stream != NULL) {
		LOG_MEDIAPLAYER ("MediaPlayer::SetAudioStreamIndex (%i). Switched stream from #%i to #%i\n", index, audio_streams_found++, index);
		prev_stream->SetSelected (false);
		next_stream->SetSelected (true);
		audio->SetAudioStream (next_stream);
	}

	audio->unref ();
	demuxer->unref ();
}

bool
MediaPlayer::GetCanPause ()
{
	// FIXME: should return false if it is streaming media
	VERIFY_MAIN_THREAD;
	return GetBit (CanPause);
}

void
MediaPlayer::SetCanPause (bool value)
{
	VERIFY_MAIN_THREAD;
	SetBitTo (CanPause, value);
}

void
MediaPlayer::Pause ()
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i\n", state_unlocked);
	VERIFY_MAIN_THREAD;

	if (IsPaused ())
		return;
	
	SetState (Paused);

	audio = GetAudio ();
	if (audio) {
		audio->Pause ();
		audio->unref ();
	}

	SetTimeout (0);

	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i [Done]\n", state_unlocked);	
}

guint64
MediaPlayer::GetTargetPts ()
{
	AudioSource *audio;
	guint64 result;

	VERIFY_MAIN_THREAD;
	
	audio = GetAudio ();

	LOG_MEDIAPLAYER_EX ("MediaPlayer::GetTargetPts (): target_pts: %" G_GUINT64_FORMAT ", HasAudio (): %i, audio->GetCurrentPts (): %" G_GUINT64_FORMAT "\n", target_pts, audio != NULL, audio != NULL ? audio->GetCurrentPts () : 0);

	if (audio != NULL && audio->GetState () == AudioPlaying)
		result = audio->GetCurrentPts ();
	else
		result = target_pts;

	if (audio)
		audio->unref ();
		
	return result;
}

void
MediaPlayer::SeekCompletedHandler (Media *media, EventArgs *args)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekCompletedHandler () seeks: %i\n", seeks);
	VERIFY_MAIN_THREAD;
	
	seeks--;
	if (seeks != 0)
		return;

	if (HasVideo ()) {
		SetBit (LoadFramePending);
		LoadVideoFrame ();
	}
}

void
MediaPlayer::NotifySeek (guint64 pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms), media: %p, state: %i, current_pts: %" G_GUINT64_FORMAT ", IsPlaying (): %i, seeks: %i\n", pts, MilliSeconds_FromPts (pts), media, state_unlocked, current_pts, IsPlaying (), seeks);
	VERIFY_MAIN_THREAD;

	seeks++;
	guint64 duration = GetDuration ();
	
	g_return_if_fail (GetCanSeek ());
	
	if (pts > start_pts + duration)
		pts = start_pts + duration;
	
	if (pts < start_pts)
		pts = start_pts;
	
	StopAudio ();
	SetTimeout (0);

	SetBit (LoadFramePending);
	RemoveBit (SeekSynched);
	RemoveBit (AudioEnded);
	RemoveBit (VideoEnded);
		
	start_time = 0;	
	current_pts = pts;
	target_pts = pts;
	
	media->SetTargetPts (target_pts);
	
	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms), media: %p, state: %i, current_pts: %" G_GUINT64_FORMAT " [END]\n", pts, MilliSeconds_FromPts (pts), media, state_unlocked, current_pts);
}

bool
MediaPlayer::GetCanSeek ()
{
	VERIFY_MAIN_THREAD;
	return GetBit (CanSeek);
}

void
MediaPlayer::SetCanSeek (bool value)
{
	VERIFY_MAIN_THREAD;
	SetBitTo (CanSeek, value);
}

void
MediaPlayer::Stop ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Stop (), state: %i\n", state_unlocked);
	VERIFY_MAIN_THREAD;

	StopAudio ();
		
	SetTimeout (0);
	
	start_time = 0;
	current_pts = 0;
	target_pts = 0;
	SetState (Stopped);
	RemoveBit (AudioEnded);
	RemoveBit (VideoEnded);
}

void
MediaPlayer::StopAudio ()
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::StopAudio (), state: %i\n", state_unlocked);
	VERIFY_MAIN_THREAD;

	audio = GetAudio (); // This returns a reffed AudioSource
	if (audio) {
		audio->Stop ();
		audio->unref ();
	}
}

double
MediaPlayer::GetBalance ()
{
	double result;
	AudioSource *audio;

	VERIFY_MAIN_THREAD;
	
	audio = GetAudio ();
	if (audio) {
		result = audio->GetBalance ();
		audio->unref ();
	} else {
		fprintf (stderr, "MediaPlayer::GetBalance (): There's no audio source to get the balance from\n");
		result = 0.0;
	}

	return result;
}

void
MediaPlayer::SetBalance (double balance)
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::SetBalance (%f)\n", balance);
	VERIFY_MAIN_THREAD;

	if (balance < -1.0)
		balance = -1.0;
	else if (balance > 1.0)
		balance = 1.0;

	audio = GetAudio ();
	if (audio) {
		audio->SetBalance (balance);
		audio->unref ();
	} else {
		//fprintf (stderr, "MediaPlayer::SetBalance (%f): There's no audio source to set the balance\n", balance);
	}
}

double
MediaPlayer::GetVolume ()
{
	AudioSource *audio;
	double result;
	
	VERIFY_MAIN_THREAD;
	
	audio = GetAudio ();
	if (audio) {
		result = audio->GetVolume ();
		audio->unref ();
	} else {
		fprintf (stderr, "MediaPlayer::GetVolume (): There's no audio source to get the volume from\n");
		result = 0.0;
	}

	return result;
}

void
MediaPlayer::SetVolume (double volume)
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::SetVolume (%f)\n", volume);
	VERIFY_MAIN_THREAD;

	if (volume < -1.0)
		volume = -1.0;
	else if (volume > 1.0)
		volume = 1.0;

	audio = GetAudio ();
	if (audio) {
		audio->SetVolume (volume);
		audio->unref ();
	} else {
		//fprintf (stderr, "MediaPlayer::SetVolume (%f): There's no audio source to set the volume\n", volume);
	}		
}
	
bool
MediaPlayer::GetMuted ()
{
	bool result;
	AudioSource *audio;

	VERIFY_MAIN_THREAD;
	
	audio = GetAudio ();
	if (audio) {
		result = audio->GetMuted ();
		audio->unref ();
	} else {
		fprintf (stderr, "MediaPlayer::GetMuted (): There's no audio.\n");
		result = false;
	}

	return result;
}

void
MediaPlayer::SetMuted (bool muted)
{
	AudioSource *audio;
	
	LOG_MEDIAPLAYER ("MediaPlayer::SetMuted (%i)\n", muted);
	VERIFY_MAIN_THREAD;

	audio = GetAudio ();
	if (audio) {
		audio->SetMuted (true);
		audio->unref ();
	} else {
		//fprintf (stderr, "MediaPlayer::SetMuted (%i): There's no audio to mute.\n", muted);
	}
}

void
MediaPlayer::SetBit (PlayerState s)
{
	mutex.Lock ();
	state_unlocked = (PlayerState) (s | state_unlocked);
	mutex.Unlock ();
}

void
MediaPlayer::RemoveBit (PlayerState s)
{
	mutex.Lock ();
	state_unlocked = (PlayerState) (~s & state_unlocked);
	mutex.Unlock ();
}

void
MediaPlayer::SetBitTo (PlayerState s, bool value)
{
	if (value) {
		SetBit (s);
	} else {
		RemoveBit (s);
	}
}

bool
MediaPlayer::GetBit (PlayerState s)
{
	bool result;
	mutex.Lock ();
	result =  (state_unlocked & s) == s;
	mutex.Unlock ();
	return result;
}

void
MediaPlayer::SetState (PlayerState s)
{
	mutex.Lock ();
	state_unlocked = (PlayerState) ((state_unlocked & ~StateMask) | s);
	mutex.Unlock ();
}

MediaPlayer::PlayerState
MediaPlayer::GetState ()
{
	PlayerState result;
	mutex.Lock ();
	result = state_unlocked;
	mutex.Unlock ();
	return result;
}

bool
MediaPlayer::IsBufferUnderflow ()
{
	return GetBit (BufferUnderflow);
}

bool
MediaPlayer::IsLoadFramePending ()
{
	return GetBit (LoadFramePending);
}

bool
MediaPlayer::IsSeeking ()
{
	return seeks > 0;
}

bool
MediaPlayer::HasRenderedFrame ()
{
	return GetBit (RenderedFrame);
}

bool
MediaPlayer::IsPlaying ()
{
	return (GetState () & StateMask) == Playing;
}

bool
MediaPlayer::IsPaused ()
{
	return (GetState () & StateMask) == Paused;
}

bool
MediaPlayer::IsStopped ()
{
	return (GetState () & StateMask) == Stopped;
}

void
MediaPlayer::SetBufferUnderflow ()
{
	SetBitTo (BufferUnderflow, true);
	EmitBufferUnderflow ();
}

void
MediaPlayer::EmitBufferUnderflow ()
{
	if (Surface::InMainThread ()) {
		Emit (BufferUnderflowEvent);
	} else {
		AddTickCallSafe (EmitBufferUnderflowAsync);
	}
}

void
MediaPlayer::EmitBufferUnderflowAsync (EventObject *obj)
{
	((MediaPlayer *) obj)->EmitBufferUnderflow ();
}
