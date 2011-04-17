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

#define BUFFER_SIZE 8192
#define NUM_BUFFERS_IN_QUEUE 10

#if TOSHOK_HONEYCOMB_HACK
extern "C" {
extern SLAPIENTRY const SLInterfaceID SL_IID_ANDROIDBUFFERQUEUE;
extern SLAPIENTRY const SLInterfaceID XA_IID_ENGINE;
extern SLAPIENTRY const SLInterfaceID XA_IID_PLAY;
extern SLAPIENTRY const SLInterfaceID XA_IID_ANDROIDBUFFERQUEUE;
};
#endif

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
	SLresult result;

	SLAndroidSimpleBufferQueueState bufferQueueState;
	result = (*playerBufferQueue)->GetState (playerBufferQueue, &bufferQueueState);
	CHECK_RESULT ("playerBufferQueue->GetState");

	void *pBuffer = buffers[(bufferQueueState.index+1) % NUM_BUFFERS_IN_QUEUE];
	int bufferSize = BUFFER_SIZE;

        Write (pBuffer, bufferSize / GetOutputBytesPerFrame ());

        result = (*playerBufferQueue)->Enqueue(playerBufferQueue, pBuffer, bufferSize);
	CHECK_RESULT("playerBufferQueue->Enqueue");
}

bool
OpenSLESSource::InitializeOpenSLES ()
{
	bool res = false;

	LOG_OSL ("OpenSLESSource::InitializeOpenSLES (%p) initialized: %i\n", this, initialized);

	if (initialized)
		return true;

	SLresult result;

	buffers = (gpointer*)g_malloc(sizeof (gpointer) * NUM_BUFFERS_IN_QUEUE);
	for (int i = 0; i < NUM_BUFFERS_IN_QUEUE; i ++)
	  buffers[i] = g_malloc (BUFFER_SIZE);

	g_debug ("channel count = %d\n", GetAudioStream()->GetChannels());

	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NUM_BUFFERS_IN_QUEUE};

	SLDataFormat_PCM format_pcm = {
	  SL_DATAFORMAT_PCM, GetAudioStream()->GetChannels(),
	  SL_SAMPLINGRATE_44_1 /* FIXME convert from audioStream->GetSampleRate() */,
	  SL_PCMSAMPLEFORMAT_FIXED_8 /* FIXME convert from audioStream->GetBitsPerSample() */,
	  SL_PCMSAMPLEFORMAT_FIXED_8 /* FIXME containerSize - not sure what to do here.. */,
	  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
	  SL_BYTEORDER_LITTLEENDIAN};

	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, player->outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	// create audio player
	const SLInterfaceID ids[1] = {
#if TOSHOK_HONEYCOMB_HACK
		SL_IID_ANDROIDBUFFERQUEUE
#else
		SL_IID_ANDROIDSIMPLEBUFFERQUEUE
#endif
	};
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
	result = (*playerObject)->GetInterface(playerObject,
#if TOSHOK_HONEYCOMB_HACK
					       SL_IID_ANDROIDBUFFERQUEUE,
#else
					       SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
#endif
					       &playerBufferQueue);
	CHECK_RESULT_BOOL ("playerObject->GetInterface(SL_IID_ANDROIDBUFFERQUEUE)");

	g_debug ("1");
	// register callback on the buffer queue
	result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, OpenSLESSource::NextBufferCallback, this);
	g_debug ("2");
	CHECK_RESULT_BOOL ("playerBufferQueue->RegisterCallback()");

	// set the player's state to playing
	// FIXME do we do this here?
	result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
	CHECK_RESULT_BOOL ("playerPlay->SetPlayState (SL_PLAYSTATE_PLAYING)");

	initialized = true;

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
	return 0; // let's be optimistic
// 	LOG_OSL ("OpenSLESSource::GetDelayInternal (): not implemented\n");
// 	return G_MAXUINT64;
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

static char *
unknownInterfaceToString (SLInterfaceID interfaceId)
{
	static char buf[1024];
	snprintf (buf, sizeof (buf), "%08X-%04X-%04X-%04X-%02X-%02X-%02X-%02X-%02X-%02X",
		  (unsigned int)interfaceId->time_low,
		  interfaceId->time_mid,
		  interfaceId->time_hi_and_version,
		  interfaceId->clock_seq,
		  interfaceId->node[0],
		  interfaceId->node[1],
		  interfaceId->node[2],
		  interfaceId->node[3],
		  interfaceId->node[4],
		  interfaceId->node[5]);
	return buf;
}

static const char*
interfaceIdToString (SLInterfaceID interfaceId)
{
#define CHECK(x) interfaceId == (x) ? #x :
#define DEFAULT(x) x

	return CHECK(SL_IID_EQUALIZER)
		CHECK(SL_IID_BASSBOOST)
		CHECK(SL_IID_VIRTUALIZER)
		CHECK(SL_IID_PRESETREVERB)
		CHECK(SL_IID_ENVIRONMENTALREVERB)
		CHECK(SL_IID_3DCOMMIT)
		CHECK(SL_IID_3DDOPPLER)
		CHECK(SL_IID_3DGROUPING)
		CHECK(SL_IID_3DLOCATION)
		CHECK(SL_IID_3DMACROSCOPIC)
		CHECK(SL_IID_3DSOURCE)
		CHECK(SL_IID_AUDIODECODERCAPABILITIES)
		CHECK(SL_IID_AUDIOENCODER)
		CHECK(SL_IID_AUDIOENCODERCAPABILITIES)
		CHECK(SL_IID_AUDIOIODEVICECAPABILITIES)
		CHECK(SL_IID_BUFFERQUEUE)
		CHECK(SL_IID_DEVICEVOLUME)
		CHECK(SL_IID_DYNAMICINTERFACEMANAGEMENT)
		CHECK(SL_IID_DYNAMICSOURCE)
		CHECK(SL_IID_EFFECTSEND)
		CHECK(SL_IID_ENGINE)
		CHECK(SL_IID_ENGINECAPABILITIES)
		CHECK(SL_IID_LED)
		CHECK(SL_IID_METADATAEXTRACTION)
		CHECK(SL_IID_METADATATRAVERSAL)
		CHECK(SL_IID_MIDIMESSAGE)
		CHECK(SL_IID_MIDIMUTESOLO)
		CHECK(SL_IID_MIDITEMPO)
		CHECK(SL_IID_MIDITIME)
		CHECK(SL_IID_MUTESOLO)
		CHECK(SL_IID_NULL)
		CHECK(SL_IID_OBJECT)
		CHECK(SL_IID_OUTPUTMIX)
		CHECK(SL_IID_PITCH)
		CHECK(SL_IID_PLAY)
		CHECK(SL_IID_PLAYBACKRATE)
		CHECK(SL_IID_PREFETCHSTATUS)
		CHECK(SL_IID_RATEPITCH)
		CHECK(SL_IID_RECORD)
		CHECK(SL_IID_SEEK)
		CHECK(SL_IID_THREADSYNC)
		CHECK(SL_IID_VIBRA)
		CHECK(SL_IID_VISUALIZATION)
		CHECK(SL_IID_VOLUME)
		CHECK(SL_IID_ANDROIDEFFECT)
		CHECK(SL_IID_ANDROIDEFFECTCAPABILITIES)
		CHECK(SL_IID_ANDROIDEFFECTSEND)
		CHECK(SL_IID_ANDROIDCONFIGURATION)
		CHECK(SL_IID_ANDROIDSIMPLEBUFFERQUEUE)
#if TOSHOK_HONEYCOMB_HACK
		CHECK(SL_IID_ANDROIDBUFFERQUEUE)
		CHECK(XA_IID_ENGINE)
		CHECK(XA_IID_PLAY)
		CHECK(XA_IID_ANDROIDBUFFERQUEUE)
#endif
		DEFAULT(unknownInterfaceToString (interfaceId));
}

bool
OpenSLESPlayer::Initialize ()
{
	SLresult result;
	SLuint32 numSupportedInterfaces;

	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("OpenSLESPlayer::Initialize ()");

	slQueryNumSupportedEngineInterfaces (&numSupportedInterfaces);

	LOG_OSL ("  %d  supported engine interfaces:", numSupportedInterfaces);

	for (SLuint32 i = 0; i < numSupportedInterfaces; i ++) {
		SLInterfaceID interfaceId;
		slQuerySupportedEngineInterfaces (i, &interfaceId);
		LOG_OSL ("      interface = %s: ", interfaceIdToString (interfaceId));
	}

	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");
	LOG_OSL ("********************************");

#if 0
	const SLInterfaceID ids[1] = {SL_IID_ENGINECAPABILITIES};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = slCreateEngine(&engineObject, 0, NULL, 1, ids, req);
#else
	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
#endif
	CHECK_RESULT_BOOL ("slCreateEngine");

	result = (*engineObject)->Realize (engineObject, SL_BOOLEAN_FALSE);
	CHECK_RESULT_BOOL ("engine->Realize");

	result = (*engineObject)->GetInterface (engineObject, SL_IID_ENGINE, &engineEngine);
	CHECK_RESULT_BOOL ("engine->GetInterface (SL_IID_ENGINE)");

#if 0
	{
		SLEngineCapabilitiesItf capabilities;
		SLuint16 profilesSupported;

		result = (*engineObject)->GetInterface (engineObject, SL_IID_ENGINECAPABILITIES, &capabilities);
		CHECK_RESULT_BOOL ("engineObject->GetInterface (SL_IID_ENGINECAPABILITIES)");

		result = (*capabilities)->QuerySupportedProfiles (capabilities, &profilesSupported);
		CHECK_RESULT_BOOL ("capabilities->QuerySupportedProfiles");
		g_debug ("supported profiles:");
		if (profilesSupported & SL_PROFILES_PHONE)
			g_debug ("   PHONE");
		if (profilesSupported & SL_PROFILES_MUSIC)
			g_debug ("   MUSIC");
		if (profilesSupported & SL_PROFILES_GAME)
			g_debug ("   GAME");
	}
#endif

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

