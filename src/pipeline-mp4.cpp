/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-mp4.cpp: MP4 related parts of the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

/*
 * Some documentation is freely available here: http://standards.iso.org/ittf/PubliclyAvailableStandards/:
 *  ISO/IEC 14496-12: http://standards.iso.org/ittf/PubliclyAvailableStandards/c038539_ISO_IEC_14496-12_2004(E).zip
 *
 * Unfortunately most is not free, though some drafts are here: http://www.chiariglione.org/mpeg/working_documents.htm
 *  ISO/IEC 14496-1:  http://www.chiariglione.org/mpeg/working_documents/mpeg-04/systems/3rd_edition_wd.zip
 *  ISO/IEC 14496-15: http://www.chiariglione.org/mpeg/working_documents/mpeg-04/avc_ff/avc_ff.zip
 */

#include <config.h>

#include "clock.h"
#include "pipeline-mp4.h"

namespace Moonlight {

#define GETBETYPE(a,b,c,d) (d + (c << 8) + (b << 16) + (a << 24))
#define MOOV_4CC GETBETYPE('m', 'o', 'o', 'v')
#define MVHD_4CC GETBETYPE('m', 'v', 'h', 'd')
#define TRAK_4CC GETBETYPE('t', 'r', 'a', 'k')
#define TKHD_4CC GETBETYPE('t', 'k', 'h', 'd')
#define MDIA_4CC GETBETYPE('m', 'd', 'i', 'a')
#define MDHD_4CC GETBETYPE('m', 'd', 'h', 'd')
#define HDLR_4CC GETBETYPE('h', 'd', 'l', 'r')
#define MINF_4CC GETBETYPE('m', 'i', 'n', 'f')
#define VMHD_4CC GETBETYPE('v', 'm', 'h', 'd')
#define SMHD_4CC GETBETYPE('s', 'm', 'h', 'd')
#define DINF_4CC GETBETYPE('d', 'i', 'n', 'f')
#define DREF_4CC GETBETYPE('d', 'r', 'e', 'f')
#define URN_4CC  GETBETYPE('u', 'r', 'n', ' ')
#define URL_4CC  GETBETYPE('u', 'r', 'l', ' ')
#define STBL_4CC GETBETYPE('s', 't', 'b', 'l')
#define STSD_4CC GETBETYPE('s', 't', 's', 'd')
#define STTS_4CC GETBETYPE('s', 't', 't', 's')
#define STSS_4CC GETBETYPE('s', 't', 's', 's')
#define CTTS_4CC GETBETYPE('c', 't', 't', 's')
#define STSC_4CC GETBETYPE('s', 't', 's', 'c')
#define STSZ_4CC GETBETYPE('s', 't', 's', 'z')
#define STZ2_4CC GETBETYPE('s', 't', 'z', '2')
#define STCO_4CC GETBETYPE('s', 't', 'c', 'o')
#define CO64_4CC GETBETYPE('c', 'o', '6', '4')
#define MDAT_4CC GETBETYPE('m', 'd', 'a', 't')
#define HMHD_4CC GETBETYPE('h', 'm', 'h', 'd')
#define NMHD_4CC GETBETYPE('n', 'm', 'h', 'd')
#define AVC1_4CC GETBETYPE('a', 'v', 'c', '1')
#define MP4A_4CC GETBETYPE('m', 'p', '4', 'a')
#define MP4V_4CC GETBETYPE('m', 'p', '4', 'v')
#define MP41_4CC GETBETYPE('m', 'p', '4', '1')
#define MP42_4CC GETBETYPE('m', 'p', '4', '2')
#define UUID_4CC GETBETYPE('u', 'u', 'i', 'd')
#define FTYP_4CC GETBETYPE('f', 't', 'y', 'p')
#define SOUN_4CC GETBETYPE('s', 'o', 'u', 'n')
#define VIDE_4CC GETBETYPE('v', 'i', 'd', 'e')
#define ESDS_4CC GETBETYPE('e', 's', 'd', 's')
#define AVCC_4CC GETBETYPE('a', 'v', 'c', 'C')

#define FORMAT_4CC "'%c%c%c%c'"
#define VALUES_4CC(x) (char) ((x) >> 24), (char) (((x) >> 16) & 0xFF), (char) (((x) >> 8) & 0xFF), (char) ((x) & 0xFF)

#define VERIFY_AVAILABLE_SIZE(x) \
	if ((gint64) source->GetRemainingSize () < (gint64) (x)) { \
		char *msg = g_strdup_printf ("%s: unexpected end of stream", __func__); \
		ReportErrorOccurred (msg); \
		g_free (msg); \
		return false;	\
	}
#define VERIFY_AVAILABLE_POSITION(x) \
	if ((gint64) source->GetSize () < (gint64) (x)) { \
		char *msg = g_strdup_printf ("%s: unexpected end of stream", __func__); \
		ReportErrorOccurred (msg); \
		g_free (msg); \
		return false;	\
	}
#define VERIFY_BOX_SIZE(x, type)	\
	if (size < (x) + (source->GetPosition () - start)) {	\
		ReportErrorOccurred ("Corrupted mp4 file (" type " box size is too small)");	\
		return false;	\
	}
#define VERIFY_EXISTING_BOX(parent, child)	\
	if (parent->child != NULL) {	\
		ReportErrorOccurred ("Corrupted mp4 file, found more than one '" #child "' boxes per '" #parent "' box");	\
		return false;	\
	}
#define VERIFY_REQUIRED_BOX(parent, child)	\
	if (parent->child == NULL) {	\
		ReportErrorOccurred ("Mp4Demuxer: the required box '" #child "' in '" #parent "' wasn't found");	\
		return false;	\
	}
/*
 * Box
 */
Box::Box (guint32 type, guint64 size)
{
	this->type = type;
	this->size = size;
	this->parsed = false;
}

/*
 * FullBox
 */
FullBox::FullBox (guint32 type, guint64 size)
	: Box (type, size)
{
	version = 0;
	flags = 0;
}

/*
 * MoovBox
 */
MoovBox::MoovBox (guint32 type, guint64 size)
	: Box (type, size)
{
	mvhd = NULL;
	trak = NULL;
	trak_count = 0;
}

MoovBox::~MoovBox ()
{
	delete mvhd;
	for (guint32 i = 0; i < trak_count; i++)
		delete trak [i];
	g_free (trak);
}

/*
 * MvhdBox
 */
MvhdBox::MvhdBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	timescale = 0;
	duration = 0;
}

/*
 * TrakBox
 */
TrakBox::TrakBox (guint32 type, guint64 size)
	: Box (type, size)
{
	tkhd = NULL;
	mdia = NULL;
	stream = NULL;
	stsc_index = 0;
	stsc_sample_index = 0;
	stsc_sample_count = 0;
	stsc_samples_size = 0;
	current_sample = 0;
}

TrakBox::~TrakBox ()
{
	delete tkhd;
	delete mdia;
	if (stream)
		stream->unref ();
}

/*
 * TkhdBox
 */
TkhdBox::TkhdBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	track_ID = 0;
	duration = 0;
	width.hi = 0;
	width.lo = 0;
	height.hi = 0;
	height.lo = 0;
}

/*
 * MdiaBox
 */
MdiaBox::MdiaBox (guint32 type, guint64 size)
	: Box (type, size)
{
	mdhd = NULL;
	minf = NULL;
	hdlr = NULL;
}

MdiaBox::~MdiaBox ()
{
	delete mdhd;
	delete minf;
	delete hdlr;
}

/*
 * MdhdBox
 */
MdhdBox::MdhdBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	timescale = 0;
	duration = 0;
}

/*
 * HdlrBox
 */
HdlrBox::HdlrBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	handler_type = 0;
	name = NULL;
}

HdlrBox::~HdlrBox ()
{
	g_free (name);
}

/*
 * MinfBox
 */
MinfBox::MinfBox (MdiaBox *parent, guint32 type, guint64 size)
	: Box (type, size)
{
	this->parent = parent;
	vmhd = NULL;
	smhd = NULL;
	dinf = NULL;
	stbl = NULL;
}

MinfBox::~MinfBox ()
{
	delete vmhd;
	delete smhd;
	delete dinf;
	delete stbl;
}

/*
 * DinfBox
 */
DinfBox::DinfBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	dref = NULL;
}

DinfBox::~DinfBox ()
{
	delete dref;
}

/*
 * DrefBox
 */
DrefBox::DrefBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	entries = NULL;
}

DrefBox::~DrefBox ()
{
	for (guint32 i = 0; i < entry_count; i++)
		delete entries [i];
	g_free (entries);
}

/*
 * UrnBox
 */
UrnBox::UrnBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	name = NULL;
	location = NULL;
}

UrnBox::~UrnBox ()
{
	g_free (name);
	g_free (location);
}

/*
 * UrlBox
 */
UrlBox::UrlBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	location = NULL;
}

UrlBox::~UrlBox ()
{
	g_free (location);
}

/*
 * StblBox
 */
StblBox::StblBox (MinfBox *parent, guint32 type, guint64 size)
	: FullBox (type, size)
{
	this->parent = parent;
	stts = NULL;
	ctts = NULL;
	stsd = NULL;
	stsz = NULL;
	stz2 = NULL;
	stsc = NULL;
	stco = NULL;
	co64 = NULL;
	stss = NULL;
}

StblBox::~StblBox ()
{
	delete stts;
	delete ctts;
	delete stsd;
	delete stsz;
	delete stz2;
	delete stsc;
	delete stco;
	delete co64;
	delete stss;
}

/*
 * StsdBox
 */
StsdBox::StsdBox (StblBox *parent, guint32 type, guint64 size)
	: FullBox (type, size)
{
	this->parent = parent;
	entry_count = 0;
	entries = NULL;
}

StsdBox::~StsdBox ()
{
	for (guint32 i = 0; i < entry_count; i++)
		delete entries [i];
	g_free (entries);
}

/*
 * SttsBox
 */
SttsBox::SttsBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	sample_count = NULL;
	sample_delta = NULL;
}

SttsBox::~SttsBox ()
{
	g_free (sample_count);
	g_free (sample_delta);
}

/*
 * StssBox
 */
StssBox::StssBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	sample_number = NULL;
}

StssBox::~StssBox ()
{
	g_free (sample_number);
}

/*
 * CttsBox
 */
CttsBox::CttsBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	sample_count = NULL;
	sample_offset = NULL;
}

CttsBox::~CttsBox ()
{
	g_free (sample_count);
	g_free (sample_offset);
}

/*
 * StscBox
 */
StscBox::StscBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	first_chunk = NULL;
	samples_per_chunk = NULL;
	sample_description_index = NULL;
}

StscBox::~StscBox ()
{
	g_free (first_chunk);
	g_free (samples_per_chunk);
	g_free (sample_description_index);
}

/*
 * StszBox
 */
StszBox::StszBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	sample_size = 0;
	sample_count = 0;
	entry_size = NULL;
}

StszBox::~StszBox ()
{
	g_free (entry_size);
}

/*
 * Stz2Box
 */
Stz2Box::Stz2Box (guint32 type, guint64 size)
	: FullBox (type, size)
{
	field_size = 0;
	sample_count = 0;
	samples = NULL;
}

Stz2Box::~Stz2Box ()
{
	g_free (samples);
}

/*
 * StcoBox
 */
StcoBox::StcoBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	chunk_offset = NULL;
}

StcoBox::~StcoBox ()
{
	g_free (chunk_offset);
}

/*
 * Co64Box
 */
Co64Box::Co64Box (guint32 type, guint64 size)
	: FullBox (type, size)
{
	entry_count = 0;
	chunk_offset = NULL;
}

Co64Box::~Co64Box ()
{
	g_free (chunk_offset);
}

/*
 * EsdsBox
 */
EsdsBox::EsdsBox (guint32 type, guint64 size)
	: FullBox (type, size)
{
	ES_ID = 0;
	streamDependenceFlag = false;
	URL_Flag = false;
	OCRstreamFlag = false;
	dependsOn_ES_ID = 0;
	URLlength = 0;
	URLstring = NULL;
	OCR_ES_Id = 0;
	objectTypeIndication = 0;
	streamType = 0;
	upStream = false;
	bufferSizeDB = 0;
	maxBitrate = 0;
	avgBitrate = 0;
}

EsdsBox::~EsdsBox ()
{
	g_free (URLstring);
}

/*
 * SampleEntry
 */
SampleEntry::SampleEntry (guint32 type, guint64 size)
	: Box (type, size)
{
	data_reference_index = 0;
	format = NULL;
	esds = NULL;
	extradata_size = 0;
	extradata = NULL;
}

SampleEntry::~SampleEntry ()
{
	g_free (format);
	delete esds;
	g_free (extradata);
}

/*
 * VisualSampleEntry
 */
VisualSampleEntry::VisualSampleEntry (guint32 type, guint64 size)
	: SampleEntry (type, size)
{
	width = 0;
	height = 0;
	horizontal_resolution.hi = 0;
	horizontal_resolution.lo = 0;
	vertical_resolution.hi = 0;
	vertical_resolution.lo = 0;
	compressor_name = NULL;
}

VisualSampleEntry::~VisualSampleEntry ()
{
	g_free (compressor_name);
}

/*
 * AudioSampleEntry
 */
AudioSampleEntry::AudioSampleEntry (guint32 type, guint64 size)
	: SampleEntry (type, size)
{
	channelcount = 0;
	samplesize = 0;
	samplerate.hi = 0;
	samplerate.lo = 0;
}

/*
 * Mp4Demuxer
 */

Mp4Demuxer::Mp4Demuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
	: IMediaDemuxer (Type::MP4DEMUXER, media, source)
{
	buffer = initial_buffer;
	buffer->ref ();
	get_frame_stream = NULL;
	last_buffer = false;
	moov = NULL;
}

Mp4Demuxer::~Mp4Demuxer ()
{
	delete moov;
}

void
Mp4Demuxer::Dispose ()
{
	if (buffer != NULL) {
		buffer->unref ();
		buffer = NULL;
	}

	IMediaDemuxer::Dispose ();
}

char *
Mp4Demuxer::TypeToString (guint32 type)
{
	static char t [5];
	int c = 0;
	while (type != 0) {
		t [c++] = (type & 0xFF000000) >> 24;
		type <<= 8;
	}
	t [c] = 0;
	return g_strdup (t);
}

bool
Mp4Demuxer::IsCompatibleType (guint32 type)
{
	switch (type) {
	case AVC1_4CC:
	case MP41_4CC:
	case MP42_4CC:
		return true;
	default:
		return false;
	}
}

void
Mp4Demuxer::UpdateSelected (IMediaStream *stream)
{
	/* Nothing to do here */
}

void
Mp4Demuxer::SeekAsyncInternal (guint64 pts)
{
	guint64 time, acc_time;
	guint32 sample, samples_left;
	guint32 chunk_count;
	TrakBox *trak;
	StblBox *stbl;
	StscBox *stsc;
	StszBox *stsz;
	Stz2Box *stz2;
	StcoBox *stco;
	Co64Box *co64;
	SttsBox *stts;
	StssBox *stss;

	LOG_MP4 ("Mp4Demuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT " ms)\n", MilliSeconds_FromPts (pts));

	g_return_if_fail (moov != NULL);
	g_return_if_fail (moov->parsed);

	/* We have a table of time->location offsets in the file header, seeking is trivial */
	for (guint32 i = 0; i < moov->trak_count; i++) {
		trak = moov->trak [i];

		if (trak->stream == NULL)
			continue; /* Moonlight doesn't understand this stream, no need to seek in it either */

		stbl = trak->mdia->minf->stbl;
		stsc = trak->mdia->minf->stbl->stsc;
		stsz = trak->mdia->minf->stbl->stsz;
		stz2 = trak->mdia->minf->stbl->stz2;
		stco = trak->mdia->minf->stbl->stco;
		co64 = trak->mdia->minf->stbl->co64;
		stts = trak->mdia->minf->stbl->stts;
		stss = trak->mdia->minf->stbl->stss;

		trak->stsc_index = 0;
		trak->stsc_sample_index = 0;
		trak->stsc_sample_count = 0;
		trak->stsc_samples_size = 0;
		trak->current_sample = 0;

		/* Find the sample corresponding to the requested pts */
		time = FromPts (pts, trak);
		acc_time = 0;
		sample = 0;
		for (guint32 i = 0; i < stts->entry_count; i++) {
			if (stts->sample_count [i] * stts->sample_delta [i] + acc_time > time) {
				sample += (time - acc_time) / stts->sample_delta [i];
			} else {
				acc_time += stts->sample_count [i] * stts->sample_delta [i];
				sample += stts->sample_count [i];
			}
		}

		/* Find the first key frame before the exact sample */
		if (stss != NULL) {
			if (sample >= stss->sample_number [stts->entry_count - 1]) {
				/* After the last key frame, seek to the last key frame */
				trak->current_sample = stss->sample_number [stts->entry_count - 1];
			} else {
				trak->current_sample = sample;
				for (guint32 i = 0; i < stss->entry_count; i++) {
					if (sample >= stss->sample_number [i] && sample < stss->sample_number [i + 1]) {
						trak->current_sample = stss->sample_number [i];
						break;
					}
				}
			}
		} else {
			/* All samples are key-frames */
			trak->current_sample = sample;
		}
		LOG_MP4 ("Mp4Demuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT " ms): %s: exact sample: #%u key frame sample: #%u\n", MilliSeconds_FromPts (pts), trak->stream->GetTypeName (), sample, trak->current_sample);

		if (!GetChunkCount (stbl, &chunk_count))
			return;

		/* Now we need to update all the cached info in the TrakBox */
		samples_left = trak->current_sample;
		for (guint32 i = 0; i < stsc->entry_count; i++) {
			if (i + 1 == stsc->entry_count) {
				if (stsc->first_chunk [i] > chunk_count) {
					char *msg = g_strdup_printf ("Mp4Demuxer: last entry in 'stsc' table has invalid first_chunk value = %u (>chunk_count = %u)", stsc->first_chunk [i], chunk_count);
					ReportErrorOccurred (msg);
					g_free (msg);
					return;
				}
				trak->stsc_index = i;
				trak->stsc_sample_count = (chunk_count - stsc->first_chunk [i]) * stsc->samples_per_chunk [i];
				if (samples_left >= trak->stsc_sample_count) {
					trak->stsc_sample_index = trak->stsc_sample_count - 1;
				} else {
					trak->stsc_sample_index = samples_left;
				}
				break;
			}

			/* When parsing the stsc box we verify that first_chunk has sequential values, so 'n' can never end up negative here */
			guint32 n = (stsc->first_chunk [i + 1] - stsc->first_chunk [i]) * stsc->samples_per_chunk [i];
			if (n > samples_left) {
				trak->stsc_index = i;
				trak->stsc_sample_index = samples_left;
				trak->stsc_sample_count = n;
				break;
			} else {
				samples_left -= n;
			}
		}
		trak->stsc_samples_size = 0;
		for (guint32 i = trak->current_sample - (trak->stsc_sample_index % stsc->samples_per_chunk [trak->stsc_index]); i < trak->current_sample; i++) {
			guint32 s;
			if (!GetSampleSize (stbl, i, &s))
				return;
			trak->stsc_samples_size += s;
		}

		LOG_MP4 ("Mp4Demuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT " ms): %s: current_sample: %u stsc_index: %u stsc_sample_index: %u stsc_sample_count: %u stsc_sample_size: %u\n",
			MilliSeconds_FromPts (pts), trak->stream->GetTypeName (), trak->current_sample, trak->stsc_index, trak->stsc_sample_index, trak->stsc_sample_count, trak->stsc_samples_size);
	}

	ReportSeekCompleted (pts);
}

void
Mp4Demuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	guint32 sample_count;
	guint32 chunk;
	guint32 chunk_sample;
	guint32 chunk_count;
	guint64 chunk_offset;
	guint64 sample_offset;
	guint32 sample_size;
	TrakBox *trak = NULL;
	StscBox *stsc;
	StszBox *stsz;
	Stz2Box *stz2;
	StcoBox *stco;
	Co64Box *co64;
	SttsBox *stts;
	StblBox *stbl;

	LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (%s)\n", stream->GetTypeName ());

	for (guint32 i = 0; i < moov->trak_count; i++) {
		if (moov->trak [i]->stream == stream) {
			trak = moov->trak [i];
			break;
		}
	}

	g_return_if_fail (trak != NULL);

	stbl = trak->mdia->minf->stbl;
	stsc = trak->mdia->minf->stbl->stsc;
	stsz = trak->mdia->minf->stbl->stsz;
	stz2 = trak->mdia->minf->stbl->stz2;
	stco = trak->mdia->minf->stbl->stco;
	co64 = trak->mdia->minf->stbl->co64;
	stts = trak->mdia->minf->stbl->stts;

	if (!GetSampleCount (stbl, &sample_count))
		return;

	if (!GetChunkCount (stbl, &chunk_count))
		return;

	if (trak->current_sample >= sample_count) {
		LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (): no more frames, sample_count: %u current sample: %u\n", sample_count, trak->current_sample);
		/* No more frames for this stream */
		ReportGetFrameCompleted (NULL);
		return;
	}

	chunk = stsc->first_chunk [trak->stsc_index] + trak->stsc_sample_index / stsc->samples_per_chunk [trak->stsc_index];
	chunk_sample = trak->stsc_sample_index % stsc->samples_per_chunk [trak->stsc_index];

	if (chunk < 1) {
		ReportErrorOccurred ("Mp4Demuxer: corrupted mp4 file (invalid first chunk in first stsc entry)");
		return;
	}

	/* chunk is 1-based in the stsc->first_chunk array, while we need it to be 0 based */
	chunk--;

	/* Get the offset of the chunk we want */
	if (!GetChunkOffset (stbl, chunk, &chunk_offset))
		return;

	/* Calculate sample size */
	if (!GetSampleSize (stbl, trak->current_sample, &sample_size))
		return;

	/* Calculate the sample offset */
	sample_offset = chunk_offset + trak->stsc_samples_size;

	/* Check if we have buffered enough data */
	if (!(buffer_position <= sample_offset && buffer_position + buffer->GetSize () >= sample_offset + sample_size)) {
		Media *media = GetMediaReffed ();
		if (media != NULL) {
			if (get_frame_stream != NULL)
				get_frame_stream->unref ();
			get_frame_stream = stream;
			get_frame_stream->ref ();
			
			MediaReadClosure *closure = new MediaReadClosure (media, ReadSampleDataAsyncCallback, this, sample_offset, MAX (sample_size, 4096));
			source->ReadAsync (closure);
			closure->unref ();
			media->unref ();
		}
		LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (%s): not enough data buffered, requesting more.\n", stream->GetTypeName ());
		return;
	}

	/* Calculate pts */
	guint64 pts = 0;
	guint64 time = 0;
	guint32 samples_left = trak->current_sample;
	/* Note that in most cases the stts table is composed of a single entry for all samples, so caching values would likely not be faster */
	for (guint32 i = 0; i < stts->entry_count && samples_left > 0; i++) {
		if (stts->sample_count [i] >= samples_left) {
			time += stts->sample_delta [i] * samples_left;
			break;
		} else {
			time += stts->sample_delta [i] * stts->sample_count [i];
			samples_left -= stts->sample_count [i];
		}
	}
	pts = ToPts (time, trak);
	
	/* We now have what we need (pts, sample offset and sample size), create the frame */
	buffer->SeekSet (sample_offset - buffer_position);

	LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (%s): sample %u at offset %" G_GUINT64_FORMAT " and size %u (chunk index: %u, stsc index: %u) pts: %" G_GUINT64_FORMAT " ms\n",
		stream->GetTypeName (), trak->current_sample, sample_offset, sample_size, trak->current_sample, trak->stsc_index, MilliSeconds_FromPts (pts));

	MediaFrame *frame = new MediaFrame (stream);
	frame->pts = pts;
	if (!frame->AllocateBuffer (sample_size)) {
		frame->unref ();
		ReportErrorOccurred ("Mp4Demuxer: error while allocating frame buffer");
		goto cleanup;
	}
	if (!buffer->Read (frame->GetBuffer (), sample_size)) {
		frame->unref ();
		ReportErrorOccurred ("Mp4Demuxer: error while reading frame data");
		goto cleanup;
	}
	frame->AddState (MediaFrameDemuxed);
	
	/* Frame read successfully, advance to the next frame */
	trak->current_sample++;
	trak->stsc_sample_index++;
	trak->stsc_samples_size += sample_size;
	if (trak->stsc_sample_index >= trak->stsc_sample_count) {
		/* Advance to next stsc entry */
		trak->stsc_index++;
		trak->stsc_sample_index = 0;
		trak->stsc_samples_size = 0;
		
		/* Find out how many samples in next stsc entry */
		if (trak->stsc_index >= stsc->entry_count) {
			LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (): reached end of stsc table.\n");
			trak->current_sample = G_MAXUINT32;
		} else if (trak->stsc_index + 1 == stsc->entry_count) {
			/* last stsc entry */
			if (chunk_count < stsc->first_chunk [trak->stsc_index]) {
				/* Corrupted file */
				LOG_MP4 ("Mp4Demuxer::GetFrameAsyncInternal (): corrupted mp4 file, too many chunks in stsc table (last entry has %u has first chunk, chunk_count = %u).\n",
					stsc->first_chunk [trak->stsc_index], chunk_count);
				trak->current_sample = G_MAXUINT32;
			} else {
				trak->stsc_sample_count = (chunk_count - stsc->first_chunk [trak->stsc_index]) * stsc->samples_per_chunk [trak->stsc_index];
			}
		} else {
			/* We check that first_chunk are sequential when parsing, so the subtraction can't end up with a negative answer here */
			trak->stsc_sample_count = (stsc->first_chunk [trak->stsc_index + 1] - stsc->first_chunk [trak->stsc_index]) * stsc->samples_per_chunk [trak->stsc_index];
		}
	} else if (trak->stsc_sample_index % stsc->samples_per_chunk [trak->stsc_index] == 0) {
		trak->stsc_samples_size = 0;
	}

	ReportGetFrameCompleted (frame);

cleanup:
	if (frame)
		frame->unref ();
}

MediaResult
Mp4Demuxer::ReadSampleDataAsyncCallback (MediaClosure *closure)
{
	((Mp4Demuxer *) closure->GetContext ())->ReadSampleDataAsync ((MediaReadClosure *) closure);
	return MEDIA_SUCCESS;
}

MediaResult
Mp4Demuxer::ReadHeaderDataAsyncCallback (MediaClosure *closure)
{
	((Mp4Demuxer *) closure->GetContext ())->ReadHeaderDataAsync ((MediaReadClosure *) closure);
	return MEDIA_SUCCESS;
}

void
Mp4Demuxer::RequestMoreHeaderData (guint32 size)
{
	Media *media;
	guint32 count = size;

	if (buffer != NULL) {
		count = size + buffer->GetSize ();
		buffer->unref ();
		buffer = NULL;
	}

	media = GetMediaReffed ();
	if (media == NULL) {
		/* Not much to do here, we've probably been disposed */
		return;
	}
	
	LOG_MP4 ("MpDemuxer::RequestMoreHeaderData (%u): requesting %u bytes.\n", size, count);
	MediaReadClosure *closure = new MediaReadClosure (media, ReadHeaderDataAsyncCallback, this, 0, count);
	source->ReadAsync (closure);
	media->unref ();
	closure->unref ();
}

void
Mp4Demuxer::ReadSampleDataAsync (MediaReadClosure *closure)
{
	VERIFY_MEDIA_THREAD;
	if (buffer != NULL) {
		buffer->unref ();
		buffer = NULL;
	}
	buffer = closure->GetData ();
	buffer->ref ();
	buffer_position = closure->GetOffset ();
	last_buffer = closure->GetCount () != closure->GetData ()->GetSize ();
	
	if (get_frame_stream != NULL) {
		IMediaStream *str = get_frame_stream;
		get_frame_stream = NULL;
		GetFrameAsyncInternal (str);
		str->unref ();
	}
}

void
Mp4Demuxer::ReadHeaderDataAsync (MediaReadClosure *closure)
{
	VERIFY_MEDIA_THREAD;
	if (buffer != NULL) {
		buffer->unref ();
		buffer = NULL;
	}
	buffer = closure->GetData ();
	buffer->ref ();
	buffer_position = closure->GetOffset ();
	last_buffer = closure->GetCount () != closure->GetData ()->GetSize ();
	OpenDemuxerAsyncInternal ();
}

guint64
Mp4Demuxer::ToPts (guint64 time, TrakBox *trak)
{
	/* input:  'timescale' units per second */
	/* result: 10.000.000 units per second = 100-nano second units */
	return (guint64) ((double) time * (double) 10000000 / (double) trak->mdia->mdhd->timescale);
}

guint64
Mp4Demuxer::FromPts (guint64 pts, TrakBox *trak)
{
	/* input:  'timescale' units per second */
	/* result: 10.000.000 units per second = 100-nano second units */
	return (guint64) ((double) pts * (double) trak->mdia->mdhd->timescale / (double) 10000000);
}

bool
Mp4Demuxer::GetChunkCount (StblBox *stbl, guint32 *result)
{
	if (stbl->stco != NULL) {
		*result = stbl->stco->entry_count;
		return true;
	}

	if (stbl->co64 != NULL) {
		*result = stbl->co64->entry_count;
		return true;
	}

	ReportErrorOccurred ("Mp4Demuxer: corrupted mp4 file, either a stco box or a co64 box is required");
	return false;
}

bool
Mp4Demuxer::GetChunkOffset (StblBox *stbl, guint32 chunk_index, guint64 *result)
{
	/* Get the offset of the chunk we want */
	if (stbl->stco != NULL) {
		*result = stbl->stco->chunk_offset [chunk_index];
		return true;
	} else if (stbl->co64 != NULL) {
		*result = stbl->co64->chunk_offset [chunk_index];
		return true;
	}

	ReportErrorOccurred ("Mp4Demuxer: corrupted mp4 file, either a stco box or a co64 box is required");
	return false;
}

bool
Mp4Demuxer::GetSampleCount (StblBox *stbl, guint32 *result)
{
	if (stbl->stsz != NULL) {
		*result = stbl->stsz->sample_count;
		return true;
	}

	if (stbl->stz2 != NULL) {
		*result = stbl->stz2->sample_count;
		return true;
	}

	ReportErrorOccurred ("Mp4Demuxer: corrupted mp4 file, either a stsz box or a stz2 box is required");
	return false;
}

bool
Mp4Demuxer::GetSampleSize (StblBox *stbl, guint32 sample_index, guint32 *result)
{
	StszBox *stsz = stbl->stsz;
	Stz2Box *stz2 = stbl->stz2;

	if (stsz != NULL) {
		g_return_val_if_fail (sample_index < stsz->sample_count, 0);
		if (stsz->sample_size != 0) {
			*result = stsz->sample_size;
			return true;
		} else if (sample_index < stsz->sample_count) {
			*result = stsz->entry_size [sample_index];
			return true;
		} else {
			char *msg = g_strdup_printf ("Mp4Demuxer: corrupted mp4 file, invalid sample index %u (>=sample count %u)", sample_index, stsz->sample_count);
			ReportErrorOccurred (msg);
			g_free (msg);
			return false;
		}
	}

	if (stz2 != NULL) {
		if (sample_index >= stsz->sample_count) {
			char *msg = g_strdup_printf ("Mp4Demuxer: corrupted mp4 file, invalid sample index %u (>=sample count %u)", sample_index, stsz->sample_count);
			ReportErrorOccurred (msg);
			g_free (msg);
			return false;
		}

		switch (stz2->field_size) {
		case 4: {
			guint32 si = sample_index;
			guint8 b = stz2->samples [si / 2];
			if (si % 2 == 0) {
				*result = b >> 4;
			} else {
				*result = b & 0x0F;
			}
			return true;
		}
		case 8:
			*result = stz2->samples [sample_index];
			return true;
		case 16:
			*result = ((guint16 *) stz2->samples) [sample_index];
			return true;
		default:
			ReportErrorOccurred ("Mp4Demuxer: invalid field size in stz2");
			return false;
		}
	}

	ReportErrorOccurred ("Mp4Demuxer: corrupted mp4 file, either a stsz box or a stz2 box is required");
	return false;
}

bool
Mp4Demuxer::OpenMoov ()
{
	IMediaStream **streams = 0;
	int stream_count = 0;
	bool result = false;
	Media *media = NULL;

	g_return_val_if_fail (moov != NULL && moov->parsed, false);

	media = GetMediaReffed ();
	if (media == NULL) {
		/* We probably got disposed */
		LOG_MP4 ("Mp4Demuxer::OpenMoov (): no media. IsDisposed: %i\n", IsDisposed ());
		return false;
	}

	streams = (IMediaStream **) g_malloc0 (sizeof (IMediaStream *) * moov->trak_count);
	for (guint32 i = 0; i < moov->trak_count; i++) {
		TrakBox *trak = moov->trak [i];
		HdlrBox *hdlr = trak->mdia->hdlr;
		MdhdBox *mdhd = trak->mdia->mdhd;
		StblBox *stbl = trak->mdia->minf->stbl;
		StsdBox *stsd = stbl->stsd;
		SampleEntry *entry;
		AudioSampleEntry *audio_entry;
		VisualSampleEntry *visual_entry;
		IMediaStream *stream;
		AudioStream *audio;
		VideoStream *video;

		LOG_MP4 ("Mp4Demuxer::OpenMoov () trak #%i has %i sample entries.\n", i, stsd->entry_count);
		if (stsd->entry_count == 0) {
			char *msg = g_strdup_printf ("Mp4Demuxer: trak #%i has no sample entries", i);
			ReportErrorOccurred (msg);
			g_free (msg);
			goto cleanup;
		}

		entry = stsd->entries [0];

		switch (hdlr->handler_type) {
		case SOUN_4CC:
			audio = new AudioStream (media);
			stream = audio;
			audio_entry = (AudioSampleEntry *) entry;
			audio->SetChannels (audio_entry->channelcount);
			audio->SetSampleRate (audio_entry->samplerate.hi);
			if (audio_entry->samplerate.lo != 0) {
				printf ("Moonlight: mp4 media specifies a fractional sample rate (%f), which isn't supported by moonlight. The fractional part of the sample rate will be ignored.\n", audio_entry->samplerate.ToDouble ());
			}
			audio->SetBitsPerSample (audio_entry->samplesize);
			audio->SetBitRate (0);
			audio->SetBlockAlign (0);
			break;
		case VIDE_4CC:
			video = new VideoStream (media);
			stream = video;
			visual_entry = (VisualSampleEntry *) entry;
			video->SetWidth (visual_entry->width);
			video->SetHeight (visual_entry->height);
			break;
		default:
			LOG_MP4 ("Mp4Demuxer::OpenMoov (): unsupported track type: " FORMAT_4CC "' (%u) %s\n", VALUES_4CC(hdlr->handler_type), hdlr->handler_type, hdlr->name);
			continue;
		}
		
		trak->stream = stream;
		trak->stream->ref ();
		streams [stream_count] = stream;
		stream_count++;

		if (entry->extradata_size != 0) {
			stream->SetExtraDataSize (entry->extradata_size);
			stream->SetExtraData (g_memdup (entry->extradata, entry->extradata_size));
		}
		stream->SetCodecId (GINT32_FROM_BE (entry->type));
		stream->SetDuration (ToPts (mdhd->duration, trak));
	}

	result = true;
	SetStreams (streams, stream_count);

cleanup:
	for (guint32 i = 0; i < moov->trak_count; i++) {
		if (streams [i] != NULL)
			streams [i]->unref ();
	}
	if (!result)
		g_free (streams);

	if (media != NULL)
		media->unref ();

	return result;
}

void
Mp4Demuxer::OpenDemuxerAsyncInternal ()
{
	MemoryBuffer *source = buffer;
	guint64 size;
	guint32 type;
	guint32 major_brand;
	guint32 minor_brand;
	guint32 compatible_brand;
	bool result;
	
	LOG_MP4 ("Mp4Demuxer::OpenDemuxerAsyncInternal () %" G_GINT64_FORMAT " bytes available.\n", source->GetSize ());
	
	if (!ReadBox (&size, &type))
		return;

	/* Check for "ISO base media file format" as specified in ISO/IEC 14496-12 */
	if (type != FTYP_4CC) {
		ReportErrorOccurred ("Mp4Demuxer could not find 'ftyp' box");
		return;
	}

	/* We check for 'avc1', 'mp41' and 'mp42' major brand or compatible brands. The MP4 file specification (ISO/IEC 14496-14)
	 * states that the types 'mp41' or 'mp42' shall appear in the list of compatible-brands in the ftyp box, though SL plays 
	 * 'avc1' files too. */
	major_brand = source->ReadBE_U32 ();
	minor_brand = source->ReadBE_U32 ();

	LOG_MP4 ("Mp4DemuxerInfo::OpenDemuxerAsyncInternal (): major brand: " FORMAT_4CC " (%u), minor brand: " FORMAT_4CC " (%u)\n", VALUES_4CC (major_brand), major_brand, VALUES_4CC (minor_brand), minor_brand);

	result = IsCompatibleType (major_brand);
	while (size > (guint64) source->GetPosition ()) {
		if (source->GetRemainingSize () < 4) {
			RequestMoreHeaderData ();
			return;
		}
		compatible_brand = source->ReadBE_U32 ();
		result = result || IsCompatibleType (compatible_brand);
		LOG_MP4 (" got compatible brand: " FORMAT_4CC " (%u)\n", VALUES_4CC (compatible_brand), compatible_brand);
	}

	LOG_MP4 ("Mp4DemuxerInfo::OpenDemuxerAsyncInternal (): result: %i\n", result);

	if (!result) {
		/* This shouldn't happen, since Mp4DemuxerInfo::Supports should return false */
		ReportErrorOccurred ("Mp4Demuxer could not find any of the required brands ('avc1', 'mp41' or 'mp42') in the ftyp box");
		return;
	}

	while (true) {
		guint64 sub_start = source->GetPosition ();
		guint64 sub_size;
		guint32 sub_type;

		if ((guint64) source->GetRemainingSize () < 28 /* max amount of data ReadBox can read */) {
			RequestMoreHeaderData ();
			return;
		}

		if (!ReadBox (&sub_size, &sub_type)) {
			/* This shouldn't happen, in any case ReadBox has already called ReportErrorOccurred */
			return;
		}

		if ((guint64) source->GetRemainingSize () < sub_size - (source->GetPosition () - sub_start)) {
			RequestMoreHeaderData (MAX (1024, sub_size - (source->GetPosition () - sub_start)));
			return;
		}

		switch (sub_type) {
		case MOOV_4CC:
			LOG_MP4 ("Mp4DemuxerInfo::OpenDemuxerAsyncInternal (): reading 'moov' box.\n");
			if (!ReadMoov (sub_type, sub_start, sub_size))
				return;

			if (OpenMoov ())
				ReportOpenDemuxerCompleted ();

			/* Nothing more to do here */
			return;
		default:
			LOG_MP4 ("Mp4DemuxerInfo::OpenDemuxerAsyncInternal (): unknown top-level type " FORMAT_4CC " (%u).\n", VALUES_4CC (sub_type), sub_type);
			break;
		}

		source->SeekSet (sub_start + sub_size);
	}
}

bool
Mp4Demuxer::ReadBox (guint64 *size, guint32 *type)
{
	MemoryBuffer *source = buffer;

	VERIFY_AVAILABLE_SIZE (8);

	*size = source->ReadBE_U32 ();
	*type = source->ReadBE_U32 ();

	if (*size == 1) {
		VERIFY_AVAILABLE_SIZE (8);
		*size = source->ReadBE_U64 ();
	}

	if (*type == UUID_4CC) {
		VERIFY_AVAILABLE_SIZE (16);
		source->SeekOffset (16);
	}

	LOG_MP4 ("Mp4Demuxer::ReadBox (): read box " FORMAT_4CC " (%u) with size %" G_GUINT64_FORMAT "\n", VALUES_4CC (*type), *type, *size);

	return true;
}

bool
Mp4Demuxer::ReadFullBox (FullBox *box)
{
	MemoryBuffer *source = buffer;

	VERIFY_AVAILABLE_SIZE (4);

	box->version = source->ReadBE_U8 ();
	box->flags = source->ReadBE_U24 ();

	return true;
}

bool
Mp4Demuxer::ReadMoov (guint32 type, guint64 start, guint64 size)
{
	MemoryBuffer *source = buffer;

	LOG_MP4 ("Mp4Demuxer::ReadMoov (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_AVAILABLE_POSITION (start + size);

	if (moov != NULL) {
		if (moov->parsed) {
			/* We have already parsed this box, skip it */
			source->SeekSet (start + size);
			return true;
		} else {
			/* We have a moov box, but didn't parse it correctly? This shouldn't really happen, since ReportErrorOccurred should have been called, preventing any further calls to us */
			ReportErrorOccurred ("Mp4Demuxer: error while parsing 'moov' object");
			return false;
		}
	} else {
		moov = new MoovBox (type, size);
	}

	if (!ReadLoop (start, size, moov))
		return false;

	VERIFY_REQUIRED_BOX (moov, mvhd);

	if (moov->trak == NULL || moov->trak_count == 0) {
		ReportErrorOccurred ("Mp4Demuxer: no 'trak' boxes found in the 'moov' box");
		return false;
	}

	moov->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadMoov (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") %u traks found [Done]\n", start, size, moov->trak_count);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadLoop (guint64 start, guint64 size, Box *container)
{
	MemoryBuffer *source = buffer;
	guint64 sub_start;
	guint64 sub_size;
	guint32 sub_type;
	bool handled;

	VERIFY_AVAILABLE_POSITION (start + size);

	while (start + size > (guint64) source->GetPosition ()) {
		sub_start = source->GetPosition ();

		if (!ReadBox (&sub_size, &sub_type))
			return false;

		VERIFY_AVAILABLE_POSITION (sub_start + sub_size);

		if (sub_size == 0) {
			/* We need to report this error, otherwise we'll go into an infinite loop */
			char *msg = g_strdup_printf ("Corrupted mp4 file at position: %" G_GUINT64_FORMAT "(box size in " FORMAT_4CC " box is 0)", sub_start, VALUES_4CC (container->type));
			ReportErrorOccurred (msg);
			g_free (msg);
			return false;
		}

		handled = true;
		switch (container->type) {
		case MOOV_4CC:
			switch (sub_type) {
			case MVHD_4CC:
				if (!ReadMvhd (sub_type, sub_start, sub_size, (MoovBox *) container))
					return false;
				break;
			case TRAK_4CC:
				if (!ReadTrak (sub_type, sub_start, sub_size, (MoovBox *) container))
					return false;
				break;
			default:
				handled = false;
				break;
			}
			break;
		case TRAK_4CC:
			switch (sub_type) {
			case TKHD_4CC:
				if (!ReadTkhd (sub_type, sub_start, sub_size, (TrakBox *) container))
					return false;
				break;
			case MDIA_4CC:
				if (!ReadMdia (sub_type, sub_start, sub_size, (TrakBox *) container))
					return false;
				break;
			default:
				handled = false;
				break;
			}
			break;
		case MDIA_4CC:
			switch (sub_type) {
			case MDHD_4CC:
				if (!ReadMdhd (sub_type, sub_start, sub_size, (MdiaBox *) container))
					return false;
				break;
			case HDLR_4CC:
				if (!ReadHdlr (sub_type, sub_start, sub_size, (MdiaBox *) container))
					return false;
				break;
			case MINF_4CC:
				if (!ReadMinf (sub_type, sub_start, sub_size, (MdiaBox *) container))
					return false;
				break;
			default:
				handled = false;
				break;
			}
			break;
		case MINF_4CC:
			switch (sub_type) {
			case VMHD_4CC:
				if (!ReadVmhd (sub_type, sub_start, sub_size, (MinfBox *) container))
					return false;
				break;
			case SMHD_4CC:
				if (!ReadSmhd (sub_type, sub_start, sub_size, (MinfBox *) container))
					return false;
				break;
			case STBL_4CC:
				if (!ReadStbl (sub_type, sub_start, sub_size, (MinfBox *) container))
					return false;
				break;
			case DINF_4CC:
				if (!ReadDinf (sub_type, sub_start, sub_size, (MinfBox *) container))
					return false;
				break;
			case HMHD_4CC:
			case NMHD_4CC:
			default:
				handled = false;
				break;
			}
			break;
		case DINF_4CC:
			switch (sub_type) {
			case DREF_4CC:
				if (!ReadDref (sub_type, sub_start, sub_size, (DinfBox *) container))
					return false;
				break;
			case URL_4CC:
			case URN_4CC:
			default:
				handled = false;
				break;
			}
			break;
		case STBL_4CC:
			switch (sub_type) {
			case STTS_4CC:
				if (!ReadStts (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case CTTS_4CC:
				if (!ReadCtts (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case STSD_4CC:
				if (!ReadStsd (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case STSC_4CC:
				if (!ReadStsc (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case STSZ_4CC:
				if (!ReadStsz (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case STCO_4CC:
				if (!ReadStco (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case CO64_4CC:
				if (!ReadCo64 (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			case STSS_4CC:
				if (!ReadStss (sub_type, sub_start, sub_size, (StblBox *) container))
					return false;
				break;
			default:
				handled = false;
				break;
			}
			break;
		case MP4A_4CC:
		case MP4V_4CC:
		case AVC1_4CC:
			switch (sub_type) {
			case ESDS_4CC:
				if (!ReadEsds (sub_type, sub_start, sub_size, (SampleEntry *) container))
					return false;
				break;
			case AVCC_4CC:
				if (!ReadAvcc (sub_type, sub_start, sub_size, (SampleEntry *) container))
					return false;
				break;
			default:
				handled = false;
				break;
			}
			break;
		default:
			LOG_MP4 ("Mp4Demuxer::ReadContainer (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") unknown container type: " FORMAT_4CC " (%u)\n", start, size, VALUES_4CC (container->type), container->type);
			break;
		}

		if (!handled) {
			LOG_MP4 ("Mp4Demuxer::ReadContainer (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") unknown type: " FORMAT_4CC " (%u) for container type " FORMAT_4CC " (%u). Skipping it (current position: %" G_GUINT64_FORMAT ").\n",
				start, size, VALUES_4CC (sub_type), sub_type, VALUES_4CC (container->type), container->type, source->GetPosition ());
		}

		/* Skip whatever data is left */
		source->SeekSet (sub_start + sub_size);
	}

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadMvhd (guint32 type, guint64 start, guint64 size, MoovBox *moov)
{
	MemoryBuffer *source = buffer;
	MvhdBox *mvhd;

	LOG_MP4 ("Mp4Demuxer::ReadMvhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (moov, mvhd);

	moov->mvhd = new MvhdBox (type, size);
	mvhd = moov->mvhd;

	if (!ReadFullBox (mvhd))
		return false;

	if (mvhd->version == 0) {
		VERIFY_BOX_SIZE (24 * 4, "mvhd");
		source->ReadBE_U32 (); /* creation time */
		source->ReadBE_U32 (); /* modification time */
		mvhd->timescale = source->ReadBE_U32 ();
		mvhd->duration = source->ReadBE_U32 ();
	} else if (mvhd->version == 1) {
		VERIFY_BOX_SIZE (27 * 4, "mvhd");
		source->ReadBE_U64 (); /* creation time */
		source->ReadBE_U64 (); /* modification time */
		mvhd->timescale = source->ReadBE_U32 ();
		mvhd->duration = source->ReadBE_U64 ();
	} else {
		char *msg = g_strdup_printf ("Corrupted mp4 file at position: %" G_GUINT64_FORMAT " (invalid mvhd version: %i)", source->GetPosition (), mvhd->version);
		ReportErrorOccurred (msg);
		g_free (msg);
		return false;
	}
	source->ReadBE_U32 (); /* rate */
	source->ReadBE_U32 (); /* volume */
	source->ReadBE_U16 (); /* reserved */
	source->ReadBE_U32 (); /* reserved [0] */
	source->ReadBE_U32 (); /* reserved [1] */
	for (int i = 0; i < 9; i++) {
		source->ReadBE_U32 (); /* matrix [i] */
	}
	for (int i = 0; i < 6; i++) {
		source->ReadBE_U32 (); /* pre_defined [i] */
	}
	source->ReadBE_U32 (); /* next_track_ID */

	mvhd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadMvhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") timescale: %u, duration: %" G_GUINT64_FORMAT " [Done]\n", start, size, mvhd->timescale, mvhd->duration);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadTrak (guint32 type, guint64 start, guint64 size, MoovBox *moov)
{
	MemoryBuffer *source = buffer;
	TrakBox *trak;

	LOG_MP4 ("Mp4Demuxer::ReadTrak (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	moov->trak_count++;
	moov->trak = (TrakBox **) g_realloc (moov->trak, moov->trak_count * sizeof (TrakBox *));
	moov->trak [moov->trak_count - 1] = new TrakBox (type, size);

	trak = moov->trak [moov->trak_count - 1];

	if (!ReadLoop (start, size, trak))
		return false;

	VERIFY_REQUIRED_BOX (trak, tkhd);
	VERIFY_REQUIRED_BOX (trak, mdia);

	trak->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadTrak (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")[Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadTkhd (guint32 type, guint64 start, guint64 size, TrakBox *trak)
{
	MemoryBuffer *source = buffer;
	TkhdBox *tkhd;

	LOG_MP4 ("Mp4Demuxer::ReadTkhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (trak, tkhd);

	trak->tkhd = new TkhdBox (type, size);
	tkhd = trak->tkhd;

	if (!ReadFullBox (tkhd))
		return false;

	if (tkhd->version == 0) {
		VERIFY_BOX_SIZE (20 * 4, "tkhd");
		source->ReadBE_U32 (); /* creation time */
		source->ReadBE_U32 (); /* modification time */
		tkhd->track_ID = source->ReadBE_U32 ();
		source->ReadBE_U32 (); /* reserved */
		tkhd->duration = source->ReadBE_U32 ();
	} else if (tkhd->version == 1) {
		VERIFY_BOX_SIZE (23 * 4, "tkhd");
		source->ReadBE_U64 (); /* creation time */
		source->ReadBE_U64 (); /* modification time */
		tkhd->track_ID = source->ReadBE_U32 ();
		source->ReadBE_U32 (); /* reserved */
		tkhd->duration = source->ReadBE_U64 ();
	} else {
		char *msg = g_strdup_printf ("Corrupted mp4 file at position: %" G_GUINT64_FORMAT " (invalid tkhd version: %i)", source->GetPosition (), tkhd->version);
		ReportErrorOccurred (msg);
		g_free (msg);
		return false;
	}

	source->ReadBE_U32 (); /* reserved [0] */
	source->ReadBE_U32 (); /* reserved [1] */
	source->ReadBE_U16 (); /* layer */
	source->ReadBE_U16 (); /* alternate_group */
	source->ReadBE_U16 (); /* volume */
	source->ReadBE_U16 (); /* reserved */
	for (int i = 0; i < 9; i++) {
		source->ReadBE_U32 (); /* matrix [i] */
	}
	tkhd->width.hi = source->ReadBE_U16 ();
	tkhd->width.lo = source->ReadBE_U16 ();
	tkhd->height.hi = source->ReadBE_U16 ();
	tkhd->height.lo = source->ReadBE_U16 ();

	tkhd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadTkhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") duration: %" G_GUINT64_FORMAT " track_id: %u width: %f height: %f [Done]\n",
		start, size, tkhd->duration, tkhd->track_ID, tkhd->width.ToDouble (), tkhd->height.ToDouble ());

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadMdia (guint32 type, guint64 start, guint64 size, TrakBox *trak)
{
	MdiaBox *mdia;

	LOG_MP4 ("Mp4Demuxer::ReadMdia (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (trak, mdia);

	trak->mdia = new MdiaBox (type, size);
	mdia = trak->mdia;

	if (!ReadLoop (start, size, mdia))
		return false;

	VERIFY_REQUIRED_BOX (mdia, mdhd);
	VERIFY_REQUIRED_BOX (mdia, hdlr);
	VERIFY_REQUIRED_BOX (mdia, minf);

	mdia->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadMdia (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	return true;
}

bool
Mp4Demuxer::ReadMinf (guint32 type, guint64 start, guint64 size, MdiaBox *mdia)
{
	MinfBox *minf;

	LOG_MP4 ("Mp4Demuxer::ReadMinf (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (mdia, minf);

	mdia->minf = new MinfBox (mdia, type, size);
	minf = mdia->minf;

	if (!ReadLoop (start, size, minf))
		return false;

	VERIFY_REQUIRED_BOX (minf, dinf);
	VERIFY_REQUIRED_BOX (minf, stbl);

	minf->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadMinf (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	return true;
}

bool
Mp4Demuxer::ReadDinf (guint32 type, guint64 start, guint64 size, MinfBox *minf)
{
	DinfBox *dinf;

	LOG_MP4 ("Mp4Demuxer::ReadDinf (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (minf, dinf);

	minf->dinf = new DinfBox (type, size);
	dinf = minf->dinf;

	if (!ReadLoop (start, size, dinf))
		return false;

	VERIFY_REQUIRED_BOX (dinf, dref);

	dinf->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadDinf (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	return true;
}

bool
Mp4Demuxer::ReadDref (guint32 type, guint64 start, guint64 size, DinfBox *dinf)
{
	MemoryBuffer *source = buffer;
	DrefBox *dref;

	LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (dinf, dref);
	
	dinf->dref = new DrefBox (type, size);
	dref = dinf->dref;

	if (!ReadFullBox (dref))
		return false;

	VERIFY_BOX_SIZE (4, "dref");

	dref->entry_count = source->ReadBE_U32 ();
	dref->entries = (FullBox **) g_malloc0 (sizeof (FullBox *) * dref->entry_count);

	VERIFY_BOX_SIZE (dref->entry_count * 8, "dref");

	LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, dref->entry_count);

	for (guint32 i = 0; i < dref->entry_count; i++) {
		guint64 sub_start = source->GetPosition ();
		guint64 sub_size;
		guint32 sub_type;
		guint32 left;

		if (!ReadBox (&sub_size, &sub_type))
			return false;

		VERIFY_BOX_SIZE (sub_size - (source->GetPosition () - sub_start), "dref");

		switch (sub_type) {
		case URN_4CC: {
			UrnBox *urn;
			urn = new UrnBox (sub_type, sub_size);
			dref->entries [i] = urn;
			if (!ReadFullBox (urn))
				return false;

			left = sub_size - (source->GetPosition () - sub_start);
			if (left > 0) {
				urn->name = source->ReadBE_UTF8 (left);
				left = sub_size - (source->GetPosition () - sub_start);
				if (left > 0) {
					urn->location = source->ReadBE_UTF8 (left);
				}
			}
			urn->parsed = true;
			LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") 'urn' entry, flags: %u name: '%s' location: '%s'\n", start, size, urn->flags, urn->name, urn->location);
			break;
		}
		case URL_4CC: {
			UrlBox *url;
			url = new UrlBox (sub_type, sub_size);
			dref->entries [i] = url;
			if (!ReadFullBox (url))
				return false;

			left = sub_size - (source->GetPosition () - sub_start);
			if (left > 0) {
				url->location = source->ReadBE_UTF8 (left);
			}
			url->parsed = true;
			LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") 'url' entry, flags: %u location: '%s'\n", start, size, url->flags, url->location);
			break;
		}
		default:
			LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") unknown entry type: " FORMAT_4CC " (%u)\n",
				start, size, VALUES_4CC (sub_type), sub_type);
			break;
		}
		/* Skip whatever we didn't consume */
		source->SeekSet (sub_start + sub_size);
	}

	dref->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadDref (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}


bool
Mp4Demuxer::ReadStbl (guint32 type, guint64 start, guint64 size, MinfBox *minf)
{
	MemoryBuffer *source = buffer;
	StblBox *stbl;

	LOG_MP4 ("Mp4Demuxer::ReadStbl (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (minf, stbl);

	minf->stbl = new StblBox (minf, type, size);
	stbl = minf->stbl;

	if (!ReadLoop (start, size, stbl))
		return false;

	VERIFY_REQUIRED_BOX (stbl, stsd);
	VERIFY_REQUIRED_BOX (stbl, stts);
	VERIFY_REQUIRED_BOX (stbl, stsc);

	if (stbl->stco == NULL && stbl->co64 == NULL) {
		ReportErrorOccurred ("Mp4Demuxer: the required box 'stco' or 'co64' in 'stbl' wasn't found");
		return false;
	}

	if (stbl->stsz == NULL && stbl->stz2 == NULL) {
		ReportErrorOccurred ("Mp4Demuxer: the required box 'stsz' or 'stz2' in 'stbl' wasn't found");
		return false;
	}

	stbl->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStbl (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStss (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	StssBox *stss;

	LOG_MP4 ("Mp4Demuxer::ReadStss (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stss);

	stbl->stss = new StssBox (type, size);
	stss = stbl->stss;

	if (!ReadFullBox (stss))
		return false;

	VERIFY_BOX_SIZE (4, "stss");

	stss->entry_count = source->ReadBE_U32 ();

	VERIFY_BOX_SIZE (stss->entry_count * 4, "stss");

	stss->sample_number = (guint32 *) g_malloc (sizeof (guint32) * stss->entry_count);
	for (guint32 i = 0; i < stss->entry_count; i++) {
		stss->sample_number [i] = source->ReadBE_U32 ();
		LOG_MP4 ("Mp4Demuxer::ReadStss (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") sample_number [%u] = %u\n", start, size, i, stss->sample_number [i]);
	}

	stss->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStts (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	SttsBox *stts;

	LOG_MP4 ("Mp4Demuxer::ReadStts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stts);

	stbl->stts = new SttsBox (type, size);
	stts = stbl->stts;

	if (!ReadFullBox (stts))
		return false;

	VERIFY_BOX_SIZE (4, "stts");

	stts->entry_count = source->ReadBE_U32 ();
	
	VERIFY_BOX_SIZE (stts->entry_count * 8, "stts");

	LOG_MP4 ("Mp4Demuxer::ReadStts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, stts->entry_count);

	stts->sample_count = (guint32 *) g_malloc (sizeof (guint32) * stts->entry_count);
	stts->sample_delta = (guint32 *) g_malloc (sizeof (guint32) * stts->entry_count);
	for (guint32 i = 0; i < stts->entry_count; i++) {
		stts->sample_count [i] = source->ReadBE_U32 ();
		stts->sample_delta [i] = source->ReadBE_U32 ();
	
		if (stts->sample_delta [i] == 0) {
			ReportErrorOccurred ("Mp4Demuxer: found invalid sample delta (0) in stts entry");
			return false;
		}

		LOG_MP4 ("Mp4Demuxer::ReadStts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry [%u]: sample_count: %u sample_delta: %u\n", start, size, i, stts->sample_count [i], stts->sample_delta [i]);
	}

	stts->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadCtts (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	CttsBox *ctts;

	LOG_MP4 ("Mp4Demuxer::ReadCtts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, ctts);

	stbl->ctts = new CttsBox (type, size);
	ctts = stbl->ctts;

	if (!ReadFullBox (ctts))
		return false;

	VERIFY_BOX_SIZE (4, "ctts");

	ctts->entry_count = source->ReadBE_U32 ();
	VERIFY_BOX_SIZE (ctts->entry_count * 8, "ctts")

	LOG_MP4 ("Mp4Demuxer::ReadCtts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, ctts->entry_count);

	ctts->sample_count = (guint32 *) g_malloc (sizeof (guint32) * ctts->entry_count);
	ctts->sample_offset = (guint32 *) g_malloc (sizeof (guint32) * ctts->entry_count);
	for (guint32 i = 0; i < ctts->entry_count; i++) {
		ctts->sample_count [i] = source->ReadBE_U32 ();
		ctts->sample_offset [i] = source->ReadBE_U32 ();
		//LOG_MP4 ("Mp4Demuxer::ReadCtts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry [%u]: sample_count: %u sample_delta: %u\n", start, size, i, ctts->sample_count [i], ctts->sample_offset [i]);
	}

	ctts->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadCtts (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadDescriptorLength (guint32 *length)
{
	MemoryBuffer *source = buffer;
	guint32 sizeByte;
	guint32 count = 4;
	bool nextByte = true;
	guint32 tmp;
	*length = 0;
	while (nextByte && count--) {
		tmp = source->ReadBE_U8 ();
		nextByte = tmp & 0x80;
		sizeByte = tmp & 0x7f;
		*length = (*length << 7) | sizeByte;
	}
	return true;
}

bool
Mp4Demuxer::ReadEsds (guint32 type, guint64 start, guint64 size, SampleEntry *entry)
{
	MemoryBuffer *source = buffer;
	EsdsBox *esds;
	guint8 tag;
	guint32 tmp;
	guint32 length;

	/* The esds box is defined in ISO/IEC 14496-14, the ES_Descriptor it contains is defined in ISO/IEC 14496-1 */

	LOG_MP4 ("Mp4Demuxer::ReadEsds (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (entry, esds);

	entry->esds = new EsdsBox (type, size);
	esds = entry->esds;

	if (!ReadFullBox (esds))
		return false;

	VERIFY_BOX_SIZE (1, "esds");

	tag = source->ReadBE_U8 ();
	if (tag == /*ES_DescrTag*/ 0x03) {
		if (!ReadDescriptorLength (&length))
			return false;
		VERIFY_BOX_SIZE (length, "esds");
		esds->ES_ID = source->ReadBE_U16 ();
		tmp = source->ReadBE_U8 ();
		esds->streamDependenceFlag = tmp & 0x1; /* 1 bit */
		esds->URL_Flag = (tmp & 0x2) >> 1; /* 1 bit */
		esds->OCRstreamFlag = (tmp & 0x4) >> 2; /* 1 bit */
		esds->streamPriority = tmp >> 3; /* 5 bits */
		if (esds->streamDependenceFlag)
			esds->dependsOn_ES_ID = source->ReadBE_U16 ();
		if (esds->URL_Flag) {
			esds->URLlength = source->ReadBE_U8 ();
			esds->URLstring = source->ReadBE_UTF8 (esds->URLlength);
		}
		if (esds->OCRstreamFlag)
			esds->OCR_ES_Id = source->ReadBE_U16 ();

		tag = source->ReadBE_U8 ();
		if (tag == /*DecoderConfigDescrTag*/ 0x04) {
			if (!ReadDescriptorLength (&length))
				return false;
			VERIFY_BOX_SIZE (length, "esds");
			esds->objectTypeIndication = source->ReadBE_U8 (); /* 8 bits */
			tmp = source->ReadBE_U8 ();
			esds->streamType = tmp & 0x2F; /* 6 bits */
			esds->upStream = (tmp & 0x40) >> 6; /* 1 bit */
			esds->bufferSizeDB = source->ReadBE_U24 (); /* 24 bits */
			esds->maxBitrate = source->ReadBE_U32 ();
			esds->avgBitrate = source->ReadBE_U32 ();
			tag = source->ReadBE_U8 ();
			if (tag == /*DecSpecificInfoTag*/ 0x05) {
				if (!ReadDescriptorLength (&length))
					return false;
				VERIFY_BOX_SIZE (length, "esds");
				entry->extradata_size = length;
				entry->extradata = source->Read (entry->extradata_size);
			}
	    }
	}
    
	esds->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadEsds (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") ES_Number: %u streamDependenceFlag: %u URL_Flag: %u OCRstreamFlag: %u URLlength: %u URLstring: %s, \n"
	"      dependsOn_ES_ID: %u objectTypeIndication: %u streamType: %u upStream: %u bufferSizeDB: %u maxBitrate: %u avgBitrate: %u extradata_size: %u [Done]\n",
		 start, size, esds->ES_ID, esds->streamDependenceFlag, esds->URL_Flag, esds->OCRstreamFlag, esds->URLlength, esds->URLstring,
		 esds->dependsOn_ES_ID, esds->objectTypeIndication, esds->streamType, esds->upStream, esds->bufferSizeDB, esds->maxBitrate, esds->avgBitrate, entry->extradata_size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadSampleEntry (SampleEntry *entry)
{
	MemoryBuffer *source = buffer;

	entry->format = TypeToString (entry->type);

	VERIFY_AVAILABLE_SIZE (8);

	for (int i = 0; i < 6; i++) {
		source->ReadBE_U8 (); /* reserved [i] */
	}
	entry->data_reference_index = source->ReadBE_U16 ();

	LOG_MP4 ("Mp4Demuxer::ReadSampleEntry (%" G_GUINT64_FORMAT ", %u): data_reference_index: %u format: %s\n", entry->size, entry->type, entry->data_reference_index, entry->format);

	return true;
}

bool
Mp4Demuxer::ReadAudioSampleEntry (AudioSampleEntry **entry)
{
	MemoryBuffer *source = buffer;
	AudioSampleEntry *audio;
	guint64 start = source->GetPosition ();
	guint32 type;
	guint64 size;

	if (!ReadBox (&size, &type))
		return false;

	audio = new AudioSampleEntry (type, size);
	*entry = audio;

	if (!ReadSampleEntry (*entry))
		return false;

	VERIFY_AVAILABLE_SIZE (20);

	for (int i = 0; i < 2; i++) {
		source->ReadBE_U32 (); /* reserved [i] */
	}
	audio->channelcount = source->ReadBE_U16 (); /* channel_count */
	audio->samplesize = source->ReadBE_U16 (); /* samplesize */
	source->ReadBE_U16 (); /* pre_defined */
	source->ReadBE_U16 (); /* reserved */
	audio->samplerate.hi = source->ReadBE_U16 ();
	audio->samplerate.lo = source->ReadBE_U16 ();

	audio->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadSoundSampleEntry () channel_count: %u samplesize: %u samplerate: %f\n",
		audio->channelcount, audio->samplesize, audio->samplerate.ToDouble ());

	if (!ReadLoop (start, size, audio))
		return false;

	/* Skip whatever data is left */
	source->SeekSet (start + audio->size);

	return true;
}

bool
Mp4Demuxer::ReadVisualSampleEntry (VisualSampleEntry **entry)
{
	MemoryBuffer *source = buffer;
	VisualSampleEntry *visual;
	guint64 start = source->GetPosition ();
	guint32 name_length;
	gint64 name_start;
	guint32 type;
	guint64 size;

	if (!ReadBox (&size, &type))
		return false;

	visual = new VisualSampleEntry (type, size);
	*entry = visual;

	if (!ReadSampleEntry (*entry))
		return false;

	VERIFY_AVAILABLE_SIZE (70);

	source->ReadBE_U16 (); /* pre_defined */
	source->ReadBE_U16 (); /* reserved */
	for (int i = 0; i < 3; i++) {
		source->ReadBE_U32 (); /* pre_defined [i] */
	}
	visual->width = source->ReadBE_U16 ();
	visual->height = source->ReadBE_U16 ();
	visual->horizontal_resolution.hi = source->ReadBE_U16 ();
	visual->horizontal_resolution.lo = source->ReadBE_U16 ();
	visual->vertical_resolution.hi = source->ReadBE_U16 ();
	visual->vertical_resolution.lo = source->ReadBE_U16 ();
	source->ReadBE_U32 (); /* reserved */
	source->ReadBE_U16 (); /* frame count */
	name_length = source->ReadBE_U8 ();
	name_start = source->GetPosition ();
	if (name_length > 0 && name_length <= 31) {
		visual->compressor_name = source->ReadBE_UTF8 (name_length);
	}
	source->SeekSet (name_start + 31);
	source->ReadBE_U16 (); /* depth */
	source->ReadBE_I16 (); /* pre_defined */

	if (!ReadLoop (start, size, visual))
		return false;

	visual->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadVisualSampleEntry () width: %u height: %u hres: %f vres: %f name: '%s'\n",
		visual->width, visual->height, visual->horizontal_resolution.ToDouble (), visual->vertical_resolution.ToDouble (), visual->compressor_name);

	/* Skip whatever data is left */
	source->SeekSet (start + visual->size);

	return true;
}

bool
Mp4Demuxer::ReadAvcc (guint32 type, guint64 start, guint64 size, SampleEntry *entry)
{
	MemoryBuffer *source = buffer;

	entry->extradata_size = size - (source->GetPosition () - start);
	if (entry->extradata_size > 0)
		entry->extradata = source->Read (entry->extradata_size);

	LOG_MP4 ("Mp4Demuxer::ReadAvcc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") read %u extradata bytes\n", start, size, entry->extradata_size);

	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStsd (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	StsdBox *stsd;

	LOG_MP4 ("Mp4Demuxer::ReadStsd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stsd);

	stbl->stsd = new StsdBox (stbl, type, size);
	stsd = stbl->stsd;

	if (!ReadFullBox (stsd))
		return false;

	VERIFY_BOX_SIZE (4, "stsd");

	stsd->entry_count = source->ReadBE_U32 ();

	VERIFY_BOX_SIZE (stsd->entry_count * 8 /* minimum sub box size */, "stsd");

	LOG_MP4 ("Mp4Demuxer::ReadStsd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, stsd->entry_count);

	stsd->entries = (SampleEntry **) g_malloc0 (sizeof (SampleEntry *) * stsd->entry_count);

	if (stbl->parent->parent->hdlr == NULL) {
		ReportErrorOccurred ("Corrupted mp4 file, a 'hdlr' box must have been parsed when we reach a 'stsd' box");
		return false;
	}

	guint32 current_handler_type = stbl->parent /*minf*/ ->parent /*mdia*/ ->hdlr->handler_type;

	for (guint32 i = 0; i < stsd->entry_count; i++) {
		switch (current_handler_type) {
		case SOUN_4CC:
			if (!ReadAudioSampleEntry ((AudioSampleEntry **) &stsd->entries [i]))
				return false;
			break;
		case VIDE_4CC:
			if (!ReadVisualSampleEntry ((VisualSampleEntry **) &stsd->entries [i]))
				return false;
			break;
		default:
			/* Skip this entry */
			guint64 sub_size;
			guint32 sub_type;
			guint64 sub_start = source->GetPosition ();
			if (!ReadBox (&sub_size, &sub_type))
				return false;
			VERIFY_AVAILABLE_SIZE (sub_size);
			source->SeekSet (sub_start + sub_size);
			LOG_MP4 ("Mp4Demuxer::ReadStsd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT "): unknown handler type: " FORMAT_4CC " (%u) with a sample entry type: " FORMAT_4CC " (%u)\n",
				start, size, VALUES_4CC (current_handler_type), current_handler_type, VALUES_4CC (sub_type), sub_type);
			break;
		}
	}

	stsd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStsd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStsz (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	StszBox *stsz;

	LOG_MP4 ("Mp4Demuxer::ReadStsz (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stsz);

	stbl->stsz = new StszBox (type, size);
	stsz = stbl->stsz;

	if (!ReadFullBox (stsz))
		return false;

	VERIFY_BOX_SIZE (8, "stsz");

	stsz->sample_size = source->ReadBE_U32 ();
	stsz->sample_count = source->ReadBE_U32 ();

	LOG_MP4 ("Mp4Demuxer::ReadStsz (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") sample_size: %u sample_count: %u\n",
		start, size, stsz->sample_size, stsz->sample_count);

	if (stsz->sample_size == 0) {
		VERIFY_BOX_SIZE (stsz->sample_count * 4, "stsz");

		stsz->entry_size = (guint32 *) g_malloc (sizeof (guint32) * stsz->sample_count);
		for (guint32 i = 0; i < stsz->sample_count; i++) {
			stsz->entry_size [i] = source->ReadBE_U32 ();
			//LOG_MP4 ("Mp4Demuxer::ReadStsz (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") sample [%u] = %u\n", start, size, i, stsz->entry_size [i]);
		}
	}

	stsz->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStsz (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStz2 (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	Stz2Box *stz2;
	guint32 sample_size;

	LOG_MP4 ("Mp4Demuxer::ReadStz2 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stz2);

	stbl->stz2 = new Stz2Box (type, size);
	stz2 = stbl->stz2;

	if (!ReadFullBox (stz2))
		return false;

	VERIFY_BOX_SIZE (8, "stz2");

	source->SeekOffset (3); /* reserved */
	stz2->field_size = source->ReadBE_U8 ();
	stz2->sample_count = source->ReadBE_U32 ();

	LOG_MP4 ("Mp4Demuxer::ReadStz2 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") field_size: %u sample_count: %u\n",
		start, size, stz2->field_size, stz2->sample_count);

	switch (stz2->field_size) {
	case 4:
		sample_size = (stz2->sample_count + 1) / 2;
		break;
	case 8:
	case 16:
		sample_size = stz2->sample_count * (stz2->field_size / 8);
		break;
	default:
		char *msg = g_strdup_printf ("Mp4Demuxer: invalid field size in 'stz2' box (%u)", stz2->field_size);
		ReportErrorOccurred (msg);
		g_free (msg);
		break;
	}

	VERIFY_BOX_SIZE (sample_size, "stz2");
	stz2->samples = (guint8 *) g_malloc (sample_size);

	for (guint32 i = 0; i < stz2->sample_count; i++) {
		switch (stz2->field_size) {
		case 4:
			if (i % 2 == 0) {
				stz2->samples [i] = source->ReadBE_U8 ();
			}
			break;
		case 8:
			stz2->samples [i] = source->ReadBE_U8 ();
			break;
		case 16:
			((guint16 *) stz2->samples) [i] = source->ReadBE_U16 ();
			break;
		}
	}

	stz2->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStz2 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStsc (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	StscBox *stsc;

	LOG_MP4 ("Mp4Demuxer::ReadStsc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stsc);
	
	stbl->stsc = new StscBox (type, size);
	stsc = stbl->stsc;

	if (!ReadFullBox (stsc))
		return false;

	VERIFY_BOX_SIZE (4, "stsc");

	stsc->entry_count = source->ReadBE_U32 ();
	VERIFY_BOX_SIZE (stsc->entry_count * 12, "stsc");

	LOG_MP4 ("Mp4Demuxer::ReadStsc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, stsc->entry_count);

	stsc->first_chunk = (guint32 *) g_malloc (stsc->entry_count * sizeof (guint32));
	stsc->samples_per_chunk = (guint32 *) g_malloc (stsc->entry_count * sizeof (guint32));
	stsc->sample_description_index = (guint32 *) g_malloc (stsc->entry_count * sizeof (guint32));
	for (guint32 i = 0; i < stsc->entry_count; i++) {
		stsc->first_chunk [i] = source->ReadBE_U32 ();
		stsc->samples_per_chunk [i] = source->ReadBE_U32 ();
		stsc->sample_description_index [i] = source->ReadBE_U32 ();

		if (i > 0 && stsc->first_chunk [i - 1] >= stsc->first_chunk [i]) {
			ReportErrorOccurred ("Corrupted mp4 file ('stsc' has non-sequential first_chunk entries)");
			return false;
		}

		if (stsc->samples_per_chunk [i] == 0) {
			ReportErrorOccurred ("Corrupted mp4 file ('stsc' has samples_per_chunk entries with a 0 value)");
			return false;
		}

		//LOG_MP4 ("Mp4Demuxer::ReadStsc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry [%u] = first_chunk: %u samples_per_chunk: %u sample_description_per_index: %u\n",
		//	start, size, i, stsc->first_chunk [i], stsc->samples_per_chunk [i], stsc->sample_description_index [i]);
	}

	stsc->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStsc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadStco (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	StcoBox *stco;
 
	LOG_MP4 ("Mp4Demuxer::ReadStco (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, stco);
	
	stbl->stco = new StcoBox (type, size);
	stco = stbl->stco;

	if (!ReadFullBox (stco))
		return false;

	VERIFY_BOX_SIZE (4, "stco");

	stco->entry_count = source->ReadBE_U32 ();
	VERIFY_BOX_SIZE (stco->entry_count * 4, "stco");

	LOG_MP4 ("Mp4Demuxer::ReadStco (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, stco->entry_count);

	stco->chunk_offset = (guint32 *) g_malloc (sizeof (guint32) * stco->entry_count);
	for (guint32 i = 0; i < stco->entry_count; i++) {
		stco->chunk_offset [i] = source->ReadBE_U32 ();
		//LOG_MP4 ("Mp4Demuxer::ReadStco (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry [%u] = chunk_offset: %u\n", start, size, i, stco->chunk_offset [i]);
	}

	stco->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadStsc (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadCo64 (guint32 type, guint64 start, guint64 size, StblBox *stbl)
{
	MemoryBuffer *source = buffer;
	Co64Box *co64;
 
	LOG_MP4 ("Mp4Demuxer::ReadCo64 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (stbl, co64);

	stbl->co64 = new Co64Box (type, size);
	co64 = stbl->co64;

	if (!ReadFullBox (co64))
		return false;

	VERIFY_BOX_SIZE (4, "co64");

	co64->entry_count = source->ReadBE_U32 ();
	VERIFY_BOX_SIZE (co64->entry_count * 8, "co64");

	LOG_MP4 ("Mp4Demuxer::ReadCo64 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry_count: %u\n", start, size, co64->entry_count);

	co64->chunk_offset = (guint64 *) g_malloc (sizeof (guint64) * co64->entry_count);
	for (guint32 i = 0; i < co64->entry_count; i++) {
		co64->chunk_offset [i] = source->ReadBE_U64 ();
		//LOG_MP4 ("Mp4Demuxer::ReadCo64 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") entry [%u] = chunk_offset: %" G_GUINT64_FORMAT "\n", start, size, i, co64->chunk_offset [i]);
	}

	co64->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadCo64 (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadVmhd (guint32 type, guint64 start, guint64 size, MinfBox *minf)
{
	MemoryBuffer *source = buffer;
	VmhdBox *vmhd;

	LOG_MP4 ("Mp4Demuxer::ReadVmhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (minf, vmhd);

	minf->vmhd = new VmhdBox (type, size);
	vmhd = minf->vmhd;

	if (!ReadFullBox (vmhd))
		return false;

	VERIFY_BOX_SIZE (8, "vmhd");

	source->ReadBE_U16 (); /* graphicsmode */
	for (int i = 0; i < 3; i++) {
		source->ReadBE_U16 (); /* opcolor [i] */
	}

	vmhd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadVmhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadSmhd (guint32 type, guint64 start, guint64 size, MinfBox *minf)
{
	MemoryBuffer *source = buffer;
	SmhdBox *smhd;

	LOG_MP4 ("Mp4Demuxer::ReadSmhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (minf, smhd);

	minf->smhd = new SmhdBox (type, size);
	smhd = minf->smhd;

	if (!ReadFullBox (smhd))
		return false;

	VERIFY_BOX_SIZE (4, "smhd");

	source->ReadBE_U16 (); /* balance */
	source->ReadBE_U16 (); /* reserved */

	smhd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadSmhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") [Done]\n", start, size);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadMdhd (guint32 type, guint64 start, guint64 size, MdiaBox *mdia)
{
	MemoryBuffer *source = buffer;
	MdhdBox *mdhd;

	LOG_MP4 ("Mp4Demuxer::ReadMdhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (mdia, mdhd);

	mdia->mdhd = new MdhdBox (type, size);
	mdhd = mdia->mdhd;

	if (!ReadFullBox (mdhd))
		return false;

	if (mdhd->version == 0) {
		VERIFY_BOX_SIZE (20, "mdhd");
		source->ReadBE_U32 (); /* creation time */
		source->ReadBE_U32 (); /* modification time */
		mdhd->timescale = source->ReadBE_U32 ();
		mdhd->duration = source->ReadBE_U32 ();
	} else if (mdhd->version == 1) {
		VERIFY_BOX_SIZE (32, "mdhd");
		source->ReadBE_U64 (); /* creation time */
		source->ReadBE_U64 (); /* modification time */
		mdhd->timescale = source->ReadBE_U32 ();
		mdhd->duration = source->ReadBE_U64 ();
	} else {
		char *msg = g_strdup_printf ("Corrupted mp4 file at position: %" G_GUINT64_FORMAT " (invalid mdhd version: %i)", source->GetPosition (), mdhd->version);
		ReportErrorOccurred (msg);
		g_free (msg);
		return false;
	}

	source->ReadBE_U16 (); /* 1 bit padding + 3 * 5 bits of language code */
	source->ReadBE_U16 (); /* pre_defined */

	mdhd->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadMdhd (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") duration: %" G_GUINT64_FORMAT " timescale: %u [Done]\n",
		start, size, mdhd->duration, mdhd->timescale);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

bool
Mp4Demuxer::ReadHdlr (guint32 type, guint64 start, guint64 size, MdiaBox *mdia)
{
	MemoryBuffer *source = buffer;
	HdlrBox *hdlr;

	LOG_MP4 ("Mp4Demuxer::ReadHdlr (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ")\n", start, size);

	VERIFY_EXISTING_BOX (mdia, hdlr);

	mdia->hdlr = new HdlrBox (type, size);
	hdlr = mdia->hdlr;

	if (!ReadFullBox (hdlr))
		return false;

	VERIFY_BOX_SIZE (20, "hdlr");

	source->ReadBE_U32 (); /* pre_defined */
	hdlr->handler_type = source->ReadBE_U32 ();
	for (int i = 0; i < 3; i++) {
		source->ReadBE_U32 (); /* reserved [i] */
	}

	guint32 len = 0;
	while (source->GetPosition () - start + len < size) {
		if (source->PeekByte (len) == 0)
			break;
		len++;
	}
	if (len > 0) {
		hdlr->name = g_strndup ((char *) source->GetCurrentPtr (), len);
	}

	hdlr->parsed = true;
	LOG_MP4 ("Mp4Demuxer::ReadHdlr (%" G_GUINT64_FORMAT ", %" G_GUINT64_FORMAT ") handler_type: " FORMAT_4CC " (%u) name: '%s' [Done]\n",
		start, size, VALUES_4CC (hdlr->handler_type), hdlr->handler_type, hdlr->name);

	/* Skip whatever data is left */
	source->SeekSet (start + size);

	return true;
}

void
Mp4Demuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	/* Nothing to do here */
}

/*
 * Mp4DemuxerInfo
 */

MediaResult
Mp4DemuxerInfo::Supports (MemoryBuffer *source)
{
	LOG_MP4 ("Mp4DemuxerInfo::Supports ().\n");

	bool result = false;
	guint64 size;
	guint32 type;
	guint32 major_brand;
	guint32 minor_brand;
	guint32 compatible_brand;

	if (source->GetRemainingSize () < 8)
		return MEDIA_NOT_ENOUGH_DATA;

	size = source->ReadBE_U32 ();
	type = source->ReadBE_U32 ();

	/* Check for "ISO base media file format" as specified in ISO/IEC 14496-12 */
	if (type != FTYP_4CC) {
		LOG_MP4 ("Mp4DemuxerInfo::Supports (): no 'ftyp' box found.\n");
		return MEDIA_FAIL;
	}

	if (size == 1)
		size = source->ReadBE_U64 ();

	if (size > (guint64) source->GetSize ()) {
		LOG_MP4 ("Mp4DemuxerInfo::Supports (): we need %" G_GUINT64_FORMAT " bytes, only got %" G_GUINT64_FORMAT " bytes available\n", size, source->GetSize ());
		return MEDIA_NOT_ENOUGH_DATA;
	}

	/* We check for 'avc1', 'mp41' and 'mp42' major brand or compatible brands. The MP4 file specification (ISO/IEC 14496-14)
	 * states that the types 'mp41' or 'mp42' shall appear in the list of compatible-brands in the ftyp box, though SL plays 
	 * 'avc1' files too. */
	major_brand = source->ReadBE_U32 ();
	minor_brand = source->ReadBE_U32 ();
	LOG_MP4 ("Mp4DemuxerInfo::Supports (): major brand: " FORMAT_4CC " (%u), minor brand: " FORMAT_4CC " (%u)\n", VALUES_4CC (major_brand), major_brand, VALUES_4CC (minor_brand), minor_brand);
	result = Mp4Demuxer::IsCompatibleType (major_brand);
	while (size > (guint64) source->GetPosition () && source->GetRemainingSize () >= 4) {
		compatible_brand = source->ReadBE_U32 ();
		result = result || Mp4Demuxer::IsCompatibleType (compatible_brand);
		LOG_MP4 (" got compatible brand: " FORMAT_4CC " (%u)\n", VALUES_4CC (compatible_brand), compatible_brand);
	}

	LOG_MP4 ("Mp4DemuxerInfo::Supports (): result: %i\n", result);

	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

IMediaDemuxer *
Mp4DemuxerInfo::Create (Media *media, IMediaSource *source, MemoryBuffer *buffer)
{
	return new Mp4Demuxer (media, source, buffer);
}

};
