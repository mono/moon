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
#include "asf.h"

int main(int argc, char *argv[])
{
	print_sizes ();
	
	const char* filename = "/tmp/BoxerSmacksdownInhoffe.wmv";
	filename = "/mono/head/moon/tmp/media/StarryNightTVCM_400kbps.wmv";
	
	if (argc >= 2)
		filename = argv [1];
	
	ASFParser* parser = new ASFParser (filename);
	parser->ReadHeader ();
	parser->ReadData ();
	ASFPacket* packet;
	while ((packet = new ASFPacket ()) && parser->ReadPacket (packet)) {
		delete packet;
	}
	delete parser;
}

void
asf_printfree (char *message)
{
	printf (message);
	g_free (message);
}

/*
	ASFParser
*/

ASFParser::ASFParser (const char* filename)
{
	ASF_LOG ("ASFParser::ASFParser ('%s'), this: %p.\n", filename, this);
	source = new ASFFileSource (this, filename);
	header = NULL;
	data = NULL;
	data_offset = 0;
	packet_offset = 0;
	packet_offset_end = 0;
	header_objects = NULL;
	last_error = NULL;
	file_properties = NULL;
	
	memset (stream_properties, 0, sizeof (asf_stream_properties*) * 127);
}

ASFParser::~ASFParser ()
{
	ASF_LOG ("ASFParser::~ASFParser ().\n");
	delete source;
	source = NULL;
		
	g_free (last_error);
	last_error = NULL;
	
	g_free (header);
	header = NULL;
	
	g_free (data);
	data = NULL;
	
	g_free (header_objects);
	header_objects = NULL;
}

asf_object*
ASFParser::ReadObject (asf_object* obj)
{
	ASF_LOG ("ASFParser::ReadObject ('%s', %llu)\n", asf_guid_tostring (&obj->id), obj->size);

	asf_object* result = NULL;
	ASFTypes type = asf_get_guid_type (&obj->id);
	
	if (type == ASF_NONE) {
		AddError (g_strdup_printf ("Unrecognized guid: %s.\n", asf_guid_tostring (&obj->id)));
		return NULL;
	}
	
	result = (asf_object*) g_malloc0 (obj->size);
	
	if (result == NULL) {
		AddError ("Out of memory.\n");
		return NULL;
	}

	memcpy (result, obj, sizeof (asf_object));

	if (obj->size > sizeof (asf_object)) {
		source->Read (((char*) result) + sizeof (asf_object), obj->size - sizeof (asf_object));
	}
	
	if (!asf_object_validate_exact (result, this)) {
		g_free (result);
		return NULL;
	}

	return result;
}

bool
ASFParser::ReadPacket (ASFPacket* packet, gint32 packet_index)
{
	ASF_LOG ("ASFParser::ReadPacket (%p, %i).\n", packet, packet_index);
	
	if (packet_index >= 0) {
		guint64 position = GetPacketOffset (packet_index);

		if (position == 0 || !source->Seek (position, SEEK_SET))
			return false;
	}
	
	return ASFParser::ReadPacket (packet);
}

bool
ASFParser::ReadPacket (ASFPacket* packet)
{
	ASF_LOG ("ASFParser::ReadPacket (%p): Reading packet at %lld (index: %i) of %lld packets.\n", packet, source->Position (), GetPacketIndex (source->Position ()), data->data_packet_count);
	
	asf_error_correction_data ecd;
	asf_payload_parsing_information ppi;
	
	//int64_t start_position = source->Position ();
	
	if (!ecd.FillInAll (source))
		return false;
	
	asf_error_correction_data_dump (&ecd);
	
	if (!ppi.FillInAll (source))
		return false;
	
	asf_payload_parsing_information_dump (&ppi);
	
	asf_multiple_payloads* mp = new asf_multiple_payloads ();
	if (ppi.is_multiple_payloads_present ()) {
		ASF_LOG ("ASFParser::ReadPacket (), reading multiple payloads.\n");
		if (!mp->FillInAll (source, &ecd, ppi)) {
			return false;
		}
	} else {
		ASF_LOG ("ASFParser::ReadPacket (), reading single payload.\n");
		asf_single_payload* sp = new asf_single_payload ();
		if (!sp->FillInAll (source, &ecd, ppi, NULL)) {
			asf_single_payload_dump (sp);
			return false;
		}
		//asf_single_payload_dump (sp);
		mp->payloads = (asf_single_payload**) g_malloc0 (sizeof (asf_single_payload*) * 2);
		mp->payloads [0] = sp;
		mp->payload_flags = 1; // 1 payload
	}
	//asf_multiple_payloads_dump (mp);
	packet->payloads = mp;
	
//	if (!source->Skip (ppi.padding_length))
//		return false;
	
	ASF_LOG ("ASFParser::ReadPacket (): Current position (end of packet): %llx (%lld), start position was: %llx (%lld), difference: %llx (%lld)\n", 
		source->Position (), source->Position (), 
		start_position, start_position,
		source->Position () - start_position, source->Position () - start_position);
	
	return true;
}

bool
ASFParser::ReadData ()
{
	ASF_LOG ("ASFParser::ReadData ().\n");
	
	asf_data* data = NULL;
	if (!source->Seek (header->size, SEEK_SET)) {
		return false;
	}
	ASF_LOG ("Current position: %llx (%lld)\n", source->Position (), source->Position ());
	data = (asf_data*) g_malloc0 (sizeof (asf_data));
	if (!source->Read (data, sizeof (asf_data)))
		return false;
	
	asf_object_dump_exact (data);
	
	ASF_LOG ("Data %p has %lld packets.\n", data, data->data_packet_count);
	
	this->data = data;
	
	return true;
}

bool
ASFParser::ReadHeader ()
{
	ASF_LOG ("ASFParser::ReadHeader ().\n");
	
	header = (asf_header*) g_malloc0 (sizeof (asf_header));
	
	if (!source->Read (header, sizeof (asf_header)))
		return false;
		
	asf_header_dump (header);

	if (!asf_header_validate (header, this)) {
		ASF_LOG ("Header validation failed, error: '%s'\n", GetLastError ());
		return false;
	}
	
	header_objects = (asf_object**) g_malloc0 ((header->object_count + 1) * sizeof (asf_object*));
	
	for (asf_dword i = 0; i < header->object_count; i++) {
		asf_object tmp;
		if (!source->Read (&tmp, sizeof (asf_object))) {
			return false;
		}
		
		header_objects [i] = ReadObject (&tmp);
		if (header_objects [i] == NULL) {
			return false;
		}
		
		if (asf_guid_compare (&asf_guids_stream_properties, &header_objects [i]->id)) {
			asf_stream_properties* stream = (asf_stream_properties*) header_objects [i];
			SetStream (stream->get_stream_number (), stream);
		}
		
		asf_object_dump_exact (header_objects [i]);
	}
	
	// TODO: Validate existence of mandatory header objects
	file_properties = (asf_file_properties*) GetHeaderObject (&asf_guids_file_properties); 
	data_offset = header->size;
	packet_offset = data_offset + sizeof (asf_data);
	packet_offset_end = packet_offset + file_properties->data_packet_count * file_properties->min_packet_size;

	ASF_LOG ("ASFParser::ReadHeader (): Header read successfully.\n");
	
	if (!ReadData ())
		return false;
		
	return true;
}

const char*
ASFParser::GetLastError ()
{
	return last_error;
}

void
ASFParser::AddError (const char* err)
{
	// FIXME: Ability to report more than one error.
	
	ASF_LOG ("ASFParser::AddError ('%s').\n", err);
	
	if (last_error)
		g_free (last_error);

	last_error = g_strdup (err);
}

void
ASFParser::AddError (char* err)
{
	AddError ((const char*) err);
	g_free (err);
}

/*
	ASFFileSource
*/

ASFFileSource::ASFFileSource (ASFParser* parser, const char* fn) : ASFSource (parser)
{
	filename = g_strdup (fn);	
}

ASFFileSource::~ASFFileSource ()
{
	g_free (filename);
	filename = NULL;
	if (fd) {
		fclose (fd);
		fd = NULL;
	}
}

bool
ASFFileSource::Seek (size_t offset, int mode)
{
	if (fseek (fd, offset, mode) != 0) {
		parser->AddError (g_strdup_printf ("Can't seek to offset %i with mode %i in '%s': %s.\n", offset, mode,  filename, strerror (errno)));
		return false;
	}
	
	return true;
}

bool
ASFFileSource::ReadInternal (void* destination, size_t bytes)
{
	size_t bytes_read;
	
	if (destination == NULL) {
		parser->AddError ("Out of memory.\n");
		return false;
	}

	if (!fd) {
		fd = fopen (filename, "rb");
		if (!fd) {
			parser->AddError (g_strdup_printf ("Could not open the file '%s': %s.\n", filename, strerror (errno)));
			return false;
		}
	}

	bytes_read = fread (destination, 1, bytes, fd);
	
	if (bytes_read != bytes) {
		if (ferror (fd) != 0) {
			parser->AddError (g_strdup_printf ("Could not read from the file '%s': %s.\n", filename, strerror (errno)));
		} else if (feof (fd) != 0) {
			parser->AddError (g_strdup_printf ("Reached end of file prematurely of the file '%s'.\n", filename));
		} else {
			parser->AddError (g_strdup_printf ("Unspecified error while reading the file '%s'.\n", filename));
		}
		return false;
	}

	return true;
}

bool
ASFSource::Read (void* destination, size_t bytes)
{
	bool result;
		
	if (bytes == 986)
		result = false;
	
	result = ReadInternal (destination, bytes);
	
	//printf ("ASFSource::Read (%.8p, %4i), pp: %lld, cp: %lld, result: %s", destination, bytes, position, Position (), result ? "true" : "false");
	if (false && result) {
		if (bytes == 986) {
			for (guint32 i = 0; i < bytes; i++) {
				printf (" %.2hhX", (int) *(i + (char*) destination));
			}
		} else {
			for (guint32 i = 0; (i < bytes && i < 10); i++) {
				printf (" %.2hhX", (int) *(i + (char*) destination));
			}
			printf ("; ");
			for (guint32 i = (bytes > 20 ? bytes - 10 : 10); i < bytes; i++) {
				printf (" %.2hhX", (int) *(i + (char*) destination));
			}
		}
		printf (".\n");
	}
	
	return result;
}

bool 
ASFSource::ReadEncoded (int length, asf_dword* destination)
{
	asf_word result2 = 0;
	asf_byte result1 = 0;
	
	switch (length) {
	case 0x00: return true;
	case 0x01: 
		if (!Read (&result1, 1))
			return false;
		*destination = result1;
		return true;
	case 0x02:
		if (!Read (&result2, 2))
			return false;
		*destination = result2;
		return true;
	case 0x03:
		return Read (destination, 4);
	default:
		parser->AddError (g_strdup_printf ("Invalid read length: %i.", length));
		return false;
	}
}

/*
	ASFFrameReader
*/

ASFFrameReader::ASFFrameReader (ASFParser* p)
{
	parser = p;
	first = NULL;
	last = NULL;
	size = 0;
	is_key_frame = true;
	pts = 0;
	stream_number = 0;
	payloads = NULL;
	current_packet_index = CanSeek () ? 0 : -1;
	
	// Most streams has at least once a media object spanning two payloads.
	// so we allocate space for two (+ NULL at the end).
	payloads_size = 2;
	payloads = (asf_single_payload**) g_malloc0 (sizeof (asf_single_payload*) * (payloads_size + 1));
}

ASFFrameReader::~ASFFrameReader ()
{
	parser = NULL;
	
	RemoveAll ();
	
	gint32 i = -1;
	while (payloads [++i] != NULL)
		delete payloads [i];
	g_free (payloads); payloads = NULL;
}

bool
ASFFrameReader::Seek (gint32 stream_number, guint64 pts)
{
	ASF_LOG ("ASFFrameReader::Seek (%i, %llu).\n", stream_number, pts);
	
	if (!CanSeek ())
		return false;
		
	// Now this is an algorithm that needs some optimization.
	// We seek from the first frame to the frame AFTER the one we want (counting the numbers of frames)
	// then we seek again from the first frame until the number of frames counted - 1.	
	
	gint32 counter = 0;
	bool found = true;
	
	current_packet_index = 0;
	RemoveAll ();
	
	while (Advance ()) {
		if (Pts () > pts) {
			found = true;
			break;
		}
		counter++;
	}
	
	if (!found)
		return false;
	
	current_packet_index = 0;
	RemoveAll ();
	
	while (counter-- > 0) {
		if (!Advance ()) {
			return false;
		}
	}
		
	return true;
}

bool
ASFFrameReader::Advance ()
{
	ASF_LOG ("ASFFrameReader::Advance ().\n");
	// Clear the current list of payloads.
	int i = -1;
	while (payloads [++i] != NULL) {
		delete payloads [i];
		payloads [i] = NULL;
	}
	
	if (first == NULL) {
		if (!ReadMore ())
			return false;
		if (first == NULL)
			return false;
	}
	gint32 payload_count = 1;
	payloads [0] = first->payload;
	gint32 media_object_number = payloads [0]->media_object_number;
	size = payloads [0]->payload_data_length;
	is_key_frame = payloads [0]->is_key_frame;
	pts = payloads [0]->get_presentation_time ();
	stream_number = payloads [0]->stream_number;
	
	ASF_LOG ("ASFFrameReader::Advance (): frame data: size = %lld, key = %s, pts = %llu, stream# = %i.\n", 
		size, is_key_frame ? "true" : "false", pts, stream_number);
	
	bool end = false;
	ASFFrameReaderData *current = first->next;
	
	Remove (first);
	
	while (!end) {
		// Loop through payloads until we find a payload with the same stream number.
		// if the media number is different, no more payloads in the current frame,
		// otherwise add the payload to the current frame's payloads and continue looping.
		while (current == NULL) {
			// We went past the end, read another packet to get more data.
			current = last; // go back to the last element.
			if (!ReadMore ()) {
				end = true; // No more data, we've reached the end
				break;
			} else {
				if (current == NULL) {
					// There was no elements before reading more, our next element is the first one
					current = first;
				} else {
					current = current->next;
				}
			}
		}
		
		if (end)
			break;
		
		if (current->payload->stream_number == stream_number) {
			if (current->payload->media_object_number == media_object_number) {
				// Add the payload to the current frame's payloads
				payload_count++;
				size += current->payload->payload_data_length;
				if (payload_count > payloads_size) {
					gint32 new_size = payload_count + 3;
					payloads = (asf_single_payload**) g_realloc (payloads, sizeof (asf_single_payload*) * (new_size + 1));
					memset (payloads + payloads_size, 0, sizeof (asf_single_payload*) * (new_size - payloads_size));
					payloads_size = new_size;
				}
				payloads [payload_count - 1] = current->payload;
				
				// Remove it from the queue
				ASFFrameReaderData* tmp = current;
				current = current->prev;
				
				tmp->payload = NULL;
				Remove (tmp);
				delete tmp;
			} else {
				// We've found the end of the current frame's payloads
				end = true;
			}
		}
		
		if (current != NULL)
			current = current->next;
	}
	
	return true;
}

bool
ASFFrameReader::ReadMore ()
{
	ASFPacket* packet = new ASFPacket ();
	
	ASF_LOG ("ASFFrameReader::ReadMore ().\n");
	
	if (!parser->ReadPacket (packet, current_packet_index)) {
		ASF_LOG ("ASFFrameReader::ReadMore (): could not read more packets.\n");
		delete packet;
		return false;
	}
	
	if (CanSeek ()) {
		current_packet_index++;
	}
	
	asf_single_payload** payloads = packet->payloads->steal_payloads ();
	int i = -1;
	while (payloads [++i] != NULL) {
		// Append the payload at the end of the queue of payloads.
		ASFFrameReaderData* node = new ASFFrameReaderData (payloads [i]);
			
		if (first == NULL) {
			first = node;
			last = node;
		} else {
			node->prev = last;
			last->next = node;
			last = node;
		}
	}
	
	ASF_LOG ("ASFFrameReader::ReadMore (): read %i payloads.\n", i);
		
	delete packet;
	return true;
}

bool
ASFFrameReader::Write (void* destination)
{
	if (payloads == NULL)
		return false;

	void* start = destination;
	gint32 i = -1;
	while (payloads [++i] != NULL) {
		memcpy (destination, payloads [i]->payload_data, payloads [i]->payload_data_length);
		destination = payloads [i]->payload_data_length + (char*) destination;
	}
	
	return true;
}

