/*
 * mplayer.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
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
	uint64_t pts_per_frame;
	
	Audio ();
};

struct Video {
	Queue queue;
	
	// input
	VideoStream *stream;
	
	// rendering
	cairo_surface_t *surface;
	uint8_t *rgb_buffer;
	
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
		LoadFramePending	= (1 << 5),
		// after seeking, we don't want to show any frames until the video has synced with
		// the audio. Since the video seeks to key frames, and there can be several seconds
		// between key frames, after seeking we will decode video as fast as possible to 
		// catch up with the audio.
		SeekSynched			= (1 << 6),
		RenderedFrame		= (1 << 7),
		Eof					= (1 << 8),
		Opened				= (1 << 9),
		CanSeek				= (1 << 10),
		CanPause			= (1 << 11),
		// If we should stop playing when we reach the duration
		// Used to support the Duration tag in asx files.
		FixedDuration		= (1 << 12), 
	};
	
 private:
	MediaElement *element;
	Media *media;
	PlayerState state;
	int32_t height;
	int32_t width;
	
	// sync
	pthread_mutex_t target_pts_lock;
	uint64_t start_time; // 100-nanosecond units (pts)
	uint64_t duration; // 100-nanosecond units (pts)
	// This is the pts we start playing (0 is still the first pts in the media).
	uint64_t start_pts; // 100-nanosecond units (pts)
	uint64_t current_pts; // 100-nanosecond units (pts)
	uint64_t target_pts; // 100-nanosecond units (pts)
	
	bool LoadVideoFrame ();
	void Initialize ();
	
	void SeekInternal (uint64_t pts/* 100-nanosecond units (pts) */);
	void RenderFrame (MediaFrame *frame);
	static MediaResult SeekCallback (MediaClosure *closure);
	static MediaResult FrameCallback (MediaClosure *closure);
	
	static void EnqueueVideoFrameCallback (void *user_data);
	static void EnqueueAudioFrameCallback (void *user_data);
	static void LoadFrameCallback (void *user_data);
	static void AudioFinishedCallback (void *user_data);
	
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
	void Seek (uint64_t pts /* 100-nanosecond units (pts) */);
	
	cairo_surface_t *GetCairoSurface () { return video.surface; }
	int32_t GetTimeoutInterval ();
	
	int GetAudioStreamCount () { return audio.stream_count; }
	Media *GetMedia () { return media; }
	
	bool HasVideo () { return video.stream != NULL; }
	bool HasAudio () { return audio.stream != NULL; }
	
	uint64_t GetPosition () { return GetTargetPts (); }
	uint64_t GetDuration () { return duration; }
	
	void SetMuted (bool muted) { audio.muted = muted; }
	bool GetMuted () { return audio.muted; }
	
	int32_t GetVideoHeight () { return height; }
	int32_t GetVideoWidth () { return width; }
	
	double GetBalance () { return audio.balance; }
	void SetBalance (double balance);
	
	bool GetEof () { return state & Eof; }
	void SetEof (bool value);
	
	double GetVolume () { return audio.volume; }
	void SetVolume (double volume);
	
	void SetTargetPts (uint64_t pts);
	uint64_t GetTargetPts ();
};

class AudioPlayer {
 public:
	enum AudioState {
		Playing,
		Paused,
		Stopped,
		WaitingForData
	};
	
	struct AudioNode  {
		MediaPlayer *mplayer;
		
		snd_pcm_t *pcm;
		snd_pcm_hw_params_t *hwparams;
		snd_pcm_sw_params_t *swparams;
		snd_pcm_uframes_t sample_size;
		pollfd *udfs;
		int ndfs;
		snd_pcm_uframes_t buffer_size;
		
		AudioState state;		
		bool started;
		
		uint8_t *first_buffer;
		uint32_t first_used;
		uint32_t first_size;
		uint64_t first_pts;
		
		uint64_t updated_pts;
		uint64_t sent_pts;
		uint64_t sent_samples;
		
		AudioNode ();
		~AudioNode ();
		
		bool GetNextBuffer ();
		bool Initialize ();
		bool SetupHW ();
		bool PreparePcm (snd_pcm_sframes_t *avail);
		
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
	
	// This value will be false if initialization fails (no audio devices, etc).
	bool initialized;
	
	// The Loop will exit once this value is true
	bool shutdown;
	
	// A list of all the audio nodes.
	AudioNode **list;
	uint32_t list_size;
	uint32_t list_count;
	
	sem_t semaphore;
	
	// A list of all the file descriptors in all the 
	// audio nodes. We need to poll on changes in any of the 
	// descriptors, so we create a big list with all of them
	// and poll on that.
	pollfd *udfs;
	int ndfs;
	void UpdatePollList (bool locked);
	
	// We also need to be able to wake up from the poll
	// whenever we want to, so we create a pipe which we
	// poll on. This is always the first file descriptor
	// in udfs.
	int fds [2];
	
	AudioPlayer ();
	~AudioPlayer ();
	
	void StartThread ();
	void StopThread ();
	
	AudioNode *Find (MediaPlayer *mplayer);
	
	// The audio thread	
	pthread_t *audio_thread;
	
	// The audio loop which is executed 
	// on the audio thread.
	static void *Loop (void *data);
	void Loop ();
	
	// our AudioPlayer instance
	static AudioPlayer *instance;
	static AudioPlayer *Instance ();
	static bool Initialize ();
	
	void Lock (); // tries to wait on the semaphore, and if not successful, wakes up the audio thread and tries again.
	void SimpleLock (); // just waits on the semaphore
	void Unlock ();
	
	bool AddInternal (MediaPlayer *mplayer);
	void RemoveInternal (MediaPlayer *mplayer);
	void PauseInternal (MediaPlayer *mplayer, bool value);
	void StopInternal (MediaPlayer *mplayer);
	void PlayInternal (MediaPlayer *mplayer);
	void WaitForData (AudioNode *node);
	
 public:
	// None of the following functions are thread-safe, they must all be called from 
	// the main thread.
	static bool Add (MediaPlayer *mplayer);
	static void Remove (MediaPlayer *mplayer);
	static void Pause (MediaPlayer *mplayer, bool value);
	static void Stop (MediaPlayer *mplayer);
	static void Play (MediaPlayer *mplayer);
	static void WakeUp ();
	static void Shutdown ();
};

#endif /* __MOON_MPLAYER_H__ */
