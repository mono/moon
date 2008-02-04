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

typedef uint8_t asf_byte;
typedef uint16_t asf_wchar; 
typedef uint16_t asf_word;
typedef uint32_t asf_dword;
typedef uint64_t asf_qword;

struct asf_guid;
struct asf_object;

class ASFParser;
class ASFSource;

#define ASF_ERROR_VAL(fail, ...) { fprintf (stderr, __VA_ARGS__); return fail; }
#define ASF_ERROR(...) ASF_ERROR_VAL(false, __VA_ARGS__)

#define ASF_CHECK_VAL(condition, val, ...) if (condition) { ASF_ERROR_VAL (val, __VA_ARGS__); }
#define ASF_CHECK(condition, ...) if (condition) { ASF_ERROR (__VA_ARGS__); }

#if DEBUG && log //|| true
#define ASF_LOG(...) printf (__VA_ARGS__)
#define ASF_LOGGING
#else
#define ASF_LOG(...)
#endif
#if DEBUG && dump //|| true
#define ASF_DUMP(...) printf (__VA_ARGS__)
#define ASF_DUMPING
#else
#define ASF_DUMP(...)
#endif

#include "asf-generated.h"
#include "asf-guids.h"
#include "asf-structures.h"
#include "asf-debug.h"

#include "../pipeline.h"
#include "../clock.h"

class ASFSource {
protected:
	virtual bool ReadInternal (void *buf, uint32_t n) = 0;
	virtual bool SeekInternal (int64_t offset, int mode) = 0;
	
public: 
	ASFSource (ASFParser *parser);
	virtual ~ASFSource ();
	
	// Reads the number of the specified encoded length (0-3)
	// encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	// into the destionation.
	bool ReadEncoded (uint32_t encoded_length, uint32_t *dest);	
	
	bool Read (void *buf, uint32_t n); // Reads the requested number of bytes into the destination
	
	bool Seek (int64_t offset); // Seeks to the offset from the current position
	bool Seek (int64_t offset, int mode); // Seeks to the offset, with the specified mode (SEEK_CUR, SEEK_END, SEEK_SET)
	
	virtual int64_t Position () = 0; // Returns the position within the source (may not apply if the source is not seekable)
	virtual bool CanSeek () = 0;
	virtual bool Eof () = 0;
	
	ASFParser *parser;
};

class ASFMediaSource : public ASFSource {
	IMediaSource *source;

protected:
	virtual bool ReadInternal (void *buf, uint32_t n);
	virtual bool SeekInternal (int64_t offset, int mode);	
	virtual int64_t Position ();
	virtual bool CanSeek ();
	virtual bool Eof ();
	
public:
	ASFMediaSource (ASFParser *parser, IMediaSource *source);
};

class ASFPacket {
	int64_t position; // The position of this packet. -1 if not known.
	int index; // The index of this packet. -1 if not known.
	
public:
	ASFPacket ();	
	virtual ~ASFPacket ();
	
	asf_multiple_payloads *payloads; // The payloads in this packet
	
	int GetPayloadCount (); // Returns the number of payloads in this packet.
	asf_single_payload *GetPayload (int index /* 0 based */);
	
	uint64_t GetPts (int stream_id /* 1 - 127 */); // Gets the pts of the first payload. 0 if no payloads.
	asf_single_payload *GetFirstPayload (int stream_id /* 1 - 127 */); // Gets the index first payload of the specified stream.
};

struct ASFFrameReaderData {
	asf_single_payload *payload;
	ASFFrameReaderData *prev;
	ASFFrameReaderData *next;
	
	ASFFrameReaderData (asf_single_payload *load) 
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

#define INVALID_START_PTS ((uint64_t) -1)

struct ASFFrameReaderIndex {
	uint64_t start_pts;
	uint64_t end_pts;
};
/*
 *	The data in an ASF file has the following structure:
 *		Data
 *			Packets
 *				Payload(s)
 *					Chunks of Media objects
 *	
 *	The problem is that one chunk of "Media object data" can span several payloads (and packets),
 *	and the pieces may come unordered, like this:
 *	
 *	- first 25% of media object #1 for stream #1
 *	- first 25% of media object #1 for stream #2
 *	- first 25% of media object #1 for stream #3
 *	- middle 50% of media object #1 for stream #2
 *	- last 75% of media object #1 for stream #1
 *	=> we have now all the data for the first media object of stream #1
 *	
 *	This class implements a reader that allows the consumer to just call Advance() and then get the all data
 *	for each "Media object" (here called "Frame", since it's shorter, and in general it corresponds
 *	to a frame of video/audio, etc, even though the ASF spec states that it can be just about anything)
 *	in one convenient Write () call.
 *	
 *	We keep reading payloads until we have all the data for a frame, the payloads currently not wanted are 
 *	kept in a queue until the next Advance ().
 */

class ASFFrameReader {
	IMediaDemuxer *demuxer;
	ASFParser *parser;
	
	// The first pts that should be returned, any frames with pts below this one will be dropped.
	uint64_t first_pts;
	// Only return key frames. Reset after we've returned a key frame.
	bool key_frames_only;
	int stream_number; // The stream this reader is reading for 
	
	uint64_t current_packet_index; // The index of the next packet that will be read when ReadMore is called.
	int32_t script_command_stream_index;
	
	// The queue of payloads we've built.
	ASFFrameReaderData *first;
	ASFFrameReaderData *last;
	
	// A list of the payloads in the current frame
	asf_single_payload **payloads;
	int payloads_size;
	
	// Information about the current frame.
	uint64_t size;
	uint64_t pts;
	
	// Index data
	uint32_t index_size; // The number of items in the index.
	ASFFrameReaderIndex *index; // A table of ASFFrameReaderIndexes.
	
	bool ResizeList (int size); // Resizes the list of payloads to the requested size. 
	MediaResult ReadMore (); // Reads another packet and stuffs the payloads into our queue 
	void RemoveAll (); // Deletes the entire queue of payloads (and deletes every element)
	void Remove (ASFFrameReaderData *data); // Unlinks the payload from the queue and deletes it.
	
	void ReadScriptCommand (); // If the current frame is a script command, decodes it and calls the callback set in the parser.
	
public:
	ASFFrameReader (ASFParser *parser, int stream_index, IMediaDemuxer *demuxer);
	virtual ~ASFFrameReader ();
	
	// Can we seek?
	bool CanSeek () { return true; }
	
	// Seek to the frame with the provided pts 
	bool Seek (uint64_t pts);
	
	// Advance to the next frame
	MediaResult Advance ();
	
	// Write the frame's data to a the destination
	bool Write (void *dest);
	
	// Information about the current frame
	uint64_t Size () { return size; }
	bool IsKeyFrame () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->is_key_frame : false; }
	uint64_t Pts () { return pts; }
	int StreamId () { return stream_number; }
	bool Eof ();
	void FindScriptCommandStream ();
	
	// Index, returns the packet index of where the frame is.
	// returns UINT32_MAX if not found in the index.
	uint32_t FrameSearch (uint64_t pts);

	int64_t GetPositionOfPts (uint64_t pts, bool *estimate);
	uint64_t GetPacketIndexOfPts (uint64_t pts, bool *estimate);

	// Adds the current frame to the index.
	void AddFrameIndex (uint64_t packet_index);
	bool IsAudio ();
	bool IsAudio (int stream);
};

class ASFParser {
private:
	struct error {
		error *next;
		char *msg;
	};
	
	error *errors;
	
	void Initialize ();
	bool ReadData ();
	asf_object *ReadObject (asf_object *guid);
	void SetStream (int stream_id, asf_stream_properties *stream);
	Media *media;
	
public:
	// The parser takes ownership of the source and will delete it when the parser is deleted.
	ASFParser (ASFSource *source, Media *media);
	virtual ~ASFParser ();
	
	bool ReadHeader ();
	bool ReadPacket (ASFPacket *packet);
	
	// Seeks to the packet index (as long as the packet index >= 0), then reads it.
	// If the packet index is < 0, then just read at the current position
	bool ReadPacket (ASFPacket *packet, int packet_index); 
	
	// Verifies that the requested size is a size that can be inside the header.
	bool VerifyHeaderDataSize (uint32_t size);
	
	// Allocates the requested memory (no size checking), reports
	// an Out of Memory error if the memory can't be allocated, and returns
	// NULL
	void *MallocVerified (uint32_t size);
	
	// Allocates the requested memory and verifies that the size
	// can actually be contained within the header. Reports an Out of Memory
	// error if the memory can't be allocated, and returns NULL
	void *Malloc (uint32_t size);
	
	// Error handling
	const char *GetLastError ();
	void AddError (const char *err);
	void AddError (char *err);
	
	// Stream index: valid values range from 1 to 127
	// If the stream_index doesn't specify a valid stream (for whatever reason), NULL is returned.
	asf_stream_properties *GetStream (int stream_index);
	
	// Checks if the stream_index (range 1 - 127) is a valid stream index in the asf file.
	bool IsValidStream (int stream_index);
	
	// Returns the sequential stream index (range 1 - 127) from the specified stream index (range 1 - 127)
	// Example: The file has streams #2, #5, #9, the sequential numbers would be 1, 2 and 3.
	int GetSequentialStreamNumber (int stream_index);
	
	// Returns 0 on failure, otherwise the offset of the packet index.
	int64_t GetPacketOffset (uint64_t packet_index);
	
	// Returns the index of the packet at the specified offset (from the beginning of the file)
	uint64_t GetPacketIndex (int64_t offset);
	
	// Searches the header objects for the specified guid
	// returns -1 if nothing is found.
	int GetHeaderObjectIndex (const asf_guid *guid, int start = 0);
	
	// Returns the packet index where the desired pts is found.
	uint64_t GetPacketIndexOfPts (int stream_id, uint64_t pts);
	
	// The number of packets in the stream (0 if unknown).
	uint64_t GetPacketCount ();
	
	// The number of streams
	int GetStreamCount ();
	
	// Field accessors
	
	Media *GetMedia ();
	asf_header *GetHeader ();
	asf_object *GetHeader (int index);
	asf_file_properties *GetFileProperties ();
	asf_object *GetHeaderObject (const asf_guid *guid);
	
	// This callback is called whenever a script command payload is encountered while decoding.
	typedef void embedded_script_command_callback (void *state, char *type, char *text, uint64_t pts);
	embedded_script_command_callback *embedded_script_command;
	void *embedded_script_command_state;
	
	// The following fields are available only after ReadHeader is called.
	
	asf_header *header;
	asf_object **header_objects;
	asf_file_properties *file_properties;
	asf_header_extension *header_extension;
	asf_stream_properties *stream_properties[127];
	asf_marker *marker;
	asf_script_command *script_command;
	
	asf_data *data;
	int64_t data_offset; // location of data object
	int64_t packet_offset; // location of the beginning of the first packet
	int64_t packet_offset_end; // location of the end of the last packet
	
	ASFSource *source; // The source used to read data.
};


#endif /* _ASF_MOONLIGHT_H */
