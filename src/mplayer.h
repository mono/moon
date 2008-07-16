/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mplayer.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_MPLAYER_H__
#define __MOON_MPLAYER_H__

#include <glib.h>
#include <cairo.h>
#include <stdint.h>
#include <pthread.h>
#include <poll.h>
#include <asoundlib.h>
#include <semaphore.h>

class MediaPlayer;

#include "asf/asf.h"
#include "pipeline.h"

struct Audio {
	Queue queue;
	
	double balance;
	double volume;
	bool muted;
	
	// input
	int stream_count;
	AudioStream *stream;
	
	// sync
	guint64 pts_per_frame;
	
	Audio ();
};

struct Video {
	Queue queue;
	
	// input
	VideoStream *stream;
	
	// rendering
	cairo_surface_t *surface;
	guint8 *rgb_buffer;
	
	Video ();
};

class MediaPlayer : public EventObject {
 public:
	enum PlayerState {
		// These are not flags, but mutually exclusive states.
		Stopped				= 0,
		Paused				= 1,
		Playing				= 2,
		StateMask			= 3,
		
		Seeking				= (1 << 4),
		// If we're waiting for a frame to show immediately
		LoadFramePending		= (1 << 5),
		// after seeking, we don't want to show any frames until the video has synced with
		// the audio. Since the video seeks to key frames, and there can be several seconds
		// between key frames, after seeking we will decode video as fast as possible to 
		// catch up with the audio.
		SeekSynched			= (1 << 6),
		RenderedFrame			= (1 << 7),
		Eof		       		= (1 << 8),
		Opened				= (1 << 9),
		CanSeek				= (1 << 10),
		CanPause			= (1 << 11),
		// If we should stop playing when we reach the duration
		// Used to support the Duration tag in asx files.
		FixedDuration			= (1 << 12), 
	};
	
 private:
	MediaElement *element;
	Media *media;
	PlayerState state;
	gint32 height;
	gint32 width;
	
	// sync
	pthread_mutex_t target_pts_lock;
	guint64 start_time; // 100-nanosecond units (pts)
	guint64 duration; // 100-nanosecond units (pts)
	// This is the first pts with live streams (when the first pts might not be 0).
	guint64 first_live_pts; // 100-nanosecond units (pts)
	// This is the pts we start playing (0 is still the first pts in the media).
	guint64 start_pts; // 100-nanosecond units (pts)
	guint64 current_pts; // 100-nanosecond units (pts)
	guint64 target_pts; // 100-nanosecond units (pts)
	
	bool LoadVideoFrame ();
	void Initialize ();
	
	void SeekInternal (guint64 pts/* 100-nanosecond units (pts) */);
	void RenderFrame (MediaFrame *frame);
	static MediaResult SeekCallback (MediaClosure *closure);
	static MediaResult FrameCallback (MediaClosure *closure);
	
	static void EnqueueVideoFrameCallback (EventObject *user_data);
	static void EnqueueAudioFrameCallback (EventObject *user_data);
	static void LoadFrameCallback (EventObject *user_data);
	static void AudioFinishedCallback (EventObject *user_data);
	
 protected:
	virtual ~MediaPlayer ();
	
 public:
	Audio audio;
	Video video;
	
	MediaPlayer (MediaElement *element);
	
	// Returns true if advanced at least one frame.
	// A false return value does not say anything about why it didn't advance
	// (No need to advance, eof, seeking, etc). 
	bool AdvanceFrame (); 
	
	bool Open (Media *media);
	void Close (bool dtor);
	void EnqueueFrames (int audio_frames, int video_frames);
	void EnqueueFramesAsync (int audio_frames, int video_frames);
	
	bool IsPlaying () { return (state & StateMask) == Playing; }
	bool IsPaused () { return (state & StateMask) == Paused; }
	bool IsStopped () { return (state & StateMask) == Stopped; }
	bool IsSeeking () { return (state & Seeking) == Seeking; }
	bool IsLoadFramePending () { return (state & LoadFramePending); }
	bool HasRenderedFrame () { return (state & RenderedFrame); }
	void AudioFinished (); // Called by the audio player when audio reaches the end (this method is thread-safe).
	
	void SetBit (PlayerState s) { state = (PlayerState) (s | state); }
	void RemoveBit (PlayerState s) { state = (PlayerState) (~s & state); }
	void SetBitTo (PlayerState s, bool value) { if (value) SetBit (s); else RemoveBit (s); }
	bool GetBit (PlayerState s) { return (state & s) == s; }
	void SetState (PlayerState s) { state = (PlayerState) ((state & ~StateMask) | s); }
	
	void Play ();
	bool GetCanPause ();
	void SetCanPause (bool value);
	void Pause ();
	void Stop (bool seek_to_start = true);
	bool MediaEnded ();
	
	void SetCanSeek (bool value);
	bool GetCanSeek ();
	void Seek (guint64 pts /* 100-nanosecond units (pts) */);
	
	void SeekCallback ();
	static void SeekCallback (EventObject *mplayer);
	virtual void SetSurface (Surface *surface);
	
	cairo_surface_t *GetCairoSurface () { return video.surface; }
	gint32 GetTimeoutInterval ();
	
	int GetAudioStreamCount () { return audio.stream_count; }
	Media *GetMedia () { return media; }
	
	bool HasVideo () { return video.stream != NULL; }
	bool HasAudio () { return audio.stream != NULL; }
	
	guint64 GetPosition () { return GetTargetPts (); }
	guint64 GetDuration () { return duration; }
	
	void SetMuted (bool muted) { audio.muted = muted; }
	bool GetMuted () { return audio.muted; }
	
	gint32 GetVideoHeight () { return height; }
	gint32 GetVideoWidth () { return width; }
	
	double GetBalance () { return audio.balance; }
	void SetBalance (double balance);
	
	bool GetEof () { return state & Eof; }
	void SetEof (bool value);
	
	double GetVolume () { return audio.volume; }
	void SetVolume (double volume);
	
	void SetTargetPts (guint64 pts);
	guint64 GetTargetPts ();
	
#if OBJECT_TRACKING
	virtual const char * GetTypeName () { return "MediaPlayer"; }
#endif
};

class AudioPlayer {
 public:
	enum AudioState {
		Playing,
		Paused,
		Stopped,
		WaitingForData
	};
	
	enum AudioAction {
		ActionPlay,
		ActionPause,
		ActionRestart,
		ActionStop,
		ActionRemove,
		ActionAdd,
		ActionDrain,
		ActionShutdown
	};
	
	struct AudioNode  {
	private:
		MediaPlayer *mplayer;
		
	public:
		snd_pcm_t *pcm;
		snd_pcm_hw_params_t *hwparams;
		snd_pcm_sw_params_t *swparams;
		snd_pcm_uframes_t sample_size;
		bool mmap;
		pollfd *udfs;
		int ndfs;
		snd_pcm_uframes_t buffer_size;
		
		AudioState state;		
		bool started;
		
		guint8 *first_buffer;
		guint32 first_used;
		guint32 first_size;
		guint64 first_pts;
		
		guint64 updated_pts;
		guint64 sent_pts;
		guint64 sent_samples;
		guint32 channels;
		int sample_rate;
		
		AudioNode (MediaPlayer *mplayer);
		~AudioNode ();
		
		bool GetNextBuffer ();
		bool Initialize ();
		bool SetupHW ();
		bool PreparePcm (snd_pcm_sframes_t *avail);
		MediaPlayer *GetMediaPlayer () { return mplayer; }
		
		// Underrun recovery
		// Handles EPIPE and ESTRPIPE, prints an error if recovery failed
		// or if err isn't any of the above values.
		bool XrunRecovery (int err);
		
		// Pushes data onto the pcm device if the
		// device can accept more data, and if the
		// there is data availabe. The node has to 
		// be locked during playback.
		// Returns false if nothing has been played
		bool Play ();
		
		void Close ();
	};
	
	class AudioListNode : public List::Node {
	public:
		AudioAction action;
		MediaPlayer *mplayer;
		AudioListNode (MediaPlayer *mplayer, AudioAction action);
		virtual ~AudioListNode ();
	};
	
	Queue work;
	
	// A list of all the audio nodes.
	AudioNode **list;
	guint32 list_size;
	guint32 list_count;
	
	// A list of all the file descriptors in all the 
	// audio nodes. We need to poll on changes in any of the 
	// descriptors, so we create a big list with all of them
	// and poll on that.
	pollfd *udfs;
	int ndfs;
	void UpdatePollList ();
	
	// We also need to be able to wake up from the poll
	// whenever we want to, so we create a pipe which we
	// poll on. This is always the first file descriptor
	// in udfs.
	int fds [2];
	
	AudioPlayer ();
	~AudioPlayer ();
	
	AudioNode *Find (MediaPlayer *mplayer);
	
	// The audio thread	
	pthread_t audio_thread;
	
	// The audio loop which is executed 
	// on the audio thread.
	static void *Loop (void *data);
	void Loop ();
	
	// our AudioPlayer instance
	static AudioPlayer *instance;
	static pthread_mutex_t instance_mutex;
	
	void AddInternal (MediaPlayer *mplayer);
	void RemoveInternal (MediaPlayer *mplayer);
	void PauseInternal (MediaPlayer *mplayer, bool value);
	void StopInternal (MediaPlayer *mplayer);
	void PlayInternal (MediaPlayer *mplayer);
	void DrainInternal (MediaPlayer *mplayer);
	void ShutdownInternal ();
	void WaitForData (AudioNode *node);
	
	void AddWork (MediaPlayer *mplayer, AudioAction action);
	
 public:
	// The following methods are thread-safe.
	// They are also async, the actions are just passed 
	// on to the audio thread.
	static void Add (MediaPlayer *mplayer);
	static void Remove (MediaPlayer *mplayer);
	static void Pause (MediaPlayer *mplayer, bool value);
	static void Stop (MediaPlayer *mplayer);
	static void Play (MediaPlayer *mplayer);
	static void Drain (MediaPlayer *mplayer);
	static void WakeUp ();
	static void Shutdown ();
};

#endif /* __MOON_MPLAYER_H__ */
