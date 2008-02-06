/*
 * mplayer.cpp: 
 *
 * Authors: Jeffrey Stedfast <fejj@novell.com>
 *          Rolf Bjarne Kvinge  <RKvinge@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

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

static uint64_t audio_play (Audio *audio, struct pollfd *ufds, int nfds);
static void    *audio_loop (void *data);

extern guint32 moonlight_flags;

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
	
	// buffering
	uint8_t buffer[AUDIO_BUFLEN + 1];
	uint8_t *outbuf;
	uint8_t *outptr;
	
	// output
	snd_pcm_t *pcm;
	snd_pcm_uframes_t sample_size;
	struct pollfd *ufds;
	int nfds;
	
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
	
	memset (buffer, 0, AUDIO_BUFLEN + 1);
	outbuf = ALIGN (buffer, 2);
	outptr = outbuf;
	
	pcm = NULL;
	sample_size = 0;
	
	pts_per_frame = 0;
	
	ufds = NULL;
	nfds = 0;
}

Audio::~Audio ()
{	
	g_free (ufds);
	
	if (pcm != NULL)
		snd_pcm_close (pcm);
	
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
	media = NULL;
	
	pthread_mutex_init (&pause_mutex, NULL);
	pthread_cond_init (&pause_cond, NULL);
	playing = false;
	opened = false;
	paused = true;
	stop = false;
	eof = false;
	seeking = false;
	rendered_frame = false;
	load_frame = false;
	caught_up_with_seek = true;
	
	audio_thread = NULL;
	
	audio = new Audio ();
	video = new Video ();
	
	start_time = 0;
	
	pthread_mutex_init (&target_pts_lock, NULL);
	current_pts = 0;
	target_pts = 0;
	
	height = 0;
	width = 0;
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
	MediaPlayer *player = (MediaPlayer*) user_data;
	if (player != NULL && player->load_frame)
		return player->LoadVideoFrame ();
		
	return false;
}

MediaResult
media_player_callback (MediaClosure *closure)
{
	MediaPlayer *player = (MediaPlayer *) closure->context;
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame->stream;
	
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
			TimeManager::Instance ()->AddTimeout (0, load_video_frame, player);
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
	MediaFrame *frame;
	int states, i;
	
	if (mplayer->HasAudio ()) {	
		for (i = 0; i < audio_frames; i++) {
			frame = new MediaFrame (mplayer->audio->stream);
			
			// To decode on the main thread comment out FRAME_DECODED.
			states = FRAME_DEMUXED | FRAME_DECODED;
			
			if ((states & FRAME_DECODED) == FRAME_DECODED)
				frame->AddState (FRAME_COPY_DECODED_DATA);
			
			mplayer->media->GetNextFrameAsync (frame, states);
		}
	}
	
	if (mplayer->HasVideo ()) {
		for (i = 0; i < video_frames; i++) {
			frame = new MediaFrame (mplayer->video->stream);
			
			// To decode on the main thread comment out FRAME_DECODED.
			states = FRAME_DEMUXED | FRAME_DECODED;
			
			if ((states & FRAME_DECODED) == FRAME_DECODED)
				frame->AddState (FRAME_COPY_DECODED_DATA);
			
			mplayer->media->GetNextFrameAsync (frame, states);
		}
	}
}

bool
MediaPlayer::Open (Media *media)
{
	IMediaDecoder *encoding;
	IMediaStream *stream;
	
	//printf ("MediaPlayer::Open ().\n");
	
	if (media == NULL) {
		printf ("MediaPlayer::Open (): media is NULL.\n");
		return false;
	}
	
	if (!media->IsOpened ()) {
		printf ("MediaPlayer::open (): media isn't open.\n");
		return false;
	}
	
	this->media = media;
	opened = true;
	
	// Set our frame reader callback
	MediaClosure *closure = new MediaClosure ();
	closure->media = media;
	closure->context = this;
	closure->callback = media_player_callback;
	media->SetQueueCallback (closure);
	
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
			audio->stream = (AudioStream *) stream;
			break;
		case MediaTypeVideo: 
			video->stream = (VideoStream *) stream;
			
			height = video->stream->height;
			width = video->stream->width;
			
			// for conversion to rgb32 format needed for rendering
			video->rgb_buffer = (uint8_t *) g_malloc0 (width * height * 4);
			
			// rendering surface
			video->surface = cairo_image_surface_create_for_data (
				video->rgb_buffer, CAIRO_FORMAT_ARGB32,
				width, height, width * 4);
			
			// printf ("video size: %i, %i\n", video->stream->width, video->stream->height);
			break;
		default:
			break;
		}
	}
		
	// Prepare audio playback
	if (!(moonlight_flags & RUNTIME_INIT_DISABLE_AUDIO) && audio->pcm == NULL && HasAudio ()) {
 		if (snd_pcm_open (&audio->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0) != 0) {
 			fprintf (stderr, "Moonlight: cannot open audio device: %s\n", strerror (errno));
 			audio->pcm = NULL;
 		}
	}
	
	if (audio->pcm != NULL && HasAudio ()) {
		snd_pcm_uframes_t buf_size;
		
		snd_pcm_set_params (audio->pcm, SND_PCM_FORMAT_S16,
				    SND_PCM_ACCESS_RW_INTERLEAVED,
				    audio->stream->channels,
				    audio->stream->sample_rate,
				    1, 0);
		
		snd_pcm_get_params (audio->pcm, &buf_size, &audio->sample_size);
		
		// 2 bytes per channel, we always calculate as 2-channel audio because it gets converted into such
		audio->pts_per_frame = (buf_size * 2 * 2) / (audio->stream->sample_rate / 100);
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
MediaPlayer::Close ()
{
	Stop ();
	
	// Reset state back to what it was at instantiation
	
	if (video->rgb_buffer != NULL) {
		g_free (video->rgb_buffer);
		video->rgb_buffer = NULL;
	}
	
	if (video->surface != NULL) {
		cairo_surface_destroy (video->surface);
		video->surface = NULL;
	}
	
	media = NULL;
	
	playing = false;
	opened = false;
	eof = false;
	
	audio->stream_count = 0;
	audio->stream = NULL;
	audio->sample_size = 0;
	audio->pts_per_frame = 0;
	
	video->stream = NULL;
	
	start_time = 0;
	
	duration = 0;
	current_pts = 0;
	target_pts = 0;
	
	height = 0;
	width = 0;

	rendered_frame = false;
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
		memcpy (video->rgb_buffer, frame->buffer, MIN (frame->buflen, (uint32_t) (mplayer->width * mplayer->height * 4)));
		mplayer->rendered_frame = true;
		return;
	}
	
	if (frame->data_stride == NULL || 
		frame->data_stride[1] == NULL || 
		frame->data_stride[2] == NULL) {
		return;
	}
	
	uint8_t *rgb_dest[3] = { video->rgb_buffer, NULL, NULL };
	int rgb_stride [3] = { video->stream->width * 4, 0, 0 };
	
	stream->converter->Convert (frame->data_stride, frame->srcStride, frame->srcSlideY,
				    frame->srcSlideH, rgb_dest, rgb_stride);
	mplayer->rendered_frame = true;
}

#define DEBUG_AF 0

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
	
#if DEBUG_AF
	int skipped = 0;
#endif
	
	load_frame = false;
	
	if (paused)
		return false;
	
	if (seeking)
		return false;
	
	if (eof)
		return false;

	if (!HasVideo ())
		return false;

	if (HasAudio ()) {
		// use target_pts as set by audio thread
		target_pts = GetTargetPts ();	
	} else {
		// no audio to sync to
		uint64_t now = TimeSpan_ToPts (TimeManager::Instance ()->GetCurrentTime ());
		uint64_t elapsed_pts = now - start_time;
		
		target_pts = elapsed_pts;
		
		this->target_pts = target_pts;
	}
	
	target_pts_start = target_pts_delta > target_pts ? 0 : target_pts - target_pts_delta;
	target_pts_end = target_pts + target_pts_delta;
	
	if (current_pts >= target_pts_end && caught_up_with_seek) {
#if DEBUG_AF
		printf ("MediaPlayer::AdvanceFrame (): video is running too fast, wait a bit (current_pts: %llu, target_pts: %llu, delta: %llu, diff: %lld).\n",
			current_pts, target_pts, target_pts_delta, current_pts - target_pts);
#endif
		return false;
	}
		
	while ((pkt = (Packet *) video->queue->Pop ())) {
		if (pkt->frame->event == FrameEventEOF) {
			delete pkt;
			eof = true;
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
#if DEBUG_AF
		printf ("MediaPlayer::AdvanceFrame (): skipped frame with pts %llu, target pts: %llu, diff: %lld, milliseconds: %lld\n", frame->pts, target_pts, target_pts - frame->pts, MilliSeconds_FromPts ((target_pts - frame->pts)));
		skipped++;
#endif
		frame = NULL;
		delete pkt;
	}
	
	if (update && frame && caught_up_with_seek) {
#if DEBUG_AF
		printf ("MediaPlayer::AdvanceFrame (): rendering pts %llu (target pts: %llu, current pts: %llu, caught_up_with_seek: %s, skipped frames: %i)\n", frame->pts, target_pts, current_pts, caught_up_with_seek ? "true" : "false", skipped);
#endif
		render_frame (this, frame);
		delete pkt;
		
		return true;
	}
	
	delete pkt;
		
	return !eof;
}

bool
MediaPlayer::LoadVideoFrame ()
{
	Packet *packet;
	bool cont = false;
	
	if (!HasVideo ())
		return false;
	
	if (!load_frame)
		return false;
	
	packet = (Packet*) video->queue->Pop ();
	media_player_enqueue_frames (this, 0, 1);
	
	if (packet == NULL)
		return false;
	
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
	if (!eof)
		return false;
	
	if ((audio->queue && !audio->queue->IsEmpty ()))
		return false;
	
	if ((video->queue && !video->queue->IsEmpty ()))
		return false;
	
	return true;
}

guint
MediaPlayer::Play (GSourceFunc callback, void *user_data)
{
	if (!paused || !opened)
		return 0;
	
	if (!playing) {
		// Start up the decoder/audio threads
		if (audio->pcm != NULL && HasAudio ()) {
			pthread_mutex_lock (&audio->init_mutex);
			audio_thread = g_thread_create (audio_loop, this, true, NULL);
			pthread_cond_wait (&audio->init_cond, &audio->init_mutex);
			pthread_mutex_unlock (&audio->init_mutex);
		}
		
		playing = true;
	} else {
		// We are simply paused...
	}
	
	PauseInternal (false);
	
	start_time = TimeSpan_ToPts (TimeManager::Instance ()->GetCurrentTime ());
	seeking = false;
	
	media_player_enqueue_frames (this, 1, 1);
		
	if (HasVideo ()) {
		// TODO: Calculate correct framerate (in the pipeline)
		return TimeManager::Instance ()->AddTimeout (MAX (video->stream->msec_per_frame, 1000 / 60), callback, user_data);
	} else {
		return TimeManager::Instance ()->AddTimeout (33, callback, user_data);
	}
	
	return 0;
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
	pthread_mutex_lock (&pause_mutex);

	caught_up_with_seek = true;

	if (paused != value) {
		paused = value;
		if (!paused) {
			// Wake up the audio loop
			pthread_cond_signal (&pause_cond);
		}
	}

	pthread_mutex_unlock (&pause_mutex);
}

void
MediaPlayer::Pause ()
{
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
	
	return result;
}

void
MediaPlayer::SetTargetPts (uint64_t pts)
{
	pthread_mutex_lock (&target_pts_lock);
	target_pts = pts;
	pthread_mutex_unlock (&target_pts_lock);
}

void
MediaPlayer::IncTargetPts (uint64_t value)
{
	pthread_mutex_lock (&target_pts_lock);
	target_pts += value;
	pthread_mutex_unlock (&target_pts_lock);
}

void
MediaPlayer::StopThreads ()
{
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
	
	audio->outptr = audio->outbuf;
	
	current_pts = 0;
	target_pts = 0;
	
	stop = false;
	eof = false;
}

MediaResult
media_player_seek_callback (MediaClosure *closure)
{
	MediaPlayer *mplayer = (MediaPlayer*) closure->context;
	mplayer->seeking = false;
	mplayer->current_pts = 0;
	return MEDIA_SUCCESS;
}

void
MediaPlayer::SeekInternal (uint64_t pts)
{
	if (media == NULL)
		return;
		
	seeking = true;
	caught_up_with_seek = false;
	MediaClosure *closure = new MediaClosure ();
	closure->callback = media_player_seek_callback;
	closure->context = this;
	media->ClearQueue ();
	media->SeekAsync (pts, closure);
}

void
MediaPlayer::Stop ()
{
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
	uint64_t duration = Duration ();
	bool resume = !paused;
	
	if (!CanSeek ())
		return;
	
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
		if (audio->pcm != NULL && HasAudio ()) {
			pthread_mutex_lock (&audio->init_mutex);
			audio_thread = g_thread_create (audio_loop, this, true, NULL);
			pthread_cond_wait (&audio->init_cond, &audio->init_mutex);
			pthread_mutex_unlock (&audio->init_mutex);
		}
		
		if (resume) {
			// Resume playback
			start_time = TimeManager::Instance ()->GetCurrentTime ();
			
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

// Audio playback thread

static int
pcm_poll (snd_pcm_t *pcm, struct pollfd *ufds, int nfds)
{
	unsigned short revents;
	
	if (ufds == NULL)
		return 0;
	
	while (true) {
		poll (ufds, nfds, -1);
		snd_pcm_poll_descriptors_revents (pcm, ufds, nfds, &revents);
		if (revents & POLLERR)
			return -1;
		if (revents & POLLOUT)
			return 0;
	}
}

// Returns the frame pts or 0 if insufficient audio data
static uint64_t
audio_play (Audio *audio, struct pollfd *ufds, int nfds)
{
	int frame_size, samples, outlen, channels, n;
	uint8_t *outbuf;
	
	channels = audio->stream->channels;
	frame_size = audio->sample_size * channels * 2;
	outlen = audio->outptr - audio->outbuf;
	samples = audio->sample_size;
	outbuf = audio->outbuf;
	
	// make sure we have enough data to play a frame of audio
	if (outlen < frame_size) {
		outbuf = ALIGN (audio->buffer, 2);
		
		if (outlen > 0) {
			// make room for more audio to be buffered
			memmove (outbuf, audio->outbuf, outlen);
			audio->outptr = outbuf + outlen;
		} else {
			audio->outptr = outbuf;
		}
		
		audio->outbuf = outbuf;
		
		return 0;
	}
	
	if (!audio->muted) {
		// set balance/volume
		int16_t volume = (uint16_t) (audio->volume * 8192);
		int16_t *inptr = (int16_t *) outbuf;
		int16_t leftvol, rightvol;
		int32_t vol;
		
		if (audio->balance < 0.0) {
			rightvol = (uint16_t) (1.0 + audio->balance) * volume;
			leftvol = volume;
		} else if (audio->balance > 0.0) {
			leftvol = (uint16_t) (1.0 - audio->balance) * volume;
			rightvol = volume;
		} else {
			leftvol = rightvol = volume;
		}
		
		for (n = 0; n < frame_size / 2; n += 2) {
			vol = (((int32_t) *inptr) * ((int32_t) leftvol)) >> 13;
			*inptr++ = (int16_t) CLAMP (vol, -32768, 32767);
			
			vol = (((int32_t) *inptr) * ((int32_t) rightvol)) >> 13;
			*inptr++ = (int16_t) CLAMP (vol, -32768, 32767);
		}
	} else {
		// mute by playing dead silence
		memset (outbuf, 0, frame_size);
	}
	
	// play only 1 frame
	while (samples > 0) {
		if (pcm_poll (audio->pcm, ufds, nfds) != 0) {
			switch (snd_pcm_state (audio->pcm)) {
			case SND_PCM_STATE_XRUN:
				printf ("SND_PCM_STATE_XRUN\n");
				snd_pcm_prepare (audio->pcm);
				break;
			case SND_PCM_STATE_SUSPENDED:
				printf ("SND_PCM_STATE_SUSPENDED\n");
				while ((n = snd_pcm_resume (audio->pcm)) == -EAGAIN)
					sleep (1);
				
				if (n < 0) {
					snd_pcm_prepare (audio->pcm);
					snd_pcm_start (audio->pcm);
				}
				break;
			default:
				break;
			}
		} else {
			if ((n = snd_pcm_writei (audio->pcm, outbuf, samples)) > 0) {
				outbuf += (n * 2 * channels);
				samples -= n;
			} else if (n == -ESTRPIPE) {
				while ((n = snd_pcm_resume (audio->pcm)) == -EAGAIN)
					sleep (1);
				
				if (n < 0) {
					snd_pcm_prepare (audio->pcm);
					snd_pcm_start (audio->pcm);
				}
			} else if (n == -EPIPE) {
				snd_pcm_prepare (audio->pcm);
			}
		}
	}
	
	audio->outbuf = outbuf;
	
	return audio->pts_per_frame;
}

static void *
audio_loop (void *data)
{
	MediaPlayer *mplayer = (MediaPlayer *) data;
	Audio *audio = mplayer->audio;
	struct pollfd *ufds = NULL;
	IMediaStream *stream;
	uint64_t frame_pts;
	Packet *pkt = NULL;
	MediaFrame *frame;
	uint8_t *outend;
	uint32_t inleft;
	uint8_t *inptr;
	uint32_t n;
	int ndfs;
	
	outend = audio->outbuf + AUDIO_BUFLEN;
	inptr = NULL;
	inleft = 0;
	
	pthread_mutex_lock (&mplayer->audio->init_mutex);
	
	if ((ndfs = snd_pcm_poll_descriptors_count (audio->pcm)) > 0) {
		ufds = (struct pollfd *) g_malloc0 (sizeof (struct pollfd) * ndfs);
		
		if (snd_pcm_poll_descriptors (audio->pcm, ufds, ndfs) < 0) {
			g_free (ufds);
			ufds = NULL;
		}
	}
	
	// signal the main thread that we are ready to begin audio playback
	pthread_cond_signal (&mplayer->audio->init_cond);
	pthread_mutex_unlock (&mplayer->audio->init_mutex);
	
	while (!mplayer->stop) {
		pthread_mutex_lock (&mplayer->pause_mutex);
		while (mplayer->paused && !mplayer->stop) {
			//printf ("audio_loop (): paused.\n");
			// wait for main thread to relinquish pause lock
			pthread_cond_wait (&mplayer->pause_cond, &mplayer->pause_mutex);
			//printf ("audio_loop (): resumed.\n");
		}
		pthread_mutex_unlock (&mplayer->pause_mutex);
		
		if (mplayer->stop)
			break;
		
		if ((frame_pts = audio_play (audio, ufds, ndfs)) > 0) {
			// calculated pts
			mplayer->IncTargetPts (frame_pts);
		} else {
			if (!pkt && (pkt = (Packet *) audio->queue->Pop ())) {
				// decode an audio packet
				stream = pkt->frame->stream;
				frame = pkt->frame;
				
				if (frame->event == FrameEventEOF) {
					mplayer->eof = true;
					break;
				}
				
				if (!frame->IsDecoded ())
					stream->decoder->DecodeFrame (frame);
				
				mplayer->SetTargetPts (frame->pts);
				
				inleft = frame->buflen;
				inptr = frame->buffer;
				
				media_player_enqueue_frames (mplayer, 1, 0);
			} else if (!pkt) {
				// need to either wait for another
				// audio packet to be queued
			}
			
			if (pkt != NULL) {
				// calculate how much room in the output buffer we have
				n = (outend - audio->outptr);
				
				if (inleft <= n) {
					// blit the remainder of the decompressed audio frame
					memcpy (audio->outptr, inptr, inleft);
					audio->outptr += inleft;
					
					inptr = NULL;
					inleft = 0;
					delete pkt;
					pkt = NULL;
				} else {
					// blit what we can...
					memcpy (audio->outptr, inptr, n);
					audio->outptr += n;
					inleft -= n;
					inptr += n;
				}
			}
		}
	}
	
	//printf ("audio_loop (): exited.\n");
	
	if (pkt != NULL)
		delete pkt;
	
	g_free (ufds);
	
	return NULL;
}
