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

typedef gint32 MediaResult;

#define MEDIA_SUCCESS ((MediaResult) 0)
#define MEDIA_FAIL ((MediaResult) 1)
#define MEDIA_INVALID_PROTOCOL ((MediaResult) 2)
#define MEDIA_INVALID_ARGUMENT ((MediaResult) 3)
#define MEDIA_INVALID_STREAM ((MediaResult) 4)
#define MEDIA_UNKNOWN_CODEC ((MediaResult) 5)
#define MEDIA_INVALID_MEDIA ((MediaResult) 6)
#define MEDIA_SEEK_ERROR ((MediaResult) 7)
#define MEDIA_FILE_ERROR ((MediaResult) 8)
#define MEDIA_CODEC_ERROR ((MediaResult) 9)
#define MEDIA_OUT_OF_MEMORY ((MediaResult) 10)
#define MEDIA_DEMUXER_ERROR ((MediaResult) 11)
#define MEDIA_CONVERTER_ERROR ((MediaResult) 12)
#define MEDIA_UNKNOWN_CONVERTER ((MediaResult) 13)
#define MEDIA_UNKNOWN_MEDIA_TYPE ((MediaResult) 14)
#define MEDIA_CODEC_DELAYED ((MediaResult) 15)
#define MEDIA_NO_MORE_DATA ((MediaResult) 16)
#define MEDIA_CORRUPTED_MEDIA ((MediaResult) 17)
#define MEDIA_NO_CALLBACK ((MediaResult) 18)
#define MEDIA_INVALID_DATA ((MediaResult) 19)
#define MEDIA_READ_ERROR ((MediaResult) 20)

#define MEDIA_SUCCEEDED(x) ((x <= 0))

#define FRAME_PLANAR (1 << 0)
#define FRAME_DECODED (1 << 1)
#define FRAME_DEMUXED (1 << 2)
#define FRAME_CONVERTED (1 << 3)
#define FRAME_KEYFRAME (1 << 4)
// Set if the pipeline needs it's own copy of the decoded data
// If this is not set, the decoder can keep one area of memory and always decode into
// that area, just passing back a pointer to that area.
// It is required to set this if the decoding is done on another thread
// (otherwise the pipeline will always access the latest decoded frame, which almost never
// is the frame you want to show).
#define FRAME_COPY_DECODED_DATA (1 << 5) 
#define FRAME_MARKER (1 << 6)

enum MediaSourceType {
	MediaSourceTypeFile = 1,
	MediaSourceTypeLive = 2,
	MediaSourceTypeProgressive = 3,
	MediaSourceTypeMemory = 4,
	MediaSourceTypeQueueMemory = 5
};

enum FrameEvent {
	FrameEventNone,
	FrameEventEOF
};

enum MoonPixelFormat {
	MoonPixelFormatNone = 0,
	MoonPixelFormatRGB32,
	MoonPixelFormatYUV420P
};

enum MoonMediaType {
	MediaTypeNone = 0,
	MediaTypeVideo,
	MediaTypeAudio,
	MediaTypeMarker
};

enum MoonWorkType {
	// The order is important, the most important work comes first (lowest number).
	// Seek is most important (as it will invalidate all other work), 
	// and will always be put first in the queue.
	// No more than one seek request should be in the queue at the same time either.
	WorkTypeSeek = 1, 
	// All audio work is done before any video work, since glitches in the audio is worse 
	// than glitches in the video.
	WorkTypeAudio, 
	WorkTypeVideo,
	WorkTypeMarker,
	// No other work should be in the queue until this has finished, so priority doesn't matter.
	WorkTypeOpen
};

typedef MediaResult MediaCallback (MediaClosure *closure);

#include "list.h"
#include "asf/asf.h"
#include "debug.h"
#include "dependencyobject.h"
#include "playlist.h"
#include "error.h"

class MediaClosure {
private:
	Media *media; // Set when this is the callback in Media::GetNextFrameAsync
	EventObject *context; // The property of whoever creates the closure.
	MediaCallback *callback; // The callback to call
	bool context_refcounted; // If we hold a ref to context.

public:
	MediaClosure (MediaCallback *callback);
	~MediaClosure ();
	
	// If this is a GetNextFrameAsync callback, and the result is positive,
	// this contains the resulting frame.
	MediaFrame *frame; 
	// Set when this is the callback in a MarkerStream
	MediaMarker *marker; 
	// The result of the work done in GetNextFrameAsync, OpenAsync or SeekAsync.
	MediaResult result;

	// Calls the callback and returns the callback's return value
	// If no callback is set, returns MEDIA_NO_CALLBACK
	MediaResult Call ();

	void SetMedia (Media *media);
	Media *GetMedia ();

	void SetContextUnsafe (EventObject *context); // Sets the context, but doesn't add a ref.
	void SetContext (EventObject *context);
	EventObject *GetContext ();

	MediaClosure *Clone ();
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
};

class DemuxerInfo : public MediaInfo  {
public:
	// <buffer> points to the first <length> bytes of a file. 
	// <length> is guaranteed to be at least 16 bytes.
	virtual bool Supports (IMediaSource *source) = 0; 
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source) = 0;
};

class ConverterInfo : public MediaInfo  {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output) = 0;
	virtual IImageConverter *Create (Media *media, VideoStream *stream) = 0;
};

class MediaWork : public List::Node {
public:
	MoonWorkType type;
	MediaClosure *closure;
	union {
		struct {
			guint64 seek_pts;
		} seek;
		struct {
			guint16 states;
			IMediaStream *stream;
		} frame;
		struct {
			IMediaSource *source;
		} open;
	} data;
	
	MediaWork (MediaClosure *closure, IMediaStream *stream, guint16 states); // GetNextFrame
	MediaWork (MediaClosure *closure, guint64 seek_pts); // Seek
	MediaWork (MediaClosure *closure, IMediaSource *source); // Open
	~MediaWork ();
};

class Media : public EventObject {
private:	
	static ConverterInfo *registered_converters;
	static DemuxerInfo *registered_demuxers;
	static DecoderInfo *registered_decoders;

	List *queued_requests;
	pthread_t queue_thread;
	pthread_cond_t queue_condition;
	pthread_mutex_t queue_mutex;
	
	char *file_or_url;
	IMediaSource *source;
	IMediaDemuxer *demuxer;
	List *markers;
	bool opened;
	bool stopping;
	bool stopped; // If the worker thread has been stopped.
	
	MediaElement *element;
	Downloader *downloader;

	//	Called on another thread, loops the queue of requested frames 
	//	and calls GetNextFrame and FrameReadCallback.
	//	If there are any requests for audio frames in the queue
	//	they are always (and all of them) satisfied before any video frame request.
	void WorkerLoop ();
	static void *WorkerLoop (void *data);
	void EnqueueWork (MediaWork *work);	
	void StopThread (); // Stops the worker thread.

	void Init (MediaElement *element, Downloader *dl);
	
protected:
	~Media ();

public:
	Media (MediaElement *element, Downloader *dl = NULL);
	
	//	Determines the container type and selects a demuxer
	//	- Default is to use our own ASF demuxer (if it's an ASF file), otherwise use ffmpeg (if available). Overridable by the environment variable MOONLIGHT_OVERRIDES, set demuxer=ffmpeg to force the ffmpeg demuxer.
	//	Makes the demuxer read the data header and fills in stream info, etc.
	//	Selects decoders according to stream info.
	//	- Default is Media *media to use MS decoder if available, otherwise ffmpeg. Overridable by MOONLIGHT_OVERRIDES, set codec=ffmpeg to force the ffmpeg decoder.
	MediaResult Open (); // Should only be called if Initialize has already been called (which will create the source)
	MediaResult Open (IMediaSource *source); // Called if you have your own source
	MediaResult OpenAsync (IMediaSource *source, MediaClosure *closure);
	
	// Seeks to the specified pts (if seekable).
	MediaResult Seek (guint64 pts);
	MediaResult SeekAsync (guint64 pts, MediaClosure *closure);
	
	//	Reads the next frame from the demuxer
	//	Requests the decoder to decode the frame
	//	Returns the decoded frame
	MediaResult GetNextFrame (MediaWork *work);
	
	//	Requests reading of the next frame
	void GetNextFrameAsync (MediaClosure *closure, IMediaStream *stream, guint16 states); 
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	
	IMediaSource *GetSource ();
	void SetSource (IMediaSource *value);
	IMediaDemuxer *GetDemuxer () { return demuxer; }
	const char *GetFileOrUrl () { return file_or_url; }
	void SetFileOrUrl (const char *value);
	MediaElement *GetElement () { return element; }
	
	void AddMessage (MediaResult result, const char *msg);
	void AddMessage (MediaResult result, char *msg);
	void AddError (MediaErrorEventArgs *args);

	// A list of MediaMarker::Node.
	// This is the list of markers found in the metadata/headers (not as a separate stream).
	// Will never return NULL.
	List *GetMarkers ();
	
	bool IsOpened () { return opened; }
	
	// Registration functions
	// This class takes ownership of the infos and will delete them (not free) when the Media is shutdown.
	static void RegisterDemuxer (DemuxerInfo *info);
	static void RegisterDecoder (DecoderInfo *info);
	static void RegisterConverter (ConverterInfo *info);
	
	static void Initialize ();
	static void Shutdown ();

	static Queue* media_objects;
	static int media_thread_count;
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "Media"; }
#endif
};
 
class MediaFrame {
public:
	MediaFrame (IMediaStream *stream);
	~MediaFrame ();
	
	void AddState (guint16 state) { this->state |= state; } // There's no way of "going back" to an earlier state 
	bool IsDecoded () { return (state & FRAME_DECODED) == FRAME_DECODED; }
	bool IsDemuxed () { return (state & FRAME_DEMUXED) == FRAME_DEMUXED; }
	bool IsConverted () { return (state & FRAME_CONVERTED) == FRAME_CONVERTED; }
	bool IsPlanar () { return (state & FRAME_PLANAR) == FRAME_PLANAR; }
	bool IsCopyDecodedData () { return (state & FRAME_COPY_DECODED_DATA) == FRAME_COPY_DECODED_DATA; }
	bool IsKeyFrame () { return (state & FRAME_KEYFRAME) == FRAME_KEYFRAME; }
	bool IsMarker () { return (state & FRAME_MARKER) == FRAME_MARKER; }
	
	IMediaStream *stream;
	void *decoder_specific_data; // data specific to the decoder
	guint64 pts; // Set by the demuxer
	guint64 duration; // Set by the demuxer
	
	guint16 state; // Current state of the frame
	guint16 event; // special frame event if non-0
	
	// The demuxer sets these to the encoded data which the
	// decoder then uses and replaces with the decoded data.
	// For markers this is a MarkerStream *
	guint8 *buffer;
	guint32 buflen;
	
	// planar data
	guint8 *data_stride[4]; // Set by the decoder
	int srcSlideY; // Set by the decoder
	int srcSlideH; // Set by the decoder
	int srcStride[4]; // Set by the decoder
};

class MediaMarker {
	guint64 pts; // 100-nanosecond units (pts)
	char *type;
	char *text;
	
public:
	class Node : public List::Node {
	public:
		Node (MediaMarker *m) : marker (m) {}
		virtual ~Node () { delete marker; }
		MediaMarker *marker;
	};
	
	MediaMarker (const char *type, const char *text, guint64 pts);
	~MediaMarker ();
	
	const char *Type () { return type; }
	const char *Text () { return text; }
	guint64 Pts () { return pts; }
};

// Interfaces

class IMediaObject : public EventObject {
protected:
	Media *media;
	virtual ~IMediaObject ();

public:
	IMediaObject (Media *media);
	
	Media *GetMedia () { return media; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "IMediaObject"; }
#endif
};


class IMediaStream : public IMediaObject {
private:
	void *context;
	bool enabled;
	bool selected;

protected:
	virtual ~IMediaStream ();

public:
	IMediaStream (Media *media);
	
	//	Video, Audio, Markers, etc.
	virtual MoonMediaType GetType () = 0; 
	IMediaDecoder *GetDecoder () { return decoder; }
	void SetDecoder (IMediaDecoder *dec) { decoder = dec; }
	
	//	If this stream is enabled (producing output). 
	//	A file might have several audio streams, 
	//	and live streams might have several video streams with different bitrates.
	bool IsEnabled () { return enabled; }
	const char *GetCodec () { return codec; }
	
	//	User defined context value.
	void *GetContext () { return context; }
	void  SetContext (void *context) { this->context = context; }
	
	bool GetSelected () { return selected; }
	void SetSelected (bool value);

	guint32 GetBitrate ();

	void *extra_data;
	int extra_data_size;
	int codec_id;
	guint64 duration; // 100-nanosecond units (pts)
	gint32 msec_per_frame;
	IMediaDecoder *decoder;
	const char *codec;
	// The minimum amount of padding any other part of the pipeline needs for frames from this stream.
	// Used by the demuxer when reading frames, ensures that there are at least min_padding extra bytes
	// at the end of the frame data (all initialized to 0).
	int min_padding;
	// 0-based index of the stream in the media
	// set by the demuxer, until then its value must be -1
	int index; 
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "IMediaStream"; }
#endif
};

class IMediaDemuxer : public IMediaObject {
private:
	IMediaStream **streams;
	int stream_count;
	
protected:
	IMediaSource *source;
	
	void SetStreams (IMediaStream **streams, int count);
	virtual ~IMediaDemuxer ();
	
public:
	IMediaDemuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader () = 0;
	// Fills the uncompressed_data field in the frame with data.
	virtual MediaResult ReadFrame (MediaFrame *frame) = 0;
	virtual MediaResult Seek (guint64 pts) = 0;
	int GetStreamCount () { return stream_count; }
	// Estimate a position for the pts.
	// This value is used when calculating the buffering progress percentage
	// (as well as determining when to stop buffering)
	// This method will be called on the main thread, so it can't
	// do anything that might block (such as read/seek ahead).
	// It's generally better to make an over-estimation than an under-estimation.
	virtual gint64 EstimatePtsPosition (guint64 pts) = 0;
	IMediaStream *GetStream (int index);
	// Gets the longest duration from all the streams
	virtual guint64 GetDuration (); // 100-nanosecond units (pts)
	virtual const char *GetName () = 0;
	virtual void UpdateSelected (IMediaStream *stream) {};
	virtual guint64 GetLastAvailablePts () { return 0; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return GetName (); }
#endif
};

class IMediaDecoder : public IMediaObject {
protected:
	virtual ~IMediaDecoder () {}

public:
	IMediaDecoder (Media *media, IMediaStream *stream);
	
	virtual MediaResult DecodeFrame (MediaFrame *frame) = 0;
	virtual MediaResult Open () = 0;
	
	// If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	virtual void Cleanup (MediaFrame *frame) {}
	virtual void CleanState () {}
	virtual bool HasDelayedFrame () { return false; }
	MoonPixelFormat pixel_format; // The pixel format this codec outputs. Open () should fill this in.
	IMediaStream *stream;
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "IMediaDecoder"; }
#endif
}; // Set when this is the callback in Media::GetNextFrameAsync


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
	
	IImageConverter (Media *media, VideoStream *stream);
	
	virtual MediaResult Open () = 0;
	virtual MediaResult Convert (guint8 *src[], int srcStride[], int srcSlideY, int srcSlideH, guint8 *dest[], int dstStride []) = 0;
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "IImageConverter"; }
#endif
};

/*
 * IMediaSource
 */
class IMediaSource : public IMediaObject {
private:
	// If all waits are aborted.
	bool aborted; 

	// Counter of how many threads are waiting in WaitForPosition.
	// This could probably be a bool, except that then we'd need
	// atomic sets and gets, which isn't available in old versions
	// of glib.
	int wait_count;

	// General locking behaviour:
	// All protected virtual methods must be called with the mutex
	// locked. If a derived virtual method needs to lock, it needs
	// to be implemented as a protected virtual method xxxInternal
	// which requires the mutex to be locked, and then a public 
	// method in IMediaSource which does the locking. No public method
	// in IMediaSource may be called from the xxxInternal methods.
	pthread_mutex_t mutex;
	pthread_cond_t condition;

	// This method must be called with the lock locked.
	void WaitForPosition (bool block, gint64 position);

protected:
	virtual ~IMediaSource ();

	void Lock ();
	void Unlock ();
	void Signal ();
	
	// To wait, call StartWaitLoop in the beginning, then in a loop call Wait, using Aborted as an exit condition of the loop, and EndWaitLoop after the loop.
	void StartWaitLoop ();
	// This method must be called with the lock locked.
	void Wait ();
	void EndWaitLoop ();
	bool Aborted () { return aborted; }

	// All these methods must/will be called with the lock locked.	
	virtual gint32 ReadInternal (void *buf, guint32 n) = 0;
	virtual gint32 PeekInternal (void *buf, guint32 n, gint64 start) = 0;
	virtual bool SeekInternal (gint64 offset, int mode) = 0;
	virtual gint64 GetLastAvailablePositionInternal () { return -1; }
	virtual gint64 GetPositionInternal () = 0;
	virtual gint64 GetSizeInternal () = 0;

public:
	IMediaSource (Media *media);
	
	// Initializes this stream (and if it succeeds, it can be read from later on).
	// streams based on remote content (live/progress) should contact the server
	// and try to start downloading content
	// file streams should try to open the file
	virtual MediaResult Initialize () = 0;
	virtual MediaSourceType GetType () = 0;
	
	// Reads 'n' bytes into 'buf'. If data isn't available (as reported by
	// GetLastAvailablePosition) it will block if 'block' is true, otherwise
	// read the amount of data available. Returns the number of bytes read.
	// This method will lock the mutex.
	gint32 ReadSome (void *buf, guint32 n, bool block = true, gint64 start = -1);

	// Reads 'n' bytes into 'buf'. If data isn't available (as reported by
	// GetLastAvailablePosition) it will block if 'block' is true.
	// Returns false if 'n' bytes couldn't be read.
	// This method will lock the mutex.
	bool ReadAll (void *buf, guint32 n, bool block = true, gint64 start = -1);

	// Reads 'n' bytes into 'buf', starting at position 'start'. If 'start' is -1,
	// then start at the current position. If data isn't available (as reported 
	// by GetLastAvailablePosition), it will block if 'block' is true, otherwise 
	// read the amount of data available. Returns false if 'n' bytes couldn't be
	// read.
	// This method will lock the mutex.
	bool Peek (void *buf, guint32 n, bool block = true, gint64 start = -1);
	
	virtual bool CanSeek () { return true; }

	// Seeks to the specified 'offset', using the specified 'mode'. 
	// This method will lock the mutex.
	bool Seek (gint64 offset, int mode = SEEK_CUR);
	
	// Seeks to the specified 'pts'.
	virtual bool CanSeekToPts () { return false; }
	virtual bool SeekToPts (guint64 pts) { return false; }

	virtual void NotifySize (gint64 size) { return; }
	virtual void NotifyFinished () { return; }

	// Returns the current reading position
	// This method will lock the mutex.
	gint64 GetPosition ();

	// Returns the size of the source. This method may return -1 if the
	// size isn't known.
	// This method will lock the mutex.
	gint64 GetSize ();

	virtual bool Eof () = 0;

	// Returns the last available position (confirms if that position 
	// can be read without blocking).
	// If the returned value is -1, then everything is available.
	// This method will lock the mutex.
	gint64 GetLastAvailablePosition ();

	// Aborts all current and future waits, no more waits will be done either.
	void Abort (); 

	// Are we waiting for something?
	bool IsWaiting (); 

	virtual const char *ToString () { return "IMediaSource"; }

	virtual void Write (void *buf, gint64 offset, gint32 n) { return; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "IMediaSource"; }
#endif
};

// Implementations
 
class FileSource : public IMediaSource {
private:
	bool PeekInBuffer (void *buf, guint32 n);

protected:
	char *filename;
	gint64 pos;
	int fd;
	bool eof;
	
	char buffer[4096];
	guint32 buflen;
	char *bufptr;
	
	virtual ~FileSource ();

	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n, gint64 start);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetPositionInternal ();
	virtual gint64 GetSizeInternal ();

public:
	FileSource (Media *media);
	FileSource (Media *media, const char *filename);
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeFile; }
	
	virtual bool Eof () { return eof; }

	virtual const char *ToString () { return filename; }

	const char *GetFileName () { return filename; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "FileSource"; }
#endif
};

class ProgressiveSource : public FileSource {
private:
	gint64 write_pos;
	gint64 wait_pos;
	gint64 size;
	
	virtual gint64 GetLastAvailablePositionInternal () { return write_pos; }
	virtual gint64 GetSizeInternal () { return size; }
	virtual bool SeekInternal (gint64 offset, int mode);

protected:
	virtual ~ProgressiveSource ();

public:
	ProgressiveSource (Media *media);
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeProgressive; }
	
	void SetTotalSize (gint64 size);
	
	virtual void Write (void *buf, gint64 offset, gint32 n);
	void NotifySize (gint64 size);
	virtual void NotifyFinished ();

#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "ProgressiveSource"; }
#endif
};

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
	virtual gint32 PeekInternal (void *buf, guint32 n, gint64 start);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetSizeInternal () { return size; }
	virtual gint64 GetPositionInternal () { return pos + start; }
	virtual gint64 GetLastAvailablePositionInternal () { return start + size - 1; }

public:
	MemorySource (Media *media, void *memory, gint32 size, gint64 start = 0);

	void *GetMemory () { return memory; }
	void Release (void) { delete this; }

	void SetOwner (bool value) { owner = value; }
	gint64 GetStart () { return start; }

	virtual MediaResult Initialize () { return MEDIA_SUCCESS; }
	virtual MediaSourceType GetType () { return MediaSourceTypeMemory; }
	
	virtual bool CanSeek () { return true; }
	virtual bool Eof () { return pos >= size; }

	virtual const char *ToString () { return "MemorySource"; }

#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "MemorySource"; }
#endif
};


// MemoryNestedSource is used to allow independent reading/seeking
// into an already created MemorySource. This is required when we 
// read data to calculate bufferingprogress (on main thread), while
// the same data might get read on the worker thread. Using the same 
// MemorySource would corrupt the current position.
class MemoryNestedSource : public MemorySource {
private:
	MemorySource *src;

protected:
	virtual ~MemoryNestedSource ();
	
public:
	MemoryNestedSource (MemorySource *src);
};

class MemoryQueueSource : public IMediaSource {
private:
	Queue queue;
	ASFParser *parser;
	bool finished;
	guint64 requested_pts;
	guint64 last_requested_pts;
	guint64 write_count;

protected:
	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n, gint64 start);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetSizeInternal ();
	virtual gint64 GetLastAvailablePositionInternal ();
	void WaitForQueue ();

public:
	class QueueNode : public List::Node {
	 public:
		ASFPacket *packet;
		MemorySource *source;
		QueueNode (ASFPacket *packet);
		QueueNode (MemorySource *source);
		virtual ~QueueNode ();
	};
	
	MemoryQueueSource (Media *media);
	virtual ~MemoryQueueSource ();
	void AddPacket (MemorySource *packet);
	ASFPacket *Pop ();
	bool Advance (); 

	virtual void NotifySize (gint64 size);
	virtual void NotifyFinished ();
	void RequestPosition (gint64 *pos);

	virtual MediaResult Initialize () { return MEDIA_SUCCESS; }
	virtual MediaSourceType GetType () { return MediaSourceTypeQueueMemory; }
	virtual gint64 GetPositionInternal ();
	virtual void Write (void *buf, gint64 offset, gint32 n);
	
	virtual bool CanSeek () { return true; }
	virtual bool Eof () { return finished && queue.IsEmpty (); }

	virtual const char *ToString () { return "MemoryQueueSource"; }
	virtual bool CanSeekToPts () { return true; }
	virtual bool SeekToPts (guint64 pts);
	
	bool IsFinished () { return finished; } // If the server sent the MMS_END packet.
	
	Queue *GetQueue ();
	void SetParser (ASFParser *parser);
	ASFParser *GetParser ();

#if DEBUG
	virtual const char* GetTypeName () { return "MemoryQueueSource"; }
#endif
};

class VideoStream : public IMediaStream {
protected:
	virtual ~VideoStream ();

public:
	IImageConverter *converter; // This stream has the ownership of the converter, it will be deleted upon destruction.
	guint32 bits_per_sample;
	guint32 msec_per_frame;
	guint64 initial_pts;
	guint32 height;
	guint32 width;
	guint32 bit_rate;
	
	VideoStream (Media *media);
	
	virtual MoonMediaType GetType () { return MediaTypeVideo; } 
	guint32 GetBitRate () { return (guint32) bit_rate; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "VideoStream"; }
#endif
};
 
class AudioStream : public IMediaStream {
protected:
	virtual ~AudioStream () {}

public:
	int bits_per_sample;
	int block_align;
	int sample_rate;
	int channels;
	int bit_rate;
	
	AudioStream (Media *media) : IMediaStream (media) {}
	
	virtual MoonMediaType GetType () { return MediaTypeAudio; }
	guint32 GetBitRate () { return (guint32) bit_rate; }
	
#if OBJECT_TRACKING
	virtual const char* GetTypeName () { return "AudioStream"; }
#endif
};

/*
 * ASX demuxer
 */ 
class ASXDemuxer : public IMediaDemuxer {
private:
	Playlist *playlist;

protected:
	virtual ~ASXDemuxer ();

public:
	ASXDemuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame) { return MEDIA_FAIL; }
	virtual MediaResult Seek (guint64 pts) { return MEDIA_FAIL; }
	virtual gint64 EstimatePtsPosition (guint64 pts) { return 0; }

	Playlist *GetPlaylist () { return playlist; }
	virtual const char *GetName () { return "ASXDemuxer"; }
};

class ASXDemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "ASXDemuxer"; }
};

/*
 * ASF related implementations
 */
class ASFDemuxer : public IMediaDemuxer {
private:
	gint32 *stream_to_asf_index;
	ASFReader *reader;
	ASFParser *parser;
	
	void ReadMarkers ();

protected:
	virtual ~ASFDemuxer ();

public:
	ASFDemuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (guint64 pts);
	virtual gint64 EstimatePtsPosition (guint64 pts);
	virtual void UpdateSelected (IMediaStream *stream);
	
	ASFParser *GetParser () { return parser; }
	void SetParser (ASFParser *parser);
	virtual const char *GetName () { return "ASFDemuxer"; }
	virtual guint64 GetLastAvailablePts ();
};

class ASFDemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "ASFDemuxer"; }
};

class MarkerStream : public IMediaStream {
private:
	MediaClosure *closure;

protected:
	virtual ~MarkerStream ();

public:
	MarkerStream (Media *media);
	
	virtual MoonMediaType GetType () { return MediaTypeMarker; }
	
	// The marker stream will never delete the closure
	void SetCallback (MediaClosure *closure);
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
};

class ASFMarkerDecoder : public IMediaDecoder {
protected:
	virtual ~ASFMarkerDecoder () {};

public:
	ASFMarkerDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult DecodeFrame (MediaFrame *frame);
	virtual MediaResult Open () { return MEDIA_SUCCESS; }
	virtual const char *GetName () { return "ASFMarkerDecoder"; }
}; 

class ASFMarkerDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char *codec) { return !strcmp (codec, "asf-marker"); };
	
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream)
	{
		return new ASFMarkerDecoder (media, stream);
	}	
	virtual const char *GetName () { return "ASFMarkerDecoder"; }
};

/*
 * Mp3 related implementations
 */

struct MpegFrame {
	gint64 offset;
	guint64 pts;
	guint32 dur;
	
	// this is needed in case this frame did not specify it's own
	// bit rate which is possible for MPEG-1 Layer 3 audio.
	gint32 bit_rate;
};

class Mp3FrameReader {
	IMediaSource *stream;
	gint64 stream_start;
	guint32 frame_dur;
	guint32 frame_len;
	guint64 cur_pts;
	gint32 bit_rate;
	bool xing;
	
	MpegFrame *jmptab;
	guint32 avail;
	guint32 used;
	
	guint32 MpegFrameSearch (guint64 pts);
	void AddFrameIndex (gint64 offset, guint64 pts, guint32 dur, gint32 bit_rate);
	
	bool SkipFrame ();
	
public:
	Mp3FrameReader (IMediaSource *source, gint64 start, guint32 frame_len, guint32 frame_duration, bool xing);
	~Mp3FrameReader ();
	
	bool Seek (guint64 pts);
	
	MediaResult ReadFrame (MediaFrame *frame);
	
	gint64 EstimatePtsPosition (guint64 pts);
};

class Mp3Demuxer : public IMediaDemuxer {
private:
	Mp3FrameReader *reader;
	bool xing;

protected:
	virtual ~Mp3Demuxer ();

public:
	Mp3Demuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (guint64 pts);
	virtual gint64 EstimatePtsPosition (guint64 pts);
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

class Mp3DemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

class NullMp3Decoder : public IMediaDecoder {
protected:
	virtual ~NullMp3Decoder () {};

public:
	NullMp3Decoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult DecodeFrame (MediaFrame *frame);
	
	virtual MediaResult Open ()
	{
		return MEDIA_SUCCESS;
	}
};

class NullMp3DecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char *codec) { return !strcmp (codec, "mp3"); };
	
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream)
	{
		return new NullMp3Decoder (media, stream);
	}
};



/*
 * MS related implementations
 */

class MSDecoder : public IMediaDecoder {
protected:
	virtual ~MSDecoder () {};

public:
	MSDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult Open ()
	{
		return MEDIA_FAIL;
	}
};

#endif
