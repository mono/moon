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

class MediaPlayer;

#include "asf/asf.h"
#include "pipeline.h"
#include "audio.h"

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
		Opened				= (1 << 9),
		CanSeek				= (1 << 10),
		CanPause			= (1 << 11),
		// If we should stop playing when we reach the duration
		// Used to support the Duration tag in asx files.
		FixedDuration			= (1 << 12),
		// If Audio/Video has finished playing 
		AudioEnded			= (1 << 13),
		VideoEnded			= (1 << 14),
	};
	
 private:
	AudioSource *audio;
	Video video;
	
	MediaElement *element;
	Media *media;
	PlayerState state;
	gint32 height;
	gint32 width;
	int audio_stream_count;
	
	// sync
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
	void CheckFinished ();
	static void NotifyFinishedCallback (EventObject *player);
	void NotifyFinished ();
	
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
	void VideoFinished (); // not thread safe.
	void AudioFinished (); // Called by the audio player when audio reaches the end (this method is thread-safe).
	void AudioFailed (AudioSource *source); // Called by the audio engine if audio failed to load (async)
	
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
	
	void SetCanSeek (bool value);
	bool GetCanSeek ();
	void Seek (guint64 pts /* 100-nanosecond units (pts) */);
	
	void SeekCallback ();
	static void SeekCallback (EventObject *mplayer);
	virtual void SetSurface (Surface *surface);
	
	cairo_surface_t *GetCairoSurface () { return video.surface; }
	gint32 GetTimeoutInterval ();
	
	int GetAudioStreamCount () { return audio_stream_count; }
	Media *GetMedia () { return media; }
	
	bool HasVideo () { return video.stream != NULL; }
	bool HasAudio () { return audio != NULL; }
	
	guint64 GetPosition () { return GetTargetPts (); }
	guint64 GetDuration () { return duration; }
	
	void SetMuted (bool muted);
	bool GetMuted ();
	
	gint32 GetVideoHeight () { return height; }
	gint32 GetVideoWidth () { return width; }
	
	double GetBalance ();
	void SetBalance (double balance);
	
	double GetVolume ();
	void SetVolume (double volume);
	
	guint64 GetTargetPts ();
	
	virtual const char * GetTypeName () { return "MediaPlayer"; }
};

#endif /* __MOON_MPLAYER_H__ */
