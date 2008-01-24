/*
 * mplayer2.cpp: 
 *
 * Authors
 * 	Jeffrey Stedfast <fejj@novell.com>
 *  Rolf Bjarne Kvinge  <RKvinge@novell.com>
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
	frame = NULL;
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
	ufds = NULL;

	if (pcm != NULL) {
		snd_pcm_close (pcm);
		pcm = NULL;
	}
	
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

MediaPlayer::MediaPlayer ()
{
	media = NULL;
	start_pts = 0;
	
	pthread_mutex_init (&pause_mutex, NULL);
	pthread_cond_init (&pause_cond, NULL);
	pthread_mutex_lock (&pause_mutex);
	playing = false;
	opened = false;
	paused = true;
	stop = false;
	eof = false;
	
	audio_thread = NULL;
	
	audio = new Audio ();
	video = new Video ();
	
	pause_time = 0;
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
	
	pthread_mutex_unlock (&pause_mutex);
	pthread_mutex_destroy (&pause_mutex);
	pthread_cond_destroy (&pause_cond);
	
	delete audio;
	delete video;
	delete media;
}

MediaResult
media_player_callback (MediaClosure *closure)
{
	MediaPlayer *player = (MediaPlayer *) closure->context;
	MediaFrame *frame = closure->frame;
	IMediaStream *stream = frame->stream;
	
	if (closure->frame == NULL)
		return MEDIA_SUCCESS;
	
	closure->frame = NULL;
	
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		player->video->queue->Push (new Packet (frame));
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

	//printf ("MediaPlayer2::Open ().\n");

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
 			fprintf (stderr, "cannot open audio device: %s\n", strerror (errno));
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
		
		target_pts = audio->stream->start_time;
		//printf ("initial pts (according to audio): %lld\n", target_pts);
	}
	
	if (HasVideo ()) {
		media_player_enqueue_frames (this, 0, 10);
		
		target_pts = video->stream->start_time;
		//printf ("initial pts (according to video): %lld\n", target_pts);
	}
	
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
	
	delete media;
	media = NULL;
	
	playing = false;
	opened = false;
	eof = false;
	
	audio->stream_count = 0;
	audio->stream = NULL;
	audio->sample_size = 0;
	audio->pts_per_frame = 0;
	
	video->stream = NULL;
	
	pause_time = 0;
	start_time = 0;
	
	current_pts = 0;
	target_pts = 0;
	start_pts = 0;
//	seek_pts = 0;
	
	height = 0;
	width = 0;
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
	
	if (!frame->IsPlanar ()) {
		// Just copy the data
		memcpy (video->rgb_buffer, frame->buffer, MIN (frame->buflen, (uint32_t) (mplayer->width * mplayer->height * 4)));
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
}

bool
MediaPlayer::AdvanceFrame ()
{
	//printf ("MediaPlayer::AdvanceFrame ()\n");
	Packet *pkt = NULL, *npkt = NULL;
	MediaFrame *frame = NULL;
	IMediaStream *stream;
	bool update = false;
	uint64_t target_pts;
	List *list;
	
	if (paused) {
		// shouldn't happen, but just in case
		printf ("WARNING: MediaPlayer::AdvanceFrame() called when paused\n");
		return false;
	}
	
	if (!HasAudio ()) {
		// no audio to sync to
		uint64_t now = TimeManager::Instance()->GetCurrentTimeUsec();
		
		uint64_t elapsed_usec = now - start_time;
		uint64_t elapsed_pts = (uint64_t) (elapsed_usec / 1000);
		
		target_pts = start_pts + elapsed_pts;
		
		// (no need to lock since there's no audio thread) pthread_mutex_lock (&target_pts_lock);
		this->target_pts = target_pts;
		// pthread_mutex_unlock (&target_pts_lock);
	} else if (!HasVideo ()) {
		// No video, return false if we've reached the end of the audio or true otherwise
		return !MediaEnded ();
	} else {
		// use target_pts as set by audio thread
		pthread_mutex_lock (&target_pts_lock);
		target_pts = this->target_pts;
		//printf ("AdvanceFrame (), syncing to audio, target_pts: %lld\n", target_pts);
		pthread_mutex_unlock (&target_pts_lock);
	}
	
	if (current_pts >= target_pts) {
		// we are ahead of playback
		return !eof;
	} else if (eof)
		return false;
	
	video->queue->Lock ();
	
	list = video->queue->LinkedList ();
	
	if ((pkt = (Packet *) list->First ())) {
		if (pkt->frame->event == FrameEventEOF) {
			list->Unlink (pkt);
			delete pkt;
			eof = true;
			
			video->queue->Unlock ();
			
			return false;
		}
		
		do {
			// always decode the frame or we get glitches in the screen
			frame = pkt->frame;
			stream = frame->stream;
			update = true;
			
			npkt = (Packet *) pkt->next;
			
			current_pts = frame->pts;
			list->Unlink (pkt);
			
			media_player_enqueue_frames (this, 0, 1);	
			
			if (!frame->IsDecoded ()) {
				//printf ("MediaPlayer::AdvanceFrame (): decoding on main thread.\n");
				MediaResult result = stream->decoder->DecodeFrame (frame);
				
				if (!MEDIA_SUCCEEDED (result)) {
					printf ("MediaPlayer::AdvanceFrame (): Couldn't decode frame.\n");
					update = false;
				}
			}
			
			if (update && current_pts >= target_pts) {
				// we are in sync (or ahead) of audio playback
				break;
			}
			
			if (!npkt) {
				// no more packets in queue, this frame is the most recent we have available
				media_player_enqueue_frames (this, 0, 1);
				//printf ("Requested an extra frame.\n");
				//printf ("MediaPlayer::AdvanceFrame () no more packets in queue (current_pts = %lld, seek_pts = %lld, target_pts = %lld).\n", current_pts, seek_pts, target_pts);
				break;
			}
			
			if (npkt->frame->event == FrameEventEOF) {
				// We've reached the EOF, current
				// frame is the most recent we can
				// render
				list->Unlink (npkt);
				delete npkt;
				eof = true;
				break;
			}
			
			// we are lagging behind, drop this frame
			frame = NULL;
			delete pkt;
			
			pkt = npkt;
		} while (pkt);
	}
	
	video->queue->Unlock ();
	
	if (update && frame) {
		render_frame (this, frame);
		delete pkt;
		
		return true;
	}
	
	delete pkt;
		
	return !eof;
}

void
MediaPlayer::LoadVideoFrame ()
{
	//bool update = false;
	MediaFrame *frame = NULL;
	MediaResult result;
	
	if (!HasVideo ())
		return;
	
	// TODO: This must be made async, otherwise it may dead-lock.
	frame = new MediaFrame (video->stream);		
	result = media->GetNextFrame (frame);
	
	if (MEDIA_SUCCEEDED (result))
		render_frame (this, frame);
	
	delete frame;
}

void
MediaPlayer::Render (cairo_t *cr)
{
	printf ("MediaPlayer::Render ()\n");
	
	if (video->stream == NULL) {
		// no video to render
		return;
	}
	
	cairo_set_source_surface (cr, video->surface, 0, 0);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
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
	//printf ("MediaPlayer::Play (), paused = %s, opened = %s, playing = %s\n", paused ? "true" : "false", opened ? "true" : "false", playing ? "true" : "false");
	if (!paused || !opened)
		return 0;
	
	if (!playing) {
		// Start up the decoder/audio threads
		media_player_enqueue_frames (this, 1, 1);
		
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
	
	paused = false;
	pthread_cond_signal (&pause_cond);
	pthread_mutex_unlock (&pause_mutex);
	
	start_time = TimeManager::Instance()->GetCurrentTimeUsec();
	start_pts = current_pts;
	
	if (HasVideo ()) {
		//printf ("MediaPlayer::Play (), timeout: %i\n", video->stream->msec_per_frame);
		// TODO: Calculate correct framerate (in the pipeline)
		return TimeManager::Instance()->AddTimeout (MAX (video->stream->msec_per_frame, 1000 / 60), callback, user_data);
	} else {
		//printf ("MediaPlayer::Play (), timeout: 33 (no video)\n");
		return TimeManager::Instance()->AddTimeout (33, callback, user_data);
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
MediaPlayer::Pause ()
{
	//printf ("MediaPlayer::Pause (), paused = %s, opened = %s, playing = %s\n", paused ? "true" : "false", opened ? "true" : "false", playing ? "true" : "false");
	
	if (paused || !CanPause ())
		return;
	
	paused = true;
	if (audio_thread != NULL)
		pthread_cond_wait (&pause_cond, &pause_mutex);
	else
		pthread_mutex_lock (&pause_mutex);
}

void
MediaPlayer::StopThreads ()
{
	uint64_t initial_pts;
	
	stop = true;
	
	if (paused) {
		paused = false;
		pthread_cond_signal (&pause_cond);
		pthread_mutex_unlock (&pause_mutex);
	}
		
	if (audio_thread != NULL) {
		g_thread_join (audio_thread);
		audio_thread = NULL;
	}
	
	audio->queue->Clear (true);
	video->queue->Clear (true);
	
	// enter paused state
	pthread_mutex_lock (&pause_mutex);
	paused = true;
	
	audio->outptr = audio->outbuf;
	
	if (audio->pcm != NULL && HasAudio ())
		initial_pts = audio->stream->start_time;
	else if (HasVideo ())
		initial_pts = video->stream->start_time;
	else
		initial_pts = 0;
	
	current_pts = initial_pts;
	target_pts = initial_pts;
	
	stop = false;
	eof = false;
}

void
MediaPlayer::Stop ()
{
	StopThreads ();
	
	playing = false;
	
	if (media != NULL) {
		media->ClearQueue ();
		media->SeekAsync (0);
	}
}

bool
MediaPlayer::CanSeek ()
{
	// FIXME: should return false if it is streaming media
	return true;
}

void
MediaPlayer::Seek (uint64_t position)
{
	uint64_t initial_pts, duration;
	bool resume = !paused;
	
	if (!CanSeek ())
		return;
	
	if (audio->pcm != NULL && HasAudio ()) {
		duration = audio->stream->duration;
		initial_pts = audio->stream->start_time;
	} else if (HasVideo ()) {
		duration = video->stream->duration;
		initial_pts = video->stream->start_time;
	} else {
		initial_pts = 0;
		duration = 0;
	}
	
	duration += initial_pts;
	position += initial_pts;
	
	if (position > duration)
		position = duration;
	else if (position < initial_pts)
		position = initial_pts;
	
	StopThreads ();
	if (media != NULL) {
		media->ClearQueue ();
		media->SeekAsync (position);
	}
	
	current_pts = position;
	target_pts = position;
	start_pts = position;
	
	if (HasVideo ()) {
		//TODO: LoadVideoFrame ();
	}
	
	//printf ("MediaPlayer::Seek (%llu) (playing: %s)\n", position, playing ? "true" : "false");
	
	if (playing) {
		media_player_enqueue_frames (this, 1, 1);
		 
		// Restart the audio/io threads
		if (audio->pcm != NULL && HasAudio ()) {
			pthread_mutex_lock (&audio->init_mutex);
			audio_thread = g_thread_create (audio_loop, this, true, NULL);
			pthread_cond_wait (&audio->init_cond, &audio->init_mutex);
			pthread_mutex_unlock (&audio->init_mutex);
		}
		
		if (resume) {
			start_time = TimeManager::Instance()->GetCurrentTimeUsec();
			
			// Resume playback
			paused = false;
			pthread_cond_signal (&pause_cond);
			pthread_mutex_unlock (&pause_mutex);
			
		}
	}
}

uint64_t
MediaPlayer::Position ()
{
	uint64_t position = target_pts;
	
	if (audio->pcm != NULL && HasAudio ())
		return position - audio->stream->start_time;
	
	if (HasVideo ())
		return position - video->stream->start_time;
	
	return position;
}

uint64_t
MediaPlayer::Duration ()
{
	if (audio->pcm != NULL && HasAudio ())
		return audio->stream->duration;
	
	if (HasVideo ())
		return video->stream->duration;
	
	return 0;
}

void
MediaPlayer::Mute ()
{
	audio->muted = true;
}

void
MediaPlayer::UnMute ()
{
	audio->muted = false;
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
		//printf ("audio: volume: %f, balance: %f\n", audio->volume, audio->balance);
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
	
	pthread_mutex_lock (&mplayer->pause_mutex);
	
	while (!mplayer->stop) {
		if (mplayer->paused) {
			printf ("audio_loop (): paused.\n");
			// allow main thread to take pause lock
			pthread_cond_signal (&mplayer->pause_cond);
			pthread_mutex_unlock (&mplayer->pause_mutex);
			
			// wait for main thread to relinquish pause lock
			pthread_cond_wait (&mplayer->pause_cond, &mplayer->pause_mutex);
			printf ("audio_loop (): resumed.\n");
			continue;
		}
		
		if ((frame_pts = audio_play (audio, ufds, ndfs)) > 0) {
			// calculated pts
			pthread_mutex_lock (&mplayer->target_pts_lock);
			mplayer->target_pts += frame_pts;
			pthread_mutex_unlock (&mplayer->target_pts_lock);
		} else {
			if (!pkt && (pkt = (Packet *) audio->queue->Pop ())) {
				// decode an audio packet
				stream = pkt->frame->stream;
				frame = pkt->frame;
				
				if (frame->event == FrameEventEOF)
					break;
				
				if (!frame->IsDecoded ())
					stream->decoder->DecodeFrame (frame);
				
				pthread_mutex_lock (&mplayer->target_pts_lock);
				mplayer->target_pts = frame->pts;
				pthread_mutex_unlock (&mplayer->target_pts_lock);
				//printf ("setting target_pts to %llu\n", mplayer->target_pts);
				
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
	
	pthread_mutex_unlock (&mplayer->pause_mutex);
	
	g_free (ufds);
	
	return NULL;
}
