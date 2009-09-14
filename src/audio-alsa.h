/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-alsa.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#if INCLUDE_ALSA

#ifndef __AUDIO_ALSA_H__
#define __AUDIO_ALSA_H__

#include <pthread.h>
#include <poll.h>
#include <asoundlib.h>

#include "audio.h"
#include "mutex.h"

class AlsaPlayer;

class AlsaSource : public AudioSource {
	AlsaPlayer *player;
	snd_pcm_t *pcm;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;
	
	Mutex mutex;
	
	bool initialized;
	bool mmap;
	bool started;
	bool drop_pending;

	bool SetupHW ();
	bool PreparePcm (snd_pcm_sframes_t *avail);
	
	// Underrun recovery
	// Handles EPIPE and ESTRPIPE, prints an error if recovery failed
	// or if err isn't any of the above values.
	bool XrunRecovery (int err);
	
	bool WriteRW ();
	bool WriteMmap ();

	void Drain ();

	bool InitializeAlsa ();
	void CloseAlsa ();

 protected:
	virtual ~AlsaSource ();

	virtual void Played ();
	virtual void Paused ();
	virtual void Stopped ();
	virtual void StateChanged (AudioState old_state);
	virtual guint64 GetDelayInternal ();

	virtual bool InitializeInternal ();
	virtual void CloseInternal ();
	
 public:
	pollfd *udfs;
	int ndfs;
	
	AlsaSource (AlsaPlayer *player, MediaPlayer *mplayer, AudioStream *stream);
		
	// Pushes data onto the pcm device if the
	// device can accept more data, and if the
	// there is data availabe. The node has to 
	// be locked during playback.
	// Returns false if nothing has been played.
	bool WriteAlsa ();
	
	bool IsDropPending () { return drop_pending; }
	void DropAlsa ();
};

class AlsaPlayer : public AudioPlayer {
	// The audio thread	
	pthread_t *audio_thread;
	bool shutdown; // set to true to exit the audio thread.
	
	// A list of all the file descriptors in all the 
	// audio nodes. We need to poll on changes in any of the 
	// descriptors, so we create a big list with all of them
	// and poll on that.
	pollfd *udfs;
	int ndfs;
	
	// We also need to be able to wake up from the poll
	// whenever we want to, so we create a pipe which we
	// poll on. This is always the first file descriptor
	// in udfs.
	int fds [2];
	
	// If UpdatePollList must be called before polling.
	bool update_poll_pending;
		
	// The audio loop which is executed 
	// on the audio thread.
	void Loop ();
	static void *Loop (void *data);
	
	void WakeUp (); // Wakes up the audio thread.
	
 protected:				
	virtual ~AlsaPlayer ();

	virtual void AddInternal (AudioSource *node);
	virtual void RemoveInternal (AudioSource *node);
	virtual void PrepareShutdownInternal ();
	virtual void FinishShutdownInternal ();
	virtual bool Initialize ();
	virtual AudioSource *CreateNode (MediaPlayer *mplayer, AudioStream *stream);
	
 public:
	AlsaPlayer ();

	void UpdatePollList ();
	
	static bool IsInstalled ();
};

#endif /* __AUDI_ALSA_H__ */

#endif /* INCLUDE_ALSA */
