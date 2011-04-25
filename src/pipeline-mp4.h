	/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-mp4.h: MP4 related parts of the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#ifndef __MOON_PIPELINE_MP4_H__
#define __MOON_PIPELINE_MP4_H__

#include "pipeline.h"

namespace Moonlight {

struct MvhdBox;
struct TrakBox;
struct TkhdBox;
struct MdiaBox;
struct MdhdBox;
struct MinfBox;
struct HdlrBox;
struct VmhdBox;
struct SmhdBox;
struct DinfBox;
struct DrefBox;
struct StblBox;
struct SttsBox;
struct StsdBox;
struct CttsBox;
struct StscBox;
struct StszBox;
struct Stz2Box;
struct StcoBox;
struct Co64Box;
struct StssBox;
struct SampleEntry;
struct VisualSampleEntry;
struct AudioSampleEntry;

struct Fixed16_16 {
	guint16 hi;
	guint16 lo;
	double ToDouble () { return hi + ((double) lo / (double) 0xFFFF); }
};

struct Box {
	guint64 size;
	guint32 type;
	bool parsed;
	Box (guint32 type, guint64 size);
	virtual ~Box () {}
};

struct FullBox : public Box {
	guint8 version;
	guint32 flags;
	FullBox (guint32 type, guint64 size);
	virtual ~FullBox () {}
};

struct MoovBox : public Box {
	MvhdBox *mvhd;
	TrakBox **trak;
	guint32 trak_count;
	MoovBox (guint32 type, guint64 size);
	virtual ~MoovBox ();
};

struct MvhdBox : public FullBox {
	guint32 timescale;
	guint64 duration;
	MvhdBox (guint32 type, guint64 size);
};

struct TrakBox : public Box {
	TkhdBox *tkhd;
	MdiaBox *mdia;

	/* The following fields are not defined in the trak box, but are values cached by the demuxer */
	IMediaStream *stream;
	guint32 stsc_index;
	guint32 stsc_sample_index;
	guint32 stsc_sample_count;
	guint32 stsc_samples_size;
	guint32 current_sample;

	TrakBox (guint32 type, guint64 size);
	virtual ~TrakBox ();
};

struct TkhdBox : public FullBox {
	guint32 track_ID;
	guint64 duration;
	Fixed16_16 width;
	Fixed16_16 height;
	TkhdBox (guint32 type, guint64 size);
};

struct MdiaBox : public Box {
	MdhdBox *mdhd;
	MinfBox *minf;
	HdlrBox *hdlr;
	MdiaBox (guint32 type, guint64 size);
	virtual ~MdiaBox ();
};

struct MdhdBox : public FullBox {
	guint32 timescale;
	guint64 duration;
	MdhdBox (guint32 type, guint64 size);
};

struct HdlrBox : public FullBox {
	guint32 handler_type;
	char *name;
	HdlrBox (guint32 type, guint64 size);
	virtual ~HdlrBox ();
};

struct MinfBox : public Box {
	MdiaBox *parent;
	VmhdBox *vmhd;
	SmhdBox *smhd;
	DinfBox *dinf;
	StblBox *stbl;
	MinfBox (MdiaBox *parent, guint32 type, guint64 size);
	virtual ~MinfBox ();
};

struct VmhdBox : public FullBox {
	VmhdBox (guint32 type, guint64 size) : FullBox (type, size) {}
};

struct SmhdBox : public FullBox {
	SmhdBox (guint32 type, guint64 size) : FullBox (type, size) {}
};

struct DinfBox : public FullBox {
	DrefBox *dref;
	DinfBox (guint32 type, guint64 size);
	virtual ~DinfBox ();
};

struct DrefBox : public FullBox {
	guint32 entry_count;
	FullBox **entries;
	DrefBox (guint32 type, guint64 size);
	virtual ~DrefBox ();
};

struct UrnBox : public FullBox {
	char *name;
	char *location;
	UrnBox (guint32 type, guint64 size);
	virtual ~UrnBox ();
};

struct UrlBox : public FullBox {
	char *location;
	UrlBox (guint32 type, guint64 size);
	virtual ~UrlBox ();
};

struct StblBox : public FullBox {
	MinfBox *parent;
	SttsBox *stts;
	CttsBox *ctts;
	StsdBox *stsd;
	StszBox *stsz;
	Stz2Box *stz2;
	StscBox *stsc;
	StcoBox *stco;
	Co64Box *co64;
	StssBox *stss;
	StblBox (MinfBox *parent, guint32 type, guint64 size);
	virtual ~StblBox ();
};

struct StsdBox : public FullBox {
	StblBox *parent;
	guint32 entry_count;
	SampleEntry **entries;
	StsdBox (StblBox *parent, guint32 type, guint64 size);
	virtual ~StsdBox ();
};

struct SttsBox : public FullBox {
	guint32 entry_count;
	guint32 *sample_count;
	guint32 *sample_delta;
	guint32 total_sample_count;
	SttsBox (guint32 type, guint64 size);
	virtual ~SttsBox ();
};

struct StssBox : public FullBox {
	guint32 entry_count;
	guint32 *sample_number;
	StssBox (guint32 type, guint64 size);
	virtual ~StssBox ();
};

struct CttsBox : public FullBox {
	guint32 entry_count;
	guint32 *sample_count;
	/* The spec states very clearly that the sample offsets are unsigned and can't be negative.
	 * Obviously that doesn't prevent people from making encoders that use negative values here. */
	gint32 *sample_offset;

	gint32 min_offset; /* the smallest sample_offset entry, used to offset pts by this amount */
	CttsBox (guint32 type, guint64 size);
	virtual ~CttsBox ();
};

struct StscBox : public FullBox {
	guint32 entry_count;
	guint32 *first_chunk;
	guint32 *samples_per_chunk;
	guint32 *sample_description_index;
	StscBox (guint32 type, guint64 size);
	virtual ~StscBox ();
};

struct StszBox : public FullBox {
	guint32 sample_size;
	guint32 sample_count;
	guint32 *entry_size;
	StszBox (guint32 type, guint64 size);
	virtual ~StszBox ();
};

struct Stz2Box : public FullBox {
	guint8 field_size;
	guint32 sample_count;
	guint8 *samples;
	Stz2Box (guint32 type, guint64 size);
	virtual ~Stz2Box ();
};

struct StcoBox : public FullBox {
	guint32 entry_count;
	guint32 *chunk_offset;
	StcoBox (guint32 type, guint64 size);
	virtual ~StcoBox ();
};

struct Co64Box : public FullBox {
	guint32 entry_count;
	guint64 *chunk_offset;
	Co64Box (guint32 type, guint64 size);
	virtual ~Co64Box ();
};

struct EsdsBox : public FullBox {
	guint16 ES_ID; /* 16 bits */
	bool streamDependenceFlag; /* 1 bit */
	bool URL_Flag; /* 1 bit */
	bool OCRstreamFlag; /* 1 bit */
	guint8 streamPriority; /* 5 bits */
	guint16 dependsOn_ES_ID; /* 16 bits */
	guint8 URLlength; /* 8 bits */
	char *URLstring;
	guint16 OCR_ES_Id;
	
	/* DecoderConfigDescr */
	guint8 objectTypeIndication; /* 8 bits */
	guint8 streamType; /* 6 bits */
	bool upStream; /* 1 bit */
	guint32 bufferSizeDB; /* 24 bits */
	guint32 maxBitrate; /* 32 bits */
	guint32 avgBitrate; /* 32 bits */

	EsdsBox (guint32 type, guint64 size);
	virtual ~EsdsBox ();
};

struct SampleEntry : public Box {
	guint16 data_reference_index;
	char *format;
	EsdsBox *esds;
	guint32 extradata_size;
	guint8 *extradata;
	SampleEntry (guint32 type, guint64 size);
	virtual ~SampleEntry ();
};

struct VisualSampleEntry : public SampleEntry {
	guint16 width;
	guint16 height;
	Fixed16_16 horizontal_resolution;
	Fixed16_16 vertical_resolution;
	char *compressor_name;
	VisualSampleEntry (guint32 type, guint64 size);
	virtual ~VisualSampleEntry ();
};

struct AudioSampleEntry : public SampleEntry {
	guint16 channelcount;
	guint16 samplesize;
	Fixed16_16 samplerate;
	AudioSampleEntry (guint32 type, guint64 size);
};

/*
 * Mp4Demuxer
 */
class Mp4Demuxer : public IMediaDemuxer {
private:
	MemoryBuffer *buffer;
	IMediaStream *get_frame_stream;
	guint64 buffer_position;
	bool needs_raw_frames;
	bool last_buffer;
	bool ftyp_validated;
	int8_t nal_size_length; // the length of the size field before each nal unit (in bytes)

	MoovBox *moov;

	static MediaResult ReadHeaderDataAsyncCallback (MediaClosure *closure);
	static MediaResult ReadSampleDataAsyncCallback (MediaClosure *closure);
	void ReadSampleDataAsync (MediaReadClosure *closure);
	void ReadHeaderDataAsync (MediaReadClosure *closure);
	void RequestMoreHeaderData (guint64 offset, guint32 size); /* size: the number of more bytes to request */
	void ParseAVCExtraData (IMediaStream *stream, SampleEntry *entry);
	bool ParseAVCFrame (MediaFrame *frame, MemoryBuffer *buffer, guint32 sample_size);

	bool ReadBox (guint64 *size, guint32 *type);
	bool ReadFullBox (FullBox *box);
	bool ReadMoov (guint32 type, guint64 start, guint64 size);
	bool ReadMvhd (guint32 type, guint64 start, guint64 size, MoovBox *moov);
	bool ReadTrak (guint32 type, guint64 start, guint64 size, MoovBox *moov);
	bool ReadTkhd (guint32 type, guint64 start, guint64 size, TrakBox *trak);
	bool ReadMdia (guint32 type, guint64 start, guint64 size, TrakBox *trak);
	bool ReadMdhd (guint32 type, guint64 start, guint64 size, MdiaBox *mdia);
	bool ReadHdlr (guint32 type, guint64 start, guint64 size, MdiaBox *mdia);
	bool ReadMinf (guint32 type, guint64 start, guint64 size, MdiaBox *mdia);
	bool ReadVmhd (guint32 type, guint64 start, guint64 size, MinfBox *minf);
	bool ReadSmhd (guint32 type, guint64 start, guint64 size, MinfBox *minf);
	bool ReadDinf (guint32 type, guint64 start, guint64 size, MinfBox *minf);
	bool ReadStbl (guint32 type, guint64 start, guint64 size, MinfBox *minf);
	bool ReadStts (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadCtts (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadStsd (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadStsc (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadStsz (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadStco (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadStss (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadDref (guint32 type, guint64 start, guint64 size, DinfBox *dinf);
	bool ReadStz2 (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadCo64 (guint32 type, guint64 start, guint64 size, StblBox *stbl);
	bool ReadEsds (guint32 type, guint64 start, guint64 size, SampleEntry *entry);
	bool ReadAvcc (guint32 type, guint64 start, guint64 size, SampleEntry *entry);
	bool ReadSampleEntry (SampleEntry *entry);
	bool ReadVisualSampleEntry (VisualSampleEntry **entry);
	bool ReadAudioSampleEntry (AudioSampleEntry **entry);
	bool ReadLoop (guint64 start, guint64 size, Box *container);
	bool ReadDescriptorLength (guint32 *length);

	bool OpenMoov ();

	guint64 ToPts (guint64 time, TrakBox *trak);
	guint64 ToPts (guint64 time, guint64 timescale);
	guint64 FromPts (guint64 pts, TrakBox *trak);
	bool GetSampleSize (StblBox *stbl, guint32 sample_index, guint32 *result);
	bool GetSampleCount (StblBox *stbl, guint32 *result);
	bool GetChunkOffset (StblBox *stbl, guint32 chunk_index, guint64 *result);
	bool GetChunkCount (StblBox *stbl, guint32 *result);

protected:
	virtual ~Mp4Demuxer ();

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();

	/* 
	 * This method will seek to the first keyframe before the requested pts in all selected streams.
	 * Note that the streams will probably be positioned at different pts after a seek (given that
	 * for audio streams any frame is considered a key frame, while for video there may be several
	 * seconds between every key frame).
	 */
	virtual void SeekAsyncInternal (guint64 seekToTime);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream);

public:
	/* @SkipFactories */
	Mp4Demuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
	virtual void Dispose ();
	virtual void UpdateSelected (IMediaStream *stream);

	void SetNeedsRawFrames (bool needs_raw_frames) {
		this->needs_raw_frames = needs_raw_frames;
	}

	static bool IsCompatibleType (guint32 type);
	static char *TypeToString (guint32 type);
};

/*
 * Mp4DemuxerInfo
 */
class Mp4DemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (MemoryBuffer *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
	virtual const char *GetName () { return "Mp4Demuxer"; }
};

};
#endif /* __MOON_PIPELINE_MP4_H__ */
 
