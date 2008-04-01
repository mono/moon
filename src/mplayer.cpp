/*
 * mplayer.cpp: 
 *
 * Authors: Jeffrey Stedfast <fejj@novell.com>
 *          Rolf Bjarne Kvinge  <RKvinge@novell.com>
 *          Michael Dominic K. <mkostrzewa@novell.com>
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

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
G_END_DECLS

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
#define LOG_AUDIO(...)// printf (__VA_ARGS__)
// This one prints out spew on every sample
#define LOG_AUDIO_EX(...)// printf (__VA_ARGS__)

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
	LOG_MEDIAPLAYER ("MediaPlayer::MediaPlayer (%p, id=%i), id=%i", el, GET_OBJ_ID (el), GET_OBJ_ID (this));

	element = el;
	pthread_mutex_init (&target_pts_lock, NULL);

	media = NULL;
	
	Initialize ();
}

MediaPlayer::~MediaPlayer ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::~MediaPlayer (), id=%i", GET_OBJ_ID (this));
	Close (true);
	
	pthread_mutex_destroy (&target_pts_lock);
}

struct EnqueueFrameStruct {
	MediaPlayer *mplayer;
	int audio;
	int video;
};

gboolean
MediaPlayer::EnqueueFramesCallback (void *user_data)
{
	LOG_MEDIAPLAYER ("MediaPlayer::EnqueueFramesCallback ()\n");
	EnqueueFrameStruct *efs = (EnqueueFrameStruct *) user_data;

	efs->mplayer->EnqueueFrames (efs->audio, efs->video);
	efs->mplayer->unref ();

	delete efs;
	return false;
}

gboolean
MediaPlayer::LoadFrameCallback (void *user_data)
{
	bool result = false;

	LOG_MEDIAPLAYER ("MediaPlayer::LoadFrameCallback ()\n");

	MediaPlayer *player = (MediaPlayer*) user_data;
	if (player != NULL) {
		// Check again if LoadFrame is still pending.
 		if (player->IsLoadFramePending ())
			result = player->LoadVideoFrame ();
		player->unref ();
	}		
	return false;
}

MediaResult
MediaPlayer::FrameCallback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *player = element->GetMediaPlayer ();
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame ? frame->stream : NULL;
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::FrameCallback (%p), state: %i, frame: %p, pts: %llu\n", closure, player->state, closure->frame, closure->frame->pts);

	if (player->GetBit (MediaPlayer::Seeking)) {
		// We don't want any frames while we're waiting for a seek.
		return MEDIA_SUCCESS;
	}
	
	if (closure->frame == NULL)
		return MEDIA_SUCCESS;
	
	closure->frame = NULL;
	
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		player->video.queue.Push (new Packet (frame));
		if (player->IsLoadFramePending ()) {
			// We need to call LoadVideoFrame on the main thread
			player->ref ();
			TimeManager::InvokeOnMainThread (LoadFrameCallback, player);
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
MediaPlayer::EnqueueFramesAsync (int audio_frames, int video_frames)
{
	EnqueueFrameStruct *efs = new EnqueueFrameStruct ();
	efs->mplayer = this;
	this->ref ();
	efs->audio = audio_frames;
	efs->video = video_frames;
	TimeManager::InvokeOnMainThread (EnqueueFramesCallback, efs);
}

void
MediaPlayer::EnqueueFrames (int audio_frames, int video_frames)
{
	MediaClosure *closure;
	int states;

	LOG_MEDIAPLAYER_EX ("MediaPlayer::EnqueueFrames (%i, %i)\n", audio_frames, video_frames);
	
	if (element == NULL)
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
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);
		encoding = stream->GetDecoder (); //stream->codec;
		
		if (encoding == NULL)
			continue; // No encoding was found for the stream.
		
		switch (stream->GetType ()) {
		case MediaTypeAudio:
			audio.stream_count++;			
			if (audio.stream == NULL) {
				audio.stream = (AudioStream *) stream;
				audio.stream->SetSelected (true);
			}
			break;
		case MediaTypeVideo: 
			if (video.stream != NULL)
				break;

			video.stream = (VideoStream *) stream;
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
				g_error ("Could not allocate memory");
			}
			
			// rendering surface
			video.surface = cairo_image_surface_create_for_data (
				video.rgb_buffer, CAIRO_FORMAT_ARGB32,
				width, height, stride);
			
			// printf ("video size: %i, %i\n", video.stream->width, video.stream->height);
			break;
		default:
			break;
		}
	}

	if (audio.stream != NULL) {
		if (!AudioPlayer::Add (this)) {
			// Can't play audio
			audio.stream->SetSelected (false);
			audio.stream = NULL;
		}
	}
	
	if (HasVideo ()) {
		SetBit (LoadFramePending);
		EnqueueFrames (0, 1);
	}
	
	current_pts = 0;
	target_pts = 0;
	duration = media->GetDemuxer ()->GetDuration ();
	
	return true;
}

void 
MediaPlayer::Initialize ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Initialize ()\n");

	state = (PlayerState) 0;
	SetState (Stopped);
	SetBit (SeekSynched);
	
	start_time = 0;
	current_pts = 0;
	target_pts = 0;
	
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
		memcpy (video.rgb_buffer, frame->buffer, MIN (frame->buflen, (uint32_t) (cairo_image_surface_get_stride (video.surface) * height)));
		SetBit (RenderedFrame);
		return;
	}
	
	if (frame->data_stride == NULL || 
		frame->data_stride[1] == NULL || 
		frame->data_stride[2] == NULL) {
		return;
	}
	
	uint8_t *rgb_dest [3] = { video.rgb_buffer, NULL, NULL };
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
	uint64_t target_pts = 0;
	uint64_t target_pts_start = 0;
	uint64_t target_pts_end = 0;
	uint64_t target_pts_delta = MilliSeconds_ToPts (100);
	bool update = false;
	
#if DEBUG_ADVANCEFRAME
	static int frames_per_second = 0;
	static int skipped_per_second = 0;
	static uint64_t last_second_pts = 0;
	int skipped = 0;
#endif
	
	LOG_MEDIAPLAYER_EX ("MediaPlayer::AdvanceFrame () state: %i, current_pts = %llu\n", state, current_pts);

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
		uint64_t now = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
		uint64_t elapsed_pts = now - start_time;
		
		target_pts = elapsed_pts;
		
		this->target_pts = target_pts;
	}
	
	target_pts_start = target_pts_delta > target_pts ? 0 : target_pts - target_pts_delta;
	target_pts_end = target_pts + target_pts_delta;
	
	if (current_pts >= target_pts_end && GetBit (SeekSynched)) {
#if DEBUG_ADVANCEFRAME
		printf ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %llu, target_pts: %llu, delta: %llu, diff: %lld).\n",
			current_pts, target_pts, target_pts_delta, current_pts - target_pts);
#endif
		return false;
	}
		
	while ((pkt = (Packet *) video.queue.Pop ())) {
		if (pkt->frame->event == FrameEventEOF) {
			delete pkt;
			SetEof (true);
			return false;
		}
		
		// always decode the frame or we get glitches in the screen
		frame = pkt->frame;
		stream = frame->stream;
		current_pts = frame->pts;
		update = true;
		
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
		uint64_t ms = 0;
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
			printf ("MediaPlayer::AdvanceFrame (): rendering pts %llu (target pts: %llu, current pts: %llu, caught_up_with_seek: %s, skipped frames: %i, fps: %i, sps: %i, ms: %llu)\n", frame->pts, target_pts, current_pts, caught_up_with_seek ? "true" : "false", skipped, fps, sps, ms);
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
	LOG_MEDIAPLAYER ("MediaPlayer::Play (), state: %i\n", state);

	if (IsPlaying () && !IsSeeking ())
		return;
	
	SetState (Playing);
	start_time = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());

	AudioPlayer::Play (this);
	
	EnqueueFrames (1, 1);

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
MediaPlayer::CanPause ()
{
	// FIXME: should return false if it is streaming media
	return true;
}

void
MediaPlayer::Pause ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i\n", state);

	if (IsPaused () || !CanPause ())
		return;
	
	SetState (Paused);
	SetBit (SeekSynched);
	AudioPlayer::Pause (this, true);

	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), state: %i [Done]\n", state);	
}

uint64_t
MediaPlayer::GetTargetPts ()
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::GetTargetPts (): target_pts: %llu\n", target_pts);

	uint64_t result;
	
	pthread_mutex_lock (&target_pts_lock);
	result = target_pts;
	pthread_mutex_unlock (&target_pts_lock);
	
	return result;
}

void
MediaPlayer::SetTargetPts (uint64_t pts)
{
	LOG_MEDIAPLAYER_EX ("MediaPlayer::SetTargetPts (%llu = %llu ms), current_pts: %llu, IsSeeking (): %i\n", pts, MilliSeconds_FromPts (pts), current_pts, IsSeeking ());

	if (IsSeeking ())
		return;

	pthread_mutex_lock (&target_pts_lock);
	target_pts = pts;
	pthread_mutex_unlock (&target_pts_lock);
}

MediaResult
MediaPlayer::SeekCallback (MediaClosure *closure)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekCallback (%p)\n", closure);

	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *mplayer = element->GetMediaPlayer ();
	mplayer->RemoveBit (Seeking);
	mplayer->current_pts = 0;

	// Clear all queues.
	mplayer->audio.queue.Clear (true);
	mplayer->video.queue.Clear (true);
	
	return MEDIA_SUCCESS;
}

void
MediaPlayer::SeekInternal (uint64_t pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekInternal (%llu = %llu ms), media: %p, state: %i, Position (): %llu\n", pts, MilliSeconds_FromPts (pts), media, state, GetPosition ());

	if (media == NULL)
		return;
		
	if (pts == 0) {
		media->SeekToStart ();
		return;
	}

	SetBit (Seeking);
	RemoveBit (SeekSynched);

	MediaClosure *closure = new MediaClosure (SeekCallback);
	closure->SetContext (element);
	media->ClearQueue ();
	media->SeekAsync (pts, closure);
}

void
MediaPlayer::Seek (uint64_t pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%llu = %llu ms), media: %p, state: %i, current_pts: %llu\n", pts, MilliSeconds_FromPts (pts), media, state, current_pts);

	uint64_t duration = GetDuration ();
	bool resume = IsPlaying ();
	
	if (!CanSeek ())
		return;
	
	if (pts > duration)
		pts = duration;
	
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
	
	if (resume) {
		Play ();
	} else if (IsLoadFramePending ()) {
		EnqueueFrames (0, 1);
	}

	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%llu = %llu ms), media: %p, state: %i, current_pts: %llu [END]\n", pts, MilliSeconds_FromPts (pts), media, state, current_pts);
}

bool
MediaPlayer::CanSeek ()
{
	// FIXME: should return false if it is streaming media
	return true;
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

static int
sem_get_value (sem_t *sem)
{
	int v;
	sem_getvalue (sem, &v);
	return v;
}

AudioPlayer *AudioPlayer::instance = NULL;

void
AudioPlayer::Shutdown ()
{
	delete instance;
	instance = NULL;
}

bool
AudioPlayer::Initialize ()
{
	instance = new AudioPlayer ();
	instance->StartThread ();
	return true;
}

AudioPlayer::AudioPlayer ()
{
	audio_thread = NULL;
	
	shutdown = false;
	initialized = false;

	sem_init (&semaphore, 0, 1);
	list = NULL;
	list_count = 0;
	list_size = 0;

	udfs = NULL;
	ndfs = 0;

	fds [0] = -1;
	fds [1] = -1;
	if (pipe (fds) != 0) {
		fprintf (stderr, "AudioPlayer::Initialize (): Unable to create pipe (%s).\n", strerror (errno));
		return;
	}

	// Make the writer pipe non-blocking.
	fcntl (fds [1], F_SETFL, fcntl (fds [1], F_GETFL) | O_NONBLOCK);

	ndfs = 1;
	udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
	udfs [0].fd = fds [0];
	udfs [0].events = POLLIN;
		
	initialized = true;
	
	LOG_AUDIO ("AudioPlayer::Initialize (): the audio player has been initialized.");
}

void
AudioPlayer::StartThread ()
{
	int result;
	
	LOG_AUDIO ("AudioPlayer::StartThread (), audio_thread: %p\n", audio_thread);
	
	if (audio_thread != NULL)
		return;
	
	shutdown = false;
	audio_thread = (pthread_t*) g_malloc0 (sizeof (pthread_t));
	result = pthread_create (audio_thread, NULL, Loop, this);
	if (result != 0) {
		fprintf (stderr, "AudioPlayer::Initialize (): could not create audio thread (error code: %i = '%s').\n", result, strerror (result));
		g_free (audio_thread);
		audio_thread = NULL;
		return;
	}
}

void
AudioPlayer::StopThread ()
{
	int result;

	if (audio_thread == NULL)
		return;

	shutdown = true;

	// Wake up the loop in case it's waiting in a poll.	
	WakeUp ();

	// Wait until the audio thread has completely finished.
	result = pthread_join (*audio_thread, NULL);
	if (result != 0) {
		fprintf (stderr, "AudioPlayer::~AudioPlayer (): failed to join the audio thread (error code: %i).\n", result);
	}
	g_free (audio_thread);
	audio_thread = NULL;
}

AudioPlayer::~AudioPlayer ()
{
	LOG_AUDIO ("AudioPlayer::~AudioPlayer (): the audio player is being shut down.\n");
		
	if (!initialized)
		return;
	
	StopThread ();
	
	if (list != NULL) {	
		for (uint32_t i = 0; i < list_count; i++)
			delete list [i];
		g_free (list);
		list = NULL;
	}

	close (fds [0]);
	close (fds [1]);
	
	g_free (udfs);
	udfs = NULL;
	
	initialized = false;

	sem_destroy (&semaphore);
	LOG_AUDIO ("AudioPlayer::~AudioPlayer (): the audio player has been shut down.\n");
}

bool
AudioPlayer::Add (MediaPlayer *mplayer)
{
	if (instance == NULL)
		return false; // We've been shutdown (or not initialized);

	return instance->AddInternal (mplayer);
}

bool
AudioPlayer::AddInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	

	AudioNode *node = new AudioNode ();
	AudioNode **new_list = NULL;

	node->mplayer = mplayer;	
	if (!node->Initialize ()) {
		delete node;
		return false;
	}
	
	Lock ();
	
	list_count++;
	if (list_count > list_size) {
		// Grow the list a bit
		list_size = (list_size == 0) ? 1 : list_size << 1;
		new_list = (AudioNode**) g_malloc0 (sizeof (AudioNode*) * (list_size + 1));
		if (list != NULL) {
			for (uint32_t i = 0; i < list_count - 1; i++)
				new_list [i] = list [i];
			g_free (list);
		}
		list = new_list;
	}
	list [list_count - 1] = node;
	
	UpdatePollList (true);

	if (list_count == 1)
		StartThread ();

	Unlock ();

	return true;
}

void
AudioPlayer::Remove (MediaPlayer *mplayer)
{
	if (instance == NULL)
		return; // We've been shutdown (or not initialized);
	
	instance->RemoveInternal (mplayer);
}

void
AudioPlayer::RemoveInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Remove (%p)\n", mplayer);
	
	Lock ();
	
	for (uint32_t i = 0; i < list_count; i++) {
		if (list [i]->mplayer == mplayer) {
			// printf ("AudioPlayer::Remove (%p): removing... (node: %p)\n", list [i]->mplayer, list [i]);
			delete list [i];

			for (uint32_t k = i + 1; k < list_count; k++)
				list [k - 1] = list [k];
			
			list [list_count - 1] = NULL;
			list_count--;

			break;
		}
	}

	UpdatePollList (true);

	// We don't stop the audio thread when we reach 0 audio nodes since
	// this may cause a lot of threads being created if a lot of
	// mediaelements/media are added and removed.

	Unlock ();
}

void
AudioPlayer::Play (MediaPlayer *mplayer)
{
	if (instance == NULL)
		return; // We've been shutdown (or not initialized);

	instance->PlayInternal (mplayer);
}


void
AudioPlayer::PlayInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::PlayInternal (%p)\n", mplayer);

	AudioNode *node;

	Lock ();
	node = Find (mplayer);
	
	if (node != NULL) {
		node->state = Playing;
		UpdatePollList (true);
	}
	Unlock ();

	WakeUp ();
}

void
AudioPlayer::WaitForData (AudioNode *node)
{
	LOG_AUDIO ("AudioPlayer::WaitForData (%p)\n", node);

	// This method should be called with the lock held in the worker loop.
	node->state = WaitingForData;
	UpdatePollList (true);
}

AudioPlayer::AudioNode*
AudioPlayer::Find (MediaPlayer *mplayer)
{
	AudioNode *result = NULL;
	
	for (uint32_t i = 0; i < list_count; i++) {
		if (list [i]->mplayer == mplayer) {
			result = list [i];
			break;
		}	
	}
	
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
		fprintf (stderr, "AudioPlayer:: Can't recover from underrun: %s\n", snd_strerror (err));
		break;
	}
	
	return err >= 0;
}

void
AudioPlayer::UpdatePollList (bool locked)
{
	int current;

	/*
	 * We need to update the list of file descriptors we poll on
	 * to only include audio nodes which are playing.
	 */

	if (!locked) {
		Lock ();
	}
	
	ndfs = 1;
	for (uint32_t i = 0; i < list_count; i++) {
		if (list [i]->state == Playing)
			ndfs += list [i]->ndfs;
	}
	
	g_free (udfs);
	udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
	udfs [0].fd = fds [0];
	udfs [0].events = POLLIN;

	current = 1;
	for (uint32_t i = 0; i < list_count; i++) {
		if (list [i]->state == Playing) {
			memcpy (&udfs [current], list [i]->udfs, list [i]->ndfs * sizeof (pollfd));
			current += list[i]->ndfs;
		}
	}

	if (!locked)
		Unlock ();
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
	AudioNode *current = NULL;
	uint32_t current_index = 0;
	// Keep track of how many of the audio nodes actually played something
	// If none of the nodes played anything, then we poll until something happens.
	int pc = 0; // The number of consecutive nodes which haven't played anything
	int lc = 0; // Save a copy of the list count for ourselves to avoid some locking.
	
	LOG_AUDIO ("AudioPlayer: entering audio .\n");
	

	SimpleLock ();

	// valgrind/helgrind reports a possible data race while accessing 'shutdown', however
	// this can be ignored since 'shutdown' is only written to once (to set it to true).

	while (!shutdown) {

		// Unlock/relock our lock so that the rest of the audio player gets a chance
		// to do something.
		Unlock ();
		SimpleLock ();

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
			if (current->state == WaitingForData && current->mplayer->audio.queue.LinkedList ()->First () != NULL) {
				current->state = Playing;
				UpdatePollList (true);
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

			// Make our own copy of the udfs structure so that we don't have to hold
			// the lock while polling. This means that any of the file descriptors
			// we are polling against might get closed while in the poll, however
			// according to the docs I have been able to find this will only cause
			// wakeups with errors (invalid fd) or spurious wakeups (fd represents
			// something else), and we handle both cases correctly anyway.
			int ndfs = this->ndfs;
			pollfd *udfs = (pollfd*) g_malloc (sizeof (pollfd) * ndfs);
			memcpy (udfs, this->udfs, sizeof (pollfd) * ndfs);
			Unlock ();
			
			do {
				pc = 0;
				udfs [0].events = POLLIN;
				udfs [0].revents = 0;
				
				LOG_AUDIO_EX ("AudioPlayer::Loop (): polling... (lc: %i, pc: %i)\n", lc, pc);
				result = poll (udfs, ndfs, 10000); // Have a timeout of 10 seconds, just in case something goes wrong.
				LOG_AUDIO_EX ("AudioPlayer::Loop (): poll result: %i, fd: %i, fd [0].revents: %i, errno: %i, err: %s, ndfs = %i, shutdown: %i\n", result, udfs [0].fd, (int) udfs [0].revents, errno, strerror (errno), ndfs, shutdown);
	
				if (result == 0) { // Timed out
					LOG_AUDIO ("AudioPlayer::Loop (): poll timed out.\n");
				} else if (result < 0) { // Some error poll exit condition
					// Doesn't matter what happened (happens quite often due to interrupts)
					LOG_AUDIO ("AudioPlayer::Loop (): poll failed: %i (%s)\n", errno, strerror (errno));
				} else { // Something woke up the poll
					if (udfs [0].revents & POLLIN) {
						// We were asked to wake up by the audio player
						// Read whatever was written into the pipe so that the pipe doesn't fill up.
						read (udfs [0].fd, &buffer, sizeof (int));
						LOG_AUDIO ("AudioPlayer::Loop (): woken up by ourselves.\n");
					} else {
						// Something happened on any of the audio streams
					}
				}
			} while (result == -1 && errno == EINTR); 

			SimpleLock ();
			g_free (udfs);
		}
	}
			
	Unlock ();
	LOG_AUDIO ("AudioPlayer: exiting audio loop.\n");
}

void
AudioPlayer::Pause (MediaPlayer *mplayer, bool value)
{
	if (instance == NULL)
		return; // We've been shutdown (or not initialized);

	instance->PauseInternal (mplayer, value);
}

void
AudioPlayer::PauseInternal (MediaPlayer *mplayer, bool value)
{
	AudioNode *node;
	int err = 0;
	
	Lock ();
	
	node = Find (mplayer);
	
	if (node == NULL) {
		Unlock ();
		return;
	}
	
	if (value != (node->state == Paused)) {
		if (value) {
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

	UpdatePollList (true);
	Unlock ();
}

void
AudioPlayer::Stop (MediaPlayer *mplayer)
{
	if (instance == NULL)
		return;

	instance->StopInternal (mplayer);
}

void
AudioPlayer::StopInternal (MediaPlayer *mplayer)
{
	// Take a minor shortcut and handle a stop just like a pause command.
	PauseInternal (mplayer, true);
}

void
AudioPlayer::SimpleLock ()
{
	LOG_AUDIO_EX ("AudioPlayer::SimpleLock (). instance: %p, semaphore: %i\n", instance, sem_get_value (&semaphore));

	while (sem_wait (&semaphore) == -1 && errno == EINTR) {};

	LOG_AUDIO_EX ("AudioPlayer::SimpleLock (). AQUIRED instance: %p, semaphore: %i\n", instance, sem_get_value (&semaphore));
}

void
AudioPlayer::Lock ()
{
	LOG_AUDIO_EX ("AudioPlayer::Lock (). instance: %p, semaphore: %i\n", instance, sem_get_value (&semaphore));

	while (sem_wait (&semaphore) == -1 && errno == EINTR) {};
#if 0
	timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = 10000000; // 10 milliseconds
	

	// Wait for a moment, if no success try to wake up the loop and and try again
	// We can't just wait since the loop might be in a poll waiting for something to happen.
	while (sem_timedwait (&semaphore, &ts) == -1 && (errno == EINTR || errno == ETIMEDOUT)) {
		WakeUp ();
	}
#endif

	LOG_AUDIO_EX ("AudioPlayer::Lock (). AQUIRED instance: %p, semaphore: %i\n", instance, sem_get_value (&semaphore));
}

void
AudioPlayer::Unlock ()
{
	LOG_AUDIO_EX ("AudioPlayer::UnLock (), semaphore: %i\n", sem_get_value (&semaphore));
	sem_post (&semaphore);
}

void
AudioPlayer::WakeUp ()
{
	int result;
		
	if (instance == NULL)
		return; // We've been shutdown (or not initialized)

	LOG_AUDIO ("AudioPlayer::WakeUp (). semaphore: %i\n", sem_get_value (&instance->semaphore));
	
	// Write until something has been written.	
	do {
		result = write (instance->fds [1], "c", 1);
	} while (result == 0);
	
	if (result == -1)
		fprintf (stderr, "AudioPlayer::WakeUp (): Could not wake up audio thread: %s\n", strerror (errno));
		
	LOG_AUDIO ("AudioPlayer::WakeUp (): thread should now wake up.\n");
	
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
	
	snd_pcm_hw_params_t *params = NULL;
	uint32_t buffer_time = 500000; // request 0.5 seconds of buffer time.
	int err = 0;
	int dir = 0;
	int channels = mplayer->audio.stream->channels;
	unsigned int rate = mplayer->audio.stream->sample_rate;
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
	
	// set transfer mode (mmap in our case)
	err = snd_pcm_hw_params_set_access (pcm, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (access type not available for playback): %s\n", snd_strerror(err));
		goto cleanup;
	}

	// set audio format
	err = snd_pcm_hw_params_set_format (pcm, params, MOON_AUDIO_FORMAT);
	if (err < 0) {
		fprintf (stderr, "AudioNode::SetupHW (): Audio HW setup failed (sample format not available for playback): %s\n", snd_strerror(err));
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
AudioPlayer::AudioNode::PreparePcm (snd_pcm_sframes_t *avail)
{
	int err;
	snd_pcm_state_t state = snd_pcm_state (pcm);
	int32_t period_size = sample_size;
	
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
	case SND_PCM_STATE_PREPARED:
		break;
	case SND_PCM_STATE_PAUSED:
	case SND_PCM_STATE_DRAINING:
	default:
		LOG_AUDIO ("AudioNode::PreparePcm (): state: %s (prepare failed)\n", snd_pcm_state_name (state));
		return false;
	}
	
	*avail = snd_pcm_avail_update (pcm);
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

	LOG_AUDIO_EX ("AudioPlayer::PreparePcm (): Prepared, avail: %li\n", *avail);

	return true;
}

bool
AudioPlayer::AudioNode::Play ()
{
	LOG_AUDIO_EX ("AudioPlayer::AudioNode::Play ()\n");

	bool result = false;	
	Audio *audio = &mplayer->audio;
	uint32_t channels = audio->stream->channels;
	const snd_pcm_channel_area_t *areas = NULL;
	snd_pcm_uframes_t offset = 0, frames, size;
	snd_pcm_sframes_t avail, commitres;
	int err = 0;
	
	int32_t bpf = snd_pcm_format_width (MOON_AUDIO_FORMAT) / 8 * channels; // bytes per frame
	int32_t steps [channels];
	int32_t count;
	int16_t *samples [channels];
	int16_t *outptr;
	
	if (state != Playing)
		return false;

	if (!PreparePcm (&avail))
		return false;
	
	LOG_AUDIO_EX ("AudioPlayer::AudioNode::Play (): entering play loop, avail: %lld, sample size: %i\n", (int64_t) avail, (int) sample_size);

	// Set the volume
	int32_t	volume = audio->volume * 8192;
	int32_t volumes [channels]; // channel #0 = left, #1 = right 
	
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
		
		err = snd_pcm_mmap_begin (pcm, &areas, &offset, &frames);
		if (err < 0) {
			if (!XrunRecovery (err)) {
				fprintf (stderr, "AudioPlayer: could not get mmapped memory: %s\n", snd_strerror (err));
				return result;
			}
			started = false;
		}

		count = frames;
		
		for (uint32_t channel = 0; channel < channels; channel++) {
			// number of 16bit samples between each sample
			steps [channel] = areas [channel].step / 16;
			// pointer to the first sample to write to
			samples [channel] = ((int16_t*) areas [channel].addr) + (areas [channel].first / 16);
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
					return result;
				}
				//printf ("play: sent_pts = %llu (from frame, old pts: %llu, diff: %lld, time: %lld milliseconds), samples sent: %i\n", first_pts, 
				//	sent_pts, first_pts - sent_pts, (int64_t) MilliSeconds_FromPts ((int64_t) first_pts - (int64_t) sent_pts), sent_samples);
				sent_pts = first_pts;
				sent_samples = 0;
				update_target_pts = true;
			} else {
				sent_pts = first_pts + sent_samples * 10000000 / audio->stream->sample_rate;
			}

			outptr = (int16_t*) &first_buffer [first_used];
			first_used += bpf;
			sent_samples++;
			for (uint32_t channel = 0; channel < channels; channel++) {
				int32_t value = (outptr [channel] * volumes [channel]) >> 13;
				*(samples[channel]) = (int16_t) CLAMP (value, -32768, 32767);
				samples[channel] += steps[channel];
			}
		}

		commitres = snd_pcm_mmap_commit (pcm, offset, frames);
		if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
			if (!XrunRecovery (commitres >= 0 ? -EPIPE : commitres)) {
				fprintf (stderr, "AudioPlayer: could not commit mmapped memory: %s\n", snd_strerror(err));
				return result;
			}
			started = false;
		}
		size -= frames;
		
		if (update_target_pts || (sent_pts - updated_pts) > 10000) {
			snd_pcm_sframes_t delay;
			uint64_t pts = sent_pts;
			err = snd_pcm_delay (pcm, &delay);
			if (err >= 0) {
				pts -= delay * (uint64_t) 10000000 / audio->stream->sample_rate;
			}
			mplayer->SetTargetPts (pts);
			updated_pts = pts;
		}
		
		result = true;
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

AudioPlayer::AudioNode::AudioNode ()
{
	mplayer = NULL;
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
}

AudioPlayer::AudioNode::~AudioNode ()
{
	LOG_AUDIO ("AudioNode::~AudioNode ()\n");

	Close ();
}

bool
AudioPlayer::AudioNode::GetNextBuffer ()
{
	MediaFrame *frame = NULL;
	Packet *packet = NULL;
	
	if (first_buffer) {
		g_free (first_buffer);
		first_buffer = NULL;
	}

	first_used = 0;
	first_size = 0;
	first_pts = 0;

	packet = (Packet*) mplayer->audio.queue.Pop ();
	
	if (packet == NULL) {
		instance->WaitForData (this);
		return false;
	}
	
	frame = packet->frame;
	
	if (frame->event == FrameEventEOF) {
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
