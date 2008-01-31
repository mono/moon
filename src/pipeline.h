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

#include "debug.h"

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
class IImageConverter;
class MediaMarker;

typedef int32_t MediaResult;

#define MEDIA_NOCALLBACK ((MediaResult) 0)
#define MEDIA_NO_MORE_DATA ((MediaResult) -1)
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
	MediaSourceTypeProgressive = 3
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

class MediaClosure {
public:
	MediaClosure ();
	~MediaClosure ();
	
	MediaCallback *callback;
	MediaFrame *frame; // Set when this is the callback in Media::GetNextFrameAsync
	Media *media; // Set when this is the callback in Media::GetNextFrameAsync
	MediaMarker *marker; // Set when this is the callback in MarkerStream
	void *context; // The property of whoever creates the closure.
	// Set when this is a seek callback, contains the result of the seek
	// also set for OpenAsync.
	MediaResult result;
	// Calls the callback and returns the callback's return value
	// If no callback is set, returns MEDIA_NO_CALLBACK
	MediaResult Call ();
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
	union {
		struct {
			uint64_t seek_pts;
			MediaClosure *closure;
		} seek;
		struct {
			MediaFrame *frame;
			uint16_t states;
		} frame;
		struct {
			IMediaSource *source;
			MediaClosure *closure;
		} open;
	} data;
	
	MediaWork (MoonWorkType tp) 
	{
		type = tp;
		memset (&data, 0, sizeof (data));
	}
	virtual ~MediaWork () {}
};

class Media {
private:	
	static ConverterInfo *registered_converters;
	static DemuxerInfo *registered_demuxers;
	static DecoderInfo *registered_decoders;

	List *queued_requests;
	pthread_t queue_thread;
	pthread_cond_t queue_condition;
	pthread_mutex_t queue_mutex;
	MediaClosure *queue_closure;
	
	char *file_or_url;
	IMediaSource *source;
	IMediaDemuxer *demuxer;
	List *markers;
	bool opened;
	
	//	Called on another thread, loops the queue of requested frames 
	//	and calls GetNextFrame and FrameReadCallback.
	//	If there are any requests for audio frames in the queue
	//	they are always (and all of them) satisfied before any video frame request.
	void WorkerLoop ();
	static void *WorkerLoop (void *data);
	void EnqueueWork (MediaWork *work);	
	
public:
	Media ();
	~Media ();
	
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
	
	//	Reads the next frame from the demuxer
	//	Requests the decoder to decode the frame
	//	Returns the decoded frame
	MediaResult GetNextFrame (MediaFrame *frame);
	MediaResult GetNextFrame (MediaFrame *frame, uint16_t states); 
	
	//	Requests reading of the next frame
	void GetNextFrameAsync (MediaFrame *frame); 
	void GetNextFrameAsync (MediaFrame *frame, uint16_t states); 
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	void SetQueueCallback (MediaClosure *closure) { queue_closure = closure; }
	
	IMediaSource *GetSource () { return source; }
	void SetSource (IMediaSource *value) { source = value; }
	IMediaDemuxer *GetDemuxer () { return demuxer; }
	const char *GetFileOrUrl () { return file_or_url; }
	void SetFileOrUrl (const char *value);
	
	void AddMessage (MediaResult result, const char *msg);
	void AddMessage (MediaResult result, char *msg);
	
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

class IMediaObject {
protected:
	Media *media;
	
public:
	IMediaObject (Media *media);
	virtual ~IMediaObject ();
	
	Media *GetMedia () { return media; }
};


class IMediaStream : public IMediaObject {
	void *context;
	bool enabled;
	
public:
	IMediaStream (Media *media);
	virtual ~IMediaStream ();
	
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
	IMediaStream **streams;
	int stream_count;
	
protected:
	IMediaSource *source;
	
	void SetStreams (IMediaStream **streams, int count);
	
public:
	IMediaDemuxer (Media *media, IMediaSource *source);
	virtual ~IMediaDemuxer ();
	
	virtual MediaResult ReadHeader () = 0;
	// Fills the uncompressed_data field in the frame with data.
	virtual MediaResult ReadFrame (MediaFrame *frame) = 0;
	virtual MediaResult Seek (uint64_t pts) = 0;
	int GetStreamCount () { return stream_count; }
	// Calculates a position for the pts
	// Might return an estimated value, 
	// if so 'estimate' will be true upon return.
	// Must return the size of the file if pts is after the
	// end of the media.
	// The position returned is the last position in the file 
	// that is required to play/render all the frames/streams 
	// with the specified pts, guaranteeing that the demuxer
	// won't seek to/read from a position after the returned
	// position when getting frames of the specified pts.
	// (if it's an estimate then the guarantee cannot be made of course)
	virtual int64_t GetPositionOfPts (uint64_t pts, bool *estimate) = 0;
	IMediaStream *GetStream (int index);
	// Gets the longest duration from all the streams
	virtual uint64_t GetDuration (); // 100-nanosecond units (pts)
};

class IMediaDecoder : public IMediaObject {
public:
	IMediaDecoder (Media *media, IMediaStream *stream);
	virtual ~IMediaDecoder () {}
	
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
public:
	MoonPixelFormat output_format;
	MoonPixelFormat input_format;
	VideoStream *stream;
	
	IImageConverter (Media *media, VideoStream *stream);
	virtual ~IImageConverter () {}
	
	virtual MediaResult Open () = 0;
	virtual MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t *dest[], int dstStride []) = 0;
};

// read data, with the possibility of returning a 'wait a bit, need more data first' error value. 
// Another way is to always do the read/demux/decode stuff on another thread, 
// in which case we can block here
class IMediaSource : public IMediaObject {
public:
	IMediaSource (Media *media) : IMediaObject (media) {}
	virtual ~IMediaSource () {}
	
	// Initializes this stream (and if it succeeds, it can be read from later on).
	// streams based on remote content (live/progress) should contact the server
	// and try to start downloading content
	// file streams should try to open the file
	virtual MediaResult Initialize () = 0;
	virtual MediaSourceType GetType () = 0;
	
	virtual bool IsSeekable () = 0;
	virtual int64_t GetPosition () = 0;
	virtual void SetPosition (int64_t position) { Seek (position, SEEK_SET); }
	virtual bool Seek (int64_t offset) = 0; // Seeks to the offset from the current position
	virtual bool Seek (int64_t offset, int mode) = 0;
	virtual int32_t Read (void *buf, uint32_t n) = 0;
	virtual bool ReadAll (void *buf, uint32_t n) = 0;
	virtual bool Peek (void *buf, uint32_t n) = 0;
	virtual int64_t GetSize () = 0;
	virtual bool Eof () = 0;
};

// Implementations
 
class FileSource : public IMediaSource {
protected:
	char *filename;
	int64_t pos;
	int fd;
	
	char buffer[4096];
	uint32_t buflen;
	char *bufptr;
	
	bool eof;
	
public:
	FileSource (Media *media);
	FileSource (Media *media, const char *filename);
	~FileSource ();
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeFile; }
	
	const char *GetFileName () { return filename; }
	
	virtual bool IsSeekable ();
	virtual int64_t GetPosition ();
	virtual bool Seek (int64_t position);
	virtual bool Seek (int64_t position, int mode);
	virtual int32_t Read (void *buf, uint32_t n);
	virtual bool ReadAll (void *buf, uint32_t n);
	virtual bool Peek (void *buf, uint32_t n);
	virtual int64_t GetSize ();
	virtual bool Eof ();
};

class ProgressiveSource : public FileSource {
	pthread_mutex_t write_mutex;
	pthread_cond_t write_cond;
	bool cancel_wait;
	int wait_count; // Counter of how many threads are waiting in WaitForPosition
	
	int64_t write_pos;
	int64_t wait_pos;
	int64_t size;
	
	static void write (void *buf, int32_t offset, int32_t n, gpointer cb_data);
	static void notify_size (int64_t size, gpointer cb_data);
	
public:
	ProgressiveSource (Media *media);
	virtual ~ProgressiveSource ();
	
	virtual MediaResult Initialize (); 
	virtual MediaSourceType GetType () { return MediaSourceTypeProgressive; }
	
	// The size of the currently available data
	void SetCurrentSize (int64_t size);
	
	// The total size of the file (might not be available)
	void SetTotalSize (int64_t size);
	
	// Blocks until the position have data
	// Returns false if failure (one possibility being that the requested position is beyond the end of the file)
	bool WaitForPosition (int64_t position);
	// Wakes up WaitForPosition to check if the position has been reached, or if the wait should be cancelled
	bool WakeUp ();
	bool WakeUp (bool lock); // lock: set to false if the write_lock is already acquired by the caller
	bool IsWaiting ();
	// Cancels any pending waits
	void CancelWait (); 
	int64_t GetWritePosition ();
	int64_t GetWaitPosition ();
	void Lock ();
	void Unlock ();
	
	virtual bool Seek (int64_t offset, int mode);
	virtual bool ReadAll (void *buf, uint32_t size);
	virtual bool Peek (void *buf, uint32_t size);
	virtual int64_t GetSize () { return size; }
	
	void Write (void *buf, int64_t offset, int32_t n);
	void NotifySize (int64_t size);
};

class LiveSource : public IMediaSource {
public:
	LiveSource (Media *media) : IMediaSource (media) {}
	
	virtual MediaResult Initialize () { return MEDIA_FAIL; }
	virtual MediaSourceType GetType () { return MediaSourceTypeLive; }
	
	virtual bool IsSeekable () { return false; }
	virtual int64_t GetPosition () { return 0; }
	virtual bool Seek (int64_t position) { return false; }
	virtual bool Seek (int64_t position, int mode) { return false; }
	virtual int32_t Read (void *buffer, uint32_t n) { return -1; }
	virtual bool ReadAll (void *buffer, uint32_t n) { return false; }
	virtual bool Peek (void *buffer, uint32_t n) { return false; }
	virtual int64_t GetSize () { return -1; }
	virtual bool Eof () { return false; }
};

class VideoStream : public IMediaStream {
public:
	IImageConverter *converter; // This stream has the ownership of the converter, it will be deleted upon destruction.
	uint32_t bits_per_sample;
	uint32_t msec_per_frame;
	uint64_t initial_pts;
	uint32_t height;
	uint32_t width;
	
	VideoStream (Media *media);
	virtual ~VideoStream ();
	
	virtual MoonMediaType GetType () { return MediaTypeVideo; } 
};
 
class AudioStream : public IMediaStream {
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
 * ASF related implementations
 */
class ASFDemuxer : public IMediaDemuxer {
	int32_t *stream_to_asf_index;
	ASFFrameReader **readers;
	ASFParser *parser;
	
	void ReadMarkers ();
	
public:
	ASFDemuxer (Media *media, IMediaSource *source);
	~ASFDemuxer ();
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (uint64_t pts);
	virtual int64_t GetPositionOfPts (uint64_t pts, bool *estimate);
	
	ASFParser *GetParser () { return parser; }
};

class ASFDemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "ASFDemuxer"; }
};

class MarkerStream : public IMediaStream {
	MediaClosure *closure;
	
public:
	MarkerStream (Media *media);
	~MarkerStream ();
	
	virtual MoonMediaType GetType () { return MediaTypeMarker; }
	
	void SetCallback (MediaClosure *closure);
};

class ASFMarkerDecoder : public IMediaDecoder {
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
	uint64_t cur_pts;
	int32_t bit_rate;
	
	MpegFrame *jmptab;
	uint32_t avail;
	uint32_t used;
	
	uint32_t MpegFrameSearch (uint64_t pts);
	void AddFrameIndex (int64_t offset, uint64_t pts, uint32_t dur, int32_t bit_rate);
	
	bool SkipFrame ();
	
public:
	Mp3FrameReader (IMediaSource *source, int64_t start);
	~Mp3FrameReader ();
	
	bool Seek (uint64_t pts);
	
	MediaResult ReadFrame (MediaFrame *frame);
};

class Mp3Demuxer : public IMediaDemuxer {
	Mp3FrameReader *reader;
	
public:
	Mp3Demuxer (Media *media, IMediaSource *source);
	~Mp3Demuxer ();
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (uint64_t pts);
	virtual int64_t GetPositionOfPts (uint64_t pts, bool *estimate) { return -1; }
};

class Mp3DemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

class NullMp3Decoder : public IMediaDecoder {
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
public:
	MSDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult Open ()
	{
		return MEDIA_FAIL;
	}
};

#endif
