/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * asf.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
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

typedef guint8 asf_byte;
typedef guint16 asf_wchar; 
typedef guint16 asf_word;
typedef guint32 asf_dword;
typedef guint64 asf_qword;

struct asf_guid;
struct asf_object;

class ASFParser;
class ASFFrameReader;
class ASFReader;
struct ASFContext;

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

#include "pipeline.h"
#include "pipeline-asf.h"
#include "clock.h"
#include "error.h"

// according to http://msdn.microsoft.com/en-us/library/cc307965(VS.85).aspx the maximum size is 10 MB
#define ASF_OBJECT_MAX_SIZE	(10 * 1024 * 1024)

struct ASFContext {
	ASFParser *parser;
	IMediaSource *source;
};

/* @IncludeInKinds */
class ASFPacket : public EventObject {
private:
	gint64 position; // The position of this packet. -1 if not known.
	int index; // The index of this packet. -1 if not known.
	IMediaSource *source; // The source which is to be used for reading into this packet.
	ASFParser *parser;
	
protected:
	virtual ~ASFPacket ();
	
public:
	ASFPacket (ASFParser *parser, IMediaSource *source);
	
	asf_multiple_payloads *payloads; // The payloads in this packet
	
	int GetPayloadCount (); // Returns the number of payloads in this packet.
	asf_single_payload *GetPayload (int index /* 0 based */);
	
	guint64 GetPts (int stream_id /* 1 - 127 */); // Gets the pts of the first payload. 0 if no payloads.
	asf_single_payload *GetFirstPayload (int stream_id /* 1 - 127 */); // Gets the index first payload of the specified stream.
	
	IMediaSource *GetSource () { return source; }
	void SetSource (IMediaSource *source);
	MediaResult Read ();
};

class ASFReader {
private:
	ASFFrameReader *readers [128];
	ASFParser *parser;
	IMediaSource *source;
	ASFDemuxer *demuxer;
	// The index of the next packet to be read.
	guint64 next_packet_index;

	// Seeks to the specified pts directly on the source.
	MediaResult SeekToPts (guint64 pts);

public:
	ASFReader (ASFParser *parser, ASFDemuxer *demuxer);
	~ASFReader ();
	// Select the specified stream.
	// No streams are selected by default.
	void SelectStream (gint32 stream_index, bool value);
	// Returns the frame reader for the specified stream.
	// The stream must first have been selected using SelectStream.
	ASFFrameReader *GetFrameReader (gint32 stream_index);

	// Have we reached end of file?
	bool Eof ();

	// This method will seek to the first keyframe before the requested pts in all selected streams.
	// Note that the streams will probably be positioned at different pts after a seek (given that
	// for audio streams any frame is considered as a key frame, while for video there may be several
	// seconds between every key frame).
	MediaResult Seek (guint64 pts);

	// Resets all readers
	void ResetAll ();

	// Estimate the packet index of the specified pts.
	// Calls EstimatePacketIndexOfPts on all readers and returns the lowest value.
	guint64 EstimatePacketIndexOfPts (guint64 pts);

 	// Reads another packet and stuffs the payloads into our queue.
	// Called by the readers when they are out of data.
	MediaResult TryReadMore ();

	// Can we seek?
	bool CanSeek () { return true; }
	
	guint64 GetLastAvailablePacketIndex ();

};


struct ASFFrameReaderData {
	asf_single_payload *payload;
	ASFFrameReaderData *prev;
	ASFFrameReaderData *next;
	guint64 packet_index;

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

#define INVALID_START_PTS ((guint64) -1)

struct ASFFrameReaderIndex {
	guint64 start_pts;
	guint64 end_pts;
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
private:
	ASFDemuxer *demuxer;
	IMediaStream *stream;
	ASFParser *parser;
	ASFReader *reader;
	
	// The first pts that should be returned, any frames with pts below this one will be dropped.
	guint64 first_pts;

	// Only return key frames. Reset after we've returned a key frame.
	bool key_frames_only;
	bool buffer_underflow; // If the last time we tried to advance the buffer ran out of data
	int stream_number; // The stream this reader is reading for 
	bool positioned;
	
	// The queue of payloads we've built.
	ASFFrameReaderData *first;
	ASFFrameReaderData *last;
	
	// A list of the payloads in the current frame
	asf_single_payload **payloads;
	int payloads_size;
	
	// Information about the current frame.
	guint64 size;
	guint64 pts;
	
	// Index data
	guint32 index_size; // The number of items in the index.
	ASFFrameReaderIndex *index; // A table of ASFFrameReaderIndexes.
	
	bool ResizeList (int size); // Resizes the list of payloads to the requested size. 
	MediaResult ReadMore (); // Reads another packet and stuffs the payloads into our queue 
	void RemoveAll (); // Deletes the entire queue of payloads (and deletes every element)
	void Remove (ASFFrameReaderData *data); // Unlinks the payload from the queue and deletes it.
		
public:
	ASFFrameReader (ASFParser *parser, int stream_index, ASFDemuxer *demuxer, ASFReader *reader, IMediaStream *stream);
	~ASFFrameReader ();
	
	// Advance to the next frame
	MediaResult Advance (bool read_if_needed = true);
	
	// Write the frame's data to a the destination
	bool Write (void *dest);
	
	// Information about the current frame
	guint64 Size () { return size; }
	bool IsKeyFrame () { return (payloads_size > 0 && payloads [0] != NULL) ? payloads [0]->is_key_frame : false; }
	guint64 Pts () { return pts; }
	int StreamId () { return stream_number; }
	
	void AppendPayload (asf_single_payload *payload, guint64 packet_index);

	// Index, returns the packet index of where the frame is.
	// returns UINT32_MAX if not found in the index.
	guint32 FrameSearch (guint64 pts);

	gint64 EstimatePtsPosition (guint64 pts);
	guint64 EstimatePacketIndexOfPts (guint64 pts);

	// Adds the current frame to the index.
	void AddFrameIndex (guint64 packet_index);
	bool IsAudio ();
	bool IsAudio (int stream);
	void SetOnlyKeyFrames (); // Sets the key_frames_only flag to true
	void SetFirstPts (guint64); // Sets the first pts which is to be returned.
	void Reset ();

	IMediaStream *GetStream () { return stream; }
};

/* @IncludeInKinds */
class ASFParser : public EventObject {
private:
	ErrorEventArgs *error;
	bool header_read_successfully;
	
	void Initialize ();
	MediaResult ReadData ();
	asf_object *ReadObject (asf_object *guid);
	void SetStream (int stream_id, const asf_stream_properties *stream);
	void SetExtendedStream (int stream_id, const asf_extended_stream_properties *stream);
	Media *media;
	IMediaSource *source; // The source used to read data.
	
protected:
	virtual ~ASFParser ();
	
public:
	// The parser takes ownership of the source and will delete it when the parser is deleted.
	ASFParser (IMediaSource *source, Media *media);
	
	MediaResult ReadHeader ();
	// Reads a packet
	// In any case (failure or success), the position of the source
	// is set to the next packet.
	MediaResult ReadPacket (ASFPacket **packet);
	
	bool IsDrm ();

	// Seeks to the packet index (as long as the packet index >= 0), then reads it.
	// If the packet index is < 0, then just read at the current position
	MediaResult ReadPacket (ASFPacket **packet, int packet_index); 

	// Reads the number of the specified encoded length (0-3)
	// encoded length 3 = read 4 bytes, rest equals encoded length and #bytes
	// into the destionation.
	static bool ReadEncoded (IMediaSource *source, guint32 encoded_length, guint32 *dest);	

	// Verifies that the requested size is a size that can be inside the header.
	bool VerifyHeaderDataSize (guint32 size);
	
	// Allocates the requested memory (no size checking), reports
	// an Out of Memory error if the memory can't be allocated, and returns
	// NULL
	void *MallocVerified (guint32 size);
	
	// Allocates the requested memory and verifies that the size
	// can actually be contained within the header. Reports an Out of Memory
	// error if the memory can't be allocated, and returns NULL
	void *Malloc (guint32 size);
	
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
	const asf_extended_stream_properties *GetExtendedStream (int stream_index);
	
	// Checks if the stream_index (range 1 - 127) is a valid stream index in the asf file.
	bool IsValidStream (int stream_index);
	
	// Returns the sequential stream index (range 1 - 127) from the specified stream index (range 1 - 127)
	// Example: The file has streams #2, #5, #9, the sequential numbers would be 1, 2 and 3.
	int GetSequentialStreamNumber (int stream_index);
	
	// Returns 0 on failure, otherwise the offset of the packet index.
	gint64 GetPacketOffset (guint64 packet_index);
	
	// Returns the index of the packet at the specified offset (from the beginning of the file)
	guint64 GetPacketIndex (gint64 offset);
	
	// Searches the header objects for the specified guid
	// returns -1 if nothing is found.
	int GetHeaderObjectIndex (const asf_guid *guid, int start = 0);
	
	// The number of packets in the stream (0 if unknown).
	guint64 GetPacketCount ();
	
	guint32 GetPacketSize ();

	// The number of streams
	int GetStreamCount ();
	
	IMediaSource *GetSource () { return source; }
	void SetSource (IMediaSource *source);

	// Field accessors
	
	Media *GetMedia ();
	asf_header *GetHeader ();
	asf_object *GetHeader (int index);
	asf_file_properties *GetFileProperties ();
	asf_object *GetHeaderObject (const asf_guid *guid);
	
	// This callback is called whenever a script command payload is encountered while decoding.
	typedef void embedded_script_command_callback (void *state, char *type, char *text, guint64 pts);
	embedded_script_command_callback *embedded_script_command;
	void *embedded_script_command_state;
	
	// The following fields are available only after ReadHeader is called.
	
	asf_header *header;
	asf_object **header_objects;
	asf_file_properties *file_properties;
	asf_header_extension *header_extension;
	const asf_stream_properties *stream_properties[127];
	const asf_extended_stream_properties *extended_stream_properties[127];
	asf_marker *marker;
	asf_script_command *script_command;
	
	asf_data *data;
	gint64 data_offset; // location of data object
	gint64 packet_offset; // location of the beginning of the first packet
	gint64 packet_offset_end; // location of the end of the last packet
};


#endif /* _ASF_MOONLIGHT_H */
