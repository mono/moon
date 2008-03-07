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

class MediaPlayer;

#include "asf/asf.h"
#include "pipeline.h"

struct Audio;
struct Video;

class MediaPlayer {
public:
	Media *media;
	
	pthread_mutex_t pause_mutex;
	pthread_cond_t pause_cond;
	bool paused;
	
	bool playing;
	bool stop;
	bool eof;
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
	~MediaPlayer ();
	
	// Returns true if advanced at least one frame.
	// A false return value does not say anything about why it didn't advance
	// (No need to advance, eof, seeking, etc). 
	bool AdvanceFrame (); 
	bool LoadVideoFrame ();
	cairo_surface_t *GetSurface ();
	
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
	
	double GetVolume ();
	void SetVolume (double volume);
	void SetTargetPts (uint64_t pts);
	uint64_t GetTargetPts ();
	void IncTargetPts (uint64_t value);
	
private:
	void StopThreads ();
};

#endif /* __MOON_MPLAYER_H__ */
