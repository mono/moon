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

//#define MOON_MEDIA

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

#define MEDIA_SUCCEEDED(x) ((x == 0))

#define MEDIA_VIDEO 1
#define MEDIA_AUDIO 2
#define MEDIA_MARKER 3

typedef MediaResult MediaCallback (MediaClosure* closure);

enum PIXEL_FORMAT {
	PIXEL_FORMAT_NONE = 0,
	PIXEL_FORMAT_RGB32 = 1,
	PIXEL_FORMAT_YUV420P
};

class MediaClosure {
public:
	MediaClosure () : 
		callback (NULL), frame (NULL), media (NULL), context (NULL)
	{
	}
	~MediaClosure ();
	
	MediaCallback* callback;
	MediaFrame* frame;
	Media* media;
	void* context;
	
	MediaResult Call ()
	{
		if (callback)
			return callback (this);
			
		return MEDIA_NOCALLBACK;
	}
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
	MediaFrame* GetNextFrame (IMediaStream* stream);
	
	//	Requests reading of the next frame
	void GetNextFrameAsync (IMediaStream* stream);
	void ClearQueue (); // Clears the queue and make sure the thread has finished processing what it's doing
	void DeleteQueue (); // Deletes the queue and finishes the thread that's processing the queue.
	void SetQueueCallback (MediaClosure* closure) { queue_closure = closure; }
	
	IMediaSource* GetSource () { return source; }
	IMediaDemuxer* GetDemuxer () { return demuxer; }
	void* GetElement () { return element; }
	const char* GetFileOrUrl () { return file_or_url; }
    
    void AddMessage (MediaResult result, const char* msg);
    void AddMessage (MediaResult result, char* msg);
    
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
		IMediaStream* stream;
	};
};
 
class MediaFrame {
public:
	IMediaStream* stream;
	void* decoder_specific_data; // data specific to the decoder
	guint64 pts; // Set by the demuxer
	guint64 duration; // Set by the demuxer
	//gint32 linesize; // ?
	guint32 compressed_size; // Set by the demuxer
	guint32 uncompressed_size; // Set by the decoder

	void* compressed_data; // Set by the demuxer
	void* uncompressed_data; // Set by the decoder

	//int linesize [4];
	guint8 *uncompressed_data_stride[4];
	int srcSlideY;
	int srcSlideH;
	int srcStride [4];
	
	void printf ()
	{
		//::printf ("pts = %llu, duration = %llu, linesize = %i, %i, %i, %i, compressed_size = %u, uncompressed_size = %u, compressed_data = %p, uncompressed_data = %p", 
		//	pts, duration, linesize [0], linesize [1], linesize [2], linesize [3], compressed_size, uncompressed_size, compressed_data, uncompressed_data);
		//dump_data (compressed_data, compressed_size);
		//dump_data (uncompressed_data, uncompressed_size);
	}
	
	~MediaFrame ();
};

// Interfaces

class IMediaObject {
public:
	IMediaObject (Media* med) : 
		media (med), callback (NULL)
	{
	}
	
	Media* GetMedia () { return media; }
	//	Sets the callback to call when the frame is read
	void SetFrameReadCallback (MediaClosure* callback);
	
	MediaResult ProcessFrame (MediaFrame* frame)
	{
		MediaResult result = ProcessFrameInternal (frame);
		
		if (!MEDIA_SUCCEEDED (result))
			return result;
			
		if (callback)
			return callback->Call ();
			
		return MEDIA_SUCCESS;
	}
	
	
	
protected:
	virtual MediaResult ProcessFrameInternal (MediaFrame* frame) = 0;
	Media* media;
	
private:
	MediaClosure* callback;
};


class IMediaStream  {
public:
	IMediaStream (Media* media) : 
		extra_data (NULL), extra_data_size (NULL), codec_id (0), start_time (0),
		msec_per_frame (0), duration (0), decoder (NULL), codec (NULL), min_padding (0),
		index (-1)
	{
	}
	virtual ~IMediaStream ();

	//	Video, Audio, Markers, etc.
	virtual int GetType () = 0; 
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
	IMediaObject* first_object;
};


// read data, with the possibility of returning a 'wait a bit, need more data first' error value. 
// Another way is to always do the read/demux/decode stuff on another thread, 
// in which case we can block here
class IMediaSource {
public:
	IMediaSource (Media* med) : media (med) {}
	virtual ~IMediaSource () {}
	virtual bool IsSeekable () = 0;
	virtual bool Seek (gint64 position) = 0;
	virtual bool Seek (gint64 position, int mode) = 0;
	virtual bool Read (void* buffer, guint32 size) = 0;
	virtual bool Peek (void* buffer, guint32 size) = 0;
	virtual guint64 GetPosition () = 0;
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
	
	int GetStreamCount () { return stream_count; }
	IMediaStream* GetStream (int index)
	{
		return (index < 0 || index >= stream_count) ? NULL : streams [index];
	}
	
protected:
	void SetStreams (IMediaStream** streams, int count)
	{
		this->streams = streams;
		this->stream_count = count;
	}
	virtual MediaResult ProcessFrameInternal (MediaFrame* frame)
	{
		return ReadFrame (frame);
	}
	
private:
	int stream_count;
	IMediaStream** streams;
};

class IMediaDecoder {
public:
	IMediaDecoder (Media* media, IMediaStream* stream)
	{
		this->media = media;
		this->stream = stream;
	}
	virtual ~IMediaDecoder () {}
	
	virtual MediaResult DecodeFrame (MediaFrame* frame) = 0;
	virtual MediaResult Open () = 0;
	virtual void Cleanup (MediaFrame* frame) {}
	PIXEL_FORMAT pixel_format; // The pixel format this codec outputs.
	IMediaStream* stream;
	Media* media;
	
protected:
	virtual MediaResult ProcessFrameInternal (MediaFrame* frame)
	{
		return DecodeFrame (frame);
	}
};

class IImageConverter {
public:
	IImageConverter (Media* med, VideoStream* str) : 
		media (med), stream (str), input_format (PIXEL_FORMAT_NONE), output_format (PIXEL_FORMAT_NONE)
	{
	}
	virtual ~IImageConverter () {}
	
	virtual MediaResult Open () = 0;
	virtual MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride []) = 0;
	
	Media* media;
	VideoStream* stream;
	PIXEL_FORMAT input_format;
	PIXEL_FORMAT output_format;
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
 
class ASFDemuxer : public IMediaDemuxer {
public:
	ASFDemuxer (Media* media);
	~ASFDemuxer ();
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame* frame);
	
private:
	ASFParser* parser;
	ASFFrameReader* reader;
	gint32* stream_to_asf_index;
};
 
class MSDecoder : public IMediaDecoder {
public:
	MSDecoder (Media* media, IMediaStream* stream) : IMediaDecoder (media, stream) {}
	virtual MediaResult Open ()
	{
	return MEDIA_FAIL;
	}
};

class VideoStream : public IMediaStream {
public:
	VideoStream (Media* media) : IMediaStream (media),
		width (0), height (0), msec_per_frame (0), initial_pts (0),
		bits_per_sample (0), converter (NULL)
	{}
	virtual ~VideoStream ()
	{
		if (converter != NULL) {
			delete converter;
			converter = NULL;
		}
	}
    int GetOutputFormat ();
    
	virtual int GetType () { return MEDIA_VIDEO; } 
    
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
	
    int GetOutputFormat ();
	virtual int GetType () { return MEDIA_AUDIO; }
	int channels;
	int sample_rate;
	int bit_rate;
	int block_align;
	int bits_per_sample;
};
 
class ASFMarkerStream : public IMediaStream {
public:
	ASFMarkerStream (Media* media) : IMediaStream (media) {}
	virtual int GetType () { return MEDIA_MARKER; } 
};

class ASFMarkerDecoder : public IMediaDecoder {
public:
	ASFMarkerDecoder (Media* media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	virtual MediaResult DecodeFrame (MediaFrame* frame) { return MEDIA_SUCCESS; }
	virtual MediaResult Open () {return MEDIA_SUCCESS; }
}; 

#endif
