/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-pulse.cpp:
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

#include "audio-pulse.h"
#include "runtime.h"
#include "clock.h"
#include "debug.h"

// stream.h
typedef pa_stream*            (dyn_pa_stream_new)                    (pa_context *c, const char *name, const pa_sample_spec *ss, const pa_channel_map *map);
typedef void                  (dyn_pa_stream_set_state_callback)     (pa_stream *s, pa_stream_notify_cb_t cb, void *userdata);
typedef void                  (dyn_pa_stream_set_write_callback)     (pa_stream *p, pa_stream_request_cb_t cb, void *userdata);
typedef void                  (dyn_pa_stream_set_underflow_callback) (pa_stream *p, pa_stream_notify_cb_t cb, void *userdata);
typedef int                   (dyn_pa_stream_connect_playback)       (pa_stream *s, const char *dev, const pa_buffer_attr *attr, pa_stream_flags_t flags, pa_cvolume *volume, pa_stream *sync_stream);
typedef int                   (dyn_pa_stream_disconnect)             (pa_stream *s);
typedef void                  (dyn_pa_stream_unref)                  (pa_stream *s);
typedef pa_stream_state_t     (dyn_pa_stream_get_state)              (pa_stream *p);
typedef int                   (dyn_pa_stream_write)                  (pa_stream *p, const void *data, size_t bytes, pa_free_cb_t free_cb, int64_t offset, pa_seek_mode_t seek);
typedef size_t                (dyn_pa_stream_writable_size)          (pa_stream *p);
typedef pa_operation*         (dyn_pa_stream_cork)                   (pa_stream *s, int b, pa_stream_success_cb_t cb, void *userdata);
typedef pa_operation*         (dyn_pa_stream_trigger)                (pa_stream *s, pa_stream_success_cb_t cb, void *userdata);
typedef pa_operation*         (dyn_pa_stream_flush)                  (pa_stream *s, pa_stream_success_cb_t cb, void *userdata);
typedef int                   (dyn_pa_stream_get_latency)            (pa_stream *s, pa_usec_t *r_usec, int *negative);
// context.h
typedef pa_context *          (dyn_pa_context_new)                   (pa_mainloop_api *mainloop, const char *name);
typedef int                   (dyn_pa_context_errno)                 (pa_context *c);
typedef pa_context_state_t    (dyn_pa_context_get_state)             (pa_context *c);
typedef void                  (dyn_pa_context_set_state_callback)    (pa_context *c, pa_context_notify_cb_t cb, void *userdata);
typedef int                   (dyn_pa_context_connect)               (pa_context *c, const char *server, pa_context_flags_t flags, const pa_spawn_api *api);
typedef void                  (dyn_pa_context_disconnect)            (pa_context *c);
typedef void                  (dyn_pa_context_unref)                 (pa_context *c);
// thread-mainloop.h
typedef pa_threaded_mainloop* (dyn_pa_threaded_mainloop_new)         (void);
typedef int                   (dyn_pa_threaded_mainloop_start)       (pa_threaded_mainloop *m);
typedef pa_mainloop_api*      (dyn_pa_threaded_mainloop_get_api)     (pa_threaded_mainloop*m);
typedef void                  (dyn_pa_threaded_mainloop_wait)        (pa_threaded_mainloop *m);
typedef int                   (dyn_pa_threaded_mainloop_in_thread)   (pa_threaded_mainloop *m);
typedef void                  (dyn_pa_threaded_mainloop_lock)        (pa_threaded_mainloop *m);
typedef void                  (dyn_pa_threaded_mainloop_unlock)      (pa_threaded_mainloop *m);
typedef void                  (dyn_pa_threaded_mainloop_signal)      (pa_threaded_mainloop *m, int wait_for_accept);
typedef void                  (dyn_pa_threaded_mainloop_stop)        (pa_threaded_mainloop *m);
typedef void                  (dyn_pa_threaded_mainloop_free)        (pa_threaded_mainloop* m);
// channelmap.h
typedef pa_channel_map*       (dyn_pa_channel_map_init_mono)         (pa_channel_map *m);
typedef pa_channel_map*       (dyn_pa_channel_map_init_stereo)       (pa_channel_map *m);
typedef pa_channel_map*       (dyn_pa_channel_map_init_auto)         (pa_channel_map *m, unsigned channels, pa_channel_map_def_t def);
// error.h
typedef const char*           (dyn_pa_strerror)                      (int error);
// operation.h
typedef pa_operation_state_t  (dyn_pa_operation_get_state)           (pa_operation *o);
typedef void                  (dyn_pa_operation_unref)                   (pa_operation *o);
// version.h
typedef const char *          (dyn_pa_get_library_version)           ();

dyn_pa_stream_new *                    d_pa_stream_new = NULL;
dyn_pa_stream_set_state_callback *     d_pa_stream_set_state_callback = NULL;
dyn_pa_stream_set_write_callback *     d_pa_stream_set_write_callback = NULL;
dyn_pa_stream_set_underflow_callback * d_pa_stream_set_underflow_callback = NULL;
dyn_pa_stream_connect_playback *       d_pa_stream_connect_playback = NULL;
dyn_pa_stream_disconnect *             d_pa_stream_disconnect = NULL;
dyn_pa_stream_unref *                  d_pa_stream_unref = NULL;
dyn_pa_stream_get_state *              d_pa_stream_get_state = NULL;
dyn_pa_stream_write *                  d_pa_stream_write = NULL;
dyn_pa_stream_writable_size *          d_pa_stream_writable_size = NULL;
dyn_pa_stream_cork *                   d_pa_stream_cork = NULL;
dyn_pa_stream_trigger *                d_pa_stream_trigger = NULL;
dyn_pa_stream_flush *                  d_pa_stream_flush = NULL;
dyn_pa_stream_get_latency *            d_pa_stream_get_latency = NULL;
dyn_pa_context_new *                   d_pa_context_new = NULL;
dyn_pa_context_errno *                 d_pa_context_errno = NULL;
dyn_pa_context_get_state *             d_pa_context_get_state = NULL;
dyn_pa_context_set_state_callback *    d_pa_context_set_state_callback = NULL;
dyn_pa_context_connect *               d_pa_context_connect = NULL;
dyn_pa_context_disconnect *            d_pa_context_disconnect = NULL;
dyn_pa_context_unref *                 d_pa_context_unref = NULL;
dyn_pa_threaded_mainloop_new *         d_pa_threaded_mainloop_new = NULL;
dyn_pa_threaded_mainloop_start *       d_pa_threaded_mainloop_start = NULL;
dyn_pa_threaded_mainloop_get_api *     d_pa_threaded_mainloop_get_api = NULL;
dyn_pa_threaded_mainloop_wait *        d_pa_threaded_mainloop_wait = NULL;
dyn_pa_threaded_mainloop_in_thread *   d_pa_threaded_mainloop_in_thread = NULL;
dyn_pa_threaded_mainloop_lock *        d_pa_threaded_mainloop_lock = NULL;
dyn_pa_threaded_mainloop_unlock *      d_pa_threaded_mainloop_unlock = NULL;
dyn_pa_threaded_mainloop_signal *      d_pa_threaded_mainloop_signal = NULL;
dyn_pa_threaded_mainloop_stop *        d_pa_threaded_mainloop_stop = NULL;
dyn_pa_threaded_mainloop_free *        d_pa_threaded_mainloop_free = NULL;
dyn_pa_channel_map_init_mono *         d_pa_channel_map_init_mono = NULL;
dyn_pa_channel_map_init_stereo *       d_pa_channel_map_init_stereo = NULL;
dyn_pa_channel_map_init_auto *         d_pa_channel_map_init_auto = NULL;
dyn_pa_strerror *                      d_pa_strerror = NULL;
dyn_pa_operation_get_state *           d_pa_operation_get_state = NULL;
dyn_pa_operation_unref *               d_pa_operation_unref = NULL;
dyn_pa_get_library_version *           d_pa_get_library_version = NULL;

#define pa_stream_new                    d_pa_stream_new
#define pa_stream_set_state_callback     d_pa_stream_set_state_callback
#define pa_stream_set_write_callback     d_pa_stream_set_write_callback
#define pa_stream_set_underflow_callback d_pa_stream_set_underflow_callback
#define pa_stream_connect_playback       d_pa_stream_connect_playback
#define pa_stream_disconnect             d_pa_stream_disconnect
#define pa_stream_unref                  d_pa_stream_unref
#define pa_stream_get_state              d_pa_stream_get_state
#define pa_stream_write                  d_pa_stream_write
#define pa_stream_writable_size          d_pa_stream_writable_size
#define pa_stream_cork                   d_pa_stream_cork
#define pa_stream_trigger                d_pa_stream_trigger
#define pa_stream_flush                  d_pa_stream_flush
#define pa_stream_get_latency            d_pa_stream_get_latency
#define pa_context_new                   d_pa_context_new
#define pa_context_errno                 d_pa_context_errno
#define pa_context_get_state             d_pa_context_get_state
#define pa_context_set_state_callback    d_pa_context_set_state_callback
#define pa_context_connect               d_pa_context_connect
#define pa_context_disconnect            d_pa_context_disconnect
#define pa_context_unref                 d_pa_context_unref
#define pa_threaded_mainloop_new		 d_pa_threaded_mainloop_new
#define pa_threaded_mainloop_start		 d_pa_threaded_mainloop_start
#define pa_threaded_mainloop_get_api	 d_pa_threaded_mainloop_get_api
#define pa_threaded_mainloop_wait		 d_pa_threaded_mainloop_wait
#define pa_threaded_mainloop_in_thread	 d_pa_threaded_mainloop_in_thread
#define pa_threaded_mainloop_lock		 d_pa_threaded_mainloop_lock
#define pa_threaded_mainloop_unlock		 d_pa_threaded_mainloop_unlock
#define pa_threaded_mainloop_signal		 d_pa_threaded_mainloop_signal
#define pa_threaded_mainloop_stop		 d_pa_threaded_mainloop_stop
#define pa_threaded_mainloop_free		 d_pa_threaded_mainloop_free
#define pa_channel_map_init_mono         d_pa_channel_map_init_mono
#define pa_channel_map_init_stereo       d_pa_channel_map_init_stereo
#define pa_channel_map_init_auto         d_pa_channel_map_init_auto
#define pa_strerror                      d_pa_strerror
#define pa_operation_get_state           d_pa_operation_get_state
#define pa_operation_unref               d_pa_operation_unref

/*
 * PulseSource
 */

PulseSource::PulseSource (PulsePlayer *player, MediaPlayer *mplayer, AudioStream *stream) : AudioSource (player, mplayer, stream)
{
	LOG_PULSE ("PulseSource::PulseSource ()\n");
	
	this->player = player;
	pulse_stream = NULL;
	initialized = false;
	triggered = false;
	is_ready = false;
	play_pending = false;
}

PulseSource::~PulseSource ()
{
	LOG_PULSE ("PulseSource::~PulseSource ()\n");
	Close ();
}

bool
PulseSource::InitializeInternal ()
{
	LOG_PULSE ("PulseSource::InitializeInternal (), initialized: %i\n", initialized);
	// this is a no-op, initialization is done when needed.
	return true;
}

bool
PulseSource::InitializePA ()
{
	int err;
	pa_sample_spec format;
	pa_channel_map channel_map;
	bool result = false;
	
	LOG_AUDIO ("PulseSource::InitializePA ()\n");
		
	if (initialized)
		return true;
	
	if (player->GetPAState () != PA_CONTEXT_READY) {
		LOG_PULSE ("PulseSource::InitializePA (), PA isn't in the ready state.\n");
		return false;
	}
	
	player->LockLoop ();
	
	switch (GetInputBytesPerSample ()) {
	case 2:
		format.format = PA_SAMPLE_S16NE;
		SetOutputBytesPerSample (2);
		break;
	case 3:
		format.format = PA_SAMPLE_S32NE;
		SetOutputBytesPerSample (4);
		break;
	default:
		LOG_AUDIO ("PulseSource::InitializePA (): Invalid bytes per sample: %i (expected 1, 2 or 3)\n", GetInputBytesPerSample ());
		goto cleanup;
		break;
	}
	
	format.rate = GetSampleRate ();
	format.channels = GetChannels ();
	
	if (format.channels == 1) {
		pa_channel_map_init_mono (&channel_map);
	} else if (format.channels == 2) {
		pa_channel_map_init_stereo (&channel_map);
	} else if (format.channels == 6 || format.channels == 8) {
		channel_map.channels = format.channels;
		for (unsigned int c = 0; c < PA_CHANNELS_MAX; c++)
			channel_map.map [c] = PA_CHANNEL_POSITION_INVALID;
		
		// this map needs testing with a 5.1/7.1 system.
		channel_map.map [0] = PA_CHANNEL_POSITION_FRONT_LEFT;
		channel_map.map [1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
		channel_map.map [2] = PA_CHANNEL_POSITION_FRONT_CENTER;
		channel_map.map [3] = PA_CHANNEL_POSITION_LFE;
		channel_map.map [4] = PA_CHANNEL_POSITION_REAR_LEFT;
		channel_map.map [5] = PA_CHANNEL_POSITION_REAR_RIGHT;
		if (format.channels == 8) {
			channel_map.map [6] = PA_CHANNEL_POSITION_SIDE_LEFT;
			channel_map.map [7] = PA_CHANNEL_POSITION_SIDE_RIGHT;
		}
	} else {
		if (pa_channel_map_init_auto (&channel_map, format.channels, PA_CHANNEL_MAP_DEFAULT) == NULL) {
			LOG_AUDIO ("PulseSource::InitializePA (): Invalid number of channels: %i\n", format.channels);
			goto cleanup;
		}
	}
	
	pulse_stream = pa_stream_new (player->GetPAContext (), "Audio stream", &format, &channel_map);
	if (pulse_stream == NULL) {
		LOG_AUDIO ("PulseSource::InitializePA (): Stream creation failed: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		goto cleanup;
	}
			
	pa_stream_set_state_callback (pulse_stream, OnStateChanged, this);
	pa_stream_set_write_callback (pulse_stream, OnWrite, this);
	pa_stream_set_underflow_callback (pulse_stream, OnUnderflow, this);
	
	err = pa_stream_connect_playback (pulse_stream, NULL, NULL, (pa_stream_flags_t) (PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_START_CORKED), NULL, NULL);
	if (err < 0) {
		LOG_AUDIO ("PulseSource::InitializePA (): failed to connect stream: %s.\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		goto cleanup;
	}
	
	result = true;
	initialized = true;
	
cleanup:
	player->UnlockLoop ();
								
	return result;
}

void
PulseSource::CloseInternal ()
{
	LOG_PULSE ("PulseSource::CloseInternal ()\n");
	
	ClosePA ();
}

void
PulseSource::ClosePA ()
{
	LOG_PULSE ("PulseSource::ClosePA () initialized: %i\n", initialized);
	
	if (!initialized)
		return;
	
	is_ready = false;
	
	player->LockLoop ();
	if (pulse_stream) {
		pa_stream_set_state_callback (pulse_stream, NULL, NULL);
		pa_stream_set_write_callback (pulse_stream, NULL, NULL);
		pa_stream_set_underflow_callback (pulse_stream, NULL, NULL);
		pa_stream_disconnect (pulse_stream);
		pa_stream_unref (pulse_stream);
		pulse_stream = NULL;
	}
	player->UnlockLoop ();
	initialized = false;
}

void
PulseSource::OnStateChanged (pa_stream *pulse_stream, void *userdata)
{
	((PulseSource *) userdata)->OnStateChanged (pulse_stream);
}

#ifdef LOGGING
static const char *
get_pa_stream_state_name (pa_stream_state_t state)
{
	switch (state) {
	case PA_STREAM_CREATING: return "PA_STREAM_CREATING";
	case PA_STREAM_TERMINATED: return "PA_STREAM_TERMINATED";
	case PA_STREAM_READY: return "PA_STREAM_READY";
	case PA_STREAM_FAILED: return "PA_STREAM_FAILED";
	default: return "<UNKNOWN>";
	}
}
#endif

pa_stream_state_t
PulseSource::GetPAState (pa_stream *pulse_stream)
{
	pa_stream_state_t result;
	
	player->LockLoop ();
	
	if (pulse_stream == NULL)
		pulse_stream = this->pulse_stream;
	if (pulse_stream != NULL) {
		result = pa_stream_get_state (pulse_stream);
	} else {
		result = PA_STREAM_FAILED;
	}
	player->UnlockLoop ();
	
	return result;
}

void
PulseSource::OnUnderflow (pa_stream *pulse_stream, void *userdata)
{
	((PulseSource *) userdata)->OnUnderflow ();
}

void
PulseSource::OnUnderflow ()
{
	AudioSource::Underflowed ();
}

void
PulseSource::StateChanged (AudioState old_state)
{
	if (is_ready && GetState () == AudioPlaying)
		WriteAvailable ();
}

void
PulseSource::OnStateChanged (pa_stream *pulse_stream)
{
	pa_stream_state_t state;
	
	if (pulse_stream != this->pulse_stream && this->pulse_stream != NULL) {
		LOG_AUDIO ("PulseSource::OnStateChanged (%p): Invalid stream.\n", pulse_stream);
		return;
	}
	
	state = GetPAState (pulse_stream);
	
	SetCurrentDeployment (false);
	
	LOG_PULSE ("PulseSource::OnStateChanged (): %s (%i)\n", get_pa_stream_state_name (state), state);
	
	switch (state) {
	case PA_STREAM_READY:
		is_ready = true;
		break;
	case PA_STREAM_CREATING:
	case PA_STREAM_TERMINATED:
		is_ready = false;
		break;
	case PA_STREAM_FAILED:
	default:
		is_ready = false;
		LOG_AUDIO ("PulseSource::OnStateChanged (): Stream error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		SetState (AudioError);
		break;
	}
}

void
PulseSource::OnWrite (pa_stream *s, size_t length, void *userdata)
{
	((PulseSource *) userdata)->OnWrite (length);
}

void
PulseSource::OnWrite (size_t length)
{
	void *buffer;
	int err;
	size_t frames;
	
	LOG_PULSE ("PulseSource::OnWrite (%" G_GINT64_FORMAT ")\n", (gint64) length);
	
	if (pulse_stream == NULL) {
		// We've been destroyed
		return;
	}
	
	if (length == 0)
		return;

	buffer = g_malloc (length);
	
	frames = Write (buffer, length / GetOutputBytesPerFrame ());
	
	LOG_PULSE ("PulseSource::OnWrite (%" G_GINT64_FORMAT "): Wrote %" G_GUINT64_FORMAT " frames\n", (gint64) length, (gint64) frames);	
	
	if (frames > 0) {
		// There is no need to lock here, if in a callback, the caller will have locked
		// if called from WriteAvailable, that method has locked
		err = pa_stream_write (pulse_stream, buffer, frames * GetOutputBytesPerFrame (), (pa_free_cb_t) g_free, 0, PA_SEEK_RELATIVE);
		if (err < 0) {
			LOG_AUDIO ("PulseSource::OnWrite (): Write error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		} else if (play_pending) {
			Played ();
		}
	} else {
		g_free (buffer);
	}
}

void
PulseSource::WriteAvailable ()
{
	size_t available;
	
	LOG_PULSE ("PulseSource::WriteAvailable ()\n");
					
	player->LockLoop ();
	if (pulse_stream != NULL && is_ready) {
		available = pa_stream_writable_size (pulse_stream);
		if (available != (size_t) -1) {
			OnWrite (available);
		} else {
			LOG_AUDIO ("PulseSource::WriteAvailable (): Write error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		}
	}
	player->UnlockLoop ();
}

void
PulseSource::PACork (bool cork)
{
	LOG_PULSE ("PulseSource::PACork (%i)\n", cork);
	pa_operation_unref (pa_stream_cork (pulse_stream, cork, NULL, this));
}

void
PulseSource::PATrigger ()
{
	LOG_PULSE ("PulseSource::PATrigger (), triggered: %i\n", triggered);
	pa_operation_unref (pa_stream_trigger (pulse_stream, NULL, this));
	triggered = true;
}

void
PulseSource::PAFlush ()
{
	pa_operation_unref (pa_stream_flush (pulse_stream, NULL, this));
}

void
PulseSource::Played ()
{
	LOG_PULSE ("PulseSource::Played ()\n");
	
	if (!InitializePA ()) {
		LOG_PULSE ("PulseSource::Played (): initialization failed.\n");
		return;
	}
	
	player->LockLoop ();
	triggered = false;
	WriteAvailable ();
	if (pulse_stream && is_ready) {
		// Uncork the stream (if it was corked)
		PACork (false);
		// And start playing it.
		PATrigger ();
		play_pending = false;
	} else {
		play_pending = true;
	}
	player->UnlockLoop ();	
}

void
PulseSource::Paused ()
{	
	player->LockLoop ();
	play_pending = false;
	if (pulse_stream && is_ready)
		PACork (true);
	player->UnlockLoop ();
}

void
PulseSource::Stopped ()
{
	LOG_PULSE ("PulseSource::Stopped ()\n");
	
	player->LockLoop ();
	play_pending = false;
	if (pulse_stream && is_ready) {
		// Pause the stream and wait for the pause to complete
		PACork (true);
		// Drop all the samples we've sent
		PAFlush ();
	}
	player->UnlockLoop ();
	
	Close ();
}

guint64
PulseSource::GetDelayInternal ()
{
	int err = 0;
	pa_usec_t latency = 0;
	int negative = 0;
	guint64 result = 0;
	
	player->LockLoop ();
	if (pulse_stream && is_ready) {
		err = pa_stream_get_latency (pulse_stream, &latency, &negative);
		if (err < 0) {
			LOG_AUDIO ("PulseSource::GetDelay (): Error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
			result = G_MAXUINT64;
		} else {
			result = MilliSeconds_ToPts (latency / 1000);
		}		
	} else {
		result = G_MAXUINT64;
	}
	player->UnlockLoop ();
	
	
	LOG_PULSE ("PulseSource::GetDelay (), result: %" G_GUINT64_FORMAT " ms, latency: %" G_GUINT64_FORMAT ", err: %i, negative: %i, is_ready: %i, pulse_stream: %p\n", 
		MilliSeconds_FromPts (result), latency, err, negative, is_ready, pulse_stream);
	
	return result;
}

/*
 * PulsePlayer
 */
 
PulsePlayer::PulsePlayer ()
{
	loop = NULL;
	context = NULL;
	connected = ConnectionUnknown;
	pthread_mutex_init (&mutex, NULL);
	pthread_cond_init (&cond, NULL);
}

PulsePlayer::~PulsePlayer ()
{
	pthread_mutex_destroy (&mutex);
	pthread_cond_destroy (&cond);
}
 
void
PulsePlayer::WaitLoop ()
{
	pa_threaded_mainloop_wait (loop);
}

void
PulsePlayer::WaitForOperation (pa_operation *op)
{
	if (pa_threaded_mainloop_in_thread (loop))
		return;
	
	//while (pa_operation_get_state (op) != PA_OPERATION_DONE)
	//	WaitLoop ();
	pa_operation_unref (op);
}

void
PulsePlayer::SignalLoop ()
{
	//pa_threaded_mainloop_signal (loop, 0);
}
 
void
PulsePlayer::LockLoop ()
{
	if (!pa_threaded_mainloop_in_thread (loop))
		pa_threaded_mainloop_lock (loop);
}

void
PulsePlayer::UnlockLoop ()
{
	if (!pa_threaded_mainloop_in_thread (loop))
		pa_threaded_mainloop_unlock (loop);
}

AudioSource *
PulsePlayer::CreateNode (MediaPlayer *mplayer, AudioStream *stream)
{
	return new PulseSource (this, mplayer, stream);
}

static int is_pulse_usable = 0; // 0 = not tested, 1 = tested, usable, 2 = tested, not usable
static void *libpulse = NULL;

bool
PulsePlayer::IsInstalled ()
{
	bool result = false;
	const char *version;

	switch (is_pulse_usable) {
	case 0:				
		libpulse = dlopen ("libpulse.so.0", RTLD_LAZY);
		if (libpulse == NULL) {
			is_pulse_usable = 2;
			return false;
		}
		result = true;
		
		result &= NULL != (d_pa_stream_new = (dyn_pa_stream_new *) dlsym (libpulse, "pa_stream_new"));
		result &= NULL != (d_pa_stream_set_state_callback = (dyn_pa_stream_set_state_callback *) dlsym (libpulse, "pa_stream_set_state_callback"));
		result &= NULL != (d_pa_stream_set_write_callback = (dyn_pa_stream_set_write_callback *) dlsym (libpulse, "pa_stream_set_write_callback"));
		result &= NULL != (d_pa_stream_set_underflow_callback = (dyn_pa_stream_set_underflow_callback *) dlsym (libpulse, "pa_stream_set_underflow_callback"));
		result &= NULL != (d_pa_stream_connect_playback = (dyn_pa_stream_connect_playback *) dlsym (libpulse, "pa_stream_connect_playback"));
		result &= NULL != (d_pa_stream_disconnect = (dyn_pa_stream_disconnect *) dlsym (libpulse, "pa_stream_disconnect"));
		result &= NULL != (d_pa_stream_unref = (dyn_pa_stream_unref *) dlsym (libpulse, "pa_stream_unref"));
		result &= NULL != (d_pa_stream_get_state = (dyn_pa_stream_get_state *) dlsym (libpulse, "pa_stream_get_state"));
		result &= NULL != (d_pa_stream_write = (dyn_pa_stream_write *) dlsym (libpulse, "pa_stream_write"));
		result &= NULL != (d_pa_stream_writable_size = (dyn_pa_stream_writable_size *) dlsym (libpulse, "pa_stream_writable_size"));
		result &= NULL != (d_pa_stream_cork = (dyn_pa_stream_cork *) dlsym (libpulse, "pa_stream_cork"));
		result &= NULL != (d_pa_stream_trigger = (dyn_pa_stream_trigger *) dlsym (libpulse, "pa_stream_trigger"));
		result &= NULL != (d_pa_stream_flush = (dyn_pa_stream_flush *) dlsym (libpulse, "pa_stream_flush"));
		result &= NULL != (d_pa_stream_get_latency = (dyn_pa_stream_get_latency *) dlsym (libpulse, "pa_stream_get_latency"));

		result &= NULL != (d_pa_context_new = (dyn_pa_context_new *) dlsym (libpulse, "pa_context_new"));
		result &= NULL != (d_pa_context_errno = (dyn_pa_context_errno *) dlsym (libpulse, "pa_context_errno"));
		result &= NULL != (d_pa_context_get_state = (dyn_pa_context_get_state *) dlsym (libpulse, "pa_context_get_state"));
		result &= NULL != (d_pa_context_set_state_callback = (dyn_pa_context_set_state_callback *) dlsym (libpulse, "pa_context_set_state_callback"));
		result &= NULL != (d_pa_context_connect = (dyn_pa_context_connect *) dlsym (libpulse, "pa_context_connect"));
		result &= NULL != (d_pa_context_disconnect = (dyn_pa_context_disconnect *) dlsym (libpulse, "pa_context_disconnect"));
		result &= NULL != (d_pa_context_unref = (dyn_pa_context_unref *) dlsym (libpulse, "pa_context_unref"));

		result &= NULL != (d_pa_threaded_mainloop_new = (dyn_pa_threaded_mainloop_new *) dlsym (libpulse, "pa_threaded_mainloop_new"));
		result &= NULL != (d_pa_threaded_mainloop_start = (dyn_pa_threaded_mainloop_start *) dlsym (libpulse, "pa_threaded_mainloop_start"));
		result &= NULL != (d_pa_threaded_mainloop_get_api = (dyn_pa_threaded_mainloop_get_api *) dlsym (libpulse, "pa_threaded_mainloop_get_api"));
		result &= NULL != (d_pa_threaded_mainloop_wait = (dyn_pa_threaded_mainloop_wait *) dlsym (libpulse, "pa_threaded_mainloop_wait"));
		result &= NULL != (d_pa_threaded_mainloop_in_thread = (dyn_pa_threaded_mainloop_in_thread *) dlsym (libpulse, "pa_threaded_mainloop_in_thread"));
		result &= NULL != (d_pa_threaded_mainloop_lock = (dyn_pa_threaded_mainloop_lock *) dlsym (libpulse, "pa_threaded_mainloop_lock"));
		result &= NULL != (d_pa_threaded_mainloop_unlock = (dyn_pa_threaded_mainloop_unlock *) dlsym (libpulse, "pa_threaded_mainloop_unlock"));
		result &= NULL != (d_pa_threaded_mainloop_signal = (dyn_pa_threaded_mainloop_signal *) dlsym (libpulse, "pa_threaded_mainloop_signal"));
		result &= NULL != (d_pa_threaded_mainloop_stop = (dyn_pa_threaded_mainloop_stop *) dlsym (libpulse, "pa_threaded_mainloop_stop"));
		result &= NULL != (d_pa_threaded_mainloop_free = (dyn_pa_threaded_mainloop_free *) dlsym (libpulse, "pa_threaded_mainloop_free"));
					
		result &= NULL != (d_pa_channel_map_init_mono = (dyn_pa_channel_map_init_mono *) dlsym (libpulse, "pa_channel_map_init_mono"));
		result &= NULL != (d_pa_channel_map_init_stereo = (dyn_pa_channel_map_init_stereo *) dlsym (libpulse, "pa_channel_map_init_stereo"));
		result &= NULL != (d_pa_channel_map_init_auto = (dyn_pa_channel_map_init_auto *) dlsym (libpulse, "pa_channel_map_init_auto"));
		
		result &= NULL != (d_pa_strerror = (dyn_pa_strerror *) dlsym (libpulse, "pa_strerror"));
		
		result &= NULL != (d_pa_operation_get_state = (dyn_pa_operation_get_state *) dlsym (libpulse, "pa_operation_get_state"));
		result &= NULL != (d_pa_operation_unref = (dyn_pa_operation_unref *) dlsym (libpulse, "pa_operation_unref"));
		
		result &= NULL != (d_pa_get_library_version = (dyn_pa_get_library_version *) dlsym (libpulse, "pa_get_library_version"));

		if (d_pa_get_library_version != NULL) {
			version = d_pa_get_library_version ();
			LOG_AUDIO ("PulsePlayer: Found libpulse version: '%s'\n", version);
		}

		if (!result)
			LOG_AUDIO ("PulsePlayer: Failed to load one or more required functions in libpulse.so.\n");
		
		is_pulse_usable = result ? 1 : 2;
		return result;
	case 1:
		return true;
	default:
		return false;
	}
	
	return true;
}

void
PulsePlayer::OnContextStateChanged (pa_context *context, void *userdata)
{
	((PulsePlayer *) userdata)->OnContextStateChanged ();	
}

pa_context_state_t
PulsePlayer::GetPAState ()
{
	pa_context_state_t result;
	
	LockLoop ();
	result = pa_context_get_state (context);
	UnlockLoop ();
	
	return result;
}

#ifdef LOGGING
static const char *
get_pa_context_state_name (pa_context_state_t state)
{
	switch (state) {
	case PA_CONTEXT_CONNECTING: return "PA_CONTEXT_CONNECTING";
	case PA_CONTEXT_AUTHORIZING: return "PA_CONTEXT_AUTHORIZING";
	case PA_CONTEXT_SETTING_NAME: return "PA_CONTEXT_SETTING_NAME";
	case PA_CONTEXT_READY: return "PA_CONTEXT_READY";
	case PA_CONTEXT_TERMINATED: return "PA_CONTEXT_TERMINATED";
	case PA_CONTEXT_FAILED: return "PA_CONTEXT_FAILED";
	default: return "<UNKNOWN>";
	}
}
#endif

void
PulsePlayer::OnContextStateChanged () {
	PulseSource *source;
	pa_context_state_t state;
	
	state = GetPAState ();
	
	LOG_PULSE ("PulsePlayer::OnContextStateChanged (): %s (%i)\n", get_pa_context_state_name (state), state);
	
	switch (state) {
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
		break;
	case PA_CONTEXT_READY:
		LockLoop ();
		sources.StartEnumeration ();
		while ((source = (PulseSource *) sources.GetNext (false)) != NULL) {
			source->Initialize ();
			source->unref ();
		}
		UnlockLoop ();
		pthread_mutex_lock (&mutex);
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): Signalling main thread that we've connected\n");
		connected = ConnectionSuccess;
		pthread_cond_signal (&cond);
		pthread_mutex_unlock (&mutex);
		break;
	case PA_CONTEXT_TERMINATED:
		break;
	case PA_CONTEXT_FAILED:
	default:
		pthread_mutex_lock (&mutex);
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): Signalling main thread that we've failed to connect\n");
		connected = ConnectionFailed;
		pthread_cond_signal (&cond);
		pthread_mutex_unlock (&mutex);
		fprintf (stderr, "Moonlight: Connection failure while trying to connect to pulseaudio daemon: %s\n", pa_strerror (pa_context_errno (context)));
		break;
	}
}

void
PulsePlayer::AddInternal (AudioSource *source)
{
	LOG_PULSE ("PulsePlayer::AddInternal (%p)\n", source);
	
	((PulseSource *) source)->Initialize ();
}

void
PulsePlayer::RemoveInternal (AudioSource *source)
{
	LOG_PULSE ("PulsePlayer::RemoveInternal (%p)\n", source);
	
	((PulseSource *) source)->Close ();
}


bool
PulsePlayer::Initialize ()
{
	int err;
	
	LOG_PULSE ("PulsePlayer::InitializeInternal ()\n");
	
	loop = pa_threaded_mainloop_new ();	
	if (loop == NULL) {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): Failed to create main loop.\n");
		return false;
	}
	
	api = pa_threaded_mainloop_get_api (loop);
	
	if (api == NULL) {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): Failed to get api.\n");
		return false;
	}
	
	context = pa_context_new (api, "Moonlight");
	if (context == NULL) {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (); Failed to create context.\n");
		return false;
	}
	
	pa_context_set_state_callback (context, OnContextStateChanged, this);
	
	err = pa_context_connect (context, NULL, (pa_context_flags_t) 0, NULL);
	if (err < 0) {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): Error %i while connecting to server.\n", err);
		return false;
	} 
	if (connected == ConnectionUnknown) {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): pa_context_connect returned but we're not connected.\n");

		// It's possible that pulse can return an error to us async
		// We need to aquire a lock, then start the mainloop
		pthread_mutex_lock (&mutex);
		
		// Of course it wont raise the async error unless we try
		// to start the mainloop
		pa_threaded_mainloop_start (loop);

		do {
			// Wait until pulse has reported the connection status to
			// us async.
			LOG_AUDIO ("PulsePlayer::InitializeInternal (): Waiting to see if we can connect.\n");
			pthread_cond_wait (&cond, &mutex);
		} while (connected == ConnectionUnknown);

		pthread_mutex_unlock (&mutex);

		// At this stag we have had connected set regardless of wether
		// PA wants to be sync or async
		if (connected == ConnectionFailed) {
			LOG_AUDIO ("PulsePlayer::InitializeInternal (): Asynchronous error while connecting to the pulse daemon\n");
			return false;
		}
	} else {
		LOG_AUDIO ("PulsePlayer::InitializeInternal (): pa_context_connect returned and connected.\n");
		// We've already connected successfully in a sync fashion
		// there is no need to lock, we can just start the loop
		pa_threaded_mainloop_start (loop);
	}
	
	return true;
}

void
PulsePlayer::PrepareShutdownInternal ()
{
}

void
PulsePlayer::FinishShutdownInternal ()
{
	LOG_PULSE ("PulsePlayer::ShutdownInternal ()\n");
	
	api = NULL; // CHECK: Do we need to unref this one?
	
	if (context) {
		pa_context_disconnect (context);
		pa_context_unref (context);	
		context = NULL;
	}
	
	if (loop) {
		pa_threaded_mainloop_stop (loop);
		pa_threaded_mainloop_free (loop);
		loop = NULL;
	}
}
