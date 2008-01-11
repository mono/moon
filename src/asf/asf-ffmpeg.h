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
 
#ifdef INCLUDE_FFMPEG

#ifndef _ASF_FFMPEG_MOONLIGHT_H
#define _ASF_FFMPEG_MOONLIGHT_H

#if log_ffmpeg || true
#define FFMPEG_LOG(...) ASF_LOG (__VA_ARGS__)
#else
#define FFMPEG_LOG(...)
#endif

extern "C" 
{
#include "avio.h"
#include "avformat.h"
}

#include "asf.h"
#include <glib.h>

void AVFormatContext_dump (AVFormatContext* c);
void AVStream_dump (AVStream* s, int t);
void AVCodecContext_dump (AVCodecContext* s, int t);
void AVPacket_dump (AVPacket* s, int t);
void AVRational_dump (AVRational* s, int t);
void AVRational_dump (AVRational s, int t);
void AVFrame_dump (AVFrame* s, int t);
char* AVRational_tostring (AVRational s);
char* AVFrac_tostring (AVFrac s);

int ffmpeg_asf_probe(AVProbeData *pd);
int ffmpeg_asf_read_header(AVFormatContext *s, AVFormatParameters *ap);
int ffmpeg_asf_read_packet(AVFormatContext *s, AVPacket *pkt);
int ffmpeg_asf_read_close(AVFormatContext *s);
int ffmpeg_asf_read_seek(AVFormatContext *s, int stream_index, int64_t pts, int flags);
int64_t ffmpeg_asf_read_pts(AVFormatContext *s, int stream_index, int64_t *ppos, int64_t pos_limit);

ASFParser* ffmpeg_asf_get_last_parser ();

class FFMPEGParser;

struct MoonASFContext {
	FFMPEGParser* parser;
};

extern AVInputFormat ffmpeg_asf_demuxer;

class FFMPEGPacket : public ASFPacket {
public:
	FFMPEGPacket ()
	{
		current_payload = 0;
	}
	virtual ~FFMPEGPacket ()
	{
	}
	
	// Initialized to 0
	// Set to -1 when all payloads have been read.
	int current_payload;
};

class FFMPEGParser : public ASFParser {
public:
	FFMPEGParser (const char* filename) : ASFParser (filename)
	{
		memset (ffmpeg_stream_indices, -1, 128);
		reader = new ASFFrameReader (this);
	}
	
	virtual ~FFMPEGParser ()
	{
		delete reader;
		reader = NULL;
	}
	
	void AddStreamIndex (gint32 ffmpeg_stream_index, gint32 asf_stream_index)
	{
		// asf_stream_index range from 1 to 127
		if (asf_stream_index < 1 || asf_stream_index > 127) {
			FFMPEG_LOG ("FFMPEGParser::AddStreamIndex (%i, %i): Invalid asf stream index.\n", ffmpeg_stream_index, asf_stream_index);
			return;
		}
			
		FFMPEG_LOG ("FFMPEGParser::AddStreamIndex (%i, %i).\n", ffmpeg_stream_index, asf_stream_index);
		ffmpeg_stream_indices [asf_stream_index] = ffmpeg_stream_index;
	}
	
	gint32 ConvertStreamIndex (gint32 asf_stream_index)
	{
		if (asf_stream_index < 1 || asf_stream_index > 127) {
			FFMPEG_LOG ("FFMPEGParser::ConvertStreamIndex (%i, %i): Invalid asf stream index.\n", asf_stream_index, asf_stream_index);
			return -1;
		}
		
		FFMPEG_LOG ("FFMPEGParser::ConvertStreamIndex (%i): %i.\n", asf_stream_index, ffmpeg_stream_indices [asf_stream_index]);
		return ffmpeg_stream_indices [asf_stream_index];
	}
	
	gint32 ConvertFFMpegStreamIndex (gint32 ffmpeg_stream_index)
	{
		FFMPEG_LOG ("FFMPEGParser::ConvertFFMpegStreamIndex (%i).\n", ffmpeg_stream_index);
		for (int i = 1; i < 128; i++) {
			gint32 val = ffmpeg_stream_indices [i];
			FFMPEG_LOG ("FFMPEGParser::ConvertFFMPegStreamIndex (%i), i = %i, value = %i.\n", ffmpeg_stream_index, i,val);
			if (val == ffmpeg_stream_index) 
				return i;
		}
		return 0; // 0 is not a valid asf stream index.
	}
	
	gint32 ffmpeg_stream_indices [128];
	
	ASFFrameReader* reader;
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
	
	virtual bool SeekInternal (size_t offset, int mode)
	{
		//printf ("FFMPEGSource::Seek (%i, %i).\n", offset, mode);
		
		int64_t result = url_fseek (buffer, offset, mode);
		
		return result; // Is this correct?
	}
	
	virtual bool Eof () { return url_feof (buffer); }
	virtual bool CanSeek () { return true; }
	
	virtual gint64 Position () 
	{
		return buffer ? (buffer->pos - (buffer->buf_end - buffer->buf_ptr)) : 0;
	}
	
private:
	ByteIOContext* buffer;
};

#endif

#endif // INCLUDE_FFMPEG
