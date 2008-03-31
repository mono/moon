/*
 * pipeline.h: Pipeline for the media
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
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
class IImageConverter;
class MediaMarker;
class ProgressiveSource;

typedef int32_t MediaResult;

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

enum MediaSourceType {
	MediaSourceTypeFile = 1,
	MediaSourceTypeLive = 2,
	MediaSourceTypeProgressive = 3,
	MediaSourceTypeMemory = 4
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
	WorkTypeSeekToStart,
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
			uint64_t seek_pts;
		} seek;
		struct {
			uint16_t states;
			IMediaStream *stream;
		} frame;
		struct {
			IMediaSource *source;
		} open;
	} data;
	
	MediaWork (MediaClosure *closure, IMediaStream *stream, uint16_t states); // GetNextFrame
	MediaWork (MediaClosure *closure, uint64_t seek_pts); // Seek
	MediaWork (MediaClosure *closure); // SeekToStart
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

	//	Called on another thread, loops the queue of requested frames 
	//	and calls GetNextFrame and FrameReadCallback.
	//	If there are any requests for audio frames in the queue
	//	they are always (and all of them) satisfied before any video frame request.
	void WorkerLoop ();
	static void *WorkerLoop (void *data);
	void EnqueueWork (MediaWork *work);	
	void StopThread (); // Stops the worker thread.
	
protected:
	~Media ();

public:
	Media (MediaElement *element);
	
	// 1. Initialize the media with a file or url (this does not read any data, but it creates the source objects and prepares for downloading content, if necessary).
	// 2. Open it (this reads headers and initializes streams)
	
	//	If it's a file, just open it with FileStream.
	//	If it's a url:
	//	 mms://		try to open with LiveStream, fallback to ProgressiveStream.
	//	 http(s)://	try to open with ProgressiveStream, fallback to LiveStream
	//   file://	try to open with FileSource
	//	 others://	no idea (FIXME).
	MediaResult Initialize (const char *file_or_url); 
	
	//	Determines the container type and selects a demuxer
	//	- Default is to use our own ASF demuxer (if it's an ASF file), otherwise use ffmpeg (if available). Overridable by the environment variable MOONLIGHT_OVERRIDES, set demuxer=ffmpeg to force the ffmpeg demuxer.
	//	Makes the demuxer read the data header and fills in stream info, etc.
	//	Selects decoders according to stream info.
	//	- Default is Media *media to use MS decoder if available, otherwise ffmpeg. Overridable by MOONLIGHT_OVERRIDES, set codec=ffmpeg to force the ffmpeg decoder.
	MediaResult Open (); // Should only be called if Initialize has already been called (which will create the source)
	MediaResult Open (IMediaSource *source); // Called if you have your own source
	MediaResult OpenAsync (IMediaSource *source, MediaClosure *closure);
	
	// Seeks to the specified pts (if seekable).
	MediaResult Seek (uint64_t pts);
	MediaResult SeekAsync (uint64_t pts, MediaClosure *closure);
	MediaResult SeekToStart ();
	MediaResult SeekToStartAsync (MediaClosure *closure);
	
	//	Reads the next frame from the demuxer
	//	Requests the decoder to decode the frame
	//	Returns the decoded frame
	MediaResult GetNextFrame (MediaWork *work);
	
	//	Requests reading of the next frame
	void GetNextFrameAsync (MediaClosure *closure, IMediaStream *stream, uint16_t states); 
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
};
 
class MediaFrame {
public:
	MediaFrame (IMediaStream *stream);
	~MediaFrame ();
	
	void AddState (uint16_t state) { this->state |= state; } // There's no way of "going back" to an earlier state 
	bool IsDecoded () { return (state & FRAME_DECODED) == FRAME_DECODED; }
	bool IsDemuxed () { return (state & FRAME_DEMUXED) == FRAME_DEMUXED; }
	bool IsConverted () { return (state & FRAME_CONVERTED) == FRAME_CONVERTED; }
	bool IsPlanar () { return (state & FRAME_PLANAR) == FRAME_PLANAR; }
	bool IsCopyDecodedData () { return (state & FRAME_COPY_DECODED_DATA) == FRAME_COPY_DECODED_DATA; }
	bool IsKeyFrame () { return (state & FRAME_KEYFRAME) == FRAME_KEYFRAME; }
	
	IMediaStream *stream;
	void *decoder_specific_data; // data specific to the decoder
	uint64_t pts; // Set by the demuxer
	uint64_t duration; // Set by the demuxer
	
	uint16_t state; // Current state of the frame
	uint16_t event; // special frame event if non-0
	
	// The demuxer sets these to the encoded data which the
	// decoder then uses and replaces with the decoded data.
	uint8_t *buffer;
	uint32_t buflen;
	
	// planar data
	uint8_t *data_stride[4]; // Set by the decoder
	int srcSlideY; // Set by the decoder
	int srcSlideH; // Set by the decoder
	int srcStride[4]; // Set by the decoder
};

class MediaMarker {
	uint64_t pts; // 100-nanosecond units (pts)
	char *type;
	char *text;
	
public:
	class Node : public List::Node {
	public:
		Node (MediaMarker *m) : marker (m) {}
		virtual ~Node () { delete marker; }
		MediaMarker *marker;
	};
	
	MediaMarker (const char *type, const char *text, uint64_t pts);
	~MediaMarker ();
	
	const char *Type () { return type; }
	const char *Text () { return text; }
	uint64_t Pts () { return pts; }
};

// Interfaces

class IMediaObject : public EventObject {
protected:
	Media *media;
	virtual ~IMediaObject ();

public:
	IMediaObject (Media *media);
	
	Media *GetMedia () { return media; }
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

	void *extra_data;
	int extra_data_size;
	int codec_id;
	uint64_t duration; // 100-nanosecond units (pts)
	int32_t msec_per_frame;
	IMediaDecoder *decoder;
	const char *codec;
	// The minimum amount of padding any other part of the pipeline needs for frames from this stream.
	// Used by the demuxer when reading frames, ensures that there are at least min_padding extra bytes
	// at the end of the frame data (all initialized to 0).
	int min_padding;
	// 0-based index of the stream in the media
	// set by the demuxer, until then its value must be -1
	int index; 
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
	virtual MediaResult Seek (uint64_t pts) = 0;
	virtual MediaResult SeekToStart () = 0;
	int GetStreamCount () { return stream_count; }
	// Estimate a position for the pts.
	// This value is used when calculating the buffering progress percentage
	// (as well as determining when to stop buffering)
	// This method will be called on the main thread, so it can't
	// do anything that might block (such as read/seek ahead).
	// It's generally better to make an over-estimation than an under-estimation.
	virtual int64_t EstimatePtsPosition (uint64_t pts) = 0;
	IMediaStream *GetStream (int index);
	// Gets the longest duration from all the streams
	virtual uint64_t GetDuration (); // 100-nanosecond units (pts)
	virtual const char *GetName () = 0;
	virtual void UpdateSelected (IMediaStream *stream) {};
	virtual uint64_t GetLastAvailablePts () { return 0; }
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
	virtual MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t *dest[], int dstStride []) = 0;
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
	// method in IMediaSource which does the locking. No public in 
	// IMediaSource may be called from the xxxInternal methods.
	pthread_mutex_t mutex;
	pthread_cond_t condition;

	// This method must be called with the lock locked.
	void WaitForPosition (bool block, int64_t position);

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
	virtual int32_t ReadInternal (void *buf, uint32_t n) = 0;
	virtual int32_t PeekInternal (void *buf, uint32_t n, int64_t start) = 0;
	virtual bool SeekInternal (int64_t offset, int mode) = 0;
	virtual int64_t GetLastAvailablePositionInternal () { return -1; }
	virtual int64_t GetPositionInternal () = 0;
	virtual int64_t GetSizeInternal () = 0;

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
	int32_t ReadSome (void *buf, uint32_t n, bool block = true, int64_t start = -1);

	// Reads 'n' bytes into 'buf'. If data isn't available (as reported by
	// GetLastAvailablePosition) it will block if 'block' is true.
	// Returns false if 'n' bytes couldn't be read.
	// This method will lock the mutex.
	bool ReadAll (void *buf, uint32_t n, bool block = true, int64_t start = -1);

	// Reads 'n' bytes into 'buf', starting at position 'start'. If 'start' is -1,
	// then start at the current position. If data isn't available (as reported 
	// by GetLastAvailablePosition), it will block if 'block' is true, otherwise 
	// read the amount of data available. Returns false if 'n' bytes couldn't be
	// read.
	// This method will lock the mutex.
	bool Peek (void *buf, uint32_t n, bool block = true, int64_t start = -1);
	
	virtual bool CanSeek () { return true; }

	// Seeks to the specified 'offset', using the specified 'mode'. 
	// This method will lock the mutex.
	bool Seek (int64_t offset, int mode = SEEK_CUR);
	
	// Seeks to the specified 'pts'.
	virtual bool CanSeekToPts () { return false; }
	virtual bool SeekToPts (uint64_t pts) { return false; }

	// Returns the current reading position
	// This method will lock the mutex.
	int64_t GetPosition ();

	// Returns the size of the source. This method may return -1 if the
	// size isn't known.
	// This method will lock the mutex.
	int64_t GetSize ();

	virtual bool Eof () = 0;

	// Returns the last available position (confirms if that position 
	// can be read without blocking).
	// If the returned value is -1, then everything is available.
	// This method will lock the mutex.
	int64_t GetLastAvailablePosition ();

	// Aborts all current and future waits, no more waits will be done either.
	void Abort (); 

	// Are we waiting for something?
	bool IsWaiting (); 

	virtual const char *ToString () { return "IMediaSource"; }
};

// Implementations
 
class FileSource : public IMediaSource {
private:
	bool PeekInBuffer (void *buf, uint32_t n);

protected:
	char *filename;
	int64_t pos;
	int fd;
	bool eof;
	
	char buffer[4096];
	uint32_t buflen;
	char *bufptr;
	
	virtual ~FileSource ();

	virtual int32_t ReadInternal (void *buf, uint32_t n);
	virtual int32_t PeekInternal (void *buf, uint32_t n, int64_t start);
	virtual bool SeekInternal (int64_t offset, int mode);
	virtual int64_t GetPositionInternal ();
	virtual int64_t GetSizeInternal ();

public:
	FileSource (Media *media);
	FileSource (Media *media, const char *filename);
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeFile; }
	
	virtual bool Eof () { return eof; }

	virtual const char *ToString () { return filename; }

	const char *GetFileName () { return filename; }
};

class ProgressiveSource : public FileSource {
private:
	bool is_live;
	
	int64_t write_pos;
	int64_t wait_pos;
	int64_t size;
	int64_t first_write_pos;
	uint64_t requested_pts;
	uint64_t last_requested_pts;
	
	virtual int64_t GetLastAvailablePositionInternal () { return write_pos; }
	virtual int64_t GetSizeInternal () { return size; }

protected:
	virtual ~ProgressiveSource ();

public:
	ProgressiveSource (Media *media, bool is_live);
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeProgressive; }
	
	void SetTotalSize (int64_t size);
	
	void Write (void *buf, int64_t offset, int32_t n);
	void NotifySize (int64_t size);
	void RequestPosition (int64_t *pos);
	virtual bool CanSeekToPts () { return is_live; }
	virtual bool SeekToPts (uint64_t pts);

};

/*
class LiveSource : public IMediaSource {
protected:
	virtual ~LiveSource () {}

public:
	LiveSource (Media *media) : IMediaSource (media) {}
	
	virtual MediaResult Initialize () { return MEDIA_FAIL; }
	virtual MediaSourceType GetType () { return MediaSourceTypeLive; }
	
	virtual bool CanSeek () { return false; }
	virtual int64_t GetPosition () { return 0; }
	virtual bool Seek (int64_t offset) { return false; }
	virtual bool Seek (int64_t offset, int mode) { return false; }
	virtual int32_t ReadSome (void *buffer, uint32_t n) { return -1; }
	virtual bool ReadAll (void *buffer, uint32_t n) { return false; }
	virtual bool Peek (void *buffer, uint32_t n) { return false; }
	virtual bool Peek (void *buf, uint32_t n, int64_t start) { return false; }
	virtual int64_t GetSize () { return -1; }
	virtual bool Eof () { return false; }
};
*/

class MemorySource : public IMediaSource {
private:
	void *memory;
	int32_t size;
	int64_t start;
	int64_t pos;

protected:
	virtual ~MemorySource ();

	virtual int32_t ReadInternal (void *buf, uint32_t n);
	virtual int32_t PeekInternal (void *buf, uint32_t n, int64_t start);
	virtual bool SeekInternal (int64_t offset, int mode);
	virtual int64_t GetPositionInternal () { return pos + start; }
	virtual int64_t GetSizeInternal () { return size; }

public:
	MemorySource (Media *media, void *memory, int32_t size, int64_t start = 0);

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
	IImageConverter *converter; // This stream has the ownership of the converter, it will be deleted upon destruction.
	uint32_t bits_per_sample;
	uint32_t msec_per_frame;
	uint64_t initial_pts;
	uint32_t height;
	uint32_t width;
	
	VideoStream (Media *media);
	
	virtual MoonMediaType GetType () { return MediaTypeVideo; } 
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
	virtual MediaResult Seek (uint64_t pts) { return MEDIA_FAIL; }
	virtual MediaResult SeekToStart () { return MEDIA_FAIL; }
	virtual int64_t EstimatePtsPosition (uint64_t pts) { return 0; }

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
	int32_t *stream_to_asf_index;
	ASFReader *reader;
	ASFParser *parser;
	
	void ReadMarkers ();

protected:
	virtual ~ASFDemuxer ();

public:
	ASFDemuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (uint64_t pts);
	virtual MediaResult SeekToStart ();
	virtual int64_t EstimatePtsPosition (uint64_t pts);
	virtual void UpdateSelected (IMediaStream *stream);
	
	ASFParser *GetParser () { return parser; }
	virtual const char *GetName () { return "ASFDemuxer"; }
	virtual uint64_t GetLastAvailablePts ();
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
	
	void SetCallback (MediaClosure *closure);
};

class ASFMarkerDecoder : public IMediaDecoder {
protected:
	virtual ~ASFMarkerDecoder () {};

public:
	ASFMarkerDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult DecodeFrame (MediaFrame *frame) { return MEDIA_SUCCESS; }
	virtual MediaResult Open () {return MEDIA_SUCCESS; }
}; 

/*
 * Mp3 related implementations
 */

struct MpegFrame {
	int64_t offset;
	uint64_t pts;
	uint32_t dur;
	
	// this is needed in case this frame did not specify it's own
	// bit rate which is possible for MPEG-1 Layer 3 audio.
	int32_t bit_rate;
};

class Mp3FrameReader {
	IMediaSource *stream;
	int64_t stream_start;
	uint32_t frame_dur;
	uint32_t frame_len;
	uint64_t cur_pts;
	int32_t bit_rate;
	bool xing;
	
	MpegFrame *jmptab;
	uint32_t avail;
	uint32_t used;
	
	uint32_t MpegFrameSearch (uint64_t pts);
	void AddFrameIndex (int64_t offset, uint64_t pts, uint32_t dur, int32_t bit_rate);
	
	bool SkipFrame ();
	
public:
	Mp3FrameReader (IMediaSource *source, int64_t start, uint32_t frame_len, uint32_t frame_duration, bool xing);
	~Mp3FrameReader ();
	
	bool Seek (uint64_t pts);
	bool SeekToStart ();
	
	MediaResult ReadFrame (MediaFrame *frame);
	
	int64_t EstimatePtsPosition (uint64_t pts);
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
	virtual MediaResult Seek (uint64_t pts);
	virtual MediaResult SeekToStart ();
	virtual int64_t EstimatePtsPosition (uint64_t pts);
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
