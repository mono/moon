/*
 * asf.cpp: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include <stdint.h>

#include "asf.h"
#include "../debug.h"
#include "../clock.h"

/*
	ASFParser
*/

ASFParser::ASFParser (IMediaSource *src, Media *m)
{
	ASF_LOG ("ASFParser::ASFParser ('%p'), this: %p.\n", src, this);
	source = src;
	if (source)
		source->ref ();
	media = m;
	Initialize ();
}

uint64_t
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
	embedded_script_command = NULL;
	embedded_script_command_state = NULL;
	memset (stream_properties, 0, sizeof (asf_stream_properties *) * 127);
}

ASFParser::~ASFParser ()
{
	ASF_LOG ("ASFParser::~ASFParser ().\n");
	
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
ASFParser::ReadEncoded (IMediaSource *source, uint32_t length, uint32_t *dest)
{
	uint16_t result2 = 0;
	uint8_t result1 = 0;
	
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
ASFParser::VerifyHeaderDataSize (uint32_t size)
{
	if (header == NULL)
		return false;
	
	return size >= 0 && size < header->size;
}

void *
ASFParser::MallocVerified (uint32_t size)
{
	void *result = g_try_malloc0 (size);
	
	if (result == NULL)
		AddError ("Out of memory.");
	
	return result;
}

void *
ASFParser::Malloc (uint32_t size)
{
	if (!VerifyHeaderDataSize (size))
		return NULL;
	
	return MallocVerified (size);
}

asf_object *
ASFParser::ReadObject (asf_object *obj)
{
	ASFTypes type = asf_get_guid_type (&obj->id);
	asf_object *result = NULL;
	char *guid;
	
	ASF_LOG ("ASFParser::ReadObject ('%s', %llu)\n", asf_guid_tostring (&obj->id), obj->size);
	
	if (type == ASF_NONE) {
		guid = asf_guid_tostring (&obj->id);
		AddError (g_strdup_printf ("Unrecognized guid: %s.", guid));
		g_free (guid);
		return NULL;
	}
	
	result = (asf_object *) Malloc (obj->size);
	if (result == NULL) {
		guid = asf_guid_tostring (&obj->id);
		AddError (g_strdup_printf ("Header corrupted (id: %s)", guid));
		g_free (guid);
		return false;
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
ASFParser::ReadPacket (ASFPacket *packet, int packet_index)
{
	ASF_LOG ("ASFParser::ReadPacket (%p, %d).\n", packet, packet_index);
	
	if (packet_index >= 0) {
		int64_t position = GetPacketOffset (packet_index);
		
		if (position == 0 || (source->GetPosition () != position && !source->Seek (position, SEEK_SET))) {
			//printf ("ASFParser::ReadPacket (%p, %i): position is 0 or seek failed (position: %lld).\n", packet, packet_index, position); 
			return MEDIA_SEEK_ERROR;
		}
	}
	
	return ASFParser::ReadPacket (packet);
}

MediaResult
ASFParser::ReadPacket (ASFPacket *packet)
{
	MediaResult result;
	asf_payload_parsing_information ppi;
	asf_error_correction_data ecd;
	IMediaSource *source;
	
	ASFContext context;
	context.parser = this;
	if (packet->GetSource ()) {
		context.source = packet->GetSource ();
	} else {
		context.source = this->source;
	}
	source = context.source;

	ASF_LOG ("ASFParser::ReadPacket (%p): Reading packet at %lld (index: %lld) of %lld packets.\n",
		 packet, source->GetPosition (), GetPacketIndex (source->GetPosition ()),
		 data->data_packet_count);
	
#if DEBUG
	// Check if we're positioned at the start of a packet.
	bool ok = false;
	int64_t current_pos = source->GetPosition ();
	int64_t offset;
	for (uint64_t i = 0; i < data->data_packet_count; i++) {
		offset = GetPacketOffset (i);
		if (current_pos == offset) {
			ok = true; 
			break;
		} else if (current_pos < offset) {
			break;
		}
	}
	if (!ok) {
		fprintf (stderr, "ASFParser::ReadPacket (%p): The source isn't positioned at the start of any packet. Reading errors will occur.\n", packet);
	}
#endif

	int64_t next_pos = GetPacketOffset (1 + GetPacketIndex (source->GetPosition ()));

	result = ecd.FillInAll (&context);
	if (!MEDIA_SUCCEEDED (result)) {
		source->Seek (next_pos, SEEK_SET);
		return result;
	}
	
	asf_error_correction_data_dump (&ecd);
	
	result = ppi.FillInAll (&context);
	if (!MEDIA_SUCCEEDED (result)) {
		source->Seek (next_pos, SEEK_SET);
		return result;
	}
	
	asf_payload_parsing_information_dump (&ppi);
	
	asf_multiple_payloads *mp = new asf_multiple_payloads ();
	result = mp->FillInAll (&context, &ecd, ppi);
	if (!MEDIA_SUCCEEDED (result)) {
		delete mp;
		source->Seek (next_pos, SEEK_SET);
		return result;
	}

	packet->payloads = mp;
	
/*	ASF_LOG ("ASFParser::ReadPacket (): Current position (end of packet): %llx (%lld), start position was: %llx (%lld), difference: %llx (%lld)\n", 
		source->Position (), source->Position (), 
		start_position, start_position,
		source->Position () - start_position, source->Position () - start_position);
	*/
	/* If we are "positioned", need to seek to the start of the next packet */
	source->Seek (next_pos, SEEK_SET);

	return MEDIA_SUCCESS;
}

bool
ASFParser::ReadData ()
{
	ASF_LOG ("ASFParser::ReadData ().\n");
	
	if (this->data != NULL) {
		AddError ("ReadData has already been called.");
		return false;
	}
	
	if (!source->Seek ((int64_t) header->size, SEEK_SET)) {
		AddError ("ReadData could not seek to the beginning of the data.");
		return false;
	}
	
	ASF_LOG ("Current position: %llx (%lld)\n", source->GetPosition (), source->GetPosition ());
	
	data = (asf_data *) Malloc (sizeof (asf_data));
	if (data == NULL) {
		AddError ("Data corruption in data.");
		return false;
	}
	
	if (!source->ReadAll (data, sizeof (asf_data))) {
		g_free (data);
		data = NULL;
		return false;
	}
	
	asf_object_dump_exact (data);
	
	ASF_LOG ("Data %p has %lld packets.\n", data, data->data_packet_count);
	
	this->data = data;
	
	return true;
}

bool
ASFParser::ReadHeader ()
{
	ASF_LOG ("ASFParser::ReadHeader ().\n");
	
	header = (asf_header *) MallocVerified (sizeof (asf_header));
	if (header == NULL) {
		ASF_LOG ("ASFParser::ReadHeader (): Malloc failed.\n");
		return false;
	}
	
	if (!source->ReadAll (header, sizeof (asf_header))) {
		ASF_LOG ("ASFParser::ReadHeader (): source->Read () failed.\n");
		return false;
	}
	
	asf_header_dump (header);

	if (!asf_header_validate (header, this)) {
		ASF_LOG ("Header validation failed, error: '%s'\n", GetLastErrorStr ());
		return false;
	}
	
	header_objects = (asf_object **) Malloc ((header->object_count + 1) * sizeof (asf_object*));
	if (header_objects == NULL) {
		AddError ("Data corruption in header.");
		return false;
	}
	
	ASF_LOG ("ASFParser::ReadHeader (): about to read streams...\n");
	
	bool any_streams = false;
	for (uint32_t i = 0; i < header->object_count; i++) {
		asf_object tmp;
		
		if (!source->ReadAll (&tmp, sizeof (asf_object)))
			return false;
		
		if (!(header_objects [i] = ReadObject (&tmp)))
			return false;
		
		if (asf_guid_compare (&asf_guids_stream_properties, &header_objects[i]->id)) {
			asf_stream_properties *stream = (asf_stream_properties *) header_objects[i];
			SetStream (stream->get_stream_id (), stream);
			any_streams = true;
		}
		
		if (asf_guid_compare (&asf_guids_file_properties, &header_objects [i]->id)) {
			if (file_properties != NULL) {
				AddError ("Multiple file property object in the asf data.");
				return false;
			}
			file_properties = (asf_file_properties*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_header_extension, &header_objects [i]->id)) {
			if (header_extension != NULL) {
				AddError ("Multiple header extension objects in the asf data.");
				return false;
			}
			header_extension = (asf_header_extension*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_marker, &header_objects [i]->id)) {
			if (marker != NULL) {
				AddError ("Multiple marker objects in the asf data.");
				return false;
			}
			marker = (asf_marker*) header_objects [i];
		}
		
		if (asf_guid_compare (&asf_guids_script_command, &header_objects [i]->id)) {
			if (script_command != NULL) {
				AddError ("Multiple script command objects in the asf data.");
				return false;
			}
			script_command = (asf_script_command*) header_objects [i];
		}
		
		asf_object_dump_exact (header_objects [i]);
	}

	// Check if there are stream properties in any extended stream properties object.
	if (header_extension != NULL) {
		asf_object **objects = header_extension->get_objects ();
		for (int i = 0; objects [i] != NULL; i++) {
			if (asf_guid_compare (&asf_guids_extended_stream_properties, &objects [i]->id)) {
				asf_extended_stream_properties *aesp = (asf_extended_stream_properties *) objects [i];
				const asf_stream_properties *stream = aesp->get_stream_properties ();
				if (stream != NULL) {
					if (stream->get_stream_id () != aesp->stream_id) {
						g_free (objects);
						AddError ("There is an invalid extended stream properties object (it contains a stream properties object whose stream id doesn't match the stream id of the extended stream properties object).");
						return false;
					} else {
						SetStream (stream->get_stream_id (), stream);
					}
				} else if (!IsValidStream (aesp->stream_id)) {
					g_free (objects);
					AddError ("There is an extended stream properties object that doesn't have a corresponding strem properties object.");
					return false;
				}
				any_streams = true;
			}
		}
		g_free (objects);
	}

	// Check for required objects.
	
	if (file_properties == NULL) {
		AddError ("No file property object in the asf data.");
		return false;
	}
	
	if (header_extension == NULL) {
		AddError ("No header extension object in the asf data.");
		return false;
	}
	
	if (!any_streams) {
		AddError ("No streams in the asf data.");
		return false;
	}
	
	data_offset = header->size;
	packet_offset = data_offset + sizeof (asf_data);
	packet_offset_end = packet_offset + file_properties->data_packet_count * file_properties->min_packet_size;

	ASF_LOG ("ASFParser::ReadHeader (): Header read successfully.\n");
	
	if (!ReadData ())
		return false;
		
	return true;
}

ErrorEventArgs *
ASFParser::GetLastError ()
{
	return error;
}

const char *
ASFParser::GetLastErrorStr ()
{
	return error ? error->error_message : "";
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
	
	if (error == NULL) {
		error = new MediaErrorEventArgs (code, msg);
		if (media)
			media->AddError (error);
	}
	g_free (msg);
}

const asf_stream_properties* 
ASFParser::GetStream (int stream_index)
{
	if (stream_index < 1 || stream_index > 127)
		return NULL;
	
	return stream_properties [stream_index - 1];
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

bool
ASFParser::IsValidStream (int stream_index)
{
	return GetStream (stream_index) != NULL;
}

int64_t
ASFParser::GetPacketOffset (uint64_t packet_index)
{
	if (packet_index < 0 || packet_index >= file_properties->data_packet_count) {
//		AddError (g_strdup_printf ("ASFParser::GetPacketOffset (%i): Invalid packet index (there are %llu packets).", packet_index, file_properties->data_packet_count)); 
		return 0;
	}
	
	// CHECK: what if min_packet_size != max_packet_size?
	return packet_offset + packet_index * file_properties->min_packet_size;
}

uint32_t
ASFParser::GetPacketSize ()
{
	return file_properties->min_packet_size;
}

uint64_t
ASFParser::GetPacketIndex (int64_t offset)
{
	if (offset < packet_offset)
		return 0;
	
	if (offset > packet_offset_end)
		return file_properties->data_packet_count - 1;
	
	return (offset - packet_offset) / file_properties->min_packet_size;
}

uint64_t
ASFParser::GetPacketIndexOfPts (int stream_id, uint64_t pts)
{
	ASFPacket *packet = NULL;
	int result = 0;
	MediaResult read_result;
	
	// Read packets until we find the packet which has a pts
	// greater than the one we're looking for.
	
	while (MEDIA_SUCCEEDED (read_result = ReadPacket (packet, result))) {
		uint64_t current_pts = packet->GetPts (stream_id);
		
		if (current_pts > pts) {
			// We've found the packet after the one we're
			// looking for: return the previous one.
			return result - 1;
		}
		
		result++;
	}
	
	return -1;
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

ASFPacket::ASFPacket ()
{
	payloads = NULL;
	position = -1;
	index = -1;
	source = NULL;
}

ASFPacket::ASFPacket (IMediaSource *source)
{
	payloads = NULL;
	position = -1;
	index = -1;
	this->source = source;
	if (this->source)
		this->source->ref ();
}

ASFPacket::~ASFPacket ()
{
	delete payloads;
	if (this->source)
		this->source->unref ();
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

uint64_t
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

/*
 * ASFReader
 */

ASFReader::ASFReader (ASFParser *parser, IMediaDemuxer *demuxer)
{
	this->parser = parser;
	this->demuxer = demuxer;
	this->source = parser->GetSource ();
	if (this->demuxer)
		this->demuxer->ref ();
	next_packet_index = 0;
	memset (readers, 0, sizeof (ASFFrameReader*) * 128);
	positioned = false;
}

ASFReader::~ASFReader ()
{
	if (demuxer)
		demuxer->unref ();

	for (int i = 0; i < 128; i++)
		delete readers [i];
}

void
ASFReader::SelectStream (int32_t stream_index, bool value)
{
	ASF_LOG ("ASFReader::SelectStream (%i, %i)\n", stream_index, value);	

	if (stream_index <= 0 || stream_index >= 128) {
		fprintf (stderr, "ASFReader::SelectStream (%i, %i): Invalid stream index\n", stream_index, value);
		return;
	}

	if (value) {
		if (readers [stream_index] == NULL) {
			readers [stream_index] = new ASFFrameReader (parser, stream_index, demuxer, this);
		}
	} else {
		if (readers [stream_index] != NULL) {
			delete readers [stream_index];
			readers [stream_index] = NULL;
		}
	}
}

ASFFrameReader *
ASFReader::GetFrameReader (int32_t stream_index)
{
	if (stream_index <= 0 || stream_index >= 128) {
		fprintf (stderr, "ASFReader::GetFrameReader (%i): Invalid stream index.\n", stream_index);
		return NULL;
	}

	return readers [stream_index];
}

MediaResult
ASFReader::ReadMore ()
{
	ASF_LOG ("ASFReader::ReadMore ().\n");
	
	int payloads_added = 0;
	uint64_t current_packet_index;
	MediaResult read_result = MEDIA_FAIL;
	ASFPacket* packet = NULL;
	
	do {
		if (Eof ()) {
			ASF_LOG ("ASFReader::ReadMore (): eof\n");
			return MEDIA_NO_MORE_DATA;
		}

		current_packet_index = next_packet_index;		
		next_packet_index++;

		packet = new ASFPacket ();
		if (positioned) {
			read_result = parser->ReadPacket (packet);
		} else {
			read_result = parser->ReadPacket (packet, current_packet_index);
		}

		ASF_LOG ("ASFReader::ReadMore (): positioned: %i, current packet index: %llu, position: %lld, calculated packet index: %llu\n", 
				positioned, current_packet_index, source->GetPosition (), parser->GetPacketIndex (source->GetPosition ()));

		if (read_result == MEDIA_INVALID_DATA) {
			ASF_LOG ("ASFReader::ReadMore (): Skipping invalid packet (index: %llu)\n", current_packet_index);
			delete packet;
			continue;
		}

		if (!MEDIA_SUCCEEDED (read_result)) {
			ASF_LOG ("ASFReader::ReadMore (): could not read more packets (error: %i)\n", (int) read_result);
			delete packet;
			return read_result;
		}
		
		asf_single_payload** payloads = packet->payloads->steal_payloads ();
		ASFFrameReader *reader;
		int i = -1;
		while (payloads [++i] != NULL) {
			reader = GetFrameReader (payloads [i]->stream_id);
			if (reader == NULL) {
				ASF_LOG ("ASFReader::ReadMore (): skipped, stream: %i, added pts: %llu\n", payloads [i]->stream_id, payloads [i]->get_presentation_time ());
				delete payloads [i];
				continue;
			}
			reader->AppendPayload (payloads [i], positioned ? 0 : current_packet_index);
			payloads_added++;
		}
		g_free (payloads);

		ASF_LOG ("ASFReader::ReadMore (): read %d payloads.\n", payloads_added);
	
		delete packet;
	} while (payloads_added == 0);
	
	return MEDIA_SUCCESS;
}

bool
ASFReader::Eof ()
{
	return next_packet_index >= parser->GetPacketCount ();
}

void
ASFReader::ResetAll ()
{
	for (int i = 0; i < 128; i++) {
		if (readers [i] != NULL)
			readers [i]->Reset ();
	}
}

uint64_t
ASFReader::EstimatePacketIndexOfPts (uint64_t pts)
{
	uint64_t result = UINT64_MAX;
	for (int i = 0; i < 128; i++) {
		if (readers [i] == NULL)
			continue;

		result = MIN (readers [i]->EstimatePacketIndexOfPts (pts), result);
	}
	return result == UINT64_MAX ? 0 : result;
}

bool
ASFReader::SeekToStart ()
{
	ASF_LOG ("ASFReader::SeekToStart ().\n");
	
	// Our Seek implementation already has a fast special case for pts = 0.
	return Seek (0);
}

bool
ASFReader::SeekToPts (uint64_t pts)
{
	positioned = true;

	ResetAll ();

	return source->SeekToPts (pts);
}

bool
ASFReader::Seek (uint64_t pts)
{
	ASF_LOG ("ASFReader::Seek (%llu).\n", pts);
	
	if (!CanSeek ())
		return false;

	// We know 0 is at the beginning of the media, so just optimize this case slightly
	if (pts == 0) {
		ResetAll ();
		next_packet_index = 0;
		return true;
	}

	if (positioned || source->CanSeekToPts ())
		return SeekToPts (pts);
	

	// For each stream we need to find a keyframe whose pts is below the requested one.
	// Read a packet, and check each payload for keyframes. If we don't find one, read 
	// the previous packet, and check again.

	MediaResult result;
	uint64_t start_pi = EstimatePacketIndexOfPts (pts); // The packet index we start testing for key frames.
	uint64_t tested_counter = 0; // The number of packet indices we have tested.
	uint64_t test_pi = 0; // The packet index we're currently testing.
	bool found_all_highest;
	bool found_all_keyframes;
	bool found_keyframe [128]; // If we've found a key frame below the requested pts.
	bool found_above [128]; // If we've found a frame which is above the requested pts.
	uint64_t highest_pts [128]; // The highest key frame pts below the requested pts.
	uint64_t highest_pi [128]; // The packet index where we found the highest key frame (see above).

	for (int i = 0; i < 128; i++) {
		found_keyframe [i] = readers [i] == NULL;
		found_above [i] = readers [i] == NULL;
		highest_pts [i] = 0;
		highest_pi [i] = UINT64_MAX;
	}

	do {
		// We can't read before the first packet
		if (start_pi < tested_counter)
			break;

		test_pi = start_pi - tested_counter++;

		ASFPacket *packet = new ASFPacket ();
		result = parser->ReadPacket (packet, test_pi);

		ASF_LOG ("ASFReader::Seek (%llu): Searching packet index %llu for key frames..\n", pts, test_pi);

		if (result == MEDIA_INVALID_DATA) {
			ASF_LOG ("ASFReader::Seek (%llu): Skipping invalid packet (index: %llu)\n", pts, test_pi);
			delete packet;
			continue;
		}

		if (!MEDIA_SUCCEEDED (result)) {
			ASF_LOG ("ASFReader::Seek (%llu): could not read more packets (error: %i)\n", pts, (int) result);
			delete packet;
			break;
		}
				
		asf_single_payload** payloads = packet->payloads->payloads;
		for (int i = 0; payloads [i] != NULL; i++) {
			asf_single_payload *payload = payloads [i];
			int stream_id = payload->stream_id;
			uint64_t payload_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);
			ASFFrameReader *reader = readers [stream_id];

			// Ignore payloads for streams we're not handling
			if (reader == NULL)
				continue;

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
			highest_pi [stream_id] = highest_pi [stream_id] == UINT64_MAX ? test_pi : MAX (highest_pi [stream_id], test_pi);
			ASF_LOG ("ASFReader::Seek (%llu): Found key frame of stream #%i with pts %llu in packet index %llu\n", pts, stream_id, payload_pts, test_pi);
		}
		
		delete packet;

		// Check if we found key frames for all streams, if not, continue looping.
		found_all_keyframes = true;
		for (int i = 0; i < 128; i++) {
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
			ASF_LOG ("ASFReader::Seek (%llu): Could not find the requested pts.\n", pts);
			return false;
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

		ASFPacket *packet = new ASFPacket ();
		result = parser->ReadPacket (packet, test_pi);

		ASF_LOG ("ASFReader::Seek (%llu): Searching packet index %llu for higher key frames..\n", pts, test_pi);

		if (result == MEDIA_INVALID_DATA) {
			ASF_LOG ("ASFReader::Seek (%llu): Skipping invalid packet (index: %llu)\n", pts, test_pi);
			delete packet;
			continue;
		}

		if (!MEDIA_SUCCEEDED (result)) {
			ASF_LOG ("ASFReader::Seek (%llu): could not read more packets (error: %i)\n", pts, (int) result);
			delete packet;
			break;
		}
		
		asf_single_payload** payloads = packet->payloads->payloads;
		for (int i = 0; payloads [i] != NULL; i++) {
			asf_single_payload *payload = payloads [i];
			int stream_id = payload->stream_id;
			uint64_t payload_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);
			ASFFrameReader *reader = readers [stream_id];

			// Ignore payloads for streams we're not handling
			if (reader == NULL)
				continue;

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

			ASF_LOG ("ASFReader::Seek (%llu): Found higher key frame of stream #%i with pts %llu in packet index %llu\n", pts, stream_id, payload_pts, test_pi);
		}
		
		delete packet;
	} while (true);
	
	// Finally we have all the data we need.
	ResetAll ();
	
	test_pi = UINT64_MAX;
	for (int i = 0; i < 128; i++) {
		if (readers [i] == NULL)
			continue;

		// Find the packet index for which it is true that all streams have a key frame below the requested pts.
		test_pi = MIN (test_pi, highest_pi [i]);
		// Set the first pts to be returned by each reader to the highest key-frame pts we found.
		readers [i]->SetFirstPts (highest_pts [i]);
	}
	
	// Don't return any frames before the pts we seeked to.
	next_packet_index = (test_pi == UINT64_MAX) ? 0 : test_pi;

	ASF_LOG ("ASFReader::Seek (%llu): Seeked to packet index %lld.\n", pts, test_pi);
	
	return true;
}

uint64_t 
ASFReader::GetLastAvailablePts ()
{
	ASF_LOG ("ASFReader::GetLastAvailablePts ()\n");
	int64_t last_pos = source->GetLastAvailablePosition ();
	uint64_t pi;
	MediaResult result;
	bool all_set;
	void *buffer = NULL;
	uint64_t ptses [128];
	uint64_t pts;
	uint64_t preroll = parser->GetFileProperties ()->preroll;

	if (last_pos < parser->GetPacketOffset (0) + parser->GetPacketSize ()) {
		ASF_LOG ("ASFReader::GetLastAvailablePts (): returing 0 (not a single packet available)\n");
		return 0;
	}

	pi = parser->GetPacketIndex (last_pos);

	if (pi == 0) {
		ASF_LOG ("ASFReader::GetLastAvailablePts (): returing 0 (only first packet available)\n");
		return 0;
	}

	// We want the packet just before the one which contains the last available position.
	pi--;
	
	if (last_pos <= parser->GetPacketOffset (pi) + parser->GetPacketSize ()) {
		ASF_LOG ("ASFReader::GetLastAvailablePts (): returing 0 (not one packet available)\n");
		return 0; // This shouldn't happen, but handle the case anyway.
	}

	for (int i = 0; i < 128; i++)
		ptses [i] = (readers [i] == NULL) ? UINT64_MAX : 0;

	for (int current_packet_index = pi; current_packet_index >= 0; current_packet_index--) {
		buffer = g_malloc (parser->GetPacketSize ());
		if (!source->Peek (buffer, parser->GetPacketSize (), parser->GetPacketOffset (current_packet_index))) {
			g_free (buffer);
			continue;
		}

		MemorySource *mem_source = new MemorySource (NULL, buffer, parser->GetPacketSize (), parser->GetPacketOffset (current_packet_index));
		ASFPacket *packet = new ASFPacket (mem_source);
		mem_source->unref ();
		
		result = parser->ReadPacket (packet, current_packet_index);
		if (result == MEDIA_INVALID_DATA) {
			delete packet;
			continue;
		}

		if (!MEDIA_SUCCEEDED (result)) {
			delete packet;
			return 0;
		}

		// Loop through all the payloads and update ptses with the highest pts
		// for each payload/stream.
		asf_single_payload** payloads = packet->payloads->payloads;
		for (int j = 0; payloads [j] != NULL; j++) {
			asf_single_payload *payload = payloads [j];
			int stream_id = payload->stream_id;
			uint64_t payload_pts = MilliSeconds_ToPts (payload->get_presentation_time () - preroll);
			ASFFrameReader *reader = readers [stream_id];
			
			if (reader == NULL)
				continue;

			// Given that we might only have the beginning of the frame in the payload,
			// this is not entirely accurate (in order to read the frame of payload_pts we
			// might need to read more packets), but it's good enough for calculating
			// buffering progress.
			ptses [stream_id] = MAX (ptses [stream_id], payload_pts);
		}

		delete packet;

		// Check if we've got pts in all our streams, if so
		// return the smallest of them.
		pts = UINT64_MAX;
		all_set = true;
		for (int j = 0; j < 128; j++) {
			pts = MIN (ptses [j], pts);
			if (ptses [j] == 0) {
				all_set = false;
				break;
			}
		}
		if (all_set) {
			ASF_LOG ("ASFReader::GetLastAvailablePts (): resulting pts: %llu\n", pts);
			return pts == UINT64_MAX ? 0 : pts;
		}
	}
	
	ASF_LOG ("ASFReader::GetLastAvailablePts (): returing 0 (searched all packets)\n");

	return 0;
}

/*
 *	ASFFrameReader
 */

ASFFrameReader::ASFFrameReader (ASFParser *p, int s, IMediaDemuxer *d, ASFReader *r)
{
	reader = r;
	stream_number = s;
	parser = p;
	demuxer = d;
	first = NULL;
	last = NULL;
	size = 0;
	pts = 0;
	payloads = NULL;
	
	payloads_size = 0;
	payloads = NULL;
	
	first_pts = 0;
	
	script_command_stream_index = 0;
	FindScriptCommandStream ();
	
	index = NULL;
	index_size = 0;
	key_frames_only = false;
	positioned = false;
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
}

void
ASFFrameReader::Reset ()
{
	key_frames_only = true;
	first_pts = 0;
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
ASFFrameReader::AddFrameIndex (uint64_t packet_index)
{
	// No need to create an index if we can't seek.
	if (!reader->CanSeek ())
		return;
		
	int64_t packet_count = parser->GetPacketCount ();
	
	// Create the index.
	if (index_size == 0) {
		if (packet_count > 0xFFFF) {
			// This is some really huge file (or a corrupted file).
			// Don't create any indices, since it would consume a whole lot of memory.
			//printf ("ASFFrameReader::AddFrameIndex (): Not creating index, too many packets to track (%llu)\n", packet_count);
			return;
		}
		
		// Max size here is 0xFFFF packets * 16 bytes per index = 1.048.560 bytes
		index_size = packet_count;
		
		// Don't create any indices if there are no packets. 
		if (index_size == 0)
			return;
		
		index = (ASFFrameReaderIndex*) g_malloc0 (index_size * sizeof (ASFFrameReaderIndex));
		
		//printf ("ASFFrameReader::AddFrameIndex (): Created index: stream_count: %i, packet_count: %lld, index_size: %i, item size: %i, gives index size: %i bytes\n", stream_count, packet_count, index_size, sizeof (ASFFrameReaderIndex), index_size * sizeof (ASFFrameReaderIndex));
		
		if (index == NULL) {
			index_size = 0;
			return;
		}
		
		for (int i = 0; i < (int) packet_count; i++) {
			index [i].start_pts = INVALID_START_PTS;
		}
	}
	 
	// index_size can't be 0 here.
	uint32_t k = MIN (packet_index, index_size - 1);
	uint64_t current_start = index [k].start_pts;
	index [k].start_pts = MIN (index [k].start_pts, Pts ());
	index [k].end_pts = MAX (index [k].end_pts, Pts ());
	if (k > 1 && current_start != INVALID_START_PTS) {
		index [k].start_pts = MAX (index [k - 1].end_pts, current_start);		
	}

	//printf ("ASFFrameReader::AddFrameIndex (%llu). k = %u, start_pts = %llu, end_pts = %llu, stream = %i\n", packet_index, k, index [k].start_pts, index [k].end_pts, stream_number);
}

uint32_t
ASFFrameReader::FrameSearch (uint64_t pts)
{
	for (uint32_t i = 0; i < index_size; i++) {
		//printf ("ASFFrameReader::FrameSearch (%llu): Checking start_pts: %llu, end_pts: %llu, pi: %i\n", pts, index [i].start_pts, index [i].end_pts, index [i].packet_index);
		
		if (index [i].start_pts == INVALID_START_PTS)
			continue; // This index isn't set
			
		if (index [i].start_pts > pts) {
			//printf ("ASFFrameReader::FrameSearch (%llu): index not created for the desired pts (found starting pts after the requested one)\n", pts);
			return UINT32_MAX;
		}
		
		if (index [i].start_pts <= pts && index [i].end_pts >= pts) {
			//printf ("ASFFrameReader::FrameSearch (%llu): found packet index: %i.\n", pts, index [i].packet_index);
			return i;
		}
		
	}
	
	//printf ("ASFFrameReader::FrameSearch (%llud): searched entire index and didn't find anything.\n", pts);
			
	return -1;
}

void
ASFFrameReader::FindScriptCommandStream ()
{
	if (script_command_stream_index > 0)
		return;
	
	for (int i = 1; i <= 127; i++) {
		const asf_stream_properties* stream = parser->GetStream (i);
		//printf ("Checking guid of stream %i (%p): %s against %s\n", i, stream, stream == NULL ? "-" : asf_guid_tostring (&stream->stream_type), stream == NULL ? "-" : asf_guid_tostring (&asf_guids_media_command));
		if (stream != NULL && asf_guid_compare (&stream->stream_type, &asf_guids_media_command)) {
			script_command_stream_index = i;
			break;
		}
	}
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
ASFFrameReader::SetFirstPts (uint64_t pts)
{
	first_pts = pts;
}

MediaResult
ASFFrameReader::Advance ()
{
start:
	MediaResult result = MEDIA_SUCCESS;
	MediaResult read_result;
	int payload_count = 0;
	uint32_t media_object_number = 0;
	uint64_t current_pts = 0;
	uint64_t first_packet_index = 0; // The packet index where the frame starts.
	ASFFrameReaderData* current = NULL;
	
	ASF_LOG ("ASFFrameReader::Advance ().\n");
	
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
	current = first;
	pts = 0;
	
	ASF_LOG ("ASFFrameReader::Advance (): frame data: size = %lld, key = %s, pts = %llu, stream# = %d, media_object_number = %u.\n", 
		 size, IsKeyFrame () ? "true" : "false", Pts (), StreamId (), media_object_number);
	
	while (true) {
		// Loop through payloads until we find a payload with the different media number
		// than the first payload in the queue.
		
		// Make sure we have any payloads in our queue of payloads
		while (current == NULL) {
			// We went past the end of the payloads, read another packet to get more data.
			current = last; // go back to the last element.
			
			ASF_LOG ("ASFFrameReader::Advance (): No more payloads, requesting more data.\n");

			read_result = reader->ReadMore ();
			if (read_result == MEDIA_NO_MORE_DATA) {
				// No more data, we've reached the end
				ASF_LOG ("ASFFrameReader::Advance (): No more data, payload count: %i\n", payload_count);
				if (payload_count == 0)				
					result = read_result;
				goto end_frame;
			} else if (!MEDIA_SUCCEEDED (read_result)) {
				result = read_result;
				goto end_frame;
			} else {
				if (current == NULL) {
					// There was no elements before reading more, our next element is the first one
					current = first;
				} else {
					current = current->next;
				}
			}
		}
		
		ASF_LOG ("ASFFrameReader::Advance (): checking payload, stream: %d, media object number %d, size: %d\n", current->payload->stream_id, current->payload->media_object_number, current->payload->payload_data_length);
		
		asf_single_payload* payload = current->payload;
		current_pts = MilliSeconds_ToPts (payload->get_presentation_time () - parser->GetFileProperties ()->preroll);
		
		if (current_pts < first_pts) {
			ASFFrameReaderData* tmp = current;
			current = current->next;
			Remove (tmp);
		} else {
			if (payload_count > 0 && payload->media_object_number != media_object_number) {
				// We've found the end of the current frame's payloads
				ASF_LOG ("ASFFrameReader::Advance (): reached media object number %i (while reading %i).\n", payload->media_object_number, media_object_number);
				goto end_frame;
			}
						
			if (key_frames_only && !IsAudio () && !payload->is_key_frame) {
				ASF_LOG ("ASFFrameReader::Advance (): dropped non-key frame, pts: %llu\n", current_pts);
				ASFFrameReaderData* tmp = current;
				current = current->next;
				Remove (tmp);
				continue;
			}
			
			if (payload_count == 0 && payload->offset_into_media_object != 0) {
				// This frame isn't complete, it's probably split over several packets (and we haven't read the first of those packets).
				ASF_LOG ("ASFFrameReader::Advance (): skipping incomplete frame, pts: %llu, offset into media object: %i.\n", current_pts, payload->offset_into_media_object);
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
		
		ASF_LOG ("ASFFrameReader::Advance (): current is %p.\n", current);
	}
	
end_frame:
/*
	printf ("ASFFrameReader::Advance (): frame data: size = %.4lld, key = %s, pts = %.5llu, stream# = %i, media_object_number = %.3u, script_command_stream_index = %u (advanced).", 
		size, IsKeyFrame () ? "true " : "false", Pts (), StreamNumber (), media_object_number, script_command_stream_index);

	dump_int_data (payloads [0]->payload_data, payloads [0]->payload_data_length, 4);
	printf ("\n");
*/
/*
	printf ("ASFFrameReader::Advance (): frame data: size = %.4lld, key = %s, Pts = %.5llu, pts = %.5u, stream# = %i, media_object_number = %.3u, script_command_stream_index = %u (advanced).\n", 
		size, IsKeyFrame () ? "true " : "false", Pts (), payloads [0]->presentation_time, StreamId (), media_object_number, script_command_stream_index);
*/
	// Check if the current frame is a script command, in which case we must call the callback set in 
	// the parser (and read another frame).
	if (StreamId () == script_command_stream_index && script_command_stream_index > 0) {
		printf ("reading script command\n");
		ReadScriptCommand ();
		goto start;
	}
	
	AddFrameIndex (first_packet_index);

	return result;
}

int64_t
ASFFrameReader::EstimatePtsPosition  (uint64_t pts)
{
	return parser->GetPacketOffset (MIN (parser->GetPacketCount () - 1, EstimatePacketIndexOfPts (pts) + 1));
}

uint64_t
ASFFrameReader::EstimatePacketIndexOfPts (uint64_t pts)
{
	//printf ("ASFFrameReader::GetPacketIndexOfPts (%llu)\n", pts);
	
	int32_t counter = 0;
	uint64_t average = 0; // average duration per packet
	uint64_t last_good_pi = 0;
	uint64_t last_good_pts = 0;
	uint64_t duration = 0;
	uint64_t total_duration = 0;
	uint64_t result = 0;
	uint64_t packet_index = 0;
	
	if (pts == 0) {
		return 0;
	}

	total_duration = parser->GetFileProperties ()->play_duration - MilliSeconds_ToPts (parser->GetFileProperties ()->preroll);
	if (pts >= total_duration) {
		return parser->GetPacketCount () - 1;
	}
	
	packet_index = FrameSearch (pts);
	
	if (packet_index != UINT32_MAX) {
		//printf ("ASFFrameReader::GetPositionOfPts (%llu): Found pts in index, position: %lld, pi: %i\n", pts, parser->GetPacketOffset (packet_index), packet_index);
		return packet_index;
	}
	
	for (uint32_t i = 0; i < index_size; i++) {
		if (!(index [i].start_pts != INVALID_START_PTS && index [i].end_pts > index [i].start_pts))
			continue;
		
		if (index [i].start_pts >= pts)
			break;
		
		last_good_pi = i;
		last_good_pts = index [i].start_pts;
		
		duration = index [i].end_pts - index [i].start_pts;
		counter++;
		average = (average / (double) counter) * (counter - 1) + (duration / (double) counter);
			
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%llu): Calculated average %llu after pi: %i, duration: %llu, start_pts: %llu, end_pts: %llu\n", pts, average, i, duration, index [i].start_pts, index [i].end_pts);
	}
	
	if (average == 0) {
		// calculate packet index from duration
		uint64_t duration = MAX (1, parser->GetFileProperties ()->play_duration - MilliSeconds_ToPts (parser->GetFileProperties ()->preroll));
		double percent = MAX (0, pts / (double) duration);
		result = percent * parser->GetPacketCount ();
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%llu): No average, calculated by percent %.2f, pi: %i, pts: %llu, preroll: %llu\n", pts, percent, pi, pts, preroll);
	} else {
		// calculate packet index from the last known packet index / pts and average pts per packet index
		last_good_pts = MIN (last_good_pts, pts);
		result = last_good_pi + (pts - last_good_pts) / average;
		//printf ("ASFFrameReader::GetPacketIndexOfPts (%llu): Calculated by averate %llu, last_good_pts: %llu, pi: %i\n", pts, average, last_good_pts, pi);
	}
	
	result = MAX (0, result);
	result = MIN (result, MAX (0, parser->GetPacketCount () - 1));
	
	//printf ("ASFFrameReader::GetPacketIndexOfPts (%llu): Final position: %lld of pi: %i. Total packets: %llu, total duration: %llu\n", pts, parser->GetPacketOffset (pi), pi, parser->GetFileProperties ()->data_packet_count, parser->GetFileProperties ()->play_duration);
	return result;
}

void
ASFFrameReader::ReadScriptCommand ()
{
	uint64_t pts;
	char *text;
	char *type;
	gunichar2 *data;
	gunichar2 *uni_type = NULL;
	gunichar2 *uni_text = NULL;
	int text_length = 0;
	int type_length = 0;
	ASF_LOG ("ASFFrameReader::ReadScriptCommand (), size = %llu.\n", size);

	if (parser->embedded_script_command == NULL) {
		ASF_LOG ("ASFFrameReader::ReadScriptCommand (): no callback set.\n");
		return;
	}

	data = (gunichar2*) g_malloc (Size ());
	
	if (!Write (data)) {
		ASF_LOG ("ASFFrameReader::ReadScriptCommand (): couldn't read the data.\n");
		return;
	}
	
	uni_type = data;
	pts = Pts ();
	
	// the data is two arrays of WCHARs (type and text), null terminated.
	// loop through the data, counting characters and null characters
	// there should be at least two null characters.
	int null_count = 0;
	
	for (uint32_t i = 0; i < (size / sizeof (gunichar2)); i++) {
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
	
	if (null_count >= 2) {
		text = wchar_to_utf8 (uni_text, text_length);
		type = wchar_to_utf8 (uni_type, type_length);
		
		ASF_LOG ("ASFFrameReader::ReadScriptCommand (): sending script command to %p, type: '%s', text: '%s', pts: '%llu'.\n", parser->embedded_script_command, type, text, pts);
		parser->embedded_script_command (parser->embedded_script_command_state, type, text, pts);
		
		g_free (text);
		g_free (type);
	} else {
		ASF_LOG ("ASFFrameReader::ReadScriptCommand (): didn't find 2 null characters in the data.\n");
	}
	
	g_free (data);

}

void
ASFFrameReader::AppendPayload (asf_single_payload *payload, uint64_t packet_index)
{
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

#if 0
	int counter = 0;
	node = first;
	while (node != NULL) {
		counter++;
		node = node->next;
	}
	printf ("ASFFrameReader::AppendPayload (%p, %llu). Stream #%i now has %i payloads.\n", payload, packet_index, StreamId (), counter);
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
