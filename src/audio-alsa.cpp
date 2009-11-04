/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-alsa.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <dlfcn.h>

#include "audio-alsa.h"
#include "runtime.h"
#include "clock.h"
#include "debug.h"

typedef int               (dyn_snd_pcm_open)                           (snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode);
typedef int               (dyn_snd_pcm_close)                          (snd_pcm_t *pcm);
typedef int               (dyn_snd_pcm_get_params)                     (snd_pcm_t *pcm, snd_pcm_uframes_t *buffer_size, snd_pcm_uframes_t *period_size);
typedef int               (dyn_snd_pcm_poll_descriptors_count)         (snd_pcm_t *pcm);
typedef int               (dyn_snd_pcm_poll_descriptors)               (snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space);
typedef int               (dyn_snd_output_stdio_attach)                (snd_output_t **outputp, FILE *fp, int _close);
typedef int               (dyn_snd_pcm_hw_params_malloc)               (snd_pcm_hw_params_t **ptr);
typedef int               (dyn_snd_pcm_hw_params_any)                  (snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
typedef int               (dyn_snd_pcm_hw_params_dump)                 (snd_pcm_hw_params_t *params, snd_output_t *out);
typedef int               (dyn_snd_pcm_hw_params_set_rate_resample)    (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);
typedef int               (dyn_snd_pcm_hw_params_test_access)          (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access);
typedef int               (dyn_snd_pcm_hw_params_set_access)           (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access);
typedef int               (dyn_snd_pcm_hw_params_set_format)           (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val);
typedef int               (dyn_snd_pcm_hw_params_set_channels)         (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);
typedef int               (dyn_snd_pcm_hw_params_set_rate_near)        (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
typedef int               (dyn_snd_pcm_hw_params_set_buffer_time_near) (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
typedef int               (dyn_snd_pcm_hw_params)                      (snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
typedef int               (dyn_snd_pcm_hw_params_can_pause)            (const snd_pcm_hw_params_t *params);
typedef void              (dyn_snd_pcm_hw_params_free)                 (snd_pcm_hw_params_t *obj);
typedef snd_pcm_state_t   (dyn_snd_pcm_state)                          (snd_pcm_t *pcm);
typedef const char *      (dyn_snd_pcm_state_name)                     (const snd_pcm_state_t state);
typedef int               (dyn_snd_pcm_drop)                           (snd_pcm_t *pcm);
typedef snd_pcm_sframes_t (dyn_snd_pcm_writei)                         (snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);
typedef int               (dyn_snd_pcm_mmap_begin)                     (snd_pcm_t *pcm, const snd_pcm_channel_area_t **areas, snd_pcm_uframes_t *offset, snd_pcm_uframes_t *frames);
typedef snd_pcm_sframes_t (dyn_snd_pcm_mmap_commit)                    (snd_pcm_t *pcm, snd_pcm_uframes_t offset, snd_pcm_uframes_t frames);
typedef int               (dyn_snd_pcm_prepare)                        (snd_pcm_t *pcm);
typedef int               (dyn_snd_pcm_resume)                         (snd_pcm_t *pcm);
typedef snd_pcm_sframes_t (dyn_snd_pcm_avail_update)                   (snd_pcm_t *pcm);
typedef int               (dyn_snd_pcm_start)                          (snd_pcm_t *pcm);
typedef int               (dyn_snd_pcm_delay)                          (snd_pcm_t *pcm, snd_pcm_sframes_t *delayp);
typedef const char *      (dyn_snd_strerror)                           (int errnum);
typedef const char *      (dyn_snd_asoundlib_version)                  (void);

dyn_snd_pcm_open *                           d_snd_pcm_open = NULL;
dyn_snd_pcm_close *                          d_snd_pcm_close = NULL;
dyn_snd_pcm_get_params *                     d_snd_pcm_get_params = NULL;
dyn_snd_pcm_poll_descriptors_count *         d_snd_pcm_poll_descriptors_count = NULL;
dyn_snd_pcm_poll_descriptors *               d_snd_pcm_poll_descriptors = NULL;
dyn_snd_output_stdio_attach *                d_snd_output_stdio_attach = NULL;
dyn_snd_strerror *                           d_snd_strerror = NULL;
dyn_snd_pcm_hw_params_malloc *               d_snd_pcm_hw_params_malloc = NULL;
dyn_snd_pcm_hw_params_any *                  d_snd_pcm_hw_params_any = NULL;
dyn_snd_pcm_hw_params_dump *                 d_snd_pcm_hw_params_dump = NULL;
dyn_snd_pcm_hw_params_set_rate_resample *    d_snd_pcm_hw_params_set_rate_resample = NULL;
dyn_snd_pcm_hw_params_test_access *          d_snd_pcm_hw_params_test_access = NULL;
dyn_snd_pcm_hw_params_set_access *           d_snd_pcm_hw_params_set_access = NULL;
dyn_snd_pcm_hw_params_set_format *           d_snd_pcm_hw_params_set_format = NULL;
dyn_snd_pcm_hw_params_set_channels *         d_snd_pcm_hw_params_set_channels = NULL;
dyn_snd_pcm_hw_params_set_rate_near *        d_snd_pcm_hw_params_set_rate_near = NULL;
dyn_snd_pcm_hw_params_set_buffer_time_near * d_snd_pcm_hw_params_set_buffer_time_near = NULL;
dyn_snd_pcm_hw_params *                      d_snd_pcm_hw_params = NULL;
dyn_snd_pcm_hw_params_can_pause *            d_snd_pcm_hw_params_can_pause = NULL;
dyn_snd_pcm_hw_params_free *                 d_snd_pcm_hw_params_free = NULL;
dyn_snd_pcm_state *                          d_snd_pcm_state = NULL;
dyn_snd_pcm_state_name *                     d_snd_pcm_state_name = NULL;
dyn_snd_pcm_drop *                           d_snd_pcm_drop = NULL;
dyn_snd_pcm_writei *                         d_snd_pcm_writei = NULL;
dyn_snd_pcm_mmap_begin *                     d_snd_pcm_mmap_begin = NULL;
dyn_snd_pcm_mmap_commit *                    d_snd_pcm_mmap_commit = NULL;
dyn_snd_pcm_prepare *                        d_snd_pcm_prepare = NULL;
dyn_snd_pcm_resume *                         d_snd_pcm_resume = NULL;
dyn_snd_pcm_avail_update *                   d_snd_pcm_avail_update = NULL;
dyn_snd_pcm_start *                          d_snd_pcm_start = NULL;
dyn_snd_pcm_delay *                          d_snd_pcm_delay = NULL;
dyn_snd_asoundlib_version *                  d_snd_asoundlib_version = NULL;

#define snd_pcm_open                           d_snd_pcm_open
#define snd_pcm_close                          d_snd_pcm_close
#define snd_pcm_get_params                     d_snd_pcm_get_params
#define snd_pcm_poll_descriptors_count         d_snd_pcm_poll_descriptors_count
#define snd_pcm_poll_descriptors               d_snd_pcm_poll_descriptors
#define snd_output_stdio_attach                d_snd_output_stdio_attach
#define snd_strerror                           d_snd_strerror
#define snd_pcm_hw_params_malloc               d_snd_pcm_hw_params_malloc
#define snd_pcm_hw_params_any                  d_snd_pcm_hw_params_any
#define snd_pcm_hw_params_dump                 d_snd_pcm_hw_params_dump
#define snd_pcm_hw_params_set_rate_resample    d_snd_pcm_hw_params_set_rate_resample
#define snd_pcm_hw_params_test_access          d_snd_pcm_hw_params_test_access
#define snd_pcm_hw_params_test_access          d_snd_pcm_hw_params_test_access
#define snd_pcm_hw_params_set_access           d_snd_pcm_hw_params_set_access
#define snd_pcm_hw_params_set_format           d_snd_pcm_hw_params_set_format
#define snd_pcm_hw_params_set_channels         d_snd_pcm_hw_params_set_channels
#define snd_pcm_hw_params_set_rate_near        d_snd_pcm_hw_params_set_rate_near
#define snd_pcm_hw_params_set_buffer_time_near d_snd_pcm_hw_params_set_buffer_time_near
#define snd_pcm_hw_params                      d_snd_pcm_hw_params
#define snd_pcm_hw_params_can_pause            d_snd_pcm_hw_params_can_pause
#define snd_pcm_hw_params_free                 d_snd_pcm_hw_params_free
#define snd_pcm_state                          d_snd_pcm_state
#define snd_pcm_state_name                     d_snd_pcm_state_name
#define snd_pcm_drop                           d_snd_pcm_drop
#define snd_pcm_writei                         d_snd_pcm_writei
#define snd_pcm_mmap_begin                     d_snd_pcm_mmap_begin
#define snd_pcm_mmap_commit                    d_snd_pcm_mmap_commit
#define snd_pcm_prepare                        d_snd_pcm_prepare
#define snd_pcm_resume                         d_snd_pcm_resume
#define snd_pcm_avail_update                   d_snd_pcm_avail_update
#define snd_pcm_start                          d_snd_pcm_start
#define snd_pcm_delay                          d_snd_pcm_delay
#define snd_asoundlib_version                  d_snd_asoundlib_version

/*
 * AlsaSource
 */
 
AlsaSource::AlsaSource (AlsaPlayer *player, MediaPlayer *mplayer, AudioStream *stream)
	: AudioSource (player, mplayer, stream), mutex (true)
{
	LOG_ALSA ("AlsaSource::AlsaSource (%p, %p)\n", player, stream);
	
	this->player = player;
	
	pcm = NULL;
	period_size = 0;
	buffer_size = 0;
	
	mmap = false;
	udfs = NULL;
	ndfs = 0;
	
	started = false;
	drop_pending = false;
	initialized = false;
}

AlsaSource::~AlsaSource ()
{
	LOG_ALSA ("AlsaSource::~AlsaSource ()\n");

	CloseAlsa ();
}

bool
AlsaSource::InitializeInternal ()
{
	LOG_AUDIO ("AlsaSource::InitializeInternal ()\n");
	// this is a no-op, initialization is done when needed.
	return true;
}

bool
AlsaSource::InitializeAlsa ()
{
	bool res = false;
	int result;
	AudioStream *stream = NULL;
	
	LOG_AUDIO ("AlsaSource::InitializeAlsa (%p) initialized: %i\n", this, initialized);
	
	mutex.Lock ();
	
	if (initialized) {
		result = true;
		goto cleanup;
	}
	
	stream = GetStreamReffed ();
		
	if (stream == NULL) {
		// Shouldn't really happen, but handle this case anyway.
		LOG_AUDIO ("AlsaSource::Initialize (): trying to initialize an audio device, but there's no audio to play.\n");
		goto cleanup;
	}
		
	// Open a pcm device
	result = snd_pcm_open (&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0 /*SND_PCM_NONBLOCK*/);
	if (result != 0) {
		LOG_AUDIO ("AlsaSource::Initialize (): cannot open audio device: %s\n", snd_strerror (result));
		pcm = NULL;
		goto cleanup;
	}

	// Configure the hardware
	if (!SetupHW ()) {
		LOG_AUDIO ("AlsaSource::Initialize (): could not configure hardware for audio playback\n");
		Close ();
		goto cleanup;
	}
	
	result = snd_pcm_get_params (pcm, &buffer_size, &period_size);
	if (result != 0) {
		LOG_AUDIO ("AlsaSource::Initialize (): error while getting parameters: %s\n", snd_strerror (result));
		Close ();
		goto cleanup;
	}

	// Get the file descriptors to poll on
	ndfs = snd_pcm_poll_descriptors_count (pcm);
	if (ndfs <= 0) {
		LOG_AUDIO ("AlsaSource::Initialize(): Unable to initialize audio for playback (could not get poll descriptor count).\n");
		Close ();
		goto cleanup;
	}

	udfs = (pollfd *) g_malloc0 (sizeof (pollfd) * ndfs);
	if (snd_pcm_poll_descriptors (pcm, udfs, ndfs) < 0) {
		LOG_AUDIO ("AlsaSource::Initialize (): Unable to initialize audio for playback (could not get poll descriptors).\n");
		Close ();
		goto cleanup;
	}
	
	LOG_AUDIO ("AlsaSource::Initialize (%p): Succeeded. Buffer size: %lu, period size: %lu\n", this, buffer_size, period_size);

	res = true;
	initialized = true;
	
cleanup:

	mutex.Unlock ();

	if (stream)
		stream->unref ();

	return res;
}

bool
AlsaSource::SetupHW ()
{
	bool result = false;
	bool rw_available = false;
	bool mmap_available = false;
#if DEBUG
	bool debug = debug_flags & RUNTIME_DEBUG_AUDIO;
#else
	bool debug = false;
#endif
	
	snd_pcm_hw_params_t *params = NULL;
	snd_output_t *output = NULL;
	guint32 buffer_time = 100000; // request 0.1 seconds of buffer time.
	int err = 0;
	int dir = 0;
	unsigned int rate = GetSampleRate ();
	unsigned int actual_rate = rate;
	guint32 channels = GetChannels ();

	if (debug) {
		err = snd_output_stdio_attach (&output, stdout, 0);
		if (err < 0)
			LOG_AUDIO ("AlsaSource::SetupHW (): Could not create alsa output: %s\n", snd_strerror (err));
	}

	err = snd_pcm_hw_params_malloc (&params);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (malloc): %s\n", snd_strerror (err));
		return false;
	}

	// choose all parameters
	err = snd_pcm_hw_params_any (pcm, params);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (no configurations available): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
	if (debug && output != NULL) {
		LOG_AUDIO ("AlsaSource::SetupHW (): hw configurations:\n");
		snd_pcm_hw_params_dump (params, output);
	}
	
	// enable software resampling
	err = snd_pcm_hw_params_set_rate_resample (pcm, params, 1);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (could not enable resampling): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
	// test for available transfer modes
	if (!(moonlight_flags & RUNTIME_INIT_AUDIO_ALSA_MMAP)) {
		err = snd_pcm_hw_params_test_access (pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
		if (err < 0) {
			LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup: RW access mode not supported (%s).\n", snd_strerror (err));			
		} else {
			rw_available = true;
		}
	}
	if (!(moonlight_flags & RUNTIME_INIT_AUDIO_ALSA_RW)) {
		err = snd_pcm_hw_params_test_access (pcm, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
		if (err < 0) {
			LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup: MMAP access mode not supported (%s).\n", snd_strerror (err));
		} else {
			mmap_available = true;
		}
	}
	if (mmap_available) {
		mmap = true;
	} else if (rw_available) {
		mmap = false;
	} else {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed, no available access mode\n");
		goto cleanup;
	}

	LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup: using %s access mode.\n", mmap ? "MMAP" : "RW");

	// set transfer mode (mmap or rw in our case)
	err = snd_pcm_hw_params_set_access (pcm, params, mmap ? SND_PCM_ACCESS_MMAP_INTERLEAVED : SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (access type not available for playback): %s\n", snd_strerror (err));
		goto cleanup;
	}

	// set audio format
	switch (GetInputBytesPerSample ()) {
	case 2: // 16 bit audio
		err = snd_pcm_hw_params_set_format (pcm, params, SND_PCM_FORMAT_S16);
		SetOutputBytesPerSample (2);
		break;
	case 3: // 24 bit audio
		// write as 32 bit audio, this is a lot easier to write to than 24 bit.
		err = snd_pcm_hw_params_set_format (pcm, params, SND_PCM_FORMAT_S32);
		SetOutputBytesPerSample (4);
		break;
	default:
		LOG_AUDIO ("AlsaSource::SetupHW (): Invalid input bytes per sample, expected 2 or 3, got %i\n", GetInputBytesPerSample ());
		goto cleanup;
	}
	
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (sample format not available for playback): %s\n", snd_strerror (err));
		goto cleanup;
	}
	
	// set channel count
	err = snd_pcm_hw_params_set_channels (pcm, params, channels);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (channels count %i not available for playback): %s\n", channels, snd_strerror (err));
		goto cleanup;
	}
	
	// set sample rate
	err = snd_pcm_hw_params_set_rate_near (pcm, params, &actual_rate, 0);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (sample rate %i Hz not available for playback): %s\n", rate, snd_strerror (err));
		goto cleanup;
	} else if (actual_rate != rate) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (sample rate %i Hz not available for playback, only got %i Hz).\n", rate, actual_rate);
		goto cleanup;
	}
	
	// set the buffer time
	err = snd_pcm_hw_params_set_buffer_time_near (pcm, params, &buffer_time, &dir);
	if (err < 0) {
		LOG_AUDIO ("AudioNode::SetupHW (): Audio HW setup failed (unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror (err));
		goto cleanup;
	}

	// write the parameters to device
	err = snd_pcm_hw_params (pcm, params);
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::SetupHW (): Audio HW setup failed (unable to set hw params for playback: %s)\n", snd_strerror (err));
		if (debug && output != NULL) {
			LOG_AUDIO ("AlsaSource::SetupHW (): current hw configurations:\n");
			snd_pcm_hw_params_dump (params, output);
		}
		goto cleanup;
	}
	
	if (debug) {
		LOG_AUDIO ("AlsaSource::SetupHW (): hardware pause support: %s\n", snd_pcm_hw_params_can_pause (params) == 0 ? "no" : "yes"); 
		LOG_AUDIO ("AlsaSource::SetupHW (): succeeded\n");
		if (output != NULL) 
			snd_pcm_hw_params_dump (params, output);
	}

	result = true;
	
cleanup:
	snd_pcm_hw_params_free (params);
	
	return result;
}

void
AlsaSource::StateChanged (AudioState old_state)
{
	if (GetState () == AudioPlaying)
		InitializeAlsa ();
	player->UpdatePollList ();
}

void
AlsaSource::Played ()
{
	InitializeAlsa ();
	player->UpdatePollList ();
}

void
AlsaSource::Paused ()
{
	mutex.Lock ();
	drop_pending = true;
	mutex.Unlock ();
}

void
AlsaSource::Stopped ()
{
	CloseAlsa ();
	mutex.Lock ();
	drop_pending = true;
	mutex.Unlock ();
}

void
AlsaSource::DropAlsa ()
{
	int err;
	
	LOG_ALSA ("AlsaSource::DropAlsa ()\n");
	
	mutex.Lock ();
	drop_pending = false;
	
	if (pcm != NULL && snd_pcm_state (pcm) == SND_PCM_STATE_RUNNING) {
		err = snd_pcm_drop (pcm);
		if (err < 0)
			LOG_AUDIO ("AlsaSource::DropAlsa (): Could not stop/drain pcm: %s\n", snd_strerror (err)); 
	}
	mutex.Unlock ();
}

void
AlsaSource::CloseInternal ()
{
	CloseAlsa ();
}

void
AlsaSource::CloseAlsa ()
{
	mutex.Lock ();
	if (pcm != NULL) {
		snd_pcm_close (pcm);
		pcm = NULL;
	}
	
	g_free (udfs);
	udfs = NULL;
	
	initialized = false;
	started = false;
	mutex.Unlock ();
}

bool
AlsaSource::WriteRW ()
{
	snd_pcm_sframes_t avail;
	snd_pcm_sframes_t commitres = 0;
	guint32 frames;
	void *buffer;
	
	if (GetState () != AudioPlaying) {
		LOG_ALSA ("AlsaSource::WriteRW (): trying to write when we're not playing (state: %i)\n", GetState ());
		return false;
	}

	if (!PreparePcm (&avail))
		return false;
	
	LOG_ALSA ("AlsaSource::WriteRW (): entering play loop, avail: %" G_GINT64_FORMAT ", sample size: %i\n", (gint64) avail, (int) period_size);
	
	buffer = g_malloc (avail * GetOutputBytesPerFrame ());
	
	frames = Write (buffer, (guint32) avail);

	mutex.Lock ();
	if (initialized)
		commitres = snd_pcm_writei (pcm, buffer, frames);
	mutex.Unlock ();
	
	g_free (buffer);
	
	LOG_ALSA ("AlsaSource::WriteRW (): played %i samples, of %i available samples, result: %i.\n", (int) frames, (int) avail, (int) commitres);
	
	if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
		if (commitres == -EAGAIN)
			LOG_AUDIO ("AlsaSource::WriteRW (): not enough space for all the data\n");
		if (!XrunRecovery (commitres >= 0 ? -EPIPE : commitres)) {
			LOG_AUDIO ("AudioPlayer: could not write audio data: %s, commitres: %li, frames: %u\n", snd_strerror (commitres), commitres, frames);
			return false;
		}
		started = false;
	}

	return frames != 0;
}

bool
AlsaSource::WriteMmap ()
{
	snd_pcm_channel_area_t *areas = NULL;
	snd_pcm_uframes_t offset = 0;
	snd_pcm_uframes_t frames;
	snd_pcm_sframes_t available_samples;
	snd_pcm_sframes_t commitres = 0;
	guint32 channels = GetChannels ();
	int err = 0;
	AudioData *data [channels + 1];
	
	if (GetState () != AudioPlaying) {
		LOG_ALSA ("AlsaSource::WriteMmap (): trying to write when we're not playing (state: %i)\n", GetState ());
		return false;
	}

	if (!PreparePcm (&available_samples))
		return false;
	
	if (GetFlag (AudioEnded)) {
		Underflowed ();
		return false;
	}
	
	LOG_ALSA_EX ("AlsaSource::WriteMmap (): entering play loop, avail: %" G_GINT64_FORMAT ", sample size: %i\n", (gint64) available_samples, (int) period_size);
	
	frames = available_samples;
	
	mutex.Lock ();
	if (!initialized)
		goto cleanup;
		
	err = snd_pcm_mmap_begin (pcm, (const snd_pcm_channel_area_t** ) &areas, &offset, &frames);
	if (err < 0) {
		if (!XrunRecovery (err)) {
			LOG_AUDIO ("AudioPlayer: could not get mmapped memory: %s\n", snd_strerror (err));
			goto cleanup;
		}
		started = false;
	}
	
	LOG_ALSA_EX ("AlsaSource::WriteMmap (): can write %lu frames, avail: %lu\n", frames, available_samples);
	
	for (guint32 channel = 0; channel < channels; channel++) {
		data [channel] = (AudioData *) g_malloc (sizeof (AudioData));
		// pointer to the first sample to write to
		data [channel]->dest = ((gint8 *) areas [channel].addr) + (areas [channel].first / 8) + offset * areas [channel].step / 8;
		// distance (in bytes) between samples
		data [channel]->distance = areas [channel].step / 8;
	}
	data [channels] = NULL;
	
	frames = WriteFull (data, frames);
	
	for (guint32 channel = 0; channel < channels; channel++) {
		g_free (data [channel]);
	}
	
	commitres = snd_pcm_mmap_commit (pcm, offset, frames);
	
	LOG_ALSA_EX ("AlsaSource::WriteMmap (): played %i samples, of %i available samples, result: %i.\n", (int) frames, (int) 0, (int) commitres);
	
	if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
		if (!XrunRecovery (commitres >= 0 ? -EPIPE : commitres)) {
			LOG_AUDIO ("AudioPlayer: could not commit mmapped memory: %s\n", snd_strerror(err));
			commitres = 0; // so that we end up returning false
			goto cleanup;
		}
		started = false;
	}

cleanup:

	mutex.Unlock ();

	return commitres > 0;
}

bool
AlsaSource::WriteAlsa ()
{
	if (mmap)
		return WriteMmap ();
	else
		return WriteRW ();
}

bool
AlsaSource::XrunRecovery (int err)
{	
	switch (err) {
	case -EPIPE: // under-run
		Underflowed ();
		mutex.Lock ();
		if (initialized) {
			err = snd_pcm_prepare (pcm);
			if (err < 0) {
				LOG_AUDIO ("AlsaPlayer: Can't recover from underrun, prepare failed: %s.\n", snd_strerror (err));
			}
		} else {
			LOG_AUDIO ("AlsaPlayer: Can't recover from underrun, pcm has been closed.\n");
		}
		mutex.Unlock ();
		break;
	case -ESTRPIPE:
		mutex.Lock ();
		if (initialized) {
			while ((err = snd_pcm_resume (pcm)) == -EAGAIN) {
				LOG_AUDIO ("XrunRecovery: waiting for resume\n");
				sleep (1); // wait until the suspend flag is released
			}
			if (err < 0) {
				err = snd_pcm_prepare (pcm);
				if (err < 0) {
					LOG_AUDIO ("AlsaPlayer: Can't recover from suspend, prepare failed: %s.\n", snd_strerror (err));
				}
			}
		} else {
			LOG_AUDIO ("AlsaPlayer: Can't recover from suspend, pcm has been closed.\n");
		}
		mutex.Unlock ();
		break;
	default:
		LOG_AUDIO ("AlsaPlayer: Can't recover from underrun: %s\n", snd_strerror (err));
		break;
	}
	
	return err >= 0;
}

bool
AlsaSource::PreparePcm (snd_pcm_sframes_t *avail)
{
	int err = 0;
	bool closed = false;
	snd_pcm_state_t state;
	
	mutex.Lock ();
	if (initialized) {
		state = snd_pcm_state (pcm);
	} else {
		LOG_ALSA ("AlsaSource::PreparePcm (): pcm has been closed.\n");
		closed = true;
	}
	mutex.Unlock ();
			
	if (closed)
		return false;
		
	switch (state) {
	case SND_PCM_STATE_XRUN:
		LOG_ALSA ("AlsaSource::PreparePcm (): SND_PCM_STATE_XRUN.\n");

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
		LOG_ALSA ("AlsaSource::PreparePcm (): state: %s (prepare failed)\n", snd_pcm_state_name (state));
		return false;
	}
	
	err = 0;
	mutex.Lock ();
	if (initialized) {
		*avail = snd_pcm_avail_update (pcm);
	} else {
		closed = true;
	}
	mutex.Unlock ();
	
	if (closed)
		return false;

	if (*avail < 0) {
		if (!XrunRecovery (*avail))
			return false;

		started = false;
		return false;
	}

	if ((snd_pcm_uframes_t) *avail < period_size) {
		if (!started) {
			LOG_ALSA ("AlsaSource::PreparePcm (): starting pcm (period size: %li, available: %li)\n", period_size, *avail);

			mutex.Lock ();
			if (initialized) {
				err = snd_pcm_start (pcm);
			} else {
				closed = true;
			}
			mutex.Unlock ();
			
			if (closed)
				return false;

			if (err < 0) {
				LOG_AUDIO ("AlsaPlayer: Could not start pcm: %s\n", snd_strerror (err));
				return false;
			}
			started = true;
		} else {
			return false;
		}
		return false;
	}

	LOG_ALSA ("AlsaSource::PreparePcm (): Prepared, avail: %li, started: %i\n", *avail, (int) started);

	return true;
}

guint64
AlsaSource::GetDelayInternal ()
{
	snd_pcm_sframes_t delay;
	int err;
	guint64 result;
	bool update_error = false;
	bool not_initialized = false;
	
	mutex.Lock ();
	if (!initialized) {
		not_initialized = true;
	} else {
		err = snd_pcm_avail_update (pcm);
	
		if (err < 0) {
			LOG_AUDIO ("AlsaSource::GetDelayInternal (): Could not update delay (%s)\n", snd_strerror (err));
			update_error = true;
		} else {
			err = snd_pcm_delay (pcm, &delay);
		}
	}
	mutex.Unlock ();
	
	if (not_initialized) {
		LOG_AUDIO ("AlsaSource::GetDelayInternal (): pcm has been closed.\n");
		return G_MAXUINT64;
	}
	
	if (update_error)
		return G_MAXUINT64;
	
	if (err < 0) {
		LOG_AUDIO ("AlsaSource::GetDelayInternal (): Could not get delay (%s)\n", snd_strerror (err));
		result = G_MAXUINT64;
	} else if (delay < 0) {
		LOG_AUDIO ("AlsaSource::GetDelayInternal (): Got negative delay (%li)\n", delay);
		result = G_MAXUINT64;
	} else {
		result = (guint64) TIMESPANTICKS_IN_SECOND * (guint64) delay / (guint64) GetSampleRate ();
	}
	
	return result;
}

/*
 * AlsaPlayer
 */
 
AlsaPlayer::AlsaPlayer ()
{	
	LOG_ALSA ("AlsaPlayer::AlsaPlayer ()\n");
	
	audio_thread = NULL;
	shutdown = false;
	update_poll_pending = true;
	udfs = NULL;
	ndfs = 0;
	fds [0] = -1;
	fds [1] = -1;
}

AlsaPlayer::~AlsaPlayer ()
{
	LOG_ALSA ("AlsaPlayer::~AlsaPlayer ()\n");
}

bool
AlsaPlayer::Initialize ()
{
	bool result;
	
	LOG_ALSA ("AlsaPlayer::Initialize ()\n");
	
	// Create our spipe
	if (pipe (fds) != 0) {
		LOG_AUDIO ("AlsaPlayer::Initialize (): Unable to create pipe (%s).\n", strerror (errno));
		return false;
	}

	// Make the writer pipe non-blocking.
	fcntl (fds [1], F_SETFL, fcntl (fds [1], F_GETFL) | O_NONBLOCK);

	// Create the audio thread
	audio_thread = (pthread_t *) g_malloc (sizeof (pthread_t));
	result = pthread_create (audio_thread, NULL, Loop, this);
	if (result != 0) {
		LOG_AUDIO ("AlsaPlayer::Initialize (): could not create audio thread (error code: %i = '%s').\n", result, strerror (result));
		g_free (audio_thread);
		audio_thread = NULL;
		return false;
	}
	
	LOG_ALSA ("AlsaPlayer::Initialize (): the audio player has been initialized.\n");
	
	return true;
}

static int is_alsa_usable = 0; // 0 = not tested, 1 = tested, usable, 2 = tested, not usable
static void *libalsa = NULL;

bool
AlsaPlayer::IsInstalled ()
{
	bool result = false;
	const char *version;
	
	switch (is_alsa_usable) {
	case 0:
		libalsa = dlopen ("libasound.so.2", RTLD_LAZY);
		if (libalsa == NULL) {
			is_alsa_usable = 2;
			return false;
		}
		result = true;
		
		result &= NULL != (d_snd_pcm_open = (dyn_snd_pcm_open *) dlsym (libalsa, "snd_pcm_open"));
		result &= NULL != (d_snd_pcm_close = (dyn_snd_pcm_close *) dlsym (libalsa, "snd_pcm_close"));
		result &= NULL != (d_snd_pcm_get_params = (dyn_snd_pcm_get_params *) dlsym (libalsa, "snd_pcm_get_params"));
		result &= NULL != (d_snd_pcm_poll_descriptors_count = (dyn_snd_pcm_poll_descriptors_count *) dlsym (libalsa, "snd_pcm_poll_descriptors_count"));
		result &= NULL != (d_snd_pcm_poll_descriptors = (dyn_snd_pcm_poll_descriptors *) dlsym (libalsa, "snd_pcm_poll_descriptors"));
		result &= NULL != (d_snd_output_stdio_attach = (dyn_snd_output_stdio_attach *) dlsym (libalsa, "snd_output_stdio_attach"));
		result &= NULL != (d_snd_pcm_hw_params_malloc = (dyn_snd_pcm_hw_params_malloc *) dlsym (libalsa, "snd_pcm_hw_params_malloc"));
		result &= NULL != (d_snd_pcm_hw_params_any = (dyn_snd_pcm_hw_params_any *) dlsym (libalsa, "snd_pcm_hw_params_any"));
		result &= NULL != (d_snd_pcm_hw_params_dump = (dyn_snd_pcm_hw_params_dump *) dlsym (libalsa, "snd_pcm_hw_params_dump"));
		result &= NULL != (d_snd_pcm_hw_params_set_rate_resample = (dyn_snd_pcm_hw_params_set_rate_resample *) dlsym (libalsa, "snd_pcm_hw_params_set_rate_resample"));
		result &= NULL != (d_snd_pcm_hw_params_test_access = (dyn_snd_pcm_hw_params_test_access *) dlsym (libalsa, "snd_pcm_hw_params_test_access"));
		result &= NULL != (d_snd_pcm_hw_params_set_access = (dyn_snd_pcm_hw_params_set_access *) dlsym (libalsa, "snd_pcm_hw_params_set_access"));
		result &= NULL != (d_snd_pcm_hw_params_set_format = (dyn_snd_pcm_hw_params_set_format *) dlsym (libalsa, "snd_pcm_hw_params_set_format"));
		result &= NULL != (d_snd_pcm_hw_params_set_channels = (dyn_snd_pcm_hw_params_set_channels *) dlsym (libalsa, "snd_pcm_hw_params_set_channels"));
		result &= NULL != (d_snd_pcm_hw_params_set_rate_near = (dyn_snd_pcm_hw_params_set_rate_near *) dlsym (libalsa, "snd_pcm_hw_params_set_rate_near"));
		result &= NULL != (d_snd_pcm_hw_params_set_buffer_time_near = (dyn_snd_pcm_hw_params_set_buffer_time_near *) dlsym (libalsa, "snd_pcm_hw_params_set_buffer_time_near"));
		result &= NULL != (d_snd_pcm_hw_params = (dyn_snd_pcm_hw_params *) dlsym (libalsa, "snd_pcm_hw_params"));
		result &= NULL != (d_snd_pcm_hw_params_can_pause = (dyn_snd_pcm_hw_params_can_pause *) dlsym (libalsa, "snd_pcm_hw_params_can_pause"));
		result &= NULL != (d_snd_pcm_hw_params_free = (dyn_snd_pcm_hw_params_free *) dlsym (libalsa, "snd_pcm_hw_params_free"));
		result &= NULL != (d_snd_pcm_state = (dyn_snd_pcm_state *) dlsym (libalsa, "snd_pcm_state"));
		result &= NULL != (d_snd_pcm_state_name = (dyn_snd_pcm_state_name *) dlsym (libalsa, "snd_pcm_state_name"));
		result &= NULL != (d_snd_pcm_drop = (dyn_snd_pcm_drop *) dlsym (libalsa, "snd_pcm_drop"));
		result &= NULL != (d_snd_pcm_writei = (dyn_snd_pcm_writei *) dlsym (libalsa, "snd_pcm_writei"));
		result &= NULL != (d_snd_pcm_mmap_begin = (dyn_snd_pcm_mmap_begin *) dlsym (libalsa, "snd_pcm_mmap_begin"));
		result &= NULL != (d_snd_pcm_mmap_commit = (dyn_snd_pcm_mmap_commit *) dlsym (libalsa, "snd_pcm_mmap_commit"));
		result &= NULL != (d_snd_pcm_prepare = (dyn_snd_pcm_prepare *) dlsym (libalsa, "snd_pcm_prepare"));
		result &= NULL != (d_snd_pcm_resume = (dyn_snd_pcm_resume *) dlsym (libalsa, "snd_pcm_resume"));
		result &= NULL != (d_snd_pcm_avail_update = (dyn_snd_pcm_avail_update *) dlsym (libalsa, "snd_pcm_avail_update"));
		result &= NULL != (d_snd_pcm_start = (dyn_snd_pcm_start *) dlsym (libalsa, "snd_pcm_start"));
		result &= NULL != (d_snd_pcm_delay = (dyn_snd_pcm_delay *) dlsym (libalsa, "snd_pcm_delay"));
		result &= NULL != (d_snd_asoundlib_version = (dyn_snd_asoundlib_version *) dlsym (libalsa, "snd_asoundlib_version"));
		result &= NULL != (d_snd_strerror = (dyn_snd_strerror *) dlsym (libalsa, "snd_strerror"));

		if (d_snd_asoundlib_version != NULL) {
			version = d_snd_asoundlib_version ();
			LOG_AUDIO ("AlsaPlayer: Found alsa/asound version: '%s'\n", version);
		}
		
		if (!result)
			LOG_AUDIO ("AlsaPlayer: Failed to load one or more required functions in libasound.so.");

		is_alsa_usable = result ? 1 : 2;
		return result;
	case 1:
		return true;
	default:
		return false;
	}
	
	return true;
}

void
AlsaPlayer::AddInternal (AudioSource *source)
{
	update_poll_pending = true;
	WakeUp ();
}

void
AlsaPlayer::RemoveInternal (AudioSource *source)
{
	update_poll_pending = true;
	WakeUp ();
}

void
AlsaPlayer::PrepareShutdownInternal ()
{
	int result = 0;
	
	LOG_ALSA ("AlsaPlayer::PrepareShutdownInternal ().\n");

	// Wait for the audio thread to finish
	shutdown = true;
	if (audio_thread != NULL) {
		WakeUp ();
		result = pthread_join (*audio_thread, NULL);
		if (result != 0) {
			LOG_AUDIO ("AudioPlayer::Shutdown (): failed to join the audio thread (error code: %i).\n", result);
		} else {
			// Only free the thread if we could join it.
			g_free (audio_thread);
		}
		audio_thread = NULL;
	}
}

void
AlsaPlayer::FinishShutdownInternal ()
{
	LOG_ALSA ("AlsaPlayer::FinishShutdownInternal ().\n");
	
	if (fds [0] != -1) {
		close (fds [0]);
		fds [0] = -1;
	}
	if (fds [1] != -1) {
		close (fds [1]);
		fds [1] = -1;
	}
	
	g_free (udfs);
	udfs = NULL;
	ndfs = 0;
}

void
AlsaPlayer::UpdatePollList ()
{
	update_poll_pending = true;
	WakeUp ();
}

AudioSource *
AlsaPlayer::CreateNode (MediaPlayer *mplayer, AudioStream *stream)
{
	return new AlsaSource (this, mplayer, stream);
}

void*
AlsaPlayer::Loop (void *data)
{
	((AlsaPlayer *) data)->Loop ();
	return NULL;
}

void
AlsaPlayer::Loop ()
{
	AlsaSource *source = NULL;

	bool played_something;
	int result;
	int buffer;
	int current;
	
	LOG_ALSA ("AlsaPlayer: entering audio loop.\n");

	while (!shutdown) {
		sources.StartEnumeration ();
		
		played_something = false;
		while ((source = (AlsaSource *) sources.GetNext (false)) != NULL) {
			if (source->GetState () == AudioPlaying) {
				if (source->WriteAlsa ())
					played_something = true;
			} else if (source->IsDropPending ()) {
				source->DropAlsa ();
			}
			source->unref ();
		}
		
		if (played_something)
			continue;
	
		// None of the audio nodes in the list played anything
		// (or there are no audio nodes), so wait for something
		// to happen. We handle spurious wakeups correctly, so 
		// there is no find out exactly what happened.

		while (!shutdown && update_poll_pending) {
			/*
			 * We need to update the list of file descriptors we poll on
			 * to only include audio nodes which are playing.
			 */
			update_poll_pending = false;
			
			ndfs = 1;
			current = 1;
			sources.StartEnumeration ();
			while ((source = (AlsaSource *) sources.GetNext (true)) != NULL) {
				ndfs += source->ndfs;
				source->unref ();
			}
			
			g_free (udfs);
			udfs = (pollfd*) g_malloc0 (sizeof (pollfd) * ndfs);
			udfs [0].fd = fds [0];
			udfs [0].events = POLLIN;
		
			sources.StartEnumeration ();
			while (!update_poll_pending && (source = (AlsaSource *) sources.GetNext (true)) != NULL) {
				if (current + source->ndfs > ndfs) {
					// the list of sources changed.
					update_poll_pending = true;
				} else {
					memcpy (&udfs [current], source->udfs, source->ndfs * sizeof (pollfd));
					current += source->ndfs;
				}
				source->unref ();
			}
			
			if (current != ndfs) {
				// The list of sources changed
				update_poll_pending = true;
			}
		}
		
		do {
			udfs [0].events = POLLIN;
			udfs [0].revents = 0;
			
			LOG_ALSA_EX ("AlsaPlayer::Loop (): polling... ndfs: %i\n", ndfs);
			
			result = poll (udfs, ndfs, 10000); // Have a timeout of 10 seconds, just in case something goes wrong.
			
			LOG_ALSA_EX ("AlsaPlayer::Loop (): poll result: %i, fd: %i, fd [0].revents: %i, errno: %i, err: %s, ndfs = %i, shutdown: %i\n", 
				result, udfs [0].fd, (int) udfs [0].revents, errno, strerror (errno), ndfs, shutdown);

			if (result == 0) { // Timed out
				LOG_ALSA_EX ("AlsaPlayer::Loop (): poll timed out.\n");
			} else if (result < 0) { // Some error poll exit condition
				// Doesn't matter what happened (happens quite often due to interrupts)
				LOG_ALSA_EX ("AlsaPlayer::Loop (): poll failed: %i (%s)\n", errno, strerror (errno));
			} else { // Something woke up the poll
				if (udfs [0].revents & POLLIN) {
					// We were asked to wake up by the audio player
					// Read whatever was written into the pipe so that the pipe doesn't fill up.
					read (udfs [0].fd, &buffer, sizeof (int));
					LOG_ALSA_EX ("AlsaPlayer::Loop (): woken up by ourselves.\n");
				} else {
					// Something happened on any of the audio streams
				}
			}
		} while (result == -1 && errno == EINTR);
	}
			
	LOG_ALSA ("AlsaPlayer: exiting audio loop.\n");
}

void
AlsaPlayer::WakeUp ()
{
	int result;
		
	LOG_ALSA_EX ("AlsaPlayer::WakeUp ().\n");
		
	// Write until something has been written.
	do {
		result = write (fds [1], "c", 1);
	} while (result == 0);
	
	if (result == -1)
		LOG_AUDIO ("AlsaPlayer::WakeUp (): Could not wake up audio thread: %s\n", strerror (errno));
		
	LOG_ALSA_EX ("AlsaPlayer::WakeUp (): thread should now wake up (or have woken up already).\n");
	
}
