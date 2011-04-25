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
#define CODEC_MP4A	MAKE_CODEC_ID ('M', 'P', '4', 'A')
#define CODEC_mp4a	MAKE_CODEC_ID ('m', 'p', '4', 'a')
#define CODEC_AVC1	MAKE_CODEC_ID ('A', 'V', 'C', '1')
#define CODEC_avc1	MAKE_CODEC_ID ('a', 'v', 'c', '1')
#define CODEC_H264	MAKE_CODEC_ID ('H', '2', '6', '4')
#define CODEC_MP3	0x55
#define CODEC_WMAV1 0x160
#define CODEC_WMAV2 0x161
#define CODEC_WMAV3 0x162
#define CODEC_PCM   0x1

#define CODEC_MPEG_AAC		0x00FF
#define CODEC_MPEG_ADTS_AAC	0x1600
#define CODEC_MPEG_RAW_AAC	0x1601
#define CODEC_MPEG_HEAAC	0x1610

/* this is an internal codec id used by moonlight, the fourcc is invalid on purpose */
#define CODEC_ASF_MARKER MAKE_CODEC_ID ('|', '@', '#', '%')

#define MAX_VIDEO_HEIGHT	2048
#define MAX_VIDEO_WIDTH		2048

namespace Moonlight {

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
class MemoryBuffer;
class MediaLog;

/* @CBindingRequisite */
typedef gint32 MediaResult;

#define MEDIA_SUCCESS ((MediaResult) 0)
#define MEDIA_FAIL ((MediaResult) 1)
#define MEDIA_UNKNOWN_CODEC ((MediaResult) 3)
#define MEDIA_NO_CALLBACK ((MediaResult) 15)
// This value might be returned by the pipeline for an open
// request, indicating that there is not enough data available
// to open the media.
// It is also used internally in the pipeline whenever something
// can't be done because of not enough data.
// Note: this value must not be used for eof condition.
#define MEDIA_NOT_ENOUGH_DATA ((MediaResult) 19)
#define MEDIA_INVALID ((MediaResult) 0xFFFFFFFF)

#define MEDIA_SUCCEEDED(x) (((x) <= 0))

#if SANITY
#define VERIFY_MEDIA_THREAD \
	if (!Media::InMediaThread ()) {	\
		printf ("Moonlight: This method should only be called from the media thread (%s)\n", __PRETTY_FUNCTION__);	\
		print_stack_trace (); \
	}
#else
#define VERIFY_MEDIA_THREAD 
#endif

/* @CBindingRequisite */
typedef MediaResult MediaCallback (MediaClosure *closure);

};

#include "list.h"
#include "debug.h"
#include "dependencyobject.h"
#include "error.h"
#include "type.h"
#include "enums.h"
#include "application.h"

namespace Moonlight {

/*
 * Range
 */
struct Range {
	guint64 offset;
	guint64 length;
	bool Contains (guint64 offset, guint64 length)
	{
		return this->offset <= offset && this->offset + this->length >= offset + length;
	}
	Range ()
	{
		offset = 0;
		length = 0;
	}
};

/*
 * Ranges
 */
class Ranges {
private:
	Range *ranges;
	gint32 count; /* number of elements in the array*/

public:
	Ranges ();
	~Ranges ();
	void Add (guint64 offset, guint32 length);
	bool Contains (guint64 offset, guint64 length);
	/* returns the first value not in the range, or G_MAXUINT64 if not found */
	guint64 GetFirstNotInRange (guint64 offset, guint64 length);
};

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
	/* @SkipFactories */
	MediaClosure (Type::Kind object_type, Media *media, MediaCallback *callback, EventObject *context);
	
public:
	/* @GenerateCBinding */
	/* @SkipFactories */
	MediaClosure (Media *media, MediaCallback *callback, EventObject *context, const char *description);
	virtual void Dispose ();
	
	void Call (); // Calls the callback and stores the result in 'result', then calls the 'finished' callback
	
	bool CallExecuted () { return result != MEDIA_INVALID; }

	MediaResult GetResult () { return result; }
	Media *GetMedia () { return media; }
	/* @GenerateCBinding */
	EventObject *GetContext () { return context; }
	const char *GetDescription () { return description != NULL ? description : GetTypeName (); }

	class Node : public List::Node {
	public:
		MediaClosure *closure;
		Node (MediaClosure *c) { closure = c; closure->ref (); }
		virtual ~Node () { closure->unref (); }
	};
};

class MediaReadClosure : public MediaClosure {
private:
	MemoryBuffer *data;
	gint64 offset;
	guint32 count;
	bool cancelled;

public:
	/* @SkipFactories */
	MediaReadClosure (Media *media, MediaCallback *callback, EventObject *context, gint64 offset, guint32 count);
	virtual void Dispose ();

	/* The 0-based offset where to begin reading */
	gint64 GetOffset () { return offset; }
	/* The requested number of bytes. This is not necessarily the number of bytes returned (if eof is reached for instance) */
	guint32 GetCount () { return count; }
	void SetData (MemoryBuffer *data);
	MemoryBuffer *GetData () { return data; }
	bool IsCancelled () { return cancelled; }
	void Cancel () { cancelled = true; } /* Media-thread only */
	void SetCount (guint32 value) { count = value; }
};

/*
 * MediaDisposeObjectClosure
 */
class MediaDisposeObjectClosure : public MediaClosure {
public:
	/* @SkipFactories */
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
	/* @SkipFactories */
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
	/* @SkipFactories */
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
	/* @SkipFactories */
	MediaReportFrameCompletedClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, MediaFrame *frame);
	virtual void Dispose ();
	
	MediaFrame *GetFrame () { return frame; }
	IMediaDemuxer *GetDemuxer () { return (IMediaDemuxer *) GetContext (); }
};

/*
 * MediaReportDecodeFrameCompletedClosure
 */
class MediaReportDecodeFrameCompletedClosure : public MediaClosure {
private:
	MediaFrame *frame;

protected:
	virtual ~MediaReportDecodeFrameCompletedClosure () {}

public:
	/* @SkipFactories */
	MediaReportDecodeFrameCompletedClosure (Media *media, MediaCallback *callback, IMediaDecoder *context, MediaFrame *frame);
	virtual void Dispose ();
	
	MediaFrame *GetFrame () { return frame; }
	IMediaDecoder *GetDecoder () { return (IMediaDecoder *) GetContext (); }
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
	/* @SkipFactories */
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
	/* @SkipFactories */
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
	/* Check if a demuxer supports this source
	 * @source: the beginning of the media.
	 * Possible return values:
	 * - MEDIA_SUCCESS: the demuxer supports this source
	 * - MEDIA_FAIL: the demuxer does not support this source
	 * - MEDIA_NOT_ENOUGH_DATA: the source doesn't have enough data available for the demuxer to know if it's a supported format or not.
	 */
	virtual MediaResult Supports (MemoryBuffer *source) = 0; 
	/* Create the demuxer
	 *
	 * @media: the Media instance
	 * @source: the source used to read data
	 * @initial_buffer: the initial buffer the pipeline used to find a demuxer.
	 */
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer) = 0;
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
	double offset;
	
	ProgressEventArgs (double progress, double offset)
	{
		this->progress = progress;
		this->offset = offset;
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
	class PendingEmitList : public List::Node {
	public:
		List list;
	};
	List *events; // list of event handlers
	List *emit_on_main_thread; // list of lists of emit calls to emit on main thread
	Mutex event_mutex;
	
	void EmitList (List *list);
	void EmitListMain ();
	static void EmitListCallback (EventObject *obj);
	
protected:
	virtual ~IMediaObject () {}

public:
	IMediaObject (Type::Kind kind, Media *media);
	virtual void Dispose ();
	
	/* @GeneratePInvoke */
	Media *GetMediaReffed ();
	void SetMedia (Media *value);

	void ReportErrorOccurred (ErrorEventArgs *args);
	/* @GeneratePInvoke */
	void ReportErrorOccurred (const char *message);
	
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
	guint64 last_popped_decoded_pts; // The pts of the last frame returned, initialized to G_MAXUINT64
	guint64 last_enqueued_demuxed_pts; // The pts of the last demuxed frame enqueued, initialized to G_MAXUINT64
	guint64 last_enqueued_decoded_pts; // The pts of the last decoded frame enqueued, initialized to G_MAXUINT64
	guint64 last_available_pts; // The last pts available, initialized to 0. Note that this field won't be correct for streams which CanSeekToPts.
	Mutex queue_mutex;
	List demuxed_queue; // Our queue of demuxed frames
	List decoded_queue; // Our queue of decoded frames
	IMediaDecoder *decoder;

	void *extra_data;
	gint32 extra_data_size;
	void *raw_extra_data;
	gint32 raw_extra_data_size;
	gint32 codec_id;
	guint64 duration; // 100-nanosecond units (pts)
	char *codec; // freed upon destruction
	// The minimum amount of padding any other part of the pipeline needs for frames from this stream.
	// Used by the demuxer when reading frames, ensures that there are at least min_padding extra bytes
	// at the end of the frame data (all initialized to 0).
	gint32 min_padding;
	// 0-based index of the stream in the media
	// set by the demuxer, until then its value must be -1
	gint32 index; 
	guint64 pts_per_frame; // Duration (in pts) of each frame. Set to 0 if unknown.

protected:
	virtual ~IMediaStream () {}
	virtual void DecodedFrameEnqueued () {}

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

	/* @SkipFactories */
	IMediaStream (Type::Kind kind, Media *media);
	virtual void Dispose ();

	/* This method is deprecated, use the Is<stream type> versions instead. */
	/* @GenerateCBinding */
	MediaStreamType GetStreamType ()
	{
		if (IsVideo ()) return MediaTypeVideo;
		if (IsAudio ()) return MediaTypeAudio;
		return MediaTypeMarker;
	}

	/* @GenerateCBinding */
	bool IsVideo () { return GetObjectType () == Type::VIDEOSTREAM; }
	/* @GenerateCBinding */
	bool IsAudio () { return GetObjectType () == Type::AUDIOSTREAM; }
	/* @GenerateCBinding */
	bool IsMarker () { return GetObjectType () == Type::MARKERSTREAM; }
	bool IsAudioOrVideo () { return IsVideo () || IsAudio (); }

	IMediaDecoder *GetDecoder ();
	void SetDecoder (IMediaDecoder *value);
	
	/* @GenerateCBinding */
	const char *GetCodec () { return codec; }

	bool GetSelected () { return selected; }
	void SetSelected (bool value);
	
	void EnqueueDemuxedFrame (MediaFrame *frame);
	void EnqueueDecodedFrame (MediaFrame *frame);
	MediaFrame *PopDecodedFrame ();
	MediaFrame *PopDemuxedFrame ();
	bool IsDecodedQueueEmpty ();
	void ClearQueue ();
	gint32 GetDemuxedQueueLength ();
	gint32 GetDecodedQueueLength ();
	guint64 GetFirstPts () { return first_pts; }
	guint64 GetLastPoppedPts () { return last_popped_decoded_pts; }
	guint64 GetLastEnqueuedDemuxedPts () { return last_enqueued_demuxed_pts; }
	guint64 GetLastEnqueuedDecodedPts () { return last_enqueued_decoded_pts; }
	guint64 GetBufferedSize (); // Returns the time between the last frame returned and the last frame available (buffer time)

	guint32 GetGeneration ();
	
	gint32 GetIndex () { return index; }
	void SetIndex (gint32 value) { index = value; }

	gint32 GetMinPadding () { return min_padding; }
	void SetMinPadding (gint32 value) { min_padding = MAX (min_padding, value); }

	/* @GenerateCBinding */
	gint32 GetExtraDataSize () { return extra_data_size; }
	/* @GenerateCBinding */
	gint32 GetRawExtraDataSize () { return raw_extra_data_size; }
	/* @GenerateCBinding */
	void SetExtraDataSize (gint32 value) { extra_data_size = value; }
	/* @GenerateCBinding */
	void SetRawExtraDataSize (gint32 value) { raw_extra_data_size = value; }
	/* @GenerateCBinding */
	void *GetExtraData () { return extra_data; }
	/* @GenerateCBinding */
	void *GetRawExtraData () { return raw_extra_data; }
	/* @GenerateCBinding */
	void SetExtraData (void *value) { extra_data = value; }
	/* @GenerateCBinding */
	void SetRawExtraData (void *value) { raw_extra_data = value; }
	/* @GenerateCBinding */
	gint32 GetCodecId () { return codec_id; }
	/* @GenerateCBinding */
	void SetCodecId (int value);
	/* @GenerateCBinding */
	guint64 GetDuration () { return duration; }
	/* @GenerateCBinding */
	void SetDuration (guint64 value) { duration = value; }

	/* @GenerateCBinding */
	bool GetInputEnded ();
	/* @GenerateCBinding */
	void SetInputEnded (bool value);
	/* @GenerateCBinding */
	bool GetOutputEnded ();
	/* @GenerateCBinding */
	void SetOutputEnded (bool value);
	
	/* @GenerateCBinding */
	IMediaDemuxer *GetDemuxerReffed ();
	
	void SetPtsPerFrame (guint64 value) { pts_per_frame = value; }
	guint64 GetPtsPerFrame () { return pts_per_frame; }

	void ReportSeekCompleted ();
#if DEBUG
	void PrintBufferInformation ();
#endif
	/* This event is emitted when a decoded frame frame is enqueued and the list of decoded frames is empty */
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
	static bool registered_ms_codecs1;

	Mutex mutex;
	
	guint64 target_pts; // Access must be protected with mutes.
	guint64 buffering_time; // Access must be protected with mutex.
	bool is_disposed; // Access must be protected with mutex. This is used to ensure that we don't add work to the thread pool after having been disposed.
	Uri *uri;
	Uri *resource_base;
	Uri *final_uri; // main thread only
	char *file;
	IMediaSource *source;
	IMediaDemuxer *demuxer;
	List *markers;
	bool seek_when_opened;
	bool initialized;
	bool opened;
	bool opening;
	bool stopped;
	bool error_reported; // If an error has been reported.
	bool in_open_internal; // detect recursive calls to OpenInternal
	bool http_retried;
	bool is_cross_domain;
	double download_progress;
	double buffering_progress;
	TimeSpan start_time;
	TimeSpan duration;
	MediaLog *log;
	
	PlaylistEntry *entry;

	// Determines the container type and selects a demuxer. We have support for mp3 and asf demuxers.
	// Also opens the demuxer.
	// This method is supposed to be called multiple times, until either 'error_reported' is true or this method
	// returns true. It will pick up wherever it left working last time.
	bool SelectDemuxerAsync ();
	bool SelectDemuxerAsync (MediaReadClosure *closure);
	static MediaResult SelectDemuxerReadCallback (MediaClosure *closure);
	
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
	/* @SkipFactories */
	Media (PlaylistEntry *entry);
	
	virtual void Dispose ();
	
	static bool InMediaThread ();
	/* @GenerateCBinding */
	bool EnqueueWork (MediaClosure *closure);
	
	// Calls obj->Dispose on the media thread.
	void DisposeObject (EventObject *obj);
	
	// Initialize the Media.
	// These methods may raise MediaError events.
	void Initialize (Downloader *downloader, const char *PartName); // MediaElement.SetSource (dl, 'PartName');
	void Initialize (const Uri *resource_base, const Uri *uri); // MediaElement.Source = 'uri';
	void Initialize (IMediaDemuxer *demuxer); // MediaElement.SetSource (demuxer);
	void Initialize (IMediaSource *source);

	// Start opening the media.
	// When done, OpenCompleted event is raised.
	// In case of failure, MediaError event is raised	
	void OpenAsync ();
	
	void ReportOpenDemuxerCompleted (); // This method is called by the demuxer when it has opened.
	void ReportOpenDecoderCompleted (IMediaDecoder *decoder); // This method is called by any of the decoders when it has opened.
	
	void ReportDownloadProgress (double progress, double offset, bool force);
	void ReportBufferingProgress (double progress);
	
	// Media playback
	void PlayAsync (); // Raises CurrentStateChanged
	void PauseAsync (); // Raises CurrentStateChanged
	void StopAsync (); // Raises CurrentStateChanged
	// Seek to the specified pts
	// When done, SeekCompleted is raised
	// In case of failure, MediaError is raised.
	void SeekAsync (guint64 pts);
	void ReportSeekCompleted (guint64 pts); // This method is called by IMediaDemuxer when a seek is completed. Raises the SeekCompleted event.
		
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	
	void SetBufferingTime (guint64 buffering_time);
	guint64 GetBufferingTime ();

	void SetTargetPts (guint64 value);
	guint64 GetTargetPts ();

	IMediaSource *GetSource () { return source; }
	IMediaDemuxer *GetDemuxerReffed (); /* thread-safe */
	const char *GetFile () { return file; }
	const Uri *GetUri () { return uri; }
	void SetFileOrUrl (const char *value);

	void SetStartTime (TimeSpan value); // main thread only, before Initialize has been called
	TimeSpan GetStartTime () { return start_time; }

	void SetDuration (TimeSpan value); // main thread only, before Initialize has been called
	TimeSpan GetDuration () { return duration; }
	
	// A list of MediaMarker::Node.
	// This is the list of markers found in the metadata/headers (not as a separate stream).
	// Will never return NULL.
	List *GetMarkers ();
	double GetDownloadProgress () { return download_progress; }
	double GetBufferingProgress () { return buffering_progress; }
	void ClearBufferingProgress (); /* Thread-safe */
	
	Playlist *GetPlaylistRoot ();
	PlaylistEntry *GetPlaylistEntry ();
	
	bool IsOpened () { return opened; }
	bool IsOpening () { return opening; }
	bool IsStopped () { return stopped; }
	bool IsInitialized () { return initialized; }
	
	void RetryHttp ();
	
	void ReportErrorOccurred (ErrorEventArgs *args);
	void ReportErrorOccurred (const char *message);
	
	bool HasReportedError () { return error_reported; }

	bool GetSeekWhenOpened () { return seek_when_opened; }
	void SetSeekWhenOpened (bool value) { seek_when_opened = value; }

	void SetFinalUri (const Uri *value); // main thread only
	const Uri *GetFinalUri (); // main thread only
	
	MediaLog *GetLog () { return log; }

	bool GetIsCrossDomain () { return is_cross_domain; }
	void SetIsCrossDomain (bool value) { is_cross_domain = value; }

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
	
	static void RegisterMSCodecs (); // Private implementation
	static bool IsMSCodecsInstalled ();
	static bool IsMSCodecs1Installed ();
	static void InstallMSCodecs (bool is_user_initiated);
	
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
	static void AddWork (MediaClosure *closure);
	static void Initialize ();
	static void Shutdown ();

	// this method checks if the current thread is a thread-pool thread
	static bool IsThreadPoolThread ();
};
 
class MediaFrame : public EventObject {
private:
	// The demuxer sets these to the encoded data which the
	// decoder then uses and replaces with the decoded data.
	guint8 *buffer;
	guint32 buflen;
	guint32 generation;
	guint64 duration;
	guint16 state; // Current state of the frame

	void Initialize ();
	
protected:
	virtual ~MediaFrame ();
	
public:
	/* @SkipFactories */
	MediaFrame (IMediaStream *stream);
	/* @GeneratePInvoke */
	/* @SkipFactories */
	MediaFrame (IMediaStream *stream, guint8 *buffer, guint32 buflen, guint64 pts, bool keyframe);
	void Dispose ();
	
	/* @GenerateCBinding */
	void AddState (MediaFrameState state) { this->state |= (guint16) state; }
	void RemoveState (MediaFrameState state) { this->state &= ~((guint16) state); }
	bool IsDecoded () { return (((MediaFrameState) state) & MediaFrameDecoded) == MediaFrameDecoded; }
	bool IsDemuxed () { return (((MediaFrameState) state) & MediaFrameDemuxed) == MediaFrameDemuxed; }
	bool IsConverted () { return (((MediaFrameState) state) & MediaFrameConverted) == MediaFrameConverted; }
	bool IsPlanar () { return (((MediaFrameState) state) & MediaFramePlanar) == MediaFramePlanar; }
	bool IsVUY2 () { return (((MediaFrameState) state) & MediaFrameVUY2) == MediaFrameVUY2; }
	/* @GenerateCBinding */
	bool IsKeyFrame () { return (((MediaFrameState) state) & MediaFrameKeyFrame) == MediaFrameKeyFrame; }
	bool IsMarker () { return (((MediaFrameState) state) & MediaFrameMarker) == MediaFrameMarker; }
	
	IMediaStream *stream;
	MediaMarker *marker;
	void *decoder_specific_data; // data specific to the decoder
	guint64 pts; // Set by the demuxer

	guint16 event; // special frame event if non-0
	
	// planar data
	guint8 *data_stride[4]; // Set by the decoder
	int srcSlideY; // Set by the decoder
	int srcSlideH; // Set by the decoder
	int srcStride[4]; // Set by the decoder
	
	// The decoded size of the frame (might be bigger or smaller than the size of the stream).
	// 0 = the size specified in the stream
	gint32 width;
	gint32 height;

	// If the demuxer knows the size of the frame, here goes those values.
	// We don't use these values, except to validate them against width/height (if set) or the stream's width/height.
	// DRT #7008.
	gint32 demuxer_width;
	gint32 demuxer_height;

	/* Allocates the buffer. Reports an error and returns false if buffer couldn't be allocated. */
	bool AllocateBuffer (guint32 size);

	/* @GeneratePInvoke */
	bool AllocateBuffer (guint32 size, guint32 alignment);

	/* @GenerateCBinding */
	void FreeBuffer ();

	/* Allocates the buffer and reads 'size' bytes from data into it. Reports any errors and returns false in case of errors. */
	bool FetchData (guint32 size, void *data);
	/* Creates a new buffer which is 'size' bytes bigger, copies 'data' into it and then the previous buffer after that */
	bool PrependData (guint32 size, void *data);

	guint32 GetGeneration () { return generation; }
	IMediaStream *GetStream () { return stream; }

	guint64 GetDuration () { return duration; }
	void SetDuration (guint64 value) { duration = value; }
	
	/* @GenerateCBinding */
	guint32 GetBufLen () { return buflen; }
	/* @GenerateCBinding */
	void SetBufLen (guint32 value) { buflen = value; }
	/* @GeneratePInvoke */
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

	/* @GeneratePInvoke */
	void SetDemuxerWidth (gint32 value) { demuxer_width = value; }
	gint32 GetDemuxerWidth () { return demuxer_width; }
	/* @GeneratePInvoke */
	void SetDemuxerHeight (gint32 value) { demuxer_height = value; }
	gint32 GetDemuxerHeight () { return demuxer_height; }

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

	/* @SkipFactories */
	MediaMarker (const char *type, const char *text, guint64 pts);
	
	const char *Type () { return type; }
	const char *Text () { return text; }
	guint64 Pts () { return pts; }
};

// Interfaces

/* @Namespace=None,ManagedEvents=Manual */
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
	guint32 generation; /* Incremented when seeked, any frames with a lower generation will automatically be dropped when enqueued */
	bool opened;
	bool opening;
	bool seeking; /* Only media thread may access, no lock required. When set, the demuxer should not request new frames */
	bool seek_pending; /* If we've called the derived class' SeekInternal and are waiting for a response */
	bool drm; /* If the content this demuxer is demuxing is drm-protected */
	/* 
	 * Set on main thread, read/reset on media thread: access needs mutex locked. 
	 * When a seek is pending, indicates the position we should seek to. We specifically
	 * do not enqueue the pts with the seek request - this would cause
	 * multiple seeks with unpredictable ordering when SeekAsync is called again before the
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
	static MediaResult ReportOpenDemuxerCompletedCallback (MediaClosure *closure);
	
	void FillBuffersInternal ();
	
protected:
	IMediaSource *source;
	
	/* @SkipFactories */
	IMediaDemuxer (Type::Kind kind, Media *media, IMediaSource *source);
	/* @SkipFactories */
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
	void EnqueueReportOpenDemuxerCompleted ();
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
	
	/* @GeneratePInvoke */
	void ReportOpenDemuxerCompleted ();
	/* @GeneratePInvoke */
	void ReportGetFrameCompleted (MediaFrame *frame);
	/* @GeneratePInvoke */
	void ReportGetFrameProgress (double bufferingProgress);
	/* @GeneratePInvoke */
	void ReportSeekCompleted (guint64 pts);
	/* @GeneratePInvoke */
	void ReportSwitchMediaStreamCompleted (IMediaStream *stream);
	/* @GeneratePInvoke */
	void ReportGetDiagnosticCompleted (MediaStreamSourceDiagnosticKind diagnosticKind, gint64 diagnosticValue);
	
	guint64 GetBufferedSize ();
	guint32 GetGeneration () { return generation; }

	/* @GenerateCBinding */
	void FillBuffers ();
	void FillBuffersSync () { FillBuffersInternal (); }
	void ClearBuffers ();
	
	void PrintBufferInformation ();
	guint64 GetSeekedToPts () { return seeked_to_pts; }
	
	int GetStreamCount () { return stream_count; }
	IMediaStream *GetStream (int index);
	// Gets the longest duration from all the streams
	virtual guint64 GetDuration (); // 100-nanosecond units (pts)
	virtual void UpdateSelected (IMediaStream *stream) {};
	
	IMediaSource *GetSource () { return source; }
	bool IsOpened () { return opened; }
	bool IsOpening () { return opening; }
	
	virtual bool GetCanSeek () { return true; }
	virtual bool IsPlaylist () { return false; }
	virtual PlaylistEntry *GetPlaylist () { return NULL; }

	/* @GeneratePInvoke */
	void SetIsDrm (bool value) { drm = value; }
	bool IsDrm () { return drm; }

	IMediaStream *GetPendingStream () { return pending_stream; }

	const static int OpenedEvent;
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
	static MediaResult ReportOpenDecoderCompletedCallback (MediaClosure *closure);
	static MediaResult ReportDecodeFrameCompletedCallback (MediaClosure *closure);
	
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
	/* @SkipFactories */
	IMediaDecoder (Type::Kind kind, Media *media, IMediaStream *stream);
	virtual void Dispose ();
	
	// If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	virtual void Cleanup (MediaFrame *frame) {}
	virtual void CleanState () {}
	virtual bool HasDelayedFrame () { return false; }
	
	virtual const char *q () { return GetTypeName (); }
	
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
	
	/* @SkipFactories */
	IImageConverter (Type::Kind kind, Media *media, VideoStream *stream);
	
	/* Opens the converter. If false is returned, ReportErrorOccurred must have been called */
	virtual bool Open () = 0;
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

protected:
	virtual ~IMediaSource ();

	void Lock ();
	void Unlock ();

	// All these methods must/will be called with the lock locked.	
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();
	virtual void ReadAsyncInternal (MediaReadClosure *closure) = 0;
	
	void ReadFD (FILE *read_fd, MediaReadClosure *closure);
	
public:
	/* @SkipFactories */
	IMediaSource (Type::Kind kind, Media *media);
	virtual void Dispose ();
	
	// Initializes this stream (and if it succeeds, it can be read from later on).
	// streams based on remote content (live/progress) should contact the server
	// and try to start downloading content
	// file streams should try to open the file
	virtual MediaResult Initialize () = 0;

	// Reads the requested number of bytes asynchronously.
	// The closure is called when the read completes.
	// This is always done asynchronously.
	void ReadAsync (MediaReadClosure *closure);
	
	virtual bool CanSeek () { return true; }

	// Seeks to the specified 'pts'.
	virtual bool CanSeekToPts () { return false; }
	virtual MediaResult SeekToPts (guint64 pts) { return MEDIA_FAIL; }

	// Returns the size of the source. This method may return -1 if the
	// size isn't known.
	// This method will lock the mutex.
	gint64 GetSize ();

	virtual bool Eof () = 0;

	// If the derived class knows which demuxer it needs, 
	// it should override this method and return a new demuxer.
	virtual IMediaDemuxer *CreateDemuxer (Media *media, MemoryBuffer *inital_buffer) { return NULL; }
};

class ManagedStreamSource : public IMediaSource {
private:
	ManagedStreamCallbacks stream;
	
protected:	
	virtual ~ManagedStreamSource ();

	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();
	virtual void ReadAsyncInternal (MediaReadClosure *closure);

public:
	/* @SkipFactories */
	ManagedStreamSource (Media *media, ManagedStreamCallbacks *stream);
	
	virtual MediaResult Initialize () { return MEDIA_SUCCESS; }
	
	virtual bool Eof () { return GetPositionInternal () == GetSizeInternal (); }
};
 
class FileSource : public IMediaSource {
private:
	gint64 size;
	FILE *fd;
	char *filename;

protected:
	
	virtual ~FileSource ();
	
	virtual void ReadAsyncInternal (MediaReadClosure *closure);
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();

public:
	/* @SkipFactories */
	FileSource (Media *media, const char *filename);

	virtual void Dispose ();
	virtual MediaResult Initialize (); 
	virtual bool Eof ();
	const char *GetFileName () { return filename; }
};

class ProgressiveSource : public IMediaSource {
private:
	bool complete; /* If the file has been fully downloaded */ 
	bool error_occurred;
	gint64 write_pos;
	gint64 size;
	Mutex mutex;
	List read_closures;
	// To avoid locking while reading and writing (since reading is done on 
	// the media thread and writing on the main thread), we open two file
	// handlers, one for reading and one for writing.
	FILE *write_fd;
	FILE *read_fd;
	char *filename;
	Uri *uri;
	Uri *resource_base;
	int brr_enabled; /* If we'll do byte range requests (if server sent 'Accept-Ranges: bytes' or not) 0: not checked, 1: yes, 2: no. Main thread only. */
	Cancellable *cancellable; /* Write on main thread & Read on other threads require lock to be held. Reads on main thread is safe. */

	/* A list of all the ranges we've downloaded */
	Ranges ranges; /* write on main thread, read on media thread: all accesses needs mutex locked) */
	gint64 current_request; /* The last range request made */
	guint64 current_request_received; /* The # of bytes received from the current range request */
	HttpRequest *range_request; /* Main thread only */

	/* We calculate approximately the download speed to know when to emit a byte range request or not.
	 * (if we calculate that we'll reach the position we want in a few seconds, don't emit a byte
	 * range request */
	guint64 bytes_received; /* The total number of bytes received. Main thread only */
	TimeSpan first_reception; /* The time when the first chunk was received. Main thread only. */

	void SetRangeRequest (HttpRequest *value);

	EVENTHANDLER (ProgressiveSource, RangeStarted, HttpRequest, EventArgs);
	EVENTHANDLER (ProgressiveSource, RangeWrite,   HttpRequest, HttpRequestWriteEventArgs);
	EVENTHANDLER (ProgressiveSource, RangeStopped, HttpRequest, HttpRequestStoppedEventArgs);

	static void DataWriteCallback (EventObject *sender, EventArgs *args, void *closure);
	static void NotifyCallback (NotifyType type, gint64 args, void *closure);
	static void DeleteCancellable (EventObject *data);
	static MediaResult CheckPendingReadsCallback (MediaClosure *data);
	static void CheckReadRequestsCallback (EventObject *data);

	void CheckReadRequests ();
	void CheckPendingReads ();
	
	void Notify (NotifyType, gint64 args);

	void DownloadComplete ();
	void DownloadFailed ();
	void DataWrite (void *data, gint32 offset, gint32 n);
	
	void Read (MediaReadClosure *closure);
	gint32 CalculateDownloadSpeed (); /* bps */
	double GetDownloadProgressOffset ();
	
protected:
	virtual ~ProgressiveSource ();
	
	virtual void ReadAsyncInternal (MediaReadClosure *closure);
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal () { return size; }

public:
	/* @SkipFactories */
	ProgressiveSource (Media *media, const Uri *resource_base, const Uri *uri);
	virtual void Dispose ();

	virtual bool Eof ();
		
	virtual MediaResult Initialize (); 
	
	Uri *GetUri () { return uri; }
	const char *GetFileName () { return filename; }
#if OBJECT_TRACKING
	guint32 GetPendingReadRequestCount ();
#endif
};

/*
 * MemoryBuffer
 */
class MemoryBuffer : public IMediaObject {
private:
	void *memory;
	gint32 size;
	gint32 pos;
	bool owner;

protected:
	virtual ~MemoryBuffer ();

public:
	/* @SkipFactories */
	MemoryBuffer (Media *media, void *memory, gint32 size, bool owner);

	void *GetCurrentPtr () { return pos + (guint8 *) memory; }
	gint64 GetSize () { return size; }
	gint64 GetPosition () { return pos; }

	bool Peek (void *buffer, guint32 count);
	bool Read (void *buffer, guint32 count);
	void SeekOffset (gint32 offset); /* This is relative to the current position */
	void SeekSet (gint32 position); /* This is relative to the starting position */
	gint64 GetRemainingSize () { return GetSize () - GetPosition (); }
	
	/* These methods do the appropiate fixups based on endian-ness */
#define CHECKSIZE(bytes) \
	if ((gint64) pos + bytes > (gint64) size) { \
		g_warning ("MemoryBuffer::Read: not enough data. Caller must check for available data first.\n");	\
		return 0;	\
	}
	// Warning, full 64-bit reads appear to be causing a SIGBUS on some android devices, lets do a 32-bit read and OR it.
	gint64   ReadBE_I64 () {
		CHECKSIZE (8);
		gint64 a = (guint64) ReadBE_I32 ();
		gint64 b = (guint64) ReadBE_I32 ();

		return GINT64_FROM_BE ((b << 32) + a);
	}
	gint32   ReadBE_I32 () { CHECKSIZE (4); gint32  result = GINT32_FROM_BE  (*(gint32  *) (((gint8  *) memory) + pos)); pos += 4; return result; }
	gint16   ReadBE_I16 () { CHECKSIZE (2); gint16  result = GINT16_FROM_BE  (*(gint16  *) (((gint8  *) memory) + pos)); pos += 2; return result; }
	gint8    ReadBE_I8  () { CHECKSIZE (1); gint8   result =                 (*(gint8   *) (((gint8  *) memory) + pos)); pos += 1; return result; }
	guint64  ReadBE_U64 () {
		CHECKSIZE (8);
		guint64 a = (guint64) ReadBE_U32 ();
		guint64 b = (guint64) ReadBE_U32 ();

		return GUINT64_FROM_BE ((b << 32) + a);
	}
	guint32  ReadBE_U32 () { CHECKSIZE (4); guint32 result = GUINT32_FROM_BE (*(guint32 *) (((guint8 *) memory) + pos)); pos += 4; return result; }
	guint16  ReadBE_U16 () { CHECKSIZE (2); guint16 result = GUINT16_FROM_BE (*(guint16 *) (((guint8 *) memory) + pos)); pos += 2; return result; }
	guint8   ReadBE_U8  () { CHECKSIZE (1); guint8  result =                 (*(guint8  *) (((guint8 *) memory) + pos)); pos += 1; return result; }
	gint64   ReadLE_I64 () {
		CHECKSIZE (8);
		gint64 a = (guint64) ReadLE_I32 ();
		gint64 b = (guint64) ReadLE_I32 ();

		return GINT64_FROM_LE ((b << 32) + a);
	}
	gint32   ReadLE_I32 () { CHECKSIZE (4); gint32  result = GINT32_FROM_LE  (*(gint32  *) (((gint8  *) memory) + pos)); pos += 4; return result; }
	gint16   ReadLE_I16 () { CHECKSIZE (2); gint16  result = GINT16_FROM_LE  (*(gint16  *) (((gint8  *) memory) + pos)); pos += 2; return result; }
	gint8    ReadLE_I8  () { CHECKSIZE (1); gint8   result =                 (*(gint8   *) (((gint8  *) memory) + pos)); pos += 1; return result; }
	guint64  ReadLE_U64 () {
		CHECKSIZE (8);
		guint64 a = (guint64) ReadLE_U32 ();
		guint64 b = (guint64) ReadLE_U32 ();

		return GUINT64_FROM_LE ((b << 32) + a);
	}
	guint32  ReadLE_U32 () { CHECKSIZE (4); guint32 result = GUINT32_FROM_LE (*(guint32 *) (((guint8 *) memory) + pos)); pos += 4; return result; }
	guint16  ReadLE_U16 () { CHECKSIZE (2); guint16 result = GUINT16_FROM_LE (*(guint16 *) (((guint8 *) memory) + pos)); pos += 2; return result; }
	guint8   ReadLE_U8  () { CHECKSIZE (1); guint8  result =                 (*(guint8  *) (((guint8 *) memory) + pos)); pos += 1; return result; }

	guint32  ReadBE_U24 () { CHECKSIZE (3); guint32 result = (((guint32) ReadBE_U8 ()) << 16) + ReadBE_U16 (); return result; }
	
	guint8 PeekByte (gint32 offset)
	{
		if (offset + pos < 0 || offset + pos >= size) {
			printf ("MemoryBuffer::Peek: not enough data. Caller must check for available data first.\n");
			return 0;
		}

		return *(((guint8 *) memory) + pos + offset);
	}

	char *ReadLE_UTF16 (guint32 length_in_bytes)
	{
		char *result;
		CHECKSIZE (length_in_bytes);
		result = (char *) g_utf16_to_utf8 ((gunichar2 *) (((guint8 *) memory) + pos), length_in_bytes / 2, NULL, NULL, NULL);
		pos += length_in_bytes;
		return result;
	}

	/* reads a null-terminated string whose max size is length_in_bytes */
	char *ReadBE_UTF8 (guint32 length_in_bytes)
	{
		char *result;
		CHECKSIZE (length_in_bytes);
		result = g_strndup ((char *) (((guint8 *) memory) + pos), length_in_bytes);
		pos += length_in_bytes;
		return result;
	}

	guint8 *Read (guint32 length_in_bytes)
	{
		guint8 *result;

		CHECKSIZE (length_in_bytes);

		result = (guint8 *) g_malloc (length_in_bytes);
		memcpy (result, ((guint8 *) memory) + pos, length_in_bytes);

		pos += length_in_bytes;

		return result;
	}
};

class VideoStream : public IMediaStream {
private:
	IImageConverter *converter;
	guint32 bits_per_sample;
	guint64 initial_pts;
	guint32 height;
	guint32 width;
	guint32 bit_rate;

protected:
	virtual ~VideoStream ();

public:
	/* @SkipFactories */
	VideoStream (Media *media);
	virtual void Dispose ();

	/* @GeneratePInvoke */
	VideoStream (Media *media, int codec_id, guint32 width, guint32 height, guint64 duration, gpointer extra_data, guint32 extra_data_size);

	void SetBitsPerSample (guint32 value) { bits_per_sample = value; }
	guint32 GetBitsPerSample () { return bits_per_sample; }
	void SetInitialPts (guint64 value) { initial_pts = value; }
	guint64 GetInitialPts () { return initial_pts; }
	void SetWidth (guint32 value) { width = value; }
	/* @GenerateCBinding */
	guint32 GetWidth () { return width; }
	void SetHeight (guint32 value) { height = value; }
	/* @GenerateCBinding */
	guint32 GetHeight () { return height; }
	void SetBitRate (guint32 value) { bit_rate = value; }
	guint32 GetBitRate () { return bit_rate; }

	IImageConverter *GetImageConverter () { return converter; }
	void SetImageConverter (IImageConverter *value) { if (converter) converter->unref (); converter = value; if (converter) converter->ref ();}
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
	/* @SkipFactories */
	AudioStream (Media *media);
	
	/* @GeneratePInvoke */
	AudioStream (Media *media, int codec_id, int bits_per_sample, int block_align, int sample_rate, int channels, int bit_rate, gpointer extra_data, guint32 extra_data_size);
	
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

#if !HAVE_PTHREAD_RWLOCK_RDLOCK
#define pthread_rwlock_init pthread_mutex_init
#define pthread_rwlock_destroy pthread_mutex_destroy
#define pthread_rwlock_unlock pthread_mutex_unlock
#define pthread_rwlock_wrlock pthread_mutex_lock
#define pthread_rwlock_rdlock pthread_mutex_lock
#define pthread_rwlock_t pthread_mutex_t
#endif

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
	/* @SkipFactories */
	ExternalDemuxer (Media *media, void *instance, CloseDemuxerCallback close_demuxer, 
		GetDiagnosticAsyncCallback get_diagnostic, GetFrameAsyncCallback get_sample, OpenDemuxerAsyncCallback open_demuxer, 
		SeekAsyncCallback seek, SwitchMediaStreamAsyncCallback switch_media_stream);
	
	virtual void Dispose ();
		
	/* @GeneratePInvoke */
	void SetCanSeek (bool value);

	/* @GeneratePInvoke */
	void ClearCallbacks (); /* thread-safe */
		
	/* @GeneratePInvoke */
	gint32 AddStream (IMediaStream *stream);
	
	virtual bool GetCanSeek () { return can_seek; }
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
/* @CBindingRequisite */
typedef void (* ExternalDecoder_InputEndedCallback) (void *instance);

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
	ExternalDecoder_InputEndedCallback input_ended;
	
protected:
	virtual ~ExternalDecoder ();
	
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();
	
	virtual void InputEnded ();
public:
	/* @GenerateCBinding */
	/* @SkipFactories */
	ExternalDecoder (Media *media, IMediaStream *stream, void *instance, const char *name,
		ExternalDecoder_DecodeFrameAsyncCallback decode_frame_async,
		ExternalDecoder_OpenDecoderAsyncCallback open_decoder_async,
		ExternalDecoder_CleanupCallback cleanup,
		ExternalDecoder_CleanStateCallback clean_state,
		ExternalDecoder_HasDelayedFrameCallback has_delayed_frame,
		ExternalDecoder_DisposeCallback dispose,
		ExternalDecoder_DtorCallback dtor,
		ExternalDecoder_InputEndedCallback input_ended);
	
	virtual void Dispose ();
	
public:
	// If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	virtual void Cleanup (MediaFrame *frame);
	virtual void CleanState ();
	virtual bool HasDelayedFrame ();
	
	virtual const char *GetName () { return name; }
	/* @GenerateCBinding */
	void *GetInstance () { return instance; }
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
	PlaylistEntry *playlist;
	MemoryBuffer *buffer;

	static MediaResult DataCallback (MediaClosure *closure);
	
protected:
	virtual ~ASXDemuxer ();
	virtual MediaResult SeekInternal (guint64 pts) { return MEDIA_FAIL; }

	virtual void GetFrameAsyncInternal (IMediaStream *stream) { ReportErrorOccurred ("GetFrameAsync isn't supported for ASXDemuxer"); }
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 seekToTime) {}
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream) {}
	
public:
	/* @SkipFactories */
	ASXDemuxer (Media *media, IMediaSource *source, MemoryBuffer *buffer);
	virtual void Dispose ();
	
	virtual bool IsPlaylist () { return true; }
	virtual PlaylistEntry *GetPlaylist () { return playlist; }
	virtual bool GetCanSeek () { return false; }
};

class ASXDemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (MemoryBuffer *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
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
	
	virtual void DecodedFrameEnqueued ();
	MediaMarker *Pop ();
};

class PassThroughDecoder : public IMediaDecoder {
protected:
	virtual ~PassThroughDecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();

	virtual void InputEnded ();
public:
	PassThroughDecoder (Media *media, IMediaStream *stream);
	virtual void Dispose ();
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
	// Video dataclass MemoryBuffer 
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

};
#endif
