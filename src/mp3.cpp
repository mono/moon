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

namespace Moonlight {

//
// Relevant links to documentation:
//  http://www.codeproject.com/KB/audio-video/mpegaudioinfo.aspx
//  http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
//  http://www.compuphase.com/mp3/sta013.htm
//

/*
 * MPEG Audio Demuxer
 */

/* validate that this is an MPEG audio stream by checking that
 * the 32bit header matches the pattern:
 *
 * 1111 1111 111* **** **** **** **** **** = 0xff 0xe0
 *
 * Use a mask of 0xffe6 (because bits 12 and 13 can both be 0 if it is
 * MPEG 2.5). Compare the second byte > 0xe0 because one of the other
 * masked bits has to be set (the Layer bits cannot both be 0).
 */
#define is_mpeg_header(buffer) (buffer[0] == 0xff && ((buffer[1] & 0xe6) > 0xe0) && (buffer[1] & 0x18) != 0x08)

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
mpeg_frame_length (MpegFrameHeader *mpeg)
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
mpeg_check_vbr_headers (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, guint8 *inptr, guint32 n)
{
	guint32 nframes = 0, size = 0;
	double len;
	guint8 *buffer;
	guint8 *bufptr;
	gint64 offset;
	int i;
	
	// first, check for a Xing header
	offset = mpeg_xing_header_offset (mpeg);
	if (offset + 16 <= n) {
		buffer = inptr + offset;
		if (!strncmp ((const char *) buffer, "Xing", 4)) {
			if (buffer [7] & 0x01) {
				// decode the number of frames
				nframes = (buffer [8] << 24) + (buffer [9] << 16) + (buffer [10] << 8) + buffer [11];
			} else if (buffer [7] & 0x02) {
				size = (buffer [8] << 24) + (buffer [9] << 16) + (buffer [10] << 8) + buffer [11];
				
				// calculate the frame length
				len = mpeg_frame_length (mpeg);
				
				// estimate the number of frames
				nframes = size / len;
			}
			
			vbr->type = MpegXingHeader;
			vbr->nframes = nframes;
			
			return true;
		}
	}
	
	// check for a Fraunhofer VBRI header
	offset = mpeg_vbri_header_offset;
	if (offset + 24 <= n) {
		buffer = inptr + offset;	
		if (!strncmp ((const char *) buffer, "VBRI", 4)) {
			// decode the number of frames
			bufptr = buffer + 14;
			for (i = 0; i < 4; i++)
				nframes = (nframes << 8) | *bufptr++;
			
			vbr->type = MpegVBRIHeader;
			vbr->nframes = nframes;
			
			return true;
		}
	}
	
	return false;
}


#define MPEG_JUMP_TABLE_GROW_SIZE 16

/*
 * Mp3FrameReader
 */

Mp3FrameReader::Mp3FrameReader (Mp3Demuxer *demuxer, IMediaSource *source, AudioStream *stream, gint64 stream_start, double frame_len, guint32 frame_duration, double nframes)
{
	jmptab = g_new (MpegFrame, MPEG_JUMP_TABLE_GROW_SIZE);
	avail = MPEG_JUMP_TABLE_GROW_SIZE;
	used = 0;
	
	this->frame_dur = frame_duration;
	this->frame_len = frame_len;
	this->is_cur_pts_guaranteed = false;
	this->nframes = nframes;
	this->stream_start = stream_start;
	this->demuxer = demuxer;
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
	if (!is_cur_pts_guaranteed) {
		/* Don't keep an index when we can't guarantee it's accurate */
		return;
	}

	if (used != 0 && offset <= jmptab[used - 1].offset) {
		/* The frame we're trying to append to the index isn't after the last frame in the index */
		return;
	}

	if (used == avail) {
		avail += MPEG_JUMP_TABLE_GROW_SIZE;
		jmptab = (MpegFrame *) g_realloc (jmptab, avail * sizeof (MpegFrame));
	}
	
	jmptab [used].bit_rate = bit_rate;
	jmptab [used].offset = offset;
	jmptab [used].pts = pts;
	jmptab [used].dur = dur;
	
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

void
Mp3FrameReader::Seek (guint64 pts)
{
	if (pts >= cur_pts && pts <= cur_pts + frame_dur) {
		/* Nothing to do here */
		LOG_MP3 ("Mp3FrameReader:Seek (%" G_GUINT64_FORMAT " ms): seeked to the current frame.\n", MilliSeconds_FromPts (pts));
		demuxer->ReportSeekCompleted (pts);
		return;
	}
	
	/* Cancel any pending reads */
	demuxer->CancelPendingReads ();

	/* Optimize seeking to the very beginning */
	if (pts == 0) {
		LOG_MP3 ("Mp3FrameReader:Seek (%" G_GUINT64_FORMAT " ms): seeked to beginning of file.\n", MilliSeconds_FromPts (pts));
		demuxer->SetNextReadPosition (0);
		this->cur_pts = 0;
		demuxer->ReportSeekCompleted (pts);
		return;
	}
	
	/* If we are seeking to some place we've been, we can use our jump table */
	if (used > 0 && pts < (jmptab [used - 1].pts + jmptab [used - 1].dur)) {
		guint32 frame;
		if (pts >= jmptab [used - 1].pts) {
			frame = used - 1;
		} else {
			frame = MpegFrameSearch (pts);
		}

		/* Seeking to somewhere within the index has the interesting side effect that we can now guarantee cur_pts
		 * even if we've seeked to unknown land earlier */
		this->is_cur_pts_guaranteed = true;
		this->bit_rate = jmptab [frame].bit_rate;
		this->cur_pts = jmptab [frame].pts;
		demuxer->SetNextReadPosition (jmptab [frame].offset);
		
		LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT " ms): Seeked using index. Next read position: %" G_GUINT64_FORMAT " bit_rate: %i cur_pts: %" G_GUINT64_FORMAT " ms.\n",
			MilliSeconds_FromPts (pts), demuxer->GetNextReadPosition (), this->bit_rate, MilliSeconds_FromPts (this->cur_pts));
			
		demuxer->ReportSeekCompleted (pts);
	
		return;
	}

	/* Check if we're seeking beyond the end */
	if (pts >= demuxer->GetDuration ()) {
		IMediaSource *source = demuxer->GetSource ();
		this->cur_pts = demuxer->GetDuration ();
		this->bit_rate = 0;
		demuxer->SetNextReadPosition (source->GetSize () == -1 ? this->cur_pts * this->frame_len : source->GetSize ());
		
		LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT " ms): Seeked beyond end. Next read position: %" G_GUINT64_FORMAT " bit_rate: %i cur_pts: %" G_GUINT64_FORMAT " ms.\n",
			MilliSeconds_FromPts (pts), demuxer->GetNextReadPosition (), this->bit_rate, MilliSeconds_FromPts (this->cur_pts));

		demuxer->ReportSeekCompleted (pts);
		return;
	}
	
	/* We're seeking to somewhere we haven't been. There is no way to figure out if we've seeked correctly 
	 * after the seek, we just have to guess where the desired pts is as good as possible */
	
	double percentage = (double) pts / (double) demuxer->GetDuration ();
	double frame = percentage * nframes;
	this->bit_rate = 0;
	this->cur_pts = frame_dur * (guint32) frame;
	demuxer->SetNextReadPosition (stream_start + frame * frame_len);

	LOG_MP3 ("Mp3FrameReader::Seek (%" G_GUINT64_FORMAT " ms): Seeked by guessing. Next read position: %" G_GUINT64_FORMAT " bit_rate: %i cur_pts: %" G_GUINT64_FORMAT " ms (percentage: %.2f frame: %.2f nframes: %.2f)\n",
		MilliSeconds_FromPts (pts), demuxer->GetNextReadPosition (), this->bit_rate, MilliSeconds_FromPts (this->cur_pts), percentage, frame, nframes);

	demuxer->ReportSeekCompleted (pts);
}

MediaResult
Mp3FrameReader::ReadFrameCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	Mp3Demuxer *demuxer;
	
	if (!closure->IsCancelled ()) {
		demuxer = (Mp3Demuxer *) closure->GetContext ();
		demuxer->SetCurrentSource (closure->GetData ());
		demuxer->SetWaitingForRead (false);
		demuxer->GetReader ()->ReadFrame ();
	}
	return MEDIA_SUCCESS;
}

void
Mp3FrameReader::ReadFrame ()
{
	MpegFrameHeader mpeg;
	guint64 duration;
	guint32 len;
	MediaFrame *frame;
	MemoryBuffer *current_source = demuxer->GetCurrentSource ();
	guint64 start_position = current_source->GetPosition ();
	
	if (!FindMpegHeader (&mpeg, NULL, current_source)) {
		LOG_MP3 ("Mp3FrameReader::ReadFrame (): Not enough data (mpeg header not found or not enough data for entire frame) - requesting more\n");
		if (!demuxer->RequestMoreData (ReadFrameCallback)) {
			/* No more data */
			LOG_MP3 ("Mp3FrameReader::ReadFrame (): reached end of stream.\n");
			demuxer->ReportGetFrameCompleted (NULL);
		}
		return;
	}

	//printf ("Mp3FrameReader::ReadFrame():\n");
	//mpeg_print_info (&mpeg);
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	AddFrameIndex (demuxer->GetCurrentPosition () + current_source->GetPosition (), cur_pts, duration, bit_rate);
	
	len = (guint32) mpeg_frame_length (&mpeg);

	/* Check if we have enough data */
	if (current_source->GetRemainingSize () < len) {
		/* We need to seek back to where we started reading this frame so that the next time we're called
		 * we start parsing from the beginning again */
		current_source->SeekSet (start_position);
		
		if (!demuxer->RequestMoreData (ReadFrameCallback, MAX (len, 1024))) {
			/* No more data */
			demuxer->ReportGetFrameCompleted (NULL);
			return;
		}
		
		return;
	}

	frame = new MediaFrame (stream);
	if (!frame->AllocateBuffer (len)) {
		frame->unref ();
		return;
	}

	if (!current_source->Read (frame->GetBuffer (), len)) {
		/* This shouldn't happen, we've already checked that we have enough data */
		demuxer->ReportErrorOccurred ("Mp3Demuxer could not read from stream.");
		frame->unref ();
		return;
	}
	
	frame->pts = cur_pts;
	frame->duration = duration;
	
	frame->AddState (MediaFrameDemuxed);
	
	cur_pts += duration;
	
	demuxer->ReportGetFrameCompleted (frame);
	frame->unref ();
}

bool
Mp3FrameReader::FindMpegHeader (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, MemoryBuffer *source)
{
	guint8 *inbuf, *inend;
	gint64 offset = 0;
	guint8 *inptr;
	MpegFrameHeader next;
	guint32 n = 0;
	guint32 len;
	
	n = source->GetRemainingSize ();

	LOG_MP3 ("Mp3FrameReader::FindMpegHeader (): %u bytes in buffer\n", n);

	if (n < 4) {
		/* Not enough data left */
		LOG_MP3 ("Mp3FrameReader::FindMpegHeader (): Failed (less than 4 bytes available).\n");
		return false;
	}

	inbuf = (guint8 *) source->GetCurrentPtr ();
	inend = inbuf + n;
	inptr = inbuf;

	do {
		/* mpeg audio sync header begins with a 0xff */
		while (inptr < inend && *inptr != 0xff) {
			offset++;
			inptr++;
		}
		
		if (offset > 0) {
			/* Discard data we've already passed by */
			source->SeekOffset (offset);
			n -= offset;
			offset = 0;
		}

		if (n < 4) {
			/* Not enough data left */
			LOG_MP3 ("Mp3FrameReader::FindMpegHeader (): Failed (less than 4 bytes left).\n");
			return false;
		}
		
		/* found a 0xff byte... could be a frame header */
		if (mpeg_parse_header (mpeg, inptr) && mpeg->bit_rate) {
			/* validate that this is really an MPEG frame header by calculating the
			 * position of the next frame header and checking that it looks like a
			 * valid frame header too */
			
			if (vbr && mpeg_check_vbr_headers (mpeg, vbr, inptr, n)) {
				/* It's a vbr frame, no need to check the next frame, this check is good enough */
				return true;
			}

			len = (guint32) mpeg_frame_length (mpeg);

			if (n == len) {
				/* The last frame in the file, there is no next frame */
				/* TODO: maybe add a check for a real eof here? */
				return true;
			}

			if (n < len + 4) {
				/* Not enough data */
				LOG_MP3 ("Mp3FrameReader::FindMpegHeader (): Failed (entire frame isn't available (%u bytes for frame, %u bytes available))\n", len + 4, n);
				return false;
			}

			/* Try to parse the memory where the next header would be */
			if (mpeg_parse_header (&next, inptr + len)) {
				/* everything checks out A-OK */
				return true;
			}
			
			/* Not an mpeg audio sync header, continue search */
		}
		
		/* not an mpeg audio sync header */
		offset++;
		inptr++;
	} while (inptr < inend);

	LOG_MP3 ("Mp3FrameReader::FindMpegHeader (): Failed (reached end of input buffer.\n");

	return false;
}

/*
 * Mp3Demuxer
 */

Mp3Demuxer::Mp3Demuxer (Media *media, IMediaSource *source) : IMediaDemuxer (Type::MP3DEMUXER, media, source)
{
	reader = NULL;
	current_source = NULL;
	read_closure = NULL;
	waiting_for_read = false;
	next_read_position = G_MAXUINT64;
	current_position = 0;
}

Mp3Demuxer::~Mp3Demuxer ()
{
	delete reader;
}

void
Mp3Demuxer::Dispose ()
{
	if (read_closure) {
		read_closure->unref ();
		read_closure = NULL;
	}
	
	if (current_source) {
		current_source->unref ();
		current_source = NULL;
	}
	
	IMediaDemuxer::Dispose ();
}

void
Mp3Demuxer::SeekAsyncInternal (guint64 pts)
{
	if (reader == NULL)
		return;

	reader->Seek (pts);
}

void
Mp3Demuxer::OpenDemuxerAsyncInternal ()
{
	LOG_MP3 ("Mp3Demuxer::OpenDemuxerAsyncInternal ()\n");
	OpenDemuxer (NULL);
}

void
Mp3Demuxer::CancelPendingReads ()
{
	if (!waiting_for_read)
		return;

	read_closure->Cancel ();
	read_closure->unref ();
	read_closure = NULL;
	waiting_for_read = false;
}

bool
Mp3Demuxer::RequestMoreData (MediaCallback *callback, guint32 count)
{
	guint64 start = 0;
	guint32 left = 0;
	guint64 previous_read_position = next_read_position;
	
	g_return_val_if_fail (!waiting_for_read, false);
	
	if (read_closure != NULL) {
		if (read_closure->GetCount () != read_closure->GetData ()->GetSize ()) {
			/* The last read didn't read everything we requested, so there is nothing more to read */
			LOG_MP3 ("Mp3Demuxer::RequestMoreData (): the last read didn't read everything we requested, so we reached eof.\n");
			return false;
		}
	
		gint64 position = read_closure->GetData ()->GetPosition ();	
		left = read_closure->GetData ()->GetRemainingSize ();
		start = read_closure->GetOffset () + position;
		read_closure->unref ();
		read_closure = NULL;
	}

	if (next_read_position != G_MAXUINT64) {
		start = next_read_position;
		next_read_position = G_MAXUINT64;
	}
	
	Media *media = GetMediaReffed ();
	read_closure = new MediaReadClosure (media, callback, this, start, count + left);
	media->unref ();
	
	LOG_MP3 ("Mp3Demuxer::RequestMoreData (%u) requesting: %u at offset %" G_GINT64_FORMAT " (left: %u next read position: %" G_GUINT64_FORMAT ")\n", 
		count, read_closure->GetCount (), read_closure->GetOffset (), left, previous_read_position);

	waiting_for_read = true;
	current_position = start;
	source->ReadAsync (read_closure);

	return true;
}

MediaResult
Mp3Demuxer::OpenDemuxerCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	Mp3Demuxer *demuxer = (Mp3Demuxer *) closure->GetContext ();

	LOG_MP3 ("Mp3Demuxer::OpenDemuxerCallback (offset: %" G_GUINT64_FORMAT " requested: %u, got: %" G_GINT64_FORMAT ") cancelled: %i\n", 
		closure->GetOffset (), closure->GetCount (), closure->GetData ()->GetSize (), closure->IsCancelled ());

	if (!closure->IsCancelled ()) {
		demuxer->SetWaitingForRead (false);
		demuxer->OpenDemuxer (closure->GetData ());
	}
	
	return MEDIA_SUCCESS;
}

void
Mp3Demuxer::OpenDemuxer (MemoryBuffer *open_source)
{
	LOG_MP3 ("Mp3Demuxer::OpenDemuxer ()\n");

	IMediaStream **streams = NULL;
	IMediaStream *stream;
	MpegFrameHeader mpeg;
	gint64 stream_start;
	AudioStream *audio;
	Media *media;
	guint8 buffer [10];
	MpegVBRHeader vbr;
	guint64 duration;
	guint32 size = 0;
	double nframes;
	int stream_count;
	double len;
	gint64 end;
	int i;

	if (open_source == NULL) {
		/* No data read yet, request a read */
		if (!RequestMoreData (OpenDemuxerCallback))
			ReportErrorOccurred ("Could not open Mp3 demuxer: unexpected end of data.");
		return;
	}

	if (open_source->GetSize () < 10) {
		/* Not enough data read, request more */
		if (!RequestMoreData (OpenDemuxerCallback))
			ReportErrorOccurred ("Could not open Mp3 demuxer: data stream ended early.");
		return;
	}
	
	if (!open_source->Peek (buffer, 10)) {
		/* This shouldn't fail */
		ReportErrorOccurred ("Could not open Mp3 demuxer: peek error.");
		return;
	}

	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80) {
				ReportErrorOccurred ("Could not open Mp3 demuxer: invalid ID3 tag.");
				return;
			}
			
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
	
	/* Seek to the stream start */
	if (stream_start > 0) {
		/* We're still at position 0, so no need to do anything if stream_start is also 0 */
		if (stream_start > open_source->GetSize ()) {
			/* Not enough data, request more */
			if (!RequestMoreData (OpenDemuxerCallback)) {
				ReportErrorOccurred ("Could not open Mp3 demuxer: could not seek to end of ID3 tag.");
				return;
			}
		}
		open_source->SeekOffset (stream_start);
	}

	// There can be an "arbitrary" amount of garbage at the
	// beginning of an mp3 stream, so we need to find the first
	// MPEG sync header by scanning.
	vbr.type = MpegNoVBRHeader;
	if (!Mp3FrameReader::FindMpegHeader (&mpeg, &vbr, open_source)) {
		/* Could not find a header with the available data */
		/* Seek back to 0, read more data, and try again */
		/* TODO: store the current state and only read new data to avoid seeking back here */
		open_source->SeekSet (0);
		if (!RequestMoreData (OpenDemuxerCallback)) {
			/* This should not happen, we should be able to seek back to 0 */
			ReportErrorOccurred ("Could not open Mp3 demuxer: error while seeking to start point.");
		}
		
		return;
	}

	if (vbr.type == MpegNoVBRHeader) {
		// calculate the frame length
		len = mpeg_frame_length (&mpeg);
		
		if ((end = source->GetSize ()) != -1) {
			// estimate the number of frames
			nframes = ((double) end - (double) stream_start) / (double) len;
		} else {
			nframes = 0;
		}
	} else {
		// calculate the frame length
		len = mpeg_frame_length (&mpeg);
		nframes = vbr.nframes;
	}
	
	// calculate the duration of the first frame
	duration = mpeg_frame_duration (&mpeg);
	
	media = GetMediaReffed ();	
	stream = audio = new AudioStream (media);
	media->unref ();
	media = NULL;
	reader = new Mp3FrameReader (this, source, audio, stream_start, len, duration, nframes);
	
	audio->SetCodecId (CODEC_MP3);
	audio->SetDuration (duration * nframes);
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
	
	this->current_source = open_source;
	this->current_source->ref ();

	LOG_MP3 ("Mp3Demuxer::OpenDemuxer (): Version: %s Layer: %u VBR: %s Duration: %" G_GUINT64_FORMAT " ms Bit rate: %u Channels: %u Sample Rate: %u Block Align: %u Bits per sample: %u Number of frames: %.2f Frame length: %.2f\n",
		mpeg.version == 3 ? "2.5" : (mpeg.version == 2 ? "2" : "1"), mpeg.layer, vbr.type == MpegNoVBRHeader ? "No" : (vbr.type == MpegXingHeader ? "Xing" : "VBRI"), MilliSeconds_FromPts (audio->GetDuration ()), audio->GetBitRate (), audio->GetChannels (), audio->GetSampleRate (), audio->GetBlockAlign (), audio->GetBitsPerSample (), nframes, len);
		
	ReportOpenDemuxerCompleted ();
}

void
Mp3Demuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	reader->ReadFrame ();
}

void
Mp3Demuxer::SetCurrentSource (MemoryBuffer *value)
{
	if (current_source != NULL)
		current_source->unref ();
	current_source = value;
	if (current_source != NULL)
		current_source->ref ();
}

/*
 * Mp3DemuxerInfo
 */

MediaResult
Mp3DemuxerInfo::Supports (MemoryBuffer *source)
{
	MediaResult result;
	MpegFrameHeader mpeg;
	guint8 buffer[10];
	guint32 size = 0;
	MpegVBRHeader vbr;
	int i;
	
	// peek at the first 10 bytes which is enough to contain
	// either the mp3 frame header or an ID3 tag header
	if (!source->Peek (buffer, 10)) {
		/* We should always get at least 10 bytes, so treat this as a fatal error */
		LOG_MP3 ("Mp3DemuxerInfo::Supports (%p): Could not peek.\n", source);
		return MEDIA_FAIL;
	}
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80) {
				LOG_MP3 ("Mp3DemuxerInfo::Supports (%p): invalid ID3 tab.\n", source);
				return MEDIA_FAIL;
			}
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		if (source->GetSize () <= size) {
			/* Not enough data, request more */
			LOG_MP3 ("Mp3DemuxerInfo::Supports (%p): not enough data.\n", source);
			return MEDIA_NOT_ENOUGH_DATA;
		}

		// skip over the ID3 tag
		source->SeekOffset (size);
	}
	
	result = Mp3FrameReader::FindMpegHeader (&mpeg, &vbr, source) ? MEDIA_SUCCESS : MEDIA_NOT_ENOUGH_DATA;
	
	source->SeekSet (0);

	/* Theoretically we can have a 100gb file filled with garbage + 1 second of mp3 data at the end
	 * The pipeline will however bail out after a certain amount of data has been downloaded and no
	 * demuxer has been found */

	LOG_MP3 ("Mp3DemuxerInfo::Supports (%p) result: %i\n", source, result);
	
	return result;
}

IMediaDemuxer *
Mp3DemuxerInfo::Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
{
	return new Mp3Demuxer (media, source);
}

};
