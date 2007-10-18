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
	while (parser->ReadPacket ())
		printf ("One more packet read.\n");
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
	printf ("ASFParser::ASFParser ('%s'), this: %p.\n", filename, this);
	source = new ASFFileSource (this, filename);
	header = NULL;
	data = NULL;
	data_offset = 0;
	packet_offset = 0;
	packet_offset_end = 0;
	header_objects = NULL;
	last_error = NULL;
	current_packet = NULL;
	file_properties = NULL;
	
	memset (stream_properties, 0, sizeof (asf_stream_properties*) * 127);
}

ASFParser::~ASFParser ()
{
	printf ("ASFParser::~ASFParser ().\n");
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
	
	g_free (current_packet);
	current_packet = NULL;
}

asf_object*
ASFParser::ReadObject (asf_object* obj)
{
#if DEBUG
	printf ("ASFParser::ReadObject ('%s', %llu)\n", asf_guid_tostring (&obj->id), obj->size);
#endif

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
ASFParser::ReadPacket ()
{
	return ASFParser::ReadPacket (new ASFPacket ());
}

bool
ASFParser::ReadPacket (ASFPacket* next_packet)
{
	printf ("ASFParser::ReadPacket (%p), data: %p\n", next_packet, data);
	
	if (!data && !ReadData ())
		return false;

	if (current_packet) {
		delete current_packet;
		current_packet = NULL;
	}
	
	printf ("ASFParser::ReadPacket (): Reading packet at %lld (index: %i) of %lld packets.\n", source->Position (), GetPacketIndex (source->Position ()), data->data_packet_count);
	
	asf_error_correction_data ecd;
	asf_payload_parsing_information ppi;
	
	int64_t start_position = source->Position ();
	
	//printf ("Current position: %llx (%lld)\n", source->Position (), source->Position ());
	
	if (!ecd.FillInAll (source))
		return false;
	
	//asf_error_correction_data_dump (&ecd);
	
	//printf ("Current position: %llx (%lld)\n", source->Position (), source->Position ());
	
	if (!ppi.FillInAll (source))
		return false;
	
	//asf_payload_parsing_information_dump (&ppi);
	
	//printf ("Current position: %llx (%lld)\n", source->Position (), source->Position ());
	
	asf_multiple_payloads* mp = new asf_multiple_payloads ();
	if (ppi.is_multiple_payloads_present ()) {
		printf ("ASFParser::ReadPacket (), reading multiple payloads.\n");
		if (!mp->FillInAll (source, &ecd, ppi)) {
			return false;
		}
	} else {
		printf ("ASFParser::ReadPacket (), reading single payload.\n");
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
	next_packet->payloads = mp;
	
	if (!source->Skip (ppi.padding_length))
		return false;
	
	current_packet = next_packet;
	
	printf ("ASFParser::ReadPacket (): Current position (end of packet): %llx (%lld), start position was: %llx (%lld), difference: %llx (%lld)\n", 
		source->Position (), source->Position (), 
		start_position, start_position,
		source->Position () - start_position, source->Position () - start_position);
	
	return true;
}

bool
ASFParser::ReadData ()
{
	printf ("ASFParser::ReadData ().\n");
	
	asf_data* data = NULL;
	if (!source->Seek (header->size, SEEK_SET)) {
		return false;
	}
	printf ("Current position: %llx (%lld)\n", source->Position (), source->Position ());
	data = (asf_data*) g_malloc0 (sizeof (asf_data));
	if (!source->Read (data, sizeof (asf_data)))
		return false;
	
	asf_object_dump_exact (data);
	
	printf ("Data %p has %lld packets.\n", data, data->data_packet_count);
	
	this->data = data;
	
	return true;
}

bool
ASFParser::ReadHeader ()
{
	printf ("ASFParser::ReadHeader ().\n");
	
	header = (asf_header*) g_malloc0 (sizeof (asf_header));
	
	if (!source->Read (header, sizeof (asf_header)))
		return false;
		
	asf_header_dump (header);

	if (!asf_header_validate (header, this)) {
#if DEBUG
		printf ("Header validation failed, error: '%s'\n", GetLastError ());
#endif
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

	printf ("ASFParser::ReadHeader (): Header read successfully.\n");
	
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
	
#if DEBUG
	printf ("ASFParser::AddError ('%s').\n", err);
#endif
	
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





