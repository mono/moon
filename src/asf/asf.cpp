/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * asf.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "asf.h"
#include "debug.h"
#include "clock.h"
#include "mediaelement.h"

/*
	ASFParser
*/

ASFParser::ASFParser (IMediaSource *source, Media *media)
	: EventObject (Type::ASFPARSER)
{
	LOG_ASF ("ASFParser::ASFParser ('%p'), this: %p.\n", source, this);
	this->media = NULL;
	this->source = NULL;
	
	g_return_if_fail (media != NULL);
	g_return_if_fail (source != NULL);
	
	this->source = source;
	this->source->ref ();
	this->media = media;

	Initialize ();
}

guint64
ASFParser::GetPacketCount ()
{
	return file_properties->data_packet_count;
}

Media*
ASFParser::GetMedia ()
{
	return media;
}

int 
ASFParser::GetSequentialStreamNumber (int stream_index)
{
	int result = 0;
	for (int i = 1; i <= stream_index; i++) {
		if (IsValidStream (i))
			result++;
	}
	return result;
}

int
ASFParser::GetStreamCount ()
{
	int result = 0;
	for (int i = 1; i <= 127; i++) {
		if (IsValidStream	 (i))
			result++;
	}
	return result;
}

void
ASFParser::Initialize ()
{
	LOG_ASF ("ASFParser::Initialize ()\n");
	header = NULL;
	data = NULL;
	data_offset = 0;
	packet_offset = 0;
	packet_offset_end = 0;
	header_objects = NULL;
	header_extension = NULL;
	script_command = NULL;
	marker = NULL;
	file_properties = NULL;
	error = NULL;
	header_read_successfully = false;
	embedded_script_command = NULL;
	embedded_script_command_state = NULL;
	memset (stream_properties, 0, sizeof (asf_stream_properties *) * 127);
	memset (extended_stream_properties, 0, sizeof (asf_extended_stream_properties *) * 127);
}

ASFParser::~ASFParser ()
{
	LOG_ASF ("ASFParser::~ASFParser ().\n");
	
	if (source)
		source->unref ();
	if (error)
		error->unref ();
	g_free (header);
	g_free (data);
	
	if (header_objects) {
		for (int i = 0; header_objects[i]; i++)
			g_free (header_objects[i]);
		
		g_free (header_objects);
	}
}

bool 
ASFParser::ReadEncoded (IMediaSource *source, guint32 length, guint32 *dest)
{
	guint16 result2 = 0;
	guint8 result1 = 0;
	
	switch (length) {
	case 0x00:
		return true;
	case 0x01: 
		if (!source->ReadAll (&result1, 1))
			return false;
		*dest = result1;
		return true;
	case 0x02:
		if (!source->ReadAll (&result2, 2))
			return false;
		*dest = result2;
		return true;
	case 0x03:
		return source->ReadAll (dest, 4);
	default:
		//TODO: parser->AddError (g_strdup_printf ("Invalid read length: %i.", length));
		return false;
	}
}

bool
ASFParser::VerifyHeaderDataSize (guint32 size)
{
	if (header == NULL)
		return false;
	
	return size >= 0 && size < header->size;
}

void *
ASFParser::MallocVerified (guint32 size)
{
	void *result = g_try_malloc0 (size);
	
	if (result == NULL)
		AddError ("Out of memory.");
	
	return result;
}

void *
ASFParser::Malloc (guint32 size)
{
	if (!VerifyHeaderDataSize (size))
		return NULL;
	
	return MallocVerified (size);
}

asf_object *
ASFParser::ReadObject (asf_object *obj)
{
	asf_object *result = NULL;
	char *guid;
	
	LOG_ASF ("ASFParser::ReadObject ('%s', %" G_GUINT64_FORMAT ")\n", asf_guid_tostring (&obj->id), obj->size);

	if (obj->size < sizeof (asf_object) || obj->size > ASF_OBJECT_MAX_SIZE) {
		AddError (g_strdup_printf ("Header corrupted (invalid size: %" G_GUINT64_FORMAT ")", obj->size));
		return NULL;
	}
	
	result = (asf_object *) Malloc (obj->size);
	if (result == NULL) {
		guid = asf_guid_tostring (&obj->id);
		AddError (g_strdup_printf ("Header corrupted (id: %s)", guid));
		g_free (guid);
		return NULL;
	}
	
	memcpy (result, obj, sizeof (asf_object));
	
	if (obj->size > sizeof (asf_object)) {
		if (!source->ReadAll (((char *) result) + sizeof (asf_object), obj->size - sizeof (asf_object))) {
			g_free (result);
			return NULL;
		}
	}
	
	if (!asf_object_validate_exact (result, this)) {
		g_free (result);
		return NULL;
	}
	
	return result;
}

MediaResult
ASFParser::ReadPacket (ASFPacket **packet, int packet_index)
{
	bool eof = false;
	
	LOG_ASF ("ASFParser::ReadPacket (%s, %d) at %" G_GUINT64_FORMAT ".\n", packet ? "non-null" : "null", packet_index, GetPacketOffset (packet_index));

	if (packet_index >= 0) {
		gint64 packet_count = GetPacketCount ();
		if (packet_count > 0 && packet_count < packet_index + 1)
			return MEDIA_NO_MORE_DATA;
		
		gint64 position = GetPacketOffset (packet_index);

		if (!source->IsPositionAvailable (position + GetPacketSize (), &eof))
			return eof ? MEDIA_NO_MORE_DATA : MEDIA_NOT_ENOUGH_DATA;

		LOG_ASF ("ASFParser::ReadPacket (%p, %i): determined that position %" G_GINT64_FORMAT " + size %i = %" G_GINT64_FORMAT " is available.\n",
			packet, packet_index, position, GetPacketSize (), position + GetPacketSize ());
		if (position == 0 || (source->GetPosition () != position))
			source->Seek (position, SEEK_SET);
	}

	return ASFParser::ReadPacket (packet);
}

bool
ASFParser::IsDrm ()
{
	for (int i = 0; header_objects [i] != NULL; i++) {
		if (asf_guid_compare (&header_objects [i]->id, &asf_guids_content_encryption)) {
			return true;
		} else if (asf_guid_compare (&header_objects [i]->id, &asf_guids_extended_content_encryption)) {
			return true;
		}
	}

	return false;
}

MediaResult
ASFParser::ReadPacket (ASFPacket **packet)
{
	MediaResult result;
	MmsPlaylistEntry *mqs;
	gint64 initial_position;
	gint64 next_pos;
	gint64 pi;

	*packet = NULL;
	
	if (source->GetType () == MediaSourceTypeMmsEntry) {
		mqs = (MmsPlaylistEntry *) source;
		*packet = mqs->Pop ();
				
		if (*packet == NULL) {
			LOG_ASF ("ASFParser::ReadPacket (%p): no more data in queue source (finished: %i).\n", packet, mqs->IsFinished ());
			return mqs->IsFinished () ? MEDIA_NO_MORE_DATA : MEDIA_BUFFER_UNDERFLOW;
		}

		return MEDIA_SUCCESS;
	}
	
	initial_position = source->GetPosition ();
	pi = GetPacketIndex (initial_position);
	next_pos = GetPacketOffset (1 + pi);

	LOG_ASF ("ASFParser::ReadPacket (%s): Reading packet at %" G_GINT64_FORMAT " (index: %" G_GINT64_FORMAT ") of %" G_GINT64_FORMAT " packets.\n",
		 packet ? "non-null" : "null", initial_position, pi, data->data_packet_count);
	
	
	*packet = new ASFPacket (this, source);

	result = (*packet)->Read ();
	if (!MEDIA_SUCCEEDED (result)) {
		source->Seek (next_pos, SEEK_SET);
		return result;
	}
	
	/* If we are "positioned", need to seek to the start of the next packet */

	if (source->GetType () != MediaSourceTypeMemory)
		source->Seek (next_pos, SEEK_SET);

	return result;
}

MediaResult
ASFParser::ReadData ()
{
	LOG_ASF ("ASFParser::ReadData ().\n");
	
	if (this->data != NULL) {
		AddError ("ReadData has already been called.");
		return MEDIA_FAIL;
	}

#if DEBUG
	// We should already be positioned at where the data is
	if (source->CanSeek () && source->GetPosition () != (gint64) header->size) {
		fprintf (stderr, "Moonlight assert failure, asf source isn't positioned correctly.\n");
	}
#endif
	
	data = (asf_data *) Malloc (sizeof (asf_data));
	if (data == NULL) {
		AddError ("Data corruption in data.");
		return MEDIA_FAIL;
	}
	
	if (!source->ReadAll (data, sizeof (asf_data))) {
		g_free (data);
		data = NULL;
		return MEDIA_FAIL;
	}
	
	asf_object_dump_exact (data);
	
	LOG_ASF ("Data %p has %" G_GINT64_FORMAT " packets.\n", data, data->data_packet_count);
	
	this->data = data;
	
	return MEDIA_SUCCESS;
}

MediaResult
ASFParser::ReadHeader ()
{
	bool eof = false;
	
	LOG_ASF ("ASFParser::ReadHeader (), header_read_successfully: %i\n", header_read_successfully);

	if (header_read_successfully)
		return MEDIA_SUCCESS;
				
	header = (asf_header *) MallocVerified (sizeof (asf_header));
	if (header == NULL) {
		LOG_ASF ("ASFParser::ReadHeader (): Malloc failed.\n");
		return MEDIA_FAIL;
	}

	if (!source->IsPositionAvailable (sizeof (asf_header), &eof)) {
		LOG_ASF ("ASFParser::ReadHeader (): Not enough data, eof: %i, requested pos: %i, actual available pos: %i\n", eof, (int) sizeof (asf_header), (int) source->GetLastAvailablePosition ());
		return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;
	}

	if (!source->Peek (header, sizeof (asf_header))) {
		LOG_ASF ("ASFParser::ReadHeader (): source->Read () failed.\n");
		return MEDIA_FAIL;
	}

	asf_header_dump (header);

	// We are not allowed to leave the stream with a position > 0 if we return MEDIA_NOT_ENOUGH_DATA,
	// so check if there is enough data to read both the header object and the data object header here,
	// which is what we need to read in order to return true from this method. This way we avoid peeking
	// after this point, we just read ahead.
	if (!source->IsPositionAvailable (header->size + sizeof (asf_data), &eof)) {
		LOG_ASF ("ASFParser::ReadHeader (): Not enough data, eof: %i, requested pos: %i, actual available pos: %i\n", eof, (int) (header->size + sizeof (asf_data)), (int) source->GetLastAvailablePosition ());
		return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;
	}
	
	if (!asf_header_validate (header, this)) {
		LOG_ASF ("Header validation failed, error: '%s'\n", GetLastErrorStr ());
		return MEDIA_FAIL;
	}
	
	header_objects = (asf_object **) Malloc ((header->object_count + 1) * sizeof (asf_object*));
	if (header_objects == NULL) {
		AddError ("Data corruption in header.");
		return MEDIA_FAIL;
	}
	
	LOG_ASF ("ASFParser::ReadHeader (): about to read streams...\n");
	if (!source->ReadAll (header, sizeof (asf_header))) {
		// We just peeked the header above, read it for real.
		LOG_ASF ("ASFParser::ReadHeader (): re-reading header failed.\n");
		return MEDIA_FAIL;
	}
	
	bool any_streams = false;
	for (guint32 i = 0; i < header->object_count; i++) {
		asf_object tmp;
		
		if (!source->ReadAll (&tmp, sizeof (asf_object)))
			return MEDIA_FAIL;
		
		if (!(header_objects [i] = ReadObject (&tmp)))
			return MEDIA_FAIL;
		
		if (asf_guid_compare (&asf_guids_stream_properties, &header_objects[i]->id)) {
			asf_stream_properties *stream = (asf_stream_properties *) header_objects[i];
			SetStream (stream->get_stream_id (), stream);
			any_streams = true;
		}
		
		if (asf_guid_compare (&asf_guids_file_properties, &header_objects [i]->id)) {
			if (file_properties != NULL) {
				AddError ("Multiple file property object in the asf data.");
				return MEDIA_FAIL;
			}
			file_properties = (asf_file_properties*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_header_extension, &header_objects [i]->id)) {
			if (header_extension != NULL) {
				AddError ("Multiple header extension objects in the asf data.");
				return MEDIA_FAIL;
			}
			header_extension = (asf_header_extension*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_marker, &header_objects [i]->id)) {
			if (marker != NULL) {
				AddError ("Multiple marker objects in the asf data.");
				return MEDIA_FAIL;
			}
			marker = (asf_marker*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_script_command, &header_objects [i]->id)) {
			if (script_command != NULL) {
				AddError ("Multiple script command objects in the asf data.");
				return MEDIA_FAIL;
			}
			script_command = (asf_script_command*) header_objects [i];
		}
		
		asf_object_dump_exact (header_objects [i]);
	}

	// Check if there are stream properties in any extended stream properties object.
	if (header_extension != NULL) {
		asf_object **objects = header_extension->get_objects ();
		for (int i = 0; objects != NULL && objects [i] != NULL; i++) {
			if (asf_guid_compare (&asf_guids_extended_stream_properties, &objects [i]->id)) {
				asf_extended_stream_properties *aesp = (asf_extended_stream_properties *) objects [i];
				SetExtendedStream (aesp->stream_id, aesp);
				const asf_stream_properties *stream = aesp->get_stream_properties ();
				if (stream != NULL) {
					if (stream->get_stream_id () != aesp->stream_id) {
						g_free (objects);
						AddError ("There is an invalid extended stream properties object (it contains a stream properties object whose stream id doesn't match the stream id of the extended stream properties object).");
						return MEDIA_FAIL;
					} else {
						SetStream (stream->get_stream_id (), stream);
					}
				} else if (!IsValidStream (aesp->stream_id)) {
					g_free (objects);
					AddError ("There is an extended stream properties object that doesn't have a corresponding strem properties object.");
					return MEDIA_FAIL;
				}
				any_streams = true;
			}
		}
		g_free (objects);
	}

	// Check for required objects.
	
	if (file_properties == NULL) {
		AddError ("No file property object in the asf data.");
		return MEDIA_FAIL;
	}
	
	if (header_extension == NULL) {
		AddError ("No header extension object in the asf data.");
		return MEDIA_FAIL;
	}
	
	if (!any_streams) {
		AddError ("No streams in the asf data.");
		return MEDIA_FAIL;
	}
	
	data_offset = header->size;
	packet_offset = data_offset + sizeof (asf_data);
	if (file_properties->data_packet_count != 0)
		packet_offset_end = packet_offset + file_properties->data_packet_count * file_properties->min_packet_size;
	else
		packet_offset_end = -1;

	LOG_ASF ("ASFParser::ReadHeader (): Header read successfully, position: %" G_GINT64_FORMAT ", header size: %" G_GINT64_FORMAT "\n", source->GetPosition (), header->size);
	
	if (!MEDIA_SUCCEEDED (ReadData ()))
		return MEDIA_FAIL;
		
	LOG_ASF ("ASFParser::ReadHeader (): Header read successfully [2].\n");
	
	header_read_successfully = true;	
	
	return MEDIA_SUCCESS;
}

void
ASFParser::SetSource (IMediaSource *source)
{
	if (this->source)
		this->source->unref ();
	this->source = source;
	if (this->source)
		this->source->ref ();
}

ErrorEventArgs *
ASFParser::GetLastError ()
{
	return error;
}

const char *
ASFParser::GetLastErrorStr ()
{
	return error ? error->GetErrorMessage() : "";
}

void
ASFParser::AddError (const char *msg)
{
	AddError (MEDIA_CORRUPTED_MEDIA, msg);
}

void
ASFParser::AddError (char *msg)
{
	AddError (MEDIA_CORRUPTED_MEDIA, msg);
}

void
ASFParser::AddError (MediaResult code, const char *msg)
{
	AddError (code, g_strdup (msg));
}

void
ASFParser::AddError (MediaResult code, char *msg)
{
	fprintf (stdout, "ASF error: %s.\n", msg);
	
	if (error == NULL && media)
		media->ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 4001, msg)));

	g_free (msg);
}

const asf_stream_properties* 
ASFParser::GetStream (int stream_index)
{
	if (stream_index < 1 || stream_index > 127)
		return NULL;
	
	return stream_properties [stream_index - 1];
}

const asf_extended_stream_properties* 
ASFParser::GetExtendedStream (int stream_index)
{
	if (stream_index < 1 || stream_index > 127)
		return NULL;
	
	return extended_stream_properties [stream_index - 1];
}

void
ASFParser::SetStream (int stream_index, const asf_stream_properties *stream)
{
	if (stream_index < 1 || stream_index > 127) {
		printf ("ASFParser::SetStream (%i, %p): Invalid stream index.\n", stream_index, stream);
		return;
	}
	
	stream_properties [stream_index - 1] = stream;
}

void
ASFParser::SetExtendedStream (int stream_index, const asf_extended_stream_properties *stream)
{
	if (stream_index < 1 || stream_index > 127) {
		printf ("ASFParser::SetExtendedStream (%i, %p): Invalid stream index.\n", stream_index, stream);
		return;
	}
	
	extended_stream_properties [stream_index - 1] = stream;
}

bool
ASFParser::IsValidStream (int stream_index)
{
	return GetStream (stream_index) != NULL;
}

gint64
ASFParser::GetPacketOffset (guint64 packet_index)
{
	if (packet_index < 0 || (file_properties->data_packet_count > 0 && packet_index >= file_properties->data_packet_count)) {
//		AddError (g_strdup_printf ("ASFParser::GetPacketOffset (%i): Invalid packet index (there are %" G_GUINT64_FORMAT " packets).", packet_index, file_properties->data_packet_count)); 
		return 0;
	}
	
	// CHECK: what if min_packet_size != max_packet_size?
	return packet_offset + packet_index * file_properties->min_packet_size;
}

guint32
ASFParser::GetPacketSize ()
{
	return file_properties->min_packet_size;
}

guint64
ASFParser::GetPacketIndex (gint64 offset)
{
	if (offset < packet_offset)
		return 0;
	
	if (packet_offset_end > 0 && offset > packet_offset_end)
		return file_properties->data_packet_count - 1;
	
	return (offset - packet_offset) / file_properties->min_packet_size;
}

asf_header *
ASFParser::GetHeader ()
{
	return header;
}

asf_file_properties *
ASFParser::GetFileProperties ()
{ 
	return file_properties;
}

asf_object *
ASFParser::GetHeaderObject (const asf_guid *guid)
{
	int index = GetHeaderObjectIndex (guid);
	
	if (index >= 0) {
		return header_objects [index];
	} else {
		return NULL;
	}
}

int 
ASFParser::GetHeaderObjectIndex (const asf_guid *guid, int start)
{
	int i = start;
	
	if (i < 0)
		return -1;
		
	while (header_objects [i] != NULL) {
		if (asf_guid_compare (guid, &header_objects [i]->id))
			return i;
	
		i++;
	}
	
	return -1;
}

asf_object *
ASFParser::GetHeader (int index) 
{
	if (index < 0)
		return NULL;
	
	return header_objects [index];
}

/*
	ASFPacket
*/

ASFPacket::ASFPacket (ASFParser *parser, IMediaSource *source)
	: EventObject (Type::ASFPACKET)
{
	payloads = NULL;
	position = -1;
	index = -1;
	this->source = source;
	if (this->source)
		this->source->ref ();
	this->parser = parser;
	if (this->parser)
		this->parser->ref ();
}

ASFPacket::~ASFPacket ()
{
	delete payloads;
	if (this->source)
		this->source->unref ();
	if (this->parser)
		this->parser->unref ();
}

void
ASFPacket::SetSource (IMediaSource *source)
{
	if (this->source != NULL)
		this->source->unref ();
	this->source = source;
	if (this->source != NULL)
		this->source->ref ();
}

int
ASFPacket::GetPayloadCount ()
{
	if (!payloads)
		return 0;
	
	return payloads->get_number_of_payloads ();
}

asf_single_payload *
ASFPacket::GetPayload (int index /* 0 based */)
{
	if (index >= 0 && index < GetPayloadCount ())
		return payloads->payloads [index];
	
	return NULL;
}

guint64
ASFPacket::GetPts (int stream_id)
{
	asf_single_payload *first;
	
	if (!payloads)
		return 0;
	
	if (!(first = GetFirstPayload (stream_id)))
		return 0;
	
	return first->get_presentation_time ();
}

asf_single_payload *
ASFPacket::GetFirstPayload (int stream_id /* 1 - 127 */)
{
	if (!payloads)
		return NULL;
	
	int index = 0;
	while (payloads->payloads [index] != NULL) {
		if (payloads->payloads [index]->stream_id == stream_id)
			return payloads->payloads [index];
		index++;
	}
	
	return NULL;
}

MediaResult
ASFPacket::Read ()
{
	MediaResult result;
	asf_payload_parsing_information ppi;
	asf_error_correction_data ecd;
	ASFContext context;
		
	LOG_ASF ("ASFPacket::Read (): source: %s, source position: %" G_GINT64_FORMAT "\n", source->ToString (), source->GetPosition ());
	
	context.parser = parser;
	context.source = this->source;
	
	result = ecd.FillInAll (&context);
	if (!MEDIA_SUCCEEDED (result))
		return result;
	
	asf_error_correction_data_dump (&ecd);
	
	result = ppi.FillInAll (&context);
	if (!MEDIA_SUCCEEDED (result)) {
		ASF_LOG_ERROR ("ASFPacket::Read (): FillIn payload parsing information failed.\n");
		return result;
	}
	
	asf_payload_parsing_information_dump (&ppi);
	
	asf_multiple_payloads *mp = new asf_multiple_payloads ();
	result = mp->FillInAll (&context, &ecd, ppi);
	if (!MEDIA_SUCCEEDED (result)) {
		ASF_LOG_ERROR ("ASFPacket::Read (): FillIn multiple payloads failed, current position: %" G_GINT64_FORMAT ", in stream %s\n", source->GetPosition (), source->ToString ());
		delete mp;
		return result;
	}
	
	payloads = mp;
	
	return MEDIA_SUCCESS;
}

/*
 * ASFReader
 */

ASFReader::ASFReader (ASFParser *parser, ASFDemuxer *demuxer)
{
	this->parser = parser;
	this->demuxer = demuxer;
	this->source = demuxer->GetSource ();
	next_packet_index = 0;
	memset (readers, 0, sizeof (ASFFrameReader*) * 128);
}

ASFReader::~ASFReader ()
{
	for (int i = 0; i < 128; i++)
		delete readers [i];
}

void
ASFReader::SelectStream (gint32 stream_index, bool value)
{
	LOG_ASF ("ASFReader::SelectStream (%i, %i)\n", stream_index, value);	

	if (stream_index <= 0 || stream_index >= 128) {
		fprintf (stderr, "ASFReader::SelectStream (%i, %i): Invalid stream index\n", stream_index, value);
		return;
	}

	if (value) {
		if (readers [stream_index] == NULL) {
			readers [stream_index] = new ASFFrameReader (parser, stream_index, demuxer, this, demuxer->GetStreamOfASFIndex (stream_index));
		}
	} else {
		if (readers [stream_index] != NULL) {
			delete readers [stream_index];
			readers [stream_index] = NULL;
		}
	}
}

ASFFrameReader *
ASFReader::GetFrameReader (gint32 stream_index)
{
	if (stream_index <= 0 || stream_index >= 128) {
		fprintf (stderr, "ASFReader::GetFrameReader (%i): Invalid stream index.\n", stream_index);
		return NULL;
	}

	return readers [stream_index];
}

MediaResult
ASFReader::TryReadMore ()
{
	LOG_ASF ("ASFReader::TryReadMore (), source: %s, next_packet_index: %i\n", source->ToString (), (int) next_packet_index);
	
	int payloads_added = 0;
	guint64 current_packet_index = 0;
	gint64 position, last_available_position;
	MediaResult read_result = MEDIA_FAIL;
	ASFPacket* packet = NULL;
	
	g_return_val_if_fail (parser != NULL, MEDIA_FAIL);
	g_return_val_if_fail (parser->GetMedia () != NULL, MEDIA_FAIL);
	
#if SANITY
	if (!parser->GetMedia ()->InMediaThread ())
		printf ("ASFReader::TryReadMore (): This method should only be called on the media thread (media id: %i).\n", GET_OBJ_ID (parser->GetMedia ()));
#endif
	
	do {
		if (Eof ()) {
			LOG_ASF ("ASFReader::ReadMore (): eof\n");
			return MEDIA_NO_MORE_DATA;
		}
		
		LOG_ASF ("ASFReader::TryReadMore (), current_packet_index: %" G_GINT64_FORMAT ", next_packet_index: %" G_GINT64_FORMAT "\n", current_packet_index, next_packet_index);
			
		if (source->GetType () == MediaSourceTypeMmsEntry) {
			read_result = parser->ReadPacket (&packet);
		} else if (source->CanSeek ()) {
			position = source->GetPosition ();
			last_available_position = source->GetLastAvailablePosition ();
			if (last_available_position != -1 && position + parser->GetPacketSize () > last_available_position) {
				LOG_ASF ("ASFReader::TryReadMore (), position: %" G_GINT64_FORMAT ", last_available_position: %" G_GINT64_FORMAT ", packet size: %i\n", position, last_available_position, parser->GetPacketSize ());
				return MEDIA_BUFFER_UNDERFLOW;
			}
			LOG_ASF ("ASFReader::TryReadMore (), position: %" G_GINT64_FORMAT ", last_available_position: %" G_GINT64_FORMAT ", packet size: %i, current packet index: %" G_GINT64_FORMAT " [READING]\n", position, last_available_position, parser->GetPacketSize (), next_packet_index);
		
			read_result = parser->ReadPacket (&packet, next_packet_index);
		} else {
			// We should either be seekable, or be MediaSourceTypeMmsEntry.
			fprintf (stderr, "Moonlight: Media assert failure (source should be either MmsSource or seekable). Media playback errors will probably occur.\n");
			return MEDIA_FAIL;
		}

		if (read_result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_ASF ("ASFReader::ReadMore (): Not enough data.\n");
			if (packet)
				packet->unref ();
			return read_result;
		}

		current_packet_index = next_packet_index;		
		next_packet_index++;

		LOG_ASF ("ASFReader::ReadMore (): current packet index: %" G_GUINT64_FORMAT ", position: %" G_GINT64_FORMAT ", calculated packet index: %" G_GUINT64_FORMAT "\n", 
				current_packet_index, source->GetPosition (), parser->GetPacketIndex (source->GetPosition ()));

		if (read_result == MEDIA_INVALID_DATA) {
			LOG_ASF ("ASFReader::ReadMore (): Skipping invalid packet (index: %" G_GUINT64_FORMAT ")\n", current_packet_index);
			if (packet)
				packet->unref ();
			continue;
		}

		if (!MEDIA_SUCCEEDED (read_result)) {
			LOG_ASF ("ASFReader::ReadMore (): could not read more packets (error: %i)\n", (int) read_result);
			if (packet)
				packet->unref ();
			return read_result;
		}
		
		// Distribute the payloads to the per-stream readers.
		asf_single_payload** payloads = packet->payloads->steal_payloads ();
		ASFFrameReader *reader;
		int i = -1;
		int stream_id;

		while (payloads [++i] != NULL) {
			stream_id = payloads [i]->stream_id;
			reader = GetFrameReader (stream_id);
			if (reader == NULL) {
				LOG_ASF ("ASFReader::ReadMore (): skipped, stream: %i, added pts: %" G_GUINT64_FORMAT "\n", payloads [i]->stream_id, (guint64) payloads [i]->get_presentation_time ());
				delete payloads [i];
				continue;
			}
			
			LOG_ASF ("ASFReader::ReadMore (): delivered payload for stream %i with pts %" G_GUINT64_FORMAT "\n", payloads [i]->stream_id, (guint64) payloads [i]->get_presentation_time () - 5000);
			reader->AppendPayload (payloads [i], current_packet_index);
			payloads_added++;
		}
		g_free (payloads);

		LOG_ASF ("ASFReader::ReadMore (): read %d payloads.\n", payloads_added);
	
		packet->unref ();
	} while (payloads_added == 0);
	
	return MEDIA_SUCCESS;
}

bool
ASFReader::Eof ()
{
	guint64 packet_count = parser->GetPacketCount ();

	if (packet_count == 0)
		return false;

	if (source->GetType () == MediaSourceTypeMms || source->GetType () == MediaSourceTypeMmsEntry)
		return source->Eof ();
		
	if (source->GetSize () <= 0)
		return false;

	return (source->GetPosition() >= source->GetSize () ||
	        next_packet_index >= packet_count);

}

void
ASFReader::ResetAll ()
{
	for (int i = 0; i < 128; i++) {
		if (readers [i] != NULL)
			readers [i]->Reset ();
	}
}

guint64
ASFReader::EstimatePacketIndexOfPts (guint64 pts)
{
	guint64 result = G_MAXUINT64;
	for (int i = 0; i < 128; i++) {
		if (readers [i] == NULL)
			continue;

		result = MIN (readers [i]->EstimatePacketIndexOfPts (pts), result);
	}
	return result == G_MAXUINT64 ? 0 : result;
}

MediaResult
ASFReader::SeekToPts (guint64 pts)
{
	ResetAll ();

	return source->SeekToPts (pts);
}

MediaResult
ASFReader::Seek (guint64 pts)
{
	LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "), CanSeek: %i, CanSeekToPts(): %i\n", pts, CanSeek (), source->CanSeekToPts ());
	
#if SANITY
	if (!parser->GetMedia ()->InMediaThread ())
		printf ("ASFReader::TryReadMore (): This method should only be called on the media thread.\n");
#endif

	if (!CanSeek ())
		return MEDIA_FAIL;

	if (source->CanSeekToPts ())
		return SeekToPts (pts);
	
	// We know 0 is at the beginning of the media, so just optimize this case slightly
	if (pts == 0) {
		ResetAll ();
		next_packet_index = 0;
		return MEDIA_SUCCESS;
	}

	// For each stream we need to find a keyframe whose pts is below the requested one.
	// Read a packet, and check each payload for keyframes. If we don't find one, read 
	// the previous packet, and check again.

	MediaResult result = MEDIA_FAIL;
	guint64 start_pi = EstimatePacketIndexOfPts (pts); // The packet index we start testing for key frames.
	guint64 tested_counter = 0; // The number of packet indices we have tested.
	guint64 test_pi = 0; // The packet index we're currently testing.
	bool found_all_highest = false;
	bool found_all_keyframes = false;
	bool found_keyframe [128]; // If we've found a key frame below the requested pts.
	bool found_above [128]; // If we've found a frame which is above the requested pts.
	bool found_any [128]; // If we've found any frame
	guint64 highest_pts [128]; // The highest key frame pts below the requested pts.
	guint64 highest_pi [128]; // The packet index where we found the highest key frame (see above).

	for (int i = 0; i < 128; i++) {
		found_keyframe [i] = readers [i] == NULL;
		found_above [i] = readers [i] == NULL;
		found_any [i] = readers [i] == NULL;
		highest_pts [i] = 0;
		highest_pi [i] = G_MAXUINT64;
	}

	// Start with the latest available packet, otherwise we may end up waiting for some position
	// which is way ahead of the position we'll actually end up wanting
	// (if the initial start_pi estimate is way off)
	if (start_pi > GetLastAvailablePacketIndex ())
		start_pi = GetLastAvailablePacketIndex ();

	do {
		// We can't read before the first packet
		if (start_pi < tested_counter)
			break;

		test_pi = start_pi - tested_counter++;

		ASFPacket *packet = NULL;
		result = parser->ReadPacket (&packet, test_pi);

		LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Searching packet index %" G_GUINT64_FORMAT " for key frames..\n", pts, test_pi);

		if (result == MEDIA_INVALID_DATA) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Skipping invalid packet (index: %" G_GUINT64_FORMAT ")\n", pts, test_pi);
			if (packet)
				packet->unref ();
			continue;
		}

		if (result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): not enough data (index: %" G_GUINT64_FORMAT ")\n", pts, test_pi);
			if (packet)
				packet->unref ();
			return result;
		}

		if (!MEDIA_SUCCEEDED (result)) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): could not read more packets (error: %i)\n", pts, (int) result);
			if (packet)
				packet->unref ();;
			break;
		}
				
		asf_single_payload** payloads = packet->payloads->payloads;
		for (int i = 0; payloads [i] != NULL; i++) {
			asf_single_payload *payload = payloads [i];
			int stream_id = payload->stream_id;
			guint64 payload_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);
			ASFFrameReader *reader = readers [stream_id];

			// Ignore payloads for streams we're not handling
			if (reader == NULL)
				continue;

			reader->GetStream ()->SetLastAvailablePts (payload_pts);

			// We're not interested in payloads with pts above the requested pts
			if (payload_pts > pts) {
				found_above [stream_id] = true;
				continue;
			}

			// We've already found a key frame for the given stream, no need to look for another one.
			if (found_keyframe [stream_id])
				continue;

			// We're not interested in payloads which doesn't represent the start of a frame
			if (payload->offset_into_media_object != 0)
				continue;
	
			// We're only interested in key frames.
			if (!payload->is_key_frame && !reader->IsAudio ())
				continue;
			
			// We've found a key frame with pts below the requested pts.
			found_keyframe [stream_id] = true;
			highest_pts [stream_id] = MAX (highest_pts [stream_id], payload_pts);
			highest_pi [stream_id] = highest_pi [stream_id] == G_MAXUINT64 ? test_pi : MAX (highest_pi [stream_id], test_pi);
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Found key frame of stream #%i with pts %" G_GUINT64_FORMAT " in packet index %" G_GUINT64_FORMAT "\n", pts, stream_id, payload_pts, test_pi);
		}
		
		packet->unref ();;

		// Check if we found key frames for all streams, if not, continue looping.
		found_all_keyframes = true;
		for (int i = 0; i < 128; i++) {
			if (test_pi == 0 && !found_any [i]) {
				// this is a lie, we haven't found a keyframe.
				// but we've searched backwards until the first packet, and we didn't find *any* frame (for this stream),
				// which means that there are no frames from the point where we started searching backwards
				// untill the first packet. In this case we just assume that there is a keyframe at pts 0 for
				// this stream.
				found_keyframe [i] = true;
				highest_pts [i] = 0;
				highest_pi [0] = 0;
			}
			
			if (!found_keyframe [i]) {
				found_all_keyframes = false;
				break;
			}
		}
	} while (!found_all_keyframes);

	// Check if we found key frames for all streams, if not, just return false.
	// We haven't changed any reader state, so the readers will continue 
	// returning data as if nothing had happened.
	for (int i = 0; i < 128; i++) {
		if (!found_keyframe [i]) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Could not find the requested pts.\n", pts);
			return MEDIA_FAIL;
		}
	}	

	// Now we have a packet index we know has key frames whose pts is below the 
	// requested pts in all streams. We do not know if those key frames are the key frames
	// immediately before the requested pts (an example: we might have
	// found the first keyframe in every stream when we wanted to seek to the last
	// pts). We need to continue reading ahead finding key frames with pts below than the 
	// requested one, until we find a frame with pts above the requested one, in which case
	// we know that we've found the key frame immediately before the requested pts.

	tested_counter = 1; // Since we've already tested start_pi, now we want to start with start_pi + 1.
	do {
		// Make this check before reading any packets, since we might already have found
		// all the highest packet indices.
		found_all_highest = true;
		for (int i = 0; i < 128; i++) {
			if (!found_above [i]) {
				found_all_highest = false;
				break;
			}
		}
		if (found_all_highest)
			break;

		test_pi = start_pi + tested_counter++;

		ASFPacket *packet = NULL;
		result = parser->ReadPacket (&packet, test_pi);

		LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Searching packet index %" G_GUINT64_FORMAT " for higher key frames..\n", pts, test_pi);

		if (result == MEDIA_INVALID_DATA) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Skipping invalid packet (index: %" G_GUINT64_FORMAT ")\n", pts, test_pi);
			if (packet)
				packet->unref ();;
			continue;
		}

		if (result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Not enough data (index: %" G_GUINT64_FORMAT ")\n", pts, test_pi);
			if (packet)
				packet->unref ();
			return result;
		}
		
		if (!MEDIA_SUCCEEDED (result)) {
			LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): could not read more packets (error: %i)\n", pts, (int) result);
			if (packet)
				packet->unref ();;
			break;
		}
		
		if (packet->payloads != NULL) {
			asf_single_payload** payloads = packet->payloads->payloads;
			for (int i = 0; payloads [i] != NULL; i++) {
				asf_single_payload *payload = payloads [i];
				int stream_id = payload->stream_id;
				guint64 payload_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);
				ASFFrameReader *reader = readers [stream_id];
	
				// Ignore payloads for streams we're not handling
				if (reader == NULL)
					continue;
	
				reader->GetStream ()->SetLastAvailablePts (payload_pts);
			
				// Found a pts above the requested pts, save it.
				if (payload_pts > pts) {
					found_above [stream_id] = true;
					continue;
				}
	
				// We've already found a higher pts for the given stream, no need to look for another one.
				if (found_above [stream_id])
					continue;
	
				// We're not interested in payloads which doesn't represent the start of a frame
				if (payload->offset_into_media_object != 0)
					continue;
		
				// We're only interested in key frames.
				if (!payload->is_key_frame && !reader->IsAudio ())
					continue;
				
				// We've found another key frame which is below the requested one
				highest_pts [stream_id] = MAX (highest_pts [stream_id], payload_pts);
				highest_pi [stream_id] = test_pi;
	
				LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Found higher key frame of stream #%i with pts %" G_GUINT64_FORMAT " in packet index %" G_GUINT64_FORMAT "\n", pts, stream_id, payload_pts, test_pi);
			}
		}
		
		packet->unref ();;
	} while (true);
	
	// Finally we have all the data we need.
	ResetAll ();
	
	test_pi = G_MAXUINT64;
	for (int i = 0; i < 128; i++) {
		if (readers [i] == NULL)
			continue;

		// Find the packet index for which it is true that all streams have a key frame below the requested pts.
		test_pi = MIN (test_pi, highest_pi [i]);
		// Set the first pts to be returned by each reader to the highest key-frame pts we found.
		readers [i]->SetFirstPts (highest_pts [i]);
	}
	
	// Don't return any frames before the pts we seeked to.
	next_packet_index = (test_pi == G_MAXUINT64) ? 0 : test_pi;

	LOG_ASF ("ASFReader::Seek (%" G_GUINT64_FORMAT "): Seeked to packet index %" G_GINT64_FORMAT ".\n", pts, test_pi);
	
	return MEDIA_SUCCESS;
}

guint64
ASFReader::GetLastAvailablePacketIndex ()
{
	gint64 last_pos = source->GetLastAvailablePosition ();
	guint64 pi;

	if (last_pos < parser->GetPacketOffset (0) + parser->GetPacketSize ()) {
		LOG_ASF ("ASFReader::GetLastAvailablePacketIndex (): returing 0 (not a single packet available)\n");
		return 0;
	}
	
	pi = parser->GetPacketIndex (last_pos);

	if (pi == 0) {
		LOG_ASF ("ASFReader::GetLastAvailablePacketIndex (): returing 0 (only first packet available)\n");
		return 0;
	}

	// We want the packet just before the one which contains the last available position.
	pi--;

	return pi;
}

/*
 *	ASFFrameReader
 */

ASFFrameReader::ASFFrameReader (ASFParser *parser, int stream_number, ASFDemuxer *demuxer, ASFReader *reader, IMediaStream *stream)
{
	this->reader = reader;
	this->stream_number = stream_number;
	this->parser = parser;
	this->demuxer = demuxer;
	this->stream = stream;
	this->stream->ref ();
	first = NULL;
	last = NULL;
	size = 0;
	pts = 0;
	payloads = NULL;
	
	payloads_size = 0;
	payloads = NULL;
	
	first_pts = 0;
	
	index = NULL;
	index_size = 0;
	key_frames_only = true;
	positioned = false;
	buffer_underflow = false;
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
	key_frames_only = true;
	first_pts = 0;
	if (payloads != NULL) {
		for (int i = 0; payloads [i]; i++) {
			delete payloads [i];
			payloads [i] = NULL;
		}
	}
	RemoveAll ();
}

bool
ASFFrameReader::IsAudio ()
{
	return IsAudio (StreamId ());
}

bool
ASFFrameReader::IsAudio (int stream)
{
	const asf_stream_properties *asp = parser->GetStream (stream);
	return asp != NULL && asp->is_audio ();
}

void
ASFFrameReader::AddFrameIndex (guint64 packet_index)
{
	// No need to create an index if we can't seek.
	if (!reader->CanSeek ())
		return;
		
	gint64 packet_count = parser->GetPacketCount ();
	
	// Create the index.
	if (index_size == 0) {
		if (packet_count > 0xFFFF) {
			// This is some really huge file (or a corrupted file).
			// Don't create any indices, since it would consume a whole lot of memory.
			//printf ("ASFFrameReader::AddFrameIndex (): Not creating index, too many packets to track (%" G_GUINT64_FORMAT ")\n", packet_count);
			return;
		}
		
		// Max size here is 0xFFFF packets * 16 bytes per index = 1.048.560 bytes
		index_size = packet_count;
		
		// Don't create any indices if there are no packets. 
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
	 
	// index_size can't be 0 here.
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
ASFFrameReader::ResizeList (int size)
{
	asf_single_payload **new_list = NULL;
	
	if (payloads_size >= size && size > 0)
		return true;
	
	// Allocate a new list
	new_list = (asf_single_payload **) parser->Malloc (sizeof (asf_single_payload*) * (size + 1));
	
	if (new_list == NULL) {
		return false;
	}
	
	if (payloads != NULL) {
		// Copy the old list to the new one
		memcpy (new_list, payloads, payloads_size * sizeof (asf_single_payload*));
		g_free (payloads);
	}
	
	payloads = new_list;
	payloads_size = size;
	
	return true;
}

void
ASFFrameReader::SetOnlyKeyFrames ()
{
	key_frames_only = true;
}

void
ASFFrameReader::SetFirstPts (guint64 pts)
{
	first_pts = pts;
}

MediaResult
ASFFrameReader::Advance (bool read_if_needed)
{
	MediaResult result = MEDIA_SUCCESS;
	MediaResult read_result;
	int payload_count = 0;
	guint32 media_object_number = 0;
	guint64 current_pts = 0;
	guint64 first_packet_index = 0; // The packet index where the frame starts.
	ASFFrameReaderData* current = NULL;
	
	LOG_ASF ("ASFFrameReader::Advance ().\n");
	
	if (buffer_underflow) {
		// Set initial values according to where we left off when the buffer underflowed
		for (int i = 0; payloads [i] != NULL; i++) {
			payload_count++;
		}
		if (payload_count == 0) {
			size = 0;
			pts = 0;
		} else {
			media_object_number = payloads [0]->media_object_number;
			current_pts = pts;
			first_packet_index = G_MAXUINT64;
		}
	} else {
		// Clear the current list of payloads.
		// Most streams has at least once a media object spanning two payloads.
		// so we allocate space for two (+ NULL at the end).
		if (payloads == NULL) {
			if (!ResizeList (2)) {
				parser->AddError ("Out of memory.");
				return MEDIA_OUT_OF_MEMORY;
			}
		} else {
			// Free all old payloads, they belong to the previous frame.
			for (int i = 0; payloads[i]; i++) {
				delete payloads[i];
				payloads[i] = NULL;
			}
		}
		size = 0;
		pts = 0;
	}

	current = first;
	
	LOG_ASF ("ASFFrameReader::Advance (): frame data: size = %" G_GINT64_FORMAT ", key = %s, pts = %" G_GUINT64_FORMAT ", stream# = %d, media_object_number = %u.\n", 
		 size, IsKeyFrame () ? "true" : "false", Pts (), StreamId (), media_object_number);
	
	buffer_underflow = false;
		
	while (true) {
		// Loop through payloads until we find a payload with the different media number
		// than the first payload in the queue.
		
		// Make sure we have any payloads in our queue of payloads
		while (current == NULL) {
			// We went past the end of the payloads, read another packet to get more data.
			current = last; // go back to the last element.
			
			LOG_ASF ("ASFFrameReader::Advance (): No more payloads, requesting more data.\n");

			if (!read_if_needed) {
				read_result = MEDIA_NO_MORE_DATA;
				goto end_frame;
			}

			read_result = reader->TryReadMore ();
			if (read_result == MEDIA_NO_MORE_DATA) {
				// No more data, we've reached the end
				LOG_ASF ("ASFFrameReader::Advance (): No more data, payload count: %i\n", payload_count);
				if (payload_count == 0)				
					result = read_result;
				goto end_frame;
			} else if (read_result == MEDIA_BUFFER_UNDERFLOW) {
				result = read_result;
				buffer_underflow = true;
				goto end_frame;
			} else if (!MEDIA_SUCCEEDED (read_result)) {
				result = read_result;
				goto end_frame;
			} else {
				if (current == NULL) {
					// There were no elements before reading more, our next element is the first one
					current = first;
				} else {
					current = current->next;
				}
			}
		}
		
		LOG_ASF ("ASFFrameReader::Advance (): checking payload, stream: %d, media object number %d, size: %d\n", current->payload->stream_id, current->payload->media_object_number, current->payload->payload_data_length);
		
		asf_single_payload* payload = current->payload;
		current_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);

		stream->SetLastAvailablePts (current_pts);

		if (current_pts < first_pts) {
			ASFFrameReaderData* tmp = current;
			current = current->next;
			Remove (tmp);
		} else {
			if (payload_count > 0 && payload->media_object_number != media_object_number) {
				// We've found the end of the current frame's payloads
				LOG_ASF ("ASFFrameReader::Advance (): reached media object number %i (while reading %i).\n", payload->media_object_number, media_object_number);
				goto end_frame;
			}
						
			if (key_frames_only && !IsAudio () && !payload->is_key_frame) {
				LOG_ASF ("ASFFrameReader::Advance (): dropped non-key frame, pts: %" G_GUINT64_FORMAT "\n", current_pts);
				ASFFrameReaderData* tmp = current;
				current = current->next;
				Remove (tmp);
				continue;
			}
			
			if (payload_count == 0 && payload->offset_into_media_object != 0) {
				// This frame isn't complete, it's probably split over several packets (and we haven't read the first of those packets).
				LOG_ASF ("ASFFrameReader::Advance (): skipping incomplete frame, pts: %" G_GUINT64_FORMAT ", offset into media object: %i.\n", current_pts, payload->offset_into_media_object);
				ASFFrameReaderData *tmp = current;
				current = current->next;
				Remove (tmp);
				continue;
			}
			
			key_frames_only = false;			
			media_object_number = payload->media_object_number;
			first_packet_index = current->packet_index;
			
			// add the payload to the current frame's payloads
			payload_count++;
			if (payload_count == 1)
				this->pts = current_pts;
			size += payload->payload_data_length;
			if (payload_count > payloads_size) {
				if (!ResizeList (payload_count + 3)) {
					return false;
				}
			}
			payloads [payload_count - 1] = payload;
			current->payload = NULL;
			
			// Remove it from the queue
			ASFFrameReaderData* tmp = current;
			current = current->next;
			Remove (tmp);
		}
		
		LOG_ASF ("ASFFrameReader::Advance (): current is %p.\n", current);
	}
	
end_frame:
/*
	printf ("ASFFrameReader::Advance (): frame data: size = %.4lld, key = %s, pts = %.5llu, stream# = %i, media_object_number = %.3u (advanced).", 
		size, IsKeyFrame () ? "true " : "false", Pts (), StreamNumber (), media_object_number);

	dump_int_data (payloads [0]->payload_data, payloads [0]->payload_data_length, 4);
	printf ("\n");
*/
/*
	if (MEDIA_SUCCEEDED (result)) {
		printf ("ASFFrameReader::Advance (): frame data: size = %.4lld, key = %s, Pts = %.5llu = %" G_GUINT64_FORMAT " ms, pts = %.5u, stream# = %i (%s), media_object_number = %.3u (advanced).\n", 
			size, IsKeyFrame () ? "true " : "false", Pts (), MilliSeconds_FromPts (Pts ()), payloads [0]->presentation_time, StreamId (), stream->GetStreamTypeName (), media_object_number);
	}
*/
	
	if (MEDIA_SUCCEEDED (result)) {
		if (first_packet_index != G_MAXUINT64)
			AddFrameIndex (first_packet_index);
	}

	return result;
}

gint64
ASFFrameReader::EstimatePtsPosition  (guint64 pts)
{
	return parser->GetPacketOffset (MIN (parser->GetPacketCount () - 1, EstimatePacketIndexOfPts (pts) + 1));
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

	total_duration = parser->GetFileProperties ()->play_duration - MilliSeconds_ToPts (parser->GetFileProperties ()->preroll);
	if (pts >= total_duration) {
		return parser->GetPacketCount () - 1;
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
		guint64 duration = MAX (1, parser->GetFileProperties ()->play_duration - MilliSeconds_ToPts (parser->GetFileProperties ()->preroll));
		double percent = MAX (0, pts / (double) duration);
		result = percent * parser->GetPacketCount ();
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): No average, calculated by percent %.2f, pi: %i, pts: %" G_GUINT64_FORMAT ", preroll: %" G_GUINT64_FORMAT "\n", pts, percent, pi, pts, preroll);
	} else {
		// calculate packet index from the last known packet index / pts and average pts per packet index
		last_good_pts = MIN (last_good_pts, pts);
		result = last_good_pi + (pts - last_good_pts) / average;
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): Calculated by averate %" G_GUINT64_FORMAT ", last_good_pts: %" G_GUINT64_FORMAT ", pi: %i\n", pts, average, last_good_pts, pi);
	}
	
	result = MAX (0, result);
	result = MIN (result, MAX (0, parser->GetPacketCount () - 1));
	
	//printf ("ASFFrameReader::GetPacketIndexOfPts (%" G_GUINT64_FORMAT "): Final position: %" G_GINT64_FORMAT " of pi: %i. Total packets: %" G_GUINT64_FORMAT ", total duration: %" G_GUINT64_FORMAT "\n", pts, parser->GetPacketOffset (pi), pi, parser->GetFileProperties ()->data_packet_count, parser->GetFileProperties ()->play_duration);
	return result;
}

void
ASFFrameReader::AppendPayload (asf_single_payload *payload, guint64 packet_index)
{
	LOG_ASF ("ASFFrameReader::AppendPayload (%p, %" G_GUINT64_FORMAT "). Stream #%i, pts: %i ms\n", payload, packet_index, StreamId (), (int) payload->get_presentation_time () - 5000);

	bool advanced;
	bool restore = false;
	
	ASFFrameReaderData* node = new ASFFrameReaderData (payload);
	node->packet_index = packet_index;
	if (first == NULL) {
		first = node;
		last = node;
	} else {
		node->prev = last;
		last->next = node;
		last = node;
	}
	
	if (stream->GetType () == MediaTypeMarker) {
		// Here we try to figure out if we have an entire marker or not
		// (determined by finding two NULL WCHARs in the data).
		// Make a copy of our payloads, Advance will delete them, 
		// and we might want to keep them until the next payload arrives.
		ASFFrameReaderData *clone_head = NULL;
		ASFFrameReaderData *clone = NULL;
		ASFFrameReaderData *tmp = first;
		ASFFrameReaderData *copy = NULL;
		
		while (tmp != NULL) {
			copy = new ASFFrameReaderData (tmp->payload->Clone ());
			if (clone == NULL) {
				clone = copy;
				clone_head = clone;
			} else {
				clone->next = copy;
				copy->prev = clone;
				clone = clone->next;
			}
			tmp = tmp->next;
		}
		
		advanced = MEDIA_SUCCEEDED (Advance (false));
		
		if (advanced) {
			// Check if we got all the data
			// determined by finding two NULL WCHARs
			gint16 *data = (gint16 *) g_malloc (Size ());
			int nulls = 0;
			
			if (Write (data)) {
				for (guint32 i = 0; i < Size () / 2; i++) {
					if (data [i] == 0) {
						nulls++;
						if (nulls >= 2)
							break;
					}
				}
			}
			
			LOG_ASF ("ASFFrameReader::AppendPayload () in data with size %" G_GUINT64_FORMAT " found %i nulls.\n", Size (), nulls);
	
			if (nulls >= 2) {
				MarkerStream *marker_stream = (MarkerStream *) stream;
				MediaFrame *frame = new MediaFrame (marker_stream);
				frame->pts = Pts ();
				frame->buflen = Size ();
				frame->buffer = (guint8 *) data;
				marker_stream->MarkerFound (frame);
				frame->unref ();
			} else {
				restore = true;
				g_free (data);
			}
		}
		
		
		if (restore && first == NULL) {
			LOG_ASF ("ASFFrameReader::AppendPayload (%p, %" G_GUINT64_FORMAT "). Restoring nodes.\n", payload, packet_index);
			// Restore everything
			// Advance () should have consumed all of the ASFFrameReaderDataNodes
			// otherwise we're having corruption (since the only way to not have consumed
			// all nodes is to get a second payload with a different media object number
			// than a first payload, and the first payload doesn't contain 2 NULLs).
			first = clone_head;
			last = first;
			while (last->next != NULL)
				last = last->next;
		} else {
			LOG_ASF ("ASFFrameReader::AppendPayload (%p, %" G_GUINT64_FORMAT "). Freeing copied list of nodes.\n", payload, packet_index);
			// Free the copied list of nodes.
			tmp = clone_head;
			while (tmp != NULL) {
				copy = tmp->next;
				delete tmp;
				tmp = copy;
			}
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
	
	delete data;
}
