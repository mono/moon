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

#include <glib.h>

namespace Moonlight {

class ASFContext;
class ASFDemuxer;
class ASFDemuxerInfo;
class ASFExtendedStreamProperties;
class ASFFileProperties;
class ASFFrameReader;
class ASFMarker;
class ASFMarkerDecoder;
class ASFMarkerDecoderInfo;
class ASFMultiplePayloads;
class ASFScriptCommand;
class ASFSinglePayload;
class ASFStreamProperties;
class MmsSource;
class MmsPlaylistEntry;

class WaveFormatEx;
class WaveFormatExtensible;
};

#include "list.h"
#include "dependencyobject.h"
#include "downloader.h"
#include "pipeline.h"
#include "mutex.h"
#include "http-streaming.h"

namespace Moonlight {

class ContentDescriptionList {
public:
	List list;
	bool Parse (const char *input, gint32 length);
};

class ContentDescription : public List::Node {
public:
	enum ValueType {
		VT_BOOL   = 11,
		VT_BSTR   =  8,
		VT_CY     =  6,
		VT_ERROR  = 10,
		VT_I1     = 16,
		VT_I2     =  2,
		VT_I4     =  3,
		VT_INT    = 22,
		VT_LPWSTR = 31,
		VT_UI1    = 17,
		VT_UI2    = 18,
		VT_UI4    = 19,
		VT_UINT   = 23,
	};

public:
	virtual ~ContentDescription ();
	char *name;
	ValueType value_type;
	void *value;
	int value_length;
};

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
 *       - Media creates an MmsSource for the uri and opens it
 *         - MmsSource sends a DESCRIBE request for the uri, also requesting a packet pair experiment
 *
 *       Media::Open ():
 *         Media::SelectDemuxer ():
 *         - MmsSource creates an MmsDemuxer
 *         Media::SelectDecoders ():
 *         - No decoders selected, since MmsDemuxer represents a playlist
 *         OpenCompletedEvent is raised:
 *          - The current PlaylistEntry will replace itself with the playlist the MmsDemuxer returns
 *
 *     - MmsSource may receive P (packet pair) responses
 *     - MmsSource may receive an M (metadata) response
 *       - MmsSource::SetMmsMetadata is called and fills in the given the metadata (playlist-gen-id, broadcast-id, features)
 *       - if we're getting a describe response, or if a stream switch has happened, create a new MmsPlaylistEntry
 *     - MmsSource will receive an H (header) response
 *       - If there is no current playlist entry (normally set with the M response, which is optional), then create a playlist entry
 *       - MmsSource::ParseHeader is called, which will create and open the ASFDemuxer.
 *     - ASFDemuxer raises it's opened event:
 *       - if it's the initial entry, send a PLAY request with the selected streams. We'll get M + H packets again, they're ignored.
 *       - otherwise (we've gotten a stream switch), send a SELECTSTREAM request. Note that the server will start sending
 *         D responses after a stream switch even if no SELECTSTREAM request has been sent, so we may have a few moments of
 *         playback with streams we didn't want.
 *
 *     - MmsSource receives D (data) responses
 *
 *     then optionally repeat this:
 *     - MmsSource may receive an E (stream ended) response, with reason = 1 (still playlist entries to be transmitted)
 *     - MmsSource receives a C (stream change/switch) response
 *       MmsSource::ReportStreamChange change is called
 *     - MmsSource may receive a M (metadata) response
 *       - MmsSource::SetMmsMetadata is called and fills in the given the metadata (playlist-gen-id, broadcast-id, features),
 *         and creates a new MmsPlaylistEntry
 *     - MmsSource receives an H (header) response
 *     - MmsSource receives D (data) responses
 *
 *     finally:
 *     - MmsSource receives an E (stream ended) response, with reason = 0 (no more data will be sent)
 */

struct MmsHeader {
	char b:1;
	char frame:7;
	guint8 id;
	guint16 length;
};

struct MmsHeaderReason {
	char b:1;
	char frame:7;
	guint8 id;
	guint16 length;
	guint32 reason;
};

struct MmsDataPacket {
	guint32 id;
	guint8 incarnation;
	guint8 flags;
	guint16 size;
};

struct MmsPacket {
	union {
		guint32 reason;
		MmsDataPacket data;
	} packet;
};

/*
 * MmsSource
 */
class MmsSource : public IMediaSource {
private:
	enum MmsWaitingState {
		MmsInitialization,
		MmsDescribeResponse,
		MmsPlayResponse,
		MmsStreamSwitchResponse,
	};

	Uri *uri; /* write in ctor only: thread-safe */
	Uri *request_uri; /* write in ctor only: thread-safe */
	char *client_id; /* must use locking to be thread-safe */
	bool finished;
	bool is_sspl;
	bool failure_reported;
	guint64 max_bitrate; /* must use locking to be thread-safe */
	HttpRequest *request; /* must use locking to be thread-safe (unrefs are done on the media thread in Dispose) */
	char *buffer; /* write in ctor/dtor, rw in main thread: thread-safe, no locks required */
	guint32 buffer_size; /* write in ctor/dtor, rw in main thread: thread-safe, no locks required */
	guint32 buffer_used; /* write in ctor/dtor, rw in main thread: thread-safe, no locks required */
	guint64 requested_pts; /* must be thread-safe */
	MmsWaitingState waiting_state; /* write in ctor, rw in main thread: thread-safe, no locks required */
	List *temporary_downloaders; /* must be thread-safe, unrefs are done on the media thread */

	/* Packet pair experiment data */
	TimeSpan p_packet_times[3]; /* write in ctor, rw in main thread: thread-safe, no locks required */
	gint32 p_packet_sizes[3]; /* write in ctor, rw in main thread: thread-safe, no locks required */
	guint8 p_packet_count; /* write in ctor, rw in main thread: thread-safe, no locks required */

	// this is the current entry being downloaded (not necessarily played).
	MmsPlaylistEntry *current;
	MmsDemuxer *demuxer;

	EVENTHANDLER (MmsSource, Started, HttpRequest, EventArgs); // Main thread only
	EVENTHANDLER (MmsSource, Stopped, HttpRequest, HttpRequestStoppedEventArgs); // Main thread only
	EVENTHANDLER (MmsSource, Write, HttpRequest, HttpRequestWriteEventArgs); // Main thread only
	EVENTHANDLER (MmsSource, Opened, IMediaDemuxer, EventArgs);
	EVENTHANDLER (MmsSource, MediaError, Media, EventArgs);

	static void DeleteTemporaryDownloaders (EventObject *data); // Main thread only

	static void SendDescribeRequestCallback (EventObject *obj) { ((MmsSource *) obj)->SendDescribeRequest (); }  /* Main thread only */
	static void SendPlayRequestCallback (EventObject *obj) { ((MmsSource *) obj)->SendPlayRequest (); } /* Main thread only */
	void SendDescribeRequest (); /* Main thread only */
	void SendPlayRequest ();/* Main thread only */
	void SendSelectStreamRequest (); /* Main thread only */
	void SendLogRequest ();/* Main thread only */

	void ProcessResponseHeader (const char *header, const char *value); /* Main thread only */

	HttpStreamingFeatures GetCurrentStreamingFeatures (); /* thread-safe */
	char *GetCurrentPlaylistGenId (); // thread safe, returns a duped string, must be freed with g_free
	char *GetCurrentBroadcastId (); // thread safe, returns a duped string, must be freed with g_free
	char *GetClientId (); // thread safe, returns a duped string, must be freed with g_free

	bool ProcessPacket             (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessDataPacket         (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessHeaderPacket       (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessMetadataPacket     (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessPairPacket         (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessStreamSwitchPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */
	bool ProcessEndPacket          (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size); /* Main thread only */

	void CreateDownloaders (const char *method); /* Thread-safe since it doesn't touch any instance fields */
	void CreateDownloaders (const char *method, HttpRequest **request); /* Main thread only */
	void SetStreamSelectionHeaders (HttpRequest *request); /* Main thread only */
	HttpRequest *GetRequestReffed (); /* Thread-safe */

	bool RemoveTemporaryDownloader (HttpRequest *request); /* main thread only */
	/* Creates the current entry if it doesn't exist, otherwise the already existing current entry is returned. Caller must unref the entry */
	MmsPlaylistEntry *CreateCurrentEntry (); /* thread-safe */

	void SetMaxBitRate (guint64 value); /* thread-safe */

	void WritePacket (void *buf, gint32 n); // forwards to the current entry. Main thread only
	MmsPlaylistEntry *GetCurrentReffed (); // thread safe

	void SetMmsMetadata (const char *playlist_gen_id, const char *broadcast_id, HttpStreamingFeatures features); // Main thread only
	MediaResult ParseHeader (void *buffer, gint32 size); // Main thread only
	void ReportStreamChange (gint32 reason); // called by the MmsDownloader when we get a C (stream change) packet. Main thread only.
	void ReportDownloadFailure (); // called by the MmsDownloader when the download fails (404 for instance). Main thread only.

	void NotifyFinished (guint32 reason); // called when we get the END packet. Main thread only.

	void Write (void *buf, gint32 n); /* main thread only */
protected:
	virtual ~MmsSource ();
	virtual void Dispose (); // Thread safe
	virtual void ReadAsyncInternal (MediaReadClosure *closure);

public:
	MmsSource (Media *media, const Uri *uri);

	virtual MediaResult Initialize (); // media thread only

	virtual bool CanSeek ();
	virtual bool Eof (); // thread safe

	virtual bool CanSeekToPts () { return true; }
	virtual MediaResult SeekToPts (guint64 pts);  // thread safe

	virtual IMediaDemuxer *CreateDemuxer (Media *media, MemoryBuffer *initial_buffer); // thread safe

	MmsDemuxer *GetDemuxerReffed (); // thread safe

	bool IsSSPL () { return is_sspl; }

	guint64 GetMaxBitRate (); /* thread-safe */
};

/*
 * MmsPlaylistEntry
 */
class MmsPlaylistEntry : public IMediaSource {
private:
	List *buffers; /* list of memory buffers we haven't passed to the demuxer yet since it hasn't opened yet. media thread only */
	bool finished;
	MmsSource *parent;
	guint64 write_count; /* just for statistics */
	ASFDemuxer *demuxer;
	bool opened;

	// mms metadata
	char *playlist_gen_id;
	char *broadcast_id;
	HttpStreamingFeatures features;

	static MediaResult WritePacketCallback (MediaClosure *closure);
	static MediaResult ParseHeaderCallback (MediaClosure *closure);
	static void AddEntryCallback (EventObject *obj);
	void AddEntry (); // main thread

	void ParseHeader (MediaClosure *closure); /* media thread */

	EVENTHANDLER (MmsPlaylistEntry, Opened, IMediaDemuxer, EventArgs); /* media thread */
protected:
	virtual void Dispose (); // thread safe
	/* We completely ignore any read requests, mms streams push data to the demuxer instead when we get it */
	virtual void ReadAsyncInternal (MediaReadClosure *closure) {}

public:
	MmsPlaylistEntry (Media *media, MmsSource *source);
	
	virtual MediaResult Initialize ();
	
	virtual bool CanSeekToPts () { return true; } // thread safe
	virtual MediaResult SeekToPts (guint64 pts);  // thread safe
	
	virtual bool Eof () { return finished; } // thread safe
	virtual IMediaDemuxer *CreateDemuxer (Media *media, MemoryBuffer *initial_buffer); // thread safe
	
	// this method reports any errors to the media
	void ParseHeaderAsync (void *buffer, gint32 size, EventObject *callback, EventHandler handler); // main thread
	
	bool IsHeaderParsed ();
	MmsSource *GetParentReffed (); // thread safe
	ASFDemuxer *GetDemuxerReffed (); // thread safe

	void WritePacket (void *buf, gint32 n); // main thread

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
	// Note that the valid indexes range from 1-127, 0 isn't used.
	void GetSelectedStreams (gint8 streams [128]); // main thread only
	
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
};

/*
 * ASFDemuxer
 */
class ASFDemuxer : public IMediaDemuxer {
private:
	/* Our list of frame readers */
	ASFFrameReader *readers [127];

	/* The index of the last packet read */
	guint64 last_packet_index;

	/* The index of the first packet read (either really the first one, or the first one after a seek) */
	guint64 first_packet_index;

	/* The index of the next packet we should read */
	guint64 next_packet_index;

	/* The index of the packet we're waiting for. */
	guint64 pending_packet_index;

	/* If we're currently seeking, the pts we seeked to. */
	guint64 seeked_to_pts;

	/* The pending read (if any) */
	MediaReadClosure *pending_read;

	/* Header object data */
	guint64 header_size;
	bool header_read;

	/* Data object data */
	guint64 data_object_size;
	guint64 data_packet_count;
	gint64 packet_offset; /* location of the beginning of the first packet */
	gint64 packet_offset_end; /* location of the end of the last packet */

	/* The objects from the header objects we need */
	ASFFileProperties *file_properties;
	ASFStreamProperties *stream_properties [127];
	ASFExtendedStreamProperties *extended_stream_properties [127];
	ASFMarker *marker;
	ASFScriptCommand *script_command;

	guint32 *stream_to_asf_index;
	MemoryBuffer *initial_buffer;

	MmsPlaylistEntry *playlist_entry; /* write at ctor/dispose time, so this field is safe to access from all threads */

	bool SetStream (ASFStreamProperties *stream);
	bool SetExtendedStream (ASFExtendedStreamProperties *stream);

	bool ReadExtendedHeaderObject (MemoryBuffer *source, guint64 size);

	void ReadMarkers ();
	void OpenDemuxer (MemoryBuffer *buffer);

	static MediaResult OpenDemuxerCallback (MediaClosure *closure);

	/* Resets all readers */
	void ResetAll ();

	/* Deliver a packet to the reader. This method will deliver each payload to its corresponding frame reader. */
	void DeliverPacket (ASFPacket *packet);

	/* The number of streams */
	guint32 GetStreamCount ();

	/*
	 * Select the specified stream.
	 * No streams are selected by default.
	 */
	void SelectStream (gint32 stream_index, bool value);

	/*
	 * Returns the frame reader for the specified stream.
	 * The stream must first have been selected using SelectStream.
	 */
	ASFFrameReader *GetFrameReader (guint32 asf_stream_index);

	/* Estimate the packet index of the specified pts.
	 * Calls EstimatePacketIndexOfPts on all readers and returns the lowest value. */
	guint64 EstimatePacketIndexOfPts (guint64 pts);

	static MediaResult DeliverDataCallback (MediaClosure *closure);

	void RequestMorePayloadData ();
	void Init (MemoryBuffer *initial_buffer, MmsPlaylistEntry *playlist_entry);

protected:
	virtual ~ASFDemuxer ();

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	
	/* 
	 * This method will seek to the first keyframe before the requested pts in all selected streams.
	 * Note that the streams will probably be positioned at different pts after a seek (given that
	 * for audio streams any frame is considered a key frame, while for video there may be several
	 * seconds between every key frame).
	 */
	virtual void SeekAsyncInternal (guint64 seekToTime);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream);

public:
	ASFDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
	ASFDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer, MmsPlaylistEntry *playlist_entry);
	virtual void Dispose ();

	virtual void UpdateSelected (IMediaStream *stream);

	IMediaStream *GetStreamOfASFIndex (guint32 asf_index);

	/* 
	 * @stream_index: valid values range from 1 to 127
	 * If the stream_index doesn't specify a valid stream (for whatever reason), NULL is returned.
	 */
	ASFStreamProperties *GetStreamProperties (guint32 stream_index);
	ASFExtendedStreamProperties *GetExtendedStreamProperties (guint32 stream_index);

	/* The file properties */
	ASFFileProperties *GetFileProperties () { return file_properties; }

	/* Checks if the stream_index (range 1 - 127) is a valid stream index in the asf file. */
	bool IsValidStream (guint32 stream_index);

	/* Deliver data to the demuxer */
	void DeliverData (gint64 offset, MemoryBuffer *source);

	/* The packet size */
	guint32 GetPacketSize ();

	/*
	 * Reads the number of the specified encoded length (0-3)
	 * encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	 * into the destionation.
	 */
	static bool ReadEncoded (MemoryBuffer *source, guint32 encoded_length, guint32 *dest);

	/* The preroll value (in milliseconds */
	guint64 GetPreroll ();

	/* The number of packets in the stream (0 if unknown). */
	guint64 GetPacketCount ();

	/* Returns 0 on failure, otherwise the offset of the packet index. */
	gint64 GetPacketOffset (guint64 packet_index);

	/* Returns the index of the packet at the specified offset (from the beginning of the file) */
	guint64 GetPacketIndex (gint64 offset);
	
	bool Eof ();

	/* Tries to read a header object from the specified source
	 * @source: input
	 * @error_message: upon exit, contains the error message.
	 * @required_size: upon exit, contains the required size (if the header could not be read due to not enough data available).
	 * 
	 * Returns true if the header could be read, false if not. If the header could not be read and there is no error
	 * message, more data is needed, and required_size is set.
	 * If error_message is set upon return, the caller must call g_free on it. 
	 */
	bool ReadHeaderObject (MemoryBuffer *source, char **error_message, guint32 *required_size);

#if DEBUG
	/* Note that this method is doing thread-unsafe things when called from the main thread (which the debug ui
	 * does), so it's only a debug method */
	guint32 GetPayloadCount (IMediaStream *stream);
#endif
};

/*
 * ASFDemuxerInfo
 */
class ASFDemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (MemoryBuffer *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
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

/*
 * ASFGuid
 */
class ASFGuid {
public:
	guint32 a;
	guint16 b;
	guint16 c;
	guint8 d [8];

	bool Read (MemoryBuffer *buffer);
	bool operator == (const ASFGuid& other) const
	{
		return (a == other.a && 
			b == other.b && 
			c == other.c && 
			d [0] == other.d [0] && 
			d [1] == other.d [1] && 
			d [2] == other.d [2] && 
			d [3] == other.d [3] && 
			d [4] == other.d [4] && 
			d [5] == other.d [5] && 
			d [6] == other.d [6] && 
			d [7] == other.d [7]);
	}
	bool operator != (const ASFGuid& other) const
	{
		return !(*this == other);
	}

	/* Returns a human-readable string for the guid. The returned string does not need to be freed. */
	const char *ToString ();
};

extern ASFGuid asf_guids_empty;

/* Top level object guids */

extern ASFGuid asf_guids_header;
extern ASFGuid asf_guids_data;
extern ASFGuid asf_guids_index;
extern ASFGuid asf_guids_simple_index;
extern ASFGuid asf_guids_media_object_index;
extern ASFGuid asf_guids_timecode_index;

/* Header object guids */
extern ASFGuid asf_guids_file_properties;
extern ASFGuid asf_guids_stream_properties;
extern ASFGuid asf_guids_header_extension;
extern ASFGuid asf_guids_codec_list;
extern ASFGuid asf_guids_script_command;
extern ASFGuid asf_guids_marker;
extern ASFGuid asf_guids_bitrate_mutual_exclusion;
extern ASFGuid asf_guids_error_correction;
extern ASFGuid asf_guids_content_description;
extern ASFGuid asf_guids_extended_content_description;
extern ASFGuid asf_guids_content_branding;
extern ASFGuid asf_guids_stream_bitrate_properties;
extern ASFGuid asf_guids_content_encryption;
extern ASFGuid asf_guids_extended_content_encryption;
extern ASFGuid asf_guids_digital_signature;
extern ASFGuid asf_guids_padding;

/* Header extension object guids */
extern ASFGuid asf_guids_extended_stream_properties;
extern ASFGuid asf_guids_advanced_mutual_exclusion;
extern ASFGuid asf_guids_group_mutual_exclusion;
extern ASFGuid asf_guids_stream_prioritization;
extern ASFGuid asf_guids_bandwidth_sharing;
extern ASFGuid asf_guids_language_list;
extern ASFGuid asf_guids_metadata;
extern ASFGuid asf_guids_metadata_library;
extern ASFGuid asf_guids_index_parameters;
extern ASFGuid asf_guids_media_object_index_parameters;
extern ASFGuid asf_guids_timecode_index_parameters;
extern ASFGuid asf_guids_compatibility;
extern ASFGuid asf_guids_advanced_content_encryption;

/* Stream properties object, stream type guids */
extern ASFGuid asf_guids_media_audio;
extern ASFGuid asf_guids_media_video;
extern ASFGuid asf_guids_media_command;
extern ASFGuid asf_guids_media_jfif;
extern ASFGuid asf_guids_media_degradable_jpeg;
extern ASFGuid asf_guids_file_transfer;
extern ASFGuid asf_guids_binary;

/* Web stream type-specific data guids */
extern ASFGuid asf_guids_webstream_media_subtype;
extern ASFGuid asf_guids_webstream_format;

/* Stream properties, object error correction type guids */
extern ASFGuid asf_guids_no_error_correction;
extern ASFGuid asf_guids_audio_stread;

/* Header extension object guids */
extern ASFGuid asf_guids_reserved1;

/* Advanced content encryption object system id guids */
extern ASFGuid asf_guids_drm;
// drm = Content_Encryption_System_Windows_Media_DRM_Network_Devides in the spec
// Figured it was somewhat long, so it got abbreviated 

/* Codec list object guids */
extern ASFGuid asf_guids_reserved2;

/* Script command object guids */ 
extern ASFGuid asf_guids_reserved3;

/* Marker object guids */
extern ASFGuid asf_guids_reserved4;

/* Mutual exclusion object exclusion type guids */
extern ASFGuid asf_guids_mutex_language;
extern ASFGuid asf_guids_mutex_bitrate;
extern ASFGuid asf_guids_mutex_unknown;

/* Bandwidth sharing object guids */
extern ASFGuid asf_guids_bandwidth_sharing_exclusive;
extern ASFGuid asf_guids_bandwidth_sharing_partial;

/* Standard payload extension system guids */
extern ASFGuid asf_guids_payload_timecode;
extern ASFGuid asf_guids_payload_filename;
extern ASFGuid asf_guids_payload_content_type;
extern ASFGuid asf_guids_payload_pixel_aspect_ratio;
extern ASFGuid asf_guids_payload_sample_duration;
extern ASFGuid asf_guids_payload_encryption_sample_id;


/*
 * WaveFormatEx
 */
class WaveFormatEx {
public:
	guint16 codec_id;
	guint16 channels;
	guint32 samples_per_second;
	guint32 bytes_per_second;
	guint16 block_alignment;
	guint16 bits_per_sample;
	guint16 codec_specific_data_size;
	guint8 *codec_specific_data;

	WaveFormatEx ();
	virtual ~WaveFormatEx ();
	virtual bool Read (ASFContext *context);

	guint8 *GetCodecSpecificData () { return codec_specific_data; }
	WaveFormatExtensible *GetWaveFormatExtensible () { if (!IsWaveFormatExtensible ()) return NULL; return (WaveFormatExtensible *) this; }
	virtual bool IsWaveFormatExtensible () { return false; }
};

/*
 * WaveFormatExtensible
 */
class WaveFormatExtensible : public WaveFormatEx {
public:
	union {
		guint16 valid_bits_per_sample;
		guint16 samples_per_block;
		guint16 reserved;
	} Samples;
	guint32 channel_mask;
	ASFGuid sub_format;

	WaveFormatExtensible ();
	virtual ~WaveFormatExtensible ();
	virtual bool Read (ASFContext *context);

	virtual bool IsWaveFormatExtensible () { return true; }
};

/*
 * BitmapInfoHeader
 */
class BitmapInfoHeader {
public:
	guint32 size;
	guint32 image_width;
	guint32 image_height;
	guint16 planes;
	guint16 bits_per_pixel;
	guint32 compression_id;
	guint32 image_size;
	guint32 hor_pixels_per_meter;
	guint32 ver_pixels_per_meter;
	guint32 colors_used;
	guint32 important_colors_used;

	guint32 extra_data_size;
	guint8 *extra_data;

	BitmapInfoHeader ();
	~BitmapInfoHeader ();
	bool Read (ASFContext *context);

	guint32 GetExtraDataSize () { return extra_data_size; }
	guint8 *GetExtraData () { return extra_data; }
};

/*
 * ASFVideoStreamData
 */
class ASFVideoStreamData {
public:
	guint32 image_width;
	guint32 image_height;
	guint8 flags;
	guint16 format_data_size;
	BitmapInfoHeader *bitmap_info_header;
	
	ASFVideoStreamData ();
	~ASFVideoStreamData ();
	bool Read (ASFContext *context);
	
	BitmapInfoHeader *GetBitmapInfoHeader () { return bitmap_info_header; }
};

/*
 * ASFErrorCorrectionData
 */
class ASFErrorCorrectionData {
public:
	guint8 data;
	guint8 first;
	guint8 second;
	guint8 size;
	
	bool Read (ASFContext *context);
	
	bool IsErrorCorrectionPresent () { return (data & 0x80); }
	bool IsOpaqueDataPresent () { return (data & 0x10); }
	int GetDataLength () { return (data & 0x0F); }
	int GetErrorCorrectionLengthType () { return (data & 0x60) >> 5; }
	
	guint32 GetSize () { return size; }
};

/*
 * ASFPayloadParsingInformation
 */
class ASFPayloadParsingInformation {
public:
	guint8 length_type_flags;
	guint8 property_flags;

	guint32 packet_length;
	guint32 sequence;
	guint32 padding_length;

	guint32 send_time;
	guint16 duration;
	guint32 size;

	bool IsMultiplePayloadsPresent () { return length_type_flags & 0x01; }
	int  GetSequenceLengthType () { return (length_type_flags >> 1) & 0x03; }
	int  GetPaddingLengthType () { return (length_type_flags >> 3) & 0x03; }
	int  GetPacketLengthType () { return (length_type_flags >> 5) & 0x03; }
	bool IsErrorCorrectionPresent () { return length_type_flags & 0x80; }
	
	int  GetReplicatedDataLengthType () { return property_flags & 0x03; }
	int  GetOffsetIntoMediaObjectLengthType () { return (property_flags >> 2) & 0x03; }
	int  GetMediaObjectNumberLengthType () { return (property_flags >> 4) & 0x03; }
	int  GetStreamNumberLengthType () { return (property_flags >> 6) & 0x03; }
	
	bool Read (ASFContext *context);

	guint32 GetSize () { return size; }
};

/*
 * ASFSinglePayload
 */
class ASFSinglePayload {
public:
	guint8 stream_id;
	bool is_key_frame;
	guint32 media_object_number;
	guint32 offset_into_media_object;
	guint32 replicated_data_length;
	guint8* replicated_data;
	guint32 payload_data_length;
	guint8* payload_data;
	guint32 presentation_time; /* milliseconds */
	
	ASFSinglePayload ();
	~ASFSinglePayload ();
	 
	bool Read (ASFContext *context, ASFErrorCorrectionData *ecd, ASFPayloadParsingInformation *ppi, ASFMultiplePayloads *mp);
	
	guint8 GetPresentationTimeDelta () { return replicated_data_length == 1 ? *payload_data : 0; }
	bool IsCompressed () { return replicated_data_length == 1; }
	guint32 GetPresentationTime () { return presentation_time; }
	ASFSinglePayload *Clone ();
};

/*
 * ASFMultiplePayloads
 */
class ASFMultiplePayloads {
public:
	guint8 payload_flags;

	ASFSinglePayload** payloads;
	guint32 payloads_size;

	ASFMultiplePayloads ();	
	~ASFMultiplePayloads ();
	bool Read (ASFContext *context, ASFErrorCorrectionData *ecd, ASFPayloadParsingInformation *ppi);

	guint32 GetNumberOfPayloads () { return payloads_size; }
	guint32 GetPayloadLengthType () { return (payload_flags & 0xC0) >> 6; }

	bool ResizeList (guint32 requested_size);
	bool ReadCompressedPayload (ASFSinglePayload *first, guint32 count, guint32 start_index);
	guint32 CountCompressedPayloads (ASFSinglePayload *first); // Returns 0 on failure, values > 0 on success.
	ASFSinglePayload **StealPayloads ();
};

/*
 * ASFFileProperties
 */
class ASFFileProperties {
public:
	ASFGuid file_id;
	guint64 file_size;
	guint64 creation_date;
	guint64 data_packet_count;
	guint64 play_duration; // 100-nanosecond units (pts)
	guint64 send_duration; // 100-nanosecond units (pts)
	guint64 preroll; // milliseconds
	guint32 flags;
	guint32 min_packet_size;
	guint32 max_packet_size;
	guint32 max_bitrate;
	
	ASFFileProperties ();
	~ASFFileProperties () { /* Nothing to do here */ }
	bool Read (ASFContext *context);
	
	bool IsBroadcast () { return (flags & 0x1) == 0x1; }
	bool IsSeekable () { return (flags & 0x2) == 0x2; }
};

/*
 * ASFStreamProperties
 */
class ASFStreamProperties {
public:
	ASFGuid stream_type;
	ASFGuid error_correction_type;
	guint64 time_offset;
	guint32 type_specific_data_length;
	guint32 error_correction_data_length;
	guint16 flags;

	ASFVideoStreamData *video_data;
	WaveFormatEx *audio_data;

	ASFStreamProperties ();
	~ASFStreamProperties ();
	bool Read (ASFContext *context);

	bool IsAudio () { return stream_type == asf_guids_media_audio; }
	bool IsVideo () { return stream_type == asf_guids_media_video; }
	bool IsCommand () { return stream_type ==  asf_guids_media_command; }
	guint32 GetStreamId () { return (flags & 0x7F); }
	bool IsEncrypted () { return (flags & (1 << 15)); }
	WaveFormatEx *GetAudioData () { return audio_data; }
	ASFVideoStreamData *GetVideoData () { return video_data; }
};

/*
 * ASFScriptCommandEntry
 */
class ASFScriptCommandEntry {
private:
	guint32 pts; // milliseconds
	guint16 type_index;
	guint16 name_length; /* length of name in characters */
	char *name;
	const char *type;

public:
	ASFScriptCommandEntry ();
	~ASFScriptCommandEntry ();
	bool Read (ASFContext *context, ASFScriptCommand *command);
	
	guint16 GetTypeIndex () { return type_index; }
	guint16 GetNameLength () { return name_length; }
	guint32 GetPts () { return pts; }
	const char *GetName () { return name; }
	const char *GetType () { return type; }
};

/*
 * ASFScriptCommand
 */
class ASFScriptCommand {
public:
	ASFGuid reserved;
	guint16 command_count;
	guint16 command_type_count;
	char **command_types;
	ASFScriptCommandEntry **commands;

	ASFScriptCommand ();
	~ASFScriptCommand ();

	bool Read (ASFContext *context);

	ASFScriptCommandEntry** GetCommands () { return commands; }
	char **GetCommandTypes () { return command_types; }
};

/*
 * ASFMarkerEntry
 */
class ASFMarkerEntry {
public:
	guint64 offset;
	guint64 pts; // 100-nanosecond units (pts)
	guint16 entry_length;
	guint32 send_time; // milliseconds
	guint32 flags;
	guint32 marker_description_length; /* number of WCHARS */
	char *marker_description;

	ASFMarkerEntry ();
	~ASFMarkerEntry ();
	bool Read (ASFContext *context);

	const char* GetMarkerDescription () const { return marker_description; }
};

/*
 * ASFMarker
 */
class ASFMarker {
public:
	guint32 marker_count;
	guint16 name_length; /* number of valid bytes */
	char *name;
	ASFMarkerEntry **markers;

	ASFMarker ();
	~ASFMarker ();
	bool Read (ASFContext *context);

	const char* GetName () { return name; }
	const ASFMarkerEntry* GetEntry (guint32 index) { return markers [index]; }
};

/*
 * ASFExtendedStreamProperties
 */
class ASFExtendedStreamProperties {
public:
	guint64 size;
	guint64 start_time;
	guint64 end_time;
	guint32 data_bitrate;
	guint32 buffer_size;
	guint32 initial_buffer_fullness;
	guint32 alternate_data_bitrate;
	guint32 alternate_buffer_size;
	guint32 alternate_initial_buffer_fullness;
	guint32 maximum_object_size;
	guint32 flags;
	guint16 stream_id;
	guint16 stream_language_id_index;
	guint64 average_time_per_frame;
	guint16 stream_name_count;
	guint16 payload_extension_system_count;
	ASFStreamProperties *stream_properties;

	ASFExtendedStreamProperties ();
	~ASFExtendedStreamProperties ();
	bool Read (ASFContext *context, guint64 size);

	guint16 GetStreamId () { return stream_id; }

	ASFStreamProperties *StealStreamProperties ()
	{
		ASFStreamProperties *result = stream_properties;
		stream_properties = NULL;
		return result;
	}
};

/*
 * ASFContext
 */
class ASFContext {
public:
	ASFDemuxer *demuxer;
	MemoryBuffer *source;
};

/*
 * ASFPacket
 */
class ASFPacket : public EventObject {
private:
	gint64 position; // The position of this packet. -1 if not known.
	gint32 index; // The index of this packet. -1 if not known.
	MemoryBuffer *source; // The source which is to be used for reading into this packet.
	ASFDemuxer *demuxer;
	ASFMultiplePayloads *payloads; // The payloads in this packet

protected:
	virtual ~ASFPacket ();

public:
	ASFPacket (ASFDemuxer *demuxer, MemoryBuffer *source, gint64 offset);

	guint32 GetPayloadCount (); // Returns the number of payloads in this packet.
	ASFSinglePayload *GetPayload (guint32 index /* 0 based */);
	ASFMultiplePayloads *GetPayloads () { return payloads; }

	bool Read ();
	gint32 GetIndex () { return index; }
};

/*
 * ASFFrameReaderData
 */
struct ASFFrameReaderData {
	ASFSinglePayload *payload;
	ASFFrameReaderData *prev;
	ASFFrameReaderData *next;
	guint64 packet_index;

	ASFFrameReaderData (ASFSinglePayload *load) 
	{
		payload = load;
		prev = NULL;
		next = NULL;
	}
	
	~ASFFrameReaderData ()
	{
		delete payload;
	}
};

/*
 * ASFFrameReaderIndex
 */
struct ASFFrameReaderIndex {
	guint64 start_pts;
	guint64 end_pts;
};

/*
 *	The data in an ASF file has the following structure:
 *		Data
 *			Packets
 *				Payload(s)
 *					Chunks of Media objects
 *		
 *	The problem is that one chunk of "Media object data" can span several payloads (and packets),
 *	and the pieces may come unordered, like this:
 *
 *	- first 25% of media object #1 for stream #1
 *	- first 25% of media object #1 for stream #2
 *	- first 25% of media object #1 for stream #3
 *	- middle 50% of media object #1 for stream #2
 *	- last 75% of media object #1 for stream #1
 *	=> we have now all the data for the first media object of stream #1
 *	
 *	This class implements a reader that allows the consumer to just call Advance() and then get the all data
 *	for each "Media object" (here called "Frame", since it's shorter, and in general it corresponds
 *	to a frame of video/audio, etc, even though the ASF spec states that it can be just about anything)
 *	in one convenient Write () call.
 *	
 *	We keep reading payloads until we have all the data for a frame, the payloads currently not wanted are 
 *	kept in a queue until the next Advance ().
 */

class ASFFrameReader {
private:
	ASFDemuxer *demuxer;
	IMediaStream *stream;
	guint32 stream_number;

	/* The queue of payloads we've built. */
	ASFFrameReaderData *first;
	ASFFrameReaderData *last;
	guint32 data_counter;

	// A list of the payloads in the current frame
	ASFSinglePayload **payloads;
	guint32 payloads_size;

	// Information about the current frame.
	guint64 size;
	guint64 pts;
	guint64 packet_index;

	// Index data
	guint32 index_size; // The number of items in the index.
	ASFFrameReaderIndex *index; // A table of ASFFrameReaderIndexes.

	bool ResizeList (guint32 size); // Resizes the list of payloads to the requested size. 
	void RemoveAll (); // Deletes the entire queue of payloads (and deletes every element)
	void Remove (ASFFrameReaderData *data); // Unlinks the payload from the queue and deletes it.

public:
	ASFFrameReader (ASFDemuxer *demuxer, guint32 stream_number, IMediaStream *stream);
	~ASFFrameReader ();

	/* Advance to the next frame. */
	void Advance ();

	/* Advance to the next key frame */
	void AdvanceToKeyFrame ();

	/* Checks if there is enough data in the queue for another frame. Returns the pts of the next key frame if found, and if the frame is marked as a key frame (audio streams are not handled specially) */
	bool IsFrameAvailable (guint64 *pts, bool *is_key_frame);

	/* Skips frames until a keyframe is the next frame (so that calling Advance will result in a KeyFrame getting read)  */
	bool SkipToKeyFrame (guint64 *pts);
	
	/* Checks if there is a keyframe in the queue after the first pending frame in the queue (and if so, returns its pts).
	 * Note that this function does not check if we have all the payloads for that subsequent keyframe */
	bool ContainsSubsequentKeyFrame (guint64 *pts);

	/* Write the frame's data to a the destination */
	bool Write (void *dest);

	/* Information about the current frame */
	guint64 Size () { return size; }
	bool IsKeyFrame () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->is_key_frame : false; }
	guint64 Pts () { return pts; }
	guint32 StreamNumber () { return stream_number; }

	void AppendPayload (ASFSinglePayload *payload, guint64 packet_index);

	// Index, returns the packet index of where the frame is.
	// returns UINT32_MAX if not found in the index.
	guint32 FrameSearch (guint64 pts);

	gint64 EstimatePtsPosition (guint64 pts);
	guint64 EstimatePacketIndexOfPts (guint64 pts);

	/* Adds the current frame to the index. */
	void AddFrameIndex ();
	void Reset ();

	guint32 GetDataCounter () { return data_counter; }
};

};
#endif
