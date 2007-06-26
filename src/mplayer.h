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

struct AVFormatContext;
struct Audio;
struct Video;

class MediaPlayer {
public:
	char *uri;
	
	GStaticMutex pause_mutex;
	bool paused;
	
	bool stop;
	
	AVFormatContext *av_ctx;
	
	GThread *audio_thread;
	GThread *io_thread;
	
	Audio *audio;
	Video *video;
	
	// sync
	uint64_t pause_time;
	uint64_t start_time;  
	
	uint64_t target_pts;
	
	// read-only
	int width, height;
	bool opened;
	
	MediaPlayer ();
	~MediaPlayer ();
	
	bool AdvanceFrame ();
	void Render (cairo_t *cr);
	cairo_surface_t *GetSurface ();
	
	bool Open ();
	bool Open (const char *uri);
	void Close ();
	
	bool IsPlaying ();
	guint Play (GSourceFunc callback, void *user_data);
	void Pause ();
	void Stop ();
	
	bool CanSeek ();
	void Seek (int64_t position);
	int64_t Position ();
	int64_t Duration ();
	
	void Mute ();
	void UnMute ();
	bool IsMuted ();
	
	double GetBalance ();
	void SetBalance (double balance);
	
	double GetVolume ();
	void SetVolume (double volume);
};

#endif /* __MOON_MPLAYER_H__ */
