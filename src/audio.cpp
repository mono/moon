/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>

#include "audio.h"
#include "audio-alsa.h"
#include "audio-pulse.h"
#include "pipeline.h"
#include "runtime.h"
#include "clock.h"
#include "debug.h"
#include "mediaplayer.h"

/*
 * AudioSource::AudioFrame
 */

AudioSource::AudioFrame::AudioFrame (MediaFrame *frame)
{
	this->frame = frame;
	this->frame->ref ();
	bytes_used = 0;
}

AudioSource::AudioFrame::~AudioFrame ()
{
	frame->unref ();
}

/*
 * AudioSource
 */

AudioSource::AudioSource (AudioPlayer *player, MediaPlayer *mplayer, AudioStream *stream)
	: EventObject (Type::AUDIOSOURCE)
{
	this->mplayer = mplayer;
	this->mplayer->ref ();
	this->stream = stream;
	this->stream->ref ();
	this->player = player;
	
	stream->AddSafeHandler (IMediaStream::FirstFrameEnqueuedEvent, FirstFrameEnqueuedCallback, this, false);
	
	current_frame = NULL;
	
	state = AudioNone;
	flags = (AudioFlags) 0;
	
	balance = 0.0f;
	volume = 1.0f;
	muted = false;
	
	last_write_pts = G_MAXUINT64;
	last_current_pts = G_MAXUINT64;
	
	channels = stream->channels;
	sample_rate = stream->sample_rate;
	
	g_static_rec_mutex_init (&mutex);
	
	if (channels != 1 && channels != 2)
		SetState (AudioError);
}

AudioSource::~AudioSource ()
{
	g_static_rec_mutex_free (&mutex);
}

void
AudioSource::Dispose ()
{
	if (stream) {
		stream->RemoveAllHandlers (this);
		stream->unref ();
		stream = NULL;
	}
	
	if (mplayer) {
		mplayer->unref ();
		mplayer = NULL;
	}
	
	if (current_frame) {
		delete current_frame;
		current_frame = NULL;
	}
	
	EventObject::Dispose ();
}

void
AudioSource::Lock ()
{
	g_static_rec_mutex_lock (&mutex);
}

void
AudioSource::Unlock ()
{
	g_static_rec_mutex_unlock (&mutex);
}

AudioStream *
AudioSource::GetAudioStream ()
{
	AudioStream *result;
	Lock ();
	result = stream;
	Unlock ();
	return result;
}

void
AudioSource::SetAudioStream (AudioStream *value)
{
	Lock ();
	if (stream)
		stream->unref ();
	stream = value;
	if (stream)
		stream->ref ();
	Unlock ();
}

guint32
AudioSource::GetBytesPerFrame ()
{
	return channels * 2 /* 16bit audio * channels */;
}

AudioStream *
AudioSource::GetStreamReffed ()
{
	AudioStream *result;
	Lock ();
	result = stream;
	Unlock ();
	return result;
}

void
AudioSource::SetFlag (AudioFlags flag, bool value)
{
	Lock ();
	if (value) {
		flags = (AudioFlags) (flag | flags);
	} else {
		flags = (AudioFlags) (~flag & flags);
	}
	LOG_AUDIO_EX ("AudioSource::SetFlag (%i = %s, %i), resulting flags: %i = %s\n", flag, GetFlagNames (flag), value, flags, GetFlagNames (flags));
	Unlock ();
}

bool
AudioSource::GetFlag (AudioFlags flag)
{
	return flags & flag;
}

#if DEBUG
char *
AudioSource::GetFlagNames (AudioFlags flags)
{
	const char *v [5];
	int i = 0;
	v [0] = v [1] = v [2] = v [3] = v [4] = NULL;
	
	if (flags & AudioInitialized)
		v [i++] = "Initialized";
	
	if (flags & AudioEOF)
		v [i++] = "EOF";
		
	if (flags & AudioWaiting)
		v [i++] = "Waiting";
	
	if (flags & AudioEnded)
		v [i++] = "Ended";
		
	return (char *) g_strjoinv (",", (gchar **) v);
	
}
#endif

const char *
AudioSource::GetStateName (AudioState state)
{
	switch (state) {
	case AudioNone: return "None";
	case AudioPlaying: return "Playing";
	case AudioPaused: return "Paused";
	case AudioError: return "Error";
	case AudioStopped: return "Stopped";
	default: return "Unknown";
	}
}

AudioState
AudioSource::GetState ()
{
	AudioState result;
	Lock ();
	result = state;
	Unlock ();
	return result;
}

void
AudioSource::SetState (AudioState value)
{
	AudioState old_state;
	bool changed = false;
	
	Lock ();
	if (state != value) {
		if (state == AudioError) {
			LOG_AUDIO ("AudioSource::SetState (%s): Current state is Error, can't change that state\n", GetStateName (value));
		} else {
			old_state = state;
			state = value;
			changed = true;
			if (value == AudioError)
				mplayer->AudioFailed (this);
		}
	}
	Unlock ();
	
	LOG_AUDIO_EX ("AudioSource::SetState (%s), old state: %s, changed: %i\n", GetStateName (value), GetStateName (old_state), changed);
	
	if (changed)
		StateChanged (old_state);
}

double 
AudioSource::GetBalance ()
{
	double result;
	Lock ();
	result = balance;
	Unlock ();
	return result;
}

void 
AudioSource::SetBalance (double value)
{
	Lock ();
	balance = value;
	Unlock ();
}

double
AudioSource::GetVolume ()
{
	double result;
	Lock ();
	result = volume;
	Unlock ();
	return result;
}

void 
AudioSource::SetVolume (double value)
{
	Lock ();
	volume = value;
	Unlock ();
}

bool
AudioSource::GetMuted ()
{
	bool result;
	Lock ();
	result = muted;
	Unlock ();
	return result;
}

void 
AudioSource::SetMuted (bool value)
{
	Lock ();
	muted = value;
	Unlock ();
}

guint32 
AudioSource::GetChannels ()
{
	// This can only be set during initialization, so there's no need to lock here.
	return channels;
}

guint32
AudioSource::GetSampleRate ()
{
	// This can only be set during initialization, so there's no need to lock here.
	return sample_rate;
}

bool
AudioSource::IsQueueEmpty ()
{
	bool result;
	AudioStream *stream;
	
	LOG_AUDIO_EX ("AudioSource::IsQueueEmpty ().\n");
	
	stream = GetStreamReffed ();
	
	g_return_val_if_fail (stream != NULL, false);
	
	result = stream->IsQueueEmpty ();
	
	stream->unref ();
	
	return result;
}

void
AudioSource::FirstFrameEnqueuedHandler (EventObject *sender, EventArgs *args)
{
	LOG_AUDIO_EX ("AudioSource::FirstFrameEnqueuedHandler ().\n");
	
	if (GetFlag (AudioWaiting)) {
		SetFlag (AudioWaiting, false);
		if (GetState () == AudioPlaying)
			Play ();
	}
}

guint64
AudioSource::GetDelay ()
{
	return GetDelayInternal ();
}

guint64
AudioSource::GetCurrentPts ()
{
	guint64 delay = 0;
	guint64 current_pts = 0;
	guint64 result = 0;
	
	if (GetState () != AudioPlaying) {
		result = last_current_pts;
	} else {
		Lock ();
		current_pts = last_write_pts;
		Unlock ();
		
		delay = GetDelay ();

		if (current_pts == G_MAXUINT64) {
			result = current_pts;
		} else if (delay == G_MAXUINT64 || GetState () != AudioPlaying) {
			result = last_current_pts;
		} else if (delay > current_pts) {
			result = 0;
		} else {
			result = current_pts - delay;
		}
	}

	last_current_pts = result;
	
	LOG_AUDIO_EX ("AudioSource::GetCurrentPts (): %" G_GUINT64_FORMAT " ms, delay: %" G_GUINT64_FORMAT ", last_write_pts: %llu\n", 
		MilliSeconds_FromPts (result), MilliSeconds_FromPts (delay), MilliSeconds_FromPts (last_write_pts));
		
	return result;
}

void
AudioSource::Stop ()
{
	Lock ();
	SetState (AudioStopped);
	last_current_pts = G_MAXUINT64;
	last_write_pts = G_MAXUINT64;
	Unlock ();
	Stopped ();
}

void
AudioSource::Play ()
{
	SetState (AudioPlaying);
	SetFlag ((AudioFlags) (AudioEnded | AudioEOF | AudioWaiting), false);
	Played ();
}

void
AudioSource::Pause ()
{
	SetState (AudioPaused);
	Paused ();
}

void
AudioSource::Underflowed ()
{
	LOG_AUDIO ("AudioSource::Underflowed (), state: %s, flags: %s\n", GetStateName (GetState ()), GetFlagNames (flags));
	
	SetCurrentDeployment (false);
	
	if (GetState () == AudioPlaying) {
		if (GetFlag (AudioEOF)) {
			Stop ();
			SetFlag (AudioEnded, true);
			mplayer->AudioFinished ();
		} else if (IsQueueEmpty ()) {
			mplayer->SetBufferUnderflow ();
		}
	}
}

bool
AudioSource::Initialize ()
{
	bool result;
	
	result = InitializeInternal ();
	
	if (result) {
		SetFlag (AudioInitialized, true);
	} else {
		SetFlag (AudioInitialized, false);
		SetState (AudioError);
	}
	
	return result;
}

void
AudioSource::Close ()
{
	CloseInternal ();
}

guint32
AudioSource::Write (void *dest, guint32 samples)
{
	AudioData **data = (AudioData **) g_alloca (sizeof (AudioData *) * (channels + 1));
	guint32 result = 0;
	
	switch (channels) {
	case 1:
		data [0] = (AudioData *) g_malloc (sizeof (AudioData));
		data [1] = NULL;
		data [0]->dest = dest;
		data [0]->distance = GetBytesPerFrame (); // 16 bit audio
		result = WriteFull (data, samples);
		break;
	case 2:
		data [0] = (AudioData *) g_malloc (sizeof (AudioData));
		data [1] = (AudioData *) g_malloc (sizeof (AudioData));
		data [2] = NULL;
		data [0]->dest = dest;
		data [1]->dest = ((char *) dest) + 2; // Interleaved audio data
		data [1]->distance = data [0]->distance = GetBytesPerFrame (); // 16 bit audio * 2 channels
		result = WriteFull (data, samples);
		break;
	default:
		SetState (AudioError);
		return 0;
	}
	
	for (int i = 0; data [i] != NULL; i++) {
		g_free (data [i]);
	}
	
	return result;
}

guint32
AudioSource::WriteFull (AudioData **channel_data, guint32 samples)
{
	guint32 channels = GetChannels ();
	gint32 *volumes = (gint32 *) g_alloca (sizeof (gint32) * channels);
	gint32 volume;
	double balance;
	bool muted;
	gint16 **write_ptr = (gint16 **) g_alloca (sizeof (gint16 *) * channels);
	gint16 *read_ptr = NULL;
	guint32 result = 0;
	guint32 bytes_per_sample = 2 * channels;
	guint32 samples_to_write;
	guint32 bytes_available;
	guint32 bytes_written;
	gint32 value;
	guint64 last_frame_pts = 0; // The pts of the last frame which was used to write samples
	guint64 last_frame_samples = 0; // Samples written from the last frame
	
	SetCurrentDeployment (false);
	
	// Validate input
	if (channel_data == NULL) {
		SetState (AudioError);
		return 0;
	}
	for (guint32 i = 0; i < channels; i++) {
		if (channel_data [i] == NULL) {
			LOG_AUDIO ("AudioSource::WriteFull (%p, %u): channel data #%i is NULL\n", channel_data, samples, i );
			SetState (AudioError);
			return 0;
		}
	}
	if (channel_data [channels] != NULL) {
		SetState (AudioError);
		return 0;
	}

	Lock ();
			
	volume = this->volume * 8192;
	balance = this->balance;
	muted = false; //this->muted;
	
	// Set the per-channel volume
	if (channels == 2) {
		if (muted) {
			volumes [0] = volumes [1] = 0;
		} else 	if (balance < 0.0) {
			volumes [0] = volume;
			volumes [1] = (1.0 + balance) * volume;
		} else if (balance > 0.0) {
			volumes [0] = (1.0 - balance) * volume;
			volumes [1] = volume;
		} else {
			volumes [0] = volumes [1] = volume;
		}
	} else if (channels == 1) {
		if (muted) {
			volumes [0] = 0;
		} else {
			volumes [0] = volume;
		}
	} else {
		SetState (AudioError);
		goto cleanup;
	}
	
	for (guint32 i = 0; i < channels; i++)
		write_ptr [i] = (gint16 *) channel_data [i]->dest;
	
	while (GetState () == AudioPlaying) {
		if (current_frame == NULL) {
			MediaFrame *frame = stream->PopFrame ();
			if (frame != NULL)
				current_frame = new AudioFrame (frame);
		}
		
		if (current_frame == NULL) {
			if (stream->GetEnded ()) {
				LOG_AUDIO ("AudioSource::WriteFull (): No more data and reached the end.\n");
				SetFlag (AudioWaiting, false);
				SetFlag ((AudioFlags) (AudioEOF | AudioEnded), true);
			} else {
				LOG_AUDIO ("AudioSource::WriteFull (): No more data, starting to wait...\n");
				if (!GetFlag (AudioEOF) && !GetFlag (AudioEnded)) {
					SetFlag (AudioWaiting, true);
					SetFlag ((AudioFlags) (AudioEOF | AudioEnded), false);
				}
			}
			goto cleanup;
		}

		bytes_available = current_frame->frame->buflen - current_frame->bytes_used;
		
		if (bytes_available < bytes_per_sample) {
			LOG_AUDIO ("AudioSource::WriteFull (): incomplete packet, bytes_available: %u, buflen: %u, bytes_used: %u\n", bytes_available, current_frame->frame->buflen, current_frame->bytes_used);
			delete current_frame;
			current_frame = NULL;
			continue;
		}
		
		samples_to_write = MIN (bytes_available / bytes_per_sample, samples - result);
		bytes_written = samples_to_write * bytes_per_sample;
		
		gint16 *initial_read_ptr;
		read_ptr = (gint16 *) (((char *) current_frame->frame->buffer) + current_frame->bytes_used);
		initial_read_ptr = read_ptr;
		
		for (guint32 i = 0; i < samples_to_write; i++) {
			for (guint32 channel = 0; channel < channels; channel++) {
				value = ((*read_ptr) * volumes [channel]) >> 13;
				*(write_ptr [channel]) = (gint16) CLAMP (value, -32768, 32767);
				write_ptr [channel] = (gint16 *) (((char *) write_ptr [channel]) + channel_data [channel]->distance);
				read_ptr++;
			}
		}
		
		result += samples_to_write;
		current_frame->bytes_used += bytes_written;
				
		last_frame_samples = current_frame->bytes_used / GetBytesPerFrame ();
		last_frame_pts = current_frame->frame->pts;
		
		if (current_frame->bytes_used == current_frame->frame->buflen) {
			// We used the entire packet
			delete current_frame;
			current_frame = NULL;
		} else {
			// There is still audio data left in the packet, just leave it.
		}
		
		if (result == samples) {
			// We've written all we were requested to write
			goto cleanup;
		} else {
			//printf ("AudioSource::WriteFull (): Written %u samples of %u requested samples, getting new packet (%i packets left)\n", result, samples, frames.Length ());
		}
	}
	
cleanup:
	LOG_AUDIO_EX ("AudioSource::Write (%p, %u): Wrote %u samples, current pts: %" G_GUINT64_FORMAT ", volume: %.2f\n", channel_data, samples, result, MilliSeconds_FromPts (GetCurrentPts ()), this->volume);
	
	if (result > 0) {
		last_write_pts = last_frame_pts + MilliSeconds_ToPts (last_frame_samples * 1000 / GetSampleRate ());
	}
	
	Unlock ();
	
	return result;
}

/*
 * AudioListNode
 */

AudioListNode::AudioListNode (AudioSource *source)
{
	this->source = source;
	this->source->ref ();
	generation = 0;
}

AudioListNode::~AudioListNode ()
{
	this->source->unref ();
}

/* 
 * AudioSources
 */

AudioSources::AudioSources ()
{
	g_static_mutex_init (&mutex);
	current_generation = 0;
}

AudioSources::~AudioSources ()
{
	g_static_mutex_free (&mutex);
}

void
AudioSources::Lock ()
{
	g_static_mutex_lock (&mutex);
}

void
AudioSources::Unlock ()
{
	g_static_mutex_unlock (&mutex);
}

void
AudioSources::Add (AudioSource *source)
{
	Lock ();
	list.Append (new AudioListNode (source));
	Unlock ();
}

bool
AudioSources::Remove (AudioSource *source)
{
	AudioListNode *node;
	bool result = false;
	
	Lock ();
	node = (AudioListNode *) list.First ();
	while (node != NULL) {
		if (node->source == source) {
			result = true;
			if (last_node == node)
				last_node = (AudioListNode *) node->prev;
			list.Remove (node);
			source->unref ();
			break;
		}
		node = (AudioListNode *) node->next;
	}
	last_node = NULL;
	Unlock ();
	
	return result;
}

void
AudioSources::StartEnumeration ()
{
	Lock ();
	current_generation++;
	last_node = NULL;
	Unlock ();
}

AudioSource *
AudioSources::GetNext (bool only_playing)
{
	AudioListNode *node = NULL;
	AudioSource *result = NULL;
	
	Lock ();
	
	// Check the last node returned from GetNext
	if (last_node != NULL && last_node->next != NULL) {
		node = (AudioListNode *) last_node->next;
		if (node->generation != current_generation && (!only_playing || node->source->IsPlaying ()))
			goto cleanup;
	}
	
	// Loop through all the nodes looking for a node not in the
	// current generation.
	node = (AudioListNode *) list.First ();
	while (node != NULL && (node->generation == current_generation || (only_playing && !node->source->IsPlaying ()))) {
		node = (AudioListNode *) node->next;
	}
	
	// Its possible that the loop has started but nothing is playing, which without this guard would
	// return list.First () in an infinite loop while we're downloading / buffering.
	// (due to the while loop above not clearing out the first value (list.First ()) if the condition is false and there's no other 
	// node which satifies the condition)
	if (only_playing && node != NULL && !node->source->IsPlaying ())
		node = NULL;

cleanup:
	if (node) {
		node->generation = current_generation;
		last_node = node;
		result = node->source;
		result->SetCurrentDeployment (false);
		result->ref ();
	} else {
		Deployment::SetCurrent (NULL, false);
	}
				
	Unlock ();
	
	return result;
}

AudioSource *
AudioSources::GetHead ()
{
	AudioSource *result = NULL;
	AudioListNode *node;
	
	Lock ();
	
	node = (AudioListNode *) list.First ();
	if (node != NULL) {
		result = node->source;
		result->SetCurrentDeployment (false);
		result->ref ();
	}
	
	Unlock ();
	
	return result;
}

#if DEBUG
int
AudioSources::Length ()
{
	int result = 0;
	
	Lock ();
	result = list.Length ();
	Unlock ();
	
	return result;
}
#endif

/*
 * AudioPlayer
 */

AudioPlayer *AudioPlayer::instance = NULL;
GStaticMutex AudioPlayer::instance_mutex = G_STATIC_MUTEX_INIT;

AudioSource *
AudioPlayer::Add (MediaPlayer *mplayer, AudioStream *stream)
{
	AudioSource *result = NULL;
	
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	
	if (moonlight_flags & RUNTIME_INIT_DISABLE_AUDIO) {
		LOG_AUDIO ("AudioPlayer: audio is disabled.\n");
		return NULL;
	}
	
	g_static_mutex_lock (&instance_mutex);
	if (instance == NULL)
		instance = CreatePlayer ();
	if (instance != NULL)
		result = instance->AddImpl (mplayer, stream);
	g_static_mutex_unlock (&instance_mutex);
	
	return result;
}

void
AudioPlayer::Remove (AudioSource *source)
{
	LOG_AUDIO ("AudioPlayer::Remove (%p)\n", source);
	
	g_static_mutex_lock (&instance_mutex);
	if (instance != NULL)
		instance->RemoveImpl (source);

	g_static_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::Shutdown ()
{
	AudioPlayer *player;
	LOG_AUDIO ("AudioPlayer::Shutdown ()\n");
	
	g_static_mutex_lock (&instance_mutex);
	
	if (instance != NULL) {
		player = instance;
		instance = NULL;
		player->ShutdownImpl ();
		delete player;
	}
	
	g_static_mutex_unlock (&instance_mutex);
}

AudioPlayer *
AudioPlayer::CreatePlayer ()
{
	bool overridden;
	AudioPlayer *result = NULL;
	
	// If any of the flags are specified, we disable all players
	// and re-enable according to the flag.
	
	overridden  = moonlight_flags & (RUNTIME_INIT_AUDIO_PULSE | RUNTIME_INIT_AUDIO_ALSA | RUNTIME_INIT_AUDIO_ALSA_MMAP | RUNTIME_INIT_AUDIO_ALSA_RW);

#if INCLUDE_PULSEAUDIO
	if (result != NULL) {
		LOG_AUDIO ("AudioPlayer: Not checking for PulseAudio support, we already found support for another configuration.\n");
	} else if (overridden && !(moonlight_flags & RUNTIME_INIT_AUDIO_PULSE)) {
		LOG_AUDIO ("AudioPlayer: PulseAudio disabled with environment variable (MOONLIGHT_OVERRIDES)\n");
	} else if (!PulsePlayer::IsInstalled ()) {
		LOG_AUDIO ("AudioPlayer: PulseAudio is not installed or configured correctly.\n");
	} else {
		printf ("AudioPlayer: Using PulseAudio.\n");
		result = new PulsePlayer ();
	}	

	if (result != NULL) {
		if (!result->Initialize ()) {
			LOG_AUDIO ("AudioPlayer: Failed initialization.\n");
			result->ShutdownImpl ();
			delete result;
			result = NULL;
		} else {
			return result;
		}
	}
#else
	LOG_AUDIO ("AudioPlayer: Built without support for pulseaudio.\n");
#endif

#if INCLUDE_ALSA
	if (result != NULL) {
		LOG_AUDIO ("AudioPlayer: Not checking for Alsa support, we already found support for another configuration.\n");
	} else if (overridden && !(moonlight_flags & (RUNTIME_INIT_AUDIO_ALSA | RUNTIME_INIT_AUDIO_ALSA_MMAP | RUNTIME_INIT_AUDIO_ALSA_RW))) {
		LOG_AUDIO ("AudioPlayer: Alsa disabled with environment variable (MOONLIGHT_OVERRIDES)\n");
	} else if (!AlsaPlayer::IsInstalled ()) {
		LOG_AUDIO ("AudioPlayer: Alsa is not installed or configured correctly.\n");
	} else {
		printf ("AudioPlayer: Using Alsa.\n");
		result = new AlsaPlayer ();
	}

	if (result != NULL) {
		if (!result->Initialize ()) {
			LOG_AUDIO ("AudioPlayer: Failed initialization.\n");
			result->ShutdownImpl ();
			delete result;
			result = NULL;
		} else {
			return result;
		}
	}
#else
	LOG_AUDIO ("AudioPlayer: Built without support for alsa.\n");
#endif

	return result;
}

AudioSource *
AudioPlayer::AddImpl (MediaPlayer *mplayer, AudioStream *stream)
{
	AudioSource *result = CreateNode (mplayer, stream);

	if (result->Initialize ()) {
		sources.Add (result);		
		AddInternal (result);
	} else {
		result->unref ();
		result = NULL;
	}
				
	return result;
}

void
AudioPlayer::RemoveImpl (AudioSource *node)
{
	node->ref ();
	if (sources.Remove (node)) {
		RemoveInternal (node);
		node->Close ();
	}
	node->unref ();
}

void
AudioPlayer::ShutdownImpl ()
{
	AudioSource *source;
	
	PrepareShutdownInternal ();

	// Remove all the sources.
	while ((source = sources.GetHead ()) != NULL) {
		RemoveImpl (source);
		source->unref ();
	}

	FinishShutdownInternal ();
}
