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

#include "asf-ffmpeg.h"
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

int ffmpeg_asf_probe(AVProbeData *pd)
{
	printf ("ffmpeg_asf_probe (%p), filename: %s, buf: %p, buf_size: %i\n", pd, pd->filename, pd->buf, pd->buf_size);
	
	if ((size_t) pd->buf_size < sizeof (asf_guid))
		return 0;
		
	if (asf_guid_compare (&asf_guids_header, (asf_guid*) pd->buf)) {
		printf ("ffmpeg_asf_probe (%p): Success.\n", pd);
		return AVPROBE_SCORE_MAX;
	}
	
	return 0;
}

int ffmpeg_asf_read_header(AVFormatContext *s, AVFormatParameters *ap)
{
	printf ("ffmpeg_asf_read_header (%p, %p).\n", s, ap);
	
	FFMPEGParser *parser = new FFMPEGParser (s->filename);
	MoonASFContext *context = (MoonASFContext*) s->priv_data;
	context->parser = parser;
	
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
			printf ("ffmpeg_asf_read_header (%p, %p): Unknown stream type: %s with index: %i.\n", s, ap, asf_guid_tostring (&asp->stream_type), index);
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
			printf ("Audio codec.\n");
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
				
				printf ("Audio codec id: %i = %i.\n", stream->codec->codec_id, id);
				switch (id) {
				case 0X160:
					stream->codec->codec_id = CODEC_ID_WMAV1;
					break;
				case 0x161:
					stream->codec->codec_id = CODEC_ID_WMAV2;
					break;
				default:
					printf ("Unknown audio codec: %i.\n", id);
					goto fail;
				}
			}
		} else if (codec_type == CODEC_TYPE_VIDEO) {
			printf ("Video codec.\n");
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
					
					int tag = stream->codec->codec_tag;
					char a = *(char*) &tag;
					char b = *(1 + (char*) &tag);
					char c = *(2 + (char*) &tag);
					char d = *(3 + (char*) &tag);
					printf ("Video codec tag: %i (%c %c %c %c).\n", tag, a, b, c, d);
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
						printf ("Unknown video codec: %i (%c%c%c%c).\n", tag, a, b, c, d);
						goto fail;
					}
				}
			}
		} else {
			printf ("Unimplemented codec type: %i.\n", codec_type);
			goto fail;
		}
	}
	
	return 0;
	
fail:
	printf ("Failure cleanup not implemented.\n");
	return -1;
}

int ffmpeg_asf_read_packet(AVFormatContext *s, AVPacket *av_packet)
{
	printf ("ffmpeg_asf_read_packet (%p, %p), parser: %p\n", s, av_packet, s->priv_data);
	
	MoonASFContext *context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	FFMPEGPacket* packet = NULL;
	int current_stream = 0;
	
	packet = (FFMPEGPacket*) parser->GetCurrentPacket ();
	
	if (packet != NULL && packet->current_stream == -1) {
		printf ("ffmpeg_asf_read_packet: Deleting packet: %p.\n", packet);
		//delete packet; //the parser deletes the packet.
		packet = NULL;
	}
	
	if (packet == NULL) {
		if (parser->source->Position () >= parser->packet_offset_end) {
			printf ("ffmpeg_asf_read_packet: no more packets.\n");
			return -1;
		}
		
		// We have to seek to the next packet
		int64_t pos = parser->GetPacketOffset (parser->packets_read);
		
		printf ("ffmpeg_asf_read_packet, seeking to packet #%lld (at position %lld, current position: %lld).\n", parser->packets_read + 1, pos, parser->source->Position ());
		printf ("\tdata_offset = %lld, data_size = %lld, packets_read = %lld, min_packet_size = %i.\n", parser->data_offset,
			parser->data ? parser->data->size : (int64_t) 0, 
			parser->packets_read, parser->GetFileProperties ()->min_packet_size);
		
		parser->source->Seek (pos, SEEK_SET);
		
		packet = new FFMPEGPacket ();
		if (!parser->ReadPacket (packet)) {
			return -1;
		}
		parser->packets_read++;
		packet->stream_count = packet->payloads->get_stream_count ();
		printf ("ffmpeg_asf_read_packet: Created new packet %p with %i streams.\n", packet, packet->stream_count);
	}

	packet->current_stream++;
	current_stream = packet->current_stream;
	
	printf ("ffmpeg_asf_read_packet: Reading stream #%i.\n", current_stream);
		
	if (packet->current_stream == packet->stream_count) {
		printf ("ffmpeg_asf_read_packet: setting current_stream of packet %p to %i.\n", packet, packet->current_stream); 
		packet->current_stream = -1;
	}
	
	asf_dword stream_size = packet->payloads->get_stream_size (current_stream);
	
	AVPacket* pkt = new AVPacket ();
	
	av_new_packet (pkt, stream_size);
		
	pkt->pts = packet->payloads->get_presentation_time (current_stream);
	pkt->dts = 0;
	pkt->stream_index = parser->ConvertStreamIndex (current_stream); 
	pkt->pos = -1;
	pkt->size = stream_size;
	
	if (packet->payloads->is_key_frame (current_stream) || s->streams [pkt->stream_index]->codec->codec_type == CODEC_TYPE_AUDIO) {
		pkt->flags |= PKT_FLAG_KEY;
	}
	
	packet->payloads->write_payload_data (current_stream, pkt->data, stream_size);
	
	printf ("\tpkt->pts = %lld\n", pkt->pts);
	printf ("\tpkt->dts = %lld\n", pkt->dts);
	printf ("\tpkt->data = %p\n", pkt->data);
	printf ("\tpkt->size = %i\n", pkt->size);
	printf ("\tpkt->stream_index = %i\n", pkt->stream_index);
	printf ("\tpkt->flags = %i\n", pkt->flags);
	printf ("\tpkt->duration = %i\n", pkt->duration);
	printf ("\tpkt->pos = %lld\n", pkt->pos);
	
	*av_packet = *pkt; 
	
	return 0;
}

int ffmpeg_asf_read_close(AVFormatContext *s)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	printf ("ffmpeg_asf_read_close (%p).\n", s);
	
	delete parser;
	
	return -1;
}

int ffmpeg_asf_read_seek(AVFormatContext *s, int stream_index, int64_t pts, int flags)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	
	printf ("ffmpeg_asf_read_seek (%p, %i, %lld, %i).\n", s, stream_index, pts, flags);
	
	return -1;
}

int64_t ffmpeg_asf_read_pts(AVFormatContext *s, int ffmpeg_stream_index, int64_t *start_pos, int64_t pos_limit)
{
	MoonASFContext* context = (MoonASFContext*) s->priv_data;
	FFMPEGParser* parser = context->parser;
	
	printf ("ffmpeg_asf_read_pts (%p, %i, %p (%lld), %lld).\n", s, ffmpeg_stream_index, start_pos, *start_pos, pos_limit);
	
	int64_t packet_index = 0;
	int64_t packet_offset = 0;
	int64_t pts = 0;
	int64_t pos = *start_pos;
	asf_file_properties* file_properties = parser->GetFileProperties ();
	
	pos_limit = parser->packet_offset_end;
	
	// Find the packet at the start position
	if (pos < parser->data_offset) {
		printf ("ffmpeg_asf_read_pts: start position (%lld) is before start of data (%lld).\n", pos, parser->data_offset);
		return AV_NOPTS_VALUE;
	}
	
	// CHECK: what if min_packet_size != max_packet_size?
	packet_index = (pos - parser->data_offset) / file_properties->min_packet_size;
	packet_offset = parser->GetPacketOffset (packet_index);
	
	printf ("ffmpeg_asf_read_pts: calculated packet index %lld and packet offset %lld.\n", packet_index, packet_offset);
	
	if (packet_index >= (int64_t) file_properties->data_packet_count) {
		printf ("ffmpeg_asf_read_pts: calculated packet index (%lld) is more than packet count (%lld).\n", packet_index, file_properties->data_packet_count); 
		return AV_NOPTS_VALUE;
	}
	
	if (!parser->source->Seek (packet_offset, SEEK_SET)) {
		printf ("ffmpeg_asf_read_pts: could not seek to position %lld.\n", packet_offset);
		return AV_NOPTS_VALUE;
	}
	
	int stream_index = parser->ConvertFFMpegStreamIndex (ffmpeg_stream_index);
	if (stream_index == 0) {
		printf ("ffmpeg_asf_read_pts: could not find asf stream of ffmpeg stream index %i.\n", ffmpeg_stream_index);
		return AV_NOPTS_VALUE;
	}
	
	printf ("ffmpeg_asf_read_pts: searching for pts of position %lld of asf stream %i.\n", *start_pos, stream_index); 
	
	bool is_audio_stream = parser->GetStream (stream_index)->is_audio ();
	
	while (packet_offset > pos_limit) {
		packet_offset += file_properties->min_packet_size;
		packet_index++;
		if (!parser->ReadPacket ()) {
			printf ("ffmpeg_asf_read_pts: could not read packet index %lld at position %lld.\n", packet_index, packet_offset);
			return AV_NOPTS_VALUE;
		}
		
		ASFPacket* packet = parser->GetCurrentPacket ();
		
		if (!is_audio_stream && !packet->payloads->is_key_frame (stream_index))
			continue;

		*start_pos = packet_offset;
		
		return packet->payloads->get_presentation_time (stream_index);
	}
		
	printf ("fmpeg_asf_read_pts: found pts (%lld) of packet index %lld.\n", pts, packet_index);
	
	return AV_NOPTS_VALUE;
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
