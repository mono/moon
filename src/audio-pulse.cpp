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

#define LOG_PULSE(...)// printf (__VA_ARGS__);
// This one prints out spew on every sample
#define LOG_PULSE_EX(...)// printf (__VA_ARGS__);

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
	
	if (initialized)
		return true;
		
	if (player->GetPAState () != PA_CONTEXT_READY)
		return true;
		
	initialized = true;
	
	if (!InitializePA ()) {
		SetState (AudioError);
		return false;
	}
	
	return true;
}

bool
PulseSource::InitializePA ()
{
	int err;
	pa_sample_spec format;
	pa_channel_map channel_map;
	bool result = false;
	
	AUDIO_DEBUG ("PulseSource::InitializePA ()\n");
	
	player->LockLoop ();
	
	format.format = PA_SAMPLE_S16NE;
	format.rate = GetSampleRate ();
	format.channels = GetChannels ();
	
	if (format.channels == 1) {
		pa_channel_map_init_mono (&channel_map);
	} else if (format.channels == 2) {
		pa_channel_map_init_stereo (&channel_map);
	} else {
		AUDIO_DEBUG ("PulseSource::InitializePA (): Invalid number of channels: %i\n", format.channels);
		goto cleanup;
	}
	
	pulse_stream = pa_stream_new (player->GetPAContext (), "Audio stream", &format, &channel_map);
	if (pulse_stream == NULL) {
		AUDIO_DEBUG ("PulseSource::InitializePA (): Stream creation failed: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		goto cleanup;
	}
			
	pa_stream_set_state_callback (pulse_stream, OnStateChanged, this);
	pa_stream_set_write_callback (pulse_stream, OnWrite, this);
	pa_stream_set_underflow_callback (pulse_stream, OnUnderflow, this);
	
	err = pa_stream_connect_playback (pulse_stream, NULL, NULL, (pa_stream_flags_t) (PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_START_CORKED), NULL, NULL);
	if (err < 0) {
		AUDIO_DEBUG ("PulseSource::InitializePA (): failed to connect stream: %s.\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		goto cleanup;
	}
	
	result = true;
	
cleanup:
	player->UnlockLoop ();
								
	return result;
}

void
PulseSource::CloseInternal ()
{
	LOG_PULSE ("PulseSource::CloseInternal ()\n");
	
	is_ready = false;
	
	player->LockLoop ();
	if (pulse_stream) {
		pa_stream_disconnect (pulse_stream);
		pa_stream_unref (pulse_stream);
		pulse_stream = NULL;
	}
	player->UnlockLoop ();
}

void
PulseSource::OnStateChanged (pa_stream *pulse_stream, void *userdata)
{
	((PulseSource *) userdata)->OnStateChanged (pulse_stream);
}

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
		AUDIO_DEBUG ("PulseSource::OnStateChanged (%p): Invalid stream.\n", pulse_stream);
		return;
	}
	
	state = GetPAState (pulse_stream);
	
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
		AUDIO_DEBUG ("PulseSource::OnStateChanged (): Stream error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
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
	
	LOG_PULSE ("PulseSource::OnWrite (%lld)\n", (gint64) length);
	
	if (pulse_stream == NULL) {
		// We've been destroyed
		return;
	}
	
	if (length == 0)
		return;
	
	buffer = g_malloc (length);
	
	frames = Write (buffer, length / GetBytesPerFrame ());
	
	LOG_PULSE ("PulseSource::OnWrite (%lld): Wrote %llu frames\n", (gint64) length, (gint64) frames);	
	
	if (frames > 0) {
		// There is no need to lock here, if in a callback, the caller will have locked
		// if called from WriteAvailable, that method has locked
		err = pa_stream_write (pulse_stream, buffer, frames * GetBytesPerFrame (), (pa_free_cb_t) g_free, 0, PA_SEEK_RELATIVE);
		if (err < 0) {
			AUDIO_DEBUG ("PulseSource::OnWrite (): Write error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		} else if (play_pending) {
			Played ();
		}
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
			AUDIO_DEBUG ("PulseSource::WriteAvailable (): Write error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
		}
	}
	player->UnlockLoop ();
}

void
PulseSource::PACork (bool cork)
{
	LOG_PULSE ("PulseSource::PACork (%i)\n", cork);
	player->WaitForOperation (pa_stream_cork (pulse_stream, cork, OnCorked, this));
}

void
PulseSource::PATrigger ()
{
	LOG_PULSE ("PulseSource::PATrigger (), triggered: %i\n", triggered);
	player->WaitForOperation (pa_stream_trigger (pulse_stream, OnTriggered, this));	
	triggered = true;
}

void
PulseSource::PAFlush ()
{
	player->WaitForOperation (pa_stream_flush (pulse_stream, OnFlushed, this));
}

void
PulseSource::OnCorked (pa_stream *pulse_stream, int successs, void *userdata)
{
	((PulseSource *) userdata)->player->SignalLoop ();
}

void
PulseSource::OnFlushed (pa_stream *pulse_stream, int success, void *userdata)
{
	((PulseSource *) userdata)->player->SignalLoop ();
}

void
PulseSource::OnTriggered (pa_stream *pulse_stream, int success, void *userdata)
{
	((PulseSource *) userdata)->player->SignalLoop ();
}

void
PulseSource::OnDrained (pa_stream *pulse_stream, int success, void *userdata)
{
	((PulseSource *) userdata)->player->SignalLoop ();
}

void
PulseSource::Played ()
{
	LOG_PULSE ("PulseSource::Played ()\n");
	
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
			AUDIO_DEBUG ("PulseSource::GetDelay (): Error: %s\n", pa_strerror (pa_context_errno (player->GetPAContext ())));
			result = G_MAXUINT64;
		} else {
			result = MilliSeconds_ToPts (latency / 1000);
		}		
	} else {
		result = G_MAXUINT64;
	}
	player->UnlockLoop ();
	
	
	LOG_PULSE ("PulseSource::GetDelay (), result: %llu ms, latency: %llu, err: %i, negative: %i, is_ready: %i, pulse_stream: %p\n", 
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
}

PulsePlayer::~PulsePlayer ()
{
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
typedef const char *PA_get_library_version ();

bool
PulsePlayer::IsInstalled ()
{
	bool result = false;
	void *libpulse;
	PA_get_library_version *get_version;
	const char *version;
	
	switch (is_pulse_usable) {
	case 0:				
		libpulse = dlopen ("libpulse.so.0", RTLD_LAZY);
		if (libpulse != NULL) {
			get_version = (PA_get_library_version *) dlsym (libpulse, "pa_get_library_version");
			if (get_version != NULL) {
				version = get_version ();
				AUDIO_DEBUG ("PulsePlayer: Found libpulse version: '%s'\n", version);
				result = true;
			}
			dlclose (libpulse);
		}
		
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
		}
		UnlockLoop ();
		break;
	case PA_CONTEXT_TERMINATED:
		break;
	case PA_CONTEXT_FAILED:
	default:
		AUDIO_DEBUG ("Connection failure: %s\n", pa_strerror (pa_context_errno (context)));
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
		AUDIO_DEBUG ("PulsePlayer::InitializeInternal (): Failed to create main loop.\n");
		return false;
	}
	
	api = pa_threaded_mainloop_get_api (loop);
	
	if (api == NULL) {
		AUDIO_DEBUG ("PulsePlayer::InitializeInternal (): Failed to get api.\n");
		return false;
	}
	
	context = pa_context_new (api, "Moonlight");
	if (context == NULL) {
		AUDIO_DEBUG ("PulsePlayer::InitializeInternal (); Failed to create context.\n");
		return false;
	}
	
	pa_context_set_state_callback (context, OnContextStateChanged, this);
	
	err = pa_context_connect (context, NULL, (pa_context_flags_t) 0, NULL);
	if (err < 0) {
		AUDIO_DEBUG ("PulsePlayer::InitializeInternal (): Error %i while connecting to server.\n", err);
		return false;
	}
	
	pa_threaded_mainloop_start (loop);
	
	return true;
}

void
PulsePlayer::ShutdownInternal ()
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
