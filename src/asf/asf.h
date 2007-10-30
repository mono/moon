/*
 * asf.h: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_MOONLIGHT_H
#define _ASF_MOONLIGHT_H

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

typedef guint8 asf_byte;
typedef guint16 asf_wchar; 
typedef guint16 asf_word;
typedef guint32 asf_dword;
typedef guint64 asf_qword;

struct asf_guid;
struct asf_object;

class ASFParser;
class ASFFileSource;
class ASFSource;


#define ASF_ERROR_VAL(fail, ...) { fprintf (stderr, __VA_ARGS__); return fail; }
#define ASF_ERROR(...) ASF_ERROR_VAL(false, __VA_ARGS__)

#define ASF_CHECK_VAL(condition, val, ...) if (condition) { ASF_ERROR_VAL (val, __VA_ARGS__); }
#define ASF_CHECK(condition, ...) if (condition) { ASF_ERROR (__VA_ARGS__); }

#if log //|| true
#define ASF_LOG(...) printf (__VA_ARGS__)
#else
#define ASF_LOG(...)
#endif
#if dump //|| true
#define ASF_DUMP(...) printf (__VA_ARGS__)
#else
#define ASF_DUMP(...)
#endif

#include "asf-generated.h"
#include "asf-guids.h"
#include "asf-structures.h"
#include "asf-debug.h"

class ASFSource {
public: 
	virtual ~ASFSource ()
	{
		this->parser = NULL;
	}
	
	// Reads the number of the specified encoded length (0-3)
	// encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	// into the destionation.
	bool ReadEncoded (int encoded_length, asf_dword* destionation);	
	virtual bool Read (void* destination, size_t bytes);
protected:
	virtual bool ReadInternal (void* destination, size_t bytes) = 0;
public:
	virtual bool Seek (size_t offset, int mode) = 0;
	virtual bool Seek (size_t offset)
	{
		return Seek (offset, SEEK_CUR);
	}
	virtual bool Skip (int bytes)
	{
		//asf_byte dummy;
		//printf ("ASFSource::Skip (%i).\n", bytes);
		return Seek (bytes);
		/*while (bytes > 0) {
			if (!Read (&dummy, 1))
				return false;
			bytes--;
		}
		return true;*/
	}
	virtual gint64 Position () = 0;
protected:
	ASFSource (ASFParser* parser)
	{
		this->parser = parser;
	}
public:
	ASFParser* parser;
};

class ASFFileSource : public ASFSource {
public:
	ASFFileSource (ASFParser* parser, const char* filename);
	virtual ~ASFFileSource ();
	
	const char* GetFileName () { return filename; }
	
	virtual bool ReadInternal (void* destination, size_t bytes);
	virtual bool Seek (size_t offset, int mode);
	virtual gint64 Position () { return fd == NULL ? 0 : ftell (fd); }
	
private:
	char* filename;
	FILE* fd;
};

class ASFPacket {
public:
	ASFPacket ()
	{
		payloads = NULL;
		index = 0;
	}
	
	virtual ~ASFPacket ()
	{
		delete payloads;
		payloads = NULL;
	}
	
	gint32 GetPayloadCount ()
	{
		if (!payloads)
			return 0;
		return payloads->get_number_of_payloads ();
	}
	
	asf_single_payload* GetPayload (gint32 index /* 0 based */)
	{
		if (index >= 0 && index < GetPayloadCount ())
			return payloads->payloads [index];
			
		return NULL;
	}
	
	// Gets the pts of the first payload.
	int64_t GetPts (gint32 stream_number)
	{
		if (!payloads)
			return -1;
		
		asf_single_payload* first = GetFirstPayload (stream_number);
		if (!first)
			return -1;
			
		return first->get_presentation_time ();
	}
	
	asf_single_payload* GetFirstPayload (gint32 stream_number)
	{
		if (!payloads)
			return NULL;
			
		int index = 0;
		while (payloads->payloads [index] != NULL) {
			if (payloads->payloads [index]->stream_number == stream_number)
				return payloads->payloads [index];
			index++;
		}
		return NULL;
	}
	
	asf_multiple_payloads* payloads;
	
	gint32 index; // The index of this packet. -1 if not known.
	gint64 position; // The position of this packet. -1 if not known.		
};

struct ASFFrameReaderData {
	asf_single_payload* payload;
	ASFFrameReaderData* prev;
	ASFFrameReaderData* next;
	
	ASFFrameReaderData (asf_single_payload* load) 
	{
		payload = load;
		prev = NULL;
		next = NULL;
	}
	
	~ASFFrameReaderData ()
	{
		delete payload;
	}
};

class ASFFrameReader {
public:
	ASFFrameReader (ASFParser* parser);
	virtual ~ASFFrameReader ();
	
	// Can we seek?
	bool CanSeek () { return true; }
	
	// Seek to the frame with the provided pts 
	bool Seek (gint32 stream_number, gint64 pts);
	
	// Advance to the next frame
	bool Advance ();
	
	// Write the frame's data to a the destination
	bool Write (void* destination);
	
	// Information about the current frame
	guint64 Size () { return size; }
	bool IsKeyFrame () { return is_key_frame; }
	gint64 Pts () { return pts; }
	gint32 StreamNumber () { return stream_number; }
	
private:
	ASFParser* parser;
	
	gint32 current_packet_index;
	
	// The queue of payloads we've built.
	ASFFrameReaderData* first;
	ASFFrameReaderData* last;
	
	// A list of the payloads in the current frame
	asf_single_payload** payloads;
	gint32 payloads_size;
	
	// Information about the current frame.
	guint64 size;
	bool is_key_frame;
	gint64 pts;
	gint32 stream_number;
	
	// Reads another packet and fills data at the end 
	bool ReadMore ();
	
	void RemoveAll ()
	{
		ASFFrameReaderData* current = first, *next = NULL;
		while (current != NULL) {
			next = current->next;
			delete current;
			current = next;
		}
		first = NULL;
		last = NULL;
	}
	
	void Remove (ASFFrameReaderData* data)
	{
		if (data->prev != NULL)
			data->prev->next = data->next;
		
		if (data->next != NULL)
			data->next->prev = data->prev;
			
		if (data == first)
			first = data->next;
			
		if (data == last)
			last = data->prev;
			
		//delete data;
	}
};

class ASFParser {
public:
	ASFParser (const char* filename);
	virtual ~ASFParser ();
	
	bool ReadHeader ();
	bool ReadData ();
	// Reads the packet at the current position.
	bool ReadPacket (ASFPacket* packet);
	// Seeks to the packet index (as long as the packet index >= 0), then reads it.
	// If the packet index is < 0, then just read at the current position
	bool ReadPacket (ASFPacket* packet, gint32 packet_index); 
	
	asf_object* ReadObject (asf_object* guid);
	const char* GetLastError ();
	bool VerifyHeaderDataSize (gint32 size); // Verifies that the requested size is a size that can be inside the header.
	bool Malloc (void** mem, gint32 size); // Allocates the requested memory and verifies that the size can actually be contained within the header.
	void AddError (const char* err); // Makes a copy of the provided error string.
	void AddError (char* err); // Frees the provided error string.

	// Stream index from 1 to 127
	asf_stream_properties* GetStream (gint32 stream_index)
	{
		if (stream_index < 1 || stream_index > 127)
			return NULL;
			
		return stream_properties [stream_index - 1];
	}
	
	void SetStream (gint32 stream_index, asf_stream_properties* stream)
	{
		if (stream_index < 1 || stream_index > 127) {
			printf ("ASFParser::SetStream (%i, %p): Invalid stream index.\n", stream_index, stream);
			return;
		}
		stream_properties [stream_index - 1] = stream;
	}
	
	bool IsValidStream (gint32 stream_index)
	{
		return GetStream (stream_index) != NULL;
	}
	
	// Returns 0 on failure, otherwise the offset of the packet index.
	guint64 GetPacketOffset (gint32 packet_index)
	{
		if (packet_index < 0 || (gint32) packet_index >= (gint32) file_properties->data_packet_count) {
			AddError (g_strdup_printf ("ASFParser::GetPacketOffset (%i): Invalid packet index (there are %llu packets).", packet_index, file_properties->data_packet_count)); 
			return 0;
		}

		return packet_offset + packet_index * file_properties->min_packet_size;
	}
	
	gint32 GetPacketIndex (guint64 offset)
	{
		if (offset < packet_offset)
			return -1;
		if (offset > (guint64) packet_offset_end)
			return file_properties->data_packet_count;
		return (offset - packet_offset) / file_properties->min_packet_size;
	}
	
	// Returns the packet index where the desired pts is found.
	// Returns -1 on failure.
	gint32 GetPacketIndexOfPts (gint32 stream_number, gint64 pts)
	{
		int result = 0;
		ASFPacket* packet = NULL;
		
		// Read packets until we find the packet which has a pts
		// greater than the one we're looking for.
		
		while (ReadPacket (packet, result)) {
			gint64 current_pts = packet->GetPts (stream_number);
			if (current_pts < 0) // Can't read pts for some reason.
				return -1;
			if (current_pts > pts) // We've found the packet after the one we're looking for
				return result - 1; // return the previous one.
			result++;
		}
		
		return -1;
	}
	
	asf_header* GetHeader ()
	{
		return header;
	}
	
	asf_file_properties* GetFileProperties ()
	{ 
		return file_properties;
	}
	
	asf_object* GetHeaderObject (const asf_guid* guid)
	{
		int index = GetHeaderObjectIndex (guid);
		if (index >= 0) {
			return header_objects [index];
		} else {
			return NULL;
		}
	}

	// Searches the header objects for the specified guid
	// returns -1 if nothing is found.
	int GetHeaderObjectIndex (const asf_guid* guid, int start = 0)
	{
		int i = start;
		if (i < 0)
			return -1;
			
		while (header_objects [i] != NULL) {
			if (asf_guid_compare (guid, &header_objects [i]->id))
				return i;
		
			i++;
		}
		
		return -1;
	}
	
	asf_object* GetHeader (int index) 
	{
		if (index < 0)
			return NULL;
		return header_objects [index];
	}

	ASFSource* source;
	
	asf_object** header_objects;
	asf_header* header;
	asf_data* data;
	asf_file_properties* file_properties;
	asf_header_extension* header_extension;
	asf_stream_properties* stream_properties [127];
	asf_marker* marker;
	asf_script_command* script_command;
	
	guint64 data_offset; // location of data object
	guint64 packet_offset; // location of the beginning of the first packet
	guint64 packet_offset_end; // location of the end of the last packet
	
	GSList* errors;
};


#endif
