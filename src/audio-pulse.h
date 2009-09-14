/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-pulse.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#if INCLUDE_PULSEAUDIO

#ifndef __AUDIO_PULSE_H__
#define __AUDIO_PULSE_H__

#include <pulse/pulseaudio.h>

#include "audio.h"

class PulsePlayer;

class PulseSource: public AudioSource {
	PulsePlayer *player;
	pa_stream *pulse_stream;
	bool triggered;
	bool is_ready;
	bool initialized;
	bool play_pending;
	
	void PACork (bool cork);
	void PATrigger ();
	void PAFlush ();
	void OnUnderflow ();
	void OnStateChanged (pa_stream *stream);
	void OnWrite (size_t length);	
	
	static void OnUnderflow (pa_stream *pulse_stream, void *userdata);
	static void OnWrite (pa_stream *pulse_stream, size_t length, void *userdata);
	static void OnStateChanged (pa_stream *pulse_stream, void *userdata);
	
	bool InitializePA ();
	void ClosePA ();
	void WriteAvailable ();
	
	pa_stream_state_t GetPAState (pa_stream *pulse_stream = NULL);
	
 protected:
	virtual ~PulseSource ();

	virtual void Played ();
	virtual void Paused ();
	virtual void Stopped ();
	virtual void StateChanged (AudioState old_state);
	virtual guint64 GetDelayInternal ();
	virtual bool InitializeInternal ();
	virtual void CloseInternal ();

 public:
	PulseSource (PulsePlayer *player, MediaPlayer *mplayer, AudioStream *stream);
};

class PulsePlayer : public AudioPlayer {
 private:
 	enum ConnectedState {
	 	ConnectionUnknown,
	 	ConnectionFailed,
	 	ConnectionSuccess
	};
 private:
	pa_context *context;
	pa_threaded_mainloop *loop;
	pa_mainloop_api *api;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	
	ConnectedState connected; // 0 = don't know, 1 = failed to connect, 2 = connected
	
	static void OnContextStateChanged (pa_context *context, void *userdata);
	void OnContextStateChanged ();
	
 protected:
	virtual void AddInternal (AudioSource *node);
	virtual void RemoveInternal (AudioSource *node);
	virtual void PrepareShutdownInternal ();
	virtual void FinishShutdownInternal ();
	virtual bool Initialize ();
	virtual AudioSource *CreateNode (MediaPlayer *mplayer, AudioStream *stream);
	
 public:
	PulsePlayer ();
	virtual ~PulsePlayer ();
	
	pa_context *GetPAContext () { return context; }
	pa_threaded_mainloop *GetPALoop () { return loop; }
	pa_mainloop_api *GetPAApi () { return api; }
	pa_context_state_t GetPAState ();
	
	void LockLoop ();
	void UnlockLoop ();
	void WaitLoop ();
	void WaitForOperation (pa_operation *op);
	void SignalLoop ();
	
	static bool IsInstalled ();
};

#endif /* __AUDIO_PULSE_H__ */

#endif /* INCLUDE_PULSE */
