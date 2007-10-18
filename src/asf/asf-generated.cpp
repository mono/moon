/*
 * asf-generated.cpp: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "asf.h"

void print_sizes () {
	printf ("sizeof (asf_header) = %i.\n", sizeof (asf_header));
	printf ("sizeof (asf_file_properties) = %i.\n", sizeof (asf_file_properties));
	printf ("sizeof (asf_stream_properties) = %i.\n", sizeof (asf_stream_properties));
	printf ("sizeof (asf_header_extension) = %i.\n", sizeof (asf_header_extension));
	printf ("sizeof (asf_codec_list) = %i.\n", sizeof (asf_codec_list));
	printf ("sizeof (asf_script_command) = %i.\n", sizeof (asf_script_command));
	printf ("sizeof (asf_marker) = %i.\n", sizeof (asf_marker));
	printf ("sizeof (asf_bitrate_mutual_exclusion) = %i.\n", sizeof (asf_bitrate_mutual_exclusion));
	printf ("sizeof (asf_error_correction) = %i.\n", sizeof (asf_error_correction));
	printf ("sizeof (asf_content_description) = %i.\n", sizeof (asf_content_description));
	printf ("sizeof (asf_extended_content_description) = %i.\n", sizeof (asf_extended_content_description));
	printf ("sizeof (asf_stream_bitrate_properties) = %i.\n", sizeof (asf_stream_bitrate_properties));
	printf ("sizeof (asf_data) = %i.\n", sizeof (asf_data));
	printf ("sizeof (asf_object) = %i.\n", sizeof (asf_object));
}

bool asf_object_validate_exact (const asf_object* obj, ASFParser* parser)
{
	switch (asf_get_guid_type (&obj->id)) {
	case ASF_HEADER:
		return asf_header_validate ((asf_header*) obj, parser);
	case ASF_FILE_PROPERTIES:
		return asf_file_properties_validate ((asf_file_properties*) obj, parser);
	case ASF_STREAM_PROPERTIES:
		return asf_stream_properties_validate ((asf_stream_properties*) obj, parser);
	case ASF_HEADER_EXTENSION:
		return asf_header_extension_validate ((asf_header_extension*) obj, parser);
	case ASF_CODEC_LIST:
		return asf_codec_list_validate ((asf_codec_list*) obj, parser);
	case ASF_SCRIPT_COMMAND:
		return asf_script_command_validate ((asf_script_command*) obj, parser);
	case ASF_MARKER:
		return asf_marker_validate ((asf_marker*) obj, parser);
	case ASF_BITRATE_MUTUAL_EXCLUSION:
		return asf_bitrate_mutual_exclusion_validate ((asf_bitrate_mutual_exclusion*) obj, parser);
	case ASF_ERROR_CORRECTION:
		return asf_error_correction_validate ((asf_error_correction*) obj, parser);
	case ASF_CONTENT_DESCRIPTION:
		return asf_content_description_validate ((asf_content_description*) obj, parser);
	case ASF_EXTENDED_CONTENT_DESCRIPTION:
		return asf_extended_content_description_validate ((asf_extended_content_description*) obj, parser);
	case ASF_STREAM_BITRATE_PROPERTIES:
		return asf_stream_bitrate_properties_validate ((asf_stream_bitrate_properties*) obj, parser);
	case ASF_DATA:
		return asf_data_validate ((asf_data*) obj, parser);
	default:
		parser->AddError (g_strdup_printf ("No validation implemented for %s.\n", asf_guid_get_name (&obj->id)));
		return false;
	}
}

/* Debug functions */ 
void asf_header_dump (const asf_header* obj)
{
	printf ("ASF_HEADER\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tobject_count = %u\n", (asf_dword) obj->object_count);
	printf ("\treserved1 = %u\n", (asf_dword) obj->reserved1);
	printf ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
}

void asf_file_properties_dump (const asf_file_properties* obj)
{
	printf ("ASF_FILE_PROPERTIES\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tfile_id = %s\n", asf_guid_tostring (&obj->file_id));
	printf ("\tfile_size = %llu\n", obj->file_size);
	printf ("\tcreation_date = %llu\n", obj->creation_date);
	printf ("\tdata_packet_count = %llu\n", obj->data_packet_count);
	printf ("\tplay_duration = %llu\n", obj->play_duration);
	printf ("\tsend_duration = %llu\n", obj->send_duration);
	printf ("\tpreroll = %llu\n", obj->preroll);
	printf ("\tflags = %u\n", (asf_dword) obj->flags);
	printf ("\tmin_packet_size = %u\n", (asf_dword) obj->min_packet_size);
	printf ("\tmax_packet_size = %u\n", (asf_dword) obj->max_packet_size);
	printf ("\tmax_bitrate = %u\n", (asf_dword) obj->max_bitrate);
}

void asf_stream_properties_dump (const asf_stream_properties* obj)
{
	printf ("ASF_STREAM_PROPERTIES\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tstream_type = %s\n", asf_guid_tostring (&obj->stream_type));
	printf ("\terror_correction_type = %s\n", asf_guid_tostring (&obj->error_correction_type));
	printf ("\ttime_offset = %llu\n", obj->time_offset);
	printf ("\ttype_specific_data_length = %u\n", (asf_dword) obj->type_specific_data_length);
	printf ("\terror_correction_data_length = %u\n", (asf_dword) obj->error_correction_data_length);
	printf ("\tflags = %u\n", (asf_dword) obj->flags);
	printf ("\treserved = %u\n", (asf_dword) obj->reserved);
}

void asf_header_extension_dump (const asf_header_extension* obj)
{
	printf ("ASF_HEADER_EXTENSION\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\treserved1 = %s\n", asf_guid_tostring (&obj->reserved1));
	printf ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
	printf ("\tdata_size = %u\n", (asf_dword) obj->data_size);
}

void asf_codec_list_dump (const asf_codec_list* obj)
{
	printf ("ASF_CODEC_LIST\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	printf ("\tentries_count = %u\n", (asf_dword) obj->entries_count);
}

void asf_script_command_dump (const asf_script_command* obj)
{
	printf ("ASF_SCRIPT_COMMAND\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	printf ("\tcommand_count = %u\n", (asf_dword) obj->command_count);
	printf ("\tcommand_type_count = %u\n", (asf_dword) obj->command_type_count);
}

void asf_marker_dump (const asf_marker* obj)
{
	printf ("ASF_MARKER\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	printf ("\tmarker_count = %u\n", (asf_dword) obj->marker_count);
	printf ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
	printf ("\tname_length = %u\n", (asf_dword) obj->name_length);
}

void asf_bitrate_mutual_exclusion_dump (const asf_bitrate_mutual_exclusion* obj)
{
	printf ("ASF_BITRATE_MUTUAL_EXCLUSION\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\texclusion_type = %s\n", asf_guid_tostring (&obj->exclusion_type));
	printf ("\tstream_number_count = %u\n", (asf_dword) obj->stream_number_count);
}

void asf_error_correction_dump (const asf_error_correction* obj)
{
	printf ("ASF_ERROR_CORRECTION\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\terror_correction_type = %s\n", asf_guid_tostring (&obj->error_correction_type));
	printf ("\terror_correction_data_length = %u\n", (asf_dword) obj->error_correction_data_length);
}

void asf_content_description_dump (const asf_content_description* obj)
{
	printf ("ASF_CONTENT_DESCRIPTION\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\ttitle_length = %u\n", (asf_dword) obj->title_length);
	printf ("\tauthor_length = %u\n", (asf_dword) obj->author_length);
	printf ("\tcopyright_length = %u\n", (asf_dword) obj->copyright_length);
	printf ("\tdescription_length = %u\n", (asf_dword) obj->description_length);
	printf ("\trating_length = %u\n", (asf_dword) obj->rating_length);
}

void asf_extended_content_description_dump (const asf_extended_content_description* obj)
{
	printf ("ASF_EXTENDED_CONTENT_DESCRIPTION\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tcontent_descriptors_count = %u\n", (asf_dword) obj->content_descriptors_count);
}

void asf_stream_bitrate_properties_dump (const asf_stream_bitrate_properties* obj)
{
	printf ("ASF_STREAM_BITRATE_PROPERTIES\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tbitrate_records_count = %u\n", (asf_dword) obj->bitrate_records_count);
}

void asf_data_dump (const asf_data* obj)
{
	printf ("ASF_DATA\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
	printf ("\tfile_id = %s\n", asf_guid_tostring (&obj->file_id));
	printf ("\tdata_packet_count = %llu\n", obj->data_packet_count);
	printf ("\treserved = %u\n", (asf_dword) obj->reserved);
}

void asf_object_dump (const asf_object* obj)
{
	printf ("ASF_OBJECT\n");
	printf ("\tid = %s\n", asf_guid_tostring (&obj->id));
	printf ("\tsize = %llu\n", obj->size);
}

void WAVEFORMATEX_dump (const WAVEFORMATEX* obj)
{
	printf ("WAVEFORMATEX\n");
	printf ("\tcodec_id = %u\n", (asf_dword) obj->codec_id);
	printf ("\tchannels = %u\n", (asf_dword) obj->channels);
	printf ("\tsamples_per_second = %u\n", (asf_dword) obj->samples_per_second);
	printf ("\tbytes_per_second = %u\n", (asf_dword) obj->bytes_per_second);
	printf ("\tblock_alignment = %u\n", (asf_dword) obj->block_alignment);
	printf ("\tbits_per_sample = %u\n", (asf_dword) obj->bits_per_sample);
	printf ("\tcodec_specific_data_size = %u\n", (asf_dword) obj->codec_specific_data_size);
}

void BITMAPINFOHEADER_dump (const BITMAPINFOHEADER* obj)
{
	printf ("BITMAPINFOHEADER\n");
	printf ("\tsize = %u\n", (asf_dword) obj->size);
	printf ("\timage_width = %u\n", (asf_dword) obj->image_width);
	printf ("\timage_height = %u\n", (asf_dword) obj->image_height);
	printf ("\treserved = %u\n", (asf_dword) obj->reserved);
	printf ("\tbits_per_pixel = %u\n", (asf_dword) obj->bits_per_pixel);
	printf ("\tcompression_id = %u\n", (asf_dword) obj->compression_id);
	printf ("\timage_size = %u\n", (asf_dword) obj->image_size);
	printf ("\thor_pixels_per_meter = %u\n", (asf_dword) obj->hor_pixels_per_meter);
	printf ("\tver_pixels_per_meter = %u\n", (asf_dword) obj->ver_pixels_per_meter);
	printf ("\tcolors_used = %u\n", (asf_dword) obj->colors_used);
	printf ("\timportant_colors_used = %u\n", (asf_dword) obj->important_colors_used);
}

void asf_video_stream_data_dump (const asf_video_stream_data* obj)
{
	printf ("ASF_VIDEO_STREAM_DATA\n");
	printf ("\timage_width = %u\n", (asf_dword) obj->image_width);
	printf ("\timage_height = %u\n", (asf_dword) obj->image_height);
	printf ("\tflags = %u\n", (asf_dword) obj->flags);
	printf ("\tformat_data_size = %u\n", (asf_dword) obj->format_data_size);
}


void asf_object_dump_exact (const asf_object* obj)
{
	switch (asf_get_guid_type (&obj->id)) {
	case ASF_HEADER:
		asf_header_dump ((asf_header*) obj); break;
	case ASF_FILE_PROPERTIES:
		asf_file_properties_dump ((asf_file_properties*) obj); break;
	case ASF_STREAM_PROPERTIES:
		asf_stream_properties_dump ((asf_stream_properties*) obj); break;
	case ASF_HEADER_EXTENSION:
		asf_header_extension_dump ((asf_header_extension*) obj); break;
	case ASF_CODEC_LIST:
		asf_codec_list_dump ((asf_codec_list*) obj); break;
	case ASF_SCRIPT_COMMAND:
		asf_script_command_dump ((asf_script_command*) obj); break;
	case ASF_MARKER:
		asf_marker_dump ((asf_marker*) obj); break;
	case ASF_BITRATE_MUTUAL_EXCLUSION:
		asf_bitrate_mutual_exclusion_dump ((asf_bitrate_mutual_exclusion*) obj); break;
	case ASF_ERROR_CORRECTION:
		asf_error_correction_dump ((asf_error_correction*) obj); break;
	case ASF_CONTENT_DESCRIPTION:
		asf_content_description_dump ((asf_content_description*) obj); break;
	case ASF_EXTENDED_CONTENT_DESCRIPTION:
		asf_extended_content_description_dump ((asf_extended_content_description*) obj); break;
	case ASF_STREAM_BITRATE_PROPERTIES:
		asf_stream_bitrate_properties_dump ((asf_stream_bitrate_properties*) obj); break;
	case ASF_DATA:
		asf_data_dump ((asf_data*) obj); break;
	default:
		asf_object_dump (obj); break;
	}
}

/* 
  Some almost read to use copy-and-paste code.


bool asf_header_validate (const asf_header* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_header, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 54) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 54, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_file_properties_validate (const asf_file_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_file_properties, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 128) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 128, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_stream_properties_validate (const asf_stream_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_stream_properties, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 102) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 102, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_header_extension_validate (const asf_header_extension* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_header_extension, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 70) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 70, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_codec_list_validate (const asf_codec_list* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_codec_list, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 68) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 68) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 72) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 72, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 66) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 66, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 68) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 58) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 58, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 50) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 50) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %llu).\n", obj->size));
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
	// FIXME: Verify that this size is correct.
	if (obj->size < 74) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 74, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_object_validate (const asf_object* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_object, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 48) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 48, got %llu).\n", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}


*/

