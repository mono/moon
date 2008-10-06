/*
 * asf-guids.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include "asf.h"


/*
	GUIDs
*/

asf_guid asf_guids_empty = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; 

/* Top level object guids */

asf_guid asf_guids_header = { 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
asf_guid asf_guids_data = { 0x75B22636, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
asf_guid asf_guids_index = { 0xD6E229D3, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
asf_guid asf_guids_simple_index = { 0x33000890, 0xE5B1, 0x11CF, { 0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB } };
asf_guid asf_guids_media_object_index = { 0xFEB103F8, 0x12AD, 0x4C64, { 0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C } };
asf_guid asf_guids_timecode_index = { 0x3CB73FD0, 0x0C4A, 0x4803, { 0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C } };

/* Header object guids */

asf_guid asf_guids_file_properties = { 0x8CABDCA1, 0xA947, 0x11CF, { 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
asf_guid asf_guids_stream_properties = { 0xB7DC0791, 0xA9B7, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
asf_guid asf_guids_header_extension = { 0x5FBF03B5, 0xA92E, 0x11CF, { 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
asf_guid asf_guids_codec_list = { 0x86D15240, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
asf_guid asf_guids_script_command = { 0x1EFB1A30, 0x0B62, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
asf_guid asf_guids_marker = { 0xF487CD01, 0xA951, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };
asf_guid asf_guids_bitrate_mutual_exclusion = { 0xD6E229DC, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
asf_guid asf_guids_error_correction = { 0x75B22635, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
asf_guid asf_guids_content_description = { 0x75B22633, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
asf_guid asf_guids_extended_content_description = { 0xD2D0A440, 0xE307, 0x11D2, { 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 } };
asf_guid asf_guids_content_branding = { 0x2211B3FA, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
asf_guid asf_guids_stream_bitrate_properties = { 0x7BF875CE, 0x468D, 0x11D1, { 0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2 } };
asf_guid asf_guids_content_encryption = { 0x2211B3FB, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
asf_guid asf_guids_extended_content_encryption = { 0x298AE614, 0x2622, 0x4C17, { 0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C } };
asf_guid asf_guids_digital_signature = { 0x2211B3FC, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };
asf_guid asf_guids_padding = { 0x1806D474, 0xCADF, 0x4509, { 0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8 } };

/* Header extension object guids */
asf_guid asf_guids_extended_stream_properties = { 0x14E6A5CB, 0xC672, 0x4332, { 0x83, 0x99, 0xA9, 0x69, 0x52, 0x06, 0x5B, 0x5A } };
asf_guid asf_guids_advanced_mutual_exclusion = { 0xA08649CF, 0x4775, 0x4670, { 0x8A, 0x16, 0x6E, 0x35, 0x35, 0x75, 0x66, 0xCD } };
asf_guid asf_guids_group_mutual_exclusion = { 0xD1465A40, 0x5A79, 0x4338, { 0xB7, 0x1B, 0xE3, 0x6B, 0x8F, 0xD6, 0xC2, 0x49 } };
asf_guid asf_guids_stream_prioritization = { 0xD4FED15B, 0x88D3, 0x454F, { 0x81, 0xF0, 0xED, 0x5C, 0x45, 0x99, 0x9E, 0x24 } };
asf_guid asf_guids_bandwidth_sharing = { 0xA69609E6, 0x517B, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };
asf_guid asf_guids_language_list = { 0x7C4346A9, 0xEFE0, 0x4BFC, { 0xB2, 0x29, 0x39, 0x3E, 0xDE, 0x41, 0x5C, 0x85 } };
asf_guid asf_guids_metadata = { 0xC5F8CBEA, 0x5BAF, 0x4877, { 0x84, 0x67, 0xAA, 0x8C, 0x44, 0xFA, 0x4C, 0xCA } };
asf_guid asf_guids_metadata_library = { 0x44231C94, 0x9498, 0x49D1, { 0xA1, 0x41, 0x1D, 0x13, 0x4E, 0x45, 0x70, 0x54 } };
asf_guid asf_guids_index_parameters = { 0xD6E229DF, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
asf_guid asf_guids_media_object_index_parameters = { 0x6B203BAD, 0x3F11, 0x48E4, { 0xAC, 0xA8, 0xD7, 0x61, 0x3D, 0xE2, 0xCF, 0xA7 } };
asf_guid asf_guids_timecode_index_parameters = { 0xF55E496D, 0x9797, 0x4B5D, { 0x8C, 0x8B, 0x60, 0x4D, 0xFE, 0x9B, 0xFB, 0x24 } };
asf_guid asf_guids_compatibility = { 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };
asf_guid asf_guids_advanced_content_encryption = { 0x43058533, 0x6981, 0x49E6, { 0x9B, 0x74, 0xAD, 0x12, 0xCB, 0x86, 0xD5, 0x8C } };

/* Stream properties object, stream type guids */

asf_guid asf_guids_media_audio = { 0xF8699E40, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
asf_guid asf_guids_media_video = { 0xBC19EFC0, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
asf_guid asf_guids_media_command = { 0x59DACFC0, 0x59E6, 0x11D0, { 0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };
asf_guid asf_guids_media_jfif = { 0xB61BE100, 0x5B4E, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
asf_guid asf_guids_media_degradable_jpeg = { 0x35907DE0, 0xE415, 0x11CF, { 0xA9, 0x17, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
asf_guid asf_guids_file_transfer = { 0x91BD222C, 0xF21C, 0x497A, { 0x8B, 0x6D, 0x5A, 0xA8, 0x6B, 0xFC, 0x01, 0x85 } };
asf_guid asf_guids_binary = { 0x3AFB65E2, 0x47EF, 0x40F2, { 0xAC, 0x2C, 0x70, 0xA9, 0x0D, 0x71, 0xD3, 0x43 } };

/* Web stream type-specific data guids */
asf_guid asf_guids_webstream_media_subtype = { 0x776257D4, 0xC627, 0x41CB, { 0x8F, 0x81, 0x7A, 0xC7, 0xFF, 0x1C, 0x40, 0xCC } };
asf_guid asf_guids_webstream_format = { 0xDA1E6B13, 0x8359, 0x4050, { 0xB3, 0x98, 0x38, 0x8E, 0x96, 0x5B, 0xF0, 0x0C } };

/* Stream properties, object error correction type guids */
asf_guid asf_guids_no_error_correction = { 0x20FB5700, 0x5B55, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
asf_guid asf_guids_audio_stread = { 0xBFC3CD50, 0x618F, 0x11CF, { 0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20 } };

/* Header extension object guids */
asf_guid asf_guids_reserved1 = { 0xABD3D211, 0xA9BA, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };

/* Advanced content encryption object system id guids */
asf_guid asf_guids_drm = { 0x7A079BB6, 0xDAA4, 0x4E12, { 0xA5, 0xCA, 0x91, 0xD3, 0x8D, 0xC1, 0x1A, 0x8D } };
// drm = Content_Encryption_System_Windows_Media_DRM_Network_Devides in the spec
// Figured it was somewhat long, so it got abbreviated 

/* Codec list object guids */
asf_guid asf_guids_reserved2 = { 0x86D15241, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };

/* Script command object guids */ 
asf_guid asf_guids_reserved3 = { 0x4B1ACBE3, 0x100B, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };

/* Marker object guids */
asf_guid asf_guids_reserved4 = { 0x4CFEDB20, 0x75F6, 0x11CF, { 0x9C, 0x0F, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB } };

/* Mutual exclusion object exclusion type guids */
asf_guid asf_guids_mutex_language = { 0xD6E22A00, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
asf_guid asf_guids_mutex_bitrate = { 0xD6E22A01, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };
asf_guid asf_guids_mutex_unknown = { 0xD6E22A02, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

/* Bandwidth sharing object guids */
asf_guid asf_guids_bandwidth_sharing_exclusive = { 0xAF6060AA, 0x5197, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };
asf_guid asf_guids_bandwidth_sharing_partial = { 0xAF6060AB, 0x5197, 0x11D2, { 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9 } };

/* Standard payload extension system guids */
asf_guid asf_guids_payload_timecode = { 0x399595EC, 0x8667, 0x4E2D, { 0x8F, 0xDB, 0x98, 0x81, 0x4C, 0xE7, 0x6C, 0x1E } };
asf_guid asf_guids_payload_filename = { 0xE165EC0E, 0x19ED, 0x45D7, { 0xB4, 0xA7, 0x25, 0xCB, 0xD1, 0xE2, 0x8E, 0x9B } };
asf_guid asf_guids_payload_content_type = { 0xD590DC20, 0x07BC, 0x436C, { 0x9C, 0xF7, 0xF3, 0xBB, 0xFB, 0xF1, 0xA4, 0xDC } };
asf_guid asf_guids_payload_pixel_aspect_ratio = { 0x1B1EE554, 0xF9EA, 0x4BC8, { 0x82, 0x1A, 0x37, 0x6B, 0x74, 0xE4, 0xC4, 0xB8 } };
asf_guid asf_guids_payload_sample_duration = { 0xC6BD9450, 0x867F, 0x4907, { 0x83, 0xA3, 0xC7, 0x79, 0x21, 0xB7, 0x33, 0xAD } };
asf_guid asf_guids_payload_encryption_sample_id = { 0x6698B84E, 0x0AFA, 0x4330, { 0xAE, 0xB2, 0x1C, 0x0A, 0x98, 0xD7, 0xA4, 0x4D } };

static const struct {
	asf_guid guid;
	ASFTypes type;
	const char *name;
} asf_types [] = {
	{ asf_guids_header, ASF_HEADER, "ASF_HEADER" },
	{ asf_guids_data, ASF_DATA, "ASF_DATA" },
	{ asf_guids_index, ASF_INDEX, "ASF_INDEX" },
	{ asf_guids_simple_index, ASF_SIMPLE_INDEX, "ASF_SIMPLE_INDEX" },
	{ asf_guids_media_object_index, ASF_MEDIA_OBJECT_INDEX, "ASF_MEDIA_OBJECT_INDEX" },
	{ asf_guids_timecode_index, ASF_TIMECODE_INDEX, "ASF_TIMECODE_INDEX" },
	
	{ asf_guids_file_properties, ASF_FILE_PROPERTIES, "ASF_FILE_PROPERTIES" },
	{ asf_guids_stream_properties, ASF_STREAM_PROPERTIES, "ASF_STREAM_PROPERTIES" },
	{ asf_guids_header_extension, ASF_HEADER_EXTENSION, "ASF_HEADER_EXTENSION" },
	{ asf_guids_codec_list, ASF_CODEC_LIST, "ASF_CODEC_LIST" },
	{ asf_guids_script_command, ASF_SCRIPT_COMMAND, "ASF_SCRIPT_COMMAND" },
	{ asf_guids_marker, ASF_MARKER, "ASF_MARKER" },
	{ asf_guids_bitrate_mutual_exclusion, ASF_BITRATE_MUTUAL_EXCLUSION, "ASF_BITRATE_MUTUAL_EXCLUSION" },
	{ asf_guids_error_correction, ASF_ERROR_CORRECTION, "ASF_ERROR_CORRECTION" },
	{ asf_guids_content_description, ASF_CONTENT_DESCRIPTION, "ASF_CONTENT_DESCRIPTION" },
	{ asf_guids_extended_content_description, ASF_EXTENDED_CONTENT_DESCRIPTION, "ASF_EXTENDED_CONTENT_DESCRIPTION" },
	{ asf_guids_content_branding, ASF_CONTENT_BRANDING, "ASF_CONTENT_BRANDING" },
	{ asf_guids_stream_bitrate_properties, ASF_STREAM_BITRATE_PROPERTIES, "ASF_STREAM_BITRATE_PROPERTIES" },
	{ asf_guids_content_encryption, ASF_CONTENT_ENCRYPTION, "ASF_CONTENT_ENCRYPTION" },
	{ asf_guids_extended_content_encryption, ASF_EXTENDED_CONTENT_ENCRYPTION, "ASF_EXTENDED_CONTENT_ENCRYPTION" },
	{ asf_guids_digital_signature, ASF_DIGITAL_SIGNATURE, "ASF_DIGITAL_SIGNATURE" },
	{ asf_guids_padding, ASF_PADDING, "ASF_PADDING" },
	
	{ asf_guids_extended_stream_properties, ASF_EXTENDED_STREAM_PROPERTIES, "ASF_EXTENDED_STREAM_PROPERTIES" },
	{ asf_guids_advanced_mutual_exclusion, ASF_ADVANCED_MUTUAL_EXCLUSION, "ASF_ADVANCED_MUTUAL_EXCLUSION" },
	{ asf_guids_group_mutual_exclusion, ASF_GROUP_MUTUAL_EXCLUSION, "ASF_GROUP_MUTUAL_EXCLUSION" },
	{ asf_guids_stream_prioritization, ASF_STREAM_PRIORITIZATION, "ASF_STREAM_PRIORITIZATION" },
	{ asf_guids_bandwidth_sharing, ASF_BANDWIDTH_SHARING, "ASF_BANDWIDTH_SHARING" },
	{ asf_guids_language_list, ASF_LANGUAGE_LIST, "ASF_LANGUAGE_LIST" },
	{ asf_guids_metadata, ASF_METADATA, "ASF_METADATA" },
	{ asf_guids_metadata_library, ASF_METADATA_LIBRARY, "ASF_METADATA_LIBRARY" },
	{ asf_guids_index_parameters, ASF_INDEX_PARAMETERS, "ASF_INDEX_PARAMETERS" },
	{ asf_guids_media_object_index_parameters, ASF_MEDIA_OBJECT_INDEX_PARAMETERS, "ASF_MEDIA_OBJECT_INDEX_PARAMETERS" },
	{ asf_guids_timecode_index_parameters, ASF_TIMECODE_INDEX_PARAMETERS, "ASF_TIMECODE_INDEX_PARAMETERS" },
	{ asf_guids_compatibility, ASF_COMPATIBILITY, "ASF_COMPATIBILITY" },
	{ asf_guids_advanced_content_encryption, ASF_ADVANCED_CONTENT_ENCRYPTION, "ASF_ADVANCED_CONTENT_ENCRYPTION" },
	
	{ asf_guids_media_audio, ASF_MEDIA_AUDIO, "ASF_MEDIA_AUDIO" },
	{ asf_guids_media_video, ASF_MEDIA_VIDEO, "ASF_MEDIA_VIDEO" },
	{ asf_guids_media_command, ASF_MEDIA_COMMAND, "ASF_MEDIA_COMMAND" },
	{ asf_guids_media_jfif, ASF_MEDIA_JFIF, "ASF_MEDIA_JFIF" },
	{ asf_guids_media_degradable_jpeg, ASF_MEDIA_DEGRADABLE_JPEG, "ASF_MEDIA_DEGRADABLE_JPEG" },
	{ asf_guids_file_transfer, ASF_FILE_TRANSFER, "ASF_FILE_TRANSFER" },
	{ asf_guids_binary, ASF_BINARY, "ASF_BINARY" },
	
	{ asf_guids_webstream_media_subtype, ASF_WEBSTREAM_MEDIA_SUBTYPE, "ASF_WEBSTREAM_MEDIA_SUBTYPE" },
	{ asf_guids_webstream_format, ASF_WEBSTREAM_FORMAT, "ASF_WEBSTREAM_FORMAT" },
	
	{ asf_guids_no_error_correction, ASF_NO_ERROR_CORRECTION, "ASF_NO_ERROR_CORRECTION" },
	{ asf_guids_audio_stread, ASF_AUDIO_STREAD, "ASF_AUDIO_STREAD" },
	
	{ asf_guids_reserved1, ASF_RESERVED1, "ASF_RESERVED1" },
	
	{ asf_guids_drm, ASF_DRM, "ASF_DRM" },
	
	{ asf_guids_reserved2, ASF_RESERVED2, "ASF_RESERVED2" },
	
	{ asf_guids_reserved3, ASF_RESERVED3, "ASF_RESERVED3" },
	
	{ asf_guids_reserved4, ASF_RESERVED4, "ASF_RESERVED4" },
	
	{ asf_guids_mutex_language, ASF_MUTEX_LANGUAGE, "ASF_MUTEX_LANGUAGE" },
	{ asf_guids_mutex_bitrate, ASF_MUTEX_BITRATE, "ASF_MUTEX_BITRATE" },
	{ asf_guids_mutex_unknown, ASF_MUTEX_UNKNOWN, "ASF_MUTEX_UNKNOWN" },
	
	{ asf_guids_bandwidth_sharing_exclusive, ASF_BANDWIDTH_SHARING_EXCLUSIVE ,"ASF_BANDWIDTH_SHARING_EXCLUSIVE" },
	{ asf_guids_bandwidth_sharing_partial, ASF_BANDWIDTH_SHARING_PARTIAL, "ASF_BANDWIDTH_SHARING_PARTIAL" },
	
	{ asf_guids_payload_timecode, ASF_PAYLOAD_TIMECODE, "ASF_PAYLOAD_TIMECODE" },
	{ asf_guids_payload_filename, ASF_PAYLOAD_FILENAME, "ASF_PAYLOAD_FILENAME" },
	{ asf_guids_payload_content_type, ASF_PAYLOAD_CONTENT_TYPE, "ASF_PAYLOAD_CONTENT_TYPE" },
	{ asf_guids_payload_pixel_aspect_ratio, ASF_PAYLOAD_PIXEL_ASPECT_RATIO, "ASF_PAYLOAD_PIXEL_ASPECT_RATIO" },
	{ asf_guids_payload_sample_duration, ASF_PAYLOAD_SAMPLE_DURATION, "ASF_PAYLOAD_SAMPLE_DURATION" },
	{ asf_guids_payload_encryption_sample_id, ASF_PAYLOAD_ENCRYPTION_SAMPLE_ID, "ASF_PAYLOAD_ENCRYPTION_SAMPLE_ID" },
	
	{ asf_guids_empty, ASF_LAST_TYPE, "ASF_LAST_TYPE" }
};




bool
asf_guid_compare (const asf_guid* a, const asf_guid* b)
{
	if (a == b)
		return true;
		
	if (a == NULL || b == NULL)
		return false;
		
	return memcmp (a, b, sizeof (asf_guid)) == 0;
}

ASFTypes
asf_get_guid_type (const asf_guid* guid)
{
	int i = 0;
	while (asf_types [i].type != ASF_LAST_TYPE) {
		if (asf_guid_compare (&asf_types [i].guid, guid)) { 
			return asf_types [i].type;
		}
		
		i++;
	}
	
	return ASF_NONE;
}

const char*
asf_type_get_name (ASFTypes type)
{
	int i = 0;
	while (asf_types [i].type != ASF_LAST_TYPE) {
		if (asf_types [i].type == type) {
			return asf_types [i].name;
		}
		
		i++;
	}
	
	return "<unknown type>";
}

const char*
asf_guid_get_name (const asf_guid* guid)
{
	return asf_type_get_name (asf_get_guid_type (guid));
}

char* 
asf_guid_tostring (const asf_guid* g)
{
	return g_strdup_printf ("GUID: %s = (%X, %X, %X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X)", 
		asf_guid_get_name (g), g->a, g->b, g->c, g->d[0], g->d[1], g->d[2], g->d[3], g->d[4], g->d[5], g->d[6], g->d[7]);
}

bool
asf_guid_validate (const asf_guid* guid_actual, const asf_guid* guid_expected, ASFParser* parser)
{
	if (!asf_guid_compare (guid_actual, guid_expected)) {
		char* expected = asf_guid_tostring (guid_expected);
		char* actual = asf_guid_tostring (guid_actual);
		parser->AddError (g_strdup_printf ("Invalid id (expected: %s, got: %s).", expected, actual));
		g_free (actual);
		g_free (expected);
		return false;
	}
	
	return true;
}
