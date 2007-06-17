/*
 * mplayer.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
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
#include <avformat.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include "mplayer.h"


#define ALIGN(addr,size) (uint8_t *) (((uint32_t) (((uint8_t *) (addr)) + (size) - 1)) & ~((size) - 1))

#define AUDIO_BUFLEN (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)

struct Packet {
	int stream_id;
	int duration;
	int64_t pts;
	size_t size;
	uint8_t data[1];
};

struct Audio {
	GAsyncQueue *queue;
	
	// Packet left from previous decode loop
	Packet *pkt;
	uint32_t inleft;
	uint8_t *inptr;
	
	double volume;
	bool muted;
	
	// input
	AVStream *stream;
	AVCodec *codec;
	int stream_id;
	
	// buffering
	uint8_t buffer[AUDIO_BUFLEN + 1];
	uint8_t *outbuf;
	uint8_t *outptr;
	
	// output
	snd_pcm_t *pcm;
	snd_pcm_uframes_t sample_size;
	
	// sync
	uint64_t initial_pts;
	int usec_per_frame;
	int usec_to_pts;
};

struct Video {
	GAsyncQueue *queue;
	
	// input
	AVStream *stream;
	AVCodec *codec;
	int stream_id;
	
	// rendering
	struct SwsContext *scaler;
	cairo_surface_t *surface;
	uint8_t *rgb_buffer;
	
	// sync
	uint64_t initial_pts;
	int usec_per_frame;
	int usec_to_pts;
};


static bool ffmpeg_initialized = false;

static void ffmpeg_init (void);

// packets in a/v queues
static Packet *pkt_new (AVPacket *avpkt);
#define pkt_free(x) g_free (x)

// threads
static void *audio_loop (void *data);
static void *io_loop (void *data);


MediaPlayer::MediaPlayer ()
{
	ffmpeg_init ();
	
	uri = NULL;
	
	g_static_mutex_init (&pause_mutex);
	g_static_mutex_lock (&pause_mutex);
	paused = true;
	stop = false;
	
	av_ctx = NULL;
	
	audio_thread = NULL;
	io_thread = NULL;
	
	audio = new Audio ();
	audio->queue = g_async_queue_new ();
	audio->pkt = NULL;
	audio->inleft = 0;
	audio->inptr = NULL;
	audio->volume = 0.0f;
	audio->muted = false;
	audio->stream_id = -1;
	audio->stream = NULL;
	audio->codec = NULL;
	memset (audio->buffer, 0, AUDIO_BUFLEN + 1);
	audio->outbuf = ALIGN (audio->buffer, 2);
	audio->outptr = audio->outbuf;
	audio->pcm = NULL;
	audio->sample_size = 0;
	audio->initial_pts = 0;
	audio->usec_per_frame = 0;
	audio->usec_to_pts = 0;
	
	video = new Video ();
	video->queue = g_async_queue_new ();
	video->stream_id = -1;
	video->stream = NULL;
	video->codec = NULL;
	video->scaler = NULL;
	video->surface = NULL;
	video->rgb_buffer = NULL;
	video->initial_pts = 0;
	video->usec_per_frame = 0;
	video->usec_to_pts = 0;
	
	pause_time = 0;
	start_time = 0;
	target_pts = 0;
	
	height = 0;
	width = 0;
}

MediaPlayer::~MediaPlayer ()
{
	Close ();
	
	g_static_mutex_unlock (&pause_mutex);
	
	if (audio->pcm != NULL)
		snd_pcm_close (audio->pcm);
	
	g_async_queue_unref (audio->queue);
	g_async_queue_unref (video->queue);
	
	delete audio;
	delete video;
}

bool
MediaPlayer::Open ()
{
	AVCodecContext *encoding;
	AVStream *stream;
	
	if (uri == NULL || *uri == '\0')
		return false;
	
	if (av_open_input_file (&av_ctx, uri, NULL, 0, NULL) < 0) {
		fprintf (stderr, "cannot open uri `%s': %s\n", uri, strerror (errno));
		av_ctx = NULL;
		return false;
	}
	
	if (av_find_stream_info (av_ctx) < 0) {
		fprintf (stderr, "cannot open uri `%s': no stream info\n", uri);
		av_close_input_file (av_ctx);
		av_ctx = NULL;
		return false;
	}
	
	opened = true;
	stop = false;
	
	av_read_play (av_ctx);
	
	// Find audio/video streams
	for (int i = 0; i < av_ctx->nb_streams; i++) {
		stream = av_ctx->streams[i];
		encoding = stream->codec;
		
		switch (encoding->codec_type) {
		case CODEC_TYPE_AUDIO:
			if (audio->stream_id != -1)
				break;
			
			if (!(audio->codec = avcodec_find_decoder (encoding->codec_id)))
				break;
			
			avcodec_open (stream->codec, audio->codec);
			audio->stream = stream;
			audio->stream_id = i;
			
			// starting time
			audio->initial_pts = stream->start_time;
			
			// usec per frame
			audio->usec_per_frame = (int) (1000 / av_q2d (stream->r_frame_rate));
			
			// usec -> pts conversion
			audio->usec_to_pts = (int) av_q2d (encoding->time_base);
			break;
		case CODEC_TYPE_VIDEO:
			if (video->stream_id != -1)
				break;
			
			if (!(video->codec = avcodec_find_decoder (encoding->codec_id)))
				break;
			
			avcodec_open (stream->codec, video->codec);
			video->stream = stream;
			video->stream_id = i;
			
			height = encoding->height;
			width = encoding->width;
			
			// for conversion to rgb32 format needed for rendering
			video->rgb_buffer = (uint8_t *) g_malloc0 (width * height * 4);
			video->scaler = sws_getContext (width, height, encoding->pix_fmt,
							width, height, PIX_FMT_RGB32,
							SWS_BICUBIC, NULL, NULL, NULL);
			
			// rendering surface
			video->surface = cairo_image_surface_create_for_data (
				video->rgb_buffer, CAIRO_FORMAT_ARGB32,
				width, height, width * 4);
			
			// starting time
			video->initial_pts = stream->start_time;
			
			// usec per frame
			video->usec_per_frame = (int) (1000 / av_q2d (stream->r_frame_rate));
			
			// usec -> pts conversion
			video->usec_to_pts = (int) av_q2d (encoding->time_base);
			break;
		default:
			break;
		}
	}
	
	// Prepare audio playback
	if (audio->pcm == NULL && audio->stream_id != -1) {
		if (snd_pcm_open (&audio->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0) != 0) {
			fprintf (stderr, "cannot open audio device: %s\n", strerror (errno));
			audio->pcm = NULL;
		}
	}
	
	if (audio->pcm != NULL && audio->stream_id != -1) {
		snd_pcm_uframes_t buf_size;
		
		encoding = audio->stream->codec;
		
		snd_pcm_set_params (audio->pcm, SND_PCM_FORMAT_S16,
				    SND_PCM_ACCESS_RW_INTERLEAVED,
				    encoding->channels,
				    encoding->sample_rate,
				    1, 0);
		
		snd_pcm_get_params (audio->pcm, &buf_size, &audio->sample_size);
		
		audio_thread = g_thread_create (audio_loop, this, true, NULL);
	}
	
	io_thread = g_thread_create (io_loop, this, true, NULL);
	
	return true;
}

bool
MediaPlayer::Open (const char *uri)
{
	Close ();
	
	g_free (this->uri);
	this->uri = g_strdup (uri);
	
	return Open ();
}

void
MediaPlayer::Close ()
{
	Packet *pkt;
	
	stop = true;
	
	if (paused) {
		g_static_mutex_unlock (&pause_mutex);
		paused = false;
	}
	
	if (io_thread != NULL) {
		g_thread_join (io_thread);
		io_thread = NULL;
	}
	
	if (audio_thread != NULL) {
		g_thread_join (audio_thread);
		audio_thread = NULL;
	}
	
	// de-queue audio/video packets
	if (audio->pkt != NULL) {
		pkt_free (audio->pkt);
		audio->inptr = NULL;
		audio->inleft = 0;
		audio->pkt = NULL;
		audio->inleft = 0;
	}
	
	while ((pkt = (Packet *) g_async_queue_try_pop (audio->queue)))
		pkt_free (pkt);
	
	while ((pkt = (Packet *) g_async_queue_try_pop (video->queue)))
		pkt_free (pkt);
	
	if (video->rgb_buffer != NULL) {
		g_free (video->rgb_buffer);
		video->rgb_buffer = NULL;
	}
	
	if (video->surface != NULL) {
		cairo_surface_destroy (video->surface);
		video->surface = NULL;
	}
	
	// FIXME: free audio->stream and video->stream?
	if (av_ctx != NULL)
		av_close_input_file (av_ctx);
	av_ctx = NULL;
	
	opened = false;
	
	audio->stream_id = -1;
	audio->stream = NULL;
	audio->codec = NULL;
	audio->sample_size = 0;
	audio->initial_pts = 0;
	audio->usec_per_frame = 0;
	audio->usec_to_pts = 0;
	
	video->stream_id = -1;
	video->stream = NULL;
	video->codec = NULL;
	video->initial_pts = 0;
	video->usec_per_frame = 0;
	video->usec_to_pts = 0;
	
	pause_time = 0;
	start_time = 0;
	target_pts = 0;
	
	// enter paused state
	g_static_mutex_lock (&pause_mutex);
	paused = true;
	
	g_free (uri);
	uri = NULL;
}

//
// Convert the AVframe into an RGB buffer we can render
//
static void
convert_to_rgb (Video *video, AVFrame *frame)
{
	AVCodecContext *cc = video->stream->codec;
	uint8_t *rgb_dest[3] = { video->rgb_buffer, NULL, NULL};
	int rgb_stride [3] = { cc->width * 4, 0, 0 };
	
	if (frame->data == NULL)
		return;
	
	sws_scale (video->scaler, frame->data, frame->linesize, 0,
		   cc->height, rgb_dest, rgb_stride);
}

bool
MediaPlayer::AdvanceFrame ()
{
	bool advanced = false;
	uint64_t target_pts;
	AVFrame *frame;
	int redraw = 0;
	Packet *pkt;
	int64_t pts;
	
	if (paused) {
		// shouldn't happen, but just in case
		printf ("WARNING: MediaPlayer::AdvanceFrame() called when paused\n");
		return false;
	}
	
	if (!audio_thread) {
		// no audio to sync to
		uint64_t now = av_gettime ();
		
		uint64_t elapsed_usec = now - start_time;
		uint64_t elapsed_pts = elapsed_usec * video->usec_to_pts;
		
		target_pts = video->initial_pts + elapsed_pts;
	} else {
		// use target_pts as set by audio thread
		target_pts = this->target_pts;
	}
	
	while ((pkt = (Packet *) g_async_queue_try_pop (video->queue))) {
		// always decode the frame or we get glitches in the screen
		frame = avcodec_alloc_frame ();
		avcodec_decode_video (video->stream->codec, frame,
				      &redraw, pkt->data, pkt->size);
		
		pts = pkt->pts;
		pkt_free (pkt);
		
		if (pts + video->usec_per_frame >= target_pts) {
			// we are in sync (or ahead) of audio playback
			break;
		}
		
		// we are lagging behind, drop this frame
		av_free (frame);
		frame = NULL;
	}
	
	if (redraw) {
		convert_to_rgb (video, frame);
		advanced = true;
	}
	
	if (frame != NULL)
		av_free (frame);
	
	return advanced;
}

void
MediaPlayer::Render (cairo_t *cr)
{
	if (video->stream_id == -1) {
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
	if (paused || stop)
		return false;
	
	return (audio->outptr > audio->outbuf ||
		g_async_queue_length (audio->queue) > 0 ||
		g_async_queue_length (video->queue) > 0);
}

guint
MediaPlayer::Play (GSourceFunc callback, void *user_data)
{
	if (!paused || !opened)
		return 0;
	
	g_static_mutex_unlock (&pause_mutex);
	paused = false;
	
	start_time += (av_gettime () - pause_time);
	
	if (video->stream_id != -1)
		return g_timeout_add (video->usec_per_frame, callback, user_data);
	
	return 0;
}

void
MediaPlayer::Pause ()
{
	if (paused)
		return;
	
	g_static_mutex_lock (&pause_mutex);
	paused = true;
	
	pause_time = av_gettime ();
}

void
MediaPlayer::Stop ()
{
	Close ();
	Open ();
}

bool
MediaPlayer::CanSeek ()
{
	// FIXME: implement seeking
	return false;
}

void
MediaPlayer::Seek (double position)
{
	// FIXME: implement me
}

double
MediaPlayer::Position ()
{
	// FIXME: implement me
	return 0.0f;
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

double
MediaPlayer::GetVolume ()
{
	// FIXME: implement
	return audio->volume;
}

void
MediaPlayer::SetVolume (double volume)
{
	// FIXME: implement
	audio->volume = volume;
}


static void
ffmpeg_init (void)
{
	if (ffmpeg_initialized)
		return;
	
	avcodec_init ();
	av_register_all ();
	avcodec_register_all ();
	ffmpeg_initialized = true;
}

static Packet *
pkt_new (AVPacket *av_pkt)
{
	Packet *pkt;
	
	pkt = (Packet *) g_malloc (sizeof (Packet) + av_pkt->size);
	pkt->stream_id = av_pkt->stream_index;
	pkt->duration = av_pkt->duration;
	pkt->size = av_pkt->size;
	pkt->pts = av_pkt->pts;
	
	memcpy (pkt->data, av_pkt->data, av_pkt->size);
	
	return pkt;
}


// Audio playback thread

// Returns true if finished decoding, false otherwise
static bool
audio_decode (Audio *audio)
{
	AVCodecContext *codec = audio->stream->codec;
	uint32_t frame_size = audio->sample_size * codec->channels * 2;
	uint8_t *outbuf;
	int outlen = 0;
	int n;
	
	/* Note: an audio packet can contain multiple audio frames */
	
	while (audio->inleft > 0) {
		/* avcodec_decode_audio2() requires that the outbuf be 16bit word aligned */
		outbuf = ALIGN (audio->outptr, 2);
		outlen = (audio->outbuf + AUDIO_BUFLEN) - outbuf;
		
		if (outlen < frame_size)
			break;
		
		n = avcodec_decode_audio2 (codec, (int16_t *) outbuf, &outlen,
					   audio->inptr, audio->inleft);
		
		if (n > 0) {
			audio->inleft -= n;
			audio->inptr += n;
			
			// append the newly decoded buffer to the end of the
			// remaining audio buffer from our previous loop
			// (if re-alignment was required).
			if (outbuf > audio->outptr)
				memmove (audio->outptr, outbuf, outlen);
			audio->outptr += outlen;
		} else if (n == 0) {
			/* complete or out of room in outbuf */
			break;
		} else {
			printf ("audio_decode error: %d\n", n);
			// pad frame with silence
			if ((n = (audio->outptr - audio->outbuf) % frame_size) > 0) {
				memset (audio->outptr, 0, n);
				audio->outptr += n;
			}
			
			return true;
		}
	}
	
	return audio->inleft == 0;
}

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

// Returns true if a frame of audio was played or false if insufficient data
static bool
audio_play (Audio *audio, struct pollfd *ufds, int nfds)
{
	int frame_size, samples, outlen, channels, n;
	uint8_t *outptr;
	
	channels = audio->stream->codec->channels;
	frame_size = audio->sample_size * channels * 2;
	outlen = audio->outptr - audio->outbuf;
	samples = audio->sample_size;
	outptr = audio->outbuf;
	
	// make sure we have enough data to play a frame of audio
	if (outlen < frame_size)
		return false;
	
	if (audio->muted) {
		// FIXME: would it be better to play silence?
		// sleep for a frame...
		g_usleep (audio->usec_per_frame);
		outptr += frame_size;
		goto shift;
	}
	
	// FIXME: set volume?
	
	// play only 1 frame
	while (samples > 0) {
		if (pcm_poll (audio->pcm, ufds, nfds) != 0) {
			switch (snd_pcm_state (audio->pcm)) {
			case SND_PCM_STATE_XRUN:
				//printf ("SND_PCM_STATE_XRUN\n");
				snd_pcm_prepare (audio->pcm);
				break;
			case SND_PCM_STATE_SUSPENDED:
				//printf ("SND_PCM_STATE_SUSPENDED\n");
				while ((n = snd_pcm_resume (audio->pcm)) == -EAGAIN)
					sleep (1);
				if (n < 0)
					snd_pcm_prepare (audio->pcm);
				break;
			default:
				break;
			}
		}
		
		n = snd_pcm_writei (audio->pcm, outptr, samples);
		if (n > 0) {
			outptr += (n * 2 * channels);
			samples -= n;
		} else if (n == -ESTRPIPE) {
			//printf ("snd_pcm_writei() returned -ESTRPIPE\n");
			while ((n = snd_pcm_resume (audio->pcm)) == -EAGAIN)
				sleep (1);
			
			if (n < 0) {
				snd_pcm_prepare (audio->pcm);
				snd_pcm_start (audio->pcm);
			}
		} else if (n == -EPIPE) {
			//printf ("snd_pcm_writei() returned -EPIPE\n");
			snd_pcm_prepare (audio->pcm);
			snd_pcm_start (audio->pcm);
		}
	}
	
shift:
	
	if (outptr < audio->outptr) {
		// make room for more audio to be buffered
		outlen = audio->outptr - outptr;
		memmove (audio->outbuf, outptr, outlen);
		audio->outptr = audio->outbuf + outlen;
	} else {
		// no more buffered audio data
		audio->outptr = audio->outbuf;
	}
	
	return true;
}

static void *
audio_loop (void *data)
{
	MediaPlayer *mplayer = (MediaPlayer *) data;
	Audio *audio = mplayer->audio;
	//uint64_t pkt_pts, pts;
	//bool have_pts = false;
	struct pollfd *ufds;
	uint64_t frame_pts;
	Packet *pkt;
	int n;
	
	if ((n = snd_pcm_poll_descriptors_count (audio->pcm)) > 0) {
		ufds = (struct pollfd *) g_malloc (sizeof (struct pollfd) * n);
		
		if (snd_pcm_poll_descriptors (audio->pcm, ufds, n) < 0) {
			g_free (ufds);
			ufds = NULL;
		}
	}
	
	frame_pts = audio->usec_per_frame * audio->usec_to_pts;
	
	mplayer->target_pts = audio->initial_pts;
	
	while (!mplayer->stop) {
		g_static_mutex_lock (&mplayer->pause_mutex);
		
		if (audio_play (audio, ufds, n)) {
			// calculated pts
			mplayer->target_pts += frame_pts;
		} else {
			// decode an audio packet
			if (!audio->pkt && (pkt = (Packet *) g_async_queue_try_pop (audio->queue))) {
				audio->inleft = pkt->size;
				audio->inptr = pkt->data;
				audio->pkt = pkt;
				
				mplayer->target_pts = pkt->pts;
			}
			
			if (audio->pkt && audio_decode (audio)) {
				pkt_free (audio->pkt);
				audio->pkt = NULL;
			}
		}
		
		g_static_mutex_unlock (&mplayer->pause_mutex);
	}
	
	g_free (ufds);
	
	return NULL;
}


// I/O thread (queues packets for audio/video)

static void *
io_loop (void *data)
{
	MediaPlayer *mplayer = (MediaPlayer *) data;
	GAsyncQueue *audio_q, *video_q;
	int audio_id, video_id;
	AVPacket pkt;
	
	audio_id = mplayer->audio->stream_id;
	video_id = mplayer->video->stream_id;
	
	audio_q = mplayer->audio_thread ? mplayer->audio->queue : NULL;
	video_q = video_id != -1 ? mplayer->video->queue : NULL;
	
	while (!mplayer->stop) {
		if ((!audio_q || g_async_queue_length (audio_q) > 100) &&
		    (!video_q || g_async_queue_length (video_q) > 100)) {
			// throttle ourselves
			g_usleep (1000);
			continue;
		}
		
		if (av_read_frame (mplayer->av_ctx, &pkt) < 0) {
			// stream is complete (or error) - nothing left to decode.
			break;
		}
		
		if (pkt.stream_index == audio_id) {
			if (audio_q)
				g_async_queue_push (audio_q, pkt_new (&pkt));
		} else if (pkt.stream_index == video_id) {
			if (video_q)
				g_async_queue_push (video_q, pkt_new (&pkt));
		} else {
			// unhandled stream
		}
		
		av_free_packet (&pkt);
	}
	
	return NULL;
}
