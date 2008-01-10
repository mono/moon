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

#if DEBUG && log //|| true
#define ASF_LOG(...) printf (__VA_ARGS__)
#else
#define ASF_LOG(...)
#endif
#if DEBUG && dump //|| true
#define ASF_DUMP(...) printf (__VA_ARGS__)
#else
#define ASF_DUMP(...)
#endif

#include "asf-generated.h"
#include "asf-guids.h"
#include "asf-structures.h"
#include "asf-debug.h"

#include "../pipeline.h"

class ASFSource {
protected:
	virtual bool ReadInternal (void* destination, size_t bytes) = 0;
	virtual bool SeekInternal (size_t offset, int mode) = 0;
	
public: 
	ASFSource (ASFParser* parser);
	virtual ~ASFSource ();
	
	// Reads the number of the specified encoded length (0-3)
	// encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	// into the destionation.
	bool ReadEncoded (gint32 encoded_length, guint32* destionation);	
	
	bool Read (void* destination, size_t bytes); // Reads the requested number of bytes into the destination

	bool Seek (size_t offset); // Seeks to the offset from the current position
	bool Seek (size_t offset, int mode); // Seeks to the offset, with the specified mode (SEEK_CUR, SEEK_END, SEEK_SET)
	
	virtual gint64 Position () = 0; // Returns the position within the source (may not apply if the source is not seekable)
	virtual bool CanSeek () = 0;
	virtual bool Eof () = 0;
	
	ASFParser* parser;
};

class ASFMediaSource : public ASFSource {
public:
	ASFMediaSource (ASFParser* parser, IMediaSource* source);
	
protected:
	virtual bool ReadInternal (void* destination, size_t bytes);
	virtual bool SeekInternal (size_t offset, int mode);	
	virtual gint64 Position ();
	virtual bool CanSeek ();
	virtual bool Eof ();
	
private:
	IMediaSource* source;
};

class ASFFileSource : public ASFSource {
public:
	ASFFileSource (ASFParser* parser, const char* filename);
	virtual ~ASFFileSource ();
	
	virtual gint64 Position () { return fd == NULL ? 0 : ftell (fd); }
	virtual bool CanSeek () { return true; }
	virtual bool Eof () { return fd == NULL ? false : feof (fd) != 0; }
	
	const char* GetFileName () { return filename; }
	
protected:
	virtual bool ReadInternal (void* destination, size_t bytes);
	virtual bool SeekInternal (size_t offset, int mode);
	
private:
	char* filename;
	FILE* fd;
};

class ASFPacket {
public:
	ASFPacket ();	
	virtual ~ASFPacket ();
	
	gint32 GetPayloadCount (); // Returns the number of payloads in this packet.
	asf_single_payload* GetPayload (gint32 index /* 0 based */);
	gint64 GetPts (gint32 stream_number /* 1 - 127 */); // Gets the pts of the first payload.
	asf_single_payload* GetFirstPayload (gint32 stream_number /* 1 - 127 */); // Gets the index first payload of the specified stream.
	
	asf_multiple_payloads* payloads; // The payloads in this packet

private:
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

/*
	The data in an ASF file has the following structure:
		Data
			Packets
				Payload(s)
					Chunks of Media objects
	
	The problem is that one chunk of "Media object data" can span several payloads (and packets),
	and the pieces may come unordered, like this:
	
	- first 25% of media object #1 for stream #1
	- first 25% of media object #1 for stream #2
	- first 25% of media object #1 for stream #3
	- middle 50% of media object #1 for stream #2
	- last 75% of media object #1 for stream #1
	=> we have now all the data for the first media object of stream #1
	
	This class implements a reader that allows the consumer to just call Advance() and then get the all data
	for each "Media object" (here called "Frame", since it's shorter, and in general it corresponds
	to a frame of video/audio, etc, even though the ASF spec states that it can be just about anything)
	in one convenient Write () call.
	
	We keep reading payloads until we have all the data for a frame, the payloads currently not wanted are 
	kept in a queue until the next Advance ().
*/

class ASFFrameReader {
public:
	ASFFrameReader (ASFParser* parser);
	virtual ~ASFFrameReader ();
	
	// Can we seek?
	bool CanSeek () { return true; }
	
	// Seek to the frame with the provided pts 
	bool Seek (gint32 stream_number, gint64 pts);
	
	// Advance to the next frame
	// Returns false if unsuccessful (if due to no more data, eof is set, otherwise some error occurred)
	bool Advance ();
	
	// Advance to the next frame of the specified stream number
	// stream_number = 0 means any stream
	bool Advance (gint32 stream_number);
	
	// Write the frame's data to a the destination
	bool Write (void* destination);
	
	// Information about the current frame
	guint64 Size () { return size; }
	bool IsKeyFrame () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->is_key_frame : false; }
	gint64 Pts () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->get_presentation_time () : 0; }
	gint32 StreamNumber () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->stream_number : 0; }
	
	void FindScriptCommandStream ();
	
private:
	ASFParser* parser;
	
	gint32 current_packet_index;
	gint32 script_command_stream_index;
	
	// The queue of payloads we've built.
	ASFFrameReaderData* first;
	ASFFrameReaderData* last;
	
	// A list of the payloads in the current frame
	asf_single_payload** payloads;
	gint32 payloads_size;
	
	// Information about the current frame.
	guint64 size;
	
	bool eof;
	
	bool ResizeList (gint32 size); // Resizes the list of payloads to the requested size. 
	bool ReadMore (); // Reads another packet and stuffs the payloads into our queue 
	void RemoveAll (); // Deletes the entire queue of payloads (and deletes every element)
	void Remove (ASFFrameReaderData* data); // Unlinks the payload from the queue and deletes it.
	
	void ReadScriptCommand (); // If the current frame is a script command, decodes it and calls the callback set in the parser.
};

class ASFParser {
private:
	void Initialize ();
	bool ReadData ();
	asf_object* ReadObject (asf_object* guid);
	void SetStream (gint32 stream_index, asf_stream_properties* stream);

public:
	ASFParser (const char* filename);
	ASFParser (ASFSource* source); // The parser takes ownership of the source and will delete it when the parser is deleted.
	virtual ~ASFParser ();
	
	bool ReadHeader (); // Reads the header of the asf file.
	bool ReadPacket (ASFPacket* packet); // Reads the packet at the current position.
	// Seeks to the packet index (as long as the packet index >= 0), then reads it.
	// If the packet index is < 0, then just read at the current position
	bool ReadPacket (ASFPacket* packet, gint32 packet_index); 
	
	bool VerifyHeaderDataSize (guint64 size); // Verifies that the requested size is a size that can be inside the header.
	void* Malloc (gint32 size); // Allocates the requested memory and verifies that the size can actually be contained within the header. Reports an Out of Memory error if the memory can't be allocated, and returns NULL
	void* MallocVerified (gint32 size); // Allocates the requested memory (no size checking), reports an Out of Memory error if the memory can't be allocated, and returns NULL

	// Error handling
	const char* GetLastError (); // Returns the last error (NULL if no errors have been reported)
	void AddError (const char* err); // Makes a copy of the provided error string.
	void AddError (char* err); // Frees the provided error string (allows you to do things like: AddError (g_strdup_printf ("error #%i", 2)))

	// Stream index: valid values range from 1 to 127
	// If the stream_index doesn't specify a valid stream (for whatever reason), NULL is returned.
	asf_stream_properties* GetStream (gint32 stream_index);
	
	// Checks if the stream_index (range 1 - 127) is a valid stream index in the asf file.
	bool IsValidStream (gint32 stream_index);
	
	// Returns 0 on failure, otherwise the offset of the packet index.
	guint64 GetPacketOffset (gint32 packet_index);
	
	// Returns the index of the packet at the specified offset (from the beginning of the file)
	gint32 GetPacketIndex (guint64 offset);
	
	// Searches the header objects for the specified guid
	// returns -1 if nothing is found.
	int GetHeaderObjectIndex (const asf_guid* guid, int start = 0);
	
	// Returns the packet index where the desired pts is found.
	// Returns -1 on failure.
	gint32 GetPacketIndexOfPts (gint32 stream_number, gint64 pts);

	// Field accessors
	
	asf_header* GetHeader ();
	asf_object* GetHeader (int index);
	asf_file_properties* GetFileProperties ();
	asf_object* GetHeaderObject (const asf_guid* guid);

	// This callback is called whenever a script command payload is encountered while decoding.
	typedef void embedded_script_command_callback (void* state, char* type, char* text, guint64 pts);
	void* embedded_script_command_state;
	embedded_script_command_callback* embedded_script_command;
	
	// The following fields are available only after ReadHeader is called.
	
	asf_header* header;
	asf_object** header_objects;
	asf_file_properties* file_properties;
	asf_header_extension* header_extension;
	asf_stream_properties* stream_properties [127];
	asf_marker* marker;
	asf_script_command* script_command;
	
	asf_data* data;
	guint64 data_offset; // location of data object
	guint64 packet_offset; // location of the beginning of the first packet
	guint64 packet_offset_end; // location of the end of the last packet

	ASFSource* source; // The source used to read data.
	
private:
	struct error {
		char* msg;
		error* next;
	};
	error* errors;
};


#endif
