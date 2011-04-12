/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-opensles.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#if INCLUDE_OPENSLES

#ifndef __AUDIO_OPENSLES_H__
#define __AUDIO_OPENSLES_H__

#include "SLES/OpenSLES.h"
// FIXME this needs some ifdef love
#include "SLES/OpenSLES_Android.h"

#include "audio.h"

namespace Moonlight {

class OpenSLESPlayer;

class OpenSLESSource : public AudioSource {
	OpenSLESPlayer *player;

	SLObjectItf playerObject;
	SLPlayItf playerPlay;
	SLAndroidSimpleBufferQueueItf playerBufferQueue;

	gpointer* buffers;

	bool is_ready;
	bool initialized;
	bool play_pending;

	bool InitializeOpenSLES ();
	void CloseOpenSLES ();

	void NextBuffer ();
	static void NextBufferCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

protected:
	virtual ~OpenSLESSource ();

	virtual void Played ();
	virtual void Paused ();
	virtual void Stopped ();
	virtual void StateChanged (AudioState old_state);
	virtual guint64 GetDelayInternal ();

	virtual bool InitializeInternal ();
	virtual void CloseInternal ();
	
public:
	/* @SkipFactories */
	OpenSLESSource (OpenSLESPlayer *player, MediaPlayer *mplayer, AudioStream *stream);
};

class OpenSLESPlayer : public AudioPlayer {
	friend class OpenSLESSource;

	SLObjectItf engineObject;
	SLEngineItf engineEngine;

	SLObjectItf outputMixObject;

protected:
	virtual ~OpenSLESPlayer ();

	virtual void AddInternal (AudioSource *source);
	virtual void RemoveInternal (AudioSource *source);
	virtual void PrepareShutdownInternal ();
	virtual void FinishShutdownInternal ();
	virtual bool Initialize ();
	virtual AudioSource *CreateNode (MediaPlayer *mplayer, AudioStream *stream);
public:
	OpenSLESPlayer ();
};

};

#endif /* __AUDIO_OPENSLES_H__ */

#endif /* INCLUDE_OPENSLES */
