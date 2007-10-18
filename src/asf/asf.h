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

#include <config.h>
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

#include "asf-generated.h"
#include "asf-guids.h"
#include "asf-structures.h"

#define ASF_ERROR_VAL(fail, ...) { fprintf (stderr, __VA_ARGS__); return fail; }
#define ASF_ERROR(...) ASF_ERROR_VAL(false, __VA_ARGS__)

#define ASF_CHECK_VAL(condition, val, ...) if (condition) { ASF_ERROR_VAL (val, __VA_ARGS__); }
#define ASF_CHECK(condition, ...) if (condition) { ASF_ERROR (__VA_ARGS__); }

#if log// || true
#define ASF_LOG(...) printf (__VA_ARGS__)
#else
#define ASF_LOG(...)
#endif
#if dump// || true
#define ASF_DUMP(...) printf (__VA_ARGS__)
#else
#define ASF_DUMP(...)
#endif

/* Debug & tostring functions */ 
void  asf_printfree (char *message);

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
	
	asf_multiple_payloads* payloads;
	int64_t index;
		
};

class ASFParser {
public:
	ASFParser (const char* filename);
	virtual ~ASFParser ();
	
	bool ReadHeader ();
	bool ReadData ();
	bool ReadPacket ();
	bool ReadPacket (ASFPacket* packet);
	asf_object* ReadObject (asf_object* guid);
	const char* GetLastError ();
	void AddError (const char* err); // Makes a copy of the provided error string.
	void AddError (char* err); // Frees the provided error string.

	ASFSource* source;
	
	asf_header* header;
	int64_t data_offset; // location of data object
	int64_t packet_offset; // location of first packet
	int64_t packet_offset_end; // location of the end of the last packet
	asf_data* data;
	asf_file_properties* file_properties;
	
	asf_stream_properties* stream_properties [127];
	
	ASFPacket* current_packet;
	
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
	
	int64_t GetPacketOffset (int64_t packet_index)
	{
		return packet_offset + packet_index * file_properties->min_packet_size;
	}
	
	gint32 GetPacketIndex (int64_t offset)
	{
		if (offset < packet_offset)
			return -1;
		if (offset > packet_offset_end)
			return file_properties->data_packet_count;
		return (offset - packet_offset) / file_properties->min_packet_size;
	}
	
	ASFPacket* GetCurrentPacket ()
	{
		return current_packet;
	}
	
	void SetCurrentPacket (ASFPacket* packet)
	{
		current_packet = packet;
	}
	
	void FreeCurrentPacket ()
	{
		delete current_packet;
		current_packet = NULL;
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
	
	
private:
	asf_object** header_objects;
	char* last_error;	
};


#endif
