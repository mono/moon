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

#ifndef __MOON_MEDIAPLAYER_H__
#define __MOON_MEDIAPLAYER_H__

#include <glib.h>
#include <cairo.h>

#include "pipeline.h"
#include "audio.h"
#include "mutex.h"

/* @Namespace=None,ManagedEvents=Manual */
class MediaPlayer : public EventObject {
 public:
	enum PlayerState {
		// These are not flags, but mutually exclusive states.
		Stopped				= 0,
		Paused				= 1,
		Playing				= 2,
		StateMask			= 3,
		
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
		BufferUnderflow     = (1 << 15),
		IsLive	      = (1 << 16),
	};
	
 private:
 	// Some instance variables can be accessed from multiple threads.
 	// This mutex must be locked while these variables are accessed from
 	// any thread.
 	Mutex mutex; 
	AudioSource *audio_unlocked; // mutex must be locked.
	VideoStream *video_stream;
	// rendering
	cairo_surface_t *surface;
	guint8 *rgb_buffer;
	gint32 buffer_width;
	gint32 buffer_height;
	gint32 seeks; // the count of pending seeks. write on main thread only.
	
	MediaElement *element;
	Media *media;
	PlayerState state_unlocked; // mutex must be locked
	gint32 height;
	gint32 width;
	MoonPixelFormat format;
	int audio_stream_count;
	int advance_frame_timeout_id;
	
	// sync
	guint64 start_time; // 100-nanosecond units (pts)
	guint64 duration; // 100-nanosecond units (pts)
	// This is the first pts with live streams (when the first pts might not be 0).
	guint64 first_live_pts; // 100-nanosecond units (pts)
	// This is the pts we start playing (0 is still the first pts in the media).
	guint64 start_pts; // 100-nanosecond units (pts)
	guint64 current_pts; // 100-nanosecond units (pts)
	guint64 target_pts; // 100-nanosecond units (pts)

	// These variables are used to implement RenderedFramesPerSecond and DroppedFramesPerSecond
	guint64 frames_update_timestamp;
	guint32 dropped_frames;
	guint32 rendered_frames;
	double rendered_frames_per_second;
	double dropped_frames_per_second;

	static void LoadVideoFrameCallback (EventObject *object);
	void LoadVideoFrame ();
	void Initialize ();
	void CheckFinished ();
	
	void RenderFrame (MediaFrame *frame);
	
	EVENTHANDLER (MediaPlayer, SeekCompleted, Media, EventArgs); // Not thread-safe
	EVENTHANDLER (MediaPlayer, FirstFrameEnqueued, EventObject, EventArgs); // Not thread-safe
	
	// Thread-safe
	static void AudioFinishedCallback (EventObject *user_data);
	
	void SetVideoBufferSize (gint32 height, gint32 width);
	void SetTimeout (gint32 interval /* set to 0 to clear */);
	void AdvanceFrame ();
	static gboolean AdvanceFrameCallback (void *user_data);
	
	void EmitBufferUnderflow ();
	static void EmitBufferUnderflowAsync (EventObject *obj);
	
	void StopAudio (); // Not thread-safe
 protected:
	virtual ~MediaPlayer ();
	
 public:
	MediaPlayer (MediaElement *element);
	virtual void Dispose ();
	
	bool Open (Media *media, PlaylistEntry *entry);
	void Close ();

	// Thread-safe.
	// Returns a refcounted AudioStream.
	// Caller must call unref when done with it.
	AudioSource *GetAudio (); 

	bool IsPlaying (); // thread safe
	bool IsPaused (); // thread safe
	bool IsStopped (); // thread safe
	bool IsSeeking (); // thread safe
	bool IsLoadFramePending (); // thread safe
	bool IsBufferUnderflow (); // thread safe
	
	bool HasRenderedFrame (); // thread safe
	void VideoFinished (); // not thread safe.
	// Thread-safe
	void AudioFinished (); // Called by the audio player when audio reaches the end
	// Thread-safe
	void AudioFailed (AudioSource *source); // Called by the audio engine if audio failed to load (async)
	
	void SetBit (PlayerState s); // thread safe
	void RemoveBit (PlayerState s); // thread safe
	void SetBitTo (PlayerState s, bool value); // thread safe
	bool GetBit (PlayerState s); // thread safe
	void SetState (PlayerState s); // thread safe
	PlayerState GetState (); // thread safe

	void SetBufferUnderflow (); // thread safe
	void SetAudioStreamIndex (gint32 i);
	
	void Play ();
	bool GetCanPause ();
	void SetCanPause (bool value);
	void Pause ();
	void Stop ();
	
	void SetCanSeek (bool value);
	bool GetCanSeek ();
	void NotifySeek (guint64 pts /* 100-nanosecond units (pts) */);
	
	virtual void SetSurface (Surface *surface);
	
	cairo_surface_t *GetCairoSurface () { return surface; }
	gint32 GetTimeoutInterval ();
	
	int GetAudioStreamCount () { return audio_stream_count; }
	Media *GetMedia () { return media; }
	
	bool HasVideo () { return video_stream != NULL; }
	// We may go from having audio to not having audio at any time
	// (async - this function may return true, but by the time it 
	// returns we don't have audio anymore).
	bool HasAudio () { return audio_unlocked != NULL; }
	
	guint64 GetPosition () { return GetTargetPts (); }
	guint64 GetDuration () { return duration; }
	
	double GetDroppedFramesPerSecond () { return dropped_frames_per_second; }
	double GetRenderedFramesPerSecond () { return rendered_frames_per_second; }
	
	void SetMuted (bool muted);
	bool GetMuted ();
	
	gint32 GetVideoHeight () { return height; }
	gint32 GetVideoWidth () { return width; }
	
	double GetBalance ();
	void SetBalance (double balance);
	
	double GetVolume ();
	void SetVolume (double volume);
	
	guint64 GetTargetPts ();
	
	const static int MediaEndedEvent; // This is raised when both audio and video has finished (or either one if not both are present).
	const static int BufferUnderflowEvent;
};

#endif /* __MOON_MPLAYER_H__ */
