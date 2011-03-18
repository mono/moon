/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-asf.cpp: ASF related parts of the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include "pipeline-asf.h"
#include "debug.h"
#include "playlist.h"
#include "clock.h"
#include "timesource.h"
#include "factory.h"
#include "medialog.h"

// according to http://msdn.microsoft.com/en-us/library/cc307965(VS.85).aspx the maximum size is 10 MB
#define ASF_OBJECT_MAX_SIZE (10 * 1024 * 1024)

namespace Moonlight {

#define VIDEO_BITRATE_PERCENTAGE 75
#define AUDIO_BITRATE_PERCENTAGE 25
#define INVALID_START_PTS ((guint64) -1)
#define ASF_DECODE_PACKED_SIZE(x) ((x == 3 ? 4 : x))

#define VERIFY_AVAILABLE_SIZE(x) \
	if ((gint64) source->GetRemainingSize () < (gint64) (x)) { \
		LOG_ASF ("%s: unexpected end of stream\n", __func__); \
		return false;	\
	}

#define CLIENT_GUID "{c77e7400-738a-11d2-9add-0020af0a3278}"
#define CLIENT_SUPPORTED "com.microsoft.wm.srvppair, com.microsoft.wm.sswitch, com.microsoft.wm.startupprofile, com.microsoft.wm.predstrm"
#define CLIENT_USER_AGENT "NSPlayer/11.08.0005.0000"

#define MMS_DATA      0x44
#define MMS_HEADER    0x48
#define MMS_METADATA  0x4D
#define MMS_STREAM_C  0x43
#define MMS_END       0x45
#define MMS_PAIR_P    0x50

/*
 * ASFDemuxer
 */

ASFDemuxer::ASFDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
	: IMediaDemuxer (Type::ASFDEMUXER, media, source)
{
	Init (initial_buffer, NULL);
}
ASFDemuxer::ASFDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer, MmsPlaylistEntry *playlist_entry)
	: IMediaDemuxer (Type::ASFDEMUXER, media, source)
{
	Init (initial_buffer, playlist_entry);
}

void
ASFDemuxer::Init (MemoryBuffer *initial_buffer, MmsPlaylistEntry *playlist_entry)
{
	memset (readers, 0, sizeof (ASFFrameReader *) * 127);
	last_packet_index = G_MAXUINT64;
	first_packet_index = G_MAXUINT64;
	next_packet_index = G_MAXUINT64;
	pending_packet_index = G_MAXUINT64;
	seeked_to_pts = G_MAXUINT64;
	header_size = 0;
	pending_read = NULL;
	data_object_size = 0;
	data_packet_count = 0;
	packet_offset = 0;
	packet_offset_end = 0;
	file_properties = NULL;
	memset (stream_properties, 0, sizeof (ASFStreamProperties *) * 127);
	memset (extended_stream_properties, 0, sizeof (ASFExtendedStreamProperties *) * 127);
	marker = NULL;
	script_command = NULL;
	stream_to_asf_index = NULL;
	header_read = false;
	this->playlist_entry = playlist_entry;
	if (this->playlist_entry != NULL)
		this->playlist_entry->ref ();
	this->initial_buffer = initial_buffer;
	this->initial_buffer->ref ();
}

void
ASFDemuxer::Dispose ()
{
	if (initial_buffer) {
		initial_buffer->unref ();
		initial_buffer = NULL;
	}

	if (playlist_entry != NULL) {
		playlist_entry->unref ();
		playlist_entry = NULL;
	}
	
	IMediaDemuxer::Dispose ();
}

ASFDemuxer::~ASFDemuxer ()
{
	for (int i = 0; i < 127; i++) {
		delete readers [i];
		delete stream_properties [i];
		delete extended_stream_properties [i];
	}
	delete file_properties;
	delete marker;
	delete script_command;
	
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;
}

#if DEBUG
guint32
ASFDemuxer::GetPayloadCount (IMediaStream *stream)
{
	ASFFrameReader *reader;

	if (stream == NULL)
		return 0;

	/* Accessing frame readers from the main thread is not thread-safe. Which is why this method
	 * is in a #if DEBUG. */

	reader = GetFrameReader (stream_to_asf_index [stream->GetIndex ()]);

	if (reader == 0)
		return 0;
	
	return reader->GetDataCounter ();
}
#endif

ASFFrameReader *
ASFDemuxer::GetFrameReader (guint32 stream_index)
{
	if (stream_index <= 0 || stream_index >= 128) {
		fprintf (stderr, "ASFDemuxer::GetFrameReader (%i): Invalid stream index.\n", stream_index);
		return NULL;
	}

	return readers [stream_index];
}

void
ASFDemuxer::ResetAll ()
{
	for (int i = 0; i < 127; i++) {
		if (readers [i] != NULL)
			readers [i]->Reset ();
	}
	last_packet_index = G_MAXUINT64;
	first_packet_index = G_MAXUINT64;
	next_packet_index = G_MAXUINT64;
	pending_packet_index = G_MAXUINT64;
	
	if (pending_read) {
		pending_read->Cancel ();
		pending_read->unref ();
		pending_read = NULL;
	}
}

guint64
ASFDemuxer::EstimatePacketIndexOfPts (guint64 pts)
{
	guint64 result = G_MAXUINT64;
	for (int i = 0; i < 127; i++) {
		if (readers [i] == NULL)
			continue;

		result = MIN (readers [i]->EstimatePacketIndexOfPts (pts), result);
	}
	return result == G_MAXUINT64 ? 0 : result;
}

void
ASFDemuxer::RequestMorePayloadData ()
{
	Media *media;

	/* We should only have one pending read request at a time */
	if (pending_packet_index != G_MAXUINT64) {
		printf ("ASFDemuxer::RequestMorePayloadData (): pending_packet_id shouldn't be set (it's %" G_GUINT64_FORMAT ")\n", pending_packet_index);
		return;
	}

	media = GetMediaReffed ();
	if (media != NULL) {
		pending_packet_index = next_packet_index;
		LOG_ASF ("ASFDemuxer::RequestMorePayloadData (): requesting %u bytes from offset %" G_GUINT64_FORMAT " (packet index: %" G_GUINT64_FORMAT ")\n", GetPacketSize (), GetPacketOffset (next_packet_index), next_packet_index);
		MediaReadClosure *closure = new MediaReadClosure (media, DeliverDataCallback, this, GetPacketOffset (next_packet_index), GetPacketSize ());
		source->ReadAsync (closure);
		closure->unref ();
		media->unref ();
	}
}

void
ASFDemuxer::DeliverPacket (ASFPacket *packet)
{
	/* Distribute the payloads to the per-stream readers. */
	ASFSinglePayload **payloads;
	ASFFrameReader *reader;
	guint32 stream_id;
	guint32 payloads_added = 0;
	guint64 stp;

	LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u) seeked_to_pts: %" G_GUINT64_FORMAT "\n", packet->GetIndex (), seeked_to_pts);

	if (IsDisposed ()) {
		LOG_ASF ("ASFDemuxer::DeliverPacket (): disposed.\n");
		return;
	}

	payloads = packet->GetPayloads ()->StealPayloads ();

	for (int i = 0; payloads [i] != NULL; i++) {
		stream_id = payloads [i]->stream_id;
		reader = GetFrameReader (stream_id);
		if (reader == NULL) {
			LOG_ASF ("ASFDemuxer::DeliverPacket (): got payload for stream #%i which doesn't exist. Payload dropped.\n", stream_id);
			delete payloads [i];
			continue;
		}

		LOG_ASF ("ASFDemuxer::DeliverPacket (): delivered payload for stream %i with pts %" G_GUINT64_FORMAT "\n", 
			stream_id, (guint64) (payloads [i]->GetPresentationTime () - GetFileProperties ()->preroll));

		reader->AppendPayload (payloads [i], packet->GetIndex ());
		payloads_added++;
	}
	g_free (payloads);

	if (seeked_to_pts == G_MAXUINT64) {
		IMediaStream *pending_stream = GetPendingStream ();
		if (pending_stream != NULL) {
			GetFrameAsyncInternal (pending_stream);
		}
	} else {
		/* Check that we have a keyframe below the seeked-to pts for all selected a/v streams */
		bool still_pending = false;
		bool seek_backwards = false;
		for (guint32 i = 0; i < GetStreamCount (); i++) {
			ASFFrameReader *reader = GetFrameReader (stream_to_asf_index [i]);
			IMediaStream *stream = GetStream (i);
			guint64 pts = 0;
			guint64 next_pts = 0;
			bool skip_keyframe;

			if (reader == NULL || stream == NULL)
				continue;

			if (!stream->GetSelected ())
				continue;

			if (!(stream->IsAudioOrVideo ()))
				continue;

			do {
				LOG_ASF ("ASFDemuxer::DeliverPacket (index: %i) stream %i (%s) current pts: %" G_GUINT64_FORMAT " seeked_to_pts: %" G_GUINT64_FORMAT "\n",
					packet->GetIndex (), i, stream->GetTypeName (), MilliSeconds_FromPts (reader->Pts ()), MilliSeconds_FromPts (seeked_to_pts));
				bool key_frame_available = reader->SkipToKeyFrame (&pts);
				bool next_key_frame_available;
				skip_keyframe = false;

				LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): stream %i (%s): key frame available: %i pts. %" G_GUINT64_FORMAT " current pts: %" G_GUINT64_FORMAT "\n",
					packet->GetIndex (), i , stream->GetTypeName (), key_frame_available, MilliSeconds_FromPts (pts), MilliSeconds_FromPts (reader->Pts ()));
	
				if (!key_frame_available) {
					LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): stream %i (%s) does not have a key frame yet. next_packet_index: %" G_GUINT64_FORMAT " packet count: %" G_GUINT64_FORMAT "\n",
						packet->GetIndex (), i, stream->GetTypeName (), next_packet_index, GetPacketCount ());
					if (next_packet_index >= GetPacketCount ()) {
						/* We didn't get a keyframe before the end of the stream, we need to seek backwards */
						seek_backwards = true;
					}
					still_pending = true;
					break;
				}
				next_key_frame_available = reader->ContainsSubsequentKeyFrame (&next_pts);
				
				if (pts > seeked_to_pts) {
					/* First keyframe is after the one we want, seek backward */
					LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): stream: %i (%s): seeked too far ahead, pts: %" G_GUINT64_FORMAT " seeked-to-pts: %" G_GUINT64_FORMAT "\n",
						packet->GetIndex (), i, stream->GetTypeName (), MilliSeconds_FromPts (pts), MilliSeconds_FromPts (seeked_to_pts));
					seek_backwards = true;
					break;
				}
				
				if (next_key_frame_available && next_pts < seeked_to_pts) {
					/* The keyframe after the first keyframe is still below the seeked-to pts
					 * Skip the first keyframe and loop again */
					skip_keyframe = true;
					reader->Advance ();
					LOG_ASF ("ASFDemuxer::DeliverPacket (index. %u): seeked to early (first keyframe: %" G_GUINT64_FORMAT ", next keyframe: %" G_GUINT64_FORMAT ", seeked_to pts: %" G_GUINT64_FORMAT ", current pts is now: %" G_GUINT64_FORMAT "\n",
						packet->GetIndex (), MilliSeconds_FromPts (pts), MilliSeconds_FromPts (next_pts), MilliSeconds_FromPts (seeked_to_pts), MilliSeconds_FromPts (reader->Pts ()));
				}
			} while (skip_keyframe);
			
			if (seek_backwards)
				break;
		}

		if (seek_backwards) {
			guint64 pi = first_packet_index > 10 ? first_packet_index - 10 : 0;
			LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): Seeked too far ahead. Seeking backwards (previously seeked to %" G_GUINT64_FORMAT ", trying %" G_GUINT64_FORMAT " now.\n",
				packet->GetIndex (), first_packet_index, pi);
			ResetAll ();
			first_packet_index = pi;
			next_packet_index = pi;
			still_pending = pi != 0;
		}

		if (!still_pending) {
			LOG_ASF ("ASFDemuxer::DeliverPacket (index. %u): Seek completed!\n", packet->GetIndex ());
			stp = seeked_to_pts;
			seeked_to_pts = G_MAXUINT64;
			ReportSeekCompleted (stp);
		} else {
			LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): requesting more data.\n", packet->GetIndex ());
			RequestMorePayloadData ();
		}
	}

	LOG_ASF ("ASFDemuxer::DeliverPacket (index: %u): delivered %u payloads.\n", packet->GetIndex (), payloads_added);
}

void
ASFDemuxer::SeekAsyncInternal (guint64 pts)
{
	LOG_ASF ("ASFDemuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT ")\n", pts);

	if (source == NULL) {
		/* disposed? */
		return;
	}

	if (!source->CanSeek ()) {
		LOG_ASF ("ASFDemuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT "): Source can't seek, so we'll skip seeking.\n", pts);
		ReportSeekCompleted (pts);
		return;
	}

	ResetAll ();

	if (source->CanSeekToPts ()) {
		if (!MEDIA_SUCCEEDED (source->SeekToPts (pts))) {
			ReportErrorOccurred ("ASFDemuxer: Seek error.");
			return;
		}

		ReportSeekCompleted (pts);
		return;
	}

	/* Special case a seek to 0, there is no need to go into a seeked state, we know which packet we must read */
	if (pts == 0) {
		next_packet_index = 0;
		ReportSeekCompleted (pts);
		return;
	}

	/* Now we can only guess which packet we should seek to */
	next_packet_index = EstimatePacketIndexOfPts (pts);
	LOG_ASF ("ASFDemuxer::SeekAsyncInternal (): Seeking to packet index: %" G_GUINT64_FORMAT ")\n", next_packet_index);
	seeked_to_pts = pts;

	RequestMorePayloadData ();
}

guint64
ASFDemuxer::GetPacketCount ()
{
	return file_properties->data_packet_count;
}

guint32
ASFDemuxer::GetStreamCount ()
{
	int result = 0;
	for (int i = 1; i <= 127; i++) {
		if (IsValidStream (i))
			result++;
	}
	return result;
}

bool 
ASFDemuxer::ReadEncoded (MemoryBuffer *source, guint32 length, guint32 *dest)
{
	switch (length) {
	case 0x00:
		return true;
	case 0x01:
		VERIFY_AVAILABLE_SIZE (1);
		*dest = source->ReadLE_U8 ();
		return true;
	case 0x02:
		VERIFY_AVAILABLE_SIZE (2);
		*dest = source->ReadLE_U16 ();
		return true;
	case 0x03:
		VERIFY_AVAILABLE_SIZE (4);
		*dest = source->ReadLE_U32 ();
		return true;
	default:
		LOG_ASF ("ASFDemuxer::ReadEncoded (): Invalid read length: %i.\n", length);
		*dest = 0;
		return false;
	}
}

bool
ASFDemuxer::ReadExtendedHeaderObject (MemoryBuffer *source, guint64 size)
{
	ASFContext context;
	ASFGuid reserved_field_1; /* Clock type */
	guint16 reserved_field_2; /* Clock size */
	guint32 data_size;
	guint32 data_size_left;
	
	VERIFY_AVAILABLE_SIZE (22);
	if (!reserved_field_1.Read (source)) {
		/* This should not happen, size has already been verified */
		return false;
	}
	reserved_field_2 = source->ReadLE_U16 ();
	data_size = source->ReadLE_U32 ();

	if (data_size > 0 && data_size < 24) {
		LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): Invalid data size for extended header object (got %u, must be 0 or >= 24)\n", data_size);
		return false;
	}

	if (data_size + 46 != size) {
		LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): Invalid data size for extended header object (got %u, must be %" G_GUINT64_FORMAT ")\n", data_size, header_size - 46);
		return false;
	}

	VERIFY_AVAILABLE_SIZE (data_size);

	data_size_left = data_size;

	context.demuxer = this;
	context.source = source;

	while (data_size_left >= 24) {
		ASFGuid guid;
		guint64 size;
		guint64 start_position = source->GetPosition ();

		if (!guid.Read (source)) {
			/* This should not happen, we've verified the available size */
			return false;
		}

		size = source->ReadLE_U64 ();
		if (size > data_size_left) {
			LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): found an object whose size (%" G_GUINT64_FORMAT ") is bigger than the size left (%u)\n", size, data_size_left);
			return false;
		}

		/* No need to verify that the source has enough data for this object, we've already verified that the source
		 * has enough data for the entire header extension object */

		data_size_left -= size;

		if (guid == asf_guids_extended_stream_properties) {
			ASFExtendedStreamProperties *stream = new ASFExtendedStreamProperties ();
			ASFStreamProperties *str = NULL;

			if (!stream->Read (&context, size)) {
				delete stream;
				return false;
			}
			if (!SetExtendedStream (stream)) {
				delete stream;
				return false;
			}
			str = stream->StealStreamProperties ();
			if (str != NULL) {
				if (!SetStream (str)) {
					delete str;
					return false;
				}
			}
		} else {
			LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): Unhandled guid: %s (no action required)\n", guid.ToString ());
			source->SeekOffset (size - 24);
		}

		if (start_position + size != (guint64) source->GetPosition ()) {
			LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): The header object whose guid is %s didn't consume all its data, or the object consumed too much (size: %" G_GUINT64_FORMAT ", bytes consumed: %" G_GUINT64_FORMAT ").\n",
				guid.ToString (), size, (guint64) source->GetPosition () - start_position);
			source->SeekSet (start_position + size);
		}
	}

	if (data_size_left > 0) {
		LOG_ASF ("ASFDemuxer::ReadExtendedHeaderObject (): found junk at the end of the object.\n");
		return false;
	}

	return true;
}

bool
ASFDemuxer::ReadHeaderObject (MemoryBuffer *source, char **error_message, guint32 *required_size)
{
	ASFContext context;
	ASFGuid guid;
	guint64 size;
	guint32 header_object_count;
	bool any_streams = false;

	context.demuxer = this;
	context.source = source;

	/* 
	 * This method has 3 exit paths:
	 * - Not enough data to read: must call required_size and return false. error_message = NULL.
	 * - Something went wrong: must set error_message and return false.
	 * - Success: returns true. error_message = NULL.
	 */

	LOG_ASF ("ASFDemuxer::ReadHeader ()\n");

	*error_message = NULL;

	*required_size = 30;
	if (*required_size > source->GetRemainingSize ()) {
		LOG_ASF ("ASFDemuxer::ReadHeader (): not enough data to start reading the header (%u bytes required, got %" G_GINT64_FORMAT " bytes)\n", *required_size, source->GetRemainingSize ());
		return false;
	}

	if (!guid.Read (source)) {
		/* This should not happen given that we've already verified the available size */
		*error_message = g_strdup_printf ("ASFDemuxer: Could not read header guid.");
		return false;
	}

	size = source->ReadLE_U64 ();
	header_size = size;

	if (size > ASF_OBJECT_MAX_SIZE) {
		*error_message = g_strdup_printf ("ASFDemuxer: header object size (%" G_GUINT64_FORMAT ") is bigger than the max object size (%u)\n", size, ASF_OBJECT_MAX_SIZE);
		return false;
	}

	*required_size = size + 50 /* The size of the data object at the end */;
	if (*required_size > source->GetSize ()) {
		LOG_ASF ("ASFDemuxer::ReadHeader (): not enough data to parse the entire header (got %" G_GINT64_FORMAT " bytes available, needs %u bytes)\n",  source->GetRemainingSize (), *required_size);
		return false;
	}

	header_object_count = source->ReadLE_U32 ();
	source->ReadLE_U8 (); /* reserved1 (alignment) */
	source->ReadLE_U8 (); /* reserved2 (architecture) */ 

	/* Calculate the position of the first packet */
	packet_offset = header_size + 50 /* The size of the data object */;

	LOG_ASF ("ASFDemuxer::ReadHeader (): header_size: %" G_GUINT64_FORMAT " header_object_count: %u\n", header_size, header_object_count);

	if (guid != asf_guids_header) {
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): header guid is not correct.");
		return false;
	}

	VERIFY_AVAILABLE_SIZE (size - 30 /* We've already read 30 bytes */  + 50 /* the size of the data object at the end */);

	for (guint32 i = 0; i < header_object_count; i++) {
		guint64 start_position = source->GetPosition ();

		if (!guid.Read (source)) {
			/* This should not happen, we have verified the size already */
			*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Error while reading guid.\n");
			return false;
		}

		size = source->ReadLE_U64 ();
		
		if (size < 20) {
			*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): data corruption, header object size must be >= 20, got %" G_GUINT64_FORMAT "\n", size);
			return false;
		}

		VERIFY_AVAILABLE_SIZE (size - 20 /* guid + uint64, which we've already read */);

		LOG_ASF ("ASFDemuxer::ReadHeader (): parsing object: %s with size: %" G_GUINT64_FORMAT "\n",
			guid.ToString (), size);

		if (asf_guids_stream_properties == guid) {
			ASFStreamProperties *stream = new ASFStreamProperties ();
			if (!stream->Read (&context)) {
				delete stream;
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while reading stream properties.");
				return false;
			}
			if (!SetStream (stream)) {
				delete stream;
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while storing stream properties.");
				return false;
			}
			any_streams = true;
		} else if (asf_guids_file_properties == guid) {
			if (file_properties != NULL) {
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Multiple file property object in the asf data.");
				return false;
			}
			file_properties = new ASFFileProperties ();
			if (!file_properties->Read (&context)) {
				delete file_properties;
				file_properties = NULL;
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while reading file properties.");
				return false;
			}
		} else if (asf_guids_header_extension == guid) {
			if (!ReadExtendedHeaderObject (source, size)) {
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while reading extended header objects.");
				return false;
			}
		} else if (asf_guids_marker == guid) {
			if (marker != NULL) {
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Multiple marker objects in the asf data.");
				return false;
			}
			marker = new ASFMarker ();
			if (!marker->Read (&context)) {
				delete marker;
				marker = NULL;
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while reading asf marker.");
				return false;
			}
		} else if (asf_guids_script_command == guid) {
			if (script_command != NULL) {
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Multiple script command objects in the asf data.");
				return false;
			}
			script_command = new ASFScriptCommand ();
			if (!script_command->Read (&context)) {
				delete script_command;
				script_command = NULL;
				*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): error while reading script command.");
				return false;
			}
		} else if (asf_guids_content_encryption == guid) {
			LOG_ASF ("ASFDemuxer::ReadHeader (): found drm header object (content encryption).\n");
			SetIsDrm (true);
			source->SeekOffset (size - 24);
		} else if (asf_guids_extended_content_encryption == guid) {
			LOG_ASF ("ASFDemuxer::ReadHeader (): found drm header object (extended content encryption).\n");
			SetIsDrm (true);
			source->SeekOffset (size - 24);
		} else {
			LOG_ASF ("ASFDemuxer::ReadHeader (): Unhandled guid: %s (this is normal)\n", guid.ToString ());
			source->SeekOffset (size - 24);
		}

		if (start_position + size != (guint64) source->GetPosition ()) {
			LOG_ASF ("ASFDemuxer::ReadHeader (): The header object whose guid is %s didn't consume all its data, or the object consumed too much (size: %" G_GUINT64_FORMAT ", bytes consumed: %" G_GUINT64_FORMAT ").\n",
				guid.ToString (), size, (guint64) source->GetPosition () - start_position);
			source->SeekSet (start_position + size);
		}
	}

	/* Check for required objects. */

	if (file_properties == NULL) {
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): No file property object in the asf data.");
		return false;
	}

	if (!any_streams) {
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): No streams in the asf data.");
		return false;
	}

	VERIFY_AVAILABLE_SIZE (50);

	/* Calculate the position of the last packet */
	if (file_properties->data_packet_count != 0)
		packet_offset_end = packet_offset + file_properties->data_packet_count * file_properties->min_packet_size;
	else
		packet_offset_end = -1;

	/* Read the data object */
	if (!guid.Read (source)) {
		/* This should not happen, we've verified the available size */
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): could not read data object guid.");
		return false;
	}

	if (guid != asf_guids_data) {
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Incorrect data guid, got: %s, expected: %s\n", guid.ToString (), asf_guids_data.ToString ());
		return false;
	}

	data_object_size = source->ReadLE_U64 ();

	if (data_object_size > 0 && data_object_size < 50) {
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Invalid data object size: %" G_GUINT64_FORMAT " (must be 0 or >= 50)\n", data_object_size);
		return false;
	}

	if (!guid.Read (source)) {
		/* This should not happen, we've verified the available size */
		*error_message = g_strdup_printf ("ASFDemuxer::ReadHeader (): Error while reading data object guid.");
		return false;
	}

	data_packet_count = source->ReadLE_U64 ();
	source->ReadLE_U16 (); /* reserved field */
	
	LOG_ASF ("ASFDemuxer::ReadHeader (): Success: data_object_size: %" G_GINT64_FORMAT " data_packet_count: %" G_GINT64_FORMAT "\n",
		data_object_size, data_packet_count);

	header_read = true;

	return true;
}

ASFStreamProperties *
ASFDemuxer::GetStreamProperties (guint32 stream_index)
{
	if (stream_index < 1 || stream_index > 127)
		return NULL;
	
	return stream_properties [stream_index - 1];
}

ASFExtendedStreamProperties * 
ASFDemuxer::GetExtendedStreamProperties (guint32 stream_index)
{
	if (stream_index < 1 || stream_index > 127)
		return NULL;
	
	return extended_stream_properties [stream_index - 1];
}

bool
ASFDemuxer::SetStream (ASFStreamProperties *stream)
{
	guint32 stream_index = stream->GetStreamId ();
	
	if (stream_index < 1 || stream_index > 127) {
		LOG_ASF ("ASFDemuxer::SetStream (%i, %p): Invalid stream index.\n", stream_index, stream);
		return false;
	}

	if (stream_properties [stream_index - 1] != NULL) {
		LOG_ASF ("ASFDemuxer::SetStream (%i, %p): The stream index already exists.\n", stream_index, stream);
		return false;
	}

	stream_properties [stream_index - 1] = stream;
	return true;
}

bool
ASFDemuxer::SetExtendedStream (ASFExtendedStreamProperties *stream)
{
	guint32 stream_index = stream->GetStreamId ();

	if (stream_index < 1 || stream_index > 127) {
		LOG_ASF ("ASFDemuxer::SetExtendedStream (%i, %p): Invalid stream index.\n", stream_index, stream);
		return false;
	}

	if (extended_stream_properties [stream_index - 1] != NULL) {
		LOG_ASF ("ASFDemuxer::SetExtendedStream (%i, %p): The stream index already exists.\n", stream_index, stream);
		return false;
	}

	extended_stream_properties [stream_index - 1] = stream;

	return true;
}

bool
ASFDemuxer::IsValidStream (guint32 stream_index)
{
	return GetStreamProperties (stream_index) != NULL;
}

gint64
ASFDemuxer::GetPacketOffset (guint64 packet_index)
{
	if (packet_index < 0 || (file_properties->data_packet_count > 0 && packet_index >= file_properties->data_packet_count)) {
		return 0;
	}
	
	/* CHECK: what if min_packet_size != max_packet_size? */
	return packet_offset + packet_index * file_properties->min_packet_size;
}

guint32
ASFDemuxer::GetPacketSize ()
{
	return file_properties->min_packet_size;
}

guint64
ASFDemuxer::GetPacketIndex (gint64 offset)
{
	if (offset < packet_offset)
		return 0;
	
	if (packet_offset_end > 0 && offset > packet_offset_end)
		return file_properties->data_packet_count - 1;
	
	return (offset - packet_offset) / file_properties->min_packet_size;
}

void
ASFDemuxer::UpdateSelected (IMediaStream *stream)
{
	LOG_ASF ("ASFDemuxer::UpdateSelected (%s => %i)\n", stream->GetTypeName (), stream->GetSelected ());

	guint32 asf_stream_index = stream_to_asf_index [stream->GetIndex ()];

	if (!IsValidStream (asf_stream_index)) {
		fprintf (stderr, "ASFDemuxer::UpdateSelected (%i => %i): Invalid stream index\n", stream->GetIndex (), stream->GetSelected ());
		return;
	}

	if (stream->GetSelected ()) {
		if (readers [asf_stream_index] == NULL) {
			readers [asf_stream_index] = new ASFFrameReader (this, asf_stream_index, stream);
		}
	} else {
		if (readers [asf_stream_index] != NULL) {
			delete readers [asf_stream_index];
			readers [asf_stream_index] = NULL;
		}
	}

	IMediaDemuxer::UpdateSelected (stream);
}

void
ASFDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	LOG_ASF ("ASFDemuxer::SwitchMediaStreamAsyncInternal (%p). TODO.\n", stream);
}

guint64
ASFDemuxer::GetPreroll ()
{
	return file_properties->preroll;
}

void
ASFDemuxer::ReadMarkers ()
{
	/*
		We can get markers from several places:
			- The header of the file, read before starting to play
				- As a SCRIPT_COMMAND
				- As a MARKER
				They are both treated the same way, added into the timeline marker collection when the media is loaded.
			- As data in the file (a separate stream whose type is ASF_COMMAND_MEDIA)
				These markers show up while playing the file, and they don't show up in the timeline marker collection,
				they only get to raise the MarkerReached event.
				currently the demuxer will call the streamed_marker_callback when it encounters any of these.    
	*/
	
	// Hookup to the marker (ASF_COMMAND_MEDIA) stream
	MediaMarker *marker;
	Media *media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	// Read the markers (if any)
	List *markers = media->GetMarkers ();
	const char *type;
	guint64 pts;
	guint64 preroll_pts = MilliSeconds_ToPts (GetPreroll ());
	const char *text;
	int i = -1;
	
	// Read the SCRIPT COMMANDs
	char **command_types = NULL;
	ASFScriptCommandEntry **commands = NULL;
	
	if (script_command != NULL) {
		commands = script_command->GetCommands ();
		command_types = script_command->GetCommandTypes ();
		
		if (command_types == NULL) {
			//printf ("MediaElement::ReadASFMarkers (): No command types.\n");
			goto cleanup;
		}
	}
	
	if (commands != NULL) {
		for (i = 0; commands[i]; i++) {
			ASFScriptCommandEntry *entry = commands [i];
			
			text = entry->GetName ();
			pts = MilliSeconds_ToPts (entry->GetPts ()) - preroll_pts;
			type = entry->GetType () == NULL ? "" : entry->GetType ();

			marker = new MediaMarker (type, text, pts);
			markers->Append (new MediaMarker::Node (marker));
			marker->unref ();
			
			//printf ("MediaElement::ReadMarkers () Added script command at %" G_GUINT64_FORMAT " (text: %s, type: %s)\n", pts, text, type);
		}
	}
	
	// Read the MARKERs
	const ASFMarkerEntry* marker_entry;
	
	if (this->marker != NULL) {
		for (i = 0; i < (int) this->marker->marker_count; i++) {
			marker_entry = this->marker->GetEntry (i);
			text = marker_entry->GetMarkerDescription ();
			
			pts = marker_entry->pts - preroll_pts;

			marker = new MediaMarker ("Name", text, pts);
			markers->Append (new MediaMarker::Node (marker));
			marker->unref ();
			
			//printf ("MediaElement::ReadMarkers () Added marker at %" G_GUINT64_FORMAT " (text: %s, type: %s)\n", pts, text, "Name");
		}
	}
	
		
cleanup:
	media->unref ();
}

void
ASFDemuxer::OpenDemuxerAsyncInternal ()
{
	LOG_PIPELINE ("ASFDemuxer::OpenDemuxerAsyncInternal ()\n");
	OpenDemuxer (initial_buffer);
}

MediaResult
ASFDemuxer::OpenDemuxerCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	((ASFDemuxer *) closure->GetContext ())->OpenDemuxer (closure->GetData ());
	return MEDIA_SUCCESS;
}

void
ASFDemuxer::OpenDemuxer (MemoryBuffer *buffer)
{
	guint32 *stream_to_asf_index = NULL;
	IMediaStream **streams = NULL;
	Media *media;
	int current_stream = 1;
	int stream_count = 0;
	int count;
	char *error_message = NULL;
	guint32 required_size;

	g_return_if_fail (buffer != NULL);

	LOG_ASF ("ASFDemuxer::OpenDemuxer (%" G_GINT64_FORMAT " bytes available)\n", buffer->GetSize ());

	media = GetMediaReffed ();
	if (media == NULL) {
		LOG_ASF ("ASFDemuxer::OpenDemuxer (): no media.\n");
		return;
	}

	if (!header_read && !ReadHeaderObject (buffer, &error_message, &required_size)) {
		if (error_message == NULL) {
			/* More data is needed. Th */
			LOG_ASF ("ASFDemuxer::OpenDemuxer (): requesting more data.\n");
			MediaReadClosure *closure = new MediaReadClosure (media, OpenDemuxerCallback, this, 0, required_size);
			source->ReadAsync (closure);
			closure->unref ();
		} else {
			/* Something went wrong */
			ReportErrorOccurred (error_message);
			g_free (error_message);
		}
		goto failure;
	}
	
	SetIsDrm (IsDrm ());

	/* Count the number of streams */
	for (int i = 1; i <= 127; i++) {
		if (IsValidStream (i)) {
			stream_count++;
		}
	}

	current_stream = 1;
	streams = (IMediaStream **) g_malloc0 (sizeof (IMediaStream *) * (stream_count + 1)); /* End with a NULL element. */
	stream_to_asf_index = (guint32 *) g_malloc0 (sizeof (gint32) * (stream_count + 1)); 

	/* keep count as a separate local since we can change its value (e.g. bad stream) */
	count = stream_count;

	/* Loop through all the streams and set stream-specific data */
	for (int i = 0; i < count; i++) {
		while (current_stream <= 127 && !IsValidStream (current_stream))
			current_stream++;

		if (current_stream > 127) {
			ReportErrorOccurred ("ASFDemuxer: Couldn't find all the claimed streams in the file.");
			goto failure;
		}

		ASFStreamProperties* stream_properties = GetStreamProperties (current_stream);
		IMediaStream* stream = NULL;

		if (stream_properties == NULL) {
			ReportErrorOccurred ("ASFDemuxer: Couldn't find stream properties for all the streams.");
			goto failure;
		}

		if (stream_properties->IsAudio ()) {
			AudioStream *audio = new AudioStream (media);
			ASFExtendedStreamProperties *aesp;

			stream = audio;

			WaveFormatEx* wave = stream_properties->GetAudioData ();
			if (wave == NULL) {
				ReportErrorOccurred ("ASFDemuxer: Couldn't find audio data in the file.");
				goto failure;
			}

			WaveFormatExtensible* wave_ex = wave->GetWaveFormatExtensible ();

			audio->SetChannels (wave->channels);
			audio->SetSampleRate (wave->samples_per_second);
			audio->SetBitRate (wave->bytes_per_second * 8);
			audio->SetBlockAlign (wave->block_alignment);
			audio->SetBitsPerSample (wave->bits_per_sample);
			audio->SetExtraData (NULL);
			audio->SetExtraDataSize (wave->codec_specific_data_size);
			audio->SetCodecId (wave->codec_id);

			if (wave_ex != NULL) {
				audio->SetBitsPerSample (wave_ex->Samples.valid_bits_per_sample);
				audio->SetCodecId (*((guint32*) &wave_ex->sub_format));
			}

			/* Fill in any extra codec data */
			if (audio->GetExtraDataSize () > 0) {
				audio->SetExtraData (g_malloc0 (audio->GetExtraDataSize ()));
				memcpy (audio->GetExtraData (), wave->GetCodecSpecificData (), audio->GetExtraDataSize ());
			}
			aesp = GetExtendedStreamProperties (current_stream);
			if (aesp != NULL) {
				audio->SetPtsPerFrame (aesp->average_time_per_frame);
			}
		} else if (stream_properties->IsVideo ()) {
			VideoStream* video = new VideoStream (media);
			stream = video;

			ASFVideoStreamData *video_data = stream_properties->GetVideoData ();
			BitmapInfoHeader *bmp;
			ASFExtendedStreamProperties *aesp;

			if (video_data != NULL) {
				bmp = video_data->GetBitmapInfoHeader ();
				aesp = GetExtendedStreamProperties (current_stream);
				if (bmp != NULL) {
					video->SetWidth (bmp->image_width);
					video->SetHeight (bmp->image_height);
					video->SetBitsPerSample (bmp->bits_per_pixel);
					video->SetCodecId (bmp->compression_id);
					video->SetExtraDataSize (bmp->GetExtraDataSize ());
					if (video->GetExtraDataSize () > 0) {
						video->SetExtraData (g_malloc0 (video->GetExtraDataSize ()));
						memcpy (video->GetExtraData (), bmp->GetExtraData (), video->GetExtraDataSize ());
					} else {
						video->SetExtraData (NULL);
					}
				}
				if (aesp != NULL) {
					video->SetBitRate (aesp->data_bitrate);
					video->SetPtsPerFrame (aesp->average_time_per_frame);
				} else {
					video->SetBitRate (video->GetWidth () * video->GetHeight ());
					video->SetPtsPerFrame (0);
				} 
			}
		} else if (stream_properties->IsCommand ()) {
			MarkerStream* marker = new MarkerStream (media);
			stream = marker;
			stream->SetCodecId (CODEC_ASF_MARKER);
		} else {
			// Unknown stream, don't include it in the count since it's NULL
			stream_count--;
			// also adjust indexes so we don't create a hole in the streams array
			count--;
			i--;
		}

		if (stream != NULL) {
			streams [i] = stream;
			stream->SetIndex (i);
			stream->SetDuration (GetFileProperties ()->play_duration - MilliSeconds_ToPts (GetPreroll ()));
			stream_to_asf_index [i] = current_stream;
		}
		
		current_stream++;
	}

	SetStreams (streams, stream_count);

	for (int i = 0; i < stream_count; i++)
		streams [i]->unref ();

	this->stream_to_asf_index = stream_to_asf_index;

	ReadMarkers ();

	media->unref ();

	ReportOpenDemuxerCompleted ();

	return;

failure:
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;

	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++) {
			if (streams [i] != NULL) {
				streams [i]->unref ();
				streams [i] = NULL;
			}
		}
		g_free (streams);
		streams = NULL;
	}

	media->unref ();
}

bool
ASFDemuxer::Eof ()
{
	return next_packet_index != G_MAXUINT64 && next_packet_index >= GetPacketCount ();
}

IMediaStream *
ASFDemuxer::GetStreamOfASFIndex (guint32 asf_index)
{
	g_return_val_if_fail (stream_to_asf_index != NULL, NULL);

	for (guint32 i = 0; i < GetStreamCount (); i++) {
		if (stream_to_asf_index [i] == asf_index)
			return GetStream (i);
	}
	return NULL;
}

MediaResult
ASFDemuxer::DeliverDataCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	ASFDemuxer *demuxer = (ASFDemuxer *) closure->GetContext ();
	demuxer->DeliverData (closure->GetOffset (), closure->GetData ());
	return MEDIA_SUCCESS;
}

void
ASFDemuxer::DeliverData (gint64 offset, MemoryBuffer *stream)
{
	guint64 packet_index;

	LOG_ASF ("ASFDemuxer::DeliverData (%" G_GINT64_FORMAT ", %" G_GINT64_FORMAT " bytes)\n", offset, stream->GetSize ());
	VERIFY_MEDIA_THREAD;

	if (stream->GetSize () == 0) {
		IMediaStream *pending_stream = GetPendingStream ();
		LOG_ASF ("ASFDemuxer::DeliverData (): reporting end of stream, pending_stream: %s\n", pending_stream ? pending_stream->GetTypeName () : NULL);
		if (pending_stream != NULL) {
			ReportGetFrameCompleted (NULL);
		}
		return;
	}

	packet_index = GetPacketIndex (offset);
	if (first_packet_index == G_MAXUINT64) {
		first_packet_index = packet_index;
	}
	last_packet_index = packet_index;
	if (next_packet_index == pending_packet_index) {
		next_packet_index++;
	}
	pending_packet_index = G_MAXUINT64;

	ASFPacket *packet = new ASFPacket (this, stream, offset);
	if (packet->Read ()) {
		DeliverPacket (packet);
	} else {
		ReportErrorOccurred ("ASFDemuxer: error while parsing packet.");
	}
	packet->unref ();
}

void
ASFDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	ASFFrameReader *frame_reader = NULL;
	MediaFrame *frame;

	LOG_ASF ("ASFDemuxer::GetFrameAsyncInternal (%s)\n", stream->GetTypeName ());

	frame_reader = GetFrameReader (stream_to_asf_index [stream->GetIndex ()]);

	g_return_if_fail (frame_reader != NULL);

	if (!frame_reader->IsFrameAvailable (NULL, NULL)) {
		LOG_ASF ("ASFDemuxer::GetFrameAsyncInternal (): next frame isn't available yet from %s (eof: %i playlistentry eof: %i), requesting more data (next packet index: %" G_GUINT64_FORMAT " GetPacketCount: %" G_GUINT64_FORMAT ").\n", 
			source->GetTypeName (), source->Eof (), playlist_entry ? playlist_entry->Eof () : -1, next_packet_index, GetPacketCount ());

		if (playlist_entry != NULL && playlist_entry->Eof ()) {
			LOG_ASF ("ASFDemuxer::GetFrameAsyncInternal (): playlist entry eof.\n");
			ReportGetFrameCompleted (NULL);
			return;
		}

		if (next_packet_index != G_MAXUINT64 && next_packet_index >= GetPacketCount ()) {
			LOG_ASF ("ASFDemuxer::GetFrameAsyncInternal (): eof, next_packet_index: %u packet count: %u\n", (int) next_packet_index, (int) GetPacketCount ());
			ReportGetFrameCompleted (NULL);
			return;
		}

		if (source->GetObjectType () == Type::MMSSOURCE && source->Eof ()) {
			ReportGetFrameCompleted (NULL);
			return;
		}

		if (source->GetObjectType () != Type::MMSSOURCE && next_packet_index == G_MAXUINT64) {
#if SANITY
			printf ("ASFDemuxer::GetFrameAsyncInternal (): a seek is probably pending.\n");
#endif
			ReportGetFrameCompleted (NULL);
			return;
		}

		RequestMorePayloadData ();
		return;
	}

	frame_reader->Advance ();
	frame = new MediaFrame (stream);	
	frame->pts = frame_reader->Pts ();
	if (frame_reader->IsKeyFrame ())
		frame->AddState (MediaFrameKeyFrame);

	if (!frame->AllocateBuffer (frame_reader->Size ())) {
		goto cleanup;
	}

	LOG_ASF ("ASFDemuxer::GetFrameAsyncInternal (): Got frame: %s pts: %" G_GUINT64_FORMAT " ms IsKeyFrame: %i buflen: %u\n", 
		stream->GetTypeName (), MilliSeconds_FromPts (frame->pts), frame->IsKeyFrame (), frame->GetBufLen ());

	if (!frame_reader->Write (frame->GetBuffer ())) {
		ReportErrorOccurred ("Error while copying the next frame.");
		goto cleanup;
	}

	frame->AddState (MediaFrameDemuxed);

	ReportGetFrameCompleted (frame);

cleanup:
	frame->unref ();
}

/*
 * ASFMarkerDecoder
 */

ASFMarkerDecoder::ASFMarkerDecoder (Media *media, IMediaStream *stream)
	: IMediaDecoder (Type::ASFMARKERDECODER, media, stream)
{
}

void
ASFMarkerDecoder::OpenDecoderAsyncInternal ()
{
	ReportOpenDecoderCompleted ();
}

void
ASFMarkerDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	LOG_ASF ("ASFMarkerDecoder::DecodeFrame ()\n");
	
	char *text;
	char *type;
	gunichar2 *data;
	gunichar2 *uni_type = NULL;
	gunichar2 *uni_text = NULL;
	int text_length = 0;
	int type_length = 0;
	guint32 size = 0;
	
	if (frame->GetBufLen () % 2 != 0 || frame->GetBufLen () == 0 || frame->GetBuffer () == NULL) {
		char *str = g_strdup_printf ("Invalid buflen (%i) found in ASFMarkerDecoder", frame->GetBufLen ());
		ReportErrorOccurred (str);
		g_free (str);
		return;
	}

	data = (gunichar2 *) frame->GetBuffer ();
	uni_type = data;
	size = frame->GetBufLen ();
	
	// the data is two arrays of WCHARs (type and text), null terminated.
	// loop through the data, counting characters and null characters
	// there should be at least two null characters.
	int null_count = 0;
	
	for (guint32 i = 0; i < (size / sizeof (gunichar2)); i++) {
		if (uni_text == NULL) {
			type_length++;
		} else {
			text_length++;
		}
		if (*(data + i) == 0) {
			null_count++;
			if (uni_text == NULL) {
				uni_text = data + i + 1;
			} else {
				break; // Found at least two nulls
			}
		}
	}

	if (null_count < 2) {
		ReportErrorOccurred ("Invalid marker found in ASFMarkerDecoder");
		return;
	}

	text = g_utf16_to_utf8 ((gunichar2 *) uni_text, text_length, NULL, NULL, NULL);
	type = g_utf16_to_utf8 ((gunichar2 *) uni_type, type_length, NULL, NULL, NULL);
	
	LOG_ASF ("ASFMarkerDecoder::DecodeFrame (): sending script command type: '%s', text: '%s', pts: '%" G_GUINT64_FORMAT "'.\n", type, text, frame->pts);

	frame->marker = new MediaMarker (type, text, frame->pts);
	
	g_free (text);
	g_free (type);
	
	ReportDecodeFrameCompleted (frame);
}

/*
 * ASFMarkerDecoderInfo
 */

IMediaDecoder *
ASFMarkerDecoderInfo::Create (Media *media, IMediaStream *stream)
{
	return new ASFMarkerDecoder (media, stream);
}	

bool 
ASFMarkerDecoderInfo::Supports (const char *codec)
{
	return !strcmp (codec, "asf-marker");
}

const char *
ASFMarkerDecoderInfo::GetName ()
{
	return "ASFMarkerDecoder";
}

/*
 * ASFDemuxerInfo
 */

MediaResult
ASFDemuxerInfo::Supports (MemoryBuffer *source)
{
	ASFGuid guid;
	bool result;

	LOG_ASF ("ASFDemuxerInfo::Supports ()\n");

	if (!guid.Read (source)) {
		fprintf (stderr, "ASFDemuxerInfo::Supports (): Reading initial guid failed.\n");
		return MEDIA_FAIL;
	}
	
	result = asf_guids_header == guid;
	
	LOG_ASF ("ASFDemuxerInfo::Supports (): probing result: %s (%s = %s)\n", result ? "true" : "false", asf_guids_header.ToString (), guid.ToString ());

	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

IMediaDemuxer *
ASFDemuxerInfo::Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
{
	return new ASFDemuxer (media, source, initial_buffer);
}

/*
 * ContentDescriptionList
 */

bool
ContentDescriptionList::Parse (const char *input, gint32 length)
{
	bool result = false;
	char *str;
	char *duped;
	int str_length = length;
	char *end;

	//LOG_MMS ("ContentDescriptionList::Parse ('%*s', %i)\n", (int) length, input, length);

	// our input may contain embedded nulls or it may not have nulls at all
	// (not even one at the end).
	// since we use string parsing functions on the input, add a null at the 
	// end to not overrun anything.

	str = (char *) g_malloc (str_length + 1);
	memcpy (str, input, str_length);
	str [str_length] = 0; // null terminate
	duped = str; // save a copy of the allocated memory to free it later
	end = str + str_length;

	/*
	 * The format is:
	 *  <name length>,<name>,<value type>,<value length>,<value>
	 */

	char *str_name_length;
	char *str_name;
	char *str_value_type;
	char *str_value_length;
	void *value;
	char *comma;

	gint64 name_length;
	gint64 value_type;
	gint64 value_length;

	do {
		// name length
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_name_length = str;
		str = comma + 1;

		name_length = strtoull (str_name_length, NULL, 10);

		if (name_length < 0 || name_length > G_MAXINT32)
			goto cleanup;

		if (end - str < name_length + 1)
			goto cleanup;

		// name
		str_name = str;
		str_name [name_length] = 0; // null terminate
		str += name_length + 1;

		// value type
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_value_type = str;
		str = comma + 1;

		value_type = strtoull (str_value_type, NULL, 10);

		if (value_type < 0 || value_type > G_MAXINT32)
			goto cleanup;

		// value length
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_value_length = str;
		str = comma + 1;

		value_length = strtoull (str_value_length, NULL, 10);

		if (value_length < 0 || value_length > G_MAXINT32)
			goto cleanup;

		if (end - str < value_length)
			goto cleanup;

		value = str; // can't null terminate, we don't necessarily have a string

		str += value_length;

		ContentDescription *cd = new ContentDescription ();
		cd->name = g_strndup (str_name, name_length);
		cd->value_type = (ContentDescription::ValueType) value_type;
		cd->value = g_malloc (value_length + 1);
		memcpy (cd->value, value, value_length);
		((char *) cd->value) [value_length] = 0;
		cd->value_length = value_length;
		list.Append (cd);

		// printf ("parsed: %*s = %*s\n", (int) name_length, str_name, (int) cd->value_length, (char *) cd->value);

		// trailing commas
		if (*str == ',') {
			str++;
		} else {
			break;
		}
	} while (str < end);

	result = true;

cleanup:

	g_free (duped);

	return result;
}

/*
 * ContentDescription
 */
 
ContentDescription::~ContentDescription ()
{
	g_free (name);
	g_free (value);
}

/*
 * MmsRequestNode
 */

class MmsRequestNode : public List::Node {
public:
	HttpRequest *request;
	MmsRequestNode (HttpRequest *request) { this->request = request; this->request->ref (); }
	virtual ~MmsRequestNode () { request->unref (); }
};

/*
 * MmsSource
 */
 
MmsSource::MmsSource (Media *media, const Uri *uri)
	: IMediaSource (Type::MMSSOURCE, media)
{
	finished = false;
	is_sspl = false;
	failure_reported = false;
	max_bitrate = 0;
	this->uri = Uri::Clone (uri);
	request_uri = NULL;
	client_id = NULL;
	request = NULL;
	current = NULL;
	demuxer = NULL;
	buffer = NULL;
	buffer_size = 0;
	buffer_used = 0;
	waiting_state = MmsInitialization;
	requested_pts = G_MAXUINT64;
	temporary_downloaders = NULL;
	is_disposing = false;
	open_demuxer_state = 0;

	p_packet_count = 0;
	p_packet_times [0] = 0;
	p_packet_times [1] = 0;
	p_packet_times [2] = 0;
	p_packet_sizes [0] = 0;
	p_packet_sizes [1] = 0;
	p_packet_sizes [2] = 0;

	/* Replace mms|rtsp|rtsps with http to not confuse lower layers */
	request_uri = Uri::CloneWithScheme (uri, "http");
}

MmsSource::~MmsSource ()
{
	delete uri;
	delete request_uri;
	delete temporary_downloaders;
}

void
MmsSource::Dispose ()
{
	// thread safe method
	MmsPlaylistEntry *current;
	IMediaDemuxer *demuxer;
	bool delete_tmp_downloaders;

	// don't lock during unref, only while nulling out the local field
	Lock ();
	current = this->current;
	this->current = NULL;
	demuxer = this->demuxer;
	this->demuxer = NULL;
	delete_tmp_downloaders = this->temporary_downloaders != NULL || this->request != NULL;
	is_disposing = true;
	Unlock ();

	if (current) {
		current->unref ();
		current = NULL;
	}

	if (demuxer) {
		demuxer->unref ();
		demuxer = NULL;
	}

	if (delete_tmp_downloaders) {
		if (Surface::InMainThread ()) {
			DeleteTemporaryDownloaders (this);
		} else {
			/* We can't touch the downloaders / request on any thread but the main one */
			Resurrect (); /* Don't call ref, since that will cause a warning to be printed */
			AddTickCall (DeleteTemporaryDownloaders);
			unref (); /* The tick call still has a ref */
		}
	}

	IMediaSource::Dispose ();
}

void
MmsSource::DeleteTemporaryDownloaders (EventObject *obj)
{
	List *temporary_downloaders;
	MmsRequestNode *node;
	HttpRequest *request;

	MmsSource *src = (MmsSource *) obj;
	src->Lock ();
	temporary_downloaders = src->temporary_downloaders;
	src->temporary_downloaders = NULL;
	request = src->request;
	src->request = NULL;
	src->Unlock ();

	if (temporary_downloaders != NULL) {
		node = (MmsRequestNode *) temporary_downloaders->First ();
		while (node != NULL) {
			node->request->RemoveAllHandlers (obj);
			node->request->Abort ();
			node = (MmsRequestNode *) node->next;
		}
		delete temporary_downloaders;
		temporary_downloaders = NULL;
	}
	if (request) {
		request->RemoveAllHandlers (obj);
		request->Abort ();
		request->unref ();
		request = NULL;
	}
}

MediaResult
MmsSource::Initialize ()
{
	VERIFY_MAIN_THREAD;

	AddTickCall (SendDescribeRequestCallback);

	return MEDIA_SUCCESS;
}

void
MmsSource::CreateDownloaders (const char *method, HttpRequest **req)
{
	CreateDownloaders (method, req, (HttpRequest::Options) 0);
}

void
MmsSource::CreateDownloaders (const char *method, HttpRequest **req, HttpRequest::Options additional_options)
{
	/* Thread-safe since it doesn't touch any instance fields */
	HttpRequest *request;
	char *id;

	*req = NULL;

	if (IsDisposed ())
		return;

	request = GetDeployment ()->CreateHttpRequest ((HttpRequest::Options) (HttpRequest::CustomHeaders | HttpRequest::DisableCache | HttpRequest::DisableFileStorage | additional_options));
	*req = request;
	if (request == NULL) {
		ReportErrorOccurred ("Couldn't create httprequest.");
		goto cleanup;
	}

	request->AddHandler (HttpRequest::StartedEvent, StartedCallback, this);
	request->AddHandler (HttpRequest::StoppedEvent, StoppedCallback, this);
	request->AddHandler (HttpRequest::WriteEvent, WriteCallback, this);

	request->Open (method, request_uri, NULL, StreamingPolicy);
	request->SetHeader ("User-Agent", CLIENT_USER_AGENT, false);
	request->SetHeader ("Pragma", "no-cache", true);
	request->SetHeader ("Pragma", "xClientGUID=" CLIENT_GUID, true);
	request->SetHeader ("Supported", CLIENT_SUPPORTED, false);
	id = GetCurrentPlaylistGenId ();
	if (id != NULL) {
		request->SetHeaderFormatted ("Pragma", g_strdup_printf ("playlist-gen-id=%s", id), true);
		g_free (id);
	}
	id = GetClientId ();
	if (id != NULL) {
		request->SetHeaderFormatted ("Pragma", g_strdup_printf ("client-id=%s", id), true);
		g_free (id);
	}

cleanup:
	;
}

void
MmsSource::CreateDownloaders (const char *method)
{
	HttpRequest *request = NULL;;

	LOG_MMS ("MmsSource::CreateDownloaders ('%s')\n", method);
	VERIFY_MAIN_THREAD;

	Lock ();
	request = this->request;
	this->request = NULL;
	Unlock ();

	/* Clean up any old downloaders */
	if (request != NULL) {
		request->RemoveAllHandlers (this);
		request->Abort ();
		request->unref ();
		request = NULL;
	}

	/* Clean up old buffers */
	g_free (buffer);
	buffer = NULL;
	buffer_size = 0;
	buffer_used = 0;

	CreateDownloaders (method, &request);

	g_return_if_fail (request != NULL);

	Lock ();
	this->request = request;
	Unlock ();
}

void
MmsSource::SendDescribeRequest ()
{
	LOG_MMS ("MmsSource::SendDescribeRequest () uri: %s request_uri: %s state: %i\n", uri->GetOriginalString (), request_uri->GetOriginalString (), waiting_state);
	VERIFY_MAIN_THREAD;
	HttpRequest *request;

	if (IsDisposed ())
		return;

	g_return_if_fail (waiting_state == MmsInitialization);

	waiting_state = MmsDescribeResponse;

	CreateDownloaders ("GET");

	request = GetRequestReffed ();
	g_return_if_fail (request != NULL);

	request->SetHeader ("Pragma", "packet-pair-experiment=1", true);
	request->Send ();
	request->unref ();
}

void
MmsSource::SendPlayRequest ()
{
	HttpRequest *request;
	guint64 pts;

	LOG_MMS ("MmsSource::SendPlayRequest () uri: %s request_uri: %s waiting_state: %i\n", uri->GetOriginalString (), request_uri->GetOriginalString (), waiting_state);
	VERIFY_MAIN_THREAD;

	switch (waiting_state) {
	case MmsInitialization:
	case MmsDescribeResponse:
		/* Can't play yet */
		LOG_MMS ("MmsSource::SendPlayRequest (): can't play yet, waiting for describe response\n");
		return;
	default:
		break;
	}

	waiting_state = MmsPlayResponse;

	Lock ();
	pts = requested_pts;
	requested_pts = G_MAXUINT64;
	Unlock ();

	if (pts == G_MAXUINT64)
		pts = 0;

	LOG_MMS ("MmsSource::SendPlayRequest () pts: %" G_GUINT64_FORMAT " ms\n", MilliSeconds_FromPts (pts));

	CreateDownloaders ("GET");

	request = GetRequestReffed ();
	g_return_if_fail (request != NULL);

	request->SetHeader ("Pragma", "rate=1.000000,stream-offset=0:0,max-duration=0", false);
	request->SetHeader ("Pragma", "xPlayStrm=1", false);
	request->SetHeader ("Pragma", "LinkBW=2147483647,rate=1.000, AccelDuration=20000, AccelBW=2147483647", false);
	request->SetHeaderFormatted ("Pragma", g_strdup_printf ("stream-time=%" G_GINT64_FORMAT ", packet-num=4294967295", pts / 10000), false);
	SetStreamSelectionHeaders (request);

	request->Send ();
	request->unref ();
}

void
MmsSource::SendSelectStreamRequest ()
{
	HttpRequest *request;
	bool is_disposing = false;

	LOG_MMS ("MmsSource::SendSelectStreamRequest ()\n");
	VERIFY_MAIN_THREAD;

	if (IsDisposed ())
		return;

	CreateDownloaders ("POST", &request);

	g_return_if_fail (this->request != request);
	g_return_if_fail (request != NULL);

	request->RemoveAllHandlers (this); /* We don't want any data from this mms downloader */
	Lock ();
	if (this->is_disposing) {
		is_disposing = true;
	} else {
		if (temporary_downloaders == NULL)
			temporary_downloaders = new List ();
		temporary_downloaders->Append (new MmsRequestNode (request));
	}
	Unlock ();

	if (!is_disposing) {
		SetStreamSelectionHeaders (request);
		request->Send ();
	}

	request->unref ();
}

void
MmsSource::SendLogRequest (MediaLog *log)
{
	HttpRequest *request = NULL;
	int length;
	char *content = NULL;
	char *content_length = NULL;
	char *player_id = NULL;
	bool is_disposing = false;

	LOG_MMS ("MmsSource::SendLogRequest ()\n");
	VERIFY_MAIN_THREAD;

	if (IsDisposed ())
		return;

	/* We need to force HTTP/1.0, since curl otherwise confuse the server by sending
	 * just the headers with an additional Expect: 100 header, expecting the server
	 * to respond to that before sending the data */
	CreateDownloaders ("POST", &request, HttpRequest::ForceHttp_1_0);

	if (request == NULL) {
		printf ("Moonlight: Could not create http request to send mms log\n");
		return;
	}

	Lock ();
	if (this->is_disposing) {
		is_disposing = true;
	} else {
		if (temporary_downloaders == NULL)
			temporary_downloaders = new List ();
		temporary_downloaders->Append (new MmsRequestNode (request));
	}
	Unlock ();
	if (is_disposing)
		goto cleanup;

	// POST /SSPLDrtOnDemandTest HTTP/1.0
	// Host: moonlightmedia
	// Content-Length: 2203
	// User-Agent: NSPlayer/11.08.0005.0000
	// Accept: * / *
	// Accept-Language: en-us, *;q=0.1
	// Connection: Keep-Alive
	// Content-Type: application/x-wms-Logstats
	// Pragma: client-id=3375607867
	// Pragma: playlist-gen-id=2769
	// Pragma: xClientGuid={00000000-0000-0000-0000-000000000000}
	// Supported: com.microsoft.wm.srvppair, com.microsoft.wm.sswitch, com.microsoft.wm.startupprofile, com.microsoft.wm.predstrm

	player_id = g_strdup_printf ("{3300AD50-2C39-46C0-AE0A-%.12X}", (guint32) g_ascii_strtoull (client_id, NULL, 10));
	log->SetPlayerId (player_id);
	log->SetUrl (request_uri->ToString ());

	content = log->GetLog (false);
	length = strlen (content);
	content_length = g_strdup_printf ("%i", length);

	request->SetHeader ("Content-Length", content_length, false);
	request->SetHeader ("Content-Type", "application/x-wms-Logstats", false);
	request->SetBody (content, length);

	request->Send ();

	LOG_MMS ("MmsSource::SendLogRequest () sent log:\n%s\n", content);

cleanup:
	g_free (content_length);
	g_free (content);
	g_free (player_id);
	request->unref ();
}

void
MmsSource::SetStreamSelectionHeaders (HttpRequest *request)
{
	MmsPlaylistEntry *entry;
	gint8 streams [128];
	int count = 0;
	GString *line;

	VERIFY_MAIN_THREAD;

	entry = GetCurrentReffed ();

#if DEBUG
	if (entry == NULL)
		printf ("MmsSource::SetStreamSelectionHeaders (): no entry. default streams will probably be selected by the server.\n");
#endif
	if (entry == NULL)
		return;

	entry->GetSelectedStreams (streams);

	line = g_string_new ("stream-switch-entry=");
	for (int i = 1; i < 128; i++) {
		switch (streams [i]) {
		case -1: // invalid stream
			break;
		case 0: // not selected
			count++;
			g_string_append_printf (line, "%i:ffff:0 ", i);
			break;
		case 1: // selected
			count++;
			g_string_append_printf (line, "ffff:%i:0 ", i);
			break;
		default: // ?
			printf ("MmsDownloader: invalid stream selection value (%i).\n", streams [i]);
			break;
		}
	}

	/* stream-switch-count && stream-switch-entry need to be on their own pragma lines */
	request->SetHeader ("Pragma", line->str, true);
	request->SetHeaderFormatted ("Pragma", g_strdup_printf ("stream-switch-count=%i", count), true);

	g_string_free (line, true);
	entry->unref ();
}

void
MmsSource::ProcessResponseHeader (const char *header, const char *value)
{
	HttpRequest *request = NULL;
	char *h = NULL;
	char *duped = NULL;

	VERIFY_MAIN_THREAD;
	LOG_MMS ("MmsSource::ProcessResponseHeader ('%s', '%s')\n", header, value);

	if (failure_reported)
		return;

	request = GetRequestReffed ();
	if (request == NULL) {
		/* Probably disposed */
		return;
	}

	// check response code
	HttpResponse *response = request->GetResponse ();
	if (response != NULL && response->GetResponseStatus () != 200) {
		fprintf (stderr, "Moonlight: The MmsDownloader could not load the uri '%s', got response status: %i (expected 200)\n", uri->GetOriginalString (), response->GetResponseStatus ());
		ReportDownloadFailure ();
		goto cleanup;
	}

	g_return_if_fail (header != NULL);
	g_return_if_fail (value != NULL);

	// we're only interested in the 'Pragma' header(s)

	if (strcmp (header, "Pragma") != 0)
		goto cleanup;

	h = g_strdup (value);
	duped = h;

	while (h != NULL && *h != 0) {
		char *key = NULL;
		char *val = NULL;
		char c;
		char *left;

		key = parse_rfc_1945_token (h, &c, &left);

		if (key == NULL)
			break;

		h = left;

		if (key [0] == 0)
			continue;

		if (c == '=' && h != NULL) {
			if (*h == '"') {
				val = parse_rfc_1945_quoted_string (h + 1, &c, &left);
				h = left;
			} else if (*h != 0) {
				val = parse_rfc_1945_token (h, &c, &left);
				h = left;
			}
		}

		// printf ("MmsDownloader::ResponseHeader (). processing 'Pragma', key='%s', value='%s'\n", key, val);

		if (strcmp (key, "client-id") == 0) {
			Lock ();
			if (client_id != NULL)
				g_free (client_id);
			client_id = g_strdup (val);
			Unlock ();
		}
	}

cleanup:
	if (request != NULL)
		request->unref ();
	g_free (duped);
}

HttpRequest *
MmsSource::GetRequestReffed ()
{
	HttpRequest *result;
	Lock ();
	result = request;
	if (result != NULL)
		result->ref ();
	Unlock ();
	return result;
}

void
MmsSource::Write (void *buf, gint32 n)
{
	LOG_MMS_EX ("MmsSource::Write (%p, %i)\n", buf, n);
	VERIFY_MAIN_THREAD;

	MmsHeader *header;
	MmsPacket *packet;
	char *payload;
	guint32 offset = 0;
	bool is_valid = false;
	HttpRequest *request;

	// Make sure our internal buffer is big enough
	if (buffer_size < buffer_used + n) {
		buffer_size = buffer_used + n;
		buffer = (char *) g_realloc (buffer, buffer_size);
	}

	// Populate the data into the buffer
	memcpy (buffer + buffer_used, buf, n);
	buffer_used += n;

	// Check  we have an entire packet available.
	while (buffer_used >= sizeof (MmsHeader)) {
		header = (MmsHeader *) buffer;

		switch (header->id) {
		case MMS_DATA:
		case MMS_HEADER:
		case MMS_METADATA:
		case MMS_STREAM_C:
		case MMS_END:
		case MMS_PAIR_P:
			is_valid = true;
			break;
		default:
			is_valid = false;
			break;
		}

		if (!is_valid) {
			LOG_MMS ("MmsDownloader::Write (): invalid mms header: %i = %c\n%*s\n", header->id, header->id, n, (char *) buf);
			request = GetRequestReffed ();
			if (request != NULL) {
				request->Abort ();
				request->unref ();
			}
			return;
		}

		if (buffer_used < (header->length + sizeof (MmsHeader)))
			return;

		packet = (MmsPacket *) (buffer + sizeof (MmsHeader));
		payload = (buffer + sizeof (MmsHeader) + sizeof (MmsDataPacket));

		if (!ProcessPacket (header, packet, payload, &offset)) {
			LOG_MMS ("MmsDownloader::Write (): packet processing failed\n");
			break;
		}

		if (buffer_used > offset) {
			memmove (buffer, buffer + offset, buffer_used - offset);
			buffer_used -= offset;
		} else {
			buffer_used = 0;
		}
	}
}

bool
MmsSource::ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS_EX ("MmsSource::ProcessPacket (%p, %p, %p, %p) '%c' length: %i\n", header, packet, payload, offset, header->id, header->length);
	
	*offset = (header->length + sizeof (MmsHeader));
 
 	switch (header->id) {
	case MMS_HEADER:
		return ProcessHeaderPacket (header, packet, payload, offset);
	case MMS_METADATA:
		return ProcessMetadataPacket (header, packet, payload, offset);
	case MMS_PAIR_P:
		return ProcessPairPacket (header, packet, payload, offset);
	case MMS_DATA:
		return ProcessDataPacket (header, packet, payload, offset);
	case MMS_END:
		return ProcessEndPacket (header, packet, payload, offset);
	case MMS_STREAM_C:
		return ProcessStreamSwitchPacket (header, packet, payload, offset);
	}

	printf ("MmsSource::ProcessPacket received a unknown packet type %i.", (int) header->id);

	return false;
}

bool
MmsSource::ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	guint64 max_bitrate;

	LOG_MMS ("MmsDownloader::ProcessPairPacket () p_packet_count: %i\n", p_packet_count);
	VERIFY_MAIN_THREAD;
	
	if (p_packet_times [p_packet_count] == 0)
		p_packet_times [p_packet_count] = get_now ();

	// NOTE: If this is the 3rd $P packet, we need to increase the size reported in the header by
	// the value in the reason field.  This is a break from the normal behaviour of MMS packets
	// so we need to guard against this occurance here and ensure we actually have enough data
	// buffered to consume
	if (p_packet_count == 2 && buffer_used < (header->length + sizeof (MmsHeader) + packet->packet.reason))
		return false;

	// NOTE: We also need to account for the size of the reason field manually with our packet massaging.
	*offset += 4;

	// NOTE: If this is the first $P packet we've seen the reason is actually the amount of data
	// that the header claims is in the payload, but is in fact not.
	if (p_packet_count == 0) {
		*offset -= packet->packet.reason;
	}

	// NOTE: If this is the third $P packet we've seen, reason is an amount of data that the packet
	// is actually larger than the advertised packet size
	if (p_packet_count == 2)
		*offset += packet->packet.reason;

	p_packet_sizes [p_packet_count] = *offset;

	++p_packet_count;

	if (p_packet_times [0] == p_packet_times [2]) {
		max_bitrate = 0; // prevent /0
	} else {
		max_bitrate = (gint64) (((p_packet_sizes [1] + p_packet_sizes [2]) * 8) / ((double) ((p_packet_times [2] - p_packet_times [0]) / (double) 10000000)));
	}

	SetMaxBitRate (max_bitrate);

	return true;
}

bool
MmsSource::ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	const char *playlist_gen_id = NULL;
	const char *broadcast_id = NULL;
	HttpStreamingFeatures features = HttpStreamingFeaturesNone;

	char *start = payload;
	char *key = NULL, *value = NULL;
	char *state = NULL;

	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (%p, %p, %s, %p)\n", header, packet, payload, offset);

	VERIFY_MAIN_THREAD;


	// format: key=value,key=value\0
	// example:
	// playlist-gen-id=1,broadcast-id=2,features="broadcast,seekable"\0
	
	// Make sure payload is null-terminated
	for (int i = 0; i < packet->packet.data.size; i++) {
		if (payload [i] == 0)
			break;
		if (i == packet->packet.data.size - 1)
			payload [i] = 0;
	}

#if 0
	// content description list
	ContentDescriptionList *content_descriptions = NULL;
	int payload_strlen = strlen (payload);
	const char *cdl_start = NULL;
	int cdl_length;

	if (packet->packet.data.size > payload_strlen + 1) {
		cdl_start = payload + payload_strlen + 1;
		cdl_length = packet->packet.data.size - payload_strlen - 2;

		// parse content description list
		content_descriptions = new ContentDescriptionList ();
		if (!content_descriptions->Parse (cdl_start, cdl_length)) {
			delete content_descriptions;
			content_descriptions = NULL;
		}
	}
	delete content_descriptions;
#endif

	do {
		key = strtok_r (start, "=", &state);
		start = NULL;
		
		if (key == NULL)
			break;
			
		if (key [0] == ' ')
			key++;
		
		if (!strcmp (key, "features")) {
			value = strtok_r (NULL, "\"", &state);
		} else {
			value = strtok_r (NULL, ",", &state);
		}
		
		if (value == NULL)
			break;
			
		LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): %s=%s\n", key, value);
		
		if (!strcmp (key, "playlist-gen-id")) {
			playlist_gen_id = value;
		} else if (!strcmp (key, "broadcast-id")) {
			broadcast_id = value;
		} else if (!strcmp (key, "features")) {
			features = parse_http_streaming_features (value);
		} else {
			LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): Unexpected metadata: %s=%s\n", key, value);
		}
	} while (true);

	SetMmsMetadata (playlist_gen_id, broadcast_id, features);

	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): playlist_gen_id: '%s', broadcast_id: '%s', features: %i\n", playlist_gen_id, broadcast_id, features);

	return true;
}

bool
MmsSource::ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	MmsPlaylistEntry *entry;
	Media *media;

	LOG_MMS ("MmsSource::ProcessHeaderPacket () state: %i\n", waiting_state);
	VERIFY_MAIN_THREAD;

	entry = CreateCurrentEntry ();

	g_return_val_if_fail (entry != NULL, false);

	if (!entry->IsHeaderParsed ()) {
		media = entry->GetMediaReffed ();
		if (media != NULL) {
			media->AddSafeHandler (Media::MediaErrorEvent, MediaErrorCallback, this);
			media->unref ();
			entry->ParseHeaderAsync (payload, header->length - sizeof (MmsDataPacket), this, OpenedCallback);
		} else {
			/* Disposed? don't do anything */
		}
	} else {
		// We've already parsed this header (in the Describe request).
		// TODO: handle the xResetStream when the playlist changes
		// TODO: can this be another header??
	}

	entry->unref ();

	return true;
}

bool
MmsSource::ProcessDataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	WritePacket (payload, header->length - sizeof (MmsDataPacket));
	
	return true;
}

bool
MmsSource::ProcessStreamSwitchPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsSource::ProcessStreamSwitchPacket ()\n");
	VERIFY_MAIN_THREAD;

	MmsHeaderReason *hr = (MmsHeaderReason *) header;

	ReportStreamChange (hr->reason);
	waiting_state = MmsStreamSwitchResponse;

	return true;
}

bool
MmsSource::ProcessEndPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessEndPacket ()\n");
	VERIFY_MAIN_THREAD;

	MmsHeaderReason *hr = (MmsHeaderReason *) header;

	NotifyFinished (hr->reason);

	// TODO: send log

	return true;
}

bool
MmsSource::OpenMmsDemuxerCalled ()
{
	/* thread-safe */
	bool result = false;
	Lock ();
	if (open_demuxer_state == 1) {
		/* MmsSource::OpenedHandler has been called */
		result = true;
		open_demuxer_state |= 4; // MmsDemuxer::ReportOpenDemuxerCompleted called (caller will call it upon return)
	}
	open_demuxer_state |= 2; // MmsDemuxer::OpenDemuxerAsync called
	Unlock ();
	return result;
}

void
MmsSource::OpenedHandler (IMediaDemuxer *demuxer, EventArgs *args)
{
	MmsDemuxer *mms_demuxer;
	bool report_open = false;


	LOG_MMS ("MmsSource::OpenedHandler () state: %i\n", waiting_state);
	VERIFY_MAIN_THREAD;

	mms_demuxer = GetDemuxerReffed ();
	Lock ();
	if (open_demuxer_state == 2) { // MmsDemuxer::OpenDemuxerAsync called
		if (mms_demuxer != NULL) {
			open_demuxer_state |= 4;
			report_open = true;
		}
	}
	open_demuxer_state |= 1; // MmsSource::OpenedHandler called
	Unlock ();
	if (mms_demuxer) {
		if (report_open) {
			LOG_MMS ("MmsSource::OpenedHandler (): the mms demuxer has already been requested to open, call ReportOpenDemuxerCompleted\n");
			mms_demuxer->ReportOpenDemuxerCompleted ();
		}
		mms_demuxer->unref ();
	}
	
	switch (waiting_state) {
	case MmsDescribeResponse:
		waiting_state = MmsInitialSeek;
		if (!CanSeek ()) {
			/* We've gotten the describe response now, and parsed it successfully. Time to start playing */
			SendPlayRequest ();
		} else {
			/* The Media will seek to the start of the media, which will end up with a play request
			 * This might have happened already, and SendPlayRequest didn't do anything (too early),
			 * so check if we need to call SendPlayRequest again now that we can */
			bool seek_pending = false;
			Lock ();
			seek_pending = requested_pts != G_MAXUINT64;
			Unlock ();
			if (seek_pending)
				SendPlayRequest ();
		}
		break;
	case MmsStreamSwitchResponse:
		SendSelectStreamRequest ();
		break;
	default:
		break;
	}
}

void
MmsSource::MediaErrorHandler (Media *media, EventArgs *args)
{
	LOG_MMS ("MmsSource::MediaErrorHandler ()\n");
	/* Anything to do here? */
}

MmsDemuxer *
MmsSource::GetDemuxerReffed ()
{
	MmsDemuxer *result;
	Lock ();
	result = demuxer;
	if (result)
		result->ref ();
	Unlock ();
	return result;
}

MmsPlaylistEntry *
MmsSource::GetCurrentReffed ()
{
	MmsPlaylistEntry *result;
	
	// Thread safe
	
	Lock ();
	result = current;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

void
MmsSource::SetMaxBitRate (guint64 value)
{
	LOG_MMS ("MmsSource::SetMaxBitRate (%" G_GUINT64_FORMAT ")\n", value);
	Lock ();
	max_bitrate = value;
	Unlock ();
}

guint64
MmsSource::GetMaxBitRate ()
{
	guint64 result;
	Lock ();
	result = max_bitrate;
	Unlock ();
	return result;
}

void
MmsSource::ReadAsyncInternal (MediaReadClosure *closure)
{
	/* Nothing to do here, we push data when we get it */
}

void
MmsSource::ReportDownloadFailure ()
{
	Media *media;
	
	LOG_MMS ("MmsSource::ReportDownloadFailure ()\n");
	VERIFY_MAIN_THREAD;
	
	failure_reported = true;

	media = GetMediaReffed ();

	if (media == NULL) {
		fprintf (stderr, "Moonlight: mms downloader failed");
		return;
	}

	media->ReportErrorOccurred ("MmsDownloader failed");
	media->unref ();
}

void
MmsSource::ReportStreamChange (gint32 reason)
{
	LOG_MMS ("MmsSource::ReportStreamChange (reason: %i)\n", reason);
	VERIFY_MAIN_THREAD;

	NotifyFinished (1 /* the current playlist entry has finished, but more will come */);
}

MmsPlaylistEntry *
MmsSource::CreateCurrentEntry ()
{
	Media *media;
	Media *entry_media;
	PlaylistEntry *parent;
	MmsPlaylistEntry *result = NULL;

	LOG_MMS ("MmsSource::CreateCurrentEntry (): current: %p\n", current);

	media = GetMediaReffed ();
	if (media == NULL)
		goto cleanup;

	parent = media->GetPlaylistEntry ();
	if (parent == NULL)
		goto cleanup;

	Lock ();
	if (current == NULL) {
		entry_media = new Media (parent);
		entry_media->SetSeekWhenOpened (false);
		this->current = new MmsPlaylistEntry (entry_media, this);
		entry_media->unref ();
	}
	result = this->current;
	result->ref ();
	Unlock ();

cleanup:
	if (media != NULL)
		media->unref ();

	LOG_MMS ("MmsSource::CreateCurrentEntry (): result: %p\n", result);

	return result;
}

void
MmsSource::SetMmsMetadata (const char *playlist_gen_id, const char *broadcast_id, HttpStreamingFeatures features)
{
	MmsPlaylistEntry *entry = NULL;
	Media *media = NULL;
	PlaylistEntry *playlist;

	LOG_MMS ("MmsSource::SetMmsMetadata ('%s', '%s', %i)\n", playlist_gen_id, broadcast_id, (int) features);

	VERIFY_MAIN_THREAD;

	media = GetMediaReffed ();
	if (media == NULL)
		goto cleanup;

	playlist = media->GetPlaylistEntry ();
	if (playlist == NULL)
		goto cleanup;

	entry = CreateCurrentEntry ();
	if (entry == NULL)
		goto cleanup;

	entry->SetPlaylistGenId (playlist_gen_id);
	entry->SetBroadcastId (broadcast_id);
	entry->SetHttpStreamingFeatures (features);

	if (features & HttpStreamingPlaylist) {
		is_sspl = true;
		playlist->SetIsDynamic ();
	}

	LOG_MMS ("MmsSource::SetMmsMetadata ('%s', '%s', %i)\n", playlist_gen_id, broadcast_id, (int) features);

cleanup:
	if (media != NULL)
		media->unref ();
	if (entry != NULL)
		entry->unref ();
}

bool
MmsSource::RemoveTemporaryDownloader (HttpRequest *request)
{
	MmsRequestNode *node;
	MmsRequestNode *found = NULL;

	VERIFY_MAIN_THREAD;

	Lock ();
	if (temporary_downloaders != NULL) {
		node = (MmsRequestNode *) temporary_downloaders->First ();
		while (node != NULL) {
			if (node->request == request) {
				found = node;
				temporary_downloaders->Unlink (node);
				break;
			}
			node = (MmsRequestNode *) node->next;
		}
	}
	Unlock ();

	/* Don't delete the node/unref the dl in a lock */
	if (found != NULL) {
		request->RemoveAllHandlers (this);
		delete node;
	}

	return false;
}

void
MmsSource::StartedHandler (HttpRequest *request, EventArgs *args)
{
	HttpResponse *response;
	List *headers;
	HttpHeader *header;

	LOG_MMS ("MmsSource::StartedHandler ()\n");

	VERIFY_MAIN_THREAD;

	response = request->GetResponse ();
	headers = response->GetHeaders ();
	if (headers != NULL) {
		header = (HttpHeader *) headers->First ();
		while (header != NULL) {
			ProcessResponseHeader (header->GetHeader (), header->GetValue ());
			header = (HttpHeader *) header->next;
		}
	}
}

void
MmsSource::WriteHandler (HttpRequest *request, HttpRequestWriteEventArgs *args)
{
	Write (args->GetData (), args->GetCount ());
}

void
MmsSource::StoppedHandler (HttpRequest *request, HttpRequestStoppedEventArgs *args)
{
	Media *media = GetMediaReffed ();
	bool is_temporary;

	LOG_MMS ("MmsSource::DownloadStoppedHandler () IsSuccess: %i Error message: %s\n", args->IsSuccess (), args->GetErrorMessage ());

	VERIFY_MAIN_THREAD;

	is_temporary = RemoveTemporaryDownloader (request);

	if (is_temporary || args->IsSuccess ()) {
		LOG_MMS ("MmsSource::DownloadStoppedHandler (): temporary downloader was stopped\n");
		goto cleanup; /* Nothing more to do here, don't report failures for temporary downloaders */
	}

	if (media == NULL)
		return;

	media->RetryHttp ();

cleanup:
	if (media != NULL)
		media->unref ();
}

void
MmsSource::NotifyFinished (guint32 reason)
{
	MmsPlaylistEntry *entry;
	
	VERIFY_MAIN_THREAD;
	
	LOG_MMS ("MmsSource::NotifyFinished (%i)\n", reason);
	
	switch (reason) {
	case 0xc00d2ee6:
		/* This is NS_E_SERVER_UNAVAILABLE - we've reached the maximum number of client connections to one encoder (this is a server config problem, not a moonlight problem) */
		printf ("Moonlight: Got a NS_E_SERVER_UNAVAILABLE error code (0xC00D2EE6) from the streaming server. This indicates a server configuration problem (if you're a server admin, searching for the error code on the web will find a way to fix it)\n");
		break;
	case 0: {
		// The server has finished streaming and no more 
		// Data packets will be transmitted until the next Play request
		finished = true;
		
		Media *media = GetMediaReffed ();
		Playlist *root;
		PlaylistEntry *entry;
		if (media != NULL) {
			root = media->GetPlaylistRoot ();
			if (root != NULL) {
				entry = root->GetCurrentEntryLeaf ();
				if (entry != NULL) {
					entry->SetHasDynamicEnded ();
				}
			}

			media->unref ();
		}
		/* Fall through */
	}
	case 1:
		// The server has finished streaming the current playlist entry. Other playlist
		// entries still remain to be streamed. The server will transmit a stream change packet
		// when it switches to the next entry.

		entry = GetCurrentReffed ();
		if (entry != NULL) {
			entry->NotifyFinished ();
			WritePacket (NULL, 0);
			Lock ();
			if (current != NULL) {
				current->unref ();
				current = NULL;
			}
			Unlock ();
			entry->unref ();
		}
		break;
	default:
		// ?
		break;
	}

	LOG_MMS ("MmsSource::NotifyFinished (%i): eof: %i\n", reason, Eof ());
}

bool
MmsSource::CanSeek ()
{
	/* We can only seek if we're not a server-side playlist and not a live streaming source */
	return !is_sspl && ((GetCurrentStreamingFeatures () & HttpStreamingBroadcast) == 0);
}

MediaResult
MmsSource::SeekToPts (guint64 pts)
{
	/* Thread safe */
	bool add_tick_call = false;
	
	LOG_MMS ("MmsSource::SeekToPts (%" G_GUINT64_FORMAT ") CanSeek: %i\n", pts, CanSeek ());

	if (!CanSeek ())
		return MEDIA_SUCCESS;

	/* We may get several seek requests before any of them has been processed,
	 * in which case just enqueue 1 play request for the main thread */
	Lock ();
	add_tick_call = requested_pts == G_MAXUINT64;
	requested_pts = pts;
	Unlock ();

	if (add_tick_call)
		AddTickCall (SendPlayRequestCallback);

	return MEDIA_SUCCESS;
}

IMediaDemuxer *
MmsSource::CreateDemuxer (Media *media, MemoryBuffer *initial_buffer)
{
	// thread safe
	MmsDemuxer *result = NULL;
	
	g_return_val_if_fail (demuxer == NULL, NULL);
	
	Lock ();
	if (demuxer == NULL) {
		result = new MmsDemuxer (media, this);
		demuxer = result;
		demuxer->ref ();
	}
	Unlock ();
	
	return result;
}

void
MmsSource::WritePacket (void *buf, gint32 n)
{
	MmsPlaylistEntry *entry;
	
	VERIFY_MAIN_THREAD;
	
	entry = GetCurrentReffed ();
	
	g_return_if_fail (entry != NULL);
	
	entry->WritePacket (buf, n);
	entry->unref ();
}

bool
MmsSource::Eof ()
{
	// thread safe
	MmsPlaylistEntry *entry;
	bool result;
	
	if (!finished)
		return false;
	
	entry = GetCurrentReffed ();
	 	
	if (entry == NULL) {
	 	result = true;
	 } else {
		result = entry->Eof ();
		entry->unref ();
	}
	
	return result;
}

char *
MmsSource::GetCurrentPlaylistGenId ()
{
	char *result = NULL;
	MmsPlaylistEntry *entry;

	entry = GetCurrentReffed ();
	if (entry != NULL) {
		result = entry->GetPlaylistGenId ();
		entry->unref ();
	}

	return result;
}

char *
MmsSource::GetCurrentBroadcastId ()
{
	char *result = NULL;
	MmsPlaylistEntry *entry;

	entry = GetCurrentReffed ();
	if (entry != NULL) {
		result = entry->GetBroadcastId ();
		entry->unref ();
	}

	return result;
}

HttpStreamingFeatures
MmsSource::GetCurrentStreamingFeatures ()
{
	HttpStreamingFeatures result = HttpStreamingFeaturesNone;
	MmsPlaylistEntry *entry;

	entry = GetCurrentReffed ();
	if (entry != NULL) {
		result = entry->GetHttpStreamingFeatures ();
		entry->unref ();
	}

	return result;
}

char *
MmsSource::GetClientId ()
{
	char *result;
	Lock ();
	result = g_strdup_printf (client_id);
	Unlock ();
	return result;
}

/*
 * ParseHeaderClosure
 */
class ParseHeaderClosure : public MediaClosure {
public:
	MemoryBuffer *buffer;
	EventObject *opened_handler_obj;
	EventHandler opened_handler;

	ParseHeaderClosure (Media *media, MediaCallback callback, MmsPlaylistEntry *entry, MemoryBuffer *buffer, EventObject *opened_handler_obj, EventHandler opened_handler)
		: MediaClosure (media, callback, entry, "ParseHeaderClosure")
	{
		this->buffer = buffer;
		this->buffer->ref ();
		this->opened_handler_obj = opened_handler_obj;
		this->opened_handler_obj->ref ();
		this->opened_handler = opened_handler;
	}
	virtual ~ParseHeaderClosure ()
	{
		this->buffer->unref ();
		this->opened_handler_obj->unref ();
	}
};

/*
 * MmsPlaylistEntry
 */

MmsPlaylistEntry::MmsPlaylistEntry (Media *media, MmsSource *source)
	: IMediaSource (Type::MMSPLAYLISTENTRY, media)
{
	finished = false;
	opened = false;
	parent = source;
	write_count = 0;
	demuxer = NULL;
	playlist_gen_id = NULL;
	broadcast_id = NULL;
	features = HttpStreamingFeaturesNone;
	buffers = NULL;
	
	g_return_if_fail (parent != NULL);
	parent->ref ();
}

MediaResult
MmsPlaylistEntry::Initialize ()
{
	return MEDIA_SUCCESS;
}

void
MmsPlaylistEntry::Dispose ()
{
	// thread safe
	MmsSource *mms_source;
	IMediaDemuxer *demux;
	List *buffers;

	Lock ();
	mms_source = this->parent;
	this->parent = NULL;
	demux = this->demuxer;
	this->demuxer = NULL;
	g_free (playlist_gen_id);
	playlist_gen_id = NULL;
	g_free (broadcast_id);
	broadcast_id = NULL;
	buffers = this->buffers;
	this->buffers = NULL;
	Unlock ();

	if (mms_source != NULL)
		mms_source->unref ();

	if (demux != NULL)
		demux->unref ();

	delete buffers;

	// This is a bit weird - in certain
	// we can end up with a circular dependency between
	// Media and MmsPlaylistEntry, where Media::Dispose
	// isn't called. So if Media::Dispose hasn't been
	// called, do it here, and only do it after our
	// instance copy of the media is cleared out to 
	// prevent infinite loops.
	Media *m = GetMediaReffed ();

	IMediaSource::Dispose ();

	if (m != NULL) {
		if (!m->IsDisposed ())
			m->Dispose ();
		m->unref ();
	}
}

MediaResult
MmsPlaylistEntry::SeekToPts (guint64 pts)
{
	MmsSource *ms = GetParentReffed ();
	if (ms) {
		ms->SeekToPts (pts);
		ms->unref ();
		return MEDIA_SUCCESS;
	} else {
		fprintf (stderr, "MmsPlaylistEntry::SeekToPts (%" G_GUINT64_FORMAT "): Could not seek to pts, no parent.\n", pts);
		return MEDIA_FAIL;
	}
}

void
MmsPlaylistEntry::NotifyFinished ()
{
	finished = true;
}

void
MmsPlaylistEntry::GetSelectedStreams (gint8 streams [128])
{
	ASFDemuxer *demuxer = NULL;
	MmsSource *mms_source = NULL;
	ASFFileProperties *properties;
	gint32 audio_bitrates [128];
	gint32 video_bitrates [128];
	gint64 max_bitrate;
	int video_stream = 0;
	int video_rate = 0;
	int audio_stream = 0;
	int audio_rate = 0;
	
	memset (audio_bitrates, 0xff, 128 * 4);
	memset (video_bitrates, 0xff, 128 * 4);
	memset (streams, 0xff, 128); 
	
	demuxer = GetDemuxerReffed ();
	if (demuxer == NULL) {
		LOG_MMS ("MmsPlaylistEntry::GetSelectedStream (): no demuxer\n");
		goto cleanup;
	}

	mms_source = GetParentReffed ();
	if (mms_source == NULL) {
		LOG_MMS ("MmsPlaylistEntry::GetSelectedStream (): no parent\n");
		goto cleanup;
	}
	max_bitrate = mms_source->GetMaxBitRate ();

	properties = demuxer->GetFileProperties ();
	if (properties == NULL) {
		LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams (): no file properties in the demuxer\n");
		goto cleanup;
	}

	LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams () max_bitrate: %" G_GINT64_FORMAT "\n", max_bitrate);

	for (int i = 1; i < 128; i++) {
		int current_stream;
		if (!demuxer->IsValidStream (i)) {
			streams [i] = -1; // inexistent
			continue;
		}
		streams [i] = 0; // disabled
		current_stream = i;

		ASFStreamProperties *stream_properties = demuxer->GetStreamProperties (current_stream);
		ASFExtendedStreamProperties *extended_stream_properties = demuxer->GetExtendedStreamProperties (current_stream);

		if (stream_properties == NULL) {
			printf ("MmsPlaylistEntry::GetSelectedStreams (): stream #%i doesn't have any stream properties.\n", current_stream);
			continue;
		}

		if (stream_properties->IsAudio ()) {
			WaveFormatEx* wave = stream_properties->GetAudioData ();
			audio_bitrates [current_stream] = wave->bytes_per_second * 8;
		} else if (stream_properties->IsVideo ()) {
			int bit_rate = 0;
			ASFVideoStreamData *video_data = stream_properties->GetVideoData ();
			BitmapInfoHeader *bmp;

			if (extended_stream_properties != NULL) {
				bit_rate = extended_stream_properties->data_bitrate;
			} else if (video_data != NULL) {
				bmp = video_data->GetBitmapInfoHeader ();
				if (bmp != NULL) {
					bit_rate = bmp->image_width*bmp->image_height;
				}
			}

			video_bitrates [current_stream] = bit_rate;
		} else if (stream_properties->IsCommand ()) {
			// we select all marker streams
			streams [current_stream] = 1;
		}
	}
	
	// select the video stream
	for (int i = 1; i < 128; i++) {
		int stream_rate = video_bitrates [i];

		if (stream_rate == -1)
			continue;

		if (video_rate == 0) {
			video_rate = stream_rate;
			video_stream = i;
		}

		if (stream_rate > video_rate && stream_rate < (max_bitrate * VIDEO_BITRATE_PERCENTAGE)) {
			video_rate = stream_rate;
			video_stream = i;
		}
	}
	streams [video_stream] = 1; // selected		
	LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams (): Selected video stream %i of rate %i\n", video_stream, video_rate);

	// select audio stream
	for (int i = 1; i < 128; i++) {
		int stream_rate = audio_bitrates [i];

		if (stream_rate == -1)
			continue;

		if (audio_rate == 0) {
			audio_rate = stream_rate;
			audio_stream = i;
		}

		if (stream_rate > audio_rate && stream_rate < (max_bitrate * AUDIO_BITRATE_PERCENTAGE)) {
			audio_rate = stream_rate;
			audio_stream = i;
		}
	}
	streams [audio_stream] = 1; // selected
	LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams (): Selected audio stream %i of rate %i\n", audio_stream, audio_rate);
	
	/* We need to select the streams right away, otherwise we might end up trying to give frames to a
	 * stream before the MediaPlayer has been able to select it, causing the first frames to be dropped */
	for (int i = 1; i < 128; i++) {
		if (streams [i] == 1) {
			IMediaStream *stream = demuxer->GetStreamOfASFIndex (i);
			if (stream == NULL) {
#if SANITY
				printf ("MmsPlaylistEntry::GetSelectedStreams (): tried to selected asf stream #%i, but it doesn't exist?\n", i);
#endif
				continue;
			}
			stream->SetSelected (true);
		}
	}

cleanup:
	if (demuxer)
		demuxer->unref ();
	if (mms_source)
		mms_source->unref ();
}

bool
MmsPlaylistEntry::IsHeaderParsed ()
{
	bool result;
	Lock ();
	result = demuxer != NULL;
	Unlock ();
	return result;
}

ASFDemuxer *
MmsPlaylistEntry::GetDemuxerReffed ()
{
	// thread safe
	ASFDemuxer *result;
	
	Lock ();
	result = demuxer;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

MmsSource *
MmsPlaylistEntry::GetParentReffed ()
{
	// thread safe
	MmsSource *result;
	
	Lock ();
	result = parent;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

IMediaDemuxer *
MmsPlaylistEntry::CreateDemuxer (Media *media, MemoryBuffer *initial_buffer)
{
	return GetDemuxerReffed ();
}

void 
MmsPlaylistEntry::SetPlaylistGenId (const char *value)
{
	// thread safe
	Lock ();
	g_free (playlist_gen_id);
	playlist_gen_id = g_strdup (value);
	Unlock ();
}

char *
MmsPlaylistEntry::GetPlaylistGenId ()
{
	// thread safe
	char *result;
	Lock ();
	result = g_strdup (playlist_gen_id);
	Unlock ();
	return result;
}

void
MmsPlaylistEntry::SetBroadcastId (const char *value)
{
	// thread safe
	Lock ();
	g_free (broadcast_id);
	broadcast_id = g_strdup (value);
	Unlock ();
}

char *
MmsPlaylistEntry::GetBroadcastId ()
{
	// thread safe
	char *result;
	Lock ();
	result = g_strdup (broadcast_id);
	Unlock ();
	return result;
}

void 
MmsPlaylistEntry::SetHttpStreamingFeatures (HttpStreamingFeatures value)
{
	features = value;
}

HttpStreamingFeatures
MmsPlaylistEntry::GetHttpStreamingFeatures ()
{
	return features;
}

void
MmsPlaylistEntry::AddEntryCallback (EventObject *obj)
{
	((MmsPlaylistEntry *) obj)->AddEntry ();
}

void
MmsPlaylistEntry::AddEntry ()
{
	Media *media = NULL;
	PlaylistEntry *playlist;
	PlaylistEntry *entry = NULL;
	MmsDemuxer *mms_demuxer = NULL;

	LOG_MMS ("MmsPlaylistEntry::AddEntry (): InMainThread: %i\n", Surface::InMainThread ());

	if (!Surface::InMainThread ()) {
		AddTickCall (AddEntryCallback);
		return;
	}
	
	media = GetMediaReffed ();
	g_return_if_fail (media != NULL);
	
	if (parent == NULL)
		goto cleanup;
	
	mms_demuxer = parent->GetDemuxerReffed ();
	
	if (mms_demuxer == NULL)
		goto cleanup;
	
	playlist = mms_demuxer->GetPlaylist ();
	
	if (playlist == NULL)
		goto cleanup;
	
	entry = new PlaylistEntry (playlist->GetRoot (), playlist);
	entry->SetIsLive (features & HttpStreamingBroadcast);
	if (playlist->HasStartTime ())
		entry->SetStartTime (playlist->GetStartTime ());
	if (playlist->HasDuration ())
		entry->SetDuration (playlist->GetDuration ());
	
	playlist->AddEntry (entry);
	
	entry->InitializeWithSource (this);

	if (playlist->GetIsDynamicWaiting ()) {
		LOG_MMS ("MmsPlaylistEntry::AddEntry (): we were waiting for this entry, calling PlayNext.\n");
		playlist->SetIsDynamicWaiting (false);
		playlist->PlayNext ();
	}

cleanup:
	if (media)
		media->unref ();
	if (mms_demuxer)
		mms_demuxer->unref ();
	if (entry)
		entry->unref ();
}

MediaResult
MmsPlaylistEntry::ParseHeaderCallback (MediaClosure *c)
{
	ParseHeaderClosure *closure = (ParseHeaderClosure *) c;
	((MmsPlaylistEntry *) closure->GetContext ())->ParseHeader (closure);
	return MEDIA_SUCCESS;
}

void
MmsPlaylistEntry::ParseHeaderAsync (void *buffer, gint32 size, EventObject *opened_handler_obj, EventHandler opened_handler)
{
	Media *media;

	VERIFY_MAIN_THREAD;

	media = GetMediaReffed ();
	if (media != NULL) {
		MemoryBuffer *buf = new MemoryBuffer (media, g_memdup (buffer, size), size, true);
		ParseHeaderClosure *closure = new ParseHeaderClosure (media, ParseHeaderCallback, this, buf, opened_handler_obj, opened_handler);
		media->EnqueueWork (closure);
		closure->unref ();
		buf->unref ();
		media->unref ();
	}
}

void
MmsPlaylistEntry::OpenedHandler (IMediaDemuxer *sender, EventArgs *ea)
{
	MediaClosure::Node *node;
	gint8 dummy [128];
	List *buffers;

	VERIFY_MEDIA_THREAD;

	GetSelectedStreams (dummy);

	Lock ();
	buffers = this->buffers;
	this->buffers = NULL;
	opened = true;
	Unlock ();

	LOG_MMS ("MmsPlaylistEntry::OpenedHandler (): demuxer has been opened, rewriting %i packets\n", buffers == NULL ? 0 : buffers->Length ());

	if (buffers != NULL) {
		node = (MediaClosure::Node *) buffers->First ();
		while (node != NULL) {
			WritePacketCallback (node->closure);
			node = (MediaClosure::Node *) node->next;
		}
		delete buffers;
		buffers = NULL;
	}
}

void
MmsPlaylistEntry::ParseHeader (MediaClosure *c)
{
	VERIFY_MEDIA_THREAD;

	LOG_MMS ("MmsPlaylistEntry::ParseHeader (%" G_GINT64_FORMAT ")\n", ((ParseHeaderClosure *) c)->buffer->GetSize ());

	bool result;
	Media *media;
	ASFDemuxer *asf_demuxer;
	char *error_message = NULL;
	guint32 required_size = 0;
	ParseHeaderClosure *closure = (ParseHeaderClosure *) c;
	MemoryBuffer *buffer = closure->buffer;

	// this method shouldn't get called more than once
	g_return_if_fail (demuxer == NULL);
	
	media = GetMediaReffed ();
	g_return_if_fail (media != NULL);
	
	media->ReportDownloadProgress (1.0, 0.0, false);
	
	asf_demuxer = new ASFDemuxer (media, parent, buffer, this);
	asf_demuxer->AddSafeHandler (IMediaDemuxer::OpenedEvent, closure->opened_handler, closure->opened_handler_obj, true);
	asf_demuxer->AddSafeHandler (IMediaDemuxer::OpenedEvent, OpenedCallback, this, false);
	result = asf_demuxer->ReadHeaderObject (buffer, &error_message, &required_size);
	media->unref ();

	if (result) {
		Lock ();
		if (this->demuxer)
			this->demuxer->unref ();
		this->demuxer = asf_demuxer;
		Unlock ();

		AddEntry ();
	} else {
		if (error_message != NULL) {
			ReportErrorOccurred (error_message);
			g_free (error_message);
		} else {
			LOG_MMS ("MmsPlaylistEntry::ParseHeader (%" G_GINT64_FORMAT "): Header parsing failed (not enough data, requires: %u bytes) - however the http streaming spec require the entire header to be present in this buffer.\n", 
				buffer->GetSize (), required_size);
			ReportErrorOccurred ("MmsPlaylistEntry: Could not parse header (not enough data when the http streaming spec requires all data to be available at this point)\n");
		}
		asf_demuxer->unref ();
	}
}

MediaResult
MmsPlaylistEntry::WritePacketCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	MmsPlaylistEntry *mpe = (MmsPlaylistEntry *) closure->GetContext ();
	ASFDemuxer *demuxer;

	VERIFY_MEDIA_THREAD;

	if (mpe->opened) {
		demuxer = mpe->GetDemuxerReffed ();
		if (demuxer) {
			demuxer->DeliverData (closure->GetOffset (), closure->GetData ());
			demuxer->unref ();
		}
	} else {
		if (mpe->buffers == NULL)
			mpe->buffers = new List ();
		mpe->buffers->Append (new MediaClosure::Node (closure));
	}

	return MEDIA_SUCCESS;
}

void
MmsPlaylistEntry::WritePacket (void *buf, gint32 n)
{
	MemoryBuffer *src;
	Media *media;

	write_count++;

	LOG_ASF ("MmsPlaylistEntry::WritePacket (%i bytes), write_count: %" G_GINT64_FORMAT "\n", n, write_count);
	VERIFY_MAIN_THREAD;

	media = GetMediaReffed ();

	g_return_if_fail (media != NULL);

	src = new MemoryBuffer (media, g_memdup (buf, n), n, true);
	MediaReadClosure *closure = new MediaReadClosure (media, WritePacketCallback, this, 0, 0);
	closure->SetData (src);
	media->EnqueueWork (closure);
	closure->unref ();
	src->unref ();

	if (media)
		media->unref ();
}

/*
 * MmsDemuxer
 */

MmsDemuxer::MmsDemuxer (Media *media, MmsSource *source)
	: IMediaDemuxer (Type::MMSDEMUXER, media, source)
{
	playlist = NULL;
	mms_source = source;
	if (mms_source)
		mms_source->ref ();
}

void 
MmsDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	printf ("MmsDemuxer::GetFrameAsyncInternal (%p): This method should never be called.\n", stream);
}

void 
MmsDemuxer::OpenDemuxerAsyncInternal ()
{
	Media *media;
	
	LOG_MMS ("MmsDemuxer::OpenDemuxerAsyncInternal ().\n");
	VERIFY_MEDIA_THREAD;
	
	media = GetMediaReffed ();
	
	g_return_if_fail (playlist == NULL);
	g_return_if_fail (media != NULL);
	
	playlist = media->GetPlaylistEntry ();
	playlist->ref ();

	if (mms_source != NULL && mms_source->IsSSPL ())
		playlist->SetIsDynamic ();
	media->unref ();

	if (mms_source->OpenMmsDemuxerCalled ()) {
		/* OpenMmsDemuxerCalled returns true if ReportOpenDemuxerCompleted needs to be called */
		LOG_MMS ("MmsDemuxer::OpenDemuxerAsyncInternal (): the mms source has already been opened, call ReportOpenDemuxerCompleted\n");
		ReportOpenDemuxerCompleted ();
	}
}

MediaResult
MmsDemuxer::SeekInternal (guint64 pts)
{
	g_warning ("MmsDemuxer::SeekInternal (%" G_GINT64_FORMAT "): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", pts);
	print_stack_trace ();

	return MEDIA_FAIL;
}

void 
MmsDemuxer::SeekAsyncInternal (guint64 seekToTime)
{
	LOG_MMS("MmsDemuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT "): mms_source: %p\n", MilliSeconds_FromPts (seekToTime), mms_source);
	if (mms_source == NULL)
		return;

	if (mms_source->SeekToPts (seekToTime) == MEDIA_SUCCESS)
		ReportSeekCompleted (seekToTime);
}

void 
MmsDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	printf ("MmsDemuxer::SwitchMediaStreamAsyncInternal (%p): Not implemented.\n", stream);
}
	
void 
MmsDemuxer::Dispose ()
{
	PlaylistEntry *pl;
	MmsSource *src;
	
	mutex.Lock ();
	pl = this->playlist;
	this->playlist = NULL;
	src = this->mms_source;
	this->mms_source = NULL;
	mutex.Unlock ();
	
	if (pl)
		pl->unref ();
		
	if (src)
		src->unref ();
	
	IMediaDemuxer::Dispose ();
}
	

/*
 * GUIDs
 */

ASFGuid asf_guids_empty = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; 

/* Top level object guids */

ASFGuid asf_guids_header = { 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
ASFGuid asf_guids_data = { 0x75B22636, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
ASFGuid asf_guids_index = { 0xD6E229D3, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
ASFGuid asf_guids_simple_index = { 0x33000890, 0xE5B1, 0x11CF, { 0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB } };
ASFGuid asf_guids_media_object_index = { 0xFEB103F8, 0x12AD, 0x4C64, { 0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C } };
ASFGuid asf_guids_timecode_index = { 0x3CB73FD0, 0x0C4A, 0x4803, { 0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C } };

/* Header object guids */

ASFGuid asf_guids_file_properties = { 0x8CABDCA1, 0xA947, 0x11CF, { 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
ASFGuid asf_guids_stream_properties = { 0xB7DC0791, 0xA9B7, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
ASFGuid asf_guids_header_extension = { 0x5FBF03B5, 0xA92E, 0x11CF, { 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
ASFGuid asf_guids_codec_list = { 0x86D15240, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
ASFGuid asf_guids_script_command = { 0x1EFB1A30, 0x0B62, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
ASFGuid asf_guids_marker = { 0xF487CD01, 0xA951, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
ASFGuid asf_guids_bitrate_mutual_exclusion = { 0xD6E229DC, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
ASFGuid asf_guids_error_correction = { 0x75B22635, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
ASFGuid asf_guids_content_description = { 0x75B22633, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
ASFGuid asf_guids_extended_content_description = { 0xD2D0A440, 0xE307, 0x11D2, { 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 } };
ASFGuid asf_guids_content_branding = { 0x2211B3FA, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
ASFGuid asf_guids_stream_bitrate_properties = { 0x7BF875CE, 0x468D, 0x11D1, { 0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2 } };
ASFGuid asf_guids_content_encryption = { 0x2211B3FB, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
ASFGuid asf_guids_extended_content_encryption = { 0x298AE614, 0x2622, 0x4C17, { 0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C } };
ASFGuid asf_guids_digital_signature = { 0x2211B3FC, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
ASFGuid asf_guids_padding = { 0x1806D474, 0xCADF, 0x4509, { 0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8 } };

/* Header extension object guids */
ASFGuid asf_guids_extended_stream_properties = { 0x14E6A5CB, 0xC672, 0x4332, { 0x83, 0x99, 0xA9, 0x69, 0x52, 0x06, 0x5B, 0x5A } };
ASFGuid asf_guids_advanced_mutual_exclusion = { 0xA08649CF, 0x4775, 0x4670, { 0x8A, 0x16, 0x6E, 0x35, 0x35, 0x75, 0x66, 0xCD } };
ASFGuid asf_guids_group_mutual_exclusion = { 0xD1465A40, 0x5A79, 0x4338, { 0xB7, 0x1B, 0xE3, 0x6B, 0x8F, 0xD6, 0xC2, 0x49 } };
ASFGuid asf_guids_stream_prioritization = { 0xD4FED15B, 0x88D3, 0x454F, { 0x81, 0xF0, 0xED, 0x5C, 0x45, 0x99, 0x9E, 0x24 } };
ASFGuid asf_guids_bandwidth_sharing = { 0xA69609E6, 0x517B, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };
ASFGuid asf_guids_language_list = { 0x7C4346A9, 0xEFE0, 0x4BFC, { 0xB2, 0x29, 0x39, 0x3E, 0xDE, 0x41, 0x5C, 0x85 } };
ASFGuid asf_guids_metadata = { 0xC5F8CBEA, 0x5BAF, 0x4877, { 0x84, 0x67, 0xAA, 0x8C, 0x44, 0xFA, 0x4C, 0xCA } };
ASFGuid asf_guids_metadata_library = { 0x44231C94, 0x9498, 0x49D1, { 0xA1, 0x41, 0x1D, 0x13, 0x4E, 0x45, 0x70, 0x54 } };
ASFGuid asf_guids_index_parameters = { 0xD6E229DF, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
ASFGuid asf_guids_media_object_index_parameters = { 0x6B203BAD, 0x3F11, 0x48E4, { 0xAC, 0xA8, 0xD7, 0x61, 0x3D, 0xE2, 0xCF, 0xA7 } };
ASFGuid asf_guids_timecode_index_parameters = { 0xF55E496D, 0x9797, 0x4B5D, { 0x8C, 0x8B, 0x60, 0x4D, 0xFE, 0x9B, 0xFB, 0x24 } };
ASFGuid asf_guids_compatibility = { 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
ASFGuid asf_guids_compatibility_object = { 0x26F18B5D, 0x4584, 0x47EC, { 0x9F, 0x5F, 0x0E, 0x65, 0x1F, 0x04, 0x52, 0xC9 } };
ASFGuid asf_guids_advanced_content_encryption = { 0x43058533, 0x6981, 0x49E6, { 0x9B, 0x74, 0xAD, 0x12, 0xCB, 0x86, 0xD5, 0x8C } };

/* Stream properties object, stream type guids */

ASFGuid asf_guids_media_audio = { 0xF8699E40, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
ASFGuid asf_guids_media_video = { 0xBC19EFC0, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
ASFGuid asf_guids_media_command = { 0x59DACFC0, 0x59E6, 0x11D0, { 0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
ASFGuid asf_guids_media_jfif = { 0xB61BE100, 0x5B4E, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
ASFGuid asf_guids_media_degradable_jpeg = { 0x35907DE0, 0xE415, 0x11CF, { 0xA9, 0x17, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
ASFGuid asf_guids_file_transfer = { 0x91BD222C, 0xF21C, 0x497A, { 0x8B, 0x6D, 0x5A, 0xA8, 0x6B, 0xFC, 0x01, 0x85 } };
ASFGuid asf_guids_binary = { 0x3AFB65E2, 0x47EF, 0x40F2, { 0xAC, 0x2C, 0x70, 0xA9, 0x0D, 0x71, 0xD3, 0x43 } };

/* Web stream type-specific data guids */
ASFGuid asf_guids_webstream_media_subtype = { 0x776257D4, 0xC627, 0x41CB, { 0x8F, 0x81, 0x7A, 0xC7, 0xFF, 0x1C, 0x40, 0xCC } };
ASFGuid asf_guids_webstream_format = { 0xDA1E6B13, 0x8359, 0x4050, { 0xB3, 0x98, 0x38, 0x8E, 0x96, 0x5B, 0xF0, 0x0C } };

/* Stream properties, object error correction type guids */
ASFGuid asf_guids_no_error_correction = { 0x20FB5700, 0x5B55, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
ASFGuid asf_guids_audio_stread = { 0xBFC3CD50, 0x618F, 0x11CF, { 0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20 } };

/* Header extension object guids */
ASFGuid asf_guids_reserved1 = { 0xABD3D211, 0xA9BA, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };

/* Advanced content encryption object system id guids */
ASFGuid asf_guids_drm = { 0x7A079BB6, 0xDAA4, 0x4E12, { 0xA5, 0xCA, 0x91, 0xD3, 0x8D, 0xC1, 0x1A, 0x8D } };
// drm = Content_Encryption_System_Windows_Media_DRM_Network_Devides in the spec
// Figured it was somewhat long, so it got abbreviated 

/* Codec list object guids */
ASFGuid asf_guids_reserved2 = { 0x86D15241, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };

/* Script command object guids */ 
ASFGuid asf_guids_reserved3 = { 0x4B1ACBE3, 0x100B, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };

/* Marker object guids */
ASFGuid asf_guids_reserved4 = { 0x4CFEDB20, 0x75F6, 0x11CF, { 0x9C, 0x0F, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB } };

/* Mutual exclusion object exclusion type guids */
ASFGuid asf_guids_mutex_language = { 0xD6E22A00, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
ASFGuid asf_guids_mutex_bitrate = { 0xD6E22A01, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
ASFGuid asf_guids_mutex_unknown = { 0xD6E22A02, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

/* Bandwidth sharing object guids */
ASFGuid asf_guids_bandwidth_sharing_exclusive = { 0xAF6060AA, 0x5197, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };
ASFGuid asf_guids_bandwidth_sharing_partial = { 0xAF6060AB, 0x5197, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };

/* Standard payload extension system guids */
ASFGuid asf_guids_payload_timecode = { 0x399595EC, 0x8667, 0x4E2D, { 0x8F, 0xDB, 0x98, 0x81, 0x4C, 0xE7, 0x6C, 0x1E } };
ASFGuid asf_guids_payload_filename = { 0xE165EC0E, 0x19ED, 0x45D7, { 0xB4, 0xA7, 0x25, 0xCB, 0xD1, 0xE2, 0x8E, 0x9B } };
ASFGuid asf_guids_payload_content_type = { 0xD590DC20, 0x07BC, 0x436C, { 0x9C, 0xF7, 0xF3, 0xBB, 0xFB, 0xF1, 0xA4, 0xDC } };
ASFGuid asf_guids_payload_pixel_aspect_ratio = { 0x1B1EE554, 0xF9EA, 0x4BC8, { 0x82, 0x1A, 0x37, 0x6B, 0x74, 0xE4, 0xC4, 0xB8 } };
ASFGuid asf_guids_payload_sample_duration = { 0xC6BD9450, 0x867F, 0x4907, { 0x83, 0xA3, 0xC7, 0x79, 0x21, 0xB7, 0x33, 0xAD } };
ASFGuid asf_guids_payload_encryption_sample_id = { 0x6698B84E, 0x0AFA, 0x4330, { 0xAE, 0xB2, 0x1C, 0x0A, 0x98, 0xD7, 0xA4, 0x4D } };

static const struct {
	ASFGuid *guid;
	const char *name;
} asf_types [] = {
	{ &asf_guids_header, "ASF_HEADER" },
	{ &asf_guids_data, "ASF_DATA" },
	{ &asf_guids_index, "ASF_INDEX" },
	{ &asf_guids_simple_index, "ASF_SIMPLE_INDEX" },
	{ &asf_guids_media_object_index, "ASF_MEDIA_OBJECT_INDEX" },
	{ &asf_guids_timecode_index, "ASF_TIMECODE_INDEX" },
	{ &asf_guids_file_properties, "ASF_FILE_PROPERTIES" },
	{ &asf_guids_stream_properties, "ASF_STREAM_PROPERTIES" },
	{ &asf_guids_header_extension, "ASF_HEADER_EXTENSION" },
	{ &asf_guids_codec_list, "ASF_CODEC_LIST" },
	{ &asf_guids_script_command, "ASF_SCRIPT_COMMAND" },
	{ &asf_guids_marker, "ASF_MARKER" },
	{ &asf_guids_bitrate_mutual_exclusion, "ASF_BITRATE_MUTUAL_EXCLUSION" },
	{ &asf_guids_error_correction, "ASF_ERROR_CORRECTION" },
	{ &asf_guids_content_description, "ASF_CONTENT_DESCRIPTION" },
	{ &asf_guids_extended_content_description, "ASF_EXTENDED_CONTENT_DESCRIPTION" },
	{ &asf_guids_content_branding, "ASF_CONTENT_BRANDING" },
	{ &asf_guids_stream_bitrate_properties, "ASF_STREAM_BITRATE_PROPERTIES" },
	{ &asf_guids_content_encryption, "ASF_CONTENT_ENCRYPTION" },
	{ &asf_guids_extended_content_encryption, "ASF_EXTENDED_CONTENT_ENCRYPTION" },
	{ &asf_guids_digital_signature, "ASF_DIGITAL_SIGNATURE" },
	{ &asf_guids_padding, "ASF_PADDING" },
	{ &asf_guids_extended_stream_properties, "ASF_EXTENDED_STREAM_PROPERTIES" },
	{ &asf_guids_advanced_mutual_exclusion, "ASF_ADVANCED_MUTUAL_EXCLUSION" },
	{ &asf_guids_group_mutual_exclusion, "ASF_GROUP_MUTUAL_EXCLUSION" },
	{ &asf_guids_stream_prioritization, "ASF_STREAM_PRIORITIZATION" },
	{ &asf_guids_bandwidth_sharing, "ASF_BANDWIDTH_SHARING" },
	{ &asf_guids_language_list, "ASF_LANGUAGE_LIST" },
	{ &asf_guids_metadata, "ASF_METADATA" },
	{ &asf_guids_metadata_library, "ASF_METADATA_LIBRARY" },
	{ &asf_guids_index_parameters, "ASF_INDEX_PARAMETERS" },
	{ &asf_guids_media_object_index_parameters, "ASF_MEDIA_OBJECT_INDEX_PARAMETERS" },
	{ &asf_guids_timecode_index_parameters, "ASF_TIMECODE_INDEX_PARAMETERS" },
	{ &asf_guids_compatibility, "ASF_COMPATIBILITY" },
	{ &asf_guids_compatibility_object, "ASF_COMPATIBILITY_OBJECT" },
	{ &asf_guids_advanced_content_encryption, "ASF_ADVANCED_CONTENT_ENCRYPTION" },
	{ &asf_guids_media_audio, "ASF_MEDIA_AUDIO" },
	{ &asf_guids_media_video, "ASF_MEDIA_VIDEO" },
	{ &asf_guids_media_command, "ASF_MEDIA_COMMAND" },
	{ &asf_guids_media_jfif, "ASF_MEDIA_JFIF" },
	{ &asf_guids_media_degradable_jpeg, "ASF_MEDIA_DEGRADABLE_JPEG" },
	{ &asf_guids_file_transfer, "ASF_FILE_TRANSFER" },
	{ &asf_guids_binary, "ASF_BINARY" },
	{ &asf_guids_webstream_media_subtype, "ASF_WEBSTREAM_MEDIA_SUBTYPE" },
	{ &asf_guids_webstream_format, "ASF_WEBSTREAM_FORMAT" },
	{ &asf_guids_no_error_correction, "ASF_NO_ERROR_CORRECTION" },
	{ &asf_guids_audio_stread, "ASF_AUDIO_STREAD" },
	{ &asf_guids_reserved1, "ASF_RESERVED1" },
	{ &asf_guids_drm, "ASF_DRM" },
	{ &asf_guids_reserved2, "ASF_RESERVED2" },
	{ &asf_guids_reserved3, "ASF_RESERVED3" },
	{ &asf_guids_reserved4, "ASF_RESERVED4" },
	{ &asf_guids_mutex_language, "ASF_MUTEX_LANGUAGE" },
	{ &asf_guids_mutex_bitrate, "ASF_MUTEX_BITRATE" },
	{ &asf_guids_mutex_unknown, "ASF_MUTEX_UNKNOWN" },
	{ &asf_guids_bandwidth_sharing_exclusive, "ASF_BANDWIDTH_SHARING_EXCLUSIVE" },
	{ &asf_guids_bandwidth_sharing_partial, "ASF_BANDWIDTH_SHARING_PARTIAL" },
	{ &asf_guids_payload_timecode, "ASF_PAYLOAD_TIMECODE" },
	{ &asf_guids_payload_filename, "ASF_PAYLOAD_FILENAME" },
	{ &asf_guids_payload_content_type, "ASF_PAYLOAD_CONTENT_TYPE" },
	{ &asf_guids_payload_pixel_aspect_ratio, "ASF_PAYLOAD_PIXEL_ASPECT_RATIO" },
	{ &asf_guids_payload_sample_duration, "ASF_PAYLOAD_SAMPLE_DURATION" },
	{ &asf_guids_payload_encryption_sample_id, "ASF_PAYLOAD_ENCRYPTION_SAMPLE_ID" },
	{ &asf_guids_empty, "ASF_LAST_TYPE" }
};

/*
 * ASFMarker
 */

ASFMarker::ASFMarker ()
{
	marker_count = 0;
	name_length = 0;
	name = NULL;
	markers = NULL;
}

ASFMarker::~ASFMarker ()
{
	if (markers != NULL) {
		for (guint32 i = 0; i < marker_count; i++) {
			delete markers [i];
		}
		g_free (markers);
		markers = NULL;
	}
	g_free (name);
	name = NULL;
}

bool
ASFMarker::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (24);

	source->SeekOffset (16); /* reserved: a guid */
	marker_count = source->ReadLE_U32 ();
	source->ReadLE_U16 (); /* reserved: guint16 */
	name_length = source->ReadLE_U16 ();

	VERIFY_AVAILABLE_SIZE (name_length);

	name = source->ReadLE_UTF16 (name_length);

	LOG_ASF ("ASFMarker::Read () marker_count: %u name_length: %u name: %s\n", marker_count, name_length, name);

	if (marker_count > 0) {
		markers = (ASFMarkerEntry **) g_try_malloc (sizeof (ASFMarker *) * marker_count);
		if (markers == NULL) {
			LOG_ASF ("ASFMarker: could not allocate %lu bytes of memory for marker entry array.\n", (unsigned long) (sizeof (ASFMarker *) * marker_count));
			return false;
		}
		memset (markers, 0, sizeof (ASFMarker *) * marker_count);
		for (guint32 i = 0; i < marker_count; i++) {
			markers [i] = new ASFMarkerEntry ();
			if (!markers [i]->Read (context)) {
				LOG_ASF ("ASFMarker: error while reading marker entry #%u\n", i);
				return false;
			}
		}
	}

	return true;
}

/*
 * ASFMarkerEntry
 */

ASFMarkerEntry::ASFMarkerEntry ()
{
	offset = 0;
	pts = 0;
	entry_length = 0;
	send_time = 0;
	flags = 0;
	marker_description_length = 0;
	marker_description = NULL;
}

ASFMarkerEntry::~ASFMarkerEntry ()
{
	g_free (marker_description);
	marker_description = NULL;
}

bool
ASFMarkerEntry::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (30);

	offset = source->ReadLE_U64 ();
	pts = source->ReadLE_U64 ();
	entry_length = source->ReadLE_U16 ();
	send_time = source->ReadLE_U32 ();
	flags = source->ReadLE_U32 ();
	marker_description_length = source->ReadLE_U32 ();

	VERIFY_AVAILABLE_SIZE (marker_description_length * 2);

	marker_description = source->ReadLE_UTF16 (marker_description_length * 2);

	LOG_ASF ("ASFMarkerEntry::Read () offset: %" G_GUINT64_FORMAT " pts: %" G_GUINT64_FORMAT " entry_length: %u send_time: %u flags: %u marker_description_length: %u marker_description: %s\n",
		offset, pts, entry_length, send_time, flags, marker_description_length, marker_description);

	return true;
}

/*
 * ASFVideoStreamData
 */

ASFVideoStreamData::ASFVideoStreamData ()
{
	image_width = 0;
	image_height = 0;
	flags = 0;
	format_data_size = 0;
	bitmap_info_header = NULL;
}

ASFVideoStreamData::~ASFVideoStreamData ()
{
	delete bitmap_info_header;
	bitmap_info_header = NULL;
}

bool
ASFVideoStreamData::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (11);

	image_width = source->ReadLE_U32 ();
	image_height = source->ReadLE_U32 ();
	flags = source->ReadLE_U8 ();
	format_data_size = source->ReadLE_U16 ();

	LOG_ASF ("ASFVideoStreamData::Read (): image_width: %u image_height: %u flags: %u format_data_size: %u\n",
		image_width, image_height, flags, format_data_size);

	VERIFY_AVAILABLE_SIZE (format_data_size);

	bitmap_info_header = new BitmapInfoHeader ();
	if (!bitmap_info_header->Read (context)) {
		LOG_ASF ("ASFVideoStreamData::Read (): error while reading bitmap info header.\n");
		return false;
	}

	return true;
}

/*
 * BitmapInfoHeader
 */

BitmapInfoHeader::BitmapInfoHeader ()
{
	size = 0;
	image_width = 0;
	image_height = 0;
	bits_per_pixel = 0;
	compression_id = 0;
	image_size = 0;
	hor_pixels_per_meter = 0;
	ver_pixels_per_meter = 0;
	colors_used = 0;
	important_colors_used = 0;
	extra_data_size = 0;
	extra_data = NULL;
}

BitmapInfoHeader::~BitmapInfoHeader ()
{
	g_free (extra_data);
	extra_data = NULL;
}

bool
BitmapInfoHeader::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (40);

	size = source->ReadLE_U32 ();
	image_width = source->ReadLE_U32 ();
	image_height = source->ReadLE_U32 ();
	planes = source->ReadLE_U16 ();
	bits_per_pixel = source->ReadLE_U16 ();
	compression_id = source->ReadLE_U32 ();
	image_size = source->ReadLE_U32 ();
	hor_pixels_per_meter = source->ReadLE_U32 ();
	ver_pixels_per_meter = source->ReadLE_U32 ();
	colors_used = source->ReadLE_U32 ();
	important_colors_used = source->ReadLE_U32 ();

	if (size < 40) {
		LOG_ASF ("BitmapInfoHeader::Read (): invalid size: %u (expected >= 40)\n", size);
		return false;
	}

	extra_data_size = size - 40;
	if (extra_data_size > 0) {
		extra_data = source->Read (extra_data_size);
	}

	return true;
}
 

/*
 * ASFExtendedStreamProperties
 */

ASFExtendedStreamProperties::ASFExtendedStreamProperties ()
{
	size = 0;
	start_time = 0;
	end_time = 0;
	data_bitrate = 0;
	buffer_size = 0;
	initial_buffer_fullness = 0;
	alternate_data_bitrate = 0;
	alternate_buffer_size = 0;
	alternate_initial_buffer_fullness = 0;
	maximum_object_size = 0;
	flags = 0;
	stream_id = 0;
	stream_language_id_index = 0;
	average_time_per_frame = 0;
	stream_name_count = 0;
	payload_extension_system_count = 0;
	stream_properties = NULL;
}

ASFExtendedStreamProperties::~ASFExtendedStreamProperties ()
{
	delete stream_properties;
	stream_properties = NULL;
}

bool
ASFExtendedStreamProperties::Read (ASFContext *context, guint64 size)
{
	ASFGuid guid;
	MemoryBuffer *source = context->source;
	guint64 stream_properties_size;
	guint64 start_position = source->GetPosition ();
	guint64 size_left;
	guint64 size_read;

	VERIFY_AVAILABLE_SIZE (64);

	start_time = source->ReadLE_U64 ();
	end_time = source->ReadLE_U64 ();
	data_bitrate = source->ReadLE_U32 ();
	buffer_size = source->ReadLE_U32 ();
	initial_buffer_fullness = source->ReadLE_U32 ();
	alternate_data_bitrate = source->ReadLE_U32 ();
	alternate_buffer_size = source->ReadLE_U32 ();
	alternate_initial_buffer_fullness = source->ReadLE_U32 ();
	maximum_object_size = source->ReadLE_U32 ();
	flags = source->ReadLE_U32 ();
	stream_id = source->ReadLE_U16 ();
	stream_language_id_index = source->ReadLE_U16 ();
	average_time_per_frame = source->ReadLE_U64 ();
	stream_name_count = source->ReadLE_U16 ();
	payload_extension_system_count = source->ReadLE_U16 ();

	LOG_ASF ("ASFExtendedStreamProperties::Read () start_time: %" G_GUINT64_FORMAT " end_time: %" G_GUINT64_FORMAT " data_bitrate: %u buffer_size: %u initial_buffer_fullness: %u "
		"alternate_data_bitrate: %u alternate_buffer_size: %u alternate_initial_buffer_fullness: %u maximum_object_size: %u flags: %u stream_id: %u stream_language_id_index: %u "
		"average_time_per_frame: %" G_GUINT64_FORMAT " stream_name_count: %u payload_extension_system_count: %u\n",
		start_time, end_time, data_bitrate, buffer_size, initial_buffer_fullness, alternate_data_bitrate, alternate_buffer_size, alternate_initial_buffer_fullness,
		maximum_object_size, flags, stream_id, stream_language_id_index, average_time_per_frame, stream_name_count, payload_extension_system_count);

	for (guint32 i = 0; i < stream_name_count; i++) {
		VERIFY_AVAILABLE_SIZE (4);
		
		guint16 language_id_index = source->ReadLE_U16 ();
		guint16 stream_name_length = source->ReadLE_U16 (); /* number of bytes */
		
		VERIFY_AVAILABLE_SIZE (stream_name_length);

		char *stream_name = source->ReadLE_UTF16 (stream_name_length);
		LOG_ASF (" parsed stream name: language_id_index: %u stream_name_length: %u stream_name: '%s'\n", language_id_index, stream_name_length, stream_name);
		g_free (stream_name);
	}

	for (guint32 i = 0; i < payload_extension_system_count; i++) {
		VERIFY_AVAILABLE_SIZE (22);

		if (!guid.Read (source)) {
			LOG_ASF ("ASFExtendedStreamProperties::Read (): error while reading guid.\n");
			return false;
		}

		guint16 extension_data_size = source->ReadLE_U16 ();
		guint32 extension_system_info_length = source->ReadLE_U32 ();

		LOG_ASF (" parsed extension system: guid: %s extension_data_size: %u extension_system_info_length: %u\n",
			guid.ToString (), extension_data_size, extension_system_info_length);

		VERIFY_AVAILABLE_SIZE (extension_system_info_length);

		source->SeekOffset (extension_system_info_length);
	}
	
	size_read = source->GetPosition () - start_position + 24 /* the initial guid + size (guint64) */;

	if (size_read >= size) {
		return true;
	}

	size_left = size - size_read;

	VERIFY_AVAILABLE_SIZE (78);

	if (!guid.Read (source)) {
		LOG_ASF ("ASFExtendedStreamProperties::Read (): error while reading stream guid.\n");
		return false;
	}

	stream_properties_size = source->ReadLE_U64 ();

	LOG_ASF ("ASFExtendedStreamProperties::Read (): reading embedded stream properties.\n");

	stream_properties = new ASFStreamProperties ();
	if (!stream_properties->Read (context)) {
		delete stream_properties;
		stream_properties = NULL;
		LOG_ASF ("ASFExtendedStreamProperties::Read (): could not read embedded stream properties.\n");
		return false;
	}

	return true;
}

/*
 * ASFFileProperties
 */

ASFFileProperties::ASFFileProperties ()
{
	file_size = 0;
	creation_date = 0;
	data_packet_count = 0;
	play_duration = 0;
	send_duration = 0;
	preroll = 0;
	flags = 0;
	min_packet_size = 0;
	max_packet_size = 0;
	max_bitrate = 0;
}

bool
ASFFileProperties::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (80);

	if (!file_id.Read (source)) {
		LOG_ASF ("ASFFileProperties::Read (): error while reading file_id guid.\n");
		return false;
	}

	file_size = source->ReadLE_U64 ();
	creation_date = source->ReadLE_U64 ();
	data_packet_count = source->ReadLE_U64 ();
	play_duration = source->ReadLE_U64 ();
	send_duration = source->ReadLE_U64 ();
	preroll = source->ReadLE_U64 ();
	flags = source->ReadLE_U32 ();
	min_packet_size = source->ReadLE_U32 ();
	max_packet_size = source->ReadLE_U32 ();
	max_bitrate = source->ReadLE_U32 ();

	LOG_ASF ("ASFFileProperties::Read () file_id: %s file_size: %" G_GUINT64_FORMAT " creation_date: %" G_GUINT64_FORMAT " data_packet_count: %" G_GUINT64_FORMAT 
		" play_duration: %" G_GUINT64_FORMAT " send_duration: %" G_GUINT64_FORMAT " preroll: %" G_GUINT64_FORMAT " flags: %u min_packet_size: %u max_packet_size: %u max_bitrate: %u\n",
		file_id.ToString (), file_size, creation_date, data_packet_count, play_duration, send_duration, preroll, flags, min_packet_size, max_packet_size, max_bitrate);

	return true;
}

/*
 * ASFGuid
 */

bool
ASFGuid::Read (MemoryBuffer *source)
{
	a = source->ReadLE_U32 ();
	b = source->ReadLE_U16 ();
	c = source->ReadLE_U16 ();
	for (int i = 0; i < 8; i++) {
		d [i] = source->ReadLE_U8 ();
	}
	return true;
}

const char *
ASFGuid::ToString ()
{
	static char result [96];

	for (int i = 0; *asf_types [i].guid != asf_guids_empty; i++) {
		if (*this == *asf_types [i].guid) {
			return asf_types [i].name;
		}
	}

	snprintf (result, sizeof (result), "{%X, %X, %X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X}", 
		a, b, c, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
	return result;
}

/*
 * ASFErrorCorrectionData
 */

bool
ASFErrorCorrectionData::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	data = 0;
	first = 0;
	second = 0;
	size = 0;

	VERIFY_AVAILABLE_SIZE (1);

	data = source->ReadLE_U8 ();

	if (IsErrorCorrectionPresent ()) {
		VERIFY_AVAILABLE_SIZE (2);

		first = source->ReadLE_U8 ();
		second = source->ReadLE_U8 ();

		size = 3;
	} else {
		source->SeekOffset (-1);
	}

	LOG_ASF ("ASFErrorCorrectionData::Read () data: %u first: %u second: %u IsErrorCorrectionPresent: %s IsOpaqueDataPresent: %s DataLength: %i ErrorCorrectionLengthType: %i\n",
		data, first, second, IsErrorCorrectionPresent () ? "yes" : "no", IsOpaqueDataPresent () ? "yes" : "no", GetDataLength (), GetErrorCorrectionLengthType ());

	return true;
}

/*
 * ASFPayloadParsingInformation
 */

bool
ASFPayloadParsingInformation::Read (ASFContext *context)
{
	ASFDemuxer *demuxer = context->demuxer;
	MemoryBuffer *source = context->source;

	/* There's no guarantee that all these fields will be written to in here */
	packet_length = 0;
	sequence = 0;
	padding_length = 0;
	send_time = 0;
	duration = 0;
	size = 2;

	VERIFY_AVAILABLE_SIZE (2);

	length_type_flags = source->ReadLE_U8 ();
	property_flags = source->ReadLE_U8 ();

	size += ASF_DECODE_PACKED_SIZE (GetPacketLengthType ());
	if (GetPacketLengthType () == 0) {
		packet_length = demuxer->GetPacketSize ();
	} else if (!ASFDemuxer::ReadEncoded (source, GetPacketLengthType (), &packet_length)) {
		return false;
	}

	size += ASF_DECODE_PACKED_SIZE (GetSequenceLengthType ());
	if (!ASFDemuxer::ReadEncoded (source, GetSequenceLengthType (), &sequence)) {
		return false;
	}

	size += ASF_DECODE_PACKED_SIZE (GetPaddingLengthType ());
	if (!ASFDemuxer::ReadEncoded (source, GetPaddingLengthType (), &padding_length)) {
		return false;
	}

	size += 6;
	VERIFY_AVAILABLE_SIZE (6);

	send_time = source->ReadLE_U32 ();
	duration = source->ReadLE_U16 ();

	LOG_ASF ("ASFPayloadParsingInformation::Read ()"
		" length_type_flags: %u packet_length: %u (%u) sequence: %u (%u) padding_length: %u (%u) send_time: %u duration: %u "
		" property_flags: %u ReplicatedDataLengthType: %u OffsetIntoMediaObjectLengthType: %u MediaObjectNumberLengthType: %u StreamNumberLengthType: %u"
		"IsMultiplePayloadsPresent: %s IsErrorCorrectionPresent: %s\n",
		length_type_flags, packet_length, GetPacketLengthType (), sequence, GetSequenceLengthType (), padding_length, GetPaddingLengthType (), send_time, duration, 
		property_flags, GetReplicatedDataLengthType (), GetOffsetIntoMediaObjectLengthType (), GetMediaObjectNumberLengthType (), GetStreamNumberLengthType (),
		IsMultiplePayloadsPresent () ? "yes" : "no", IsErrorCorrectionPresent () ? "yes" : "no");

	return true;
}

/*
 * ASFSinglePayload
 */

ASFSinglePayload::ASFSinglePayload ()
{
	stream_id = 0;
	is_key_frame = false;
	media_object_number = 0;
	offset_into_media_object = 0;
	replicated_data_length = 0;
	replicated_data = NULL;
	payload_data_length = 0;
	payload_data = NULL;
	presentation_time = 0;
}

ASFSinglePayload::~ASFSinglePayload ()
{
	g_free (replicated_data);
	replicated_data = NULL;
	
	g_free (payload_data);
	payload_data = NULL;
}

ASFSinglePayload *
ASFSinglePayload::Clone ()
{
	ASFSinglePayload *result = new ASFSinglePayload ();
	
	result->stream_id = stream_id;
	result->is_key_frame = is_key_frame;
	result->media_object_number = media_object_number;
	result->offset_into_media_object = offset_into_media_object;
	result->replicated_data_length = replicated_data_length;
	if (replicated_data != NULL) {
		result->replicated_data = (guint8 *) g_malloc (replicated_data_length);
		memcpy (result->replicated_data, replicated_data, replicated_data_length);
	}
	result->payload_data_length = payload_data_length;
	if (payload_data != NULL) {
		result->payload_data = (guint8 *) g_malloc (payload_data_length);
		memcpy (result->payload_data, payload_data, payload_data_length);
	}
	result->presentation_time = presentation_time;
	
	return result;
}

bool
ASFSinglePayload::Read (ASFContext *context, ASFErrorCorrectionData *ecd, ASFPayloadParsingInformation *ppi, ASFMultiplePayloads *mp)
{	
	ASFDemuxer *demuxer = context->demuxer;
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (1);

	stream_id = source->ReadLE_U8 ();

	is_key_frame = stream_id & 0x80;
	stream_id = stream_id & 0x7F;

	if (!demuxer->IsValidStream (stream_id)) {
		LOG_ASF ("ASFSinglePayload::Read: Invalid stream number (%d).\n", (int) stream_id);
		return false;
	}

	if (!ASFDemuxer::ReadEncoded (source, ppi->GetMediaObjectNumberLengthType (), &media_object_number)) {
		return false;
	}

	if (!ASFDemuxer::ReadEncoded (source, ppi->GetOffsetIntoMediaObjectLengthType (), &offset_into_media_object)) {
		return false;
	}

	if (!ASFDemuxer::ReadEncoded (source, ppi->GetReplicatedDataLengthType (), &replicated_data_length)) {
		return false;
	}

	if (replicated_data_length >= 2 && replicated_data_length < 7) {
		LOG_ASF ("ASFSinglePayload::Read (): Invalid replicated data length (%i, must be be < 2 or >= 8)\n", replicated_data_length);
		return false;
	} 

	VERIFY_AVAILABLE_SIZE (replicated_data_length);
	replicated_data = (guint8 *) g_try_malloc (replicated_data_length);
	if (replicated_data == NULL) {
		printf ("Moonlight: failed to allocate %i bytes.\n", replicated_data_length);
		return false;
	}

	if (!source->Read (replicated_data, replicated_data_length)) {
		/* This shouldn't happen, given that we've already checked that the memory source has enough data */
		LOG_ASF ("ASFSinglePayload::Read (): Read error while reading 'replicated_data'.\n");
		return false;
	}

	if (replicated_data_length == 1) {
		presentation_time = offset_into_media_object;
	} else if (replicated_data_length >= 8) {
		presentation_time = *(((guint32 *) replicated_data) + 1);
	}

	if (mp != NULL) {
		if (!ASFDemuxer::ReadEncoded (source, mp->GetPayloadLengthType (), &payload_data_length)) {
			return false;
		}
	} else {
		guint32 payload_length;
		// The number of bytes in this array can be calculated from the overall Packet Length field, 
		// and is equal to the Packet Length
		payload_length = ppi->packet_length;
		// minus the packet header length,
		payload_length -= ppi->GetSize ();
		payload_length -= ecd->GetSize (),
		// minus the payload header length (including Replicated Data),
		payload_length -= 1; // stream_number
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi->GetMediaObjectNumberLengthType ());
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi->GetOffsetIntoMediaObjectLengthType ());
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi->GetReplicatedDataLengthType ());
		payload_length -= replicated_data_length;
		// minus the Padding Length.
		payload_length -= ppi->padding_length;
		LOG_ASF ("payload_length: %d. packet_length: %d, replicated_data_length: %d, padding_length: %d\n",
			payload_length, ppi->packet_length, replicated_data_length, ppi->padding_length);
			
		if (payload_length < 0) {
			LOG_ASF ("ASFSinglePayload::Read (): Invalid payload length: %u\n", payload_length);
			return false;
		} 
		
		payload_data_length = payload_length;
	}
	
	if (payload_data_length > 0) {
		VERIFY_AVAILABLE_SIZE (payload_data_length);
		
		payload_data = (guint8 *) g_try_malloc (payload_data_length);
		if (payload_data == NULL) {
			printf ("Moonlight: Failed to allocate %i bytes for payload data.\n", payload_data_length);
			return false;
		}
		
		if (!source->Read (payload_data, payload_data_length)) {
			LOG_ASF ("ASFSinglePayload::Read (): Read error while reading payload data.\n");
			return false;
		}
	}

	LOG_ASF ("ASFSinglePayload::Read () stream_id: %u, is_key_frame: %i, media_object_number: %u offset_into_media_object: %u replicated_data_length: %u payload_data_length: %u presentation_time: %u\n",
		stream_id, is_key_frame, media_object_number, offset_into_media_object, replicated_data_length, payload_data_length, presentation_time);

	return true;
}

/*
 * ASFMultiplePayloads
 */

ASFMultiplePayloads::ASFMultiplePayloads ()
{
	payload_flags = 0;
	payloads = NULL;
	payloads_size = 0;
}

ASFMultiplePayloads::~ASFMultiplePayloads () 
{
	if (payloads) {
		for (int i = 0; payloads[i]; i++)
			delete payloads[i];
		g_free (payloads);
	}
}

ASFSinglePayload**
ASFMultiplePayloads::StealPayloads ()
{
	ASFSinglePayload** result = payloads;
	payloads = NULL;
	return result;
}

bool
ASFMultiplePayloads::ResizeList (guint32 requested_size)
{
	ASFSinglePayload** new_list;
	size_t new_size;
	size_t old_size;

	if (requested_size <= payloads_size)
		return true;

	new_size = (requested_size + 1) * sizeof (ASFSinglePayload *);
	new_list = (ASFSinglePayload **) g_try_malloc (new_size);

	if (new_list == NULL) {
		printf ("Moonlight: Failed to allocate %lu bytes for list of payloads.\n", (unsigned long) new_size);
		return false;
	}

	if (payloads != NULL) {
		old_size = sizeof (ASFSinglePayload *) * payloads_size;
		memcpy (new_list, payloads, old_size);
		memset (((guint8 *) new_list) + old_size, 0, new_size - old_size);
		g_free (payloads);
	} else {
		memset (new_list, 0, new_size);
	}

	payloads = new_list;
	payloads_size = requested_size;

	return true;
}

guint32
ASFMultiplePayloads::CountCompressedPayloads (ASFSinglePayload *payload)
{
	guint8* data = payload->payload_data;
	guint32 length = payload->payload_data_length;
	guint8 size = 0;
	guint32 offset = 0;
	guint32 counter = 0;
	
	if (data == NULL) {
		LOG_ASF ("ASFMultiplePayloads::CountCompressedPayloads (): no data (data corruption?).\n");
		return 0;
	}
	
	while (true) {
		counter++;
		size = *(data + offset);
		offset += (size + 1);
		if (offset > length || size == 0) {
			LOG_ASF ("ASFMultiplePayloads::CountCompressedPayloads (): no data (data corruption?).\n");
			return 0;
		} else if (offset == length) {
			break;
		}
	};
	
	return counter;
}

bool
ASFMultiplePayloads::ReadCompressedPayload (ASFSinglePayload* first, guint32 count, guint32 start_index)
{
	guint8* data = first->payload_data;
	guint8 size = 0;
	guint32 offset = 0;
	ASFSinglePayload* payload = NULL;

	for (guint32 i = 0; i < count; i++) {
		size = *(data + offset);
		offset += 1;
		
		payload = new ASFSinglePayload ();
		payloads [start_index + i] = payload;
		
		payload->stream_id = first->stream_id;
		payload->is_key_frame = first->is_key_frame;
		payload->media_object_number = first->media_object_number + i;
		payload->offset_into_media_object = 0;
		payload->replicated_data_length = 0;
		payload->replicated_data = NULL;
		payload->presentation_time = first->presentation_time + i * first->GetPresentationTimeDelta ();
		payload->payload_data_length = size;
		payload->payload_data = (guint8 *) g_try_malloc (size);
		if (payload->payload_data == NULL) {
			printf ("Moonlight: Failed to allocate %i bytes for compressed payload.\n", size);
			return false;
		}
		memcpy (payload->payload_data, data + offset, size);
		offset += size;
	}
	
	return true;
}

bool
ASFMultiplePayloads::Read (ASFContext *context, ASFErrorCorrectionData *ecd, ASFPayloadParsingInformation *ppi)
{
	MemoryBuffer *source = context->source;
	bool result;
	guint32 count;

	if (ppi->IsMultiplePayloadsPresent ()) {
		VERIFY_AVAILABLE_SIZE (1);
		payload_flags = source->ReadLE_U8 ();

		count = payload_flags & 0x3F; /* number of payloads is encoded in a byte, no need to check for extreme values. */
		
		if (count <= 0) {
			LOG_ASF ("ASFMultiplePayloads::Read (): Invalid number of payloads: %u\n", count);
			return false;
		}

		if (!ResizeList (count)) {
			return false;
		}

		LOG_ASF ("ASFMultiplePayloads::Read (): Reading %u payloads...\n", count); 

		guint32 current_index = 0;
		for (guint32 i = 0; i < count; i++) {
			payloads [current_index] = new ASFSinglePayload ();

			result = payloads [current_index]->Read (context, ecd, ppi, this);
			if (!result) {
				delete payloads [current_index];
				payloads [current_index] = NULL;
				return result;
			}

			if (payloads [current_index]->IsCompressed ()) {
				ASFSinglePayload* first = payloads [current_index];
				guint32 number = CountCompressedPayloads (first);
				if (number <= 0) {
					LOG_ASF ("ASFMultiplePayloads::Read (): No compressed payloads?\n");
					return false;
				}

				if (!ResizeList (number + payloads_size)) {
					return false;
				}

				result = ReadCompressedPayload (first, number, current_index);
				if (!result) {
					return result;
				}
				delete first;
			}

			current_index++;
		}
	} else {
		LOG_ASF ("ASFMultiplePayloads::Read (%p, %p, ?): A single payload\n", context, ecd);

		ASFSinglePayload* payload = new ASFSinglePayload ();
		result = payload->Read (context, ecd, ppi, NULL);
		if (!result) {
			delete payload;
			return result;
		}

		if (payload->IsCompressed ()) {
			int counter = 0;

			counter = CountCompressedPayloads (payload);
			if (counter <= 0) {
				LOG_ASF ("ASFMultiplePayloads::Read (): No compressed payloads?\n");
				return false;
			}

			if (!ResizeList (counter)) {
				return false;
			}

			result = ReadCompressedPayload (payload, counter, 0);
			if (result) {
				return result;
			}

			delete payload;
		} else {
			payloads = (ASFSinglePayload **) g_try_malloc (sizeof (ASFSinglePayload *) * 2);
			if (payloads == NULL) {
				printf ("Moonlight: Failed to allocate %lu bytes for payloads.\n", (unsigned long) (sizeof (ASFSinglePayload *) * 2));
				return false;
			}
			payloads [0] = payload;
			payloads [1] = NULL;

			payload_flags = 1; /*  1 payload */
		}
	}

	LOG_ASF ("ASFMultiplePayloads::Read () payload_flags: %u number of payloads: %u payload_length_type: %u\n",
		payload_flags, GetNumberOfPayloads (), GetPayloadLengthType ());

	return true;
}

/*
 * ASFScriptCommandEntry
 */

ASFScriptCommandEntry::ASFScriptCommandEntry ()
{
	pts = 0;
	type_index = 0;
	name_length = 0;
	name = NULL;
	type = NULL;
}

ASFScriptCommandEntry::~ASFScriptCommandEntry ()
{
	g_free (name);
	name = NULL;
	type = NULL;
}

bool
ASFScriptCommandEntry::Read (ASFContext *context, ASFScriptCommand *command)
{
	MemoryBuffer *source = context->source;

	pts = 0;
	type_index = 0;
	name_length = 0;
	name = NULL;
	type = NULL;

	VERIFY_AVAILABLE_SIZE (8);

	pts = source->ReadLE_U32 ();
	type_index = source->ReadLE_U16 ();
	name_length = source->ReadLE_U16 ();

	VERIFY_AVAILABLE_SIZE (name_length * 2);

	name = source->ReadLE_UTF16 (name_length * 2);

	if (type_index >= command->command_type_count) {
		LOG_ASF ("ASFScriptCommandEntry::Read (): Invalid type index (%u) - command type count = %u\n", type_index, command->command_type_count);
		return false;
	}

	type = command->command_types [type_index];

	LOG_ASF ("ASFScriptCommandEntry::Read (): pts: %u type_index: %u type: %s name_length: %u name: %s\n",
		pts, type_index, type, name_length, name);

	return true;
}

/*
 * ASFScriptCommand
 */

ASFScriptCommand::ASFScriptCommand ()
{
	command_count = 0;
	command_type_count = 0;
	command_types = NULL;
	commands = NULL;
}

ASFScriptCommand::~ASFScriptCommand ()
{
	if (commands != NULL) {
		for (guint32 i = 0; i < command_count; i++) {
			delete commands [i];
		}
		g_free (commands);
		commands = NULL;
	}
	if (command_types != NULL) {
		for (guint32 i = 0; i < command_type_count; i++) {
			g_free (command_types [i]);
		}
		g_free (command_types);
		command_types = NULL;
	}
}

bool
ASFScriptCommand::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (20);

	if (!reserved.Read (source)) {
		return false;
	}

	command_count = context->source->ReadLE_U16 ();
	command_type_count = context->source->ReadLE_U16 ();
	
	if (command_count > 0) {
		size_t s = (command_count + 1) * sizeof (ASFScriptCommandEntry *);
		commands = (ASFScriptCommandEntry **) g_try_malloc (s);
		if (commands == NULL) {
			printf ("Moonlight: Failed to allocate memory for script commands.\n");
			return false;
		}
		memset (commands, 0, s);
	}

	if (command_type_count > 0) {
		size_t s = (command_type_count + 1) * sizeof (char *);
		command_types = (char **) g_try_malloc (s);
		if (command_types == NULL) {
			printf ("Moonlight: Failed to allocate memory for script command types.\n");
			return false;
		}
		memset (command_types, 0, s);
	}

	LOG_ASF ("ASFScriptCommand::Read (): reading %u types and %u commands.\n", command_type_count, command_count);

	for (int i = 0; i < command_type_count; i++) {
		guint16 command_type_length; /* number of WCHAR characters */

		VERIFY_AVAILABLE_SIZE (2);
		command_type_length = source->ReadLE_U16 ();
		
		VERIFY_AVAILABLE_SIZE (command_type_length * 2);
		command_types [i] = source->ReadLE_UTF16 (command_type_length * 2);

		LOG_ASF (" read command with length %u: '%s'\n", command_type_length, command_types [i]);
	}

	for (int i = 0; i < command_count; i++) {
		commands [i] = new ASFScriptCommandEntry ();
		if (!commands [i]->Read (context, this)) {
			return false;
		}
	}
	
	return true;
}

/*
 * ASFStreamProperties
 */

ASFStreamProperties::ASFStreamProperties ()
{
	time_offset = 0;
	type_specific_data_length = 0;
	error_correction_data_length = 0;
	flags = 0;
	video_data = NULL;
	audio_data = NULL;
}

ASFStreamProperties::~ASFStreamProperties ()
{
	delete video_data;
	video_data = NULL;
	delete audio_data;
	audio_data = NULL;
}

bool
ASFStreamProperties::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (54);

	if (!stream_type.Read (source)) {
		LOG_ASF ("ASFStreamProperties::Read (): error while reading stream type guid.\n");
		return false;
	}

	if (!error_correction_type.Read (source)) {
		LOG_ASF ("ASFStreamProperties::Read (): error while reading error correction type guid.\n");
		return false;
	}

	time_offset = source->ReadLE_U64 ();
	type_specific_data_length = source->ReadLE_U32 ();
	error_correction_data_length = source->ReadLE_U32 ();
	flags = source->ReadLE_U16 ();
	source->ReadLE_U32 (); /* reserved */

	LOG_ASF ("ASFStreamProperties::Read (): time_offset: %" G_GUINT64_FORMAT " type_specific_data_length: %u error_correction_data_length: %u flags: %u\n",
		time_offset, type_specific_data_length, error_correction_data_length, flags);

	VERIFY_AVAILABLE_SIZE (type_specific_data_length);

	if (IsAudio ()) {
		if (type_specific_data_length >= 40) {
			audio_data = new WaveFormatExtensible ();
		} else if (type_specific_data_length >= 18) {
			audio_data = new WaveFormatEx ();
		} else {
			LOG_ASF ("ASFStreamProperties::Read (): an audio stream didn't provide a WAVEFORMATEX structure.\n");
			return false;
		}

		if (!audio_data->Read (context)) {
			LOG_ASF ("ASFStreamProperties::Read (): error while reading WAVEFORMATEX[TENSIBLE] structure.\n");
			return false;
		}
	} else if (IsVideo ()) {
		if (type_specific_data_length >= 11) {
			video_data = new ASFVideoStreamData ();
			if (!video_data->Read (context)) {
				LOG_ASF ("ASFStreamProperties::Read (): error while reading video stream data.\n");
				return false;
			}
		} else {
			LOG_ASF ("ASFStreamProperties::Read (): a video stream didn't provide a video info object.\n");
			return false;
		}
	} else {
		LOG_ASF ("ASFStreamProperties::Read (): got unknown stream type.\n");
		source->SeekOffset (type_specific_data_length);
	}

	VERIFY_AVAILABLE_SIZE (error_correction_data_length);
	source->SeekOffset (error_correction_data_length);

	return true;
}

/*
 * WaveFormatEx
 */

WaveFormatEx::WaveFormatEx ()
{
	codec_id = 0;
	channels = 0;
	samples_per_second = 0;
	bytes_per_second = 0;
	block_alignment = 0;
	bits_per_sample = 0;
	codec_specific_data_size = 0;
	codec_specific_data = NULL;
}

WaveFormatEx::~WaveFormatEx ()
{
	g_free (codec_specific_data);
	codec_specific_data = NULL;
}

bool
WaveFormatEx::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;

	VERIFY_AVAILABLE_SIZE (18);

	codec_id = source->ReadLE_U16 ();
	channels = source->ReadLE_U16 ();
	samples_per_second = source->ReadLE_U32 ();
	bytes_per_second = source->ReadLE_U32 ();
	block_alignment = source->ReadLE_U16 ();
	bits_per_sample = source->ReadLE_U16 ();
	codec_specific_data_size = source->ReadLE_U16 ();

	LOG_ASF ("WaveFormatEx::Read (): codec_id: %u channels: %u samples_per_second: %u bytes_per_second: %u block_alignment: %u bits_per_sample: %u codec_specific_data_size: %u\n",
		codec_id, channels, samples_per_second, bytes_per_second, block_alignment, bits_per_sample, codec_specific_data_size);

	VERIFY_AVAILABLE_SIZE (codec_specific_data_size);

	codec_specific_data = source->Read (codec_specific_data_size);

	return true;
}

/*
 * WaveFormatExtensible
 */

WaveFormatExtensible::WaveFormatExtensible ()
{
	Samples.valid_bits_per_sample = 0;
	channel_mask = 0;
}

WaveFormatExtensible::~WaveFormatExtensible ()
{
}

bool
WaveFormatExtensible::Read (ASFContext *context)
{
	MemoryBuffer *source = context->source;
	

	VERIFY_AVAILABLE_SIZE (40);

	if (!WaveFormatEx::Read (context)) {
		LOG_ASF ("WaveFormatExtensible::Read (): could not read WaveFormatEx\n");
		return false;
	}

	source->SeekOffset (-codec_specific_data_size);

	Samples.valid_bits_per_sample = source->ReadLE_U16 ();
	channel_mask = source->ReadLE_U32 ();

	if (!sub_format.Read (source)) {
		LOG_ASF ("WaveFormatExtensible::Read (): Could not read sub format.\n");
		return false;
	}

	LOG_ASF ("WaveFormatExtensible::Read () valid_bits_per_sample: %u channel_mask: %u sub_format: %s\n",
		Samples.valid_bits_per_sample, channel_mask, sub_format.ToString ());

	source->SeekOffset (codec_specific_data_size - 22);
	return true;
}

/*
 * ASFPacket
 */

ASFPacket::ASFPacket (ASFDemuxer *demuxer, MemoryBuffer *source, gint64 offset)
	: EventObject (Type::ASFPACKET)
{
	payloads = NULL;
	position = offset;
	index = demuxer->GetPacketIndex (offset);
	this->source = source;
	if (this->source)
		this->source->ref ();
	this->demuxer = demuxer;
	this->demuxer->ref ();
}

ASFPacket::~ASFPacket ()
{
	delete payloads;
	if (this->source)
		this->source->unref ();
	if (this->demuxer)
		this->demuxer->unref ();
}

guint32
ASFPacket::GetPayloadCount ()
{
	if (!payloads)
		return 0;
	
	return payloads->GetNumberOfPayloads ();
}

ASFSinglePayload *
ASFPacket::GetPayload (guint32 index /* 0 based */)
{
	if (index >= 0 && index < GetPayloadCount ())
		return payloads->payloads [index];
	
	return NULL;
}

bool
ASFPacket::Read ()
{
	ASFPayloadParsingInformation ppi;
	ASFErrorCorrectionData ecd;
	ASFContext context;
	ASFMultiplePayloads *mp;
	bool result;
	
	LOG_ASF ("ASFPacket::Read ()\n");

	context.demuxer = demuxer;
	context.source = this->source;

	if (!ecd.Read (&context)) {
		return false;
	}

	if (!ppi.Read (&context)) {
		return false;
	}
	
	mp = new ASFMultiplePayloads ();
	result = mp->Read (&context, &ecd, &ppi);
	if (!result) {
		delete mp;
		return result;
	}

	payloads = mp;

	return true;
}

/*
 *	ASFFrameReader
 */

ASFFrameReader::ASFFrameReader (ASFDemuxer *demuxer, guint32 stream_number, IMediaStream *stream)
{
	this->stream_number = stream_number;
	this->demuxer = demuxer;
	this->stream = stream;
	this->stream->ref ();
	first = NULL;
	last = NULL;
	data_counter = 0;
	size = 0;
	pts = 0;
	payloads = NULL;
	packet_index = 0;

	payloads_size = 0;
	payloads = NULL;

	index = NULL;
	index_size = 0;
}

ASFFrameReader::~ASFFrameReader ()
{
	RemoveAll ();

	if (payloads != NULL) {	
		for (int i = 0; payloads[i]; i++)
			delete payloads[i];
		
		g_free (payloads);
	}
	
	g_free (index);
	
	if (stream) {
		stream->unref ();
		stream = NULL;
	}
}

void
ASFFrameReader::Reset ()
{
	if (payloads != NULL) {
		for (int i = 0; payloads [i]; i++) {
			delete payloads [i];
			payloads [i] = NULL;
		}
	}
	RemoveAll ();
}

void
ASFFrameReader::AddFrameIndex ()
{
	gint64 packet_count = demuxer->GetPacketCount ();

	if (packet_index == G_MAXUINT64) {
		/* This stream isn't seekable (mms for instance) */
		return;
	}

	packet_count = demuxer->GetPacketCount ();
	/* Create the index. */
	if (index_size == 0) {
		if (packet_count > 0xFFFF) {
			/* This is some really huge file (or a corrupted file).
			 * Don't create any indices, since it would consume a whole lot of memory. */
			//printf ("ASFFrameReader::AddFrameIndex (): Not creating index, too many packets to track (%" G_GUINT64_FORMAT ")\n", packet_count);
			return;
		}

		/* Max size here is 0xFFFF packets * 16 bytes per index = 1.048.560 bytes */
		index_size = packet_count;

		/* Don't create any indices if there are no packets. */
		if (index_size == 0)
			return;

		index = (ASFFrameReaderIndex*) g_malloc0 (index_size * sizeof (ASFFrameReaderIndex));

		//printf ("ASFFrameReader::AddFrameIndex (): Created index: stream_count: %i, packet_count: %" G_GINT64_FORMAT ", index_size: %i, item size: %i, gives index size: %i bytes\n", stream_count, packet_count, index_size, sizeof (ASFFrameReaderIndex), index_size * sizeof (ASFFrameReaderIndex));

		if (index == NULL) {
			index_size = 0;
			return;
		}

		for (int i = 0; i < (int) packet_count; i++) {
			index [i].start_pts = INVALID_START_PTS;
		}
	}
 
	/* index_size can't be 0 here. */
	guint32 k = MIN (packet_index, index_size - 1);
	guint64 current_start = index [k].start_pts;
	index [k].start_pts = MIN (index [k].start_pts, Pts ());
	index [k].end_pts = MAX (index [k].end_pts, Pts ());
	if (k > 1 && current_start != INVALID_START_PTS) {
		index [k].start_pts = MAX (index [k - 1].end_pts, current_start);		
	}

	//printf ("ASFFrameReader::AddFrameIndex (%" G_GUINT64_FORMAT "). k = %u, start_pts = %" G_GUINT64_FORMAT ", end_pts = %" G_GUINT64_FORMAT ", stream = %i\n", packet_index, k, index [k].start_pts, index [k].end_pts, stream_number);
}

guint32
ASFFrameReader::FrameSearch (guint64 pts)
{
	for (guint32 i = 0; i < index_size; i++) {
		//printf ("ASFFrameReader::FrameSearch (%" G_GUINT64_FORMAT "): Checking start_pts: %" G_GUINT64_FORMAT ", end_pts: %" G_GUINT64_FORMAT ", pi: %i\n", pts, index [i].start_pts, index [i].end_pts, index [i].packet_index);
		
		if (index [i].start_pts == INVALID_START_PTS)
			continue; // This index isn't set
			
		if (index [i].start_pts > pts) {
			//printf ("ASFFrameReader::FrameSearch (%" G_GUINT64_FORMAT "): index not created for the desired pts (found starting pts after the requested one)\n", pts);
			return G_MAXUINT32;
		}
		
		if (index [i].start_pts <= pts && index [i].end_pts >= pts) {
			//printf ("ASFFrameReader::FrameSearch (%" G_GUINT64_FORMAT "): found packet index: %i.\n", pts, index [i].packet_index);
			return i;
		}
		
	}
	
	//printf ("ASFFrameReader::FrameSearch (%" G_GUINT64_FORMAT "d): searched entire index and didn't find anything.\n", pts);
			
	return -1;
}

bool
ASFFrameReader::ResizeList (guint32 size)
{
	ASFSinglePayload **new_list = NULL;
	size_t new_size;
	
	if (payloads_size >= size && size > 0)
		return true;
	
	// Allocate a new list
	new_size = sizeof (ASFSinglePayload *) * (size + 1);
	new_list = (ASFSinglePayload **) g_try_malloc (new_size);
	
	if (new_list == NULL) {
		return false;
	} else {
		memset (new_list, 0, new_size);
	}
	
	if (payloads != NULL) {
		// Copy the old list to the new one
		memcpy (new_list, payloads, payloads_size * sizeof (ASFSinglePayload *));
		g_free (payloads);
	}
	
	payloads = new_list;
	payloads_size = size;
	
	return true;
}

bool
ASFFrameReader::IsFrameAvailable (guint64 *pts, bool *is_key_frame)
{
	ASFFrameReaderData *current = NULL;
	ASFFrameReaderData *start;

	if (first == NULL)
		return false;

	start = first;
	
	if (demuxer->Eof ()) {
		/* We must have a complete frame available. */
	} else {
		/* Skip over any partial frames we might have */
		while (start != NULL && start->payload->offset_into_media_object != 0) {
			start = start->next;
		}
		
		if (start == NULL)
			return false;

		/* Check if we have any payloads for the next frame, if so, we have a complete frame available */
		current = start;
		do {
			if (current->next == NULL) {
				return false;
			}

			current = current->next;
		} while (current->payload->media_object_number == start->payload->media_object_number);
	}

	if (pts != NULL) {
		*pts = MilliSeconds_ToPts (start->payload->GetPresentationTime () - demuxer->GetPreroll ());
	}

	if (is_key_frame != NULL) {
		*is_key_frame = start->payload->is_key_frame;
	}

	return true;
}

bool
ASFFrameReader::SkipToKeyFrame (guint64 *pts)
{
	bool is_key_frame = false;

	do {
		if (first == NULL)
			return false;

		is_key_frame = false;
		if (!IsFrameAvailable (pts, &is_key_frame))
			return false;

		if (is_key_frame)
			return true;

		if (stream->IsAudio ())
			return true;

		Advance ();
	} while (true);
}

bool
ASFFrameReader::ContainsSubsequentKeyFrame (guint64 *pts)
{
	ASFFrameReaderData *current = first;
	guint32 first_media_object = G_MAXUINT32;

	/* Skip any incomplete frames we might have */
	while (current != NULL && current->payload->offset_into_media_object != 0) {
		current = current->next;
	}

	do {
		if (current == NULL)
			return false;

		if (current->payload->is_key_frame || stream->IsAudio ()) {
			if (first_media_object == G_MAXUINT32) {
				first_media_object = current->payload->media_object_number;
			} else if (first_media_object != current->payload->media_object_number) {
				*pts = MilliSeconds_ToPts (current->payload->GetPresentationTime () - demuxer->GetPreroll ());
				return true;
			}
		}
		current = current->next;
	} while (true);
}

void
ASFFrameReader::Advance ()
{
	guint32 payload_count = 0;
	guint32 media_object_number = 0;
	ASFFrameReaderData *current = NULL;

	LOG_ASF ("ASFFrameReader::Advance () IsFrameAvailable: %i\n", IsFrameAvailable (NULL, NULL));

	/* Don't assert that IsFrameAvailable, for markers IsFrameAvailable returns false, but AppendPayload has determined
	 * that we do have a full (marker) frame */

	/* Skip over any incomplete payloads we may have. This may happen when seeking. */
	while (first != NULL && first->payload->offset_into_media_object != 0) {
		LOG_ASF ("ASFFrameReader::Advance (): skipping incomplete frame, pts: %" G_GUINT64_FORMAT " ms, offset: %u\n",
			(guint64) (first->payload->presentation_time - demuxer->GetPreroll ()), first->payload->offset_into_media_object);
		Remove (first);
	}

	g_return_if_fail (first != NULL);

	/* Clear the current list of payloads. */
	if (payloads == NULL) {
		/*
		 * Most streams has at least once a media object spanning two payloads.
		 * so we allocate space for two (+ NULL at the end). 
		 */
		if (!ResizeList (2)) {
			demuxer->ReportErrorOccurred ("Could not resize list of payloads: out of memory.\n");
			return;
		}
	} else {
		/* Free all old payloads, they belong to the previous frame. */
		for (int i = 0; payloads [i]; i++) {
			delete payloads [i];
			payloads [i] = NULL;
		}
	}
	size = 0;
	pts = 0;

	current = first;
	media_object_number = first->payload->media_object_number;
	packet_index = first->packet_index;

	LOG_ASF ("ASFFrameReader::Advance (): frame data: size = %" G_GINT64_FORMAT ", key = %s, pts = %" G_GUINT64_FORMAT ", stream# = %d %s, media_object_number = %u.\n", 
		 size, IsKeyFrame () ? "true" : "false", Pts (), StreamNumber (), stream->GetTypeName (), media_object_number);

	/*
	 * Loop through payloads until we find a payload with the different media number
	 * than the first payload in the queue.
	 */
	while (current != NULL && current->payload->media_object_number == media_object_number) {
		LOG_ASF ("ASFFrameReader::Advance (): checking payload, stream: %d, media object number %d, size: %d\n",
			current->payload->stream_id, current->payload->media_object_number, current->payload->payload_data_length);

		ASFSinglePayload* payload = current->payload;
		guint64 current_pts = MilliSeconds_ToPts (payload->GetPresentationTime () - demuxer->GetPreroll ());

		/* add the payload to the current frame's payloads */
		payload_count++;
		if (payload_count == 1) {
			this->pts = current_pts;
#if SANITY
		} else if (this->pts != current_pts) {
			printf ("ASFFrameReader::Advance (): same media object number, but differen pts?\n");
#endif
		}
		size += payload->payload_data_length;
		if (payload_count > payloads_size) {
			if (!ResizeList (payload_count + 3)) {
				/* Out of memory, ResizeList reports the error */
				return;
			}
		}
		payloads [payload_count - 1] = payload;
		current->payload = NULL;
		
		// Remove it from the queue
		ASFFrameReaderData *tmp = current;
		current = current->next;
		Remove (tmp);
	}

/*
	LOG_ASF ("ASFFrameReader::Advance (): frame data: size = %.4lld, key = %s, pts = %.5llu, stream# = %i %s, media_object_number = %.3u.", 
		size, IsKeyFrame () ? "true " : "false", Pts (), StreamNumber (), stream->GetTypeName (), media_object_number);
*/

	AddFrameIndex ();
}

gint64
ASFFrameReader::EstimatePtsPosition  (guint64 pts)
{
	return demuxer->GetPacketOffset (MIN (demuxer->GetPacketCount () - 1, EstimatePacketIndexOfPts (pts) + 1));
}

guint64
ASFFrameReader::EstimatePacketIndexOfPts (guint64 pts)
{
	//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT ")\n", pts);
	
	gint32 counter = 0;
	guint64 average = 0; // average duration per packet
	guint64 last_good_pi = 0;
	guint64 last_good_pts = 0;
	guint64 duration = 0;
	guint64 total_duration = 0;
	guint64 result = 0;
	guint64 packet_index = 0;
	
	if (pts == 0) {
		return 0;
	}

	total_duration = demuxer->GetFileProperties ()->play_duration - MilliSeconds_ToPts (demuxer->GetPreroll ());
	if (pts >= total_duration) {
		return demuxer->GetPacketCount () - 1;
	}
	
	packet_index = FrameSearch (pts);
	
	if (packet_index != G_MAXUINT32) {
		//printf ("ASFFrameReader::GetPositionOfPts (%" G_GUINT64_FORMAT "): Found pts in index, position: %" G_GINT64_FORMAT ", pi: %i\n", pts, parser->GetPacketOffset (packet_index), packet_index);
		return packet_index;
	}
	
	for (guint32 i = 0; i < index_size; i++) {
		if (!(index [i].start_pts != INVALID_START_PTS && index [i].end_pts > index [i].start_pts))
			continue;
		
		if (index [i].start_pts >= pts)
			break;
		
		last_good_pi = i;
		last_good_pts = index [i].start_pts;
		
		duration = index [i].end_pts - index [i].start_pts;
		counter++;
		average = (average / (double) counter) * (counter - 1) + (duration / (double) counter);
			
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): Calculated average %" G_GUINT64_FORMAT " after pi: %i, duration: %" G_GUINT64_FORMAT ", start_pts: %" G_GUINT64_FORMAT ", end_pts: %" G_GUINT64_FORMAT "\n", pts, average, i, duration, index [i].start_pts, index [i].end_pts);
	}
	
	if (average == 0) {
		// calculate packet index from duration
		guint64 duration = MAX (1, demuxer->GetFileProperties ()->play_duration - MilliSeconds_ToPts (demuxer->GetPreroll ()));
		double percent = MAX (0, pts / (double) duration);
		result = percent * demuxer->GetPacketCount ();
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): No average, calculated by percent %.2f, pi: %i, pts: %" G_GUINT64_FORMAT ", preroll: %" G_GUINT64_FORMAT "\n", pts, percent, pi, pts, preroll);
	} else {
		// calculate packet index from the last known packet index / pts and average pts per packet index
		last_good_pts = MIN (last_good_pts, pts);
		result = last_good_pi + (pts - last_good_pts) / average;
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): Calculated by averate %" G_GUINT64_FORMAT ", last_good_pts: %" G_GUINT64_FORMAT ", pi: %i\n", pts, average, last_good_pts, pi);
	}
	
	result = MAX (0, result);
	result = MIN (result, MAX (0, demuxer->GetPacketCount () - 1));
	
	//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): Final position: %" G_GINT64_FORMAT " of pi: %i. Total packets: %" G_GUINT64_FORMAT ", total duration: %" G_GUINT64_FORMAT "\n", pts, parser->GetPacketOffset (pi), pi, parser->GetFileProperties ()->data_packet_count, parser->GetFileProperties ()->play_duration);
	return result;
}

void
ASFFrameReader::AppendPayload (ASFSinglePayload *payload, guint64 packet_index)
{
	LOG_ASF ("ASFFrameReader::AppendPayload (%" G_GUINT64_FORMAT "). %s #%i KeyFrame: %i pts: %" G_GUINT64_FORMAT " ms (pre) Count: %i\n", 
		packet_index, stream->GetTypeName (), StreamNumber (), payload->is_key_frame, payload->GetPresentationTime () - demuxer->GetPreroll (), data_counter);

	ASFFrameReaderData* node = new ASFFrameReaderData (payload);
	node->packet_index = packet_index;
	if (first == NULL) {
		first = node;
		last = node;
		data_counter = 1;
	} else {
		node->prev = last;
		last->next = node;
		last = node;
		data_counter++;
	}

	if (stream->IsMarker ()) {
		/* ASF frames has one deficiency: the only way to know if we have a complete frame is to get data for
		 * the next frame. This isn't a problem for audio/video, but it is a problem for markers where we might
		 * not get more frames. Fortunately we can detect if we have a full frame for a marker: the marker consists
		 * of two strings, each terminated with a NULL WCHAR. So we have a full frame when we've found two NULL
		 * (nonconsecutive) WCHARs. */

		ASFFrameReaderData *current;
		int zeroes = 0;
		int nulls = 0;
		int n = 0;

		/* Check all the payloads in the queue for NULL WCHARs (which would be 2 zeroed bytes). This is slightly
		 * complicated since a WCHAR may end up split between payloads. */
		current = first;
		while (current != NULL && nulls < 2) {
			char *ptr = (char *) current->payload->payload_data;
			for (guint32 i = 0; i < current->payload->payload_data_length; i++) {
				/* Keep a total of the number of bytes processed so that we know when a WCHAR begins */
				n++;

				if (ptr [i] != 0) {
					zeroes = 0;
					continue;
				}

				/* we only want to start counting zeros when a WCHAR begins, i.e. n%2 = 0 */
				if (zeroes == 0 && n % 2 != 0) {
					zeroes = 0;
					continue;
				}

				if (zeroes == 1) {
					zeroes = 0;
					nulls++;
					if (nulls == 2)
						break;
				} else {
					zeroes++;
				}
			}
			current = current->next;
		}

		if (nulls == 2) {
			/* We have an entire marker */
			Advance ();
			MarkerStream *marker_stream = (MarkerStream *) stream;
			MediaFrame *frame = new MediaFrame (marker_stream);
			frame->pts = Pts ();
			if (frame->AllocateBuffer (Size ())) {
				Write (frame->GetBuffer ());
				marker_stream->MarkerFound (frame);
			} else {
				/* Not sure if there is anything to do here, an error has already been reported (by AllocateBuffer) */
			}
			frame->unref ();
		} else {
			/* Wait for next payload */
		}
	}

#if 0
	int counter = 0;
	node = first;
	while (node != NULL) {
		counter++;
		node = node->next;
	}
	printf ("ASFFrameReader::AppendPayload (%p, %" G_GUINT64_FORMAT "). Stream #%i now has %i payloads.\n", payload, packet_index, StreamId (), counter);
#endif
}

bool
ASFFrameReader::Write (void *dest)
{
	if (payloads == NULL)
		return false;
	
	for (int i = 0; payloads[i]; i++) {
		memcpy (dest, payloads[i]->payload_data, payloads[i]->payload_data_length);
		dest = ((char *) dest) + payloads[i]->payload_data_length;
	}
	
	return true;
}

void 
ASFFrameReader::RemoveAll ()
{
	ASFFrameReaderData* current = first, *next = NULL;
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
	first = NULL;
	last = NULL;
	data_counter = 0;
}

void
ASFFrameReader::Remove (ASFFrameReaderData* data)
{
	if (data->prev != NULL)
		data->prev->next = data->next;
	
	if (data->next != NULL)
		data->next->prev = data->prev;
	
	if (data == first)
		first = data->next;
	
	if (data == last)
		last = data->prev;
	
	data_counter--;
	
	delete data;
}

};
