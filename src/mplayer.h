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
	
	GThread *audio_thread;
	Audio *audio;
	Video *video;
	
	// sync
	uint64_t start_time;  
	
	pthread_mutex_t target_pts_lock;
	uint64_t initial_pts;
	uint64_t current_pts;
	uint64_t target_pts;
	
	/* Public API */
	
	// read-only
	int width, height;
	bool opened;
	
	MediaPlayer ();
	~MediaPlayer ();
	
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
	void Stop ();
	
	bool CanSeek ();
	void Seek (uint64_t position);
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
