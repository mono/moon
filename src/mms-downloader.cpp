/*
 * mms-downlodaer.cpp: MMS Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include "asf/asf.h"

#include "mms-downloader.h"

#define LOG_MMS(...) //printf (__VA_ARGS__);

static inline bool
is_valid_mms_header (MmsHeader *header)
{
	if (header->id != MMS_DATA && header->id != MMS_HEADER && header->id != MMS_METADATA && header->id != MMS_STREAM_C && header->id != MMS_END && header->id != MMS_PAIR_P)
		return false;

	return true;
}

MmsDownloader::MmsDownloader (Downloader *dl) : InternalDownloader (dl)
{
	this->uri = NULL;
	this->buffer = NULL;

	this->asf_packet_size = 0;
	this->header_size = 0;
	this->requested_position = -1;
	this->size = 0;
	this->packets_received = 0;

	this->p_packet_count = 0;

	this->described = false;
	this->seekable = false;

	this->best_audio_stream = 0;
	this->best_video_stream = 0;
	this->best_audio_stream_rate = 0;
	this->best_video_stream_rate = 0;

	memset (audio_streams, 0xff, 128 * 4);
	memset (video_streams, 0xff, 128 * 4);
}

MmsDownloader::~MmsDownloader ()
{
	if (this->buffer != NULL) {
		g_free (this->buffer);
	}
}

void
MmsDownloader::Open (const char *verb, const char *uri)
{
	this->uri = g_strdup_printf ("http://%s", uri+6);

	dl->InternalOpen (verb, this->uri, true);

	dl->InternalSetHeader ("User-Agent", "NSPlayer/11.1.0.3856");
	dl->InternalSetHeader ("Pragma", "no-cache,xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}");
	dl->InternalSetHeader ("Supported", "com.microsoft.wm.srvppair,com.microsoft.wm.sswitch,com.microsoft.wm.predstrm,com.microsoft.wm.startupprofile");
	dl->InternalSetHeader ("Pragma", "packet-pair-experiment=1");
}

void
MmsDownloader::Write (void *buf, int32_t off, int32_t n)
{
	MmsHeader *header;
	MmsPacket *packet;
	char *payload;
	uint32_t offset = 0;

	// Resize our internal buffer
	if (this->buffer == NULL) {
		this->buffer = (char *) g_malloc (n);
	} else {
		this->buffer = (char *) g_realloc (this->buffer, this->size + n);
	}

	// Populate the data into the buffer
	memcpy (this->buffer + this->size, buf, n);
	this->size += n;

	// FIXME: We shouldn't poll here, we should notify over
	dl->RequestPosition (&requested_position);
	if (requested_position != -1) {
		dl->InternalAbort ();

		dl->InternalOpen ("GET", uri, true);
		dl->InternalSetHeader ("User-Agent", "NSPlayer/11.1.0.3856");
		dl->InternalSetHeader ("Pragma", "no-cache,xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}");
		dl->InternalSetHeader ("Pragma", "rate=1.000000,stream-offset=0:0,max-duration=0");
		dl->InternalSetHeader ("Pragma", "xPlayStrm=1");

		char *header = g_strdup_printf ("stream-time=%lld, packet-num=4294967295", requested_position / 10000);
		dl->InternalSetHeader ("Pragma", header);
		g_free (header);
		/* stream-switch-count && stream-switch-entry need to be on their own pragma lines
		 * we (ab)use SetBody for this*/
		char *stream_headers = g_strdup_printf ("Pragma: stream-switch-count=2\r\nPragma: stream-switch-entry=ffff:%i:0 ffff:%i:0\r\n\r\n", this->best_video_stream, this->best_audio_stream);
		dl->InternalSetBody (stream_headers, strlen (stream_headers));
		g_free (stream_headers);

		dl->Send ();

		return;
	}

	// Check if we have an entire packet available.
process_packet:
	if (this->size < sizeof (MmsHeader))
		return;

	header = (MmsHeader *) this->buffer;

	if (!is_valid_mms_header (header)) {
		dl->Abort ();
		dl->NotifyFailed ("invalid mms source");
		return;
	}


	if (this->size < (header->length + sizeof (MmsHeader)))
		return;

	packet = (MmsPacket *) (this->buffer + sizeof (MmsHeader));
	payload = (this->buffer + sizeof (MmsHeader) + sizeof (MmsDataPacket));

	if (ProcessPacket (header, packet, payload, &offset)) {
		if (this->size - offset > 0) {
			// FIXME: We should refactor this to increment the buffer pointer to the new position
			// but coalense the free / malloc / memcpy into batches to improve performance on big
			// streams
			char *new_buffer = (char *) g_malloc (this->size - offset);
			memcpy (new_buffer, this->buffer + offset, this->size - offset);
			g_free (this->buffer);

			this->buffer = new_buffer;
		} else {
			g_free (this->buffer);
			this->buffer = NULL;
		}

		this->size -= offset;

		goto process_packet;
	}
}

bool
MmsDownloader::ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	LOG_MMS ("MmsDownloader::ProcessPacket ()\n");
	
	*offset = (header->length + sizeof (MmsHeader));
 
 	switch (header->id) {
	case MMS_HEADER:
		return ProcessHeaderPacket (header, packet, payload, offset);
	case MMS_METADATA:
		return ProcessMetadataPacket (header, packet, payload, offset);
	case MMS_PAIR_P:
		return ProcessPairPacket (header, packet, payload, offset);
	case MMS_DATA:
		return ProcessDataPacket (header, packet, payload, offset);
	case MMS_END:
		return true; // TODO: Do we need to do something here?
	case MMS_STREAM_C:
		return true; // TODO: Do we need to do something here?
	}
	
	g_warning ("MmsDownloader::ProcessPacket received a unknown packet type %i in an impossible manner.", (int) header->id);

	return false;
}

bool
MmsDownloader::ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	LOG_MMS ("MmsDownloader::ProcessHeaderPacket ()\n");
	
	MemorySource *asf_src = new MemorySource (NULL, payload, header->length-sizeof (MmsDataPacket), 0);
	ASFParser *parser = new ASFParser (asf_src, NULL);

	asf_src->SetOwner (false);
	asf_src->unref ();
	if (!parser->ReadHeader ()) {
		this->asf_packet_size = ASF_DEFAULT_PACKET_SIZE;
		delete parser;
		return true;
	}

	asf_file_properties *properties = parser->GetFileProperties ();

	if (!this->described) {
		gpointer state;
		uint8_t current_stream;

		for (int i = 1; i < 127; i++) {
			if (!parser->IsValidStream (i))
				continue;
				
			current_stream = i;

			const asf_stream_properties *stream_properties = parser->GetStream (current_stream);
			const asf_extended_stream_properties *extended_stream_properties = parser->GetExtendedStream (current_stream);

			if (stream_properties == NULL) {
				g_warning ("The file claims there were more streams than we could locate");
				continue;
			}

			if (stream_properties->is_audio ()) {
				const WAVEFORMATEX* wave = stream_properties->get_audio_data ();
				this->AddAudioStream (current_stream, wave->bytes_per_second * 8);
			}
			if (stream_properties->is_video ()) {
				int bit_rate = 0;
				const asf_video_stream_data* video_data = stream_properties->get_video_data ();
				const BITMAPINFOHEADER* bmp;

				if (extended_stream_properties != NULL) {
					bit_rate = extended_stream_properties->data_bitrate;
				} else if (video_data != NULL) {
					bmp = video_data->get_bitmap_info_header ();
					if (bmp != NULL) {
						bit_rate = bmp->image_width*bmp->image_height;
					}
				}

				this->AddVideoStream (current_stream, bit_rate);
			}
			current_stream++;
		}
		this->described = true;

		// Free the current buffer of data, we've collected enough for our Describe request
		g_free (this->buffer);
		this->buffer = NULL;
		this->size = 0;

		// Abort and resend the real request
		dl->InternalAbort ();

		dl->InternalOpen ("GET", uri, true);
		dl->InternalSetHeader ("User-Agent", "NSPlayer/11.1.0.3856");
		dl->InternalSetHeader ("Pragma", "no-cache,xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}");
		dl->InternalSetHeader ("Pragma", "rate=1.000000,stream-offset=0:0,max-duration=0");
		dl->InternalSetHeader ("Pragma", "xPlayStrm=1");
		/* stream-switch-count && stream-switch-entry need to be on their own pragma lines
		 * we (ab)use SetBody for this*/
		char *stream_headers = g_strdup_printf ("Pragma: stream-switch-count=2\r\nPragma: stream-switch-entry=ffff:%i:0 ffff:%i:0\r\n\r\n", this->best_video_stream, this->best_audio_stream);
		dl->InternalSetBody (stream_headers, strlen (stream_headers));
		g_free (stream_headers);

		dl->Send ();
	
		delete parser;

		return false;
	}
	this->asf_packet_size = parser->GetPacketSize ();
	this->header_size = header->length - sizeof (MmsDataPacket);

	if (properties->file_size == this->header_size) {
		this->seekable = false;
	} else {
		this->seekable = true;
		dl->NotifySize (properties->file_size);
	}

	dl->InternalWrite (payload, 0, this->header_size);

	delete parser;

	return true;
}

bool
MmsDownloader::ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (%p, %p, %s, %p)\n", header, packet, payload, offset);
	
	//playlist-gen-id and broadcast-id isn't used anywhere right now,
	//but I'm keeping the code since we'll probably need it
	//uint32_t playlist_gen_id = 0;
	//uint32_t broadcast_id = 0;
	HttpStreamingFeatures features = HttpStreamingFeaturesNone;
	
	char *start = payload;
	char *key = NULL, *value = NULL;
	char *state = NULL;
	
	// format: key=value,key=value\0
	// example:
	// playlist-gen-id=1,broadcast-id=2,features="broadcast,seekable"\0
	
	// Make sure payload is null-terminated
	for (int i = 0; i < packet->packet.data.size; i++) {
		if (payload [i] == 0)
			break;
		if (i == packet->packet.data.size - 1)
			payload [i] = NULL;
	}
	
	do {
		key = strtok_r (start, "=", &state);
		start = NULL;
		
		if (key == NULL)
			break;
			
		if (key [0] == ' ')
			key++;
		
		if (!strcmp (key, "features")) {
			value = strtok_r (NULL, "\"", &state);
		} else {
			value = strtok_r (NULL, ",", &state);
		}
		
		if (value == NULL)
			break;
			
		LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): %s=%s\n", key, value);
		
		if (!strcmp (key, "playlist-gen-id")) {
			//playlist_gen_id = strtoul (value, NULL, 10);
		} else if (!strcmp (key, "broadcast-id")) {
			//broadcast_id = strtoul (value, NULL, 10);
		} else if (!strcmp (key, "features")) {
			features = parse_http_streaming_features (value);
			dl->SetHttpStreamingFeatures (features);
		} else {
			printf ("MmsDownloader::ProcessMetadataPacket (): Unexpected metadata: %s=%s\n", key, value);
		}
	} while (true);
	
	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): features: %i\n", features);
	
	return true;
}

bool
MmsDownloader::ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	LOG_MMS ("MmsDownloader::ProcessPairPacket ()\n");
	
	// NOTE: If this is the 3rd $P packet, we need to increase the size reported in the header by
	// the value in the reason field.  This is a break from the normal behaviour of MMS packets
	// so we need to guard against this occurnace here and ensure we actually have enough data
	// buffered to consume
	if (p_packet_count == 2 && this->size < (header->length + sizeof (MmsHeader) + packet->packet.reason))
		return false;

	// NOTE: We also need to account for the size of the reason field manually with our packet massaging.
	*offset += 4;

	// NOTE: If this is the first $P packet we've seen the reason is actually the amount of data
	// that the header claims is in the payload, but is in fact not.
	if (p_packet_count == 0) {
		*offset -= packet->packet.reason;
	}

	// NOTE: If this is the third $P packet we've seen, reason is an amount of data that the packet
	// is actually larger than the advertised packet size
	if (p_packet_count == 2)
		*offset += packet->packet.reason;

	++p_packet_count;

	// FIXME: Log the actual time stamp of packet creation so that we can do appropriate stream-select
	// after finishing the describe request

	return true;
}

bool
MmsDownloader::ProcessDataPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	LOG_MMS ("MmsDownloader::ProcessDataPacket ()\n");
	
	int32_t off = this->header_size;
	int32_t size = this->asf_packet_size;
		
	if (this->seekable) {
		off += packet->packet.data.id * size;
	} else {
		off += packets_received * size;
	}
	
	dl->InternalWrite (payload, off, size);
	packets_received++;
	
	return true;
}
