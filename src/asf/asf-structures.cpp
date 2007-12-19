/*
 * asf.cpp: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "asf.h"

char*
wchar_to_utf8 (void* unicode, guint32 length)
{
	char* result = NULL;
	
	if (length <= 0)
		return NULL;
	
	GError* err = NULL;
	result = g_utf16_to_utf8 ((const gunichar2*) unicode, length, NULL, NULL, &err);
	if (result == NULL) {
		ASF_LOG ("Could not convert to utf8from utf16: %s\n", err->message);
		g_error_free (err);
		err = NULL;
	}
	
	return result;
}

bool
asf_header_validate (const asf_header* header, ASFParser* parser)
{
	// SPEC: This field shall be set to ASF_Header_Object
	if (!asf_guid_validate (&header->id, &asf_guids_header, parser)) {
		return false;
	}
		
	// SPEC: valid values are at least 30 bytes
	if (header->size < 30) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 30, got %llu).\n", header->size));
		return false;
	}
	
	// SPEC: This field must be set to the value 0x02. If the this value is different when read, the application should fail to source the content.
	if (header->reserved2 != 0x02) {
		parser->AddError (g_strdup_printf ("Invalid reserved2 value (expected 0x02, got: %x).\n", header->reserved2));
		return false;
	}
	
	return true;
}

bool
asf_file_properties_validate (const asf_file_properties* obj, ASFParser* parser)
{
	// SPEC: This field shall be set to ASF_File_Properties_Object
	if (!asf_guid_validate (&obj->id, &asf_guids_file_properties, parser)) {
		return false;
	}
	
	// SPEC: Valid values are at least 104 bytes
	if (obj->size < 104) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 104, got %llu).\n", obj->size));
		return false;
	}

	// SPEC: the values for the Minimum Data Packet Size and Maximum Data Packet Size 
	// fields shall be set to the same value, and this value should be set to the packet size, 
	// even when the Broadcast Flag in the Flags field is set to 1.
	if (obj->min_packet_size != obj->max_packet_size) {
		// This is not logical at all, but it's what the spec says.
		// besides, our code depends on it (it makes a few minor things easier).
		parser->AddError (g_strdup_printf ("The min packet size (%i) is different from the max packet size (%i).\n", obj->min_packet_size, obj->max_packet_size));
		return false;
	}

	if (obj->size > parser->header->size) {
		parser->AddError (g_strdup_printf ("The size of the file property object (%llu) is bigger than the sizeof the entire header itself (%llu).\n", obj->size, parser->header->size)); 
		return false;
	}

	return true;
}

bool
asf_stream_properties_validate (const asf_stream_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_stream_properties, parser))) {
		return false;
	}
	
	if (obj->size < 78) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 78, got %llu).\n", obj->size));
		return false;
	}

	return true;
}

bool
asf_header_extension_validate (const asf_header_extension* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_header_extension, parser))) {
			return false;
	}
	
	if (obj->size < 46) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 46, got %llu).\n", obj->size));
		return false;
	}
	
	if (obj->data_size < 24 && obj->data_size > 1) {
		parser->AddError (g_strdup_printf ("Invalid data_size (expected >= 24 or 0, got %u).\n", obj->data_size));
		return false;
	} else if (obj->data_size != 0 && obj->data_size + 46 != obj->size) {
		parser->AddError (g_strdup_printf ("Invalid data_size (expected size - 46, got %llu - 46 = %u).\n", obj->size, obj->data_size)); 
		return false;
	}
	
	return true;
}

bool asf_codec_list_validate (const asf_codec_list* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_codec_list, parser))) {
		return false;
	}
	
	if (obj->size < 44) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_script_command_validate (const asf_script_command* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_script_command, parser))) {
		return false;
	}

	if (obj->size < 44) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_marker_validate (const asf_marker* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_marker, parser))) {
		return false;
	}

	if (obj->size < 48) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 48, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_bitrate_mutual_exclusion_validate (const asf_bitrate_mutual_exclusion* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_bitrate_mutual_exclusion, parser))) {
		return false;
	}

	if (obj->size < 42) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 42, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_error_correction_validate (const asf_error_correction* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_error_correction, parser))) {
		return false;
	}

	if (obj->size < 44) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_content_description_validate (const asf_content_description* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_content_description, parser))) {
		return false;
	}

	if (obj->size < 34) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 34, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_extended_content_description_validate (const asf_extended_content_description* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_extended_content_description, parser))) {
		return false;
	}

	if (obj->size < 26) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 26, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_stream_bitrate_properties_validate (const asf_stream_bitrate_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_stream_bitrate_properties, parser))) {
		return false;
	}
	
	if (obj->size < 26) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 26, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_data_validate (const asf_data* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_data, parser))) {
		return false;
	}

	if (obj->size < 50) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %llu).\n", obj->size));
		return false;
	}
	
	if (!(asf_guid_compare (&obj->file_id, &parser->GetFileProperties ()->file_id))) {
		parser->AddError ("Data file id and header's file properties file id don't match.");
		return false;
	}
	
	// TODO: More verifications?
	return true;
}

bool
asf_error_correction_data::FillInAll (ASFParser* parser)
{
	ASFSource* source = parser->source;
	data = 0;
	first = 0;
	second = 0;
		
	if (!source->Read (&data, 1))
		return false;
		
	if (!is_error_correction_present ())
		return true;
		
	if (!source->Read (&first, 1))
		return false;
	
	if (!source->Read (&second, 1))
		return false;
		
	return true;
}

void asf_error_correction_data_dump (asf_error_correction_data* obj)
{
	char* tostring = obj->tostring ();
	ASF_DUMP ("ASF_ERROR_CORRECTION_DATA\n");
	ASF_DUMP ("\tdata = 0x%X\n", (asf_dword) obj->data);
	ASF_DUMP ("\tdata = 0b%s\n",  tostring); g_free (tostring);
	ASF_DUMP ("\t\tis_error_correction_present: %i\n", obj->is_error_correction_present ());
	ASF_DUMP ("\t\tis_opaque_data_present: %i\n", obj->is_opaque_data_present ());
	ASF_DUMP ("\t\tdata_length: %i\n", obj->get_data_length ());
	ASF_DUMP ("\t\tlength_type: %i\n", obj->get_error_correction_length_type ());
	ASF_DUMP ("\tfirst = %X\n", (asf_dword) obj->first);
	
	ASF_DUMP ("\tsecond = %X\n", (asf_dword) obj->second);
}

bool
asf_payload_parsing_information::FillInAll (ASFParser* parser)
{
	ASFSource* source = parser->source;
	// There's no guarantee that these fields will be written to by Read ()
	
	packet_length = 0;
	sequence = 0;
	padding_length = 0;
	send_time = 0;
	duration = 0;
	
	if (!source->Read (&length_type_flags, 1))
		return false;
	if (!source->Read (&property_flags, 1))
		return false;
		
	if (get_packet_length_type () == 0) {
		packet_length = source->parser->GetFileProperties ()->min_packet_size;
	} else {
		if (!source->ReadEncoded (get_packet_length_type (), &packet_length))
			return false;
	}
	if (!source->ReadEncoded (get_sequence_type (), &sequence))
		return false;
	if (!source->ReadEncoded (get_padding_length_type (), &padding_length))
		return false;
	if (!source->Read (&send_time, 4))
		return false;
	if (!source->Read (&duration, 2))
		return false;
		
	return true;
}

void asf_payload_parsing_information_dump (asf_payload_parsing_information* obj)
{
	ASF_DUMP ("ASF_PAYLOAD_PARSING_INFORMATION\n");
	ASF_DUMP ("\tlength_type_flags = %X\n", (asf_dword) obj->length_type_flags);
	ASF_DUMP ("\t\tmultiple_payloads_present = %i\n", obj->is_multiple_payloads_present ());
	ASF_DUMP ("\t\tsequence_type = %i\n", obj->get_sequence_type ());
	ASF_DUMP ("\t\tpadding_length_type = %i\n", obj->get_padding_length_type ());
	ASF_DUMP ("\t\tpacket_length_type = %i\n", obj->get_packet_length_type ());
	ASF_DUMP ("\tproperty_flags = %X\n", (asf_dword) obj->property_flags);
	ASF_DUMP ("\t\treplicated_data_length_type = %i\n", obj->get_replicated_data_length_type ());
	ASF_DUMP ("\t\toffset_into_media_object_length_type = %i\n", obj->get_offset_into_media_object_length_type ());
	ASF_DUMP ("\t\tmedia_object_number_length_type = %i\n", obj->get_media_object_number_length_type ());
	ASF_DUMP ("\t\tstream_number_length_type = %i\n", obj->get_stream_number_length_type ());
	ASF_DUMP ("\tpacket_length = %u\n", (asf_dword) obj->packet_length);
	ASF_DUMP ("\tsequence = %u\n", (asf_dword) obj->sequence);
	ASF_DUMP ("\tpadding_length = %u\n", (asf_dword) obj->padding_length);
	ASF_DUMP ("\tsend_time = %u\n", (asf_dword) obj->send_time);
	ASF_DUMP ("\tduration = %u\n", (asf_dword) obj->duration);
}

bool
asf_single_payload::FillInAll (ASFParser* parser, asf_error_correction_data* ecd, asf_payload_parsing_information ppi, asf_multiple_payloads* mp)
{	
	ASFSource* source = parser->source;
	
	if (!source->Read (&stream_number, 1))
		return false;
	
	is_key_frame = stream_number & 0x80;
	stream_number = stream_number & 0x7F;
	
	if (!source->parser->IsValidStream (stream_number)) {
		ASF_LOG ("asf_single_payload::FillInAll: Invalid stream number (%i).\n", (asf_dword) stream_number);
		return false;
	}
	
	media_object_number = 0;
	offset_into_media_object =  0;
	replicated_data_length =  0;
	replicated_data = NULL;
	payload_data_length =  0;
	payload_data = NULL;
	presentation_time = 0;
	
	if (!source->ReadEncoded (ppi.get_media_object_number_length_type (), &media_object_number))
		return false;
		
	if (!source->ReadEncoded (ppi.get_offset_into_media_object_length_type (), &offset_into_media_object))
		return false;
		
	if (!source->ReadEncoded (ppi.get_replicated_data_length_type (), &replicated_data_length))
		return false;
	
	if (replicated_data_length >= 2 && replicated_data_length < 7) {
		source->parser->AddError (g_strdup_printf ("Invalid replicated data length: %i", replicated_data_length));
		return false;
	} 
		
	if (replicated_data_length > parser->file_properties->max_packet_size) {
		parser->AddError ("Data corruption in payload.");
		return false;
	}
	
	replicated_data = (asf_byte*) parser->MallocVerified (replicated_data_length);
	if (replicated_data == NULL) {
		return false;
	}
	
	if (!source->Read (replicated_data, replicated_data_length))
		return false;

	if (replicated_data_length == 1) {
		presentation_time = offset_into_media_object;
	} else if (replicated_data_length >= 8) {
		presentation_time = *(((asf_dword*) replicated_data) + 1);
	}

	if (mp != NULL) {
		if (!source->ReadEncoded (mp->get_payload_length_type (), &payload_data_length))
			return false;
			
		if (payload_data_length == 0) {
			source->parser->AddError ("Warning: Invalid payload data length: can't be 0.");
			//return false;
		}
	} else {
		int payload_length;
		// The number of bytes in this array can be calculated from the overall Packet Length field, 
		// and is equal to the Packet Length	
		payload_length = ppi.packet_length;
		// minus the packet header length,
		payload_length -= ppi.get_struct_size ();
		payload_length -= ecd->get_struct_size (),
		// minus the payload header length (including Replicated Data),
		payload_length -= 1; // stream_number
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi.get_media_object_number_length_type ());
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi.get_offset_into_media_object_length_type ());
		payload_length -= ASF_DECODE_PACKED_SIZE (ppi.get_replicated_data_length_type ());
		payload_length -= replicated_data_length;
		// minus the Padding Length.
		payload_length -= ppi.padding_length;
		ASF_LOG ("payload_length: %i. packet_length: %i, ppi.get_struct_size: %i, replicated_data_length: %i, padding_length: %i, ecd.get_struct_size: %i\n",
			payload_length, ppi.packet_length, ppi.get_struct_size (), replicated_data_length, ppi.padding_length, ecd->get_struct_size ());
			
		if (payload_length < 0) {
			source->parser->AddError (g_strdup_printf ("Invalid payload length: %i", payload_length));
			return false;
		} 
		
		payload_data_length = (asf_dword) payload_length;
	}
	
	if (payload_data_length > 0) {
		if (payload_data_length >= parser->file_properties->max_packet_size) {
			parser->AddError ("Data corruption in payload.");
			return false;
		}
		
		payload_data = (asf_byte*) parser->MallocVerified (payload_data_length);
		if (payload_data == NULL) {
			return false;
		}
		
		if (!source->Read (payload_data, payload_data_length))
			return false;
	}

	return true;
}

asf_single_payload::~asf_single_payload ()
{
	g_free (replicated_data);
	replicated_data = NULL;
	
	g_free (payload_data);
	payload_data = NULL;
}

void
asf_single_payload_dump (asf_single_payload* obj)
{
	ASF_DUMP ("ASF_SINGLE_PAYLOAD\n");
	ASF_DUMP ("\tstream_number = %u\n", (asf_dword) obj->stream_number);
	ASF_DUMP ("\tis_key_frame = %s\n", obj->is_key_frame ? "true" : "false");
	ASF_DUMP ("\tmedia_object_number = %u\n", (asf_dword) obj->media_object_number);
	ASF_DUMP ("\toffset_into_media_object = %u\n", (asf_dword) obj->offset_into_media_object);
	ASF_DUMP ("\treplicated_data_length = %u\n", (asf_dword) obj->replicated_data_length);
	ASF_DUMP ("\treplicated_data = %p\n", obj->replicated_data);
	ASF_DUMP ("\tpayload_data_length = %u\n", (asf_dword) obj->payload_data_length);
	ASF_DUMP ("\tpayload_data = %p\n", obj->payload_data);
	ASF_DUMP ("\tget_presentation_time = %lld\n", obj->get_presentation_time ());
}

bool
asf_multiple_payloads::ResizeList (ASFParser* parser, gint32 requested_size)
{
	if (requested_size <= payloads_size)
		return true;
		
	asf_single_payload** new_list;
	
	new_list = (asf_single_payload**) parser->MallocVerified ((requested_size + 1) * sizeof (asf_single_payload*));
	
	if (new_list == NULL) {
		return false;
	}
	
	if (payloads != NULL) {
		memcpy (new_list, payloads, sizeof (asf_single_payload*) * payloads_size);		
		g_free (payloads);
	}
	
	payloads = new_list;
	payloads_size = requested_size;
	
	return true;
}

gint32 
asf_multiple_payloads::CountCompressedPayloads (ASFParser* parser, asf_single_payload* payload)
{
	asf_byte* data = payload->payload_data;
	asf_dword length = payload->payload_data_length;
	asf_byte size = 0;
	guint32 offset = 0;
	gint32 counter = 0;
	
	if (data == NULL) {
		parser->AddError ("Compressed payload is corrupted.");
		return false;
	}
	
	while (true) {
		counter++;
		size = *(data + offset);
		offset += (size + 1);
		if (offset > length || size == 0) {
			parser->AddError ("Compressed payloads are corrupted.");
			return false;
		} else if (offset == length) {
			break;
		}
	};
	
	return counter;
}

bool
asf_multiple_payloads::ReadCompressedPayload (ASFParser* parser, asf_single_payload* first, gint32 count, gint32 start_index)
{
	asf_byte* data = first->payload_data;
	asf_byte size = 0;
	guint32 offset = 0;
	asf_single_payload* payload = NULL;

	for (int i = 0; i < count; i++) {
		size = *(data + offset);
		offset += 1;
		
		payload = new asf_single_payload ();
		payloads [start_index + i] = payload;
		
		payload->stream_number = first->stream_number;
		payload->is_key_frame = first->is_key_frame;
		payload->media_object_number = first->media_object_number + i;
		payload->offset_into_media_object = 0;
		payload->replicated_data_length = 0;
		payload->replicated_data = NULL;
		payload->presentation_time = first->presentation_time + i * first->get_presentation_time_delta ();
		payload->payload_data_length = size;
		payload->payload_data = (asf_byte*) parser->Malloc (size);
		if (payload->payload_data == NULL) {
			return false;
		}
		memcpy (payload->payload_data, data + offset, size);
		offset += size;
	}
	
	return true;
}

bool
asf_multiple_payloads::FillInAll (ASFParser* parser, asf_error_correction_data* ecd, asf_payload_parsing_information ppi)
{
	ASFSource* source = parser->source;
	gint32 count;
	
	if (ppi.is_multiple_payloads_present ()) {		
		if (!source->Read (&payload_flags, 1))
			return false;
		
		count = payload_flags & 0x3F; // number of payloads is encoded in a byte, no need to check for extreme values.
		
		if (count <= 0) {
			source->parser->AddError (g_strdup_printf ("Invalid number of payloads: %i", count));
			return false;
		}

		if (!ResizeList (parser, count)) {
			return false;
		}
		
		ASF_LOG ("asf_multiple_payloads::FillInAll (): Reading %i payloads...\n", count); 
		
		gint32 current_index = 0;
		for (int i = 0; i < count; i++) {
			payloads [current_index] = new asf_single_payload ();
			
			if (!payloads [current_index]->FillInAll (parser, ecd, ppi, this))
				return false;
			
			if (payloads [current_index]->is_compressed ()) {
				asf_single_payload* first = payloads [current_index];
				gint32 number = CountCompressedPayloads (parser, first);
				if (number <= 0) {
					return false;
				}
				if (!ResizeList (parser, number + payloads_size)) {
					return false;
				}
				if (!ReadCompressedPayload (parser, first, number, current_index)) {
					return false;
				}
				delete first;
			}
			
			ASF_DUMP ("-Payload #%i:\n", current_index + 1);
			asf_single_payload_dump (payloads [current_index]);
			
			current_index++;
		}
	} else {
		asf_single_payload* payload = new asf_single_payload ();
		if (!payload->FillInAll (parser, ecd, ppi, NULL)) {
			delete payload;
			return false;
		}
		
		if (payload->is_compressed ()) {
			gint32 counter = 0;
			
			counter = CountCompressedPayloads (parser, payload);
			if (counter <= 0) {
				return false;
			}
						
			if (!ResizeList (parser, counter)) {
				return false;
			}
			
			if (!ReadCompressedPayload (parser, payload, counter, 0)) {
				return false;
			}
			
			delete payload;
			
		} else {
			payloads = (asf_single_payload**) parser->MallocVerified (sizeof (asf_single_payload*) * 2);
			if (payloads == NULL) {
				return false;
			}		
			payloads [0] = payload;
			
			payload_flags = 1; //  1 payload
		}
	}
	return true;
}

void asf_multiple_payloads_dump (asf_multiple_payloads* obj)
{
	ASF_DUMP ("ASF_MULTIPLE_PAYLOADS\n");
	ASF_DUMP ("\tpayload_flags = %u\n", (asf_dword) obj->payload_flags);
	ASF_DUMP ("\t\tnumber of payloads = %i\n", obj->get_number_of_payloads ());
	ASF_DUMP ("\t\tpayload_length_type = %i\n", obj->get_payload_length_type ());
	
	if (obj->payloads) {
		int i = 0;
		while (obj->payloads [i] != NULL) {
			ASF_DUMP ("\tpayload #%i:\n", i + 1);
			asf_single_payload_dump (obj->payloads [i++]);
		}
	} else {
		ASF_DUMP ("\t<no payloads here>\n");
	}
}

asf_script_command_entry** 
asf_script_command::get_commands (ASFParser* parser, char*** command_types)
{
	//printf ("asf_script_command::get_commands ().\n");
	gint32 size_left = size;
	gint32 size_requested = 0;
	char** types = NULL;
	char* start = NULL;
	asf_script_command_entry** result = NULL;
	asf_script_command_entry* next = NULL;
	
	if (size == sizeof (asf_script_command)) {
		//printf ("asf_script_command::get_commands (), size = %i\n", size);
		return NULL;
	}
	
	size_left -= sizeof (asf_script_command);
	
	size_requested = sizeof (char*) * (command_count + 1);
	if (size_requested > size_left) {
		parser->AddError ("Data corruption in script command.");
		goto failure;
	}

	result = (asf_script_command_entry**) parser->MallocVerified (size_requested);
	if (result == NULL)
		goto failure;
	
	size_requested = sizeof (char*) * (command_type_count + 1);
	if (size_requested > size_left) {
		parser->AddError ("Data corruption in script command.");
		goto failure;
	}

	types = (char**) parser->MallocVerified (size_requested);
	if (types == NULL)
		goto failure;
	
	if (command_types != NULL) 
		*command_types = types;

	// Walk past by the command type table.
	start = (sizeof (asf_script_command) + (char*) this);
	for (gint32 i = 0; i < command_type_count; i++) {
		asf_word length = * (asf_word*) start;
		
		// Verify data
		size_requested = sizeof (asf_word) + sizeof (guint16) * length;
		if (size_requested > size_left) {
			parser->AddError ("Data corruption in script command.");
			goto failure;
		}
		size_left -= size_requested;
		
		// Convert strings
		types [i] = wchar_to_utf8 (start + sizeof (asf_word), length);
		start += size_requested;
	}
	
	// Fill in the commands table
	next = (asf_script_command_entry*) start;
	for (gint32 i = 0; i < command_count; i++) {
		result [i] = next;
		
		char* tmp = (char*) next;
		tmp += sizeof (asf_script_command_entry);
		tmp += (next->name_length * sizeof (guint16));
		
		size_requested = sizeof (asf_script_command_entry) + next->name_length * sizeof (guint16);
		if (size_requested > size_left) {
			parser->AddError ("Data corruption in script command.");
			goto failure;
		}
		size_left -= size_requested;
		
		next = (asf_script_command_entry*) tmp;
	}
	
	//printf ("asf_script_command::read_commands (): success, read %i commands and %i types.\n", command_count, command_type_count);
	return result;
	
failure:
	//printf ("asf_script_command::read_commands (): failure.\n");
	g_free (result);
	if (types != NULL) {
		int i = -1;
		while (types [++i] != NULL)
			g_free (types [i]);
		g_free (types);
	}
	
	if (command_types != NULL)
		*command_types = NULL;
		
	return NULL;
}


void asf_marker_entry_dump (const asf_marker_entry* obj)
{
	asf_marker_entry* o = (asf_marker_entry*) obj;
	
	ASF_DUMP ("\tASF_MARKER_ENTRY\n");
	ASF_DUMP ("\t\toffset = %llu\n", obj->offset);
	ASF_DUMP ("\t\tpts = %llu\n", obj->pts);
	ASF_DUMP ("\t\tentry_length = %i\n", (asf_dword) obj->entry_length);
	ASF_DUMP ("\t\tsend_time = %i\n", obj->send_time);
	ASF_DUMP ("\t\tflags = %i\n", obj->flags);
	ASF_DUMP ("\t\tmarker_description_length = %i\n", o->marker_description_length);
	ASF_DUMP ("\t\tmarker_description = %s\n", o->get_marker_description ());
}

void asf_marker_dump (const asf_marker* obj)
{
	asf_marker* o = (asf_marker*) obj;
	
	ASF_DUMP ("ASF_MARKER\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %llu\n", obj->size);
	ASF_DUMP ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	ASF_DUMP ("\tmarker_count = %u\n", (asf_dword) obj->marker_count);
	ASF_DUMP ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
	ASF_DUMP ("\tname_length = %u\n", (asf_dword) obj->name_length);
	ASF_DUMP ("\tname = %s\n", o->get_name ());
	
	for (guint32 i = 0; i < obj->marker_count; i++) {
		asf_marker_entry_dump (o->get_entry (i));
	}
}

void asf_script_command_dump (ASFParser* parser, const asf_script_command* obj)
{
	asf_script_command* o = (asf_script_command*) obj;
	
	ASF_DUMP ("ASF_SCRIPT_COMMAND\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %llu\n", obj->size);
	ASF_DUMP ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	ASF_DUMP ("\tcommand_count = %u\n", (asf_dword) obj->command_count);
	ASF_DUMP ("\tcommand_type_count = %u\n", (asf_dword) obj->command_type_count);
	
	char** command_types = NULL;
	asf_script_command_entry** entries = NULL;
	
	entries = o->get_commands (parser, &command_types);
	
	for (guint32 i = 0; i < obj->command_type_count; i++) {
		ASF_DUMP ("\tASF_SCRIPT_COMMAND_TYPE #%i\n", i);
		ASF_DUMP ("\t\tname = %s\n", command_types [i]);
	}
	
	for (guint32 i = 0; i < obj->command_count; i++) {
		ASF_DUMP ("\tASF_SCRIPT_COMMAND #%i\n", i);
		asf_script_command_entry* entry = entries [i];
		ASF_DUMP ("\t\tpts = %i\n", entry->pts);
		ASF_DUMP ("\t\ttype_index = %i\n", (asf_dword) entry->type_index);
		ASF_DUMP ("\t\tname_length = %i\n", (asf_dword) entry->name_length);
		ASF_DUMP ("\t\tname = %s\n", entry->get_name ());
	}
}

