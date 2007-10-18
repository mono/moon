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
		parser->AddError ("Data file id and header's file properties file id doesn't match.");
		return false;
	}
	
	// TODO: More verifications?
	return true;
}

bool
asf_error_correction_data::FillInAll (ASFSource* source)
{
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
	printf ("ASF_ERROR_CORRECTION_DATA\n");
	printf ("\tdata = 0x%X\n", (asf_dword) obj->data);
	printf ("\tdata = 0b%s\n",  obj->tostring ());
	printf ("\t\tis_error_correction_present: %i\n", obj->is_error_correction_present ());
	printf ("\t\tis_opaque_data_present: %i\n", obj->is_opaque_data_present ());
	printf ("\t\tdata_length: %i\n", obj->get_data_length ());
	printf ("\t\tlength_type: %i\n", obj->get_error_correction_length_type ());
	printf ("\tfirst = %X\n", (asf_dword) obj->first);
	printf ("\tsecond = %X\n", (asf_dword) obj->second);
}

bool
asf_payload_parsing_information::FillInAll (ASFSource* source)
{
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
	printf ("ASF_PAYLOAD_PARSING_INFORMATION\n");
	printf ("\tlength_type_flags = %X\n", (asf_dword) obj->length_type_flags);
	printf ("\t\tmultiple_payloads_present = %i\n", obj->is_multiple_payloads_present ());
	printf ("\t\tsequence_type = %i\n", obj->get_sequence_type ());
	printf ("\t\tpadding_length_type = %i\n", obj->get_padding_length_type ());
	printf ("\t\tpacket_length_type = %i\n", obj->get_packet_length_type ());
	printf ("\tproperty_flags = %X\n", (asf_dword) obj->property_flags);
	printf ("\t\treplicated_data_length_type = %i\n", obj->get_replicated_data_length_type ());
	printf ("\t\toffset_into_media_object_length_type = %i\n", obj->get_offset_into_media_object_length_type ());
	printf ("\t\tmedia_object_number_length_type = %i\n", obj->get_media_object_number_length_type ());
	printf ("\t\tstream_number_length_type = %i\n", obj->get_stream_number_length_type ());
	printf ("\tpacket_length = %u\n", (asf_dword) obj->packet_length);
	printf ("\tsequence = %u\n", (asf_dword) obj->sequence);
	printf ("\tpadding_length = %u\n", (asf_dword) obj->padding_length);
	printf ("\tsend_time = %u\n", (asf_dword) obj->send_time);
	printf ("\tduration = %u\n", (asf_dword) obj->duration);
}

bool
asf_single_payload::FillInAll (ASFSource* source, asf_error_correction_data* ecd, asf_payload_parsing_information ppi, asf_multiple_payloads* mp)
{	
	if (!source->Read (&stream_number, 1))
		return false;
	
	is_key_frame = stream_number & 0x80;
	stream_number = stream_number & 0x7F;
	
	if (!source->parser->IsValidStream (stream_number)) {
		printf ("asf_single_payload::FillInAll: Invalid stream number (%i).\n", (asf_dword) stream_number);
		return false;
	}
	
	media_object_number = 0;
	offset_into_media_object =  0;
	replicated_data_length =  0;
	replicated_data = NULL;
	payload_data_length =  0;
	payload_data = NULL;
	
	if (!source->ReadEncoded (ppi.get_media_object_number_length_type (), &media_object_number))
		return false;
		
	if (!source->ReadEncoded (ppi.get_offset_into_media_object_length_type (), &offset_into_media_object))
		return false;
		
	if (!source->ReadEncoded (ppi.get_replicated_data_length_type (), &replicated_data_length))
		return false;
	
	if (replicated_data_length == 1) {
		// compressed data
		source->parser->AddError ("Compressed payload not implemented.");
		return false;
	} else if (replicated_data_length >= 2 && replicated_data_length < 7) {
		source->parser->AddError (g_strdup_printf ("Invalid replicated data length: %i", replicated_data_length));
		return false;
	} else if (replicated_data_length >= 8) {
		replicated_data = (asf_byte*) g_malloc0 (replicated_data_length);
		if (!source->Read (replicated_data, replicated_data_length))
			return false;
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
		payload_length -= replicated_data_length; // TODO: when compressed?
		// minus the Padding Length.
		payload_length -= ppi.padding_length;
		printf ("payload_length: %i. packet_length: %i, ppi.get_struct_size: %i, replicated_data_length: %i, padding_length: %i, ecd.get_struct_size: %i\n",
			payload_length, ppi.packet_length, ppi.get_struct_size (), replicated_data_length, ppi.padding_length, ecd->get_struct_size ());
			
		if (payload_length < 0) {
			source->parser->AddError (g_strdup_printf ("Invalid payload length: %i", payload_length));
			return false;
		} 
		
		payload_data_length = (asf_dword) payload_length;
	}
	
	if (payload_data_length > 0) {
		payload_data = (asf_byte*) g_malloc0 (payload_data_length);
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
	printf ("ASF_SINGLE_PAYLOAD\n");
	printf ("\tstream_number = %u\n", (asf_dword) obj->stream_number);
	printf ("\tis_key_frame = %s\n", obj->is_key_frame ? "true" : "false");
	printf ("\tmedia_object_number = %u\n", (asf_dword) obj->media_object_number);
	printf ("\toffset_into_media_object = %u\n", (asf_dword) obj->offset_into_media_object);
	printf ("\treplicated_data_length = %u\n", (asf_dword) obj->replicated_data_length);
	printf ("\treplicated_data = %p\n", obj->replicated_data);
	printf ("\tpayload_data_length = %u\n", (asf_dword) obj->payload_data_length);
	printf ("\tpayload_data = %p\n", obj->payload_data);
	printf ("\tget_presentation_time = %i\n", obj->get_presentation_time ());
}

bool
asf_multiple_payloads::FillInAll (ASFSource* source, asf_error_correction_data* ecd, asf_payload_parsing_information ppi)
{
	int count;
	
	if (!source->Read (&payload_flags, 1))
		return false;
	
	count = get_number_of_payloads ();
	
	if (count <= 0) {
		source->parser->AddError (g_strdup_printf ("Invalid number of payloads: %i", count));
		return false;
	}

	payloads = (asf_single_payload**) g_malloc0 (sizeof (asf_single_payload*) * (count + 1));
	
	printf ("asf_multiple_payloads::FillInAll (): Reading %i payloads...\n", count); 
	
	for (int i = 0; i < count; i++) {
		payloads [i] = new asf_single_payload ();
		if (!payloads [i]->FillInAll (source, ecd, ppi, this))
			return false;
		//printf ("-Payload #%i:\n", i + 1);
		//asf_single_payload_dump (payloads [i]);
	}
	
	return true;
}

void asf_multiple_payloads_dump (asf_multiple_payloads* obj)
{
	printf ("ASF_MULTIPLE_PAYLOADS\n");
	printf ("\tpayload_flags = %u\n", (asf_dword) obj->payload_flags);
	printf ("\t\tnumber of payloads = %i\n", obj->get_number_of_payloads ());
	printf ("\t\tpayload_length_type = %i\n", obj->get_payload_length_type ());
	
	if (obj->payloads) {
		int i = 0;
		while (obj->payloads [i] != NULL) {
			printf ("\tpayload #%i:\n", i + 1);
			asf_single_payload_dump (obj->payloads [i++]);
		}
	} else {
		printf ("\t<no payloads here>\n");
	}
}



