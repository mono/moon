/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio-opensles.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "config.h"

#if INCLUDE_OPENSLES

#define LOG_OSL(...) g_debug (__VA_ARGS__)

#include "audio-opensles.h"

#define CHECK_RESULT(action) do { \
	if (result != SL_RESULT_SUCCESS) { \
		g_warning ("%s failed: %s", action, SLResultToString(result)); \
		return; \
	} \
} while (0)

#define CHECK_RESULT_BOOL(action) do { \
	if (result != SL_RESULT_SUCCESS) { \
		g_warning ("%s failed: %s", action, SLResultToString(result)); \
		return false; \
	} \
} while (0)

namespace Moonlight {

static const char*
SLResultToString (SLresult result)
{
	switch (result) {
#define SLRESULTCASE(x) case x: return #x
	  SLRESULTCASE(SL_RESULT_SUCCESS);
	  SLRESULTCASE(SL_RESULT_PRECONDITIONS_VIOLATED);
	  SLRESULTCASE(SL_RESULT_PARAMETER_INVALID);
	  SLRESULTCASE(SL_RESULT_MEMORY_FAILURE);
	  SLRESULTCASE(SL_RESULT_RESOURCE_ERROR);
	  SLRESULTCASE(SL_RESULT_RESOURCE_LOST);
	  SLRESULTCASE(SL_RESULT_IO_ERROR);
	  SLRESULTCASE(SL_RESULT_BUFFER_INSUFFICIENT);
	  SLRESULTCASE(SL_RESULT_CONTENT_CORRUPTED);
	  SLRESULTCASE(SL_RESULT_CONTENT_UNSUPPORTED);
	  SLRESULTCASE(SL_RESULT_CONTENT_NOT_FOUND);
	  SLRESULTCASE(SL_RESULT_PERMISSION_DENIED);
	  SLRESULTCASE(SL_RESULT_FEATURE_UNSUPPORTED);
	  SLRESULTCASE(SL_RESULT_INTERNAL_ERROR);
	  SLRESULTCASE(SL_RESULT_UNKNOWN_ERROR);
	  SLRESULTCASE(SL_RESULT_OPERATION_ABORTED);
	  SLRESULTCASE(SL_RESULT_CONTROL_LOST);
#undef SLRESULTCASE
	default:
		return "UNKNOWN";
	}
}


OpenSLESSource::OpenSLESSource (OpenSLESPlayer *player, MediaPlayer *mplayer, AudioStream *stream)
	: AudioSource (Type::OPENSLESSOURCE, player, mplayer, stream)
{
	LOG_OSL ("OpenSLESSource::OpenSLESSource (%p, %p)\n", player, stream);
	
	this->player = player;

	playerObject = NULL;
	playerPlay = NULL;
	playerBufferQueue = NULL;

	initialized = false;
	is_ready = false;
	play_pending = false;
}

OpenSLESSource::~OpenSLESSource ()
{
	LOG_OSL ("OpenSLESSource::~OpenSLESSource ()\n");

	Close ();
}

bool
OpenSLESSource::InitializeInternal ()
{
	LOG_AUDIO ("OpenSLESSource::InitializeInternal ()\n");
	// this is a no-op, initialization is done when needed.
	return true;
}

// this callback handler is called every time a buffer finishes playing
void
OpenSLESSource::NextBufferCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	((OpenSLESSource*)context)->NextBuffer ();
}

void
OpenSLESSource::NextBuffer ()
{
	LOG_OSL ("OpenSLESSource::PlayBuffer");

	// FIXME: allocate new buffer (or get it from a free list or something..)
	void *pBuffer = NULL;
	SLuint32 bufferSize = 0;
	SLresult result;

	Write (pBuffer, bufferSize);

        result = (*playerBufferQueue)->Enqueue(playerBufferQueue, pBuffer, bufferSize);
	CHECK_RESULT("playerBufferQueue->Enqueue");
#if notyet
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
#endif
}

bool
OpenSLESSource::InitializeOpenSLES ()
{
	bool res = false;

	LOG_OSL ("OpenSLESSource::InitializeOpenSLES (%p) initialized: %i\n", this, initialized);

	SLresult result;

	

	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2/*numbuffers*/};

	SLDataFormat_PCM format_pcm = {
	  SL_DATAFORMAT_PCM, GetAudioStream()->GetChannels(),
	  SL_SAMPLINGRATE_8 /* FIXME convert from audioStream->GetSampleRate() */,
	  SL_PCMSAMPLEFORMAT_FIXED_16 /* FIXME convert from audioStream->GetBitsPerSample() */,
	  SL_PCMSAMPLEFORMAT_FIXED_16 /* FIXME containerSize - not sure what to do here.. */,
	  SL_SPEAKER_FRONT_CENTER,
	  SL_BYTEORDER_LITTLEENDIAN};

	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, player->outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	// create audio player
	const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = (*player->engineEngine)->CreateAudioPlayer(player->engineEngine,
							    &playerObject,
							    &audioSrc, &audioSnk,
							    1, ids, req);
	CHECK_RESULT_BOOL ("engineEngine->CreateAudioPlayer");

	// realize the player
	result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
	CHECK_RESULT_BOOL ("playerObject->CreateAudioPlayer");

	// get the play interface
	result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
	CHECK_RESULT_BOOL ("playerObject->GetInterface (SL_IID_PLAY)");

	// get the buffer queue interface
	result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
	CHECK_RESULT_BOOL ("playerObject->GetInterface(SL_IID_BUFFERQUEUE)");

	// register callback on the buffer queue
	result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, OpenSLESSource::NextBufferCallback, this);
	CHECK_RESULT_BOOL ("playerBufferQueue->RegisterCallback()");

	// set the player's state to playing
	// FIXME do we do this here?
	result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
	CHECK_RESULT_BOOL ("playerPlay->SetPlayState (SL_PLAYSTATE_PLAYING)");

	return true;
}

void
OpenSLESSource::CloseInternal ()
{
	CloseOpenSLES ();
}

void
OpenSLESSource::CloseOpenSLES ()
{
	LOG_OSL ("OpenSLESSource::CloseOpenSLES");
}

void
OpenSLESSource::StateChanged (AudioState old_state)
{
	if (GetState () == AudioPlaying)
		InitializeOpenSLES ();
	//	player->UpdatePollList ();
}

void
OpenSLESSource::Played ()
{
	LOG_OSL ("OpenGLESSource::Played ()\n");

	if (InitializeOpenSLES ()) {
		LOG_OSL ("OpenGLESSource::Player (): initialization failed.\n");
		return;
	}

	//	player->UpdatePollList ();
}

void
OpenSLESSource::Paused ()
{
	// FIXME we need to drop stop queueing new buffers
}

void
OpenSLESSource::Stopped ()
{
	// FIXME we need to drop stop queueing new buffers, and tell opensles to drop all samples we have already enqueued
	Close ();
}


guint64
OpenSLESSource::GetDelayInternal ()
{
	LOG_OSL ("OpenSLESSource::GetDelayInternal (): not implemented\n");
	return G_MAXUINT64;
}

/*
 * OpenSLESPlayer
 */


OpenSLESPlayer::OpenSLESPlayer ()
{
	engineObject = NULL;
	engineEngine = NULL;

	outputMixObject = NULL;
}

OpenSLESPlayer::~OpenSLESPlayer ()
{
}

void
OpenSLESPlayer::AddInternal (AudioSource *source)
{
	LOG_OSL ("OpenSLESPlayer::AddInternal (%p)\n", source);
	
	((OpenSLESSource *) source)->Initialize ();
}

void
OpenSLESPlayer::RemoveInternal (AudioSource *source)
{
	LOG_OSL ("OpenSLESPlayer::RemoveInternal (%p)\n", source);
	
	((OpenSLESSource *) source)->Close ();
}

void
OpenSLESPlayer::PrepareShutdownInternal ()
{
}

void
OpenSLESPlayer::FinishShutdownInternal ()
{
}

bool
OpenSLESPlayer::Initialize ()
{
	SLresult result;
	SLuint32 numSupportedInterfaces;

	LOG_OSL ("OpenSLESPlayer::Initialize ()");


	slQueryNumSupportedEngineInterfaces (&numSupportedInterfaces);

	LOG_OSL ("  %d  supported engine interfaces:", numSupportedInterfaces);

#if notyet
	for (int i = 0; i < numSupportedInterfaces; i ++) {
		SLInterfaceID interfaceId;
		slQuerySupportedEngineInterfaces (i, &interfaceId);
		LOG_OSL ("      %d: ", i);
	}
#endif

	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	CHECK_RESULT_BOOL ("slCreateEngine");

	result = (*engineObject)->Realize (engineObject, SL_BOOLEAN_FALSE);
	CHECK_RESULT_BOOL ("engine->Realize");

	result = (*engineObject)->GetInterface (engineObject, SL_IID_ENGINE, &engineEngine);
	CHECK_RESULT_BOOL ("engine->GetInterface (SL_IID_ENGINE)");

	result = (*engineEngine)->CreateOutputMix (engineEngine, &outputMixObject, 0, NULL, NULL);
	CHECK_RESULT_BOOL ("engine->CreateOutputMix");

	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	CHECK_RESULT_BOOL ("outputMixObject->Realize");

	return true;
}

AudioSource*
OpenSLESPlayer::CreateNode (MediaPlayer *mplayer, AudioStream *stream)
{
	return new OpenSLESSource (this, mplayer, stream);
}

};

#endif

