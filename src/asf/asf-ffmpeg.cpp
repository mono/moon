/*
 * asf-ffmpeg.cpp: 
 *   An asf demuxer for ffmpeg.
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
 
#include <config.h>
#include <inttypes.h>
#include "asf-ffmpeg.h"
#include <stdlib.h>

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

#define FFMPEG_DUMP(...) printf (__VA_ARGS__)
//ASF_DUMP(__VA_ARGS__)

int ffmpeg_asf_probe(AVProbeData *pd)
{
	FFMPEG_LOG ("ffmpeg_asf_probe (%p), filename: %s, buf: %p, buf_size: %i\n", pd, pd->filename, pd->buf, pd->buf_size);
	
	char* env = getenv ("MOON_DEMUXER");
	if (env == NULL || *env == 0) {
		printf ("Using ffmpegs demuxer.\n");
		return 0;
	}
	printf ("Using the moon demuxer.\n");
	
	if ((size_t) pd->buf_size < sizeof (asf_guid))
		return 0;
		
	if (asf_guid_compare (&asf_guids_header, (asf_guid*) pd->buf)) {
		FFMPEG_LOG ("ffmpeg_asf_probe (%p): Success.\n", pd);
		return AVPROBE_SCORE_MAX;
	}
	
	return 0;
}

ASFParser* ffmpeg_asf_last_parser = NULL;

ASFParser* 
ffmpeg_asf_get_last_parser ()
{
	ASFParser* result = ffmpeg_asf_last_parser;
	ffmpeg_asf_last_parser = NULL;
	return result;
}

int ffmpeg_asf_read_header(AVFormatContext *s, AVFormatParameters *ap)
{
	FFMPEG_LOG ("ffmpeg_asf_read_header (%p, %p).\n", s, ap);
	
	FFMPEGParser *parser = new FFMPEGParser (s->filename);
	MoonASFContext *context = (MoonASFContext*) s->priv_data;
	context->parser = parser;
	
	ffmpeg_asf_last_parser = parser;
	
	parser->source = new FFMPEGSource (parser, &s->pb);
	
	if (!parser->ReadHeader ()) {
		return -1;
	}
	
	// Parse stream properties
	int index;
//	asf_object* obj = NULL;
	asf_file_properties* file_properties = (asf_file_properties*) parser->GetHeaderObject (&asf_guids_file_properties);
	
	index = -1;
	while ((index = parser->GetHeaderObjectIndex (&asf_guids_stream_properties, index + 1)) >= 0) {
		asf_stream_properties* asp = (asf_stream_properties*) parser->GetHeader (index);
		
		
		CodecType codec_type;
		if (asf_guid_compare (&asf_guids_media_audio, &asp->stream_type)) {
			codec_type = CODEC_TYPE_AUDIO;
		} else if (asf_guid_compare (&asf_guids_media_video, &asp->stream_type)) {
			codec_type = CODEC_TYPE_VIDEO;
		} else {
			// Unknown stream type, ignore it.
			FFMPEG_LOG ("ffmpeg_asf_read_header (%p, %p): Unknown stream type: %s with index: %i.\n", s, ap, asf_guid_tostring (&asp->stream_type), index);
			continue;
		}
		
		AVStream *stream = NULL;
		stream = av_new_stream (s, 0);
		if (!stream)
			goto fail;
			
		av_set_pts_info(stream, 32, 1, 1000);
		stream->priv_data = parser;
		stream->codec->codec_type = codec_type;
		stream->id = asp->get_stream_number ();
		parser->AddStreamIndex (s->nb_streams - 1, asp->get_stream_number ());
		if (!file_properties->is_broadcast ()) {
			stream->duration = file_properties->play_duration / 10000; /// (10000 /*000 / 1000*/) - file_properties->preroll;
		}

		if (codec_type == CODEC_TYPE_AUDIO) {
			FFMPEG_LOG ("Audio codec.\n");
			WAVEFORMATEX* wave = asp->get_audio_data ();
			WAVEFORMATEXTENSIBLE* wave_ex = wave->get_wave_format_extensible ();
			if (wave != NULL) {
				asf_dword id = wave->codec_id;
				int data_size = asp->size - sizeof (asf_stream_properties) - sizeof (WAVEFORMATEX);
				stream->codec->codec_type = CODEC_TYPE_AUDIO;
				stream->codec->codec_tag = wave->codec_id;
				stream->codec->channels = wave->channels;
				stream->codec->sample_rate = wave->samples_per_second;
				stream->codec->bit_rate = wave->bytes_per_second * 8;
				stream->codec->block_align = wave->block_alignment;
				stream->codec->bits_per_sample = wave->bits_per_sample;
				stream->codec->extradata_size = data_size > wave->codec_specific_data_size ? wave->codec_specific_data_size : data_size;
				stream->codec->extradata = NULL;
				if (wave_ex != NULL) {
					stream->codec->bits_per_sample = wave_ex->Samples.valid_bits_per_sample;
					stream->codec->extradata_size -= (sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX));
					id = *((asf_dword*) &wave_ex->sub_format);
				}
				if (stream->codec->extradata_size > 0) {
					stream->codec->extradata = (uint8_t*) av_mallocz (stream->codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
					char* src = ((char*) wave) + (wave_ex ? sizeof (WAVEFORMATEX) : sizeof (WAVEFORMATEX));
					memcpy (stream->codec->extradata, src, stream->codec->extradata_size);
				}
				WAVEFORMATEX_dump (wave);
				//stream->codec->codec_id = wav_codec_get_id (id, stream->codec->bits_per_sample);
				stream->need_parsing = AVSTREAM_PARSE_FULL;
				
				FFMPEG_LOG ("Audio codec id: %i = %i.\n", stream->codec->codec_id, id);
				switch (id) {
				case 0X160:
					stream->codec->codec_id = CODEC_ID_WMAV1;
					break;
				case 0x161:
					stream->codec->codec_id = CODEC_ID_WMAV2;
					break;
				default:
					FFMPEG_LOG ("Unknown audio codec: %i.\n", id);
					goto fail;
				}
			}
		} else if (codec_type == CODEC_TYPE_VIDEO) {
			FFMPEG_LOG ("Video codec.\n");
			asf_video_stream_data* video_data = asp->get_video_data ();
			BITMAPINFOHEADER* bmp;
//			asf_word id = 0;
			if (video_data != NULL) {
				bmp = video_data->get_bitmap_info_header ();
				if (bmp != NULL) {
					stream->codec->width = bmp->image_width;
					stream->codec->height = bmp->image_height;
					stream->codec->bits_per_sample = bmp->bits_per_pixel;
					stream->codec->codec_tag = bmp->compression_id;
					stream->codec->extradata_size = bmp->get_extra_data_size ();
					if (stream->codec->extradata_size > 0) {
						stream->codec->extradata = (uint8_t*) av_mallocz (stream->codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
						memcpy (stream->codec->extradata, bmp->get_extra_data (), stream->codec->extradata_size);
					} else {
						stream->codec->extradata = NULL;
					}
					
					FFMPEG_LOG ("Video codec tag: %i (%c %c %c %c).\n", stream->codec->codec_tag, *(char*) &stream->codec->codec_tag, *(1 + (char*) &stream->codec->codec_tag), *(2 + (char*) &stream->codec->codec_tag), *(3 + (char*) &stream->codec->codec_tag));
					asf_video_stream_data_dump (video_data);
					BITMAPINFOHEADER_dump (bmp);
					
					switch (stream->codec->codec_tag) {
					case MKTAG('W', 'M', 'V', '1'): 
						stream->codec->codec_id = CODEC_ID_WMV1; break;
					case MKTAG('W', 'M', 'V', '2'):
						stream->codec->codec_id = CODEC_ID_WMV2; break;
					case MKTAG('W', 'M', 'V', '3'):
						stream->codec->codec_id = CODEC_ID_WMV3; break;
					case MKTAG('W', 'V', 'C', '1'):
					case MKTAG('W', 'M', 'V', 'A'):
						stream->codec->codec_id = CODEC_ID_VC1; break;
					default:
						FFMPEG_LOG ("Unknown video codec: %i (%c%c%c%c).\n", stream->codec->codec_tag, *(char*) &stream->codec->codec_tag, *(1 + (char*) &stream->codec->codec_tag), *(2 + (char*) &stream->codec->codec_tag), *(3 + (char*) &stream->codec->codec_tag));
						goto fail;
					}
				}
			}
		} else {
			FFMPEG_LOG ("Unimplemented codec type: %i.\n", codec_type);
			goto fail;
		}
	}
	
	//AVFormatContext_dump (s);
	
	return 0;
	
fail:
	ASF_ERROR_VAL (-1, "Failure cleanup not implemented.\n");
	return -1;
}

int ffmpeg_asf_read_packet(AVFormatContext *s, AVPacket *av_packet)
{
	FFMPEG_LOG ("ffmpeg_asf_read_packet (%p, %p), parser: %p\n", s, av_packet, s->priv_data);
	
	MoonASFContext *context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	ASFFrameReader* reader = parser->reader;
	
	if (!reader->Advance ())
		return -1;
		
	AVPacket* pkt = new AVPacket ();
	
	av_new_packet (pkt, reader->Size ());
		
	pkt->pts = reader->Pts ();
	pkt->dts = 0;
	pkt->stream_index = parser->ConvertStreamIndex (reader->StreamNumber ()); 
	pkt->pos = -1;
	pkt->size = reader->Size ();
	reader->Write (pkt->data);
	if (reader->IsKeyFrame () || s->streams [pkt->stream_index]->codec->codec_type == CODEC_TYPE_AUDIO) {
		pkt->flags |= PKT_FLAG_KEY;
	}
		
	//AVPacket_dump (pkt, 1);
	
	*av_packet = *pkt; 
	
	return 0;
	
	//////////////
	
#if 0
	
	MoonASFContext *context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	FFMPEGPacket* packet = NULL;
	
	packet = (FFMPEGPacket*) parser->GetCurrentPacket ();
	
	if (packet == NULL) {
		if (parser->source->Position () >= parser->packet_offset_end) {
			FFMPEG_LOG ("ffmpeg_asf_read_packet: no more packets.\n");
			return -1;
		}
		
		// We have to seek to the next packet, no guarantee the position hasn't changed.
		int64_t pos = parser->GetPacketOffset (parser->packets_read);
		
		printf ("ffmpeg_asf_read_packet, seeking to packet #%lld (at position %lld, current position: %lld).\n", parser->packets_read + 1, pos, parser->source->Position ());
		FFMPEG_LOG ("\tdata_offset = %lld, data_size = %lld, packets_read = %lld, min_packet_size = %i.\n", parser->data_offset,
			parser->data ? parser->data->size : (int64_t) 0, 
			parser->packets_read, parser->GetFileProperties ()->min_packet_size);
		
		parser->source->Seek (pos, SEEK_SET);
		
		packet = new FFMPEGPacket ();
		if (!parser->ReadPacket (packet)) {
			return -1;
		}
		parser->SetCurrentPacket (packet);
		parser->packets_read++;
		FFMPEG_LOG ("ffmpeg_asf_read_packet: Created new packet %p with %i payloads.\n", packet, packet->GetPayloadCount ());
	}

	gint32 current_payload = ++packet->current_payload;
	asf_single_payload* payload = packet->GetPayload (current_payload - 1);
	guint32 payload_size = payload->payload_data_length;
	gint32 current_stream = payload->stream_number;
	
	guint32 next_data_size = 0;
	void* next_data = NULL;
	int64_t pts = packet->payloads->get_presentation_time (current_stream);
	
	FFMPEG_LOG ("ffmpeg_asf_read_packet: Reading payload #%i/%i with size %u.\n", current_payload, packet->GetPayloadCount (), payload_size);
	
	if (packet->current_payload == packet->GetPayloadCount ()) {
		int64_t pos = parser->GetPacketOffset (parser->packets_read);
		parser->source->Seek (pos, SEEK_SET);
		parser->SetCurrentPacket (NULL);
		if (parser->ReadPacket (new FFMPEGPacket ())) {
			parser->packets_read++;
			
			asf_single_payload* next_payload;
			packet = (FFMPEGPacket*) parser->GetCurrentPacket ();
			next_payload = packet->GetPayload (packet->current_payload++);
			if (next_payload != NULL && 
				next_payload->media_object_number == payload->media_object_number &&
				next_payload->stream_number == payload->stream_number) {
				next_data_size = next_payload->payload_data_length;
				next_data = g_memdup (next_payload->payload_data, next_data_size);
			}
		}
	}
	
	AVPacket* pkt = new AVPacket ();
	
	av_new_packet (pkt, payload_size + next_data_size);
		
	pkt->pts = pts;
	pkt->dts = 0;
	pkt->stream_index = parser->ConvertStreamIndex (current_stream); 
	pkt->pos = -1;
	pkt->size = payload_size + next_data_size;
	
	if (payload->is_key_frame || s->streams [pkt->stream_index]->codec->codec_type == CODEC_TYPE_AUDIO) {
		pkt->flags |= PKT_FLAG_KEY;
	}
	//printf ("pkt: %p, pkt->data: %p, payload_size: %i, next_data_size: %i\n", pkt, pkt->data, payload_size, next_data_size);
	memcpy (pkt->data, payload->payload_data, payload_size);
	if (next_data_size > 0) {
		memcpy (pkt->data + payload_size, next_data, next_data_size);
		g_free (next_data);
	}
		
	//AVPacket_dump (pkt, 1);
	if (false && packet->current_payload == packet->GetPayloadCount ()) {
		parser->FreeCurrentPacket ();
	}
	
	*av_packet = *pkt; 
	
	return 0;
#endif
}

int ffmpeg_asf_read_close(AVFormatContext *s)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	FFMPEG_LOG ("ffmpeg_asf_read_close (%p).\n", s);
	
	delete parser;
	
	return 0;
}

int ffmpeg_asf_read_seek(AVFormatContext *s, int stream_index, int64_t pts, int flags)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	
	FFMPEG_LOG ("ffmpeg_asf_read_seek (%p, %i, %lld, %i).\n", s, stream_index, pts, flags);
	
	if (parser->reader->Seek (parser->ConvertFFMpegStreamIndex (stream_index), pts))
		return 0;
	
	return -1;
}

int64_t ffmpeg_asf_read_pts(AVFormatContext *s, int ffmpeg_stream_index, int64_t *start_pos, int64_t pos_limit)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	
	FFMPEG_LOG ("ffmpeg_asf_read_pts (%p, %i, %p (%lld), %lld).\n", s, ffmpeg_stream_index, start_pos, *start_pos, pos_limit);
	
	int64_t packet_index = 0;
	int64_t packet_offset = 0;
	int64_t pos = *start_pos;
	asf_file_properties* file_properties = parser->GetFileProperties ();
	
	pos_limit = parser->packet_offset_end;
	
	// Find the packet at the start position
	if (pos < (int64_t) parser->data_offset) {
		FFMPEG_LOG ("ffmpeg_asf_read_pts: start position (%lld) is before start of data (%lld).\n", pos, parser->data_offset);
		return AV_NOPTS_VALUE;
	}
	
	// CHECK: what if min_packet_size != max_packet_size?
	packet_index = (pos - parser->data_offset) / file_properties->min_packet_size;
	packet_offset = parser->GetPacketOffset (packet_index);
	
	FFMPEG_LOG ("ffmpeg_asf_read_pts: calculated packet index %lld and packet offset %lld.\n", packet_index, packet_offset);
	
	if (packet_index >= (int64_t) file_properties->data_packet_count) {
		FFMPEG_LOG ("ffmpeg_asf_read_pts: calculated packet index (%lld) is more than packet count (%lld).\n", packet_index, file_properties->data_packet_count); 
		return AV_NOPTS_VALUE;
	}
	
	if (!parser->source->Seek (packet_offset, SEEK_SET)) {
		FFMPEG_LOG ("ffmpeg_asf_read_pts: could not seek to position %lld.\n", packet_offset);
		return AV_NOPTS_VALUE;
	}
	
	int stream_index = parser->ConvertFFMpegStreamIndex (ffmpeg_stream_index);
	if (stream_index == 0) {
		FFMPEG_LOG ("ffmpeg_asf_read_pts: could not find asf stream of ffmpeg stream index %i.\n", ffmpeg_stream_index);
		return AV_NOPTS_VALUE;
	}
	
	FFMPEG_LOG ("ffmpeg_asf_read_pts: searching for pts of position %lld of asf stream %i.\n", *start_pos, stream_index); 
	
	bool is_audio_stream = parser->GetStream (stream_index)->is_audio ();
	int64_t result = AV_NOPTS_VALUE;
	while (packet_offset > pos_limit) {
		packet_offset += file_properties->min_packet_size;
		packet_index++;
		
		ASFPacket* packet = new ASFPacket ();
		if (!parser->ReadPacket (packet)) {
			FFMPEG_LOG ("ffmpeg_asf_read_pts: could not read packet index %lld at position %lld.\n", packet_index, packet_offset);
			delete packet;
			return AV_NOPTS_VALUE;
		}
		
		if (!is_audio_stream && !packet->payloads->is_key_frame (stream_index))
			continue;

		*start_pos = packet_offset;
		
		result = packet->payloads->get_presentation_time (stream_index);
		
		delete packet;
		
		break;
	}
		
	FFMPEG_LOG ("fmpeg_asf_read_pts: found pts (%lld) of packet index %lld.\n", pts, packet_index);
	
	return result;
}

AVInputFormat ffmpeg_asf_demuxer = {
    "asf",
    "asf format",
    sizeof(MoonASFContext),
    ffmpeg_asf_probe,
    ffmpeg_asf_read_header,
    ffmpeg_asf_read_packet,
    ffmpeg_asf_read_close,
    ffmpeg_asf_read_seek,
    ffmpeg_asf_read_pts,
};






void AVFormatContext_dump (AVFormatContext* c)
{
	FFMPEG_DUMP ("AVFormatContext: %p\n", c);
	if (!c) return;
//	typedef struct AVFormatContext {
	FFMPEG_DUMP ("\t"  "av_class "  "= "  "%p"  "\n", c->av_class); //    const AVClass *av_class; /**< set by av_alloc_format_context */
//    /* can only be iformat or oformat, not both at the same time */
	FFMPEG_DUMP ("\t"  "av_iformat "  "= "  "%p"  "\n", c->iformat); //    struct AVInputFormat *iformat;
	FFMPEG_DUMP ("\t"  "oformat "  "= "  "%p"  "\n", c->oformat); //    struct AVOutputFormat *oformat;
	FFMPEG_DUMP ("\t"  "priv_data "  "= "  "%p"  "\n", c->priv_data); //    void *priv_data;
	FFMPEG_DUMP ("\t"  "pb "  "= "  "%p"  "\n", &c->pb); //    ByteIOContext pb;
	FFMPEG_DUMP ("\t"  "nb_streams "  "= "  "%u"  "\n", c->nb_streams); //    unsigned int nb_streams;
	FFMPEG_DUMP ("\t"  "streams "  "= "  "%p"  "\n", c->streams); //    AVStream *streams[MAX_STREAMS];
	for (guint32 i = 0; i < c->nb_streams; i++)
		AVStream_dump (c->streams [i], 2);
	FFMPEG_DUMP ("\t"  "filename "  "= "  "%s"  "\n", c->filename); //    char filename[1024]; /**< input or output filename */
//    /* stream info */
	FFMPEG_DUMP ("\t"  "timestamp "  "= "  "%lld"  "\n", c->timestamp); //    int64_t timestamp;
	FFMPEG_DUMP ("\t"  "title "  "= "  "%s"  "\n", c->title); //    char title[512];
	FFMPEG_DUMP ("\t"  "author "  "= "  "%s"  "\n", c->author); //    char author[512];
	FFMPEG_DUMP ("\t"  "copyright "  "= "  "%s"  "\n", c->copyright); //    char copyright[512];
	FFMPEG_DUMP ("\t"  "comment "  "= "  "%s"  "\n", c->comment); //    char comment[512];
	FFMPEG_DUMP ("\t"  "album "  "= "  "%s"  "\n", c->album); //    char album[512];
	FFMPEG_DUMP ("\t"  "year "  "= "  "%i"  "\n", c->year); //    int year;  /**< ID3 year, 0 if none */
	FFMPEG_DUMP ("\t"  "track "  "= "  "%i"  "\n", c->track); //    int track; /**< track number, 0 if none */
	FFMPEG_DUMP ("\t"  "genre "  "= "  "%s"  "\n", c->genre); //    char genre[32]; /**< ID3 genre */
//
	FFMPEG_DUMP ("\t"  "ctx_flags "  "= "  "%i"  "\n", c->ctx_flags); //    int ctx_flags; /**< format specific flags, see AVFMTCTX_xx */
//    /* private data for pts handling (do not modify directly) */
//    /** This buffer is only needed when packets were already buffered but
//       not decoded, for example to get the codec parameters in mpeg
//       streams */
	FFMPEG_DUMP ("\t"  "packet_buffer "  "= "  "%p"  "\n", c->packet_buffer); //    struct AVPacketList *packet_buffer;
//
//    /** decoding: position of the first frame of the component, in
//       AV_TIME_BASE fractional seconds. NEVER set this value directly:
//       it is deduced from the AVStream values.  */
	FFMPEG_DUMP ("\t"  "start_time "  "= "  "%llu"  "\n", c->start_time); //    int64_t start_time;
//    /** decoding: duration of the stream, in AV_TIME_BASE fractional
//       seconds. NEVER set this value directly: it is deduced from the
//       AVStream values.  */
	FFMPEG_DUMP ("\t"  "duration "  "= "  "%llu"  "\n", c->duration); //    int64_t duration;
//    /** decoding: total file size. 0 if unknown */
	FFMPEG_DUMP ("\t"  "file_size "  "= "  "%llu"  "\n", c->file_size); //    int64_t file_size;
//    /** decoding: total stream bitrate in bit/s, 0 if not
//       available. Never set it directly if the file_size and the
//       duration are known as ffmpeg can compute it automatically. */
	FFMPEG_DUMP ("\t"  "bit_rate "  "= "  "%i"  "\n", c->bit_rate); //    int bit_rate;
//
//    /* av_read_frame() support */
	FFMPEG_DUMP ("\t"  "cur_st "  "= "  "%p"  "\n", c->cur_st); //    AVStream *cur_st;
	FFMPEG_DUMP ("\t"  "cur_ptr "  "= "  "%p"  "\n", c->cur_ptr); //    const uint8_t *cur_ptr;
	FFMPEG_DUMP ("\t"  "cur_len "  "= "  "%i"  "\n", c->cur_len); //    int cur_len;
	FFMPEG_DUMP ("\t"  "cur_pkt "  "= "  "%p"  "\n", &c->cur_pkt); //    AVPacket cur_pkt;
//
//    /* av_seek_frame() support */
	FFMPEG_DUMP ("\t"  "data_offset "  "= "  "%llu"  "\n", c->data_offset); //    int64_t data_offset; /** offset of the first packet */
	FFMPEG_DUMP ("\t"  "index_built "  "= "  "%i"  "\n", c->index_built); //    int index_built;
//
	FFMPEG_DUMP ("\t"  "mux_rate "  "= "  "%i"  "\n", c->mux_rate); //    int mux_rate;
	FFMPEG_DUMP ("\t"  "packet_size "  "= "  "%i"  "\n", c->packet_size); //    int packet_size;
	FFMPEG_DUMP ("\t"  "preload "  "= "  "%i"  "\n", c->preload); //    int preload;
	FFMPEG_DUMP ("\t"  "max_delay "  "= "  "%i"  "\n", c->max_delay); //    int max_delay;
//
//#define AVFMT_NOOUTPUTLOOP -1
//#define AVFMT_INFINITEOUTPUTLOOP 0
//    /** number of times to loop output in formats that support it */
	FFMPEG_DUMP ("\t"  "loop_output "  "= "  "%i"  "\n", c->loop_output); //    int loop_output;
//
	FFMPEG_DUMP ("\t"  "flags "  "= "  "%i"  "\n", c->flags); //    int flags;
//#define AVFMT_FLAG_GENPTS       0x0001 ///< generate pts if missing even if it requires parsing future frames
//#define AVFMT_FLAG_IGNIDX       0x0002 ///< ignore index
//#define AVFMT_FLAG_NONBLOCK     0x0004 ///< do not block when reading packets from input
//
	FFMPEG_DUMP ("\t"  "loop_input "  "= "  "%i"  "\n", c->loop_input); //    int loop_input;
//    /** decoding: size of data to probe; encoding unused */
	FFMPEG_DUMP ("\t"  "probesize "  "= "  "%u"  "\n", c->probesize); //    unsigned int probesize;
//
//    /**
//     * maximum duration in AV_TIME_BASE units over which the input should be analyzed in av_find_stream_info()
//     */
	FFMPEG_DUMP ("\t"  "max_analyze_duration "  "= "  "%i"  "\n", c->max_analyze_duration); //    int max_analyze_duration;
//
	FFMPEG_DUMP ("\t"  "key "  "= "  "%p"  "\n", c->key); //    const uint8_t *key;
	FFMPEG_DUMP ("\t"  "keylen "  "= "  "%i"  "\n", c->keylen); //    int keylen;
//
	//FFMPEG_DUMP ("\t"  "nb_programs "  "= "  "%u"  "\n", c->nb_programs); //    unsigned int nb_programs;
	//FFMPEG_DUMP ("\t"  "programs "  "= "  "%p"  "\n", c->programs); //    AVProgram **programs;
//} AVFormatContext;
}

void AVStream_dump (AVStream* s, int t)
{
	char* tabs = g_strnfill (t, '\t');
	char* tabs1 = g_strnfill (t-1, '\t');
	FFMPEG_DUMP ("%s" "AVStream: %p\n", tabs1, s);
	if (!s) return;
    FFMPEG_DUMP ("%s" "index" " = %i\n", tabs, s->index);//int index;    /**< stream index in AVFormatContext */
    FFMPEG_DUMP ("%s" "id" " = %i\n", tabs, s->id);//    int id;       /**< format specific stream id */
    FFMPEG_DUMP ("%s" "codec" " = %p\n", tabs, s->codec);//AVCodecContext *codec; /**< codec context */
    AVCodecContext_dump (s->codec, t+1);
    /**
     * real base frame rate of the stream.
     * this is the lowest framerate with which all timestamps can be
     * represented accurately (it is the least common multiple of all
     * framerates in the stream), Note, this value is just a guess!
     * for example if the timebase is 1/90000 and all frames have either
     * approximately 3600 or 1800 timer ticks then r_frame_rate will be 50/1
     */
    FFMPEG_DUMP ("%s" "r_frame_rate" " = %p\n", tabs, &s->r_frame_rate);//AVRational r_frame_rate;
    FFMPEG_DUMP ("%s" "priv_data" " = %p\n", tabs, s->priv_data);//void *priv_data;

    /* internal data used in av_find_stream_info() */
    //FFMPEG_DUMP ("%s" "first_dts" " = %lli\n", tabs, s->first_dts);//int64_t first_dts;
#if LIBAVFORMAT_VERSION_INT < (52<<16)
    FFMPEG_DUMP ("%s" "codec_info_nb_frames" " = %i\n", tabs, s->codec_info_nb_frames);//int codec_info_nb_frames;
#endif
    /** encoding: PTS generation when outputing stream */
    FFMPEG_DUMP ("%s" "pts" " = %p\n", tabs, &s->pts);//struct AVFrac pts;

    /**
     * this is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. for fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identically 1.
     */
    FFMPEG_DUMP ("%s" "time_base" " = %p\n", tabs, &s->time_base);//AVRational time_base;
    FFMPEG_DUMP ("%s" "pts_wrap_bits" " = %i\n", tabs, s->pts_wrap_bits);//int pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */
    /* ffmpeg.c private use */
    FFMPEG_DUMP ("%s" "stream_copy" " = %i\n", tabs, s->stream_copy);//int stream_copy; /**< if set, just copy stream */
    FFMPEG_DUMP ("%s" "discard" " = %i\n", tabs, s->discard);//enum AVDiscard discard; ///< selects which packets can be discarded at will and do not need to be demuxed
    //FIXME move stuff to a flags field?
    /** quality, as it has been removed from AVCodecContext and put in AVVideoFrame
     * MN: dunno if that is the right place for it */
    FFMPEG_DUMP ("%s" "quality" " = %f\n", tabs, s->quality);//float quality;
    /**
     * decoding: pts of the first frame of the stream, in stream time base.
     * only set this if you are absolutely 100% sure that the value you set
     * it to really is the pts of the first frame
     * This may be undefined (AV_NOPTS_VALUE).
     * @note the ASF header does NOT contain a correct start_time the ASF
     * demuxer must NOT set this
     */
    FFMPEG_DUMP ("%s" "start_time" " = %llu\n", tabs, s->start_time);//int64_t start_time;
    /**
     * decoding: duration of the stream, in stream time base.
     * If a source file does not specify a duration, but does specify
     * a bitrate, this value will be estimates from bit rate and file size.
     */
    FFMPEG_DUMP ("%s" "duration" " = %llu\n", tabs, s->duration);//int64_t duration;

    FFMPEG_DUMP ("%s" "language" " = %s\n", tabs, s->language);//char language[4]; /** ISO 639 3-letter language code (empty string if undefined) */

    /* av_read_frame() support */
    FFMPEG_DUMP ("%s" "need_parsing" " = %i\n", tabs, s->need_parsing);//enum AVStreamParseType need_parsing;
    FFMPEG_DUMP ("%s" "parser" " = %p\n", tabs, s->parser);//struct AVCodecParserContext *parser;

    FFMPEG_DUMP ("%s" "cur_dts" " = %llu\n", tabs, s->cur_dts);//int64_t cur_dts;
    FFMPEG_DUMP ("%s" "last_IP_duration" " = %i\n", tabs, s->last_IP_duration);//int last_IP_duration;
    FFMPEG_DUMP ("%s" "last_IP_pts" " = %llu\n", tabs, s->last_IP_pts);//int64_t last_IP_pts;
    /* av_seek_frame() support */
    FFMPEG_DUMP ("%s" "index_entries" " = %p\n", tabs, s->index_entries);//AVIndexEntry *index_entries; 
    /**< only used if the format does not
                                    support seeking natively */
    FFMPEG_DUMP ("%s" "nb_index_entries" " = %i\n", tabs, s->nb_index_entries);//int nb_index_entries;
    FFMPEG_DUMP ("%s" "index_entries_allocated_size" " = %u\n", tabs, s->index_entries_allocated_size);//unsigned int index_entries_allocated_size;

    FFMPEG_DUMP ("%s" "nb_frames" " = %llu\n", tabs, s->nb_frames);//int64_t nb_frames;                 ///< number of frames in this stream if known or 0

#define MAX_REORDER_DELAY 4
    FFMPEG_DUMP ("%s" "pts_buffer" " = %p\n", tabs, s->pts_buffer);//int64_t pts_buffer[MAX_REORDER_DELAY+1];
//} AVStream;
}


void AVCodecContext_dump (AVCodecContext* s, int t)
{
	char* tabs = g_strnfill (t, '\t');
	char* tabs1 = g_strnfill (t-1, '\t');
	FFMPEG_DUMP ("%sAVCodecContext: %p\n", tabs1, s);
	if (!s) return;
FFMPEG_DUMP ("%sav_class = %p\n", tabs, s->av_class);
FFMPEG_DUMP ("%sbit_rate = %i\n", tabs, s->bit_rate);
FFMPEG_DUMP ("%sbit_rate_tolerance = %i\n", tabs, s->bit_rate_tolerance);
FFMPEG_DUMP ("%sflags = %i\n", tabs, s->flags);
FFMPEG_DUMP ("%ssub_id = %i\n", tabs, s->sub_id);
FFMPEG_DUMP ("%sme_method = %i\n", tabs, s->me_method);
FFMPEG_DUMP ("%sextradata = %p\n", tabs, s->extradata);
FFMPEG_DUMP ("%sextradata_size = %i\n", tabs, s->extradata_size);
FFMPEG_DUMP ("%stime_base = <AVRational>\n", tabs);
FFMPEG_DUMP ("%swidth = %i\n", tabs, s->width);
FFMPEG_DUMP ("%sgop_size = %i\n", tabs, s->gop_size);
FFMPEG_DUMP ("%spix_fmt = <PixelFormat>\n", tabs);
FFMPEG_DUMP ("%srate_emu = %i\n", tabs, s->rate_emu);
FFMPEG_DUMP ("%sdraw_horiz_band = %p\n", tabs, s->draw_horiz_band);
FFMPEG_DUMP ("%ssample_rate = %i\n", tabs, s->sample_rate);
FFMPEG_DUMP ("%schannels = %i\n", tabs, s->channels);
FFMPEG_DUMP ("%ssample_fmt = <SampleFormat>\n", tabs);
FFMPEG_DUMP ("%sframe_size = %i\n", tabs, s->frame_size);
FFMPEG_DUMP ("%sframe_number = %i\n", tabs, s->frame_number);
FFMPEG_DUMP ("%sreal_pict_num = %i\n", tabs, s->real_pict_num);
FFMPEG_DUMP ("%sdelay = %i\n", tabs, s->delay);
FFMPEG_DUMP ("%sqcompress = %f\n", tabs, s->qcompress);
FFMPEG_DUMP ("%sqblur = %f\n", tabs, s->qblur);
FFMPEG_DUMP ("%sqmin = %i\n", tabs, s->qmin);
FFMPEG_DUMP ("%sqmax = %i\n", tabs, s->qmax);
FFMPEG_DUMP ("%smax_qdiff = %i\n", tabs, s->max_qdiff);
FFMPEG_DUMP ("%smax_b_frames = %i\n", tabs, s->max_b_frames);
FFMPEG_DUMP ("%sb_quant_factor = %f\n", tabs, s->b_quant_factor);
FFMPEG_DUMP ("%src_strategy = %i\n", tabs, s->rc_strategy);
FFMPEG_DUMP ("%sb_frame_strategy = %i\n", tabs, s->b_frame_strategy);
FFMPEG_DUMP ("%shurry_up = %i\n", tabs, s->hurry_up);
FFMPEG_DUMP ("%scodec = %p\n", tabs, s->codec);
FFMPEG_DUMP ("%spriv_data = %p\n", tabs, s->priv_data);
FFMPEG_DUMP ("%srtp_mode = %i\n", tabs, s->rtp_mode);
FFMPEG_DUMP ("%srtp_payload_size = %i\n", tabs, s->rtp_payload_size);
FFMPEG_DUMP ("%srtp_callback = %p\n", tabs, s->rtp_callback);
FFMPEG_DUMP ("%smv_bits = %i\n", tabs, s->mv_bits);
FFMPEG_DUMP ("%sheader_bits = %i\n", tabs, s->header_bits);
FFMPEG_DUMP ("%si_tex_bits = %i\n", tabs, s->i_tex_bits);
FFMPEG_DUMP ("%sp_tex_bits = %i\n", tabs, s->p_tex_bits);
FFMPEG_DUMP ("%si_count = %i\n", tabs, s->i_count);
FFMPEG_DUMP ("%sp_count = %i\n", tabs, s->p_count);
FFMPEG_DUMP ("%sskip_count = %i\n", tabs, s->skip_count);
FFMPEG_DUMP ("%smisc_bits = %i\n", tabs, s->misc_bits);
FFMPEG_DUMP ("%sframe_bits = %i\n", tabs, s->frame_bits);
FFMPEG_DUMP ("%sopaque = %p\n", tabs, s->opaque);
FFMPEG_DUMP ("%scodec_name = %s\n", tabs, s->codec_name);
FFMPEG_DUMP ("%scodec_type = %i\n", tabs, s->codec_type);
FFMPEG_DUMP ("%scodec_id = %i\n", tabs, s->codec_id);
FFMPEG_DUMP ("%scodec_tag = <unsignedint>\n", tabs);
FFMPEG_DUMP ("%sworkaround_bugs = %i\n", tabs, s->workaround_bugs);
FFMPEG_DUMP ("%sluma_elim_threshold = %i\n", tabs, s->luma_elim_threshold);
FFMPEG_DUMP ("%schroma_elim_threshold = %i\n", tabs, s->chroma_elim_threshold);
FFMPEG_DUMP ("%sstrict_std_compliance = %i\n", tabs, s->strict_std_compliance);
FFMPEG_DUMP ("%sb_quant_offset = %f\n", tabs, s->b_quant_offset);
FFMPEG_DUMP ("%serror_resilience = %i\n", tabs, s->error_resilience);
FFMPEG_DUMP ("%sget_buffer = %p\n", tabs, s->get_buffer);
FFMPEG_DUMP ("%srelease_buffer = %p\n", tabs, s->release_buffer);
FFMPEG_DUMP ("%shas_b_frames = %i\n", tabs, s->has_b_frames);
FFMPEG_DUMP ("%sblock_align = %i\n", tabs, s->block_align);
FFMPEG_DUMP ("%sparse_only = %i\n", tabs, s->parse_only);
FFMPEG_DUMP ("%smpeg_quant = %i\n", tabs, s->mpeg_quant);
FFMPEG_DUMP ("%sstats_out = %s\n", tabs, s->stats_out);
FFMPEG_DUMP ("%sstats_in = %s\n", tabs, s->stats_in);
FFMPEG_DUMP ("%src_qsquish = %f\n", tabs, s->rc_qsquish);
FFMPEG_DUMP ("%src_qmod_amp = %f\n", tabs, s->rc_qmod_amp);
FFMPEG_DUMP ("%src_qmod_freq = %i\n", tabs, s->rc_qmod_freq);
FFMPEG_DUMP ("%src_override = %p\n", tabs, s->rc_override);
FFMPEG_DUMP ("%src_override_count = %i\n", tabs, s->rc_override_count);
FFMPEG_DUMP ("%src_eq = %s\n", tabs, s->rc_eq);
FFMPEG_DUMP ("%src_max_rate = %i\n", tabs, s->rc_max_rate);
FFMPEG_DUMP ("%src_min_rate = %i\n", tabs, s->rc_min_rate);
FFMPEG_DUMP ("%src_buffer_size = %i\n", tabs, s->rc_buffer_size);
FFMPEG_DUMP ("%src_buffer_aggressivity = %f\n", tabs, s->rc_buffer_aggressivity);
FFMPEG_DUMP ("%si_quant_factor = %f\n", tabs, s->i_quant_factor);
FFMPEG_DUMP ("%si_quant_offset = %f\n", tabs, s->i_quant_offset);
FFMPEG_DUMP ("%src_initial_cplx = %f\n", tabs, s->rc_initial_cplx);
FFMPEG_DUMP ("%sdct_algo = %i\n", tabs, s->dct_algo);
FFMPEG_DUMP ("%slumi_masking = %f\n", tabs, s->lumi_masking);
FFMPEG_DUMP ("%stemporal_cplx_masking = %f\n", tabs, s->temporal_cplx_masking);
FFMPEG_DUMP ("%sspatial_cplx_masking = %f\n", tabs, s->spatial_cplx_masking);
FFMPEG_DUMP ("%sp_masking = %f\n", tabs, s->p_masking);
FFMPEG_DUMP ("%sdark_masking = %f\n", tabs, s->dark_masking);
FFMPEG_DUMP ("%sunused = %i\n", tabs, s->unused);
FFMPEG_DUMP ("%sidct_algo = %i\n", tabs, s->idct_algo);
FFMPEG_DUMP ("%sslice_count = %i\n", tabs, s->slice_count);
FFMPEG_DUMP ("%sslice_offset = %p\n", tabs, s->slice_offset);
FFMPEG_DUMP ("%serror_concealment = %i\n", tabs, s->error_concealment);
FFMPEG_DUMP ("%sdsp_mask = %u\n", tabs, s->dsp_mask);
FFMPEG_DUMP ("%sbits_per_sample = %i\n", tabs, s->bits_per_sample);
FFMPEG_DUMP ("%sprediction_method = %i\n", tabs, s->prediction_method);
FFMPEG_DUMP ("%ssample_aspect_ratio = <AVRational>\n", tabs);
FFMPEG_DUMP ("%scoded_frame = %p\n", tabs, s->coded_frame);
FFMPEG_DUMP ("%sdebug = %i\n", tabs, s->debug);
FFMPEG_DUMP ("%sdebug_mv = %i\n", tabs, s->debug_mv);
FFMPEG_DUMP ("%serror = %p\n", tabs, s->error);
FFMPEG_DUMP ("%smb_qmin = %i\n", tabs, s->mb_qmin);
FFMPEG_DUMP ("%smb_qmax = %i\n", tabs, s->mb_qmax);
FFMPEG_DUMP ("%sme_cmp = %i\n", tabs, s->me_cmp);
FFMPEG_DUMP ("%sme_sub_cmp = %i\n", tabs, s->me_sub_cmp);
FFMPEG_DUMP ("%smb_cmp = %i\n", tabs, s->mb_cmp);
FFMPEG_DUMP ("%sildct_cmp = %i\n", tabs, s->ildct_cmp);
FFMPEG_DUMP ("%sdia_size = %i\n", tabs, s->dia_size);
FFMPEG_DUMP ("%slast_predictor_count = %i\n", tabs, s->last_predictor_count);
FFMPEG_DUMP ("%spre_me = %i\n", tabs, s->pre_me);
FFMPEG_DUMP ("%sme_pre_cmp = %i\n", tabs, s->me_pre_cmp);
FFMPEG_DUMP ("%spre_dia_size = %i\n", tabs, s->pre_dia_size);
FFMPEG_DUMP ("%sme_subpel_quality = %i\n", tabs, s->me_subpel_quality);
FFMPEG_DUMP ("%sget_format = %p\n", tabs, s->get_format);
FFMPEG_DUMP ("%sdtg_active_format = %i\n", tabs, s->dtg_active_format);
FFMPEG_DUMP ("%sme_range = %i\n", tabs, s->me_range);
FFMPEG_DUMP ("%sintra_quant_bias = %i\n", tabs, s->intra_quant_bias);
FFMPEG_DUMP ("%sinter_quant_bias = %i\n", tabs, s->inter_quant_bias);
FFMPEG_DUMP ("%scolor_table_id = %i\n", tabs, s->color_table_id);
FFMPEG_DUMP ("%sinternal_buffer_count = %i\n", tabs, s->internal_buffer_count);
FFMPEG_DUMP ("%sinternal_buffer = %p\n", tabs, s->internal_buffer);
FFMPEG_DUMP ("%sglobal_quality = %i\n", tabs, s->global_quality);
FFMPEG_DUMP ("%scoder_type = %i\n", tabs, s->coder_type);
FFMPEG_DUMP ("%scontext_model = %i\n", tabs, s->context_model);
FFMPEG_DUMP ("%sslice_flags = %i\n", tabs, s->slice_flags);
FFMPEG_DUMP ("%sxvmc_acceleration = %i\n", tabs, s->xvmc_acceleration);
FFMPEG_DUMP ("%smb_decision = %i\n", tabs, s->mb_decision);
FFMPEG_DUMP ("%sintra_matrix = %p\n", tabs, s->intra_matrix);
FFMPEG_DUMP ("%sinter_matrix = %p\n", tabs, s->inter_matrix);
FFMPEG_DUMP ("%sstream_codec_tag = <unsignedint>\n", tabs);
FFMPEG_DUMP ("%sscenechange_threshold = %i\n", tabs, s->scenechange_threshold);
FFMPEG_DUMP ("%slmin = %i\n", tabs, s->lmin);
FFMPEG_DUMP ("%slmax = %i\n", tabs, s->lmax);
FFMPEG_DUMP ("%spalctrl = %p\n", tabs, s->palctrl);
FFMPEG_DUMP ("%snoise_reduction = %i\n", tabs, s->noise_reduction);
FFMPEG_DUMP ("%sreget_buffer = %p\n", tabs, s->reget_buffer);
FFMPEG_DUMP ("%src_initial_buffer_occupancy = %i\n", tabs, s->rc_initial_buffer_occupancy);
FFMPEG_DUMP ("%sinter_threshold = %i\n", tabs, s->inter_threshold);
FFMPEG_DUMP ("%sflags2 = %i\n", tabs, s->flags2);
FFMPEG_DUMP ("%serror_rate = %i\n", tabs, s->error_rate);
FFMPEG_DUMP ("%santialias_algo = %i\n", tabs, s->antialias_algo);
FFMPEG_DUMP ("%squantizer_noise_shaping = %i\n", tabs, s->quantizer_noise_shaping);
FFMPEG_DUMP ("%sthread_count = %i\n", tabs, s->thread_count);
FFMPEG_DUMP ("%sexecute = %p\n", tabs, s->execute);
FFMPEG_DUMP ("%sthread_opaque = %p\n", tabs, s->thread_opaque);
FFMPEG_DUMP ("%sme_threshold = %i\n", tabs, s->me_threshold);
FFMPEG_DUMP ("%smb_threshold = %i\n", tabs, s->mb_threshold);
FFMPEG_DUMP ("%sintra_dc_precision = %i\n", tabs, s->intra_dc_precision);
FFMPEG_DUMP ("%snsse_weight = %i\n", tabs, s->nsse_weight);
FFMPEG_DUMP ("%sskip_top = %i\n", tabs, s->skip_top);
FFMPEG_DUMP ("%sskip_bottom = %i\n", tabs, s->skip_bottom);
FFMPEG_DUMP ("%sprofile = %i\n", tabs, s->profile);
FFMPEG_DUMP ("%slevel = %i\n", tabs, s->level);
FFMPEG_DUMP ("%slowres = %i\n", tabs, s->lowres);
FFMPEG_DUMP ("%scoded_width = %i\n", tabs, s->coded_width);
FFMPEG_DUMP ("%sframe_skip_threshold = %i\n", tabs, s->frame_skip_threshold);
FFMPEG_DUMP ("%sframe_skip_factor = %i\n", tabs, s->frame_skip_factor);
FFMPEG_DUMP ("%sframe_skip_exp = %i\n", tabs, s->frame_skip_exp);
FFMPEG_DUMP ("%sframe_skip_cmp = %i\n", tabs, s->frame_skip_cmp);
FFMPEG_DUMP ("%sborder_masking = %f\n", tabs, s->border_masking);
FFMPEG_DUMP ("%smb_lmin = %i\n", tabs, s->mb_lmin);
FFMPEG_DUMP ("%smb_lmax = %i\n", tabs, s->mb_lmax);
FFMPEG_DUMP ("%sme_penalty_compensation = %i\n", tabs, s->me_penalty_compensation);
FFMPEG_DUMP ("%sskip_loop_filter = <AVDiscard>\n", tabs);
FFMPEG_DUMP ("%sskip_idct = <AVDiscard>\n", tabs);
FFMPEG_DUMP ("%sskip_frame = <AVDiscard>\n", tabs);
FFMPEG_DUMP ("%sbidir_refine = %i\n", tabs, s->bidir_refine);
FFMPEG_DUMP ("%sbrd_scale = %i\n", tabs, s->brd_scale);
FFMPEG_DUMP ("%scrf = %f\n", tabs, s->crf);
FFMPEG_DUMP ("%scqp = %i\n", tabs, s->cqp);
FFMPEG_DUMP ("%skeyint_min = %i\n", tabs, s->keyint_min);
FFMPEG_DUMP ("%srefs = %i\n", tabs, s->refs);
FFMPEG_DUMP ("%schromaoffset = %i\n", tabs, s->chromaoffset);
FFMPEG_DUMP ("%sbframebias = %i\n", tabs, s->bframebias);
FFMPEG_DUMP ("%strellis = %i\n", tabs, s->trellis);
FFMPEG_DUMP ("%scomplexityblur = %f\n", tabs, s->complexityblur);
FFMPEG_DUMP ("%sdeblockalpha = %i\n", tabs, s->deblockalpha);
FFMPEG_DUMP ("%sdeblockbeta = %i\n", tabs, s->deblockbeta);
FFMPEG_DUMP ("%spartitions = %i\n", tabs, s->partitions);
FFMPEG_DUMP ("%sdirectpred = %i\n", tabs, s->directpred);
FFMPEG_DUMP ("%scutoff = %i\n", tabs, s->cutoff);
FFMPEG_DUMP ("%sscenechange_factor = %i\n", tabs, s->scenechange_factor);
FFMPEG_DUMP ("%smv0_threshold = %i\n", tabs, s->mv0_threshold);
FFMPEG_DUMP ("%sb_sensitivity = %i\n", tabs, s->b_sensitivity);
FFMPEG_DUMP ("%scompression_level = %i\n", tabs, s->compression_level);
FFMPEG_DUMP ("%suse_lpc = %i\n", tabs, s->use_lpc);
FFMPEG_DUMP ("%slpc_coeff_precision = %i\n", tabs, s->lpc_coeff_precision);
FFMPEG_DUMP ("%smin_prediction_order = %i\n", tabs, s->min_prediction_order);
FFMPEG_DUMP ("%smax_prediction_order = %i\n", tabs, s->max_prediction_order);
FFMPEG_DUMP ("%sprediction_order_method = %i\n", tabs, s->prediction_order_method);
FFMPEG_DUMP ("%smin_partition_order = %i\n", tabs, s->min_partition_order);
FFMPEG_DUMP ("%smax_partition_order = %i\n", tabs, s->max_partition_order);
FFMPEG_DUMP ("%stimecode_frame_start = %llu\n", tabs, s->timecode_frame_start);
//FFMPEG_DUMP ("%srequest_channels = %i\n", tabs, s->request_channels);


}


void AVPacket_dump (AVPacket* pkt, int t)
{
	char* tabs = g_strnfill (t, '\t');
	char* tabs1 = g_strnfill (t-1, '\t');
	FFMPEG_DUMP ("%sAVPacket: %p\n", tabs1, pkt);
	if (!pkt) return;
	
	FFMPEG_DUMP ("%spts = %lld\n", tabs, pkt->pts);
	FFMPEG_DUMP ("%sdts = %lld\n", tabs, pkt->dts);
	FFMPEG_DUMP ("%sdata = %p\n", tabs, pkt->data);
	FFMPEG_DUMP ("%ssize = %i\n", tabs, pkt->size);
	FFMPEG_DUMP ("%sstream_index = %i\n", tabs, pkt->stream_index);
	FFMPEG_DUMP ("%sflags = %i\n", tabs, pkt->flags);
	FFMPEG_DUMP ("%sduration = %i\n", tabs, pkt->duration);
	FFMPEG_DUMP ("%spos = %lld\n", tabs, pkt->pos);
	
}
