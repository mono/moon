/*
 * asf-generated.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_MOONLIGHT_GENERATED_H_
#define _ASF_MOONLIGHT_GENERATED_H_

struct WAVEFORMATEX;
struct WAVEFORMATEXTENSIBLE;
struct BITMAPINFOHEADER;
struct asf_video_stream_data;
struct asf_error_correction_data;
struct asf_payload_parsing_information;
struct asf_single_payload;
struct asf_multiple_payloads;
struct asf_object;
struct asf_header;
struct asf_file_properties;
struct asf_stream_properties;
struct asf_header_extension;
struct asf_codec_list;
struct asf_script_command_entry;
struct asf_script_command;
struct asf_marker_entry;
struct asf_marker;
struct asf_bitrate_mutual_exclusion;
struct asf_error_correction;
struct asf_content_description;
struct asf_extended_content_description;
struct asf_stream_bitrate_properties;
struct asf_data;
struct asf_extended_stream_name;
struct asf_payload_extension_system;
struct asf_extended_stream_properties;

class ASFPacket;
class ASFReader;
class ASFFrameReader;
class ASFParser;

 /* Validation functions */ 
bool asf_object_validate_exact (const asf_object* obj, ASFParser* parser);
bool WAVEFORMATEX_validate (const WAVEFORMATEX* obj, ASFParser* parser);
bool WAVEFORMATEXTENSIBLE_validate (const WAVEFORMATEXTENSIBLE* obj, ASFParser* parser);
bool BITMAPINFOHEADER_validate (const BITMAPINFOHEADER* obj, ASFParser* parser);
bool asf_video_stream_data_validate (const asf_video_stream_data* obj, ASFParser* parser);
bool asf_error_correction_data_validate (const asf_error_correction_data* obj, ASFParser* parser);
bool asf_payload_parsing_information_validate (const asf_payload_parsing_information* obj, ASFParser* parser);
bool asf_single_payload_validate (const asf_single_payload* obj, ASFParser* parser);
bool asf_multiple_payloads_validate (const asf_multiple_payloads* obj, ASFParser* parser);
bool asf_object_validate (const asf_object* obj, ASFParser* parser);
bool asf_header_validate (const asf_header* obj, ASFParser* parser);
bool asf_file_properties_validate (const asf_file_properties* obj, ASFParser* parser);
bool asf_stream_properties_validate (const asf_stream_properties* obj, ASFParser* parser);
bool asf_header_extension_validate (const asf_header_extension* obj, ASFParser* parser);
bool asf_codec_list_validate (const asf_codec_list* obj, ASFParser* parser);
bool asf_script_command_entry_validate (const asf_script_command_entry* obj, ASFParser* parser);
bool asf_script_command_validate (const asf_script_command* obj, ASFParser* parser);
bool asf_marker_entry_validate (const asf_marker_entry* obj, ASFParser* parser);
bool asf_marker_validate (const asf_marker* obj, ASFParser* parser);
bool asf_bitrate_mutual_exclusion_validate (const asf_bitrate_mutual_exclusion* obj, ASFParser* parser);
bool asf_error_correction_validate (const asf_error_correction* obj, ASFParser* parser);
bool asf_content_description_validate (const asf_content_description* obj, ASFParser* parser);
bool asf_extended_content_description_validate (const asf_extended_content_description* obj, ASFParser* parser);
bool asf_stream_bitrate_properties_validate (const asf_stream_bitrate_properties* obj, ASFParser* parser);
bool asf_data_validate (const asf_data* obj, ASFParser* parser);
bool asf_extended_stream_name_validate (const asf_extended_stream_name* obj, ASFParser* parser);
bool asf_payload_extension_system_validate (const asf_payload_extension_system* obj, ASFParser* parser);
bool asf_extended_stream_properties_validate (const asf_extended_stream_properties* obj, ASFParser* parser);

/* Debug functions */ 
void asf_header_dump (const asf_header* obj);
void asf_file_properties_dump (const asf_file_properties* obj);
void asf_stream_properties_dump (const asf_stream_properties* obj);
void asf_header_extension_dump (const asf_header_extension* obj);
void asf_codec_list_dump (const asf_codec_list* obj);
void asf_marker_dump (const asf_marker* obj);
void asf_bitrate_mutual_exclusion_dump (const asf_bitrate_mutual_exclusion* obj);
void asf_error_correction_dump (const asf_error_correction* obj);
void asf_content_description_dump (const asf_content_description* obj);
void asf_extended_content_description_dump (const asf_extended_content_description* obj);
void asf_stream_bitrate_properties_dump (const asf_stream_bitrate_properties* obj);
void asf_data_dump (const asf_data* obj);
void asf_extended_stream_properties_dump (const asf_extended_stream_properties* obj);
void asf_object_dump (const asf_object* obj);
void WAVEFORMATEX_dump (const WAVEFORMATEX* obj);
void BITMAPINFOHEADER_dump (const BITMAPINFOHEADER* obj);
void asf_video_stream_data_dump (const asf_video_stream_data* obj);

void print_sizes ();
void asf_object_dump_exact (const asf_object* obj);

#endif

