/*
 * asf-generated.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include "asf.h"

void print_sizes () {
	printf ("sizeof (asf_header) = %i.\n", (int) sizeof (asf_header));
	printf ("sizeof (asf_file_properties) = %i.\n", (int) sizeof (asf_file_properties));
	printf ("sizeof (asf_stream_properties) = %i.\n", (int) sizeof (asf_stream_properties));
	printf ("sizeof (asf_header_extension) = %i.\n", (int) sizeof (asf_header_extension));
	printf ("sizeof (asf_codec_list) = %i.\n", (int) sizeof (asf_codec_list));
	printf ("sizeof (asf_script_command) = %i.\n", (int) sizeof (asf_script_command));
	printf ("sizeof (asf_marker) = %i.\n", (int) sizeof (asf_marker));
	printf ("sizeof (asf_bitrate_mutual_exclusion) = %i.\n", (int) sizeof (asf_bitrate_mutual_exclusion));
	printf ("sizeof (asf_error_correction) = %i.\n", (int) sizeof (asf_error_correction));
	printf ("sizeof (asf_content_description) = %i.\n", (int) sizeof (asf_content_description));
	printf ("sizeof (asf_extended_content_description) = %i.\n", (int) sizeof (asf_extended_content_description));
	printf ("sizeof (asf_stream_bitrate_properties) = %i.\n", (int) sizeof (asf_stream_bitrate_properties));
	printf ("sizeof (asf_data) = %i.\n", (int) sizeof (asf_data));
	printf ("sizeof (asf_extended_stream_properties) = %i.\n", (int) sizeof (asf_extended_stream_properties));
	printf ("sizeof (asf_object) = %i.\n", (int) sizeof (asf_object));
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
	case ASF_EXTENDED_STREAM_PROPERTIES:
		return asf_extended_stream_properties_validate ((asf_extended_stream_properties*) obj, parser);
	case ASF_NONE:
	case ASF_LANGUAGE_LIST:
	case ASF_METADATA:
	case ASF_PADDING:
	case ASF_ADVANCED_MUTUAL_EXCLUSION:
	case ASF_STREAM_PRIORITIZATION:
	case ASF_INDEX_PARAMETERS:
		return true; // Do nothing, we don't use these objects at all, so there's no need to validate either.
	default:
#if DEBUG
		printf ("ASF warning: No validation implemented for %s.\n", asf_guid_get_name (&obj->id));
#endif
		return true;
	}
}

/* Debug functions */ 
void asf_header_dump (const asf_header* obj)
{
	ASF_DUMP ("ASF_HEADER\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tobject_count = %u\n", (asf_dword) obj->object_count);
	ASF_DUMP ("\treserved1 = %u\n", (asf_dword) obj->reserved1);
	ASF_DUMP ("\treserved2 = %u\n", (asf_dword) obj->reserved2);
}

void asf_file_properties_dump (const asf_file_properties* obj)
{
	ASF_DUMP ("ASF_FILE_PROPERTIES\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tfile_id = %s\n", asf_guid_tostring (&obj->file_id));
	ASF_DUMP ("\tfile_size = %" G_GUINT64_FORMAT "\n", obj->file_size);
	ASF_DUMP ("\tcreation_date = %" G_GUINT64_FORMAT "\n", obj->creation_date);
	ASF_DUMP ("\tdata_packet_count = %" G_GUINT64_FORMAT "\n", obj->data_packet_count);
	ASF_DUMP ("\tplay_duration = %" G_GUINT64_FORMAT "\n", obj->play_duration);
	ASF_DUMP ("\tsend_duration = %" G_GUINT64_FORMAT "\n", obj->send_duration);
	ASF_DUMP ("\tpreroll = %" G_GUINT64_FORMAT "\n", obj->preroll);
	ASF_DUMP ("\tflags = %u\n", (asf_dword) obj->flags);
	ASF_DUMP ("\tmin_packet_size = %u\n", (asf_dword) obj->min_packet_size);
	ASF_DUMP ("\tmax_packet_size = %u\n", (asf_dword) obj->max_packet_size);
	ASF_DUMP ("\tmax_bitrate = %u\n", (asf_dword) obj->max_bitrate);
}

void asf_stream_properties_dump (const asf_stream_properties* obj)
{
	ASF_DUMP ("ASF_STREAM_PROPERTIES\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tstream_type = %s\n", asf_guid_tostring (&obj->stream_type));
	ASF_DUMP ("\terror_correction_type = %s\n", asf_guid_tostring (&obj->error_correction_type));
	ASF_DUMP ("\ttime_offset = %" G_GUINT64_FORMAT "\n", obj->time_offset);
	ASF_DUMP ("\ttype_specific_data_length = %u\n", (asf_dword) obj->type_specific_data_length);
	ASF_DUMP ("\terror_correction_data_length = %u\n", (asf_dword) obj->error_correction_data_length);
	ASF_DUMP ("\tflags = %u\n", (asf_dword) obj->flags);
	ASF_DUMP ("\treserved = %u\n", (asf_dword) obj->reserved);
}

void asf_codec_list_dump (const asf_codec_list* obj)
{
	ASF_DUMP ("ASF_CODEC_LIST\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\treserved = %s\n", asf_guid_tostring (&obj->reserved));
	ASF_DUMP ("\tentries_count = %u\n", (asf_dword) obj->entries_count);
}

void asf_bitrate_mutual_exclusion_dump (const asf_bitrate_mutual_exclusion* obj)
{
	ASF_DUMP ("ASF_BITRATE_MUTUAL_EXCLUSION\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\texclusion_type = %s\n", asf_guid_tostring (&obj->exclusion_type));
	ASF_DUMP ("\tstream_count = %u\n", (asf_dword) obj->stream_count);
}

void asf_error_correction_dump (const asf_error_correction* obj)
{
	ASF_DUMP ("ASF_ERROR_CORRECTION\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\terror_correction_type = %s\n", asf_guid_tostring (&obj->error_correction_type));
	ASF_DUMP ("\terror_correction_data_length = %u\n", (asf_dword) obj->error_correction_data_length);
}

void asf_content_description_dump (const asf_content_description* obj)
{
	ASF_DUMP ("ASF_CONTENT_DESCRIPTION\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\ttitle_length = %u\n", (asf_dword) obj->title_length);
	ASF_DUMP ("\tauthor_length = %u\n", (asf_dword) obj->author_length);
	ASF_DUMP ("\tcopyright_length = %u\n", (asf_dword) obj->copyright_length);
	ASF_DUMP ("\tdescription_length = %u\n", (asf_dword) obj->description_length);
	ASF_DUMP ("\trating_length = %u\n", (asf_dword) obj->rating_length);
}

void asf_extended_content_description_dump (const asf_extended_content_description* obj)
{
	ASF_DUMP ("ASF_EXTENDED_CONTENT_DESCRIPTION\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tcontent_descriptors_count = %u\n", (asf_dword) obj->content_descriptors_count);
}

void asf_stream_bitrate_properties_dump (const asf_stream_bitrate_properties* obj)
{
	ASF_DUMP ("ASF_STREAM_BITRATE_PROPERTIES\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tbitrate_records_count = %u\n", (asf_dword) obj->bitrate_records_count);
}

void asf_data_dump (const asf_data* obj)
{
	ASF_DUMP ("ASF_DATA\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
	ASF_DUMP ("\tfile_id = %s\n", asf_guid_tostring (&obj->file_id));
	ASF_DUMP ("\tdata_packet_count = %" G_GUINT64_FORMAT "\n", obj->data_packet_count);
	ASF_DUMP ("\treserved = %u\n", (asf_dword) obj->reserved);
}

void asf_object_dump (const asf_object* obj)
{
	ASF_DUMP ("ASF_OBJECT\n");
	ASF_DUMP ("\tid = %s\n", asf_guid_tostring (&obj->id));
	ASF_DUMP ("\tsize = %" G_GUINT64_FORMAT "\n", obj->size);
}

void WAVEFORMATEX_dump (const WAVEFORMATEX* obj)
{
	ASF_DUMP ("WAVEFORMATEX\n");
	ASF_DUMP ("\tcodec_id = %u\n", (asf_dword) obj->codec_id);
	ASF_DUMP ("\tchannels = %u\n", (asf_dword) obj->channels);
	ASF_DUMP ("\tsamples_per_second = %u\n", (asf_dword) obj->samples_per_second);
	ASF_DUMP ("\tbytes_per_second = %u\n", (asf_dword) obj->bytes_per_second);
	ASF_DUMP ("\tblock_alignment = %u\n", (asf_dword) obj->block_alignment);
	ASF_DUMP ("\tbits_per_sample = %u\n", (asf_dword) obj->bits_per_sample);
	ASF_DUMP ("\tcodec_specific_data_size = %u\n", (asf_dword) obj->codec_specific_data_size);
}

void BITMAPINFOHEADER_dump (const BITMAPINFOHEADER* obj)
{
	ASF_DUMP ("BITMAPINFOHEADER\n");
	ASF_DUMP ("\tsize = %u\n", (asf_dword) obj->size);
	ASF_DUMP ("\timage_width = %u\n", (asf_dword) obj->image_width);
	ASF_DUMP ("\timage_height = %u\n", (asf_dword) obj->image_height);
	ASF_DUMP ("\treserved = %u\n", (asf_dword) obj->reserved);
	ASF_DUMP ("\tbits_per_pixel = %u\n", (asf_dword) obj->bits_per_pixel);
	ASF_DUMP ("\tcompression_id = %u\n", (asf_dword) obj->compression_id);
	ASF_DUMP ("\timage_size = %u\n", (asf_dword) obj->image_size);
	ASF_DUMP ("\thor_pixels_per_meter = %u\n", (asf_dword) obj->hor_pixels_per_meter);
	ASF_DUMP ("\tver_pixels_per_meter = %u\n", (asf_dword) obj->ver_pixels_per_meter);
	ASF_DUMP ("\tcolors_used = %u\n", (asf_dword) obj->colors_used);
	ASF_DUMP ("\timportant_colors_used = %u\n", (asf_dword) obj->important_colors_used);
}

void asf_video_stream_data_dump (const asf_video_stream_data* obj)
{
	ASF_DUMP ("ASF_VIDEO_STREAM_DATA\n");
	ASF_DUMP ("\timage_width = %u\n", (asf_dword) obj->image_width);
	ASF_DUMP ("\timage_height = %u\n", (asf_dword) obj->image_height);
	ASF_DUMP ("\tflags = %u\n", (asf_dword) obj->flags);
	ASF_DUMP ("\tformat_data_size = %u\n", (asf_dword) obj->format_data_size);
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
	case ASF_EXTENDED_STREAM_PROPERTIES:
		asf_extended_stream_properties_dump ((asf_extended_stream_properties*) obj); break;
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 54, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 128, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 102, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 70, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 72, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 66, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 68, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 58, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 50, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 74, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}

bool asf_extended_stream_properties_validate (const asf_extended_stream_properties* obj, ASFParser* parser)
{
	if (!(asf_guid_validate (&obj->id, &asf_guids_extended_stream_properties, parser))) {
		return false;
	}
	// FIXME: Verify that this size is correct.
	if (obj->size < 112) {
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 112, got %" G_GUINT64_FORMAT ").", obj->size));
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
		parser->AddError (g_strdup_printf ("Invalid size (expected >= 48, got %" G_GUINT64_FORMAT ").", obj->size));
		return false;
	}
	// TODO: More verifications?
	return true;
}


*/

