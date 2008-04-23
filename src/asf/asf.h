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
class ASFFrameReader;
class ASFReader;
class ASFContext;

#define ASF_ERROR_VAL(fail, ...) { fprintf (stderr, __VA_ARGS__); return fail; }
#define ASF_ERROR(...) ASF_ERROR_VAL(false, __VA_ARGS__)

#define ASF_CHECK_VAL(condition, val, ...) if (condition) { ASF_ERROR_VAL (val, __VA_ARGS__); }
#define ASF_CHECK(condition, ...) if (condition) { ASF_ERROR (__VA_ARGS__); }

#define ASF_LOG(...)// printf (__VA_ARGS__)
//#define ASF_LOGGING
#define ASF_LOG_ERROR(...) printf (__VA_ARGS__)
#define ASF_DUMP(...)// printf (__VA_ARGS__)
//#define ASF_DUMPING

#include "asf-generated.h"
#include "asf-guids.h"
#include "asf-structures.h"
#include "asf-debug.h"

#include "../pipeline.h"
#include "../clock.h"
#include "../error.h"

struct ASFContext {
	ASFParser *parser;
	IMediaSource *source;
};

class ASFPacket {
private:
	int64_t position; // The position of this packet. -1 if not known.
	int index; // The index of this packet. -1 if not known.
	IMediaSource *source; // The source which is to be used for reading into this packet.
	
public:
	ASFPacket ();	
	ASFPacket (IMediaSource *source);
	virtual ~ASFPacket ();
	
	asf_multiple_payloads *payloads; // The payloads in this packet
	
	int GetPayloadCount (); // Returns the number of payloads in this packet.
	asf_single_payload *GetPayload (int index /* 0 based */);
	
	uint64_t GetPts (int stream_id /* 1 - 127 */); // Gets the pts of the first payload. 0 if no payloads.
	asf_single_payload *GetFirstPayload (int stream_id /* 1 - 127 */); // Gets the index first payload of the specified stream.
	
	IMediaSource *GetSource () { return source; }
};

class ASFReader {
private:
	ASFFrameReader *readers [128];
	ASFParser *parser;
	IMediaSource *source;
	IMediaDemuxer *demuxer;
	bool positioned;
	// The index of the next packet to be read.
	uint64_t next_packet_index;
	int last_reader; // The index of the last reader which was given a payload

	// Seeks to the specified pts directly on the source.
	bool SeekToPts (uint64_t pts);

public:
	ASFReader (ASFParser *parser, IMediaDemuxer *demuxer);
	~ASFReader ();
	// Select the specified stream.
	// No streams are selected by default.
	void SelectStream (int32_t stream_index, bool value);
	// Returns the frame reader for the specified stream.
	// The stream must first have been selected using SelectStream.
	ASFFrameReader *GetFrameReader (int32_t stream_index);

	// Have we reached end of file?
	bool Eof ();

	// This method will seek to the first keyframe before the requested pts in all selected streams.
	// Note that the streams will probably be positioned at different pts after a seek (given that
	// for audio streams any frame is considered as a key frame, while for video there may be several
	// seconds between every key frame).
	bool Seek (uint64_t pts);

	// Resets all readers
	void ResetAll ();

	// Estimate the packet index of the specified pts.
	// Calls EstimatePacketIndexOfPts on all readers and returns the lowest value.
	uint64_t EstimatePacketIndexOfPts (uint64_t pts);

 	// Reads another packet and stuffs the payloads into our queue.
	// Called by the readers when they are out of data.
	MediaResult ReadMore ();

	// Can we seek?
	bool CanSeek () { return true; }
	
	uint64_t GetLastAvailablePacketIndex ();
	uint64_t GetLastAvailablePts ();

};


struct ASFFrameReaderData {
	asf_single_payload *payload;
	ASFFrameReaderData *prev;
	ASFFrameReaderData *next;
	uint64_t packet_index;

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
 *	The problem is that one chunk of "Media object data" can span several payloads (and packets).
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
private:
	IMediaDemuxer *demuxer;
	ASFParser *parser;
	ASFReader *reader;
	MarkerStream *marker_stream;
	
	// The first pts that should be returned, any frames with pts below this one will be dropped.
	uint64_t first_pts;

	// Only return key frames. Reset after we've returned a key frame.
	bool key_frames_only;
	int stream_number; // The stream this reader is reading for 
	bool positioned;
	bool last_payload; // If the last payload in this reader is the last payload in the entire file.
	
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
		
public:
	ASFFrameReader (ASFParser *parser, int stream_index, IMediaDemuxer *demuxer, ASFReader *reader);
	~ASFFrameReader ();
	
	// Advance to the next frame
	MediaResult Advance ();
	
	// Write the frame's data to a the destination
	bool Write (void *dest);
	
	// Information about the current frame
	uint64_t Size () { return size; }
	bool IsKeyFrame () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->is_key_frame : false; }
	uint64_t Pts () { return pts; }
	int StreamId () { return stream_number; }
	
	void AppendPayload (asf_single_payload *payload, uint64_t packet_index);

	// Index, returns the packet index of where the frame is.
	// returns UINT32_MAX if not found in the index.
	uint32_t FrameSearch (uint64_t pts);

	int64_t EstimatePtsPosition (uint64_t pts);
	uint64_t EstimatePacketIndexOfPts (uint64_t pts);

	// Adds the current frame to the index.
	void AddFrameIndex (uint64_t packet_index);
	bool IsAudio ();
	bool IsAudio (int stream);
	void SetOnlyKeyFrames (); // Sets the key_frames_only flag to true
	void SetFirstPts (uint64_t); // Sets the first pts which is to be returned.
	void Reset ();
	
	void SetMarkerStream (MarkerStream *stream);
	MarkerStream *GetMarkerStream () { return marker_stream; }
	void SetLastPayload (bool value) { last_payload = value; }
};

class ASFParser {
private:	
	MediaErrorEventArgs *error;
	
	void Initialize ();
	bool ReadData ();
	asf_object *ReadObject (asf_object *guid);
	void SetStream (int stream_id, const asf_stream_properties *stream);
	Media *media;
	IMediaSource *source; // The source used to read data.
	
public:
	// The parser takes ownership of the source and will delete it when the parser is deleted.
	ASFParser (IMediaSource *source, Media *media);
	virtual ~ASFParser ();
	
	bool ReadHeader ();
	// Reads a packet
	// In any case (failure or success), the position of the source
	// is set to the next packet.
	MediaResult ReadPacket (ASFPacket *packet);
	
	// Seeks to the packet index (as long as the packet index >= 0), then reads it.
	// If the packet index is < 0, then just read at the current position
	MediaResult ReadPacket (ASFPacket *packet, int packet_index); 
	
	// Reads the number of the specified encoded length (0-3)
	// encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	// into the destionation.
	static bool ReadEncoded (IMediaSource *source, uint32_t encoded_length, uint32_t *dest);	

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
	ErrorEventArgs *GetLastError ();
	const char *GetLastErrorStr ();
	void AddError (char *msg);
	void AddError (const char *msg);
	void AddError (MediaResult code, char *msg);
	void AddError (MediaResult code, const char *msg);
	
	// Stream index: valid values range from 1 to 127
	// If the stream_index doesn't specify a valid stream (for whatever reason), NULL is returned.
	const asf_stream_properties *GetStream (int stream_index);
	
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
	
	uint32_t GetPacketSize ();

	// The number of streams
	int GetStreamCount ();
	
	IMediaSource *GetSource () { return source; }

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
	const asf_stream_properties *stream_properties[127];
	asf_marker *marker;
	asf_script_command *script_command;
	
	asf_data *data;
	int64_t data_offset; // location of data object
	int64_t packet_offset; // location of the beginning of the first packet
	int64_t packet_offset_end; // location of the end of the last packet
};


#endif /* _ASF_MOONLIGHT_H */
