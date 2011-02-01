/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline.h: Pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_PIPELINE_H_
#define __MOON_PIPELINE_H_

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
#include "utils.h"
#include "mutex.h"

#define MAKE_CODEC_ID(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

#define CODEC_WMV1	MAKE_CODEC_ID ('W', 'M', 'V', '1')
#define CODEC_WMV2	MAKE_CODEC_ID ('W', 'M', 'V', '2')
#define CODEC_WMV3	MAKE_CODEC_ID ('W', 'M', 'V', '3')
#define CODEC_WMVA	MAKE_CODEC_ID ('W', 'M', 'V', 'A')
#define CODEC_WVC1	MAKE_CODEC_ID ('W', 'V', 'C', '1')
#define CODEC_RGBA	MAKE_CODEC_ID ('R', 'G', 'B', 'A')
#define CODEC_YV12	MAKE_CODEC_ID ('Y', 'V', '1', '2')
#define CODEC_MP3	0x55
#define CODEC_WMAV1 0x160
#define CODEC_WMAV2 0x161
#define CODEC_WMAV3 0x162
#define CODEC_PCM   0x1

#define MAX_VIDEO_HEIGHT	2048
#define MAX_VIDEO_WIDTH		2048

typedef void (*register_codec) (int abi_version);

/*
 *	Should be capable of:
 *	- play files and live streams
 *	- open a file and get information about its streams
 *	    - audio: format, initial_pts, # of channels, sample rate, sample size
 *	    - video: format, initial_pts, height, width, frame rate
 *	- needs to be able to pause playback when there's no more source (reached end of downloaded data / progressive downloading)
 *	- informing back about streamed markers (these are quite rare, so it doesn't make sense to poll on every AdvanceFrame if there are any new markers)
 */

/*
 *	
 *	Playing media more or less goes like this with async frame reading (from MediaPlayer's perspective)
 *		Open ():
 *			Stop ()
 *			set state to paused
 *			Open the file, read data/headers, initialize whatever has to be initialized
 *			if any video streams, request first frame to be decoded (sync) and show it
 *		Play ():
 *			set state to playing
 *			set flag that we need more frames
 *			enqueue a few frame requests
 *		Pause ():
 *			set state to paused
 *			clear the queue of requested frames (no need to wait until frames being decoded are finished)
 *		Stop ():
 *			set state to stopped
 *			EmptyQueues ()
 *		AdvanceFrame ():
 *			if not playing, return
 *			if no decoded frames, return
 *			aquire queue-lock
 *			pop a decoded video+audio frame
 *			release queue-lock
 *			draw/play a decoded video/audio frame(s)
 *			enqueue more frame requests (one for each drawn/played)
 *		Seek ():
 *			EmptyQueues ()
 *			seek to the desired position
 *			enqueue a few frame requests
 *		EmptyQueues ():
 *			set flag that we need no more frames (saving old state)
 *			clear the queue of requested frames and wait until no more frames are being decoded
 *			// No need to lock here, since we know that nobody will call FrameDecodedCallback now (there are no requested frames)
 *			empty the queue of decoded frames
 *			set flag to saved state
 *		
 *		FrameDecodedCallback () -> called on another thread
 *			if flag that we need no more frames is set, do nothing
 *			aquire queue-lock
 *			add the decoded frame to the queue of decoded frames
 *			release queue-lock
 *			
 *			
 *
 */

class Media;
class IMediaSource;
class IMediaStream;
class IMediaDemuxer;
class IMediaDecoder;
class IMediaObject;
class FileSource;
class LiveSource;
class MediaClosure;
class MediaFrame;
class VideoStream;
class AudioStream;
class MarkerStream;
class IImageConverter;
class MediaMarker;
class ProgressiveSource;
class MediaMarkerFoundClosure;
class Playlist;

typedef gint32 MediaResult;

#define MEDIA_SUCCESS ((MediaResult) 0)
#define MEDIA_FAIL ((MediaResult) 1)
#define MEDIA_INVALID_STREAM ((MediaResult) 2)
#define MEDIA_UNKNOWN_CODEC ((MediaResult) 3)
#define MEDIA_INVALID_MEDIA ((MediaResult) 4)
#define MEDIA_FILE_ERROR ((MediaResult) 5)
#define MEDIA_CODEC_ERROR ((MediaResult) 6)
#define MEDIA_OUT_OF_MEMORY ((MediaResult) 7)
#define MEDIA_DEMUXER_ERROR ((MediaResult) 8)
#define MEDIA_CONVERTER_ERROR ((MediaResult) 9)
#define MEDIA_UNKNOWN_CONVERTER ((MediaResult) 10)
#define MEDIA_UNKNOWN_MEDIA_TYPE ((MediaResult) 11)
#define MEDIA_CODEC_DELAYED ((MediaResult) 12)
#define MEDIA_NO_MORE_DATA ((MediaResult) 13)
#define MEDIA_CORRUPTED_MEDIA ((MediaResult) 14)
#define MEDIA_NO_CALLBACK ((MediaResult) 15)
#define MEDIA_INVALID_DATA ((MediaResult) 16)
#define MEDIA_READ_ERROR ((MediaResult) 17)
// The pipeline returns this value in GetNextFrame if the
// buffer is empty.
#define MEDIA_BUFFER_UNDERFLOW ((MediaResult) 18)
// This value might be returned by the pipeline for an open
// request, indicating that there is not enough data available
// to open the media.
// It is also used internally in the pipeline whenever something
// can't be done because of not enough data.
// Note: this value must not be used for eof condition.
#define MEDIA_NOT_ENOUGH_DATA ((MediaResult) 19)
#define MEDIA_INVALID ((MediaResult) 0xFFFFFFFF)

#define MEDIA_SUCCEEDED(x) (((x) <= 0))

typedef MediaResult MediaCallback (MediaClosure *closure);

#include "list.h"
#include "debug.h"
#include "dependencyobject.h"
#include "error.h"
#include "type.h"
#include "enums.h"
#include "application.h"

/*
 * MediaClosure: 
 */ 
class MediaClosure : public EventObject {
private:
	MediaCallback *callback;
	MediaResult result;

	Media *media;
	EventObject *context; // The property of whoever creates the closure.
	const char *description;
	void Init (Media *media, MediaCallback *callback, EventObject *context);
	
protected:
	virtual ~MediaClosure () {}
	MediaClosure (Type::Kind object_type, Media *media, MediaCallback *callback, EventObject *context);
	
public:
	MediaClosure (Media *media, MediaCallback *callback, EventObject *context, const char *description);
	virtual void Dispose ();
	
	void Call (); // Calls the callback and stores the result in 'result', then calls the 'finished' callback
	
	bool CallExecuted () { return result != MEDIA_INVALID; }

	MediaResult GetResult () { return result; }
	Media *GetMedia () { return media; }
	EventObject *GetContext () { return context; }
	const char *GetDescription () { return description != NULL ? description : GetTypeName (); }
};

/*
 * MediaDisposeObjectClosure
 */
class MediaDisposeObjectClosure : public MediaClosure {
public:
	MediaDisposeObjectClosure (Media *media, MediaCallback *callback, EventObject *context);
	virtual void Dispose ();
};

/*
 * MediaReportSeekCompletedClosure
 */
class MediaReportSeekCompletedClosure : public MediaClosure {
private:
	guint64 pts;

protected:
	virtual ~MediaReportSeekCompletedClosure ();
		
public:
	MediaReportSeekCompletedClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, guint64 pts);
	virtual void Dispose ();
	
	guint64 GetPts () { return pts; }
	IMediaDemuxer *GetDemuxer () { return (IMediaDemuxer *) GetContext (); }
};

/*
 * MediaGetFrameClosure
 */
class MediaGetFrameClosure : public MediaClosure {
private:
	IMediaStream *stream;

protected:
	virtual ~MediaGetFrameClosure ();
		
public:
	MediaGetFrameClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, IMediaStream *stream);
	virtual void Dispose ();
	
	IMediaStream *GetStream () { return stream; }
	IMediaDemuxer *GetDemuxer () { return (IMediaDemuxer *) GetContext (); }
};

/*
 * MediaReportFrameCompletedClosure
 */
class MediaReportFrameCompletedClosure : public MediaClosure {
private:
	MediaFrame *frame;

protected:
	virtual ~MediaReportFrameCompletedClosure () {}

public:
	MediaReportFrameCompletedClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, MediaFrame *frame);
	virtual void Dispose ();
	
	MediaFrame *GetFrame () { return frame; }
	IMediaDemuxer *GetDemuxer () { return (IMediaDemuxer *) GetContext (); }
};

/*
 * MediaMarkerFoundClosure
 */
class MediaMarkerFoundClosure : public MediaClosure {
private:
	MediaMarker *marker;

protected:
	virtual ~MediaMarkerFoundClosure () {}
	
public:
	MediaMarkerFoundClosure (Media *media, MediaCallback *callback, MediaElement *context);
	virtual void Dispose ();
	
	MediaMarker *GetMarker () { return marker; }
	void SetMarker (MediaMarker *marker);
};

/*
 * MediaSeekClosure
 */
class MediaSeekClosure : public MediaClosure {
private:
	guint64 pts;

protected:
	virtual ~MediaSeekClosure () {}
	
public:
	MediaSeekClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, guint64 pts);
	
	IMediaDemuxer *GetDemuxer () { return (IMediaDemuxer *) GetContext (); }
	guint64 GetPts () { return pts; }
};

/*
 * *Info classes used to register codecs, demuxers and converters.
 */

class MediaInfo {
public:
	MediaInfo *next; // Used internally by Media.
	MediaInfo () : next (NULL) {}
	virtual ~MediaInfo () {}
	virtual const char *GetName () { return "Unknown"; }
};

class DecoderInfo : public MediaInfo {
public:
	virtual bool Supports (const char *codec) = 0;
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream) = 0;
	
	virtual ~DecoderInfo ()  {}
};

class DemuxerInfo : public MediaInfo  {
public:
	// <buffer> points to the first <length> bytes of a file. 
	// <length> is guaranteed to be at least 16 bytes.
	// Possible return values:
	// - MEDIA_SUCCESS: the demuxer supports this source
	// - MEDIA_FAIL: the demuxer does not support this source
	// - MEDIA_NOT_ENOUGH_DATA: the source doesn't have enough data available for the demuxer to know if it's a supported format or not.
	virtual MediaResult Supports (IMediaSource *source) = 0; 
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source) = 0;
};

class ConverterInfo : public MediaInfo  {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output) = 0;
	virtual IImageConverter *Create (Media *media, VideoStream *stream) = 0;
};

/*
 * ProgressEventArgs
 */
/* @Namespace=None */
class ProgressEventArgs : public EventArgs {
public:
	double progress;
	
	ProgressEventArgs (double progress)
	{
		this->progress = progress;
	}
};

/*
 * MediaWork
 */ 

class MediaWork : public List::Node {
public:
	MediaClosure *closure;
	MediaWork (MediaClosure *closure);
	virtual ~MediaWork ();
};

/*
 * IMediaObject
 */
class IMediaObject : public EventObject {
private:
	Media *media;
	Mutex media_mutex;
	// Media event handling
	// media needs to support event handling on all threads, and EventObject isn't thread-safe
	class EventData : public List::Node {
	public:
		int event_id;
		EventHandler handler;
		EventObject *context;
		bool invoke_on_main_thread;
		EventData (int event_id, EventHandler handler, EventObject *context, bool invoke_on_main_thread);
		virtual ~EventData ();
	};
	class EmitData : public List::Node {
	public:
		int event_id;
		EventHandler handler;
		EventObject *context;
		EventArgs *args;
		EmitData (int event_id, EventHandler handler, EventObject *context, EventArgs *args);
		virtual ~EmitData ();
	};
	List *events; // list of event handlers
	List *emit_on_main_thread; // list of emit calls to emit on main thread
	Mutex event_mutex;
	
	void EmitList (List *list);
	void EmitListMain ();
	static void EmitListCallback (EventObject *obj);
	
protected:
	virtual ~IMediaObject () {}

public:
	IMediaObject (Type::Kind kind, Media *media);
	virtual void Dispose ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	Media *GetMediaReffed ();
	void SetMedia (Media *value);

	void ReportErrorOccurred (ErrorEventArgs *args);
	/* @GenerateCBinding */
	void ReportErrorOccurred (const char *message);
	void ReportErrorOccurred (MediaResult result);
	
	// All the event methods are thread-safe
	void AddSafeHandler (int event_id, EventHandler handler, EventObject *context, bool invoke_on_main_thread = true);
	void RemoveSafeHandlers (EventObject *context);
	void EmitSafe (int event_id, EventArgs *args = NULL);
};

/* @Namespace=None,ManagedEvents=Manual */
class IMediaStream : public IMediaObject {
private:
	void *context;
	bool selected;
	bool input_ended; // end of stream reached in demuxer
	bool output_ended; // end of stream reached in decoder
	guint64 first_pts; // The first pts in the stream, initialized to G_MAXUINT64
	guint64 last_popped_pts; // The pts of the last frame returned, initialized to G_MAXUINT64
	guint64 last_enqueued_pts; // The pts of the last frae enqueued, initialized to G_MAXUINT64
	guint64 last_available_pts; // The last pts available, initialized to 0. Note that this field won't be correct for streams which CanSeekToPts.
	Queue queue; // Our queue of demuxed frames
	IMediaDecoder *decoder;

protected:
	virtual ~IMediaStream () {}
	virtual void FrameEnqueued () {}

	static char *CreateCodec (int codec_id); // converts fourcc int value into a string
public:
	class StreamNode : public List::Node {
	 private:
	 	MediaFrame *frame;
	 public:
		StreamNode (MediaFrame *frame);
		virtual ~StreamNode ();
		MediaFrame *GetFrame () { return frame; }
	};
	
	IMediaStream (Type::Kind kind, Media *media);
	virtual void Dispose ();
	
	//	Video, Audio, Markers, etc.
	virtual MediaStreamType GetType () = 0; // TODO: This should be removed, it clashes with GetType in EventObject.
	/* @GenerateCBinding */
	virtual MediaStreamType GetStreamType () { return GetType (); }
	const char *GetStreamTypeName ();
	
	IMediaDecoder *GetDecoder ();
	void SetDecoder (IMediaDecoder *value);
	
	/* @GenerateCBinding */
	const char *GetCodec () { return codec; }
	
	//	User defined context value.
	void *GetContext () { return context; }
	void  SetContext (void *context) { this->context = context; }
	
	bool GetSelected () { return selected; }
	void SetSelected (bool value);

	void *extra_data;
	int extra_data_size;
	int codec_id;
	guint64 duration; // 100-nanosecond units (pts)
	char *codec; // freed upon destruction
	// The minimum amount of padding any other part of the pipeline needs for frames from this stream.
	// Used by the demuxer when reading frames, ensures that there are at least min_padding extra bytes
	// at the end of the frame data (all initialized to 0).
	int min_padding;
	// 0-based index of the stream in the media
	// set by the demuxer, until then its value must be -1
	int index; 
	
	void EnqueueFrame (MediaFrame *frame);
	MediaFrame *PopFrame ();
	bool IsQueueEmpty ();
	bool IsInQueue (MediaFrame *frame);
	void ClearQueue ();
	gint32 GetQueueLength () { return queue.Length (); }
	guint64 GetFirstPts () { return first_pts; }
	guint64 GetLastPoppedPts () { return last_popped_pts; }
	guint64 GetLastEnqueuedPts () { return last_enqueued_pts; }
	void SetLastAvailablePts (guint64 value) { last_available_pts = MAX (value, last_available_pts); }
	guint64 GetLastAvailablePts () { return last_available_pts; }
	guint64 GetBufferedSize (); // Returns the time between the last frame returned and the last frame available (buffer time)
	
	/* @GenerateCBinding */
	int GetExtraDataSize () { return extra_data_size; }
	/* @GenerateCBinding */
	void SetExtraDataSize (int value) { extra_data_size = value; }
	/* @GenerateCBinding */
	void *GetExtraData () { return extra_data; }
	/* @GenerateCBinding */
	void SetExtraData (void *value) { extra_data = value; }
	/* @GenerateCBinding */
	int GetCodecId () { return codec_id; }
	/* @GenerateCBinding */
	void SetCodecId (int value) { codec_id = value; }
	/* @GenerateCBinding */
	guint64 GetDuration () { return duration; }
	/* @GenerateCBinding */
	void SetDuration (guint64 value) { duration = value; }

	bool GetInputEnded ();
	void SetInputEnded (bool value);
	bool GetOutputEnded ();
	void SetOutputEnded (bool value);
	
	IMediaDemuxer *GetDemuxerReffed ();
	
	void ReportSeekCompleted ();
#if DEBUG
	void PrintBufferInformation ();
#endif
	const static int FirstFrameEnqueuedEvent;
};

/*
 * Media
 */
/* @Namespace=None,ManagedEvents=Manual */
class Media : public IMediaObject {
private:	
	static ConverterInfo *registered_converters;
	static DemuxerInfo *registered_demuxers;
	static DecoderInfo *registered_decoders;
	static bool registering_ms_codecs;
	static bool registered_ms_codecs;

	Mutex mutex;
	
	guint64 target_pts; // Access must be protected with mutes.
	guint64 buffering_time; // Access must be protected with mutex.
	bool is_disposed; // Access must be protected with mutex. This is used to ensure that we don't add work to the thread pool after having been disposed.
	char *uri;
	char *file;
	IMediaSource *source;
	IMediaDemuxer *demuxer;
	List *markers;
	bool initialized;
	bool opened;
	bool opening;
	bool stopped;
	bool error_reported; // If an error has been reported.
	bool buffering_enabled;
	bool in_open_internal; // detect recursive calls to OpenInternal
	bool http_retried;
	double download_progress;
	double buffering_progress;
	
	PlaylistRoot *playlist;

	// Determines the container type and selects a demuxer. We have support for mp3 and asf demuxers.
	// Also opens the demuxer.
	// This method is supposed to be called multiple times, until either 'error_reported' is true or this method
	// returns true. It will pick up wherever it left working last time.
	bool SelectDemuxerAsync ();
	
	//	Selects decoders according to stream info.
	//	- Default is to use any MS decoder if available (and applicable), otherwise ffmpeg. 
	//    Overridable by MOONLIGHT_OVERRIDES, set ms-codecs=no to disable ms codecs, and ffmpeg-codecs=no to disable ffempg codecs
	// This method is supposed to be called multiple times, until either 'error_reported' is true or this method
	// returns true. It will pick up wherever it left working last time.
	bool SelectDecodersAsync ();
	
	// This method is supposed to be called multiple times, until the media has been successfully opened or an error occurred.
	void OpenInternal ();
	static MediaResult OpenInternal (MediaClosure *closure);
	static MediaResult DisposeObjectInternal (MediaClosure *closure);

	static MediaResult StopCallback (MediaClosure *closure);
	static MediaResult PauseCallback (MediaClosure *closure);
	static MediaResult PlayCallback (MediaClosure *closure);
	static MediaResult ClearBufferingProgressCallback (MediaClosure *closure);
	void Stop ();
	void Pause ();
	void Play ();
	
protected:
	virtual ~Media ();

public:
	Media (PlaylistRoot *root);
	
	virtual void Dispose ();
	
	static bool InMediaThread ();
	bool EnqueueWork (MediaClosure *closure, bool wakeup = true);
	
	// Calls obj->Dispose on the media thread.
	void DisposeObject (EventObject *obj);
	
	// Initialize the Media.
	// These methods may raise MediaError events.
	void Initialize (Downloader *downloader, const char *PartName); // MediaElement.SetSource (dl, 'PartName');
	void Initialize (const char *uri); // MediaElement.Source = 'uri';
	void Initialize (IMediaDemuxer *demuxer); // MediaElement.SetSource (demuxer); 
	void Initialize (IMediaSource *source);

	// Start opening the media.
	// When done, OpenCompleted event is raised.
	// In case of failure, MediaError event is raised	
	void OpenAsync ();
	
	void ReportOpenDemuxerCompleted (); // This method is called by the demuxer when it has opened.
	void ReportOpenDecoderCompleted (IMediaDecoder *decoder); // This method is called by any of the decoders when it has opened.
	void ReportOpenCompleted (); // Raise the OpenCompleted event.
	
	void ReportDownloadProgress (double progress);
	void ReportBufferingProgress (double progress);
	
	// Media playback
	void PlayAsync (); // Raises CurrentStateChanged
	void PauseAsync (); // Raises CurrentStateChanged
	void StopAsync (); // Raises CurrentStateChanged
	// Seek to the specified pts
	// When done, SeekCompleted is raised
	// In case of failure, MediaError is raised.
	void SeekAsync (guint64 pts);
	void ReportSeekCompleted (guint64 pts, bool pending_seeks); // This method is called by IMediaDemuxer when a seek is completed. Raises the SeekCompleted event.
		
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	void WakeUp ();
	
	void SetBufferingTime (guint64 buffering_time);
	guint64 GetBufferingTime ();

	void SetTargetPts (guint64 value);
	guint64 GetTargetPts ();

	void SetBufferingEnabled (bool value);

	IMediaSource *GetSource () { return source; }
	IMediaDemuxer *GetDemuxerReffed (); /* thread-safe */
	const char *GetFile () { return file; }
	const char *GetUri () { return uri; }
	void SetFileOrUrl (const char *value);
	
	static void Warning (MediaResult result, const char *format, ...);
	// A list of MediaMarker::Node.
	// This is the list of markers found in the metadata/headers (not as a separate stream).
	// Will never return NULL.
	List *GetMarkers ();
	double GetDownloadProgress () { return download_progress; }
	double GetBufferingProgress () { return buffering_progress; }
	void ClearBufferingProgress (); /* Thread-safe */
	
	PlaylistRoot *GetPlaylistRoot ();
	
	bool IsOpened () { return opened; }
	bool IsOpening () { return opening; }
	bool IsStopped () { return stopped; }
	
	void RetryHttp (ErrorEventArgs *args);
	
	void ReportErrorOccurred (ErrorEventArgs *args);
	void ReportErrorOccurred (const char *message);
	void ReportErrorOccurred (MediaResult result);
	
	bool HasReportedError () { return error_reported; }
	
	const static int OpeningEvent;
	const static int OpenCompletedEvent;
	const static int SeekingEvent;
	const static int SeekCompletedEvent;
	const static int CurrentStateChangedEvent;
	const static int MediaErrorEvent;
	const static int DownloadProgressChangedEvent;
	const static int BufferingProgressChangedEvent;
	
	// Registration functions
	// This class takes ownership of the infos and will delete them (not free) when the Media is shutdown.
	static void RegisterDemuxer (DemuxerInfo *info);
	/* @GenerateCBinding */
	static void RegisterDecoder (DecoderInfo *info);
	static void RegisterConverter (ConverterInfo *info);
	
	static void RegisterMSCodecs ();
	static bool IsMSCodecsInstalled () MOON_API;
	
	static void Initialize ();
	static void Shutdown ();
};

/*
 * MediaThreadPool
 *
 * The most important requirement for the thread pool is that it never executes several work items for a single Media instance simultaneously.
 * It accomplishes this by having a list of Media instances which is currently being worked on, and whenever a thread is free, it finds work for
 * a Media instance which is not in the list.
 */ 
class MediaThreadPool {
private:
	static pthread_mutex_t mutex;
	static pthread_cond_t condition; /* signalled when work has been added */
	static pthread_cond_t completed_condition; /* signalled when work has completed executing */
	static const int max_threads = 4; /* max 4 threads for now */
	static int count; // the number of created threads 
	static pthread_t threads [max_threads]; // array of threads
	static bool valid [max_threads]; // specifies which thread indices are valid.
	static Media *medias [max_threads]; // array of medias currently being worked on (indices corresponds to the threads array). Only one media can be worked on at the same time.
	static Deployment *deployments [max_threads]; // array of deployments currently being worked on.
	static bool shutting_down; // flag telling if we're shutting down (in which case no new threads should be created) - it's also used to check if we've been shut down already (i.e. it's not set to false when the shutdown has finished).
	static List *queue;
	
	static void *WorkerLoop (void *data);
	
public:
	// Removes all enqueued work for the specified media.
	static void RemoveWork (Media *media);
	// Waits until all enqueued work for the specified deployment has finished
	// executing and there is no more work for the specified deployment. Note that
	// it does not touch the queue, it just waits for the threads to finish cleaning
	// up the queue.
	static void WaitForCompletion (Deployment *deployment); /* Main thread only */
	static void AddWork (MediaClosure *closure, bool wakeup);
	static void WakeUp ();
	static void Initialize ();
	static void Shutdown ();

	// this method checks if the current thread is a thread-pool thread
	static bool IsThreadPoolThread ();
};
 
class MediaFrame : public EventObject {
private:
	void Initialize ();
	
protected:
	virtual ~MediaFrame ();
	
public:
	MediaFrame (IMediaStream *stream);
	/* @GenerateCBinding,GeneratePInvoke */
	MediaFrame (IMediaStream *stream, guint8 *buffer, guint32 buflen, guint64 pts, bool keyframe);
	void Dispose ();
	
	/* @GenerateCBinding */
	void AddState (MediaFrameState state) { this->state |= (guint16) state; } // There's no way of "going back" to an earlier state 
	bool IsDecoded () { return (((MediaFrameState) state) & MediaFrameDecoded) == MediaFrameDecoded; }
	bool IsDemuxed () { return (((MediaFrameState) state) & MediaFrameDemuxed) == MediaFrameDemuxed; }
	bool IsConverted () { return (((MediaFrameState) state) & MediaFrameConverted) == MediaFrameConverted; }
	bool IsPlanar () { return (((MediaFrameState) state) & MediaFramePlanar) == MediaFramePlanar; }
	/* @GenerateCBinding */
	bool IsKeyFrame () { return (((MediaFrameState) state) & MediaFrameKeyFrame) == MediaFrameKeyFrame; }
	bool IsMarker () { return (((MediaFrameState) state) & MediaFrameMarker) == MediaFrameMarker; }
	
	IMediaStream *stream;
	MediaMarker *marker;
	void *decoder_specific_data; // data specific to the decoder
	guint64 pts; // Set by the demuxer
	guint64 duration; // Set by the demuxer
	
	guint16 state; // Current state of the frame
	guint16 event; // special frame event if non-0
	
	// The demuxer sets these to the encoded data which the
	// decoder then uses and replaces with the decoded data.
	guint8 *buffer;
	guint32 buflen;
	
	// planar data
	guint8 *data_stride[4]; // Set by the decoder
	int srcSlideY; // Set by the decoder
	int srcSlideH; // Set by the decoder
	int srcStride[4]; // Set by the decoder
	
	// The decoded size of the frame (might be bigger or smaller than the size of the stream).
	// 0 = the size specified in the stream
	gint32 width;
	gint32 height;
	
	/* @GenerateCBinding */
	guint32 GetBufLen () { return buflen; }
	/* @GenerateCBinding */
	void SetBufLen (guint32 value) { buflen = value; }
	/* @GenerateCBinding */
	guint8* GetBuffer () { return buffer; }
	/* @GenerateCBinding */
	void SetBuffer (guint8 *value) { buffer = value; }
	/* @GenerateCBinding */
	guint64 GetPts () { return pts; }
	/* @GenerateCBinding */
	void SetPts (guint64 value) { pts = value; }
	
	/* @GenerateCBinding */
	gint32 GetWidth () { return width; }
	/* @GenerateCBinding */
	void SetWidth (gint32 value) { width = value; }
	/* @GenerateCBinding */
	gint32 GetHeight () { return height; }
	/* @GenerateCBinding */
	void SetHeight (gint32 value) { height = value; }
	/* @GenerateCBinding */
	void SetDataStride (guint8 *a, guint8 *b, guint8 *c, guint8 *d);
	/* @GenerateCBinding */
	void SetSrcStride (int a, int b, int c, int d);
	/* @GenerateCBinding */
	void SetSrcSlideY (int value);
	/* @GenerateCBinding */
	void SetSrcSlideH (int value);
	/* @GenerateCBinding */
	void SetDecoderSpecificData (void *value) { decoder_specific_data = value; }
	
	
};

class MediaMarker : public EventObject {
	guint64 pts; // 100-nanosecond units (pts)
	char *type;
	char *text;

protected:
	virtual ~MediaMarker ();
	
public:
	class Node : public List::Node {
	public:
		Node (MediaMarker *m)
		{
			marker = m;
			marker->ref ();
		}
		
		virtual ~Node ()
		{
			marker->unref ();
		}
		
		MediaMarker *marker;
	};
	
	MediaMarker (const char *type, const char *text, guint64 pts);
	
	const char *Type () { return type; }
	const char *Text () { return text; }
	guint64 Pts () { return pts; }
};

// Interfaces

class IMediaDemuxer : public IMediaObject {
private:
	class PtsNode : public List::Node {
	public:
		guint64 pts;
		PtsNode (guint64 pts)
		{
			this->pts = pts;
		}
	};
	IMediaStream **streams;
	int stream_count;
	bool opened;
	bool opening;
	bool seeking; /* Only media thread may access, no lock required. When set, the demuxer should not request new frames */
	bool drm; /* If the content this demuxer is demuxing is drm-protected */
	/* 
	 * Set on main thread, read/reset on media thread: access needs mutex locked. 
	 * When a seek is pending, indicates the position we should seek to. We specifically
	 * do not enqueue the pts with the seek request - this would cause
	 * multiple seeks with unpredictable ordeing when SeekAsync is called again before the
	 * first seek has finished
	 */
	List seeks; /* The FIFO list of seeked-to pts. All threads may use, locking required. */
	IMediaStream *pending_stream; // the stream we're waiting for a frame for. media thread only.
	bool pending_fill_buffers;
	Mutex mutex;
	/*
	 * We only want frames after the last keyframe before the pts we seeked to.
	 * Store the seeked-to pts here, so that the IMediaStream can drop frames it
	 * doesn't need.
	 */
	guint64 seeked_to_pts;
	
	static MediaResult ReportSeekCompletedCallback (MediaClosure *closure);
	static MediaResult ReportGetFrameCompletedCallback (MediaClosure *closure);
	static MediaResult GetFrameCallback (MediaClosure *closure);
	static MediaResult FillBuffersCallback (MediaClosure *closure);
	static MediaResult OpenCallback (MediaClosure *closure);
	static MediaResult SeekCallback (MediaClosure *closure);
	
	void FillBuffersInternal ();
	
protected:
	IMediaSource *source;
	
	IMediaDemuxer (Type::Kind kind, Media *media, IMediaSource *source);
	IMediaDemuxer (Type::Kind kind, Media *media);
	
	virtual ~IMediaDemuxer () {}
	
	void SetStreams (IMediaStream **streams, int count);
	gint32 AddStream (IMediaStream *stream);
	
	virtual void CloseDemuxerInternal () {};
	virtual void GetDiagnosticAsyncInternal (MediaStreamSourceDiagnosticKind diagnosticKind) {}
	virtual void GetFrameAsyncInternal (IMediaStream *stream) = 0;
	virtual void OpenDemuxerAsyncInternal () = 0;
	virtual void SeekAsyncInternal (guint64 pts) = 0;
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream) = 0;
	
	void EnqueueOpen ();
	void EnqueueReportSeekCompleted (guint64 pts);
	void EnqueueGetFrame (IMediaStream *stream);
	void EnqueueReportGetFrameCompleted (MediaFrame *frame);
	/* Re-enqueue the seek. */
	void EnqueueSeek ();
	void SeekAsync ();
	
public:
	virtual void Dispose ();

	void CloseDemuxer () {};
	void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind) {}
	void GetFrameAsync (IMediaStream *stream);
	void OpenDemuxerAsync ();
	/* Might not seek immediately, It will wait until there are no pending frames. */
	void SeekAsync (guint64 pts);
	void SwitchMediaStreamAsync (IMediaStream *stream);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportOpenDemuxerCompleted ();
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportGetFrameCompleted (MediaFrame *frame);
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportGetFrameProgress (double bufferingProgress);
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportSeekCompleted (guint64 pts);
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportSwitchMediaStreamCompleted (IMediaStream *stream);
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportGetDiagnosticCompleted (MediaStreamSourceDiagnosticKind diagnosticKind, gint64 diagnosticValue);
	
	guint64 GetBufferedSize ();
	void FillBuffers ();
	void ClearBuffers ();
	
	void PrintBufferInformation ();
	guint64 GetSeekedToPts () { return seeked_to_pts; }
	
	int GetStreamCount () { return stream_count; }
	IMediaStream *GetStream (int index);
	// Gets the longest duration from all the streams
	virtual guint64 GetDuration (); // 100-nanosecond units (pts)
	virtual const char *GetName () = 0;
	virtual void UpdateSelected (IMediaStream *stream) {};
	
	guint64 GetLastAvailablePts ();
	IMediaSource *GetSource () { return source; }
	bool IsOpened () { return opened; }
	bool IsOpening () { return opening; }
	
	virtual bool GetCanSeek () { return true; }
	virtual bool IsPlaylist () { return false; }
	virtual Playlist *GetPlaylist () { return NULL; }

	/* @GenerateCBinding,GeneratePInvoke */
	void SetIsDrm (bool value) { drm = value; }
	bool IsDrm () { return drm; }
};

class IMediaDecoder : public IMediaObject {
private:
	bool opening;
	bool opened;
	bool input_ended;
	MoonPixelFormat pixel_format; // The pixel format this codec outputs. Open () should fill this in.
	IMediaStream *stream;
	Queue queue; // the list of frames to decode.
		
	static MediaResult DecodeFrameCallback (MediaClosure *closure);
	
	class FrameNode : public List::Node {
	public:
		MediaFrame *frame;
		
		FrameNode (MediaFrame *f) : frame (f)
		{
			frame->ref ();
		}
		
		virtual ~FrameNode ()
		{
			frame->unref ();
		}
	};
	
protected:
	virtual ~IMediaDecoder () {}

	/*
	 * Called when a frame needs to get decoded. The implementation needs to call
	 * ReportDecodeFrameCompleted at least once for each DecodeFrameAsync request
	 * (calling ReportDecodeFrameCompleted more than once for each DecodeFrameAsync
	 * request is allowed). The implementation may call ReportDecodeFrameCompleted
	 * with a null buffer (for instance if it does not have enough input to produce output).
	 */
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame) = 0;
	virtual void OpenDecoderAsyncInternal () = 0;
	
	// InputEnded is called when there is no more input. If the codec has delayed frames,
	// it must call ReportDecodeFrameCompleted with those frames (all of them).
	virtual void InputEnded () { };
	
public:
	IMediaDecoder (Type::Kind kind, Media *media, IMediaStream *stream);
	virtual void Dispose ();
	
	// If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	virtual void Cleanup (MediaFrame *frame) {}
	virtual void CleanState () {}
	virtual bool HasDelayedFrame () { return false; }
	
	virtual const char *GetName () { return GetTypeName (); }
	
	// This method is called when the demuxer has finished seeking.
	void ReportSeekCompleted ();
	// This method is called when the demuxer is out of data.
	void ReportInputEnded ();
	/* @GenerateCBinding */
	void ReportDecodeFrameCompleted (MediaFrame *frame);
	/* @GenerateCBinding */
	void ReportOpenDecoderCompleted ();
	/* @GenerateCBinding */
	void SetPixelFormat (MoonPixelFormat value) { pixel_format = value; }
	
	void DecodeFrameAsync (MediaFrame *frame, bool enqueue_always);
	void OpenDecoderAsync ();
	
	bool IsOpening () { return opening; }
	bool IsOpened () { return opened; }
	MoonPixelFormat GetPixelFormat () { return pixel_format; }
	IMediaStream *GetStream () { return stream; }	
	
	bool IsDecoderQueueEmpty () { return queue.IsEmpty (); }
};


/*
 * Inherit from this class to provide image converters (yuv->rgb for instance) 
 */
class IImageConverter : public IMediaObject {
protected:
	virtual ~IImageConverter () {}

public:
	MoonPixelFormat output_format;
	MoonPixelFormat input_format;
	VideoStream *stream;
	
	IImageConverter (Type::Kind kind, Media *media, VideoStream *stream);
	
	virtual MediaResult Open () = 0;
	virtual MediaResult Convert (guint8 *src[], int srcStride[], int srcSlideY, int srcSlideH, guint8 *dest[], int dstStride []) = 0;
};

/*
 * IMediaSource
 */
class IMediaSource : public IMediaObject {
private:
	// General locking behaviour:
	// All protected virtual methods must be called with the mutex
	// locked. If a derived virtual method needs to lock, it needs
	// to be implemented as a protected virtual method xxxInternal
	// which requires the mutex to be locked, and then a public 
	// method in IMediaSource which does the locking. No public method
	// in IMediaSource may be called from the xxxInternal methods.
	pthread_mutex_t mutex;
	pthread_cond_t condition;

protected:
	virtual ~IMediaSource ();

	void Lock ();
	void Unlock ();

	// All these methods must/will be called with the lock locked.	
	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetLastAvailablePositionInternal () { return -1; }
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();

public:
	IMediaSource (Type::Kind kind, Media *media);
	virtual void Dispose ();
	
	// Initializes this stream (and if it succeeds, it can be read from later on).
	// streams based on remote content (live/progress) should contact the server
	// and try to start downloading content
	// file streams should try to open the file
	virtual MediaResult Initialize () = 0;
	virtual MediaSourceType GetType () = 0;
	
	// Reads 'n' bytes into 'buf'. If data isn't available it will 
	// read the amount of data available. Returns the number of bytes read.
	// This method will lock the mutex.
	gint32 ReadSome (void *buf, guint32 n);

	// Reads 'n' bytes into 'buf'.
	// Returns false if 'n' bytes couldn't be read.
	// This method will lock the mutex.
	bool ReadAll (void *buf, guint32 n);

	// Reads 'n' bytes into 'buf', starting at position 'start'. If 'start' is -1,
	// then start at the current position. If data isn't available it will
	// read the amount of data available. Returns false if 'n' bytes couldn't be
	// read.
	// This method will lock the mutex.
	bool Peek (void *buf, guint32 n);
	
	virtual bool CanSeek () { return true; }

	// Seeks to the specified 'offset', using the specified 'mode'. 
	// This method will lock the mutex.
	bool Seek (gint64 offset, int mode = SEEK_CUR);
	
	// Seeks to the specified 'pts'.
	virtual bool CanSeekToPts () { return false; }
	virtual MediaResult SeekToPts (guint64 pts) { return MEDIA_FAIL; }

	// Returns the current reading position
	// This method will lock the mutex.
	gint64 GetPosition ();

	// Returns the size of the source. This method may return -1 if the
	// size isn't known.
	// This method will lock the mutex.
	gint64 GetSize ();

	virtual bool Eof () = 0;

	// Returns the last available position
	// If the returned value is -1, then everything is available.
	// This method will lock the mutex.
	gint64 GetLastAvailablePosition ();

	// Checks if the specified position can be read
	// upon return, and if the position is not availble eof determines whether the position is not available because
	// the file isn't that big (eof = true), or the position hasn't been read yet (eof = false).
	// if the position is available, eof = false
	bool IsPositionAvailable (gint64 position, bool *eof);

	// If the derived class knows which demuxer it needs, 
	// it should override this method and return a new demuxer.
	virtual IMediaDemuxer *CreateDemuxer (Media *media) { return NULL; }

	virtual const char *ToString () { return "IMediaSource"; }
};

class ManagedStreamSource : public IMediaSource {
private:
	ManagedStreamCallbacks stream;
	
protected:	
	virtual ~ManagedStreamSource ();

	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();

public:
	ManagedStreamSource (Media *media, ManagedStreamCallbacks *stream);
	
	virtual MediaResult Initialize () { return MEDIA_SUCCESS; }
	virtual MediaSourceType GetType () { return MediaSourceTypeManagedStream; }
	
	virtual bool Eof () { return GetPositionInternal () == GetSizeInternal (); }

	virtual const char *ToString () { return GetTypeName (); }
};
 
class FileSource : public IMediaSource {
public: // private:
	gint64 size;
	FILE *fd;
	bool temp_file;
	char buffer [1024];

	void UpdateSize ();
protected:
	char *filename;
	
	virtual ~FileSource ();
	FileSource (Media *media, bool temp_file);
	
	MediaResult Open (const char *filename);

	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();

public:
	FileSource (Media *media, const char *filename);
	virtual void Dispose ();
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeFile; }
	
	virtual bool Eof ();

	virtual const char *ToString () { return filename; }

	const char *GetFileName () { return filename; }
};

class ProgressiveSource : public FileSource {
private:
	gint64 write_pos;
	gint64 size;
	// To avoid locking while reading and writing (since reading is done on 
	// the media thread and writing on the main thread), we open two file
	// handlers, one for reading (in FileSource) and the other one here
	// for writing.
	FILE *write_fd;
	char *uri;
	Cancellable *cancellable;
	
	virtual gint64 GetLastAvailablePositionInternal () { return size == write_pos ? write_pos : write_pos & ~(1024*4-1); }
	virtual gint64 GetSizeInternal () { return size; }

	static void data_write (void *data, gint32 offset, gint32 n, void *closure);
	static void notify_func (NotifyType type, gint64 args, void *closure);
	static void delete_cancellable (EventObject *data);
	
	void Notify (NotifyType, gint64 args);

	void DownloadComplete ();
	void DownloadFailed ();
	void DataWrite (void *data, gint32 offset, gint32 n);
	void NotifySize (gint64 size);
	void SetTotalSize (gint64 size);
	
	void CloseWriteFile ();
	
protected:
	virtual ~ProgressiveSource ();

public:
	ProgressiveSource (Media *media, const char *uri);
	virtual void Dispose ();
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeProgressive; }
};

/*
 * MemorySource
 */
class MemorySource : public IMediaSource {
private:
	void *memory;
	gint32 size;
	gint64 start;
	gint64 pos;
	bool owner;

protected:
	virtual ~MemorySource ();

	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetSizeInternal () { return size; }
	virtual gint64 GetPositionInternal () { return pos + start; }
	virtual gint64 GetLastAvailablePositionInternal () { return start + size; }

public:
	MemorySource (Media *media, void *memory, gint32 size, gint64 start = 0, bool owner = true);

	void *GetMemory () { return memory; }
	void Release (void) { delete this; }

	void SetOwner (bool value) { owner = value; }
	gint64 GetStart () { return start; }

	virtual MediaResult Initialize () { return MEDIA_SUCCESS; }
	virtual MediaSourceType GetType () { return MediaSourceTypeMemory; }
	
	virtual bool CanSeek () { return true; }
	virtual bool Eof () { return pos >= size; }

	virtual const char *ToString () { return "MemorySource"; }
};

class VideoStream : public IMediaStream {
protected:
	virtual ~VideoStream ();

public:
	void Dispose ();
	
	IImageConverter *converter; // This stream has the ownership of the converter, it will be deleted upon destruction.
	guint32 bits_per_sample;
	guint64 pts_per_frame; // Duration (in pts) of each frame. Set to 0 if unknown.
	guint64 initial_pts;
	guint32 height;
	guint32 width;
	guint32 bit_rate;
	
	VideoStream (Media *media);
	
	/* @GenerateCBinding,GeneratePInvoke */
	VideoStream (Media *media, int codec_id, guint32 width, guint32 height, guint64 duration, gpointer extra_data, guint32 extra_data_size);
	
	virtual MediaStreamType GetType () { return MediaTypeVideo; } 

	guint32 GetBitsPerSample () { return bits_per_sample; }
	guint32 GetPtsPerFrame () { return pts_per_frame; }
	guint32 GetInitialPts () { return initial_pts; }
	/* @GenerateCBinding */
	guint32 GetWidth () { return width; }
	/* @GenerateCBinding */
	guint32 GetHeight () { return height; }
	guint32 GetBitRate () { return bit_rate; }
};
 
class AudioStream : public IMediaStream {
private:
	/* input format */
	int input_bits_per_sample;
	int input_block_align;
	int input_sample_rate;
	int input_channels;
	int input_bit_rate;
	
	/* output format */
	int output_bits_per_sample;
	int output_block_align;
	int output_sample_rate;
	int output_channels;
	int output_bit_rate;
protected:
	virtual ~AudioStream () {}

public:

	AudioStream (Media *media);
	
	/* @GenerateCBinding,GeneratePInvoke */
	AudioStream (Media *media, int codec_id, int bits_per_sample, int block_align, int sample_rate, int channels, int bit_rate, gpointer extra_data, guint32 extra_data_size);
	
	virtual MediaStreamType GetType () { return MediaTypeAudio; }

	// TODO: remove the non Input/Output accessors
	// wait until later since it is a two-way codec abi breakage.

	/* @GenerateCBinding */
	int GetBitsPerSample () { return input_bits_per_sample; }
	/* @GenerateCBinding */
	void SetBitsPerSample (int value) { output_bits_per_sample = input_bits_per_sample = value; }
	/* @GenerateCBinding */
	int GetBlockAlign () { return input_block_align; }
	/* @GenerateCBinding */
	void SetBlockAlign (int value) { output_block_align = input_block_align = value; }
	/* @GenerateCBinding */
	int GetSampleRate () { return input_sample_rate; }
	/* @GenerateCBinding */
	void SetSampleRate (int value) { output_sample_rate = input_sample_rate = value; }
	/* @GenerateCBinding */
	int GetChannels () { return input_channels; }
	/* @GenerateCBinding */
	void SetChannels (int value) { output_channels = input_channels = value; }
	/* @GenerateCBinding */
	int GetBitRate () { return input_bit_rate; }
	/* @GenerateCBinding */
	void SetBitRate (int value) { output_bit_rate = input_bit_rate = value; }
	
	// input accessors
	
	/* @GenerateCBinding */
	int GetInputBitsPerSample () { return input_bits_per_sample; }
	/* @GenerateCBinding */
	void SetInputBitsPerSample (int value) { input_bits_per_sample = value; }
	
	/* @GenerateCBinding */
	int GetInputBlockAlign () { return input_block_align; }
	/* @GenerateCBinding */
	void SetInputBlockAlign (int value) { input_block_align = value; }
	
	/* @GenerateCBinding */
	int GetInputSampleRate () { return input_sample_rate; }
	/* @GenerateCBinding */
	void SetInputSampleRate (int value) { input_sample_rate = value; }
	
	/* @GenerateCBinding */
	int GetInputChannels () { return input_channels; }
	/* @GenerateCBinding */
	void SetInputChannels (int value) { input_channels = value; }
	
	/* @GenerateCBinding */
	int GetInputBitRate () { return input_bit_rate; }
	/* @GenerateCBinding */
	void SetInputBitRate (int value) { input_bit_rate = value; }
	
	// output accessors
	
	/* @GenerateCBinding */
	int GetOutputBitsPerSample () { return output_bits_per_sample; }
	/* @GenerateCBinding */
	void SetOutputBitsPerSample (int value) { output_bits_per_sample = value; }
	
	/* @GenerateCBinding */
	int GetOutputBlockAlign () { return output_block_align; }
	/* @GenerateCBinding */
	void SetOutputBlockAlign (int value) { output_block_align = value; }
	
	/* @GenerateCBinding */
	int GetOutputSampleRate () { return output_sample_rate; }
	/* @GenerateCBinding */
	void SetOutputSampleRate (int value) { output_sample_rate = value; }
	
	/* @GenerateCBinding */
	int GetOutputChannels () { return output_channels; }
	/* @GenerateCBinding */
	void SetOutputChannels (int value) { output_channels = value; }
	
	/* @GenerateCBinding */
	int GetOutputBitRate () { return output_bit_rate; }
	/* @GenerateCBinding */
	void SetOutputBitRate (int value) { output_bit_rate = value; }
};

/*
 * ExternalDemuxer
 */

/* @CBindingRequisite */
typedef void (* CloseDemuxerCallback) (void *instance);
/* @CBindingRequisite */
typedef void (* GetDiagnosticAsyncCallback) (void *instance, int /*MediaStreamSourceDiagnosticKind*/ diagnosticKind);
/* @CBindingRequisite */
typedef void (* GetFrameAsyncCallback) (void *instance, int /*MediaStreamType*/ mediaStreamType);
/* @CBindingRequisite */
typedef void (* OpenDemuxerAsyncCallback) (void *instance, IMediaDemuxer *demuxer);
/* @CBindingRequisite */
typedef void (* SeekAsyncCallback) (void *instance, guint64 seekToTime);
/* @CBindingRequisite */
typedef void (* SwitchMediaStreamAsyncCallback) (void *instance, IMediaStream *mediaStreamDescription);
		
class ExternalDemuxer : public IMediaDemuxer {
private:
	void *instance;
	bool can_seek;
	pthread_rwlock_t rwlock;
	CloseDemuxerCallback close_demuxer_callback;
	GetDiagnosticAsyncCallback get_diagnostic_async_callback;
	GetFrameAsyncCallback get_sample_async_callback;
	OpenDemuxerAsyncCallback open_demuxer_async_callback;
	SeekAsyncCallback seek_async_callback;
	SwitchMediaStreamAsyncCallback switch_media_stream_async_callback;
	
protected:
	virtual ~ExternalDemuxer ();

	virtual void CloseDemuxerInternal ();
	virtual void GetDiagnosticAsyncInternal (MediaStreamSourceDiagnosticKind diagnosticsKind);
	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 seekToTime);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *mediaStreamDescription);
	
public:
	ExternalDemuxer (Media *media, void *instance, CloseDemuxerCallback close_demuxer, 
		GetDiagnosticAsyncCallback get_diagnostic, GetFrameAsyncCallback get_sample, OpenDemuxerAsyncCallback open_demuxer, 
		SeekAsyncCallback seek, SwitchMediaStreamAsyncCallback switch_media_stream);
	
	virtual void Dispose ();
		
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCanSeek (bool value);

	/* @GenerateCBinding,GeneratePInvoke */
	void ClearCallbacks (); /* thread-safe */
		
	/* @GenerateCBinding,GeneratePInvoke */
	gint32 AddStream (IMediaStream *stream);
	
	virtual bool GetCanSeek () { return can_seek; }
	
	virtual const char *GetName () { return "ExternalDemuxer"; }
};

/*
 * ExternalDecoder
 */

/* @CBindingRequisite */
typedef void (* ExternalDecoder_DecodeFrameAsyncCallback) (void *instance, MediaFrame *frame);
/* @CBindingRequisite */
typedef void (* ExternalDecoder_OpenDecoderAsyncCallback) (void *instance);
/* @CBindingRequisite */
typedef void (* ExternalDecoder_CleanupCallback) (void *instance, MediaFrame *frame);
/* @CBindingRequisite */
typedef void (* ExternalDecoder_CleanStateCallback) (void *instance);
/* @CBindingRequisite */
typedef bool (* ExternalDecoder_HasDelayedFrameCallback) (void *instance);
/* @CBindingRequisite */
typedef void (* ExternalDecoder_DisposeCallback) (void *instance);
/* @CBindingRequisite */
typedef void (* ExternalDecoder_DtorCallback) (void *instance);

class ExternalDecoder : public IMediaDecoder {
private:
	void *instance;
	char *name;
	ExternalDecoder_DecodeFrameAsyncCallback decode_frame_async;
	ExternalDecoder_OpenDecoderAsyncCallback open_decoder_async;
	ExternalDecoder_CleanupCallback cleanup;
	ExternalDecoder_CleanStateCallback clean_state;
	ExternalDecoder_HasDelayedFrameCallback has_delayed_frame;
	ExternalDecoder_DisposeCallback dispose;
	ExternalDecoder_DtorCallback dtor;
	
protected:
	virtual ~ExternalDecoder ();
	
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();
	
	virtual void InputEnded ();
public:
	/* @GenerateCBinding */
	ExternalDecoder (Media *media, IMediaStream *stream, void *instance, const char *name,
		ExternalDecoder_DecodeFrameAsyncCallback decode_frame_async,
		ExternalDecoder_OpenDecoderAsyncCallback open_decoder_async,
		ExternalDecoder_CleanupCallback cleanup,
		ExternalDecoder_CleanStateCallback clean_state,
		ExternalDecoder_HasDelayedFrameCallback has_delayed_frame,
		ExternalDecoder_DisposeCallback dispose,
		ExternalDecoder_DtorCallback dtor);
	
	virtual void Dispose ();
	
public:
	// If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	virtual void Cleanup (MediaFrame *frame);
	virtual void CleanState ();
	virtual bool HasDelayedFrame ();
	
	virtual const char *GetName () { return name; }
};

/*
 * ExternalDecoderInfo
 */

/* @CBindingRequisite */
typedef bool (* ExternalDecoderInfo_SupportsCallback) (void *instance, const char *codec);
/* @CBindingRequisite */
typedef IMediaDecoder * (* ExternalDecoderInfo_Create) (void *instance, Media *media, IMediaStream *stream);
/* @CBindingRequisite */
typedef void (* ExternalDecoderInfo_dtor) (void *instance);

class ExternalDecoderInfo : public DecoderInfo {
private:
	void *instance;
	char *name;
	ExternalDecoderInfo_SupportsCallback supports;
	ExternalDecoderInfo_Create create;
	ExternalDecoderInfo_dtor dtor;
	
public:
	/* @GenerateCBinding */
	ExternalDecoderInfo (void *instance, const char *name, ExternalDecoderInfo_SupportsCallback supports, ExternalDecoderInfo_Create create, ExternalDecoderInfo_dtor dtor);

	virtual bool Supports (const char *codec);
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream);
	
	virtual ~ExternalDecoderInfo ();

	virtual const char *GetName () { return name; }
};

/*
 * ASX demuxer
 */ 
class ASXDemuxer : public IMediaDemuxer {
private:
	Playlist *playlist;

protected:
	virtual ~ASXDemuxer ();
	virtual MediaResult SeekInternal (guint64 pts) { return MEDIA_FAIL; }

	virtual void GetFrameAsyncInternal (IMediaStream *stream) { ReportErrorOccurred ("GetFrameAsync isn't supported for ASXDemuxer"); }
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 seekToTime) {}
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream) {}
	
public:
	ASXDemuxer (Media *media, IMediaSource *source);
	virtual void Dispose ();
	
	virtual bool IsPlaylist () { return true; }
	virtual Playlist *GetPlaylist () { return playlist; }
	virtual const char *GetName () { return "ASXDemuxer"; }
};

class ASXDemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "ASXDemuxer"; }
};

class MarkerStream : public IMediaStream {
private:
	MediaMarkerFoundClosure *closure;
	Mutex mutex;
	List list; // a list of markers found while there were no callback.
	
protected:
	virtual ~MarkerStream () {}

public:
	MarkerStream (Media *media);
	virtual void Dispose ();
	
	virtual MediaStreamType GetType () { return MediaTypeMarker; }
	
	void SetCallback (MediaMarkerFoundClosure *closure);

	// Since the markers are taking the wrong way through the pipeline 
	// (it's the pipeline who is pushing the markers up to the consumer, 
	// not the consumer reading new markers), this works in the following way:
	// The demuxer reaches a marker somehow, creates a MediaFrame with the marker data and calls MarkerFound on the MarkerStream.
	// The MarkerStream calls the decoder to decode the frame.
	// The decoder must create an instance of MediaMarker and store it in the frame's buffer.
	// The MarkerStream then calls the closure with the MediaMarker.
	// Cleanup:
	// 	- The stream (in MarkerFound) frees the MediaMarker.
	//  - The demuxer frees the MediaFrame, and the original frame buffer (before decoding).
	void MarkerFound (MediaFrame *frame);
	
	virtual void FrameEnqueued ();
	MediaMarker *Pop ();
};

class PassThroughDecoder : public IMediaDecoder {
protected:
	virtual ~PassThroughDecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();

public:
	PassThroughDecoder (Media *media, IMediaStream *stream);
	virtual void Dispose ();
	
	virtual const char *GetName () { return "PassThroughDecoder"; }
};

class PassThroughDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char *codec);
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream)
	{
		return new PassThroughDecoder (media, stream);
	}
	virtual const char *GetName () { return "PassThroughDecoder"; }
};

class NullDecoder : public IMediaDecoder {
private:
	// Video data
	guint8 *logo;
	guint32 logo_size;

	// Audio datarf
	guint64 prev_pts;
	
	MediaResult DecodeVideoFrame (MediaFrame *frame);
	MediaResult DecodeAudioFrame (MediaFrame *frame);
	MediaResult OpenAudio ();
	MediaResult OpenVideo ();
	
protected:
	virtual ~NullDecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();

public:
	NullDecoder (Media *media, IMediaStream *stream);
	virtual void Dispose ();
	
	virtual const char *GetName () { return "NullDecoder"; }
};

class NullDecoderInfo : public DecoderInfo {	
public:
	virtual bool Supports (const char *codec);
	
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream)
	{
		return new NullDecoder (media, stream);
	}	
	virtual const char *GetName () { return "NullDecoder"; }
};

#endif
