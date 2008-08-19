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

#include <config.h>

#include <pthread.h>

#include "audio.h"
#include "audio-alsa.h"
#include "audio-pulse.h"
#include "pipeline.h"
#include "runtime.h"
#include "debug.h"

#define LOG_AUDIO(...)// printf (__VA_ARGS__);
// This one prints out spew on every sample
#define LOG_AUDIO_EX(...)// printf (__VA_ARGS__);

/*
 * AudioFrameNode
 */

class AudioFrameNode : public List::Node {
public:
	MediaFrame *frame;
	guint32 bytes_used;
	AudioFrameNode (MediaFrame *frame) 
	{
		this->frame = frame;
		bytes_used = 0;
	}
	~AudioFrameNode ()
	{
		delete frame;
	}
};

/*
 * AudioSource
 */

AudioSource::AudioSource (AudioPlayer *player, MediaPlayer *mplayer, AudioStream *stream)
{
	pthread_mutexattr_t attribs;
	
	this->mplayer = mplayer;
	this->mplayer->ref ();
	this->stream = stream;
	this->stream->ref ();
	this->player = player;
	
	state = AudioNone;
	flags = (AudioFlags) 0;
	
	balance = 0.0f;
	volume = 1.0f;
	muted = false;
	
	first_pts = 0;
	last_write_pts = 0;
	last_current_pts = 0;
	
	channels = stream->channels;
	sample_rate = stream->sample_rate;
	
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&mutex, &attribs);
	pthread_mutexattr_destroy (&attribs);
}

AudioSource::~AudioSource ()
{
	stream->unref ();
	mplayer->unref ();
	
	pthread_mutex_destroy (&mutex);
}

void
AudioSource::Lock ()
{
	pthread_mutex_lock (&mutex);
}

void
AudioSource::Unlock ()
{
	pthread_mutex_unlock (&mutex);
}

guint32
AudioSource::GetBytesPerFrame ()
{
	return channels * 2 /* 16bit audio * channels */;
}

AudioStream *
AudioSource::GetStream ()
{
	// This can only be set during initialization, so there's no need to lock here.
	return stream;
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
			AUDIO_DEBUG ("AudioSource::SetState (%s): Current state is Error, can't change that state\n", GetStateName (value));
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
	
#if DEBUG
	if (value == AudioError)
		print_stack_trace ();
#endif
	
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

void
AudioSource::AppendFrame (MediaFrame *frame)
{
	LOG_AUDIO_EX ("AudioSource::AppendFrame (%p): now got %i frames, this frame's EOF: %i, buflen: %i\n", frame, frames.Length () + 1, frame->event == FrameEventEOF, frame->buflen);
		
	frames.Push (new AudioFrameNode (frame));
	
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
	guint64 delay;
	guint64 current_pts;
	guint64 result;
	
	if (GetState () != AudioPlaying) {
		result = last_current_pts;
	} else {
		Lock ();
		current_pts = last_write_pts;
		Unlock ();
		
		delay = GetDelay ();
		
		if (delay == G_MAXUINT64 || GetState () != AudioPlaying) {
			result = last_current_pts;
		} else if (delay > current_pts) {
			result = 0;
		} else {
			result = current_pts - delay;
		}
	}

	last_current_pts = result;
	
	LOG_AUDIO_EX ("AudioSource::GetCurrentPts (): %llu ms, delay: %llu, last_write_pts: %llu\n", 
		MilliSeconds_FromPts (result), MilliSeconds_FromPts (delay), MilliSeconds_FromPts (last_write_pts));
		
	return result;
}

void
AudioSource::Stop ()
{
	Lock ();
	SetState (AudioStopped);
	frames.Clear (true);
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
	if (GetState () == AudioPlaying && GetFlag (AudioEOF)) {
		Stop ();
		SetFlag (AudioEnded, true);
		mplayer->AudioFinished ();
	}
}

MediaResult
AudioSource::FrameCallback (MediaClosure *closure)
{
	AudioSource *source = (AudioSource *) closure->GetContext ();
	MediaFrame *frame = closure->frame;
	closure->frame = NULL;
	source->AppendFrame (frame);
	return MEDIA_SUCCESS;
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
	AudioData *data [channels + 1];
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
	gint32 volumes [channels];
	gint32 volume;
	double balance;
	bool muted;
	gint16 *write_ptr [channels];
	gint16 *read_ptr = NULL;
	guint32 result = 0;
	guint32 bytes_per_sample = 2 * channels;
	guint32 samples_to_write;
	guint32 bytes_available;
	guint32 bytes_written;
	gint32 value;
	guint64 last_frame_pts; // The pts of the last frame which was used to write samples
	guint64 last_frame_samples; // Samples written from the last frame
	AudioFrameNode *node;
	
	// Validate input
	if (channel_data == NULL) {
		SetState (AudioError);
		return 0;
	}
	for (guint32 i = 0; i < channels; i++) {
		if (channel_data [i] == NULL) {
			AUDIO_DEBUG ("AudioSource::WriteFull (%p, %u): channel data #%i is NULL\n", channel_data, samples, i );
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
		node = (AudioFrameNode *) frames.Pop ();
		
		if (node == NULL) {
			LOG_AUDIO ("AudioSource::WriteFull (): No more data, starting to wait...\n");
			if (!GetFlag (AudioEOF) && !GetFlag (AudioEnded)) {
				SetFlag (AudioWaiting, true);
				SetFlag ((AudioFlags) (AudioEOF | AudioEnded), false);
			}
			goto cleanup;
		}
		
		if (node->frame->event == FrameEventEOF && node->bytes_used == node->frame->buflen) {
			// We've used all the data from the last packet
			LOG_AUDIO ("AudioSource::WriteFull (): Reached end of data\n");
			SetFlag (AudioEOF, true);
			SetFlag ((AudioFlags) (AudioWaiting | AudioEnded), false);
		}
		
		bytes_available = node->frame->buflen - node->bytes_used;
		
		if (bytes_available < bytes_per_sample) {
			LOG_AUDIO ("AudioSource::WriteFull (): incomplete packet, bytes_available: %u, buflen: %u, bytes_used: %u\n", bytes_available, node->frame->buflen, node->bytes_used);
			mplayer->EnqueueFramesAsync (1, 0);
			delete node;
			continue;
		}
		
		samples_to_write = MIN (bytes_available / bytes_per_sample, samples - result);
		bytes_written = samples_to_write * bytes_per_sample;
		
		gint16 *initial_read_ptr;
		read_ptr = (gint16 *) (((char *) node->frame->buffer) + node->bytes_used);
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
		node->bytes_used += bytes_written;
				
		last_frame_samples = node->bytes_used / GetBytesPerFrame ();
		last_frame_pts = node->frame->pts;
		
		if (node->bytes_used == node->frame->buflen) {
			// We used the entire packet			
			mplayer->EnqueueFramesAsync (1, 0);
			delete node;
			node = NULL;
		} else {
			// There is still audio data left in the packet, put it back in the queue until the next Write.
			frames.Lock ();
			frames.LinkedList ()->Prepend (node);
			frames.Unlock ();
		}
		
		if (result == samples) {
			// We've written all we were requested to write
			goto cleanup;
		} else {
			//printf ("AudioSource::WriteFull (): Written %u samples of %u requested samples, getting new packet (%i packets left)\n", result, samples, frames.Length ());
		}
	}
	
cleanup:
	LOG_AUDIO_EX ("AudioSource::Write (%p, %u): Wrote %u samples, current pts: %llu\n", channel_data, samples, result, MilliSeconds_FromPts (GetCurrentPts ()));
	
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
	pthread_mutex_init (&mutex, NULL);
	current_generation = 0;
}

AudioSources::~AudioSources ()
{
	pthread_mutex_destroy (&mutex);
}

void
AudioSources::Lock ()
{
	pthread_mutex_lock (&mutex);
}

void
AudioSources::Unlock ()
{
	pthread_mutex_unlock (&mutex);
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
		if (node->generation != current_generation && (!only_playing || node->source->GetState () == AudioPlaying))
			goto cleanup;
	}
	
	// Loop through all the nodes looking for a node not in the
	// current generation.
	node = (AudioListNode *) list.First ();
	while (node != NULL && node->generation == current_generation && (!only_playing || node->source->GetState () == AudioPlaying)) {
		node = (AudioListNode *) node->next;
	}
	
cleanup:
	if (node) {
		node->generation = current_generation;
		last_node = node;
		result = node->source;
		result->ref ();
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

AudioPlayer * AudioPlayer::instance = NULL;
pthread_mutex_t AudioPlayer::instance_mutex = PTHREAD_MUTEX_INITIALIZER;

AudioSource *
AudioPlayer::Add (MediaPlayer *mplayer, AudioStream *stream)
{
	AudioSource *result = NULL;
	
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	
	if (moonlight_flags & RUNTIME_INIT_DISABLE_AUDIO) {
		LOG_AUDIO ("AudioPlayer: audio is disabled.\n");
		return NULL;
	}
	
	pthread_mutex_lock (&instance_mutex);
	if (instance == NULL)
		instance = CreatePlayer ();
	if (instance != NULL)
		result = instance->AddImpl (mplayer, stream);
	pthread_mutex_unlock (&instance_mutex);
	
	return result;
}

void
AudioPlayer::Remove (AudioSource *source)
{
	LOG_AUDIO ("AudioPlayer::Remove (%p)\n", source);
	
	pthread_mutex_lock (&instance_mutex);
	if (instance != NULL)
		instance->RemoveImpl (source);

	pthread_mutex_unlock (&instance_mutex);
}

void
AudioPlayer::Shutdown ()
{
	LOG_AUDIO ("AudioPlayer::Shutdown ()\n");
	
	pthread_mutex_lock (&instance_mutex);
	if (instance != NULL) {
		instance->ShutdownImpl ();
		delete instance;
	}
	pthread_mutex_unlock (&instance_mutex);
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
		AUDIO_DEBUG ("AudioPlayer: Not checking for PulseAudio support, we already found support for another configuration.\n");
	} else if (overridden && !(moonlight_flags & RUNTIME_INIT_AUDIO_PULSE)) {
		AUDIO_DEBUG ("AudioPlayer: PulseAudio disabled with environment variable (MOONLIGHT_OVERRIDES)\n");
	} else if (!PulsePlayer::IsInstalled ()) {
		AUDIO_DEBUG ("AudioPlayer: PulseAudio is not installed or configured correctly.\n");
	} else {
		printf ("AudioPlayer: Using PulseAudio.\n");
		result = new PulsePlayer ();
	}	
#else
	AUDIO_DEBUG ("AudioPlayer: Built without support for pulseaudio.\n");
#endif

#if INCLUDE_ALSA
	if (result != NULL) {
		AUDIO_DEBUG ("AudioPlayer: Not checking for Alsa support, we already found support for another configuration.\n");
	} else if (overridden && !(moonlight_flags & (RUNTIME_INIT_AUDIO_ALSA | RUNTIME_INIT_AUDIO_ALSA_MMAP | RUNTIME_INIT_AUDIO_ALSA_RW))) {
		AUDIO_DEBUG ("AudioPlayer: Alsa disabled with environment variable (MOONLIGHT_OVERRIDES)\n");
	} else if (!AlsaPlayer::IsInstalled ()) {
		AUDIO_DEBUG ("AudioPlayer: Alsa is not installed or configured correctly.\n");
	} else {
		printf ("AudioPlayer: Using Alsa.\n");
		result = new AlsaPlayer ();
	}
#else
	AUDIO_DEBUG ("AudioPlayer: Built without support for alsa.\n");
#endif

	if (result && !result->Initialize ()) {
		AUDIO_DEBUG ("AudioPlayer: Failed initialization.\n");
		result->ShutdownImpl ();
		delete result;
		result = NULL;
	}

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
	
	// Remove all the sources.
	while ((source = sources.GetHead ()) != NULL) {
		RemoveImpl (source);
		source->unref ();
	}
	
	ShutdownInternal ();
}
