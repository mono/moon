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

#ifndef __MOON_PIPELINE_ASF_H_
#define __MOON_PIPELINE_ASF_H_

class ASFDemuxer;
class ASFDemuxerInfo;
class ASFMarkerDecoder;
class ASFMarkerDecoderInfo;
class MmsSource;
class MmsPlaylistEntry;

#include "pipeline.h"
#include "asf.h"
#include "mms-downloader.h"
#include "mutex.h"

/*
 * Mms:
 *
 * # Normal playback (single file or live stream)
 *   * Handled as a playlist with a single entry
 *
 * # Playlist
 *   * Order of events
 *
 *     - Media is created, with an mms:// uri
 *
 *       Media::Initialize ():
 *       - Media creates a downloader for the uri and opens it
 *         - MmsDownloader sends a DESCRIBE request for the uri, also requesting a packet pair experiment
 *       - Media creates an MmsSource for the downloader (which is an MmsDownloader)
 *         - MmsSource creates the first MmsPlaylistEntry for the first entry (which will be the only one if the mms uri isn't a server-side playlist)
 *
 *       Media::Open ():
 *         Media::SelectDemuxer ():
 *         - MmsSource creates an MmsDemuxer
 *         Media::SelectDecoders ():
 *         - No decoders selected, since MmsDemuxer represents a playlist
 *         OpenCompletedEvent is raised:
 *          - The current PlaylistEntry will replace itself with the playlist the MmsDemuxer returns
 *
 *     NOTE: We rely on the fact that mozilla needs a tick to start downloading something,
 *           after the DESCRIBE request everything has been done sync up till now
 *
 *      (ticks)
 *
 *     - MmsDownloader may receive P (packet pair) responses
 *     - MmsDownloader may receive an M (metadata) response
 *       - MmsSource::SetMmsMetadata is called and fills in the given the metadata (playlist-gen-id, broadcast-id, features)
 *     - MmsDownloader will receive an H (header) response
 *       - MmsSource::ParseHeader is called
 *
 *     then optionally repeat this:
 *     - MmsDownloader receives a C (stream change) response
 *       MmsSource::ReportStreamChange change is called
 *       - MmsSource creates a new MmsPlaylistEntry
 *     - MmsDownloader may receive a M (metadata) response
 *       - MmsSource::SetMmsMetadata is called and fills in the given the metadata (playlist-gen-id, broadcast-id, features)
 *     - MmsDownloader receives an H (header) response
 *
 */


/*
 * MmsSource
 */
class MmsSource : public IMediaSource {
private:
	bool finished;
	guint64 write_count;
	Downloader *downloader;
	// this is the current entry being downloaded (not necessarily played).
	MmsPlaylistEntry *current;
	MmsDemuxer *demuxer;

	EVENTHANDLER (MmsSource, DownloadFailed, Downloader, EventArgs); // Main thread only
	EVENTHANDLER (MmsSource, DownloadComplete, Downloader, EventArgs); // Main thread only
	
protected:
	virtual void Dispose (); // Thread safe

public:	
	MmsSource (Media *media, Downloader *downloader);

	void NotifyFinished (guint32 reason); // called by the MmsDownloader when we get the END packet. Main thread only.

	virtual MediaResult Initialize (); // main thread only
	virtual MediaSourceType GetType () { return MediaSourceTypeMms; }

	virtual bool CanSeek () { return true; }
	virtual bool Eof (); // thread safe

	virtual bool CanSeekToPts () { return true; }
	virtual MediaResult SeekToPts (guint64 pts);  // thread safe

	virtual IMediaDemuxer *CreateDemuxer (Media *media); // thread safe

	bool IsFinished () { return finished; } // If the server sent the MMS_END packet.

	Downloader *GetDownloaderReffed (); // thread safe
	MmsDemuxer *GetDemuxerReffed (); // thread safe

	ASFPacket *Pop (); // forwards to the current entry, thread safe
	void WritePacket (void *buf, gint32 n); // forwards to the current entry. Main thread only
	MmsPlaylistEntry *GetCurrentReffed (); // thread safe

	void SetMmsMetadata (const char *playlist_gen_id, const char *broadcast_id, HttpStreamingFeatures features); // Main thread only
	MediaResult ParseHeader (void *buffer, gint32 size); // Main thread only
	void ReportStreamChange (gint32 reason); // called by the MmsDownloader when we get a C (stream change) packet. Main thread only.
	void ReportDownloadFailure (); // called by the MmsDownloader when the download fails (404 for instance). Main thread only.

	// returns the MmsDownloader for the Downloader
	// you must own a ref to the downloader (since this method must be thread safe, 
	// that's the only way to ensure the downloader isn't deleted at any time)
	// and the returned MmsDownloader is only valid as long as you have a ref to 
	// the downloader.
	static MmsDownloader *GetMmsDownloader (Downloader *dl); // thread safe
};

/*
 * MmsPlaylistEntry
 */
class MmsPlaylistEntry : public IMediaSource {
private:
	bool finished;
	Queue queue;
	MmsSource *parent;
	ASFParser *parser;
	guint64 write_count; // just for statistics
	ASFDemuxer *demuxer;

	// mms metadata
	char *playlist_gen_id;
	char *broadcast_id;
	HttpStreamingFeatures features;

protected:
	virtual void Dispose (); // thread safe

public:
	class QueueNode : public List::Node {
	 public:
		ASFPacket *packet;
		MemorySource *source;
		QueueNode (ASFPacket *packet);
		QueueNode (MemorySource *source);
		virtual ~QueueNode ();
	};
	
	MmsPlaylistEntry (Media *media, MmsSource *source);
	
	virtual MediaResult Initialize ();
	virtual MediaSourceType GetType () { return MediaSourceTypeMmsEntry; }
	
	virtual bool CanSeekToPts () { return true; } // thread safe
	virtual MediaResult SeekToPts (guint64 pts);  // thread safe
	
	virtual bool Eof () { return finished && queue.IsEmpty (); } // thread safe
	virtual IMediaDemuxer *CreateDemuxer (Media *media); // thread safe
	bool IsFinished ();  // thread safe
	
	// this method reports any errors to the media
	MediaResult ParseHeader (void *buffer, gint32 size); // main thread
	bool IsHeaderParsed ();
	ASFParser *GetParserReffed (); // thread safe
	MmsSource *GetParentReffed (); // thread safe
	IMediaDemuxer *GetDemuxerReffed (); // thread safe

	ASFPacket *Pop (); // thread safe
	void WritePacket (void *buf, gint32 n); // main thread

	void AddEntry (); // main thread

	void SetPlaylistGenId (const char *value); // thread safe
	char *GetPlaylistGenId (); // thread safe, returns a duped string, must be freed with g_free
	void SetBroadcastId (const char *value); // thread safe
	char *GetBroadcastId (); // thread safe, returns a duped string, must be freed with g_free
	void SetHttpStreamingFeatures (HttpStreamingFeatures value); // thread safe
	HttpStreamingFeatures GetHttpStreamingFeatures (); // thread safe

	// fills in each entry of the array with:
	// -1 - stream does not exist
	//  0 - stream not selected
	//  1 - stream selected
	void GetSelectedStreams (gint64 max_bitrate, gint8 streams [128]); // main thread only
	
	void NotifyFinished (); // called by the MmsSource when we get the END packet for this entry. Main thread only.
};

/*
 * MmsDemuxer
 */
class MmsDemuxer : public IMediaDemuxer {
private:
	Playlist *playlist;
	MmsSource *mms_source;
	Mutex mutex;

protected:
	virtual ~MmsDemuxer () {}
	virtual MediaResult SeekInternal (guint64 pts);

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 seekToTime);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream);

public:
	MmsDemuxer (Media *media, MmsSource *source);
	virtual void Dispose ();

	virtual bool IsPlaylist () { return true; }
	virtual Playlist *GetPlaylist () { return playlist; }
	virtual const char *GetName () { return "MmsDemuxer"; }
};

/*
 * ASFDemuxer
 */
class ASFDemuxer : public IMediaDemuxer {
private:
	gint32 *stream_to_asf_index;
	ASFReader *reader;
	ASFParser *parser;
	
	void ReadMarkers ();
	MediaResult Open ();
	
	static MediaResult GetFrameCallback (MediaClosure *closure);
	
protected:
	virtual ~ASFDemuxer () {}

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 seekToTime);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream);
	
public:
	ASFDemuxer (Media *media, IMediaSource *source);
	virtual void Dispose ();
	
	virtual void UpdateSelected (IMediaStream *stream);
	
	ASFParser *GetParser () { return parser; }
	void SetParser (ASFParser *parser);
	virtual const char *GetName () { return "ASFDemuxer"; }

	IMediaStream *GetStreamOfASFIndex (gint32 asf_index);
};

/*
 * ASFDemuxerInfo
 */
class ASFDemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "ASFDemuxer"; }
};

/*
 * ASFMarkerDecoder
 */
class ASFMarkerDecoder : public IMediaDecoder {
protected:
	virtual ~ASFMarkerDecoder () {};

	virtual void DecodeFrameAsyncInternal (MediaFrame *frame);
	virtual void OpenDecoderAsyncInternal ();
	
public:
	ASFMarkerDecoder (Media *media, IMediaStream *stream) ;
	
	virtual const char *GetName () { return "ASFMarkerDecoder"; }
}; 

/*
 * ASFMarkerDecoderInfo
 */
class ASFMarkerDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char *codec);
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream);
	virtual const char *GetName ();
};

#endif
