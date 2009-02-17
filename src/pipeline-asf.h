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
class MemoryQueueSource;

#include "pipeline.h"
#include "asf.h"
#include "mms-downloader.h"

/*
 * MemoryQueueSource
 */
class MemoryQueueSource : public IMediaSource {
private:
	Queue *queue;
	ASFParser *parser;
	bool finished;
	guint64 write_count;
	MmsDownloader *mms_downloader;
	Downloader *downloader;

	EVENTHANDLER (MemoryQueueSource, DownloadFailed, Downloader, EventArgs);
	EVENTHANDLER (MemoryQueueSource, DownloadComplete, Downloader, EventArgs);
	
protected:
	virtual gint32 ReadInternal (void *buf, guint32 n);
	virtual gint32 PeekInternal (void *buf, guint32 n);
	virtual bool SeekInternal (gint64 offset, int mode);
	virtual gint64 GetSizeInternal ();
	virtual gint64 GetLastAvailablePositionInternal ();
	virtual void Dispose ();

public:
	class QueueNode : public List::Node {
	 public:
		ASFPacket *packet;
		MemorySource *source;
		QueueNode (ASFPacket *packet);
		QueueNode (MemorySource *source);
		virtual ~QueueNode ();
	};
	
	MemoryQueueSource (Media *media, Downloader *downloader);
	void AddPacket (MemorySource *packet);
	ASFPacket *Pop ();
	bool Advance (); 

	virtual void NotifySize (gint64 size);
	virtual void NotifyFinished ();

	virtual MediaResult Initialize ();
	virtual MediaSourceType GetType () { return MediaSourceTypeQueueMemory; }
	virtual gint64 GetPositionInternal ();
	virtual void Write (void *buf, gint64 offset, gint32 n);
	void WritePacket (void *buf, gint32 n);
	
	virtual bool CanSeek () { return true; }
	virtual bool Eof () { return finished && queue && queue->IsEmpty (); }

	virtual const char *ToString () { return "MemoryQueueSource"; }
	virtual bool CanSeekToPts () { return true; }
	virtual MediaResult SeekToPts (guint64 pts);
	
	bool IsFinished () { return finished; } // If the server sent the MMS_END packet.
	
	Queue *GetQueue ();
	void SetParser (ASFParser *parser);
	ASFParser *GetParser ();
	MmsDownloader *GetMmsDownloader () { return mms_downloader; }
	Downloader *GetDownloader () { return downloader; }
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
