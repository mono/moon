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

struct Audio;
struct Video;

class MediaPlayer : public EventObject {
private:
	gint eof;
	void StopThreads ();

protected:
	virtual ~MediaPlayer ();

public:
	Media *media;
	
	pthread_mutex_t pause_mutex;
	pthread_cond_t pause_cond;
	bool paused;
	
	bool playing;
	bool stop;
	bool seeking;
	bool load_frame; // If we're waiting for a frame to show immediately
	// after seeking, we don't want to show any frames until the video has synced with
	// the audio. Since the video seeks to key frames, and there can be several seconds
	// between key frames, after seeking we will decode video as fast as possible to 
	// catch up with the audio.
	bool caught_up_with_seek;
	MediaElement *element;
	
	GThread *audio_thread;
	Audio *audio;
	Video *video;
	
	bool rendered_frame;
	
	gint32 GetTimeoutInterval ();

	// sync
	pthread_mutex_t target_pts_lock;
	uint64_t start_time; // 100-nanosecond units (pts)
	uint64_t duration; // 100-nanosecond units (pts)
	uint64_t current_pts; // 100-nanosecond units (pts)
	uint64_t target_pts; // 100-nanosecond units (pts)
	
	/* Public API */
	
	// read-only
	int width, height;
	bool opened;
	
	MediaPlayer (MediaElement *element);
	
	// Returns true if advanced at least one frame.
	// A false return value does not say anything about why it didn't advance
	// (No need to advance, eof, seeking, etc). 
	bool AdvanceFrame (); 
	bool LoadVideoFrame ();
	cairo_surface_t *GetCairoSurface ();
	
	bool Open (Media *media);
	void Initialize ();
	void Close ();
	
	bool IsPlaying ();
	bool MediaEnded ();
	void Play ();
	bool CanPause ();
	bool IsPaused ();
	void Pause ();
	void PauseInternal (bool pause);
	void Stop ();
	
	bool CanSeek ();
	void Seek (uint64_t pts /* 100-nanosecond units (pts) */);
	void SeekInternal (uint64_t pts/* 100-nanosecond units (pts) */);
	uint64_t Position ();
	uint64_t Duration ();
	
	void SetMuted (bool muted);
	bool IsMuted ();
	
	int GetAudioStreamCount ();
	int GetAudioStreamIndex ();
	bool HasVideo ();
	bool HasAudio ();
	
	double GetBalance ();
	void SetBalance (double balance);
	
	// Thread safe property accessors.
	bool GetEof ();
	void SetEof (bool value); 

	double GetVolume ();
	void SetVolume (double volume);
	void SetTargetPts (uint64_t pts);
	uint64_t GetTargetPts ();
	void IncTargetPts (uint64_t value);
};

class AudioPlayer {
public:
	enum AudioState {
		Playing,
		Paused,
		Stopped
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

	AudioNode* Find (MediaPlayer *mplayer);

	// The audio thread	
	pthread_t audio_thread;

	// The audio loop which is executed 
	// on the audio thread.
	static void* Loop (void *data);
	void Loop ();

	// our AudioPlayer instance
	static AudioPlayer* instance;
	static AudioPlayer* Instance ();
	static bool Initialize ();

	void Lock (); // tries to wait on the semaphore, and if not successful, wakes up the audio thread and tries again.
	void SimpleLock (); // just waits on the semaphore
	void Unlock ();

	bool AddInternal (MediaPlayer *mplayer);
	void RemoveInternal (MediaPlayer *mplayer);
	void PauseInternal (MediaPlayer *mplayer, bool value);
	void StopInternal (MediaPlayer *mplayer);
	void PlayInternal (MediaPlayer *mplayer);
	void WakeUpInternal ();
	
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
