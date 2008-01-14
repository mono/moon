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

#define MOON_MEDIA

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

#include "list.h"
#include "asf/asf.h"

typedef gint32 MediaResult;

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

#define MEDIA_SUCCEEDED(x) ((x <= 0))

#define FRAME_PLANAR (1)
#define FRAME_DECODED (2)
#define FRAME_DEMUXED (4)
#define FRAME_CONVERTED (8)
// Set if the pipeline needs it's own copy of the decoded data
// If this is not set, the decoder can keep one area of memory and always decode into
// that area, just passing back a pointer to that area.
// It is required to set this if the decoding is done on another thread
// (otherwise the pipeline will always access the latest decoded frame, which almost never
// is the frame you want to show).
#define FRAME_COPY_DECODED_DATA (16) 

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

typedef MediaResult MediaCallback (MediaClosure* closure);

class MediaClosure {
public:
	MediaClosure ();
	~MediaClosure ();
	
	MediaCallback* callback;
	MediaFrame* frame;
	Media* media;
	void* context;
	
	// Calls the callback and returns the callback's return value
	// If no callback is set, returns MEDIA_NO_CALLBACK
	MediaResult Call ();
}; 

/*
 * *Info classes used to register codecs, demuxers and converters.
 */

class MediaInfo {
public:
	MediaInfo* next; // Used internally by Media.
	MediaInfo () : next (NULL) {}
	virtual ~MediaInfo () {}
	virtual const char* GetName () { return "Unknown"; }
};

class DecoderInfo : public MediaInfo {
public:
	virtual bool Supports (const char* codec) = 0;
	virtual IMediaDecoder* Create (Media* media, IMediaStream* stream) = 0;
};

class DemuxerInfo : public MediaInfo  {
public:
	// <buffer> points to the first <length> bytes of a file. 
	// <length> is guaranteed to be at least 16 bytes.
	virtual bool Supports (uint8_t* buffer, uint32_t length) = 0; 
	virtual IMediaDemuxer* Create (Media* media) = 0;
};

class ConverterInfo : public MediaInfo  {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output) = 0;
	virtual IImageConverter* Create (Media* media, VideoStream* stream) = 0;
};

class Media {
public:
	Media (void* element);
	~Media ();
	
	//	Opens the file using a FileSource
	MediaResult OpenFile (char* file);
	
	//	Opens the file using a LiveSource
	MediaResult OpenURL (char* url);
	
	//	If it's a file, just open it with FileStream.
	//	If it's a url:
	//	 mms://		try to open with LiveStream, fallback to ProgressiveStream.
	//	 http(s)://	try to open with ProgressiveStream, fallback to LiveStream
	//   file://	try to open with FileSource
	//	 others://	no idea (FIXME).
	MediaResult Open (const char* file_or_url); 
	
	//	Opens the file
	//	Determines the container type and selects a demuxer
	//	- Default is to use our own ASF demuxer (if it's an ASF file), otherwise use ffmpeg (if available). Overridable by the environment variable MOONLIGHT_OVERRIDES, set demuxer=ffmpeg to force the ffmpeg demuxer.
	//	Makes the demuxer read the data header and fills in stream info, etc.
	//	Selects decoders according to stream info.
	//	- Default is Media* mediato use MS decoder if available, otherwise ffmpeg. Overridable by MOONLIGHT_OVERRIDES, set codec=ffmpeg to force the ffmpeg decoder.
	MediaResult Open (IMediaSource* source);
	
	// Seeks to the specified pts (if seekable).
	MediaResult Seek (gint64 pts);
	
	//	Reads the next frame from the demuxer
	//	Requests the decoder to decode the frame
	//	Returns the decoded frame
	MediaResult GetNextFrame (MediaFrame* frame);
	MediaResult GetNextFrame (MediaFrame* frame, int states); 
	
	//	Requests reading of the next frame
	void GetNextFrameAsync (MediaFrame* frame); 
	void GetNextFrameAsync (MediaFrame* frame, int states); 
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	void DeleteQueue (); // Deletes the queue and finishes the thread that's processing the queue.
	void SetQueueCallback (MediaClosure* closure) { queue_closure = closure; }
	
	IMediaSource* GetSource () { return source; }
	IMediaDemuxer* GetDemuxer () { return demuxer; }
	void* GetElement () { return element; }
	const char* GetFileOrUrl () { return file_or_url; }
    
    void AddMessage (MediaResult result, const char* msg);
    void AddMessage (MediaResult result, char* msg);
    
    
public:
    // Registration functions
    // This class takes ownership of the infos and will delete them (not free) when the Media is shutdown.
    static void RegisterDemuxer (DemuxerInfo *info);
    static void RegisterDecoder (DecoderInfo *info);
    static void RegisterConverter (ConverterInfo *info);

	static void Initialize ();
	static void Shutdown ();
private:
	static DemuxerInfo* registered_demuxers;
	static DecoderInfo* registered_decoders;
	static ConverterInfo* registered_converters;
    
private:
	//	Called on another thread, loops the queue of requested frames 
	//	and calls GetNextFrame and FrameReadCallback.
	//	If there are any requests for audio frames in the queue
	//	they are always (and all of them) satisfied before any video frame request.
	void FrameReaderLoop ();
	static void* FrameReaderLoop (void* data);

	IMediaSource* source;
	IMediaDemuxer* demuxer;
	void* element;
	char* file_or_url;
	
	List* queued_requests;
	pthread_t queue_thread;
	pthread_cond_t queue_condition;
	pthread_mutex_t queue_mutex;
	MediaClosure* queue_closure;
	
	class Node : public List::Node {
	public:
		MediaFrame* frame;
		int states;
	};
};
 
class MediaFrame {
public:
	~MediaFrame ();
	MediaFrame (IMediaStream* stream);
	
	void AddState (gint32 state) { this->state |= state; } // There's no way of "going back" to an earlier state 
	bool IsDecoded () { return (state & FRAME_DECODED) == FRAME_DECODED; }
	bool IsDemuxed () { return (state & FRAME_DEMUXED) == FRAME_DEMUXED; }
	bool IsConverted () { return (state & FRAME_CONVERTED) == FRAME_CONVERTED; }
	bool IsPlanar () { return (state & FRAME_PLANAR) == FRAME_PLANAR; }
	bool IsCopyDecodedData () { return (state & FRAME_COPY_DECODED_DATA) == FRAME_COPY_DECODED_DATA; }
	
	IMediaStream* stream;
	void* decoder_specific_data; // data specific to the decoder
	guint64 pts; // Set by the demuxer
	guint64 duration; // Set by the demuxer
	
	gint32 state; // Current state of the frame
	
	guint32 compressed_size; // Set by the demuxer
	void* compressed_data; // Set by the demuxer
	
	// non-planar data
	guint32 uncompressed_size; // Set by the decoder
	void* uncompressed_data; // Set by the decoder

	// planar data
	guint8 *uncompressed_data_stride[4]; // Set by the decoder
	int srcSlideY; // Set by the decoder
	int srcSlideH; // Set by the decoder
	int srcStride [4]; // Set by the decoder
};

// Interfaces

class IMediaObject {
public:
	IMediaObject (Media* med);
	virtual ~IMediaObject ();
	
	//	Sets the callback to call when the frame is read
	void SetFrameReadCallback (MediaClosure* callback);
	
	Media* GetMedia () { return media; }

protected:
	Media* media;
	
private:
	MediaClosure* callback;
};


class IMediaStream  {
public:
	IMediaStream (Media* media);
	virtual ~IMediaStream ();

	//	Video, Audio, Markers, etc.
	virtual MoonMediaType GetType () = 0; 
	IMediaDecoder* GetDecoder () { return decoder; }
	void SetDecoder (IMediaDecoder* dec) { decoder = dec; }
	//	If this stream is enabled (producing output). 
	//	A file might have several audio streams, 
	//	and live streams might have several video streams with different bitrates.
	bool IsEnabled () { return enabled; }
	const char* GetCodec () { return codec; }
	
	//	User defined context value.
	void* GetContext () { return context; }
	void  SetContext (void* context) { this->context = context; }
	
	void* extra_data;
	int extra_data_size;
	int codec_id;
	guint64 start_time;
	gint32 msec_per_frame;
	guint64 duration;
	IMediaDecoder* decoder;
	const char* codec;
	// The minimum amount of padding any other part of the pipeline needs for frames from this stream.
	// Used by the demuxer when reading frames, ensures that there are at least min_padding extra bytes
	// at the end of the frame data (all initialized to 0).
	int min_padding;
	// 0-based index of the stream in the media
	// set by the demuxer, until then its value must be -1
	int index; 
	
private:
	bool enabled;
	void* context;
};


// read data, with the possibility of returning a 'wait a bit, need more data first' error value. 
// Another way is to always do the read/demux/decode stuff on another thread, 
// in which case we can block here
class IMediaSource {
public:
	IMediaSource (Media* med) : media (med) {}
	virtual ~IMediaSource () {}
	virtual bool IsSeekable () = 0;
	virtual bool Seek (gint64 offset) = 0; // Seeks to the offset from the current position
	virtual bool Seek (gint64 offset, int mode) = 0;
	virtual bool Read (void* buffer, guint32 size) = 0;
	virtual bool Peek (void* buffer, guint32 size) = 0;
	virtual guint64 GetPosition () = 0;
	virtual void SetPosition (guint64 position) { Seek (position, SEEK_SET); }
	virtual bool Eof () = 0;
	
protected:
	Media* media;
};

class IMediaDemuxer : public IMediaObject {
public:
	IMediaDemuxer (Media* media) : IMediaObject (media) {}
	virtual ~IMediaDemuxer ();
	virtual MediaResult ReadHeader () = 0;
	// Fills the uncompressed_data field in the frame with data.
	virtual MediaResult ReadFrame (MediaFrame* frame) = 0;
	virtual MediaResult Seek (guint64 pts) = 0;
	int GetStreamCount () { return stream_count; }
	IMediaStream* GetStream (int index)
	{
		return (index < 0 || index >= stream_count) ? NULL : streams [index];
	}
	
protected:
	void SetStreams (IMediaStream** streams, int count);
	
private:
	int stream_count;
	IMediaStream** streams;
};

class IMediaDecoder {
public:
	IMediaDecoder (Media* media, IMediaStream* stream);
	virtual ~IMediaDecoder () {}
	
	virtual MediaResult DecodeFrame (MediaFrame* frame) = 0;
	virtual MediaResult Open () = 0;
	virtual void Cleanup (MediaFrame* frame) {} // If MediaFrame->decoder_specific_data is non-NULL, this method is called in ~MediaFrame.
	
	MoonPixelFormat pixel_format; // The pixel format this codec outputs. Open () should fill this in.
	IMediaStream* stream;
	Media* media;
};


/*
 * Inherit from this class to provide image converters (yuv->rgb for instance) 
 */
class IImageConverter {
public:
	IImageConverter (Media* med, VideoStream* str);
	virtual ~IImageConverter () {}
	
	virtual MediaResult Open () = 0;
	virtual MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride []) = 0;
	
	Media* media;
	VideoStream* stream;
	MoonPixelFormat input_format;
	MoonPixelFormat output_format;
};

// Implementations
 
class FileSource : public IMediaSource {
public:
	FileSource (Media* media);
	~FileSource ();
	
	virtual bool IsSeekable ();
	virtual bool Seek (gint64 position);
	virtual bool Seek (gint64 position, int mode);
	virtual bool Read (void* buffer, guint32 size);
	virtual bool Peek (void* buffer, guint32 size);
	virtual guint64 GetPosition ();
	virtual bool Eof ();
	
private:
	char* filename;
	FILE* fd;
};

class ProgressiveSource : public FileSource {
public:
	ProgressiveSource (Media* media) : FileSource (media) {}
	virtual ~ProgressiveSource () {}
		
	//	The size of the currently available data
	void SetCurrentSize (long size);
	//	The total size of the file (might not be available)
	void SetTotalSize (long size);

private:
};

class LiveSource : public IMediaSource {
public:
	LiveSource (Media* media) : IMediaSource (media) {}
	
	virtual bool IsSeekable () { return false; }
	virtual bool Seek (gint64 position) { return false; }
	virtual bool Seek (gint64 position, int mode) { return false; }
	virtual bool Read (void* buffer, guint32 size) { return false; }
	virtual bool Peek (void* buffer, guint32 size) { return false; }
	virtual guint64 GetPosition () { return 0; }
	virtual bool Eof () { return false; }
	
};

class VideoStream : public IMediaStream {
public:
	VideoStream (Media* media);
	virtual ~VideoStream ();
	    
	virtual MoonMediaType GetType () { return MediaTypeVideo; } 
    
    guint32 width;
    guint32 height;
    guint32 msec_per_frame;
    guint64 initial_pts;
    guint32 bits_per_sample;
    IImageConverter* converter; // This stream has the ownership of the converter, it will be deleted upon destruction.
};
 
class AudioStream : public IMediaStream {
public:
	AudioStream (Media* media) : IMediaStream (media) {}
	
	virtual MoonMediaType GetType () { return MediaTypeAudio; }
	
	int channels;
	int sample_rate;
	int bit_rate;
	int block_align;
	int bits_per_sample;
};

/*
 * ASF related implementations
 */
class ASFDemuxer : public IMediaDemuxer {
public:
	ASFDemuxer (Media* media);
	~ASFDemuxer ();
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame* frame);
	virtual MediaResult Seek (guint64 pts);
	
private:
	ASFParser* parser;
	ASFFrameReader* reader;
	gint32* stream_to_asf_index;
};

class ASFDemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (uint8_t* buffer, uint32_t length); 
	virtual IMediaDemuxer* Create (Media* media); 
	virtual const char* GetName () { return "ASFDemuxer"; }
};

class ASFMarkerStream : public IMediaStream {
public:
	ASFMarkerStream (Media* media) : IMediaStream (media) {}
	virtual MoonMediaType GetType () { return MediaTypeMarker; } 
};

class ASFMarkerDecoder : public IMediaDecoder {
public:
	ASFMarkerDecoder (Media* media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	virtual MediaResult DecodeFrame (MediaFrame* frame) { return MEDIA_SUCCESS; }
	virtual MediaResult Open () {return MEDIA_SUCCESS; }
}; 

/*
 * Mp3 related implementations
 */
 
class Mp3Decoder : public IMediaDecoder {
public:
	Mp3Decoder (Media* media, IMediaStream* stream);
	virtual ~Mp3Decoder ();
	
	virtual MediaResult DecodeFrame (MediaFrame* frame);
	virtual MediaResult Open ();
};

/*
 * MS related implementations
 */

class MSDecoder : public IMediaDecoder {
public:
	MSDecoder (Media* media, IMediaStream* stream) : IMediaDecoder (media, stream) {}
	virtual MediaResult Open ()
	{
		return MEDIA_FAIL;
	}
};

#endif
