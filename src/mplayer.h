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
	
	GThread *audio_thread;
	Audio *audio;
	Video *video;
	
	bool rendered_frame;

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
	
	MediaPlayer ();
	~MediaPlayer ();
	
	// Returns true if advanced at least one frame.
	// A false return value does not say anything about why it didn't advance
	// (No need to advance, eof, seeking, etc). 
	bool AdvanceFrame (); 
	void LoadVideoFrame ();
	void Render (cairo_t *cr);
	cairo_surface_t *GetSurface ();
	
	bool Open (Media *media);
	void Close ();
	
	bool IsPlaying ();
	bool MediaEnded ();
	guint Play (GSourceFunc callback, void *user_data);
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
