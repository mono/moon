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
	// The pts when we start playing (set when resuming playback to current pts, when starting to play to initial pts, and when seeking to the seeked pts)
	// While playing it can be used to calculate the current pts (knowing the time)
	guint64 start_pts;
	guint64 initial_pts;
	gint64 duration;
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
	uint64_t pause_time;
	uint64_t start_time;  
	
	pthread_mutex_t target_pts_lock;
	int64_t current_pts;
	int64_t target_pts;
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
	void Seek (int64_t position);
	int64_t Position ();
	int64_t Duration ();
	
	void Mute ();
	void UnMute ();
	bool IsMuted ();
	
	int GetAudioStreamCount ();
	int GetAudioStreamIndex ();
	bool HasVideo ();
	bool HasAudio ();
	
	double GetBalance ();
	void SetBalance (double balance);
	
	double GetVolume ();
	void SetVolume (double volume);
	
private:
	
	void StopThreads ();
};

#endif /* __MOON_MPLAYER_H__ */
