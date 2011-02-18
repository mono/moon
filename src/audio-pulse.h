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

namespace Moonlight {

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
	/* @SkipFactories */
	PulseSource (PulsePlayer *player, MediaPlayer *mplayer, AudioStream *stream);
};

class PulseRecorder : public AudioRecorder {
public:
	class SourceNode;

private:
	struct ReadData : public List::Node {
		guint8 *data;
		gsize nbytes;
		virtual ~ReadData ()
		{
			g_free (data);
		}
	};
	PulsePlayer *player;
	pa_stream *pulse_stream;
	bool initialized;
	bool is_ready;
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;
	char *name;
	char *description;
	pa_sample_spec default_spec;
	pa_sample_spec recording_spec;
	AudioFormat *recording_format;
	guint64 position;
	CaptureSource *capture_source;
	Mutex capture_mutex;

	bool InitializePA ();

	void OnStateChanged (pa_stream *stream);
	void OnRead (size_t length);

	static void OnRead (pa_stream *pulse_stream, size_t length, void *userdata);
	static void OnStateChanged (pa_stream *pulse_stream, void *userdata);

	pa_stream_state_t GetPAState (pa_stream *pulse_stream = NULL);
	static AudioFormat SampleSpecToAudioFormat (const pa_sample_spec *spec);

public:
	/* @SkipFactories */
	PulseRecorder (PulsePlayer *player, SourceNode *info);
	virtual ~PulseRecorder ();
	virtual void Dispose ();
	virtual void Record ();
	virtual void Stop ();
	virtual void GetSupportedFormats (AudioFormatCollection *col);
	virtual const char *GetFriendlyName () { return description; }

	struct SourceNode : public List::Node {
		char *name;
		char *description;
		pa_sample_spec sample_spec;
		bool is_monitor;
		SourceNode (const pa_source_info *i)
		{
			this->name = g_strdup (i->name);
			this->description = g_strdup (i->description);
			this->sample_spec = i->sample_spec;
			this->is_monitor = i->monitor_of_sink != PA_INVALID_INDEX;
		}
		virtual ~SourceNode ()
		{
			g_free (name);
			g_free (description);
		}
	};
};

class PulsePlayer : public AudioPlayer {
 private:
 	enum ConnectedState {
	 	ConnectionUnknown,
	 	ConnectionFailed,
	 	ConnectionSuccess
	};

	pa_context *context;
	pa_threaded_mainloop *loop;
	pa_mainloop_api *api;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	
	pthread_cond_t recording_cond;
	pthread_mutex_t recording_mutex;
	bool fetching_recording_devices;
	List recording_devices;

	ConnectedState connected; // 0 = don't know, 1 = failed to connect, 2 = connected
	
	static void OnContextStateChanged (pa_context *context, void *userdata);
	static void OnSourceInfoCallback (pa_context *context, const pa_source_info *i, int eol, void *userdata);
	void OnContextStateChanged ();
	void OnSourceInfo (const pa_source_info *i, int eol);
	
 protected:
	virtual void AddInternal (AudioSource *node);
	virtual void RemoveInternal (AudioSource *node);
	virtual void PrepareShutdownInternal ();
	virtual void FinishShutdownInternal ();
	virtual bool Initialize ();
	virtual AudioSource *CreateNode (MediaPlayer *mplayer, AudioStream *stream);
	virtual guint32 CreateRecordersInternal (AudioRecorder **recorders, guint32 size);
	
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

};

#endif /* __AUDIO_PULSE_H__ */

#endif /* INCLUDE_PULSE */
