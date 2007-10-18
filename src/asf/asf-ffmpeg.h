/*
 * asf-ffmpeg.h: 
 *   An asf demuxer for ffmpeg.
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_FFMPEG_MOONLIGHT_H
#define _ASF_FFMPEG_MOONLIGHT_H

#include <config.h>

extern "C" 
{
#include "avio.h"
#include "avformat.h"
}

#include "asf.h"
#include <glib.h>

int ffmpeg_asf_probe(AVProbeData *pd);
int ffmpeg_asf_read_header(AVFormatContext *s, AVFormatParameters *ap);
int ffmpeg_asf_read_packet(AVFormatContext *s, AVPacket *pkt);
int ffmpeg_asf_read_close(AVFormatContext *s);
int ffmpeg_asf_read_seek(AVFormatContext *s, int stream_index, int64_t pts, int flags);
int64_t ffmpeg_asf_read_pts(AVFormatContext *s, int stream_index, int64_t *ppos, int64_t pos_limit);

class FFMPEGParser;

struct MoonASFContext {
	FFMPEGParser* parser;
};

extern AVInputFormat ffmpeg_asf_demuxer;

class FFMPEGPacket : public ASFPacket {
public:
	FFMPEGPacket ()
	{
		current_stream = 0;
		stream_count = 0;
	}
	virtual ~FFMPEGPacket ()
	{
	}
	
	// Initialized to 0
	// Set to -1 when all streams have been read.
	int current_stream;
	int stream_count;
};

class FFMPEGParser : public ASFParser {
public:
	FFMPEGParser (const char* filename) : ASFParser (filename)
	{
		memset (ffmpeg_stream_indices, -1, 128);
		packets_read = 0;
	}
	
	virtual ~FFMPEGParser ()
	{
	}
	
	void AddStreamIndex (gint32 ffmpeg_stream_index, gint32 asf_stream_index)
	{
		// asf_stream_index range from 1 to 127
		if (asf_stream_index < 1 || asf_stream_index > 127) {
			printf ("FFMPEGParser::AddStreamIndex (%i, %i): Invalid asf stream index.\n", ffmpeg_stream_index, asf_stream_index);
			return;
		}
			
		printf ("FFMPEGParser::AddStreamIndex (%i, %i).\n", ffmpeg_stream_index, asf_stream_index);
		ffmpeg_stream_indices [asf_stream_index] = ffmpeg_stream_index;
	}
	
	gint32 ConvertStreamIndex (gint32 asf_stream_index)
	{
		if (asf_stream_index < 1 || asf_stream_index > 127) {
			printf ("FFMPEGParser::ConvertStreamIndex (%i, %i): Invalid asf stream index.\n", asf_stream_index, asf_stream_index);
			return -1;
		}
		
		printf ("FFMPEGParser::ConvertStreamIndex (%i): %i.\n", asf_stream_index, ffmpeg_stream_indices [asf_stream_index]);
		return ffmpeg_stream_indices [asf_stream_index];
	}
	
	gint32 ConvertFFMpegStreamIndex (gint32 ffmpeg_stream_index)
	{
		printf ("FFMPEGParser::ConvertFFMpegStreamIndex (%i).\n", ffmpeg_stream_index);
		for (int i = 1; i < 128; i++) {
			gint32 val = ffmpeg_stream_indices [i];
			printf ("FFMPEGParser::ConvertFFMPegStreamIndex (%i), i = %i, value = %i.\n", ffmpeg_stream_index, i,val);
			if (val == ffmpeg_stream_index) 
				return i;
		}
		return 0; // 0 is not a valid asf stream index.
	}
	
	gint32 ffmpeg_stream_indices [128];
	
	int64_t packets_read;
};

class FFMPEGSource : public ASFSource {
public:
	FFMPEGSource (ASFParser* parser, ByteIOContext* buffer)
		: ASFSource (parser)
	{
		this->buffer = buffer;
	}
	
	virtual ~FFMPEGSource ()
	{
		buffer = NULL;
	}
	
	ByteIOContext* GetBuffer ()
	{
		return buffer;
	}
	
	virtual bool ReadInternal (void* destination, size_t bytes)
	{
		int result;
		result = get_buffer (buffer, (unsigned char*) destination, (int) bytes);
		return result != 0; // Is this correct
	}
	
	virtual bool Seek (size_t offset, int mode)
	{
		//printf ("FFMPEGSource::Seek (%i, %i).\n", offset, mode);
		
		int64_t result = url_fseek (buffer, offset, mode);
		
		return result; // Is this correct?
	}
	
	virtual gint64 Position () 
	{
		return buffer ? (buffer->pos - (buffer->buf_end - buffer->buf_ptr)) : 0;
	}
	
private:
	ByteIOContext* buffer;
};

#endif
