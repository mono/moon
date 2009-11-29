/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mp3.cpp: Pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include "mp3.h"
#include "pipeline.h"
#include "clock.h"
#include "debug.h"

//
// Relevant links to documentation:
//  http://www.codeproject.com/KB/audio-video/mpegaudioinfo.aspx
//  http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
//  http://www.compuphase.com/mp3/sta013.htm
//

/*
 * MPEG Audio Demuxer
 */

static int mpeg1_bitrates[3][15] = {
	/* version 1, layer 1 */
	{ 0, 32000, 48000, 56000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000 },
	/* version 1, layer 2 */
	{ 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000 },
	/* version 1, layer 3 */
	{ 0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000 },
};

static int mpeg2_bitrates[3][15] = {
	/* version 2, layer 1 */
	{ 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000 },
	/* version 2, layer 2 */
	{ 0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000 },
	/* version 2, layer 3 */
	{ 0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000 }
};

static bool
mpeg_parse_bitrate (MpegFrameHeader *mpeg, guint8 byte)
{
	int i = (byte & 0xf0) >> 4;
	
	if (i > 14)
		return false;
	
	if (mpeg->version == 1)
		mpeg->bit_rate = mpeg1_bitrates[mpeg->layer - 1][i];
	else
		mpeg->bit_rate = mpeg2_bitrates[mpeg->layer - 1][i];
	
	return true;
}

static guint8
mpeg_encode_bitrate (MpegFrameHeader *mpeg, int bit_rate)
{
	int i;
	
	if (mpeg->version == 1) {
		for (i = 1; i < 15; i++) {
			if (mpeg1_bitrates[mpeg->layer - 1][i] == bit_rate)
				break;
		}
	} else {
		for (i = 1; i < 15; i++) {
			if (mpeg2_bitrates[mpeg->layer - 1][i] == bit_rate)
				break;
		}
	}
	
	if (i == 15)
		return 0;
	
	return ((i << 4) & 0xf0);
}

static int mpeg_samplerates[3][3] = {
	{ 44100, 48000, 32000 },  // version 1
	{ 22050, 24000, 16000 },  // version 2
	{ 11025, 12000,  8000 }   // version 2.5
};

static bool
mpeg_parse_samplerate (MpegFrameHeader *mpeg, guint8 byte)
{
	int i = (byte >> 2) & 0x03;
	
	if (i > 2)
		return false;
	
	mpeg->sample_rate = mpeg_samplerates[mpeg->version - 1][i];
	
	return true;
}

static bool
mpeg_parse_channels (MpegFrameHeader *mpeg, guint8 byte)
{
	int mode = (byte >> 6) & 0x03;
	
	switch (mode) {
	case 0: /* stereo */
		mpeg->channels = 2;
		break;
	case 1: /* joint stereo */
		mpeg->channels = 2;
		break;
	case 2: /* dual channel (2 mono channels) */
		mpeg->channels = 2;
		break;
	case 3: /* mono */
		mpeg->channels = 1;
		break;
	}
	
	mpeg->intensity = (byte & 0x20) ? 1 : 0;
	mpeg->ms = (byte & 0x10) ? 1 : 0;
	
	return true;
}

bool
mpeg_parse_header (MpegFrameHeader *mpeg, const guint8 *buffer)
{
	if (!is_mpeg_header (buffer))
		return false;
	
	// extract the MPEG version
	switch ((buffer[1] >> 3) & 0x03) {
	case 0: /* MPEG Version 2.5 */
		mpeg->version = 3;
		break;
	case 1: /* reserved */
		return false;
	case 2: /* MPEG Version 2 */
		mpeg->version = 2;
		break;
	case 3: /* MPEG Version 1 */
		mpeg->version = 1;
		break;
	}
	
	// extract the MPEG layer
	switch ((buffer[1] >> 1) & 0x03) {
	case 1:
		mpeg->layer = 3;
		break;
	case 2:
		mpeg->layer = 1;
		break;
	case 3:
		mpeg->layer = 2;
		break;
	default:
		// invalid layer
		return false;
	}
	
	// protection (via 16bit crc) bit
	mpeg->prot = (buffer[1] & 0x01) ? 1 : 0;
	
	// extract the bit rate
	if (!mpeg_parse_bitrate (mpeg, buffer[2]))
		return false;
	
	// extract the sample rate
	if (!mpeg_parse_samplerate (mpeg, buffer[2]))
		return false;
	
	// check if the frame is padded
	mpeg->padded = (buffer[2] & 0x02) ? 1 : 0;
	
	// extract the channel mode */
	if (!mpeg_parse_channels (mpeg, buffer[3]))
		return false;
	
	mpeg->copyright = (buffer[3] & 0x08) ? 1 : 0;
	mpeg->original = (buffer[3] & 0x04) ? 1 : 0;
	
	return true;
}

static int mpeg_block_sizes[3][3] = {
	{ 384, 1152, 1152 },  // version 1
	{ 384, 1152,  576 },  // version 2
	{ 384, 1152,  576 }   // version 2.5
};

#define mpeg_block_size(mpeg) mpeg_block_sizes[(mpeg)->version - 1][(mpeg)->layer - 1]

double
mpeg_frame_length (MpegFrameHeader *mpeg, bool xing)
{
	double len;
	
	// calculate the frame length
	if (mpeg->layer == 1)
		len = (((12 * mpeg->bit_rate) / (double) mpeg->sample_rate) + mpeg->padded) * 4;
	else if (mpeg->version == 1)
		len = ((144 * mpeg->bit_rate) / (double) mpeg->sample_rate) + mpeg->padded;
	else
		len = ((72 * mpeg->bit_rate) / (double) mpeg->sample_rate) + mpeg->padded;
	
	return len;
}

#define MPEG_FRAME_LENGTH_MAX ((((144 * 160000) / 8000) + 1) + 2)

#define mpeg_frame_size(mpeg) (((mpeg)->bit_rate * (mpeg)->channels * mpeg_block_size (mpeg)) / (mpeg)->sample_rate)

static guint64
mpeg_frame_duration (MpegFrameHeader *mpeg)
{
	guint64 result = ((guint64) mpeg_block_size (mpeg)) * TIMESPANTICKS_IN_SECOND / mpeg->sample_rate;
	return result;
}

#if 0
static void
mpeg_print_info (MpegFrameHeader *mpeg)
{
	const char *version;
	
	switch (mpeg->version) {
	case 1:
		version = "1";
		break;
	case 2:
		version = "2";
		break;
	default:
		version = "2.5";
		break;
	}
	
	printf ("MPEG-%s Audio Layer %d; %d Hz, %d ch, %d kbit\n",
		version, mpeg->layer, mpeg->sample_rate, mpeg->channels,
		mpeg->bit_rate / 1000);
	
	printf ("\t16bit crc=%s; padded=%s\n", mpeg->prot ? "true" : "false",
		mpeg->padded ? "true" : "false");
	
	printf ("\tframe length = %u bytes\n", mpeg_frame_length (mpeg, false));
}
#endif

static int
mpeg_xing_header_offset (MpegFrameHeader *mpeg)
{
	if (mpeg->version == 1)
		return mpeg->channels == 1 ? 21 : 36;
	else
		return mpeg->channels == 1 ? 13 : 21;
}

#define mpeg_vbri_header_offset 36

static bool
mpeg_check_vbr_headers (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, IMediaSource *source, gint64 pos)
{
	guint32 nframes = 0, size = 0;
	double len;
	guint8 buffer[24], *bufptr;
	gint64 offset;
	int i;
	
	// first, check for a Xing header
	offset = mpeg_xing_header_offset (mpeg);
	if (!source->Seek (pos + offset, SEEK_SET))
		return false;
	
	if (!source->Peek (buffer, 16))
		return false;
	
	if (!strncmp ((const char *) buffer, "Xing", 4)) {
		if (buffer [7] & 0x01) {
			// decode the number of frames
			nframes = (buffer [8] << 24) + (buffer [9] << 16) + (buffer [10] << 8) + buffer [11];
		} else if (buffer [7] & 0x02) {
			size = (buffer [8] << 24) + (buffer [9] << 16) + (buffer [10] << 8) + buffer [11];
			
			// calculate the frame length
			len = mpeg_frame_length (mpeg, true);
			
			// estimate the number of frames
			nframes = size / len;
		}
		
		vbr->type = MpegXingHeader;
		vbr->nframes = nframes;
		
		return true;
	}
	
	// check for a Fraunhofer VBRI header
	offset = mpeg_vbri_header_offset;
	if (!source->Seek (pos + offset, SEEK_SET))
		return false;
	
	if (!source->Peek (buffer, 24))
		return false;
	
	if (!strncmp ((const char *) buffer, "VBRI", 4)) {
		// decode the number of frames
		bufptr = buffer + 14;
		for (i = 0; i < 4; i++)
			nframes = (nframes << 8) | *bufptr++;
		
		vbr->type = MpegVBRIHeader;
		vbr->nframes = nframes;
		
		return true;
	}
	
	return false;
}


#define MPEG_JUMP_TABLE_GROW_SIZE 16

Mp3FrameReader::Mp3FrameReader (IMediaSource *source, AudioStream *stream, gint64 start, guint32 frame_len, guint32 frame_duration, bool xing)
{
	jmptab = g_new (MpegFrame, MPEG_JUMP_TABLE_GROW_SIZE);
	avail = MPEG_JUMP_TABLE_GROW_SIZE;
	used = 0;
	
	this->frame_dur = frame_duration;
	this->frame_len = frame_len;
	this->xing = xing;
	this->sync_lost = false;
	
	stream_start = start;
	this->source = source;
	this->stream = stream;
	
	bit_rate = 0;
	cur_pts = 0;
}

Mp3FrameReader::~Mp3FrameReader ()
{
	g_free (jmptab);
}

void
Mp3FrameReader::AddFrameIndex (gint64 offset, guint64 pts, guint32 dur, gint32 bit_rate)
{
	if (used == avail) {
		avail += MPEG_JUMP_TABLE_GROW_SIZE;
		jmptab = (MpegFrame *) g_realloc (jmptab, avail * sizeof (MpegFrame));
	}
	
	jmptab[used].bit_rate = bit_rate;
	jmptab[used].offset = offset;
	jmptab[used].pts = pts;
	jmptab[used].dur = dur;
	
	used++;
}

/**
 * MID:
 * @lo: the low bound
 * @hi: the high bound
 *
 * Finds the midpoint between positive integer values, @lo and @hi.
 *
 * Notes: Typically expressed as '(@lo + @hi) / 2', this is incorrect
 * when @lo and @hi are sufficiently large enough that combining them
 * would overflow their integer type. To work around this, we use the
 * formula, '@lo + ((@hi - @lo) / 2)', thus preventing this problem
 * from occuring.
 *
 * Returns the midpoint between @lo and @hi (rounded down).
 **/
#define MID(lo, hi) (lo + ((hi - lo) >> 1))

guint32
Mp3FrameReader::MpegFrameSearch (guint64 pts)
{
	guint64 start, end;
	guint32 hi = used - 1;
	guint32 m = hi >> 1;
	guint32 lo = 0;
	
	do {
		end = start = jmptab[m].pts;
		end += jmptab[m].dur;
		
		if (pts > end) {
			lo = m + 1;
		} else if (pts < start) {
			hi = m;
		} else {
			if (pts == end) {
				// pts should be exactly the beginning of the next frame
				m++;
			}
			
			break;
		}
		
		m = MID (lo, hi);
	} while (lo < hi);
	
	return m;
}

MediaResult
Mp3FrameReader::Seek (guint64 pts)
{
	gint64 offset = source->GetPosition ();
	gint32 bit_rate = this->bit_rate;
	guint64 cur_pts = this->cur_pts;
	guint32 frame;
	MediaResult result = MEDIA_FAIL;
	
	if (pts == cur_pts)
		return MEDIA_SUCCESS;
	
	if (pts == 0) {
		if (!source->Seek (stream_start, SEEK_SET)) {
			LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Seek error (#1)\n", pts);
			goto exception;
		}
		
		bit_rate = 0;
		cur_pts = 0;
		
		return MEDIA_SUCCESS;
	}
	
	// if we are seeking to some place we've been, then we can use our jump table
	if (used > 0 && pts < (jmptab[used - 1].pts + jmptab[used - 1].dur)) {
		if (pts >= jmptab[used - 1].pts) {
			if (!source->Seek (jmptab[used - 1].offset, SEEK_SET)) {
				LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Seek error (#2)\n", pts)
				goto exception;
			}
			
			this->bit_rate = jmptab[used - 1].bit_rate;
			this->cur_pts = jmptab[used - 1].pts;
			
			return MEDIA_SUCCESS;
		}
		
		// search for our requested pts
		frame = MpegFrameSearch (pts);
		
		if (!source->Seek (jmptab[frame].offset, SEEK_SET)) {
			LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Seek error (#3)\n", pts);
			goto exception;
		}
		
		this->bit_rate = jmptab[frame].bit_rate;
		this->cur_pts = jmptab[frame].pts;
		
		return MEDIA_SUCCESS;
	}
	
	// keep skipping frames until we read to (or past) the requested pts
	while (this->cur_pts < pts) {
		result = SkipFrame ();

		if (!MEDIA_SUCCEEDED (result)) {
			LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Error while skipping frame: %i\n", pts, result);
			goto exception;
		}
	}
	
	// pts requested is at the start of the next frame in the source
	if (this->cur_pts == pts)
		return MEDIA_SUCCESS;
	
	// pts requested was non-key frame, need to seek back to the most recent key frame
	if (!source->Seek (jmptab[used - 1].offset, SEEK_SET)) {
			LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Seek error (#4)\n", pts);
		goto exception;
	}
	
	this->bit_rate = jmptab[used - 1].bit_rate;
	this->cur_pts = jmptab[used - 1].pts;
	
	return MEDIA_SUCCESS;
	
exception:
	
	// restore FrameReader to previous state
	source->Seek (offset, SEEK_SET);
	this->bit_rate = bit_rate;
	this->cur_pts = cur_pts;
	
	LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT "): Could not find pts\n", pts);
	
	return result;
}

MediaResult
Mp3FrameReader::SkipFrame ()
{
	MpegFrameHeader mpeg;
	guint64 duration;
	guint8 buffer[4];
	gint64 offset;
	guint32 len;
	bool eof;
	
	offset = source->GetPosition ();

	if (!source->IsPositionAvailable (offset + 4, &eof))
		return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;

	if (!source->Peek (buffer, 4))
		return MEDIA_FAIL;
	
	if (!mpeg_parse_header (&mpeg, buffer)) {
		sync_lost = true;
		return MEDIA_FAIL;
	}
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = (guint32) mpeg_frame_length (&mpeg, xing);
	
	if (!source->IsPositionAvailable (offset + len, &eof))
		return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;
		
	if (!source->Seek ((gint64) len, SEEK_CUR))
		return MEDIA_FAIL;
	
	cur_pts += duration;

	stream->SetLastAvailablePts (cur_pts);

	return MEDIA_SUCCESS;
}

MediaResult
Mp3FrameReader::TryReadFrame (MediaFrame **f)
{
	MpegFrameHeader mpeg;
	guint64 duration;
	guint8 buffer[4];
	gint64 offset;
	guint32 len;
	MediaFrame *frame;
	bool eof = false;
	MediaResult result;
	

	if (sync_lost) {
		result = FindMpegHeader (&mpeg, NULL, source, source->GetPosition (), &offset);
		if (!MEDIA_SUCCEEDED (result))
			return result;
			
		if (!source->IsPositionAvailable (offset, &eof))
			return eof ? MEDIA_NO_MORE_DATA : MEDIA_NOT_ENOUGH_DATA;
			
		if (!source->Seek (offset, SEEK_SET))
			return MEDIA_FAIL;
		sync_lost = false;
	} else {
		offset = source->GetPosition ();
	}
	
	// Check if there is enough data available
	if (!source->IsPositionAvailable (offset + 4, &eof)) {
		//printf ("Mp3FrameReader::TryReadFrame (): Exit 2: Buffer underflow (last available pos: %" G_GINT64_FORMAT ", offset: %" G_GUINT64_FORMAT ", diff: %" G_GUINT64_FORMAT ", len: %u)\n", source->GetLastAvailablePosition (), offset, source->GetLastAvailablePosition () - offset, len);
		return eof? MEDIA_NO_MORE_DATA : MEDIA_NOT_ENOUGH_DATA;
	}
	
	if (!source->Peek (buffer, 4)) {
		//printf ("Mp3FrameReader::TryReadFrame (): Exit 3\n");
		return MEDIA_FAIL; // Now this shouldn't fail, given that we've checked for the previous error conditions
	}
	
	if (!mpeg_parse_header (&mpeg, buffer)) {
		//printf ("Mp3FrameReader::TryReadFrame (): Exit 4\n");
		sync_lost = true;
		return MEDIA_DEMUXER_ERROR;
	}
	
	//printf ("Mp3FrameReader::ReadFrame():\n");
	//mpeg_print_info (&mpeg);
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
		
		// re-encode the bitrate into the header
		buffer[2] |= mpeg_encode_bitrate (&mpeg, bit_rate);
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = (guint32) mpeg_frame_length (&mpeg, xing);

	if (!source->IsPositionAvailable (offset + len, &eof)) {
		//printf ("Mp3FrameReader::TryReadFrame (): Exit 6: Buffer underflow (last available pos: %" G_GINT64_FORMAT ", offset: %" G_GUINT64_FORMAT ", diff: %" G_GUINT64_FORMAT ", len: %u)\n", source->GetLastAvailablePosition (), offset, source->GetLastAvailablePosition () - offset, len);
		return eof ? MEDIA_NO_MORE_DATA : MEDIA_BUFFER_UNDERFLOW;
	}

	frame = new MediaFrame (stream);
	*f = frame;
	frame->buflen = len;
	
	if (mpeg.layer != 1 && !mpeg.padded)
		frame->buffer = (guint8 *) g_try_malloc (frame->buflen + 1);
	else
		frame->buffer = (guint8 *) g_try_malloc (frame->buflen);
	
	if (frame->buffer == NULL)
		return MEDIA_OUT_OF_MEMORY;
	
	if (mpeg.layer != 1 && !mpeg.padded)
		frame->buffer[frame->buflen - 1] = 0;
	
	if (!source->ReadAll (frame->buffer, len)) {
		//printf ("Mp3FrameReader::TryReadFrame (): Exit 7\n");
		return MEDIA_FAIL;
	}
					
	memcpy (frame->buffer, buffer, 4);
	
	frame->pts = cur_pts;
	frame->duration = duration;
	
	frame->AddState (MediaFrameDemuxed);
	
	cur_pts += duration;
	
	return MEDIA_SUCCESS;
}


/*
 * Mp3Demuxer
 */

Mp3Demuxer::Mp3Demuxer (Media *media, IMediaSource *source) : IMediaDemuxer (Type::MP3DEMUXER, media, source)
{
	reader = NULL;
	xing = false;
}

Mp3Demuxer::~Mp3Demuxer ()
{
	if (reader)
		delete reader;
}

void
Mp3Demuxer::SeekAsyncInternal (guint64 pts)
{
	MediaResult result = MEDIA_FAIL;
	
	if (reader)
		result = reader->Seek (pts);
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportSeekCompleted (pts);
	} else if (result == MEDIA_NOT_ENOUGH_DATA) {
		EnqueueSeek ();
	} else {
		ReportErrorOccurred (result);
	}
}

MediaResult
Mp3FrameReader::FindMpegHeader (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, IMediaSource *source, gint64 start, gint64 *result)
{
	guint8 buf[4096], hdr[4], *inbuf, *inend;
	gint64 pos, offset = start;
	register guint8 *inptr;
	MpegFrameHeader next;
	gint32 n = 0;
	guint32 len;
	bool eof = false;
	
	*result = -1;

	if (!source->Seek (start, SEEK_SET))
		return MEDIA_FAIL;
	
	inbuf = buf;
	
	do {
		if (!source->IsPositionAvailable (offset + sizeof (buf) - n, &eof)) {
			if (!eof) {
				return MEDIA_NOT_ENOUGH_DATA;
			}
			// If we reached eof we still need to check if this is a valid mpeg header, so don't fail that case
		}
		
		if ((n = source->ReadSome (inbuf, sizeof (buf) - n)) <= 0)
			return MEDIA_NO_MORE_DATA;
		
		inend = inbuf + n;
		inptr = buf;
		
		if ((inend - inptr) < 4)
			return MEDIA_FAIL;
		
		do {
			/* mpeg audio sync header begins with a 0xff */
			while (inptr < inend && *inptr != 0xff) {
				offset++;
				inptr++;
			}
			
			if (inptr == inend)
				break;
			
			/* found a 0xff byte... could be a frame header */
			if ((inptr + 3) < inend) {
				if (mpeg_parse_header (mpeg, inptr) && mpeg->bit_rate) {
					/* validate that this is really an MPEG frame header by calculating the
					 * position of the next frame header and checking that it looks like a
					 * valid frame header too */
					len = (guint32) mpeg_frame_length (mpeg, false);
					pos = source->GetPosition ();
					
					if (vbr && mpeg_check_vbr_headers (mpeg, vbr, source, offset)) {
						if (vbr->type == MpegXingHeader)
							len = (guint32) mpeg_frame_length (mpeg, true);
						
						*result = offset + len;
						return MEDIA_SUCCESS;
					}
					
					if (!source->IsPositionAvailable (offset + len + 4, &eof)) {
						return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;
					} else if (source->Seek (offset + len, SEEK_SET) && source->Peek (hdr, 4)) {
						if (mpeg_parse_header (&next, hdr)) {
							/* everything checks out A-OK */
							*result = offset;
							return MEDIA_SUCCESS;
						}
					}
					
					/* restore state */
					if (pos == -1 || !source->Seek (pos, SEEK_SET))
						return MEDIA_FAIL;
				}
				
				/* not an mpeg audio sync header */
				offset++;
				inptr++;
			} else {
				/* not enough data to check */
				break;
			}
		} while (inptr < inend);
		
		if ((n = (inend - inptr)) > 0) {
			/* save the remaining bytes */
			memmove (buf, inptr, n);
		}
		
		/* if we scan more than 'MPEG_FRAME_LENGTH_MAX' bytes, this is unlikely to be an mpeg audio stream */
	} while ((offset - start) < MPEG_FRAME_LENGTH_MAX);
	
	return MEDIA_FAIL;
}

void
Mp3Demuxer::OpenDemuxerAsyncInternal ()
{
	MediaResult result;
	
	LOG_MP3 ("Mp3Demuxer::OpenDemuxerAsyncInternal ()\n");
	
	result = ReadHeader ();
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportOpenDemuxerCompleted ();
	} else {
		ReportErrorOccurred (result);
	}
}

MediaResult
Mp3Demuxer::ReadHeader ()
{
	LOG_MP3 ("Mp3Demuxer::ReadHeader ()\n");

	MediaResult result;
	IMediaStream **streams = NULL;
	gint64 stream_start;
	gint64 header_start = -1;
	IMediaStream *stream;
	MpegFrameHeader mpeg;
	AudioStream *audio;
	Media *media;
	guint8 buffer[10];
	MpegVBRHeader vbr;
	guint64 duration;
	guint32 size = 0;
	double nframes;
	int stream_count;
	double len;
	gint64 end;
	int i;
	bool eof = false;
	
	if (!source->IsPositionAvailable (10, &eof))
		return eof ? MEDIA_FAIL : MEDIA_NOT_ENOUGH_DATA;

	if (!source->Peek (buffer, 10))
		return MEDIA_INVALID_MEDIA;
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80)
				return MEDIA_INVALID_MEDIA;
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		// MPEG stream data starts at the end of the ID3 tag
		stream_start = (gint64) size;
	} else {
		stream_start = 0;
	}
	
	// There can be an "arbitrary" amount of garbage at the
	// beginning of an mp3 stream, so we need to find the first
	// MPEG sync header by scanning.
	vbr.type = MpegNoVBRHeader;
	if (!MEDIA_SUCCEEDED (result = Mp3FrameReader::FindMpegHeader (&mpeg, &vbr, source, stream_start, &header_start))) {
		source->Seek (0, SEEK_SET);
		return result;
	}
	
	stream_start = header_start;
	
	if (!source->Seek (stream_start, SEEK_SET))
		return MEDIA_INVALID_MEDIA;
	
	if (vbr.type == MpegNoVBRHeader) {
		// calculate the frame length
		len = mpeg_frame_length (&mpeg, false);
		
		if ((end = source->GetSize ()) != -1) {
			// estimate the number of frames
			nframes = ((double) end - (double) stream_start) / (double) len;
		} else {
			nframes = 0;
		}
	} else {
		if (vbr.type == MpegXingHeader)
			xing = true;
		
		// calculate the frame length
		len = mpeg_frame_length (&mpeg, xing);
		nframes = vbr.nframes;
	}
	
	// calculate the duration of the first frame
	duration = mpeg_frame_duration (&mpeg);
	
	media = GetMediaReffed ();	
	stream = audio = new AudioStream (media);
	media->unref ();
	media = NULL;
	reader = new Mp3FrameReader (source, audio, stream_start, len, duration, xing);
	
	audio->codec_id = CODEC_MP3;
	audio->codec = g_strdup ("mp3");
	
	audio->duration = duration * nframes;
	audio->SetBitRate (mpeg.bit_rate);
	audio->SetChannels (mpeg.channels);
	audio->SetSampleRate (mpeg.sample_rate);
	audio->SetBlockAlign (mpeg_block_size (&mpeg));
	audio->SetBitsPerSample (mpeg.layer == 1 ? 32 : 8);
	audio->SetExtraData (NULL);
	audio->SetExtraDataSize (0);
	
	streams = g_new (IMediaStream *, 2);
	streams[0] = stream;
	streams[1] = NULL;
	stream_count = 1;
	
	SetStreams (streams, stream_count);
	stream->unref ();
	
	return MEDIA_SUCCESS;
}

MediaResult
Mp3Demuxer::GetFrameCallback (MediaClosure *c)
{
	MediaGetFrameClosure *closure = (MediaGetFrameClosure *) c;
	((Mp3Demuxer *) closure->GetDemuxer ())->GetFrameAsyncInternal (closure->GetStream ());
	return MEDIA_SUCCESS;
}

void
Mp3Demuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	MediaFrame *frame = NULL;
	MediaResult result;
	
	result = reader->TryReadFrame (&frame);

	if (result == MEDIA_DEMUXER_ERROR || result == MEDIA_BUFFER_UNDERFLOW || result == MEDIA_DEMUXER_ERROR || result == MEDIA_NOT_ENOUGH_DATA) {
		Media *media = GetMediaReffed ();
		g_return_if_fail (media != NULL);
		MediaGetFrameClosure *closure = new MediaGetFrameClosure (media, GetFrameCallback, this, stream);
		media->EnqueueWork (closure, false);
		closure->unref ();
		media->unref ();
		return;
	}
	
	if (result == MEDIA_NO_MORE_DATA) {
		ReportGetFrameCompleted (NULL);
	} else if (MEDIA_SUCCEEDED (result)) {
		ReportGetFrameCompleted (frame);
	} else {
		ReportErrorOccurred (result);
	}
	
	if (frame)
		frame->unref ();
}

/*
 * Mp3DemuxerInfo
 */

MediaResult
Mp3DemuxerInfo::Supports (IMediaSource *source)
{
	MediaResult result;
	gint64 stream_start = 0;
	gint64 header_start = 0;
	MpegFrameHeader mpeg;
	guint8 buffer[10];
	guint32 size = 0;
	MpegVBRHeader vbr;
	int i;
	
	// peek at the first 10 bytes which is enough to contain
	// either the mp3 frame header or an ID3 tag header
	if (!source->Peek (buffer, 10))
		return MEDIA_FAIL;
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80)
				return MEDIA_FAIL;
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		// skip over the ID3 tag
		stream_start = (gint64) size;
	}
	
	result = Mp3FrameReader::FindMpegHeader (&mpeg, &vbr, source, stream_start, &header_start);
	
	source->Seek (0, SEEK_SET);
	
	LOG_MP3 ("Mp3DemuxerInfo::Supports (%p) result: %i\n", source, result);
	
	return result;
}

IMediaDemuxer *
Mp3DemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new Mp3Demuxer (media, source);
}
