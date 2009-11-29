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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 30, got %" G_GUINT64_FORMAT ").", header->size));
		return false;
	}
	
	// SPEC: This field must be set to the value 0x02. If the this value is different when read, the application should fail to source the content.
	if (header->reserved2 != 0x02) {
		parser->AddError (g_strdup_printf ("Invalid reserved2 value (expected 0x02, got: %x).", header->reserved2));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 104, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}

	// SPEC: the values for the Minimum Data Packet Size and Maximum Data Packet Size 
	// fields shall be set to the same value, and this value should be set to the packet size, 
	// even when the Broadcast Flag in the Flags field is set to 1.
	if (obj->min_packet_size != obj->max_packet_size) {
		// This is not logical at all, but it's what the spec says.
		// besides, our code depends on it (it makes a few minor things easier).
		parser->AddError (g_strdup_printf ("The min packet size (%d) is different from the max packet size (%d).", obj->min_packet_size, obj->max_packet_size));
		return false;
	}

	if (obj->size > parser->header->size) {
		parser->AddError (g_strdup_printf ("The size of the file property object (%" G_GUINT64_FORMAT ") is bigger than the sizeof the entire header itself (%" G_GUINT64_FORMAT ").", obj->size, parser->header->size)); 
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 78, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 46, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}
	
	if (obj->data_size < 24 && obj->data_size > 1) {
		parser->AddError (g_strdup_printf ("Invalid data_size (expected >= 24 or 0, got %u).", obj->data_size));
		return false;
	} else if (obj->data_size != 0 && obj->data_size + 46 != obj->size) {
		parser->AddError (g_strdup_printf ("Invalid data_size (expected size - 46, got %" G_GUINT64_FORMAT " - 46 = %u).", obj->size, obj->data_size)); 
		return false;
	}

	if (obj->data_size == 0)
		return true;

	guint64 max_size = obj->size;
	guint64 size = 46;
	guint64 length;
	guint64 accum_length = 0;
	void *data = obj->get_data ();
	asf_object *header_obj;

	do {
		if (size + 24 /* minimum object size */ > max_size) {
			parser->AddError (g_strdup_printf ("Invalid header extension size."));
			return false;
		}
		
		header_obj = (asf_object *) (((char *) data) + accum_length);
		length = header_obj->size;
		if (length == 0) {
			parser->AddError (g_strdup_printf ("Invalid header length is zero"));
			return false;
		}
		accum_length += length;
		size += length;
		if (size > max_size) {
			parser->AddError (g_strdup_printf ("Invalid header extension object."));
			return false;
		}
		
		if (!asf_object_validate_exact (header_obj, parser))
			return false;
		
	} while (size < max_size);
	
	return true;
}

bool asf_codec_list_validate (const asf_codec_list* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_codec_list, parser))) {
		return false;
	}
	
	if (obj->size < 44) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 48, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 42, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 44, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 34, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 26, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 26, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}
	
	if (!(asf_guid_compare (&obj->file_id, &parser->GetFileProperties ()->file_id))) {
		parser->AddError ("Data file id and header's file properties file id don't match.");
		return false;
	}
	
	// TODO: More verifications?
	return true;
}

MediaResult
asf_error_correction_data::FillInAll (ASFContext *context)
{
	IMediaSource *source = context->source;
	data = 0;
	first = 0;
	second = 0;
		
	if (!source->ReadAll (&data, 1)) {
		ASF_LOG_ERROR ("asf_error_correction_data::FillInAll (): Error while reading 'data'.\n");
		return MEDIA_READ_ERROR;
	}
		
	if (!is_error_correction_present ())
		return MEDIA_SUCCESS;
		
	if (!source->ReadAll (&first, 1)) {
		ASF_LOG_ERROR ("asf_error_correction_data::FillInAll (): Error while reading 'first'.\n");
		return MEDIA_READ_ERROR;
	}
	
	if (!source->ReadAll (&second, 1)) {
		ASF_LOG_ERROR ("asf_error_correction_data::FillInAll (): Error while reading 'second'.\n");
		return MEDIA_READ_ERROR;
	}
		
	return MEDIA_SUCCESS;
}

void 
asf_error_correction_data_dump (asf_error_correction_data* obj)
{
	char* tostring = obj->tostring ();
	ASF_DUMP ("ASF_ERROR_CORRECTION_DATA\n");
	ASF_DUMP ("\tdata = 0x%X\n", (asf_dword) obj->data);
	ASF_DUMP ("\tdata = 0b%s\n",  tostring); g_free (tostring);
	ASF_DUMP ("\t\tis_error_correction_present: %d\n", obj->is_error_correction_present ());
	ASF_DUMP ("\t\tis_opaque_data_present: %d\n", obj->is_opaque_data_present ());
	ASF_DUMP ("\t\tdata_length: %d\n", obj->get_data_length ());
	ASF_DUMP ("\t\tlength_type: %d\n", obj->get_error_correction_length_type ());
	ASF_DUMP ("\tfirst = %X\n", (asf_dword) obj->first);
	
	ASF_DUMP ("\tsecond = %X\n", (asf_dword) obj->second);
}

MediaResult
asf_payload_parsing_information::FillInAll (ASFContext *context)
{
	ASFParser *parser = context->parser;
	IMediaSource *source = context->source;
	// There's no guarantee that these fields will be written to by Read ()
	
	packet_length = 0;
	sequence = 0;
	padding_length = 0;
	send_time = 0;
	duration = 0;
	
	if (!source->ReadAll (&length_type_flags, 1)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'length_type_flags'.\n");
		return MEDIA_READ_ERROR;
	}

	if (!source->ReadAll (&property_flags, 1)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'property_flags'.\n");
		return MEDIA_READ_ERROR;
	}
		
	if (get_packet_length_type () == 0) {
		packet_length = parser->GetPacketSize ();
	} else {
		if (!ASFParser::ReadEncoded (source, get_packet_length_type (), &packet_length))  {
			ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'packet_length'.\n");
			return MEDIA_READ_ERROR;
		}
	}

	if (!ASFParser::ReadEncoded (source, get_sequence_type (), &sequence)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'sequence'.\n");
		return MEDIA_READ_ERROR;
	}

	if (!ASFParser::ReadEncoded (source, get_padding_length_type (), &padding_length)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'padding_length'.\n");
		return MEDIA_READ_ERROR;
	}

	if (!source->ReadAll (&send_time, 4)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'send_time'.\n");
		return MEDIA_READ_ERROR;
	}

	if (!source->ReadAll (&duration, 2)) {
		ASF_LOG_ERROR ("asf_payload_parsing_information::FillInAll (): Error while reading 'duration'.\n");
		return MEDIA_READ_ERROR;
	}
		
	return MEDIA_SUCCESS;
}

void asf_payload_parsing_information_dump (asf_payload_parsing_information* obj)
{
	ASF_DUMP ("ASF_PAYLOAD_PARSING_INFORMATION\n");
	ASF_DUMP ("\tlength_type_flags = %X\n", (asf_dword) obj->length_type_flags);
	ASF_DUMP ("\t\tmultiple_payloads_present = %d\n", obj->is_multiple_payloads_present ());
	ASF_DUMP ("\t\tsequence_type = %d\n", obj->get_sequence_type ());
	ASF_DUMP ("\t\tpadding_length_type = %d\n", obj->get_padding_length_type ());
	ASF_DUMP ("\t\tpacket_length_type = %d\n", obj->get_packet_length_type ());
	ASF_DUMP ("\tproperty_flags = %X\n", (asf_dword) obj->property_flags);
	ASF_DUMP ("\t\treplicated_data_length_type = %d\n", obj->get_replicated_data_length_type ());
	ASF_DUMP ("\t\toffset_into_media_object_length_type = %d\n", obj->get_offset_into_media_object_length_type ());
	ASF_DUMP ("\t\tmedia_object_number_length_type = %d\n", obj->get_media_object_number_length_type ());
	ASF_DUMP ("\t\tstream_number_length_type = %d\n", obj->get_stream_number_length_type ());
	ASF_DUMP ("\tpacket_length = %u\n", (asf_dword) obj->packet_length);
	ASF_DUMP ("\tsequence = %u\n", (asf_dword) obj->sequence);
	ASF_DUMP ("\tpadding_length = %u\n", (asf_dword) obj->padding_length);
	ASF_DUMP ("\tsend_time = %u\n", (asf_dword) obj->send_time);
	ASF_DUMP ("\tduration = %u\n", (asf_dword) obj->duration);
}

asf_single_payload *
asf_single_payload::Clone ()
{
	asf_single_payload *result = new asf_single_payload ();
	
	result->stream_id = stream_id;
	result->is_key_frame = is_key_frame;
	result->media_object_number = media_object_number;
	result->offset_into_media_object = offset_into_media_object;
	result->replicated_data_length = replicated_data_length;
	if (replicated_data != NULL) {
		result->replicated_data = (asf_byte *) g_malloc (replicated_data_length);
		memcpy (result->replicated_data, replicated_data, replicated_data_length);
	}
	result->payload_data_length = payload_data_length;
	if (payload_data != NULL) {
		result->payload_data = (asf_byte *) g_malloc (payload_data_length);
		memcpy (result->payload_data, payload_data, payload_data_length);
	}
	result->presentation_time = presentation_time;
	
	return result;
}

MediaResult
asf_single_payload::FillInAll (ASFContext *context, asf_error_correction_data* ecd, asf_payload_parsing_information ppi, asf_multiple_payloads* mp)
{	
	ASFParser *parser = context->parser;
	IMediaSource *source = context->source;
	
	if (!source->ReadAll (&stream_id, 1)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'stream_id'.\n");
		return MEDIA_READ_ERROR;
	}
	
	is_key_frame = stream_id & 0x80;
	stream_id = stream_id & 0x7F;
	
	if (!parser->IsValidStream (stream_id)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll: Invalid stream number (%d).", (int) stream_id);
		return MEDIA_INVALID_DATA;
	}
	
	media_object_number = 0;
	offset_into_media_object =  0;
	replicated_data_length =  0;
	replicated_data = NULL;
	payload_data_length =  0;
	payload_data = NULL;
	presentation_time = 0;
	
	ASF_LOG ("asf_single_payload::FillInAll (%p, %p, [Length type flags: %i, property flags: %i (mon:%i,oimo:%i,rpl:%i,sl:%i]). Stream: %i\n", context, ecd, ppi.length_type_flags, ppi.property_flags, ppi.get_offset_into_media_object_length_type (), ppi.get_media_object_number_length_type (), ppi.get_replicated_data_length_type (), ppi.get_stream_number_length_type (), stream_id);
	
	if (!ASFParser::ReadEncoded (source, ppi.get_media_object_number_length_type (), &media_object_number)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'media_object_number'.\n");
		return MEDIA_READ_ERROR;
	}
		
	if (!ASFParser::ReadEncoded (source, ppi.get_offset_into_media_object_length_type (), &offset_into_media_object)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'offset_into_media_object'.\n");
		return MEDIA_READ_ERROR;
	}
		
	if (!ASFParser::ReadEncoded (source, ppi.get_replicated_data_length_type (), &replicated_data_length)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'replicated_data_length'.\n");
		return MEDIA_READ_ERROR;
	}
	
	if (replicated_data_length >= 2 && replicated_data_length < 7) {
		parser->AddError (g_strdup_printf ("Invalid replicated data length: %d", replicated_data_length));
		return MEDIA_INVALID_DATA;
	} 
		
	if (replicated_data_length > parser->file_properties->max_packet_size) {
		parser->AddError ("Data corruption in payload.");
		return MEDIA_INVALID_DATA;
	}
	
	replicated_data = (asf_byte*) parser->MallocVerified (replicated_data_length);
	if (replicated_data == NULL) {
		return MEDIA_OUT_OF_MEMORY;
	}
	
	if (!source->ReadAll (replicated_data, replicated_data_length)) {
		ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'replicated_data'.\n");
		return MEDIA_READ_ERROR;
	}

	if (replicated_data_length == 1) {
		presentation_time = offset_into_media_object;
	} else if (replicated_data_length >= 8) {
		presentation_time = *(((asf_dword*) replicated_data) + 1);
	}

	if (mp != NULL) {
		if (!ASFParser::ReadEncoded (source, mp->get_payload_length_type (), &payload_data_length)) {
			ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'payload_data_length'.\n");
			return MEDIA_READ_ERROR;
		}
			
		if (payload_data_length == 0) {
			parser->AddError ("Warning: Invalid payload data length: can't be 0.");
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
		ASF_LOG ("payload_length: %d. packet_length: %d, ppi.get_struct_size: %d, replicated_data_length: %d, padding_length: %d, ecd.get_struct_size: %d\n",
			payload_length, ppi.packet_length, ppi.get_struct_size (), replicated_data_length, ppi.padding_length, ecd->get_struct_size ());
			
		if (payload_length < 0) {
			parser->AddError (g_strdup_printf ("Invalid payload length: %d", payload_length));
			return MEDIA_INVALID_DATA;
		} 
		
		payload_data_length = (asf_dword) payload_length;
	}
	
	if (payload_data_length > 0) {
		if (payload_data_length >= parser->file_properties->max_packet_size) {
			parser->AddError ("Data corruption in payload.");
			return MEDIA_INVALID_DATA;
		}
		
		payload_data = (asf_byte*) parser->MallocVerified (payload_data_length);
		if (payload_data == NULL) {
			return MEDIA_OUT_OF_MEMORY;
		}
		
		if (!source->ReadAll (payload_data, payload_data_length)) {
			ASF_LOG_ERROR ("asf_single_payload::FillInAll (): Error while reading 'payload_data'.\n");
			return MEDIA_READ_ERROR;
		}
	}


	return MEDIA_SUCCESS;
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
	ASF_DUMP ("\tstream_number = %u\n", (asf_dword) obj->stream_id);
	ASF_DUMP ("\tis_key_frame = %s\n", obj->is_key_frame ? "true" : "false");
	ASF_DUMP ("\tmedia_object_number = %u\n", (asf_dword) obj->media_object_number);
	ASF_DUMP ("\toffset_into_media_object = %u\n", (asf_dword) obj->offset_into_media_object);
	ASF_DUMP ("\treplicated_data_length = %u\n", (asf_dword) obj->replicated_data_length);
	ASF_DUMP ("\treplicated_data = %s\n", obj->replicated_data ? "non-null" : "null");
	ASF_DUMP ("\tpayload_data_length = %u\n", (asf_dword) obj->payload_data_length);
	ASF_DUMP ("\tpayload_data = %s\n", obj->payload_data ? "non-null" : "null");
	ASF_DUMP ("\tget_presentation_time = %" G_GINT64_FORMAT "\n", obj->get_presentation_time ());
}

bool
asf_multiple_payloads::ResizeList (ASFParser* parser, int requested_size)
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

int 
asf_multiple_payloads::CountCompressedPayloads (ASFParser* parser, asf_single_payload* payload)
{
	asf_byte* data = payload->payload_data;
	asf_dword length = payload->payload_data_length;
	asf_byte size = 0;
	guint32 offset = 0;
	int counter = 0;
	
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

MediaResult
asf_multiple_payloads::ReadCompressedPayload (ASFParser* parser, asf_single_payload* first, int count, int start_index)
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
		
		payload->stream_id = first->stream_id;
		payload->is_key_frame = first->is_key_frame;
		payload->media_object_number = first->media_object_number + i;
		payload->offset_into_media_object = 0;
		payload->replicated_data_length = 0;
		payload->replicated_data = NULL;
		payload->presentation_time = first->presentation_time + i * first->get_presentation_time_delta ();
		payload->payload_data_length = size;
		payload->payload_data = (asf_byte*) parser->Malloc (size);
		if (payload->payload_data == NULL) {
			return MEDIA_OUT_OF_MEMORY;
		}
		memcpy (payload->payload_data, data + offset, size);
		offset += size;
	}
	
	return MEDIA_SUCCESS;
}

MediaResult
asf_multiple_payloads::FillInAll (ASFContext *context, asf_error_correction_data* ecd, asf_payload_parsing_information ppi)
{
	ASFParser *parser = context->parser;
	IMediaSource *source = context->source;
	MediaResult result;
	int count;
	
	if (ppi.is_multiple_payloads_present ()) {		
		if (!source->ReadAll (&payload_flags, 1)) {
			ASF_LOG_ERROR ("asf_multiple_payload::FillInAll (): Error while reading 'payload_flags'.\n");
			return MEDIA_READ_ERROR;
		}			
		
		count = payload_flags & 0x3F; // number of payloads is encoded in a byte, no need to check for extreme values.
		
		if (count <= 0) {
			parser->AddError (g_strdup_printf ("Invalid number of payloads: %d", count));
			return MEDIA_INVALID_DATA;
		}

		if (!ResizeList (parser, count)) {
			return MEDIA_OUT_OF_MEMORY;
		}
		
		ASF_LOG ("asf_multiple_payloads::FillInAll (): Reading %d payloads...\n", count); 
		
		int current_index = 0;
		for (int i = 0; i < count; i++) {
			payloads [current_index] = new asf_single_payload ();
			
			result = payloads [current_index]->FillInAll (context, ecd, ppi, this);
			if (!MEDIA_SUCCEEDED (result)) {
				delete payloads [current_index];
				payloads [current_index] = NULL;
				return result;
			}
			
			if (payloads [current_index]->is_compressed ()) {
				asf_single_payload* first = payloads [current_index];
				int number = CountCompressedPayloads (parser, first);
				if (number <= 0) {
					return MEDIA_INVALID_DATA;
				}
				if (!ResizeList (parser, number + payloads_size)) {
					return MEDIA_OUT_OF_MEMORY;
				}
				result = ReadCompressedPayload (parser, first, number, current_index);
				if (!MEDIA_SUCCEEDED (result)) {
					return result;
				}
				delete first;
			}
			
			ASF_DUMP ("-Payload #%d:\n", current_index + 1);
			asf_single_payload_dump (payloads [current_index]);
			
			current_index++;
		}
	} else {
		ASF_LOG ("asf_multiple_payloads::FillInAll (%p, %p, ?): A single payload\n", context, ecd);
		
		asf_single_payload* payload = new asf_single_payload ();
		result = payload->FillInAll (context, ecd, ppi, NULL);
		if (!MEDIA_SUCCEEDED (result)) {
			delete payload;
			return result;
		}
		
		if (payload->is_compressed ()) {
			int counter = 0;
			
			counter = CountCompressedPayloads (parser, payload);
			if (counter <= 0) {
				return MEDIA_INVALID_DATA;
			}
						
			if (!ResizeList (parser, counter)) {
				return MEDIA_OUT_OF_MEMORY;
			}
			
			result = ReadCompressedPayload (parser, payload, counter, 0);
			if (!MEDIA_SUCCEEDED (result)) {
				return result;
			}
			
			delete payload;
			
		} else {
			payloads = (asf_single_payload**) parser->MallocVerified (sizeof (asf_single_payload*) * 2);
			if (payloads == NULL) {
				return MEDIA_OUT_OF_MEMORY;
			}		
			payloads [0] = payload;
			
			payload_flags = 1; //  1 payload
		}
	}
	return MEDIA_SUCCESS;
}

void asf_multiple_payloads_dump (asf_multiple_payloads* obj)
{
	ASF_DUMP ("ASF_MULTIPLE_PAYLOADS\n");
	ASF_DUMP ("\tpayload_flags = %u\n", (asf_dword) obj->payload_flags);
	ASF_DUMP ("\t\tnumber of payloads = %d\n", obj->get_number_of_payloads ());
	ASF_DUMP ("\t\tpayload_length_type = %d\n", obj->get_payload_length_type ());
	
	if (obj->payloads) {
		int i = 0;
		while (obj->payloads [i] != NULL) {
			ASF_DUMP ("\tpayload #%d:\n", i + 1);
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
	int size_left = size;
	int size_requested = 0;
	char** types = NULL;
	char* start = NULL;
	asf_script_command_entry** result = NULL;
	asf_script_command_entry* next = NULL;
	
	if (size == sizeof (asf_script_command)) {
		//printf ("asf_script_command::get_commands (), size = %d\n", size);
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
	for (int i = 0; i < command_type_count; i++) {
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
	for (int i = 0; i < command_count; i++) {
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
	
	//printf ("asf_script_command::read_commands (): success, read %d commands and %d types.\n", command_count, command_type_count);
	return result;
	
failure:
	//printf ("asf_script_command::read_commands (): failure.\n");
	g_free (result);
	if (types != NULL) {
		for (int i = 0; types[i]; i++)
			g_free (types [i]);
		g_free (types);
	}
	
	if (command_types != NULL)
		*command_types = NULL;
		
	return NULL;
}


void asf_marker_entry_dump (const asf_marker_entry* obj)
{
#ifdef ASF_DUMPING
	asf_marker_entry* o = (asf_marker_entry*) obj;
	ASF_DUMP ("\tASF_MARKER_ENTRY\n");
	ASF_DUMP ("\t\toffset = %" G_GUINT64_FORMAT "\n", obj->offset);
	ASF_DUMP ("\t\tpts = %" G_GUINT64_FORMAT "\n", obj->pts);
	ASF_DUMP ("\t\tentry_length = %d\n", (asf_dword) obj->entry_length);
	ASF_DUMP ("\t\tsend_time = %d\n", obj->send_time);
	ASF_DUMP ("\t\tflags = %d\n", obj->flags);
	ASF_DUMP ("\t\tmarker_description_length = %d\n", o->marker_description_length);
	ASF_DUMP ("\t\tmarker_description = %s\n", o->get_marker_description ());
#endif
}

void asf_marker_dump (const asf_marker* obj)
{
	asf_marker* o = (asf_marker*) obj;
	
	ASF_DUMP ("ASF_MARKER\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
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
#ifdef ASF_DUMPING
	asf_script_command* o = (asf_script_command*) obj;
	guint32 i;
	
	ASF_DUMP ("ASF_SCRIPT_COMMAND\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	ASF_DUMP ("\tcommand_count = %u\n", (asf_dword) obj->command_count);
	ASF_DUMP ("\tcommand_type_count = %u\n", (asf_dword) obj->command_type_count);
	
	char** command_types = NULL;
	asf_script_command_entry** entries = NULL;
	
	entries = o->get_commands (parser, &command_types);
	
	for (i = 0; i < obj->command_type_count; i++) {
		ASF_DUMP ("\tASF_SCRIPT_COMMAND_TYPE #%d\n", i);
		ASF_DUMP ("\t\tname = %s\n", command_types [i]);
	}
	
	for (i = 0; i < obj->command_count; i++) {
		ASF_DUMP ("\tASF_SCRIPT_COMMAND #%u\n", i);
		asf_script_command_entry* entry = entries [i];
		ASF_DUMP ("\t\tpts = %u\n", entry->pts);
		ASF_DUMP ("\t\ttype_index = %d\n", (asf_dword) entry->type_index);
		ASF_DUMP ("\t\tname_length = %d\n", (asf_dword) entry->name_length);
		ASF_DUMP ("\t\tname = %s\n", entry->get_name ());
	}
#endif
}

void asf_header_extension_dump (const asf_header_extension* obj)
{
	ASF_DUMP ("ASF_HEADER_EXTENSION\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\treserved1 = %s\n", asf_guid_tostring (&obj->reserved1));
	ASF_DUMP ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
	ASF_DUMP ("\tdata_size = %u\n", (asf_dword) obj->data_size);
	
	asf_dword count = obj->get_object_count ();
	asf_object** objects = obj->get_objects ();
	
	for (asf_dword i = 0; i < count; i++) {
		ASF_DUMP ("\n\textended object #%d:\n", i);
		asf_object_dump_exact (objects [i]);
	}
	ASF_DUMP ("\n\n");
	g_free (objects);
}

bool asf_extended_stream_properties_validate (const asf_extended_stream_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_extended_stream_properties, parser))) {
		return false;
	}
	
	if (obj->size < 88) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 112, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}

	if (obj->data_bitrate == 0) {
		parser->AddError (g_strdup_printf ("Invalid bitrate (expected != 0)."));
		return false;
	}

	if (obj->initial_buffer_fullness > obj->buffer_size) {
		parser->AddError (g_strdup_printf ("Invalid initial buffer fullness (expected <= buffer size (%i), got %i).", obj->buffer_size, obj->initial_buffer_fullness));
		return false;
	}

	if (obj->alternate_initial_buffer_fullness > obj->alternate_buffer_size) {
		parser->AddError (g_strdup_printf ("Invalid alternate initial buffer fullness (expected <= alternate buffer size (%i), got %i).", obj->alternate_buffer_size, obj->alternate_initial_buffer_fullness));
		return false;
	}

	if (obj->stream_id == 0 || obj->stream_id > 127) {
		parser->AddError (g_strdup_printf ("Invalid stream number, must be 0 < stream number <= 127, got %i.", obj->stream_id	));
		return false;
	}

	guint64 max_size = obj->size;
	guint64 stream_names_length = 0; // accumulated length of stream names
	guint64 payload_ex_sys_length = 0; // accumulated length of payload extension systems
	guint64 size = 88; // size of all the fixed fields

	for (gint32 i = 0; i < obj->stream_name_count; i++) {
		if (size + 4 > max_size) { // 4 = minimum size of stream name
			parser->AddError (g_strdup_printf ("Invalid stream name count."));
			return false;
		}
		gint16 length = 4 + *(gint16 *) (((char *) obj) + 88 + stream_names_length + 2 /* offset into length */);
		size += length;
		stream_names_length += length;
		if (size > max_size) {
			parser->AddError (g_strdup_printf ("Invalid stream name."));
			return false;
		}
	}

	for (gint32 i = 0; i < obj->payload_extension_system_count; i++) {
		if (size + 22 > max_size) { // 22 = minimum size of payload extension system
			parser->AddError (g_strdup_printf ("Invalid payload extension system count."));
			return false;
		}
		guint32 length = 22 + *(guint32 *) (((char *) obj) + 88 + stream_names_length + payload_ex_sys_length + 18 /* offset into length */);
		if (length > max_size) { // Sanity check length before doing algorithm with it to avoid overflows.
			parser->AddError (g_strdup_printf ("Invalid payload extension system."));
			return false;
		}
		size += length;
		payload_ex_sys_length += length;
		if (size > max_size) {
			parser->AddError (g_strdup_printf ("Invalid payload extension system."));
			return false;
		}
	}

	return true;
}

void asf_extended_stream_properties_dump (const asf_extended_stream_properties* obj)
{
#ifdef ASF_DUMPING
	ASF_DUMP ("ASF_EXTENDED_STREAM_PROPERTIES\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tstart_time = %" G_GUINT64_FORMAT "\n", obj->start_time);
	ASF_DUMP ("\tend_time = %" G_GUINT64_FORMAT "\n", obj->end_time);
	ASF_DUMP ("\tdata_bitrate = %u\n", (asf_dword) obj->data_bitrate);
	ASF_DUMP ("\tbuffer_size = %u\n", (asf_dword) obj->buffer_size);
	ASF_DUMP ("\tinitial_buffer_fullness = %u\n", (asf_dword) obj->initial_buffer_fullness);
	ASF_DUMP ("\talternate_data_bitrate = %u\n", (asf_dword) obj->alternate_data_bitrate);
	ASF_DUMP ("\talternate_buffer_size = %u\n", (asf_dword) obj->alternate_buffer_size);
	ASF_DUMP ("\talternate_initial_buffer_fullness = %u\n", (asf_dword) obj->alternate_initial_buffer_fullness);
	ASF_DUMP ("\tmaximum_object_size = %u\n", (asf_dword) obj->maximum_object_size);
	ASF_DUMP ("\tflags = %u\n", (asf_dword) obj->flags);
	ASF_DUMP ("\tstream_id = %u\n", (asf_dword) obj->stream_id);
	ASF_DUMP ("\tstream_language_id_index = %u\n", (asf_dword) obj->stream_language_id_index);
	ASF_DUMP ("\taverage_time_per_frame = %" G_GUINT64_FORMAT "\n", obj->average_time_per_frame);
	ASF_DUMP ("\tstream_name_count = %u\n", (asf_dword) obj->stream_name_count);
	ASF_DUMP ("\tpayload_extension_system_count = %u\n", (asf_dword) obj->payload_extension_system_count);

	asf_extended_stream_name **names = obj->get_stream_names ();
	ASF_DUMP ("\tstream_names: %p\n", names);
	if (names != NULL) {
		for (int i = 0; names [i] != NULL; i++)
			asf_extended_stream_name_dump (names [i]);
	}
	g_free (names);

	asf_payload_extension_system **systems = obj->get_payload_extension_systems ();
	ASF_DUMP ("\tpayload_extension_systems: %p\n", systems);
	if (systems != NULL) {
		for (int i = 0; systems [i] != NULL; i++)
			asf_payload_extension_system_dump (systems [i]);
	}
	g_free (systems);
	
	const asf_stream_properties *asp = obj->get_stream_properties ();
	ASF_DUMP ("\tasf_stream_properties = %p\n", asp);
	if (asp != NULL)
		asf_stream_properties_dump (asp);
#endif
}

void asf_extended_stream_name_dump (const asf_extended_stream_name* obj)
{
	ASF_DUMP ("ASF_EXTENDED_STREAM_NAME\n");
	ASF_DUMP ("\tlanguage_id_index = %i\n", (int) obj->language_id_index);
	ASF_DUMP ("\tstream_name_length = %i\n", (int) obj->stream_name_length);
	ASF_DUMP ("\tstream_name = %s\n", obj->get_stream_name ());
}

void asf_payload_extension_system_dump (const asf_payload_extension_system* obj)
{
	ASF_DUMP ("ASF_PAYLOAD_EXTENSION_SYSTEM\n");
	ASF_DUMP ("\textension_system_id = %s\n", asf_guid_tostring (&obj->extension_system_id));
	ASF_DUMP ("\textension_data_size = %i\n", (int) obj->extension_data_size);
}
