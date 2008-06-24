/*
 * asf-guids.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_GUIDS_MOONLIGHT_H
#define _ASF_GUIDS_MOONLIGHT_H

struct asf_guid {
	asf_dword a;
	asf_word b;
	asf_word c;
	asf_byte d [8];
};

enum ASFTypes {
	ASF_NONE = 0,
	ASF_HEADER,
	ASF_DATA,
	ASF_INDEX,
	ASF_SIMPLE_INDEX,
	ASF_MEDIA_OBJECT_INDEX,
	ASF_TIMECODE_INDEX,
	
	ASF_FILE_PROPERTIES,
	ASF_STREAM_PROPERTIES,
	ASF_HEADER_EXTENSION,
	ASF_CODEC_LIST,
	ASF_SCRIPT_COMMAND,
	ASF_MARKER,
	ASF_BITRATE_MUTUAL_EXCLUSION,
	ASF_ERROR_CORRECTION,
	ASF_CONTENT_DESCRIPTION,
	ASF_EXTENDED_CONTENT_DESCRIPTION,
	ASF_CONTENT_BRANDING,
	ASF_STREAM_BITRATE_PROPERTIES,
	ASF_CONTENT_ENCRYPTION,
	ASF_EXTENDED_CONTENT_ENCRYPTION,
	ASF_DIGITAL_SIGNATURE,
	ASF_PADDING,
	
	ASF_EXTENDED_STREAM_PROPERTIES,
	ASF_ADVANCED_MUTUAL_EXCLUSION,
	ASF_GROUP_MUTUAL_EXCLUSION,
	ASF_STREAM_PRIORITIZATION,
	ASF_BANDWIDTH_SHARING,
	ASF_LANGUAGE_LIST,
	ASF_METADATA,
	ASF_METADATA_LIBRARY,
	ASF_INDEX_PARAMETERS,
	ASF_MEDIA_OBJECT_INDEX_PARAMETERS,
	ASF_TIMECODE_INDEX_PARAMETERS,
	ASF_COMPATIBILITY,
	ASF_ADVANCED_CONTENT_ENCRYPTION,
	
	ASF_MEDIA_AUDIO,
	ASF_MEDIA_VIDEO,
	ASF_MEDIA_COMMAND,
	ASF_MEDIA_JFIF,
	ASF_MEDIA_DEGRADABLE_JPEG,
	ASF_FILE_TRANSFER,
	ASF_BINARY,
	
	ASF_WEBSTREAM_MEDIA_SUBTYPE,
	ASF_WEBSTREAM_FORMAT,
	
	ASF_NO_ERROR_CORRECTION,
	ASF_AUDIO_STREAD,
	
	ASF_RESERVED1,
	
	ASF_DRM,
	
	ASF_RESERVED2,
	
	ASF_RESERVED3,
	
	ASF_RESERVED4,
	
	ASF_MUTEX_LANGUAGE,
	ASF_MUTEX_BITRATE,
	ASF_MUTEX_UNKNOWN,
	
	ASF_BANDWIDTH_SHARING_EXCLUSIVE,
	ASF_BANDWIDTH_SHARING_PARTIAL,
	
	ASF_PAYLOAD_TIMECODE,
	ASF_PAYLOAD_FILENAME,
	ASF_PAYLOAD_CONTENT_TYPE,
	ASF_PAYLOAD_PIXEL_ASPECT_RATIO,
	ASF_PAYLOAD_SAMPLE_DURATION,
	ASF_PAYLOAD_ENCRYPTION_SAMPLE_ID,
	
	ASF_LAST_TYPE
};


extern asf_guid asf_guids_empty;

/* Top level object guids */

extern asf_guid asf_guids_header;
extern asf_guid asf_guids_data;
extern asf_guid asf_guids_index;
extern asf_guid asf_guids_simple_index;
extern asf_guid asf_guids_media_object_index;
extern asf_guid asf_guids_timecode_index;

/* Header object guids */
extern asf_guid asf_guids_file_properties;
extern asf_guid asf_guids_stream_properties;
extern asf_guid asf_guids_header_extension;
extern asf_guid asf_guids_codec_list;
extern asf_guid asf_guids_script_command;
extern asf_guid asf_guids_marker;
extern asf_guid asf_guids_bitrate_mutual_exclusion;
extern asf_guid asf_guids_error_correction;
extern asf_guid asf_guids_content_description;
extern asf_guid asf_guids_extended_content_description;
extern asf_guid asf_guids_content_branding;
extern asf_guid asf_guids_stream_bitrate_properties;
extern asf_guid asf_guids_content_encryption;
extern asf_guid asf_guids_extended_content_encryption;
extern asf_guid asf_guids_digital_signature;
extern asf_guid asf_guids_padding;

/* Header extension object guids */
extern asf_guid asf_guids_extended_stream_properties;
extern asf_guid asf_guids_advanced_mutual_exclusion;
extern asf_guid asf_guids_group_mutual_exclusion;
extern asf_guid asf_guids_stream_prioritization;
extern asf_guid asf_guids_bandwidth_sharing;
extern asf_guid asf_guids_language_list;
extern asf_guid asf_guids_metadata;
extern asf_guid asf_guids_metadata_library;
extern asf_guid asf_guids_index_parameters;
extern asf_guid asf_guids_media_object_index_parameters;
extern asf_guid asf_guids_timecode_index_parameters;
extern asf_guid asf_guids_compatibility;
extern asf_guid asf_guids_advanced_content_encryption;

/* Stream properties object, stream type guids */
extern asf_guid asf_guids_media_audio;
extern asf_guid asf_guids_media_video;
extern asf_guid asf_guids_media_command;
extern asf_guid asf_guids_media_jfif;
extern asf_guid asf_guids_media_degradable_jpeg;
extern asf_guid asf_guids_file_transfer;
extern asf_guid asf_guids_binary;

/* Web stream type-specific data guids */
extern asf_guid asf_guids_webstream_media_subtype;
extern asf_guid asf_guids_webstream_format;

/* Stream properties, object error correction type guids */
extern asf_guid asf_guids_no_error_correction;
extern asf_guid asf_guids_audio_stread;

/* Header extension object guids */
extern asf_guid asf_guids_reserved1;

/* Advanced content encryption object system id guids */
extern asf_guid asf_guids_drm;
// drm = Content_Encryption_System_Windows_Media_DRM_Network_Devides in the spec
// Figured it was somewhat long, so it got abbreviated 

/* Codec list object guids */
extern asf_guid asf_guids_reserved2;

/* Script command object guids */ 
extern asf_guid asf_guids_reserved3;

/* Marker object guids */
extern asf_guid asf_guids_reserved4;

/* Mutual exclusion object exclusion type guids */
extern asf_guid asf_guids_mutex_language;
extern asf_guid asf_guids_mutex_bitrate;
extern asf_guid asf_guids_mutex_unknown;

/* Bandwidth sharing object guids */
extern asf_guid asf_guids_bandwidth_sharing_exclusive;
extern asf_guid asf_guids_bandwidth_sharing_partial;

/* Standard payload extension system guids */
extern asf_guid asf_guids_payload_timecode;
extern asf_guid asf_guids_payload_filename;
extern asf_guid asf_guids_payload_content_type;
extern asf_guid asf_guids_payload_pixel_aspect_ratio;
extern asf_guid asf_guids_payload_sample_duration;
extern asf_guid asf_guids_payload_encryption_sample_id;

G_BEGIN_DECLS
/* Misc funtions */
bool asf_guid_compare (const asf_guid* a, const asf_guid* b);
ASFTypes asf_get_guid_type (const asf_guid* guid);
const char* asf_guid_get_name (const asf_guid* guid);
const char* asf_type_get_name (ASFTypes type);
char* asf_guid_tostring (const asf_guid* obj);
bool asf_guid_validate (const asf_guid* guid_actual, const asf_guid* guid_expected, ASFParser* parser);
G_END_DECLS

#endif
