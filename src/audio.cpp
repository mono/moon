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
#include "clock.h"
#include "debug.h"
#include "mediaplayer.h"
#include "deployment.h"

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
	: EventObject (Type::AUDIOSOURCE, true)
{
	pthread_mutexattr_t attribs;
	
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
	
	channels = stream->GetOutputChannels ();
	sample_rate = stream->GetOutputSampleRate ();
	input_bytes_per_sample = stream->GetOutputBitsPerSample () / 8;
	output_bytes_per_sample = input_bytes_per_sample;
	
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&mutex, &attribs);
	pthread_mutexattr_destroy (&attribs);
	
	
#ifdef DUMP_AUDIO
	char *fname = g_strdup_printf ("/tmp/AudioSource-%iHz-%iChannels-%iBit.raw", sample_rate, channels, input_bytes_per_sample * 8);
	dump_fd = fopen (fname, "w+");
	printf ("AudioSource: Dumping pcm data to: %s, command line to play:\n", fname);
	printf ("play -s -t raw -%i -c %i --rate %i %s\n", input_bytes_per_sample, channels, sample_rate, fname);
	g_free (fname);
#endif
}

AudioSource::~AudioSource ()
{
	pthread_mutex_destroy (&mutex);
	
#ifdef DUMP_AUDIO
	fclose (dump_fd);
#endif
}

void
AudioSource::Dispose ()
{
	IMediaStream *stream;
	MediaPlayer *mplayer;
	AudioFrame *current_frame;
	
	Stop ();

	Lock ();
	stream = this->stream;
	this->stream = NULL;
	mplayer = this->mplayer;
	this->mplayer = NULL;
	current_frame = this->current_frame;
	this->current_frame = NULL;
	Unlock ();
	
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

MediaPlayer *
AudioSource::GetMediaPlayerReffed ()
{
	MediaPlayer *result = NULL;
	Lock ();
	if (mplayer != NULL) {
		result = mplayer;
		result->ref ();
	}
	Unlock ();
	return result;
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
AudioSource::GetInputBytesPerFrame ()
{
	/* No locking required, this can only be set during initialization */
	return channels * input_bytes_per_sample;
}

guint32
AudioSource::GetInputBytesPerSample ()
{ 
	/* No locking required, this can only be set during initialization */
	return input_bytes_per_sample;
}

guint32
AudioSource::GetOutputBytesPerFrame ()
{
	/* No locking required, this can only be set during initialization */
	return channels * output_bytes_per_sample;
}

guint32
AudioSource::GetOutputBytesPerSample ()
{
	/* No locking required, this can only be set during initialization */
	return output_bytes_per_sample;
}

void
AudioSource::SetOutputBytesPerSample (guint32 value)
{
	/* No locking required, this can only be set during initialization */
	output_bytes_per_sample = value;
}

AudioStream *
AudioSource::GetStreamReffed ()
{
	AudioStream *result;
	Lock ();
	result = stream;
	if (result)
		result->ref ();
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

#if LOGGING
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
	AudioState old_state = AudioNone;
	bool changed = false;
	bool audio_failed = false;
		
	Lock ();
	if (state != value) {
		if (state == AudioError) {
			LOG_AUDIO ("AudioSource::SetState (%s): Current state is Error, can't change that state\n", GetStateName (value));
		} else {
			old_state = state;
			state = value;
			changed = true;
			if (value == AudioError)
				audio_failed = true;
		}
	}
	Unlock ();
	
	if (audio_failed) {
		MediaPlayer *mplayer = GetMediaPlayerReffed ();
		if (mplayer != NULL) {
			mplayer->AudioFailed (this);
			mplayer->unref ();
		}
	}
	
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
	
	if (stream == NULL) {
		result = true;
	} else {
		result = stream->IsQueueEmpty ();
		stream->unref ();
	}
	
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
	
	LOG_AUDIO_EX ("AudioSource::GetCurrentPts (): %" G_GUINT64_FORMAT " ms, delay: %" G_GUINT64_FORMAT ", last_write_pts: %" G_GUINT64_FORMAT "\n", 
		MilliSeconds_FromPts (result), MilliSeconds_FromPts (delay), MilliSeconds_FromPts (last_write_pts));
		
	return result;
}

void
AudioSource::Stop ()
{
	LOG_AUDIO ("AudioSource::Stop ()\n");
	
	Lock ();
	SetState (AudioStopped);
	last_current_pts = G_MAXUINT64;
	last_write_pts = G_MAXUINT64;
	delete current_frame;
	current_frame = NULL;
	Unlock ();
	Stopped ();
}

void
AudioSource::Play ()
{
	LOG_AUDIO ("AudioSource::Play ()\n");
	
	SetState (AudioPlaying);
	SetFlag ((AudioFlags) (AudioEnded | AudioEOF | AudioWaiting), false);
	Played ();
}

void
AudioSource::Pause ()
{
	LOG_AUDIO ("AudioSource::Pause ()\n");
	
	SetState (AudioPaused);
	Paused ();
}

void
AudioSource::Underflowed ()
{
	MediaPlayer *mplayer;
	LOG_AUDIO ("AudioSource::Underflowed (), state: %s, flags: %s\n", GetStateName (GetState ()), GetFlagNames (flags));
	
	if (IsDisposed ())
		return;
	
	SetCurrentDeployment (false);
	
	mplayer = GetMediaPlayerReffed ();
	
	if (GetState () == AudioPlaying) {
		if (GetFlag (AudioEOF)) {
			Stop ();
			SetFlag (AudioEnded, true);
			if (mplayer != NULL)
				mplayer->AudioFinished ();
		} else if (IsQueueEmpty ()) {
			SetFlag (AudioWaiting, true);
			if (mplayer != NULL)
				mplayer->SetBufferUnderflow ();
		}
	}
	
	if (mplayer != NULL)
		mplayer->unref ();
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
	
	for (unsigned int i = 0; i < channels; i++)
		data [i] = (AudioData *) g_malloc (sizeof (AudioData));
	
	data [0]->dest = dest;
	data [0]->distance = GetOutputBytesPerFrame ();
	// Interleaved multi-channel audio data
	for (unsigned int i = 1; i < channels; i++) {
		data [i]->dest = ((char *) dest) + output_bytes_per_sample * i;
		data [i]->distance = data [0]->distance;
	}
	data [channels] = NULL;
	result = WriteFull (data, samples);
	
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
	guint32 result = 0;
	guint32 bytes_per_frame = input_bytes_per_sample * channels;
	guint32 frames_to_write;
	guint32 bytes_available;
	guint32 bytes_written;
	gint32 value;
	guint64 last_frame_pts = 0; // The pts of the last frame which was used to write samples
	guint64 last_frame_samples = 0; // Samples written from the last frame
	IMediaStream *stream;
	
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
	
	stream = GetStreamReffed ();
	if (stream == NULL) {
		LOG_AUDIO ("AudioSource::WriteFull (): no stream.\n");
		return 0;
	}
	
	Lock ();
	
	volume = this->volume * 8192;
	balance = this->balance;
	muted = false; //this->muted;
	
	// Set the per-channel volume
	if (channels > 2) {
		// TODO: how does the balance work here?
		// We probably need a channel map to figure out left and right
		for (unsigned int i = 0; i < channels; i++) {
			volumes [i] = muted ? 0.0 : volume;
		}
	} else if (channels == 2) {
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
			if (frame != NULL) {
				current_frame = new AudioFrame (frame);
				frame->unref ();
			}
		}
		
		if (current_frame == NULL) {
			if (stream->GetOutputEnded ()) {
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
		
		if (bytes_available < bytes_per_frame) {
			LOG_AUDIO ("AudioSource::WriteFull (): incomplete packet, bytes_available: %u, buflen: %u, bytes_used: %u\n", bytes_available, current_frame->frame->buflen, current_frame->bytes_used);
			delete current_frame;
			current_frame = NULL;
			continue;
		}
		
		frames_to_write = MIN (bytes_available / bytes_per_frame, samples - result);
		bytes_written = frames_to_write * bytes_per_frame;
		
#ifdef DUMP_AUDIO	
		fwrite ((((char *) current_frame->frame->buffer) + current_frame->bytes_used), 1, bytes_written, dump_fd);	
#endif

		switch (this->input_bytes_per_sample) {
		case 2: {
			switch (this->output_bytes_per_sample) {
			case 2: {
				// 16bit audio -> 16bit audio
				gint16 *read_ptr = (gint16 *) (((char *) current_frame->frame->buffer) + current_frame->bytes_used);
				
				for (guint32 i = 0; i < frames_to_write; i++) {
					for (guint32 channel = 0; channel < channels; channel++) {
						value = ((*read_ptr) * volumes [channel]) >> 13;
						*(write_ptr [channel]) = (gint16) CLAMP (value, -32768, 32767);
						write_ptr [channel] = (gint16 *) (((char *) write_ptr [channel]) + channel_data [channel]->distance);
						read_ptr++;
					}
				}
				break;
			}
			default: // implement others as needed
				LOG_AUDIO ("AudioSource::Write (): Invalid output_bytes_per_sample, expected 2, got: %i\n", this->output_bytes_per_sample);
				break;
			}
			break;
		}
		case 3: {
			switch (this->output_bytes_per_sample) {
			case 2: {
				// 24bit audio -> 16bit audio
				gint16 *read_ptr = (gint16 *) (((char *) current_frame->frame->buffer) + current_frame->bytes_used);
				
				for (guint32 i = 0; i < frames_to_write; i++) {
					for (guint32 channel = 0; channel < channels; channel++) {
						read_ptr = (gint16 *) (((gint8 *) read_ptr) + 1); // +1 byte
						value = *read_ptr;
						value = (gint16) CLAMP (((value * volumes [channel]) >> 13), -32768, 32767);
						*write_ptr [channel] = value;
						write_ptr [channel] = (gint16 *) (((char *) write_ptr [channel]) + channel_data [channel]->distance);
						read_ptr += 1; // +2 bytes
					}
				}
				break;
			}
			// case 3: // 24bit audio -> 24bit audio, this is painful to both read and write.
			case 4: {
				// 24bit audio -> 32bit audio
				gint32 *read_ptr = (gint32 *) (((char *) current_frame->frame->buffer) + current_frame->bytes_used);
				
				for (guint32 i = 0; i < frames_to_write; i++) {
					for (guint32 channel = 0; channel < channels; channel++) {
						if (false && i > 0) {
							// can overread before, mask out the upper bits.
							value = * (gint32 *) (((gint8 *) read_ptr) - 1);
							value &= 0xFFFFFF00;
						} else {
							// can't overread before, use byte pointers
							value = 0;
							((guint8 *) &value) [1] = (((guint8 *) read_ptr) [0]);
							((guint8 *) &value) [2] = (((guint8 *) read_ptr) [1]);
							((guint8 *) &value) [3] = (((guint8 *) read_ptr) [2]);
						}
						// not sure how to calculate volume here, this shifts down 13 bits
						// and then multiply with volume. This loses the lowest 5 bits of information 
						// from the 24 bit sample. Not quite sure how to do this with 32bit math without
						// losing information though.
						value = (value >> 13) * (volumes [channel]);
						*((gint32 *) write_ptr [channel]) = value;
						write_ptr [channel] = (gint16 *) (((char *) write_ptr [channel]) + channel_data [channel]->distance);
						read_ptr = (gint32 *) (((gint8 *) read_ptr) + 3); // += input_bytes_per_sample;
					}
				}
				break;
			}
			default: // implement others as needed
				LOG_AUDIO ("AudioSource::Write (): Invalid output_bytes_per_sample, expected 2 or 4, got: %i\n", this->output_bytes_per_sample);
				break;
			}
			break;
		}
		default:
			LOG_AUDIO ("AudioSource::Write (): Invalid input_bytes_per_sample, can only be 2 or 3, but got: %i\n", this->input_bytes_per_sample);
			SetState (AudioError);
			break;
		}
		
		result += frames_to_write;
		current_frame->bytes_used += bytes_written;
				
		last_frame_samples = current_frame->bytes_used / GetInputBytesPerFrame ();
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
	LOG_AUDIO_EX ("AudioSource::WriteFull (%p, %u): Wrote %u samples, current pts: %" G_GUINT64_FORMAT ", volume: %.2f\n", channel_data, samples, result, MilliSeconds_FromPts (GetCurrentPts ()), this->volume);
	
	if (result > 0) {
		last_write_pts = last_frame_pts + MilliSeconds_ToPts (last_frame_samples * 1000 / GetSampleRate ());
	}
	
	Unlock ();
	
	if (stream)
		stream->unref ();
	
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

AudioPlayer * AudioPlayer::instance = NULL;
pthread_mutex_t AudioPlayer::instance_mutex = PTHREAD_MUTEX_INITIALIZER;

AudioPlayer::AudioPlayer ()
{
	refcount = 1;
}

void
AudioPlayer::ref ()
{
	g_atomic_int_inc (&refcount);
}

void
AudioPlayer::unref ()
{
	int v = g_atomic_int_exchange_and_add (&refcount, -1) - 1;

	if (v == 0) {
		Dispose ();
		delete this;
	}
}

AudioPlayer *
AudioPlayer::GetInstance ()
{
	AudioPlayer *result;
	pthread_mutex_lock (&instance_mutex);
	result = instance;
	if (result)
		result->ref ();
	pthread_mutex_unlock (&instance_mutex);
	return result;
}

AudioSource *
AudioPlayer::Add (MediaPlayer *mplayer, AudioStream *stream)
{
	AudioPlayer *inst;
	AudioSource *result = NULL;
	
	LOG_AUDIO ("AudioPlayer::Add (%p)\n", mplayer);
	
	if (moonlight_flags & RUNTIME_INIT_DISABLE_AUDIO) {
		LOG_AUDIO ("AudioPlayer: audio is disabled.\n");
		return NULL;
	}
	
	pthread_mutex_lock (&instance_mutex);
	if (instance == NULL) {
		/* here we get a (global) ref which is unreffed in Shutdown () */
		instance = CreatePlayer ();
	}
	inst = instance;
	if (inst)
		inst->ref (); /* this is the ref we unref below */
	pthread_mutex_unlock (&instance_mutex);
	
	if (inst != NULL) {
		result = inst->AddImpl (mplayer, stream);
		inst->unref ();
	}
	
	return result;
}

void
AudioPlayer::Remove (AudioSource *source)
{
	AudioPlayer *inst;
	
	LOG_AUDIO ("AudioPlayer::Remove (%p)\n", source);
	
	inst = GetInstance ();
	if (inst != NULL) {
		inst->RemoveImpl (source);
		inst->unref ();
	}
}

void
AudioPlayer::Shutdown ()
{
	AudioPlayer *player = NULL;

	LOG_AUDIO ("AudioPlayer::Shutdown ()\n");
	
	pthread_mutex_lock (&instance_mutex);
	if (instance != NULL) {
		player = instance;
		instance = NULL;
	}
	pthread_mutex_unlock (&instance_mutex);

	if (player != NULL)
		player->unref ();
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
			result->unref ();
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
			result->unref ();
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
AudioPlayer::Dispose ()
{
	ShutdownImpl ();
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
