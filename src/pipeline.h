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

#include "runtime.h"

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

class IMediaSource;
class IMediaDemuxer;
class IMediaDecoder;
class FileSource;
class LiveSource;

typedef void FrameReadCallback (Media* media, 

class Media {
public:
	Media (MediaElement* element);
	~Media ();
	//	Opens the file using a FileSource
	bool OpenFile (char* file);
	//	Opens the file using a LiveSource
	bool OpenURL (char* url);
	
	//	If it's a file, just open it with FileStream.
	//	If it's a url:
	//	 mms://		try to open with LiveStream, fallback to ProgressiveStream.
	//	 http(s)://	try to open with ProgressiveStream, fallback to LiveStream
	//	 others://	no idea (FIXME).
	bool Open (const char* file_or_url); 
	//	Opens the file
	//	Determines the container type and selects a demuxer
	//	- Default is to use our own ASF demuxer (if it's an ASF file), otherwise use ffmpeg (if available). Overridable by the environment variable MOONLIGHT_OVERRIDES, set demuxer=ffmpeg to force the ffmpeg demuxer.
	//	Makes the demuxer read the data header and fills in stream info, etc.
	//	Selects decoders according to stream info.
	//	- Default is to use MS decoder if available, otherwise ffmpeg. Overridable by MOONLIGHT_OVERRIDES, set codec=ffmpeg to force the ffmpeg decoder.
	bool Open (IMediaSource* source);
	//	Reads the next frame from the demuxer
	//	Requests the decoder to decode the frame
	//	Returns the decoded frame
	Frame* GetNextFrame (IMediaStream* stream);
	//	Requests reading of the next frame
	void GetNextFrameAsync (IMediaStream* stream);
	//	Sets the callback to call when the frame is read
	void SetFrameReadCallback (FrameReadCallback* callback);
	//	Called on another thread, loops the queue of requested frames 
	//	and calls GetNextFrame and FrameReadCallback.
	//	If there are any requests for audio frames in the queue
	//	they are always (and all of them) satisfied before any video frame request.
	void FrameReaderLoop ();

	void EnableStream (IMediaStream stream, bool disable_the_rest);

	IMediaSource* GetSource () { return source; }
	IMediaDemuxer* GetDemuxer () { return demuxer; }
	const char* GetFileOrUrl () { return file_or_url; }
	MediaElement* GetElement () { return element; }
    
private:
	IMediaSource* source;
	IMediaDemuxer* demuxer;
	char* file_or_url;
	MediaElement* element;
};
 
struct Frame {
	IMediaStream* stream;
	guint64 pts;
	guint64 duration;
	guint32 refcount;
	guint32 compresed_size;
	guint32 uncompressed_size;

	void* compressed_data;
	void* uncompressed_data;
};

// Interfaces

class IMediaStream {
public:
	IMediaStream (Media* media);
	virtual ~IMediaStream ();

	//	Video, Audio, Markers, etc.
	virtual int GetType () = 0; 
	IMediaDecoder* GetDecoder () { return decoder; }
	//	If this stream is enabled (producing output). 
	//	A file might have several audio streams, 
	//	and live streams might have several video streams with different bitrates.
	bool IsEnabled () { return enabled; }
	
	//	User defined context value.
	void* GetContext () { return context; }
	void  SetContext (void* context) { this->context = context; }
	
private:
	IMediaDecoder* decoder;
	bool enabled;
	void* context;
};


// read data, with the possibility of returning a 'wait a bit, need more data first' error value. 
// Another way is to always do the read/demux/decode stuff on another thread, 
// in which case we can block here
class IMediaSource {
public:
	IMediaSource (Media* media);
	virtual ~IMediaSource ();
	virtual bool IsSeekable () = 0;
	virtual bool Seek (long position) = 0;
	virtual long Read (void* buffer, long size) = 0;
	virtual long Peek (void* buffer, long size) = 0;
	virtual void EnableStream (IMediaStream stream, bool disable_the_rest) = 0;
	
private:
	Media* media;
};

class IMediaDemuxer {
public:
	IMediaDemuxer (Media* media);
	virtual ~IMediaDemuxer ();
	// Fills the uncompressed_data field in the frame with data.
	virtual bool ReadFrame (Frame* frame) = 0;
};

class IMediaDecoder {
public:
	IMediaDecoder (Media* media);
	virtual ~IMediaDecoder ();
	
	virtual bool DecodeFrame (Frame* frame) = 0;
};

// Implementations
 
class FileSource : public IMediaSource {
public:
	FileSource (Media* media);
	
private: 
};

class ProgressiveSource : public FileSource {
public:
	ProgressiveSource (Media* media);
	virtual ~ProgressiveSource ();
	
	//	The size of the currently available data
	void SetCurrentSize (long size);
	//	The total size of the file (might not be available)
	void SetTotalSize (long size);

private:
};

class LiveSource : public IMediaSource {
public:
	LiveSource (Media* media);
	
};
 
class FfmpegDemuxer : public IMediaDemuxer {
public:
	FfmpegDemuxer ();
};
 
class FfmpegDecoder : public IMediaDecoder {
public:
	FfmpegDecoder ();
};
 
class MSDecoder : public IMediaDecoder {
public:
	MSDecoder ();
	
};

class VideoStream : IMediaStream {
public:
	VideoStream ();
	
    int GetOutputFormat ();
    
    guint32 GetWidth () { return width; }
    guint32 GetHeight () { return height; }
    guint32 GetMilliSecondsPerFrame () { return msec_per_frame; }
    guint64 GetInitialPts () { return initial_pts; }
    
private:
    guint32 width;
    guint32 height;
    guint32 msec_per_frame;
    guint64 initial_pts; 
};
 
class AudioStream : public IMediaStream {
public:
	AudioStream ();
	
    int GetOutputFormat ();
};
 
class ASFMarkerStream : public IMediaStream {
public:
	ASFMarkerStream ();
};
