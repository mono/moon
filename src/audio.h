/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * audio.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __AUDIO_H__
#define __AUDIO_H__

class AudioStream;
class MediaPlayer;
class MediaFrame;

class AudioSource;
class AudioSources;
class AudioPlayer;

#include "dependencyobject.h"
#include "pipeline.h"

// uncomment to dump raw audio data to /tmp.
// the exact command to play the raw audio file will be printed to stdout
// #define DUMP_AUDIO

enum AudioFlags {
	// The AudioSource has been initialized correctly.
	// This flag is removed if SetState (AudioError) is called.
	AudioInitialized = 1 << 0,
	// The audio source has run out of data to write (the last frame had FrameEventEOF set).
	// There still may be samples in the pipeline somewhere causing audio to be played.
	// This flag is removed when AppendFrame is called.
	AudioEOF         = 1 << 1,
	// The audio source has run out of data to write, and is waiting for more.
	// There still may be samples in the pipeline somewhere causing audio to be played.
	// This flag is removed when AppendFrame is called.
	AudioWaiting     = 1 << 2,
	// The audio source has run out of data to write and has played all available samples.
	// This flag is removed when Play/Pause/Stop/AppendFrame is called.
	AudioEnded       = 1 << 3,
};

enum AudioState {
	AudioNone, // Initial state.
	AudioError, // An error has occured.
	AudioPlaying, // Play has been called
	AudioPaused, // Pause has been called
	AudioStopped, // Stop has been called (or we've played all the available data).
};

struct AudioData {
	void *dest; // Audio samples
	gint32 distance; // The distance between samples (in bytes)
};

// All AudioSource's public methods must be safe to call from any thread.
class AudioSource : public EventObject {
 private:
	struct AudioFrame {
		MediaFrame *frame;
		guint32 bytes_used;
		AudioFrame (MediaFrame *frame);
		~AudioFrame ();
	};

	MediaPlayer *mplayer;
	AudioStream *stream;
	AudioPlayer *player;
	AudioFrame *current_frame;
	AudioState state;
	AudioFlags flags; 
	
	double balance;
	double volume;
	bool muted;
	
	guint64 last_write_pts; // The last pts written
	guint64 last_current_pts; // The last value returned from GetCurrentPts
		
	guint32 channels; // The number of channels
	guint32 sample_rate; // The sample rate in the audio source
	guint32 input_bytes_per_sample; // The number of bytes per sample
	guint32 output_bytes_per_sample; // The number of bytes per sample in the output. Defaults to same as input_bytes_per_sample.
	
	pthread_mutex_t mutex;
	
	void Lock ();
	void Unlock ();
	
	MediaPlayer *GetMediaPlayerReffed ();
	
	EVENTHANDLER (AudioSource, FirstFrameEnqueued, EventObject, EventArgs);
	
#ifdef DUMP_AUDIO
	FILE *dump_fd;
#endif

 protected:
	AudioSource (AudioPlayer *player, MediaPlayer *mplayer, AudioStream *stream);
	virtual ~AudioSource ();
		
	// Writes frames to the specified destination
	// Returns the number of frames actually written
	// frame: consists of 1 audio sample of input_bytes_per_sample bytes * number of channels
	guint32 Write (void *dest, guint32 samples);
	guint32 WriteFull (AudioData **channel_data /* Array of info about channels, NULL ended. */, guint32 samples);
	
	virtual void Played () { }
	virtual void Paused () { }
	virtual void Stopped () { }
	
	// The deriving class must call this method when it has finished playing
	// all the written samples.
	void Underflowed ();
	
	// Called whenever the state changes
	virtual void StateChanged (AudioState old_state) {}
	
	// Must return the time difference between the last written sample
	// and what the audio hw is playing now (in pts).
	// This method will only be called if GetState () == AudioPlaying
	// Must return G_MAXUINT64 in case of any errors.
	virtual guint64 GetDelayInternal () = 0;
	
	virtual bool InitializeInternal () { return true; }
	
	// There's no guarantee CloseInternal won't be called more than once.
	virtual void CloseInternal () {};
	
 public:
 	virtual void Dispose ();
 	
	void Play ();
	void Pause ();
	void Stop ();
	guint64 GetDelay ();
	
	// Initialize(Internal) is called before adding the source to the list of sources
	// If Initialize fails (returns false), the source is not added to the list of sources.
	bool Initialize ();
	
	// Close(Internal) is called after removing the source from the list of sources
	void Close ();

	// This method may return G_MAXUINT64 if it has no idea which is the current pts.
	// This may happen if nothing has been done yet (the derived audio source hasn't
	// requested any writes).
	guint64 GetCurrentPts ();
	guint32 GetInputBytesPerFrame ();
	guint32 GetOutputBytesPerFrame ();
	guint32 GetInputBytesPerSample ();
	guint32 GetOutputBytesPerSample ();
	
	/* This method must only be called during initilization so that output_bytes_per_sample is thread-safe without locking */
	void SetOutputBytesPerSample (guint32 value);
	
	AudioStream *GetStreamReffed ();
	bool IsQueueEmpty ();

	AudioState GetState ();
	void SetState (AudioState value);
	static const char *GetStateName (AudioState state);
	
	void SetFlag (AudioFlags, bool value);
	bool GetFlag (AudioFlags flag);
	
#if LOGGING
	static char *GetFlagNames (AudioFlags flags);
#endif

	guint32 GetChannels ();
	guint32 GetSampleRate ();

	double GetBalance ();
	void SetBalance (double value);
	
	double GetVolume ();
	void SetVolume (double value);
	
	bool GetMuted ();
	void SetMuted (bool value);

	void SetAudioStream (AudioStream *value);
	AudioStream *GetAudioStream ();

	bool IsPlaying () { return GetState () == AudioPlaying && !GetFlag (AudioWaiting); }

	virtual const char *GetTypeName () { return "AudioSource"; }
};

class AudioListNode : public List::Node {
 public:
	AudioSource *source;
	gint32 generation;
	
	AudioListNode (AudioSource *source);
	virtual ~AudioListNode ();
};

class AudioSources {
	pthread_mutex_t mutex;
	List list;
	gint32 current_generation;
	AudioListNode *last_node; // The last node returned by GetNext.
	
	void Lock ();
	void Unlock ();
	
 public:
	AudioSources ();
	~AudioSources ();
	
	void Add (AudioSource *node);
	// Returns true if the node existed in the list
	bool Remove (AudioSource *node);
	
	// Enumerating all sources:
	// First call StartEnumeration, then call GetNext until NULL is returned.
	// Only one enumeration can be going at the same time, otherwise they'll 
	// interfere with eachother.
	// GetNext returns the AudioSource reffed, the caller must unref.
	void StartEnumeration ();
	AudioSource *GetNext (bool only_playing);
	
	// Returns the first AudioSource in the list (reffed, the caller must unref)
	// Returns NULL if the list is empty.
	AudioSource *GetHead ();
#if DEBUG
	int Length ();
#endif
};

class AudioPlayer {
	// our AudioPlayer instance
	static AudioPlayer *instance;
	static pthread_mutex_t instance_mutex;
	
	static AudioPlayer *CreatePlayer ();
		

	AudioSource *AddImpl (MediaPlayer *mplayer, AudioStream *stream);
	void RemoveImpl (AudioSource *node);
	void ShutdownImpl ();
	
	static AudioPlayer *GetInstance ();

	/*
	 * We use our own refcounting here, since we can't derive from EventObject
	 * (which is always a per deployment object, while AudioPlayer is per-process).
	 * As with EventObject, the AudioPlayer will be deleted once refcount reaches 0.
	 */
	gint32 refcount;
	void ref ();
	void unref ();

 protected:
	// The list of all the audio sources.
	// This is protected so that derived classes can enumerate the sources,
	// derived classes must not add/remove sources.
	AudioSources sources;
	
	AudioPlayer ();
	virtual ~AudioPlayer () {}
	virtual void Dispose ();
	
	// called after the node has been created and added to the list of sources
	virtual void AddInternal (AudioSource *node) = 0;
	// called after the node has been removed from the list of sources, but before the node is deleted.			
	// not called if the node isn't in the list of sources.
	virtual void RemoveInternal (AudioSource *node) = 0;
	 // called just after ctor. 
	virtual bool Initialize () = 0;
	// before all the nodes will be removedthis method is called
	// to ensure that we have stopped the play loop
	virtual void PrepareShutdownInternal () = 0;
	// all the nodes will have been removed when this method is called
	// after this call has returned, the player will be deleted immediately.
	virtual void FinishShutdownInternal () = 0;
	// Must return a new AudioSource specific for each backend.
	virtual AudioSource *CreateNode (MediaPlayer *mplayer, AudioStream *stream) = 0;
	
 public:
	// Creates a audio source from the MediaPlayer and AudioStream.
	// Returns NULL if there were any errors.
	// Note: Actually returning an object doesn't mean audio will be played.
	// some backends are async and will only cause an error to be raised
	// later (in which case the AudioSource's state would be AudioError)
	static AudioSource *Add (MediaPlayer *mplayer, AudioStream *stream);
	// Removes an audio source.
	static void Remove (AudioSource *source);
	// Shuts down the audio engine
	static void Shutdown ();
};

#endif /* __AUDIO_H__ */
