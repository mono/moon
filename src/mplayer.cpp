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

#if GLIB_SIZEOF_VOID_P == 8
#define ALIGN(addr,size) (uint8_t *) (((uint64_t) (((uint8_t *) (addr)) + (size) - 1)) & ~((size) - 1))
#else
#define ALIGN(addr,size) (uint8_t *) (((uint32_t) (((uint8_t *) (addr)) + (size) - 1)) & ~((size) - 1))
#endif

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#define AUDIO_BUFLEN (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)

#define LOG_MEDIAPLAYER(...)// printf (__VA_ARGS__);
#define DEBUG_ADVANCEFRAME 0
#define LOG_AUDIO(...)// printf (__VA_ARGS__)
// This one prints out spew on every frame/sample
#define LOG_AUDIO_EX(...)// printf (__VA_ARGS__)

class Packet : public List::Node {
public:
	MediaFrame *frame;
	
	Packet (MediaFrame *frame);
	virtual ~Packet ();
};

Packet::Packet (MediaFrame *frame)
{
	this->frame = frame;
}

Packet::~Packet ()
{
	delete frame;
}


struct Audio {
	Queue *queue;
	
	pthread_mutex_t init_mutex;
	pthread_cond_t init_cond;
	
	double balance;
	double volume;
	bool muted;
	
	// input
	int stream_count;
	AudioStream *stream;
	
	// sync
	uint64_t pts_per_frame;
	
	Audio ();
	~Audio ();
};

Audio::Audio ()
{
	pthread_mutex_init (&init_mutex, NULL);
	pthread_cond_init (&init_cond, NULL);
	
	queue = new Queue ();
	
	balance = 0.0f;
	volume = 1.0f;
	muted = false;
	
	stream_count = 0;
	stream = NULL;
	
	pts_per_frame = 0;
}

Audio::~Audio ()
{	
	delete queue;
}

struct Video {
	Queue *queue;
	
	// input
	VideoStream *stream;
	
	// rendering
	cairo_surface_t *surface;
	uint8_t *rgb_buffer;
	
	Video ();
	~Video ();
};

Video::Video ()
{
	queue = new Queue ();
	
	stream = NULL;
	
	surface = NULL;
	rgb_buffer = NULL;
}

Video::~Video ()
{
	delete queue;
}

MediaPlayer::MediaPlayer (MediaElement *el)
{
	element = el;
	pthread_mutex_init (&pause_mutex, NULL);
	pthread_cond_init (&pause_cond, NULL);
	pthread_mutex_init (&target_pts_lock, NULL);

	media = NULL;
	audio_thread = NULL;
	
	audio = new Audio ();
	video = new Video ();
	
	Initialize ();
}

MediaPlayer::~MediaPlayer ()
{
	Close ();
	
	pthread_mutex_destroy (&target_pts_lock);
	pthread_mutex_destroy (&pause_mutex);
	pthread_cond_destroy (&pause_cond);
	
	delete audio;
	delete video;
}

gboolean
load_video_frame (void *user_data)
{
	LOG_MEDIAPLAYER ("load_video_frame\n");

	MediaPlayer *player = (MediaPlayer*) user_data;
	if (player != NULL && player->load_frame)
		return player->LoadVideoFrame ();
		
	return false;
}

MediaResult
media_player_callback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *player = element->GetMediaPlayer ();
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame ? frame->stream : NULL;
	
	//printf ("media_player_callback (%p), seeking: %i, frame: %p\n", closure, player->seeking, closure->frame);

	if (player->seeking) {
		// We don't want any frames while we're waiting for a seek.
		return MEDIA_SUCCESS;
	}
	
	if (closure->frame == NULL)
		return MEDIA_SUCCESS;
	
	closure->frame = NULL;
	
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		player->video->queue->Push (new Packet (frame));
		if (player->load_frame) {
			// We need to call LoadVideoFrame on the main thread
			TimeManager::InvokeOnMainThread (load_video_frame, player);
		}
		return MEDIA_SUCCESS;
	case MediaTypeAudio: 
		player->audio->queue->Push (new Packet (frame));
		return MEDIA_SUCCESS;
	default:
		return MEDIA_SUCCESS;
	}
}

void
media_player_enqueue_frames (MediaPlayer *mplayer, int audio_frames, int video_frames)
{
	MediaElement *element = mplayer->element;
	MediaClosure *closure;
	int states;

	//printf ("media_player_enqueue_frames (%p, %i, %i)\n", mplayer, audio_frames, video_frames);
	
	if (mplayer->HasAudio ()) {	
		for (int i = 0; i < audio_frames; i++) {
			closure = new MediaClosure (media_player_callback);
			closure->SetContext (element);
			
			// To decode on the main thread comment out FRAME_DECODED and FRAME_COPY_DECODED_DATA.
			states = FRAME_DEMUXED | FRAME_DECODED | FRAME_COPY_DECODED_DATA;
			
			mplayer->media->GetNextFrameAsync (closure, mplayer->audio->stream, states);
		}
	}
	
	if (mplayer->HasVideo ()) {
		for (int i = 0; i < video_frames; i++) {
			closure = new MediaClosure (media_player_callback);
			closure->SetContext (element);
			
			// To decode on the main thread comment out FRAME_DECODED and FRAME_COPY_DECODED_DATA.
			states = FRAME_DEMUXED | FRAME_DECODED | FRAME_COPY_DECODED_DATA;
				
			mplayer->media->GetNextFrameAsync (closure, mplayer->video->stream, states);
		}
	}
}

bool
MediaPlayer::Open (Media *media)
{
	IMediaDecoder *encoding;
	IMediaStream *stream;
	
	LOG_MEDIAPLAYER ("MediaPlayer::Open (%p), current media: %p\n", media, this->media);
	
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
	opened = true;
	int stride;
	
	// Find audio/video streams
	IMediaDemuxer *demuxer = media->GetDemuxer ();
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		stream = demuxer->GetStream (i);
		encoding = stream->GetDecoder (); //stream->codec;
		
		if (encoding == NULL)
			continue; // No encoding was found for the stream.
		
		switch (stream->GetType ()) {
		case MediaTypeAudio:
			audio->stream_count++;			
			if (audio->stream == NULL) {
				audio->stream = (AudioStream *) stream;
				audio->stream->selected = true;
			}
			break;
		case MediaTypeVideo: 
			if (video->stream != NULL)
				break;

			video->stream = (VideoStream *) stream;
			video->stream->selected = true;
			
			height = video->stream->height;
			width = video->stream->width;

			stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
			if (stride % 16) {
				int remain = stride % 16;
				stride += 16 - remain;
			}
			
			// for conversion to rgb32 format needed for rendering with 16 byte alignment
			if (posix_memalign ((void **)(&video->rgb_buffer), 16, height * stride)) {
				g_error ("Could not allocate memory");
			}
			
			// rendering surface
			video->surface = cairo_image_surface_create_for_data (
				video->rgb_buffer, CAIRO_FORMAT_ARGB32,
				width, height, stride);
			
			// printf ("video size: %i, %i\n", video->stream->width, video->stream->height);
			break;
		default:
			break;
		}
	}

	if (audio->stream != NULL) {
		if (!AudioPlayer::Add (this)) {
			// Can't play audio
			audio->stream->selected = false;
			audio->stream = NULL;
		}
	}
	
	if (HasVideo ()) {
		load_frame = true;
		media_player_enqueue_frames (this, 0, 1);
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

	playing = false;
	opened = false;
	paused = true;
	stop = false;
	eof = false;
	seeking = false;
	rendered_frame = false;
	load_frame = false;
	caught_up_with_seek = true;
	
	start_time = 0;
	current_pts = 0;
	target_pts = 0;
	
	height = 0;
	width = 0;
}

void
MediaPlayer::Close ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Close ()\n");

	AudioPlayer::Remove (this);

	Stop ();
	
	// Reset state back to what it was at instantiation

	if (video->rgb_buffer != NULL) {
		free (video->rgb_buffer);
		video->rgb_buffer = NULL;
	}

	if (video->surface != NULL) {
		cairo_surface_destroy (video->surface);
		video->surface = NULL;
	}
	video->stream = NULL;
	
	audio->stream_count = 0;
	audio->stream = NULL;
	audio->pts_per_frame = 0;
	
	if (media)
		media->unref ();
	media = NULL;

	Initialize ();
}

//
// Puts the data into our rgb buffer.
// If necessary converts the data from its source format to rgb first.
//

static void
render_frame (MediaPlayer *mplayer, MediaFrame *frame)
{
	VideoStream *stream = (VideoStream *) frame->stream;
	Video *video = mplayer->video;
	
	if (!frame->IsDecoded ()) {
		fprintf (stderr, "render_frame (): Trying to render a frame which hasn't been decoded yet.\n");
		return;
	}
	
	if (!frame->IsPlanar ()) {
		// Just copy the data
		memcpy (video->rgb_buffer, frame->buffer, MIN (frame->buflen, (uint32_t) (cairo_image_surface_get_stride (video->surface) * mplayer->height)));
		mplayer->rendered_frame = true;
		return;
	}
	
	if (frame->data_stride == NULL || 
		frame->data_stride[1] == NULL || 
		frame->data_stride[2] == NULL) {
		return;
	}
	
	uint8_t *rgb_dest[3] = { video->rgb_buffer, NULL, NULL };
	int rgb_stride [3] = { cairo_image_surface_get_stride (video->surface), 0, 0 };
	
	stream->converter->Convert (frame->data_stride, frame->srcStride, frame->srcSlideY,
				    frame->srcSlideH, rgb_dest, rgb_stride);
	mplayer->rendered_frame = true;
}


bool
MediaPlayer::AdvanceFrame ()
{
	//printf ("MediaPlayer::AdvanceFrame ()\n");
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
	
	load_frame = false;
	
	if (paused)
		return false;
	
	if (seeking)
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
	
	if (current_pts >= target_pts_end && caught_up_with_seek) {
#if DEBUG_ADVANCEFRAME
		printf ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %llu, target_pts: %llu, delta: %llu, diff: %lld).\n",
			current_pts, target_pts, target_pts_delta, current_pts - target_pts);
#endif
		return false;
	}
		
	while ((pkt = (Packet *) video->queue->Pop ())) {
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
		
		media_player_enqueue_frames (this, 0, 1);	
		
		if (!frame->IsDecoded ()) {
			//printf ("MediaPlayer::AdvanceFrame (): decoding on main thread.\n");
			MediaResult result = stream->decoder->DecodeFrame (frame);
			
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("MediaPlayer::AdvanceFrame (): Couldn't decode frame (%i)\n", result);
				update = false;
			}
		}
		
		if (update && current_pts >= target_pts_start) {
			caught_up_with_seek = true;
			// we are in sync (or ahead) of audio playback
			break;
		}
		
		if (video->queue->IsEmpty ()) {
			// no more packets in queue, this frame is the most recent we have available
			media_player_enqueue_frames (this, 0, 1);
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
	
	if (update && frame && caught_up_with_seek) {
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
		render_frame (this, frame);
		delete pkt;
		
		return true;
	}
	
	delete pkt;
		
	return !GetEof ();
}

bool
MediaPlayer::LoadVideoFrame ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame ()\n");

	Packet *packet;
	bool cont = false;
	
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), HasVideo: %i, load_frame: %i, queue size: %i\n", HasVideo (), load_frame, video->queue->Length ());

	if (!HasVideo ())
		return false;
	
	if (!load_frame)
		return false;
	
	packet = (Packet*) video->queue->Pop ();

	if (packet != NULL && packet->frame->event == FrameEventEOF)
		return false;

	media_player_enqueue_frames (this, 0, 1);
	
	if (packet == NULL)
		return false;
	
	LOG_MEDIAPLAYER ("MediaPlayer::LoadVideoFrame (), packet pts: %llu, target pts: %llu\n", packet->frame->pts, GetTargetPts ());

	if (packet->frame->pts >= GetTargetPts ()) {
		load_frame = false;
		render_frame (this, packet->frame);
		element->Invalidate ();
	} else {
		cont = true;
	}
	
	delete packet;
	
	return cont;
}

cairo_surface_t *
MediaPlayer::GetSurface ()
{
	return video->surface;
}

bool
MediaPlayer::IsPlaying ()
{
	return playing && !paused;
}

bool
MediaPlayer::MediaEnded ()
{
	if (!GetEof ())
		return false;
	
	if ((audio->queue && !audio->queue->IsEmpty ()))
		return false;
	
	if ((video->queue && !video->queue->IsEmpty ()))
		return false;
	
	return true;
}

void
MediaPlayer::SetEof (bool value)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SetEof (%i)\n", value);
	eof = (gint) value; //g_atomic_int_set (&eof, (gint) value);
}

bool
MediaPlayer::GetEof ()
{
	return g_atomic_int_get (&eof);
}

void
MediaPlayer::Play ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Play (), paused: %i, opened: %i, seeking: %i, playing: %i\n", paused, opened, seeking, playing);

	if (!paused || !opened)
		return;
	
	if (!playing) {
		// Start up the decoder/audio threads
		AudioPlayer::Play (this);
		playing = true;
	} else {
		// We are simply paused...
	}
	
	PauseInternal (false);
	
	start_time = TimeSpan_ToPts (element->GetTimeManager()->GetCurrentTime ());
	seeking = false;
	
	media_player_enqueue_frames (this, 1, 1);

	LOG_MEDIAPLAYER ("MediaPlayer::Play (), paused: %i, opened: %i, seeking: %i, playing: %i [Done]\n", paused, opened, seeking, playing);
}

gint32
MediaPlayer::GetTimeoutInterval ()
{
	if (HasVideo ()) {
		// TODO: Calculate correct framerate (in the pipeline)
		return MAX (video->stream->msec_per_frame, 1000 / 60);
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

bool
MediaPlayer::IsPaused ()
{
	return paused;
}

void
MediaPlayer::PauseInternal (bool value)
{
	LOG_MEDIAPLAYER ("MediaPlayer::PauseInternal (%i), paused: %i\n", value, paused);	

	pthread_mutex_lock (&pause_mutex);

	caught_up_with_seek = true;

	if (paused != value) {
		paused = value;
		AudioPlayer::Pause (this, paused);
		if (!paused) {
			// Wake up the audio loop
			pthread_cond_signal (&pause_cond);
		}
	}

	pthread_mutex_unlock (&pause_mutex);
	LOG_MEDIAPLAYER ("MediaPlayer::PauseInternal (%i), paused: %i [Done]\n", value, paused);	
}

void
MediaPlayer::Pause ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Pause (), paused: %i\n", paused);

	if (paused || !CanPause ())
		return;
	
	PauseInternal (true);
}

uint64_t
MediaPlayer::GetTargetPts ()
{
	uint64_t result;
	
	pthread_mutex_lock (&target_pts_lock);
	result = target_pts;
	pthread_mutex_unlock (&target_pts_lock);
	
	//printf ("MediaPlayer::GetTargetPts (): result: %llu\n", result);
	
	return result;
}

void
MediaPlayer::SetTargetPts (uint64_t pts)
{
	//printf ("MediaPlayer::SetTargetPts (%llu = %llu ms)\n", pts, MilliSeconds_FromPts (pts));
	pthread_mutex_lock (&target_pts_lock);
	target_pts = pts;
	pthread_mutex_unlock (&target_pts_lock);
}

void
MediaPlayer::IncTargetPts (uint64_t value)
{
	//printf ("MediaPlayer::IncTargetPts (%llu): final target_pts: %llu\n", value, value + target_pts);
	
	pthread_mutex_lock (&target_pts_lock);
	target_pts += value;
	pthread_mutex_unlock (&target_pts_lock);
}

void
MediaPlayer::StopThreads ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::StopThreads ()\n");

	stop = true;
	
	PauseInternal (false);
		
	if (audio_thread != NULL) {
		g_thread_join (audio_thread);
		audio_thread = NULL;
	}
	
	audio->queue->Clear (true);
	video->queue->Clear (true);
	
	// enter paused state
	PauseInternal (true);
	
	start_time = 0;
	
	current_pts = 0;
	target_pts = 0;
	
	stop = false;
	eof = false;
}

MediaResult
media_player_seek_callback (MediaClosure *closure)
{
	MediaElement *element = (MediaElement *) closure->GetContext ();
	MediaPlayer *mplayer = element->GetMediaPlayer ();
	mplayer->seeking = false;
	mplayer->current_pts = 0;
	return MEDIA_SUCCESS;
}

void
MediaPlayer::SeekInternal (uint64_t pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::SeekInternal (%llu), media: %p, seeking: %i, Position (): %llu\n", pts, media, seeking, Position ());

	if (media == NULL)
		return;
		
	if (pts == 0) {
		media->SeekToStart ();
		return;
	}

	if (pts == Position ())
		return;

	seeking = true;
	caught_up_with_seek = false;
	MediaClosure *closure = new MediaClosure (media_player_seek_callback);
	closure->SetContext (element);
	media->ClearQueue ();
	media->SeekAsync (pts, closure);
}

void
MediaPlayer::Stop ()
{
	LOG_MEDIAPLAYER ("MediaPlayer::Stop ()\n");

	StopThreads ();
	
	playing = false;
	SeekInternal (0);
}

bool
MediaPlayer::CanSeek ()
{
	// FIXME: should return false if it is streaming media
	return true;
}

void
MediaPlayer::Seek (uint64_t pts)
{
	LOG_MEDIAPLAYER ("MediaPlayer::Seek (%llu), media: %p, seeking: %i\n", pts, media, seeking);

	uint64_t duration = Duration ();
	bool resume = !paused;
	
	if (!CanSeek ())
		return;
	
	if (pts == 0) {
		SeekInternal (0);
		return;
	}

	seeking = true;
	
	if (pts > duration)
		pts = duration;
	
	StopThreads ();
	SeekInternal (pts);
	
	load_frame = !(playing && resume);
	current_pts = pts;
	target_pts = pts;
	
	if (playing) {
		media_player_enqueue_frames (this, 4, 4);
		 
		// Restart the audio/io threads
		AudioPlayer::Play (this);
		
		if (resume) {
			// Resume playback
			start_time = element->GetTimeManager()->GetCurrentTime ();
			
			PauseInternal (false);
		}
	}
	
	if (load_frame)
		media_player_enqueue_frames (this, 0, 1);
}

uint64_t
MediaPlayer::Position ()
{
	return GetTargetPts ();
}

uint64_t
MediaPlayer::Duration ()
{
	return duration;
}

void
MediaPlayer::SetMuted (bool value)
{
	audio->muted = value;
}

bool
MediaPlayer::IsMuted ()
{
	return audio->muted;
}

int
MediaPlayer::GetAudioStreamCount ()
{
	//printf ("MediaPlayer::GetAudioStreamCount ().\n");
	return audio->stream_count;
}

bool
MediaPlayer::HasVideo ()
{
	return video->stream != NULL;
}

bool
MediaPlayer::HasAudio ()
{
	return audio->stream != NULL;
}

double
MediaPlayer::GetBalance ()
{
	return audio->balance;
}

void
MediaPlayer::SetBalance (double balance)
{
	if (balance < -1.0)
		balance = -1.0;
	else if (balance > 1.0)
		balance = 1.0;
	
	audio->balance = balance;
}

double
MediaPlayer::GetVolume ()
{
	return audio->volume;
}

void
MediaPlayer::SetVolume (double volume)
{
	if (volume < -1.0)
		volume = -1.0;
	else if (volume > 1.0)
		volume = 1.0;
	
	audio->volume = volume;
}

/*
 * AudioPlayer
 */

AudioPlayer *AudioPlayer::instance = NULL;

AudioPlayer*
AudioPlayer::Instance ()
{
	// There is no need to lock here 
	// since we should only be called
	// on the main thread.

	if (instance == NULL)
		instance = new AudioPlayer ();

	return instance;
}

void
AudioPlayer::Shutdown ()
{
	delete instance;
	instance = NULL;
}

bool
AudioPlayer::Initialize ()
{
	return Instance () != NULL;
}

AudioPlayer::AudioPlayer ()
{
	int result;

	audio_thread = NULL;
	
	shutdown = false;
	initialized = false;

	pthread_mutex_init (&list_mutex, NULL);
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
	ndfs = 1;
	udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
	udfs [0].fd = fds [0];
	udfs [0].events = POLLIN;
		
	result = pthread_create (&audio_thread, NULL, Loop, this);
	if (result != 0) {
		fprintf (stderr, "AudioPlayer::Initialize (): could not create audio thread (error code: %i).\n", result);
		return;
	}
	
	initialized = true;
	
	LOG_AUDIO ("AudioPlayer::Initialize (): the audio player has been initialized.");
}

AudioPlayer::~AudioPlayer ()
{
	LOG_AUDIO ("AudioPlayer::~AudioPlayer (): the audio player is being shut down.\n");
	
	int result;
	
	if (!initialized)
		return;
	
	if (shutdown)
		return;
	
	shutdown = true;
	
	WakeUp ();
	
	result = pthread_join (audio_thread, NULL);
	if (result != 0) {
		fprintf (stderr, "AudioPlayer::~AudioPlayer (): failed to join the audio thread (error code: %i).\n", result);
	}

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
	
	shutdown = false;
	initialized = false;

	pthread_mutex_destroy (&list_mutex);
	
	LOG_AUDIO ("AudioPlayer::~AudioPlayer (): the audio player has been shut down.\n");
}

bool
AudioPlayer::Add (MediaPlayer *mplayer)
{
	if (!Initialize ())
		return false;

	return Instance ()->AddInternal (mplayer);
}

bool
AudioPlayer::AddInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	
	if (!Initialize ())
		return false;

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

	Unlock ();

	return true;
}

void
AudioPlayer::Remove (MediaPlayer *mplayer)
{
	if (!Initialize ())
		return;
	
	Instance ()->RemoveInternal (mplayer);
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
	Unlock ();
}

void
AudioPlayer::Play (MediaPlayer *mplayer)
{
	if (!Initialize ())
		return;

	Instance ()->PlayInternal (mplayer);
}


void
AudioPlayer::PlayInternal (MediaPlayer *mplayer)
{
	LOG_AUDIO ("AudioPlayer::PlayInternal (%p)\n", mplayer);

	AudioNode *node = Find (mplayer);
	
	if (node == NULL)
		return;
	
	node->state = Playing;
	node->Unlock ();

	UpdatePollList (false);
	
	WakeUp ();
}

AudioPlayer::AudioNode*
AudioPlayer::Find (MediaPlayer *mplayer)
{
	AudioNode *result = NULL;
	
	Lock();
	
	for (uint32_t i = 0; i < list_count; i++) {
		if (list [i]->mplayer == mplayer) {
			result = list [i];
			result->Lock ();
			break;
		}	
	}
	
	Unlock ();
	
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

	/*
	 * We can't just wake up the poll and then lock on a poll mutex,
	 * since when we lock the mutex we might be in a poll again.
	 * So we lock the list mutex, and then wake up the poll. This means that 
	 * the audio loop will be waiting for the list mutex, instead of entering
	 * another poll.
	 */

	if (!locked) {
		Lock ();
	}

	WakeUp ();
	
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
	
	while (!shutdown) {
		Lock ();

		current = NULL;
		lc = list_count;
		if (list_count > 0) {
			// Get the next node in the list
			if (current_index >= list_count)
				current_index = 0;
			current = list [current_index];
			current_index++;
			if (current != NULL) {
				current->Lock ();
			}
		}
				
		Unlock ();

		pc++;

		// Play something from the node we got
		if (current != NULL) {
			if (current->state == Playing) {
				if (current->Play ())
					pc = 0;
			}
			current->Unlock ();
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
				LOG_AUDIO_EX ("AudioPlayer::Loop (): poll result: %i, fd: %i, fd [0].revents: %i, errno: %i, err: %s, ndfs = %i\n", result, udfs [0].fd, (int) udfs [0].revents, errno, strerror (errno), ndfs);
	
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
		}
	}
			
	LOG_AUDIO ("AudioPlayer: exiting audio loop.\n");
}

void
AudioPlayer::Pause (MediaPlayer *mplayer, bool value)
{
	if (!Initialize ())
		return;

	Instance ()->PauseInternal (mplayer, value);
}

void
AudioPlayer::PauseInternal (MediaPlayer *mplayer, bool value)
{
	AudioNode *node = Find (mplayer);
	int err = 0;
	
	if (node == NULL)
		return;
	
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
	node->Unlock ();
	UpdatePollList (false);
}

void
AudioPlayer::Lock ()
{
	LOG_AUDIO_EX ("AudioPlayer::Lock ()\n");
	pthread_mutex_lock (&Instance ()->list_mutex);
}

void
AudioPlayer::Unlock ()
{
	LOG_AUDIO_EX ("AudioPlayer::UnLock ()\n");
	pthread_mutex_unlock (&Instance ()->list_mutex);
}

void
AudioPlayer::WakeUp ()
{
	int result;
		
	if (!Initialize ())
		return;

	LOG_AUDIO ("AudioPlayer::WakeUp ()\n");
	
	// Write until something has been written.	
	do {
		result = write (Instance ()->fds [1], "c", 1);
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
	int channels = mplayer->audio->stream->channels;
	unsigned int rate = mplayer->audio->stream->sample_rate;
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
	Audio *audio = mplayer->audio;
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
				sent_pts = first_pts + sent_samples * 10000000 / mplayer->audio->stream->sample_rate;
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
				pts -= delay * (uint64_t) 10000000 / mplayer->audio->stream->sample_rate;
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
	
	stream = mplayer->audio->stream;
	
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
	pthread_mutex_init (&mutex, NULL);
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

	packet = (Packet*) mplayer->audio->queue->Pop ();
	
	if (packet == NULL)
		return false;
	
	frame = packet->frame;
	
	if (frame->event == FrameEventEOF) {
		mplayer->SetEof (true);
		return false;
	}
	
	if (!frame->IsDecoded ())
		frame->stream->decoder->DecodeFrame (frame);
		
	media_player_enqueue_frames (mplayer, 1, 0);
				
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
	
	Lock ();
	
	if (pcm != NULL) {
		snd_pcm_close (pcm);
		pcm = NULL;
	}
	
	g_free (udfs);
	udfs = NULL;
	
	g_free (first_buffer);
	first_buffer = NULL;	

	Unlock ();
	
	pthread_mutex_destroy (&mutex);
}
 
void
AudioPlayer::AudioNode::Lock ()
{
	LOG_AUDIO_EX ("AudioNode::Lock () %p\n", this);
	pthread_mutex_lock (&mutex);
}

void
AudioPlayer::AudioNode::Unlock ()
{
	LOG_AUDIO_EX ("AudioNode::UnLock () %p\n", this);
	pthread_mutex_unlock (&mutex);
}


