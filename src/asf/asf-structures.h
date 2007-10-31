/*
 * asf-structures.h: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_STRUCTURES_MOONLIGHT_H
#define _ASF_STRUCTURES_MOONLIGHT_H

#include <glib.h>

#include "asf-debug.h"
#include "asf.h"

void asf_error_correction_data_dump (asf_error_correction_data* obj);
void asf_payload_parsing_information_dump (asf_payload_parsing_information* obj);

// Converts from ASF's WCHAR fields to a utf8 string.
// The returned string must be freed with g_free.
G_BEGIN_DECLS
char* wchar_to_utf8 (void* unicode, guint32 length);
G_END_DECLS

#define ASF_DECODE_PACKED_SIZE(x) ((x == 3 ? 4 : x))

// SPEC: All structure definitions assume 1-byte packing
#pragma pack(push)
#pragma pack(1) 

struct WAVEFORMATEXTENSIBLE;

struct WAVEFORMATEX {
	asf_word codec_id;
	asf_word channels;
	asf_dword samples_per_second;
	asf_dword bytes_per_second;
	asf_word block_alignment;
	asf_word bits_per_sample;
	asf_word codec_specific_data_size;
	
	bool is_wave_format_extensible ()
	{
		return codec_id == 0xFFFE && codec_specific_data_size >= 22;
	}
	
	WAVEFORMATEXTENSIBLE* get_wave_format_extensible ()
	{
		if (!is_wave_format_extensible ())
			return NULL;
		return (WAVEFORMATEXTENSIBLE*) this;
	}
};

struct WAVEFORMATEXTENSIBLE {
	WAVEFORMATEX format;
	union {
		asf_word valid_bits_per_sample;
		asf_word samples_per_block;
		asf_word reserved;
	} Samples;
	asf_dword channel_mask;
	asf_guid sub_format;
};

struct BITMAPINFOHEADER {
	asf_dword size;
	asf_dword image_width;
	asf_dword image_height;
	asf_word reserved;
	asf_word bits_per_pixel;
	asf_dword compression_id;
	asf_dword image_size;
	asf_dword hor_pixels_per_meter;
	asf_dword ver_pixels_per_meter;
	asf_dword colors_used;
	asf_dword important_colors_used;
	
	gint32 get_extra_data_size ()
	{
		return size - sizeof (BITMAPINFOHEADER);
	}
	void* get_extra_data ()
	{
		if (get_extra_data_size () <= 0)
			return NULL;
		return sizeof (BITMAPINFOHEADER) + ((char*) this);
	}
};

struct asf_video_stream_data {
	asf_dword image_width;
	asf_dword image_height;
	asf_byte flags;
	asf_word format_data_size;
	
	BITMAPINFOHEADER* get_bitmap_info_header ()
	{
		if (format_data_size < sizeof (BITMAPINFOHEADER))
			return NULL;
		return (BITMAPINFOHEADER*) (((char*) this) + sizeof (asf_video_stream_data));
	}
};

struct asf_error_correction_data {
	asf_byte data;
	asf_byte first;
	asf_byte second;
	
	bool FillInAll (ASFParser* parser);
	
	bool is_error_correction_present () { return data & 0x80; }
	bool is_opaque_data_present () { return data & 0x10; }
	int get_data_length () { return data & 0x0F; }
	int get_error_correction_length_type () { return (data & 0x60) >> 5; }
	char* tostring ()
	{
		char* result = (char*) g_malloc0 (sizeof (char) * 9);
		
		for (int i = 0; i < 8; i++) {
			result [7 - i] = (data & (1 << i)) ? '1' : '0';
		}
		
		return result;
	}
	
	asf_dword get_struct_size () {
		return !is_error_correction_present () ? 0 : (sizeof (data) + get_data_length ());
	}
};

struct asf_payload_parsing_information {
	asf_byte length_type_flags;
	asf_byte property_flags;
	
	asf_dword packet_length;	// dword, word, byte or inexistent.
	asf_dword sequence;			// dword, word, byte or inexistent.
	asf_dword padding_length;	// dword, word, byte or inexistent.
	
	asf_dword send_time;
	asf_word  duration;
	
	bool is_multiple_payloads_present () { return length_type_flags & 0x01; }
	int  get_sequence_type () { return (length_type_flags >> 1) & 0x03; }
	int  get_padding_length_type () { return (length_type_flags >> 3) & 0x03; }
	int  get_packet_length_type () { return (length_type_flags >> 5) & 0x03; }
	bool is_error_correction_present () { return length_type_flags & 0x80; }
	
	int  get_replicated_data_length_type () { return property_flags & 0x03; }
	int  get_offset_into_media_object_length_type () { return (property_flags >> 2) & 0x03; }
	int  get_media_object_number_length_type () { return (property_flags >> 4) & 0x03; }
	int  get_stream_number_length_type () { return (property_flags >> 6) & 0x03; }
	
	bool FillInAll (ASFParser* parser);
	
	
	// The following fields don't have a fixed offset.
	int  get_offset (asf_dword length_type) {
		return length_type == 0x11 ? 4 : length_type;
	}
	void* get_packet_length_offset () {
		return ((char*) this + 2);
	}
	void* get_sequence_offset () {
		return (char*) get_packet_length_offset () + get_offset (get_packet_length_type ());
	}
	void* get_padding_length_offset () {
		return (char*) get_sequence_offset () + get_offset (get_sequence_type ());
	}
	asf_dword* get_send_time_offset () {
		return (asf_dword*) ((char*) get_padding_length_offset () + get_offset (get_padding_length_type ()));
	}
	asf_word* get_duration_offset () {
		return (asf_word*) ((char*) get_send_time_offset () + sizeof (asf_dword)); 
	}
	
	asf_dword get_struct_size () {
		return sizeof (length_type_flags) 
			+ sizeof (property_flags)
			+ ASF_DECODE_PACKED_SIZE (get_packet_length_type ())
			+ ASF_DECODE_PACKED_SIZE (get_sequence_type ())
			+ ASF_DECODE_PACKED_SIZE (get_padding_length_type ())
			+ sizeof (send_time) 
			+ sizeof (duration);
		
	}
};

// struct with variable length fields (all but the first field)
struct asf_single_payload {
	asf_byte stream_number;
	bool is_key_frame;
	asf_dword media_object_number;
	asf_dword offset_into_media_object;
	asf_dword replicated_data_length;
	asf_byte* replicated_data;
	asf_dword payload_data_length;
	asf_byte* payload_data;
	
	bool FillInAll (ASFParser* parser, asf_error_correction_data* ecd, asf_payload_parsing_information ppi, asf_multiple_payloads* mp);
	
	gint64 get_presentation_time ()
	{
		if (replicated_data_length >= 8) {
			return * (((asf_dword*) replicated_data) + 1);
		}
		return -1;
	}
	
	asf_single_payload () 
	{
		stream_number = 0; media_object_number = 0; offset_into_media_object = 0;
		replicated_data_length = 0; replicated_data = 0; payload_data_length = 0;
		payload_data = 0;
	}
	~asf_single_payload ();
};

void asf_single_payload_dump (asf_single_payload* obj);

struct asf_multiple_payloads {
	asf_byte payload_flags;
	
	asf_single_payload** payloads;
	
	int get_number_of_payloads () { return payload_flags & 0x3F; }
	int get_payload_length_type () { return (payload_flags & 0xC0) >> 6; }
	
	bool FillInAll (ASFParser* parser, asf_error_correction_data* ecd, asf_payload_parsing_information ppi);
	asf_multiple_payloads () { payload_flags = 0; payloads = NULL; }
	
	~asf_multiple_payloads () 
	{
		if (payloads) {
			int i = -1;
			while (payloads [++i] != NULL)
				delete payloads [i];
			g_free (payloads);
			payloads = NULL;
		}
	}
	
	asf_byte get_stream_count ()
	{
		if (!payloads)
			return 0;
			
		asf_byte result = 0;
		int index = 0;
		while (index < get_number_of_payloads () && payloads [index] != NULL) {
			int current = payloads [index]->stream_number;
			result = current > result ? current : result;
			index++;
		}
		return result;
	}
	
	asf_dword get_stream_size (asf_byte stream_number)
	{
		if (!payloads)
			return 0;

		asf_dword result = 0;
		int index = 0;
		while (index < get_number_of_payloads () && payloads [index] != NULL) {
			if (payloads [index]->stream_number == stream_number) {
				result += payloads [index]->payload_data_length;
			}
			index++;
		}
		return result;
	}
	
	bool write_payload_data (asf_byte stream_number, unsigned char* destination, size_t max_size)
	{
		if (!payloads)
			return false;
		
		unsigned char* initial_destination = destination;
		//asf_dword result = 0;
		size_t size = 0;
		int index = 0;
		while (index < get_number_of_payloads () && payloads [index] != NULL) {
			if (payloads [index]->stream_number == stream_number) {
				size = payloads [index]->payload_data_length;
				g_assert ((size_t) (destination - initial_destination) + size  <= max_size);
				memcpy (destination, payloads [index]->payload_data, size);
				destination += size;
			}
			index++;
		}
		return true;
	}
	
	asf_dword get_presentation_time (asf_byte stream_number)
	{
		if (!payloads)
			return 0;
			
		int index = 0;
		while (index < get_number_of_payloads () && payloads [index] != NULL) {
			if (payloads [index]->stream_number == stream_number) {
				return payloads [index]->get_presentation_time ();
			}
			index++;
		}
		
		return 0;
	}
		
	bool is_key_frame (asf_dword stream_index)
	{
		if (payloads == NULL)
			return false;
			
		int index = 0;
		while (payloads [index] != NULL) {
			if (payloads [index]->stream_number == stream_index)
				return payloads [index]->is_key_frame;
			index++;
		}
		return false;
	}
	
	asf_single_payload** steal_payloads ()
	{
		asf_single_payload** result = payloads;
		payloads = NULL;
		return result;
	}

};

void asf_multiple_payloads_dump (asf_multiple_payloads* obj);
void asf_script_command_dump (ASFParser* parser, const asf_script_command* obj);

struct asf_object {
	asf_guid id;
	asf_qword size;
};
			

struct asf_header : public asf_object {
	asf_dword object_count;
	asf_byte reserved1;
	asf_byte reserved2;
};

enum asf_file_properties_flags {
	ASF_FILE_PROPERTIES_BROADCAST = 0x1,
	ASF_FILE_PROPERTIES_SEEKABLE = 0x2
};

struct asf_file_properties : public asf_object {
	asf_guid file_id;
	asf_qword file_size;
	asf_qword creation_date;
	asf_qword data_packet_count;
	asf_qword play_duration;
	asf_qword send_duration;
	asf_qword preroll;
	asf_dword flags;
	asf_dword min_packet_size;
	asf_dword max_packet_size;
	asf_dword max_bitrate;
	
	bool is_broadcast () 
	{
		return (flags & ASF_FILE_PROPERTIES_BROADCAST) == ASF_FILE_PROPERTIES_BROADCAST;
	}
	
	bool is_seekable ()
	{
		return (flags & ASF_FILE_PROPERTIES_SEEKABLE) == ASF_FILE_PROPERTIES_SEEKABLE;
	}
};

struct asf_stream_properties : public asf_object {
	asf_guid stream_type;
	asf_guid error_correction_type;
	asf_qword time_offset;
	asf_dword type_specific_data_length;
	asf_dword error_correction_data_length;
	asf_word flags;
	asf_dword reserved;

	// asf_byte type_specific_data;
	// asf_byte error_correction_data;

	bool is_audio ()
	{
		return asf_guid_compare (&stream_type, &asf_guids_media_audio);
	}
	
	bool is_video ()
	{
		return asf_guid_compare (&stream_type, &asf_guids_media_video);
	}

	asf_dword get_stream_number () 
	{
		return flags & 0x7F;
	}
	
	bool is_encrypted ()
	{
		return flags & 1 << 15;
	}
	
	WAVEFORMATEX* get_audio_data ()
	{
		if (size < sizeof (WAVEFORMATEX) + sizeof (asf_stream_properties))
			return NULL;
			
		return (WAVEFORMATEX*) (((char*) this) + sizeof (asf_stream_properties));
	}
	
	asf_video_stream_data* get_video_data ()
	{
		if (size < sizeof (asf_video_stream_data) + sizeof (asf_stream_properties))
			return NULL;
			
		return (asf_video_stream_data*) (((char*) this) + sizeof (asf_stream_properties));
	}
};

struct asf_header_extension : public asf_object {
	asf_guid reserved1;
	asf_word reserved2;
	asf_dword data_size;

	// data follows.
};

struct asf_codec_list : public asf_object {
	asf_guid reserved;
	asf_dword entries_count;
	
	// data follows
};

struct asf_script_command_entry {
	asf_dword pts;
	asf_word type_index;
	asf_word name_length;
	
	char* get_name ()
	{
		return wchar_to_utf8 (sizeof (asf_script_command_entry) + (char*) this, name_length);
	}
};

struct asf_script_command : public asf_object {
	asf_guid reserved;
	asf_word command_count;
	asf_word command_type_count;
	
	// data follows
	
	// A NULL terminated array of commands.
	// command_types: an array of strings, use g_free on the elements and the array.
	// result: Free the array with g_free (do NOT free each element).
	asf_script_command_entry** get_commands (ASFParser* parser, char*** command_types);
};

struct asf_marker_entry {
	asf_qword offset;
	asf_qword pts;
	asf_word entry_length;
	asf_dword send_time;
	asf_dword flags;
	asf_dword marker_description_length;
		
	char* get_marker_description ()
	{
		char* result = NULL;
		
		char* md = sizeof (asf_marker_entry) + (char*) this;
		result = wchar_to_utf8 (md, marker_description_length);
		
		return result;
	}
};

struct asf_marker : public asf_object {
	asf_guid reserved;
	asf_dword marker_count;
	asf_word reserved2;
	asf_word name_length;
	
	char* get_name ()
	{
		char* result = NULL;
		
		char* md = sizeof (asf_marker) + (char*) this;
		result = wchar_to_utf8 (md, name_length);
				
		return result;
	}
	
	asf_marker_entry* get_entry (guint32 index) 
	{
		asf_marker_entry* result = NULL;
		
		if (marker_count < index + 1)
			return NULL;
		
		asf_marker_entry* tmp = (asf_marker_entry*) (sizeof (asf_marker) + name_length + (char*) this);	
		for (guint32 i = 0; i < index; i++) {
			char* next = (char*) tmp;
			next += sizeof (asf_marker_entry);
			next += (tmp->marker_description_length * sizeof (guint16));
			tmp = (asf_marker_entry*) next;
		}
		result = tmp;
		
		return result;
	}
	
};

struct asf_bitrate_mutual_exclusion : public asf_object {
	asf_guid exclusion_type;
	asf_word stream_number_count;
	
	// data follows
};

struct asf_error_correction : public asf_object {
	asf_guid error_correction_type;
	asf_dword error_correction_data_length;
	
	// data follows
};

struct asf_content_description : public asf_object {
	asf_word title_length;
	asf_word author_length;
	asf_word copyright_length;
	asf_word description_length;
	asf_word rating_length;
};

struct asf_extended_content_description : public asf_object {
	asf_word content_descriptors_count;
};

struct asf_stream_bitrate_properties : public asf_object {
	asf_word bitrate_records_count;
};

struct asf_data : public asf_object {
	asf_guid file_id;
	asf_qword data_packet_count;
	asf_word reserved;
};


#pragma pack(pop)

#endif
