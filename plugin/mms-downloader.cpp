/*
 * mms-downloader.cpp: Mms implementation of a Browser Http downloader
 *
 * Author:
 *   Geoff Norton  (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "mms-downloader.h"

static inline bool
is_valid_mms_header (MmsHeader *header)
{
	if (header->id != MMS_DATA && header->id != MMS_HEADER && header->id != MMS_METADATA && header->id != MMS_STREAM_C && header->id != MMS_END && header->id != MMS_PAIR_P)
		return false;

	return true;
}

void
MmsDownloader::Abort ()
{
	if (response != NULL)
		response->Abort ();
}

uint32_t
MmsDownloader::Read (char *buffer, uint32_t length)
{
	MmsHeader *header;
	MmsPacket *packet;
	char *payload;
	uint32_t offset = 0;

	// Resize our internal buffer
	if (this->buffer == NULL) {
		this->buffer = (char *) g_malloc (length);
	} else {
		this->buffer = (char *) g_realloc (this->buffer, this->size + length);
	}

	// Populate the data into the buffer
	memcpy (this->buffer + this->size, buffer, length); 
	this->size += length;

	// FIXME: We shouldn't poll here, we should notify over
	pd->dl->RequestPosition (&requested_position);
	if (requested_position != -1) {
		this->Abort ();
		this->Send ();
		return DOWNLOADER_ERR;
	}

	// Check if we have an entire packet available.
process_packet:
	if (this->size < sizeof (MmsHeader))
		return DOWNLOADER_OK;

	header = (MmsHeader *) this->buffer;

	if (!is_valid_mms_header (header)) {
		this->Abort ();
		return DOWNLOADER_ERR;
	}


	if (this->size < (header->length + sizeof (MmsHeader)))
		return DOWNLOADER_OK;

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
	
	return DOWNLOADER_OK;
}

bool
MmsDownloader::ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	*offset = (header->length + sizeof (MmsHeader));
 
	if (header->id == MMS_HEADER)
		return ProcessHeaderPacket (header, packet, payload, offset);
	if (header->id == MMS_METADATA)
		return ProcessMetadataPacket (header, packet, payload, offset);
	if (header->id == MMS_PAIR_P)
		return ProcessPairPacket (header, packet, payload, offset);
	if (header->id == MMS_DATA)
		return ProcessDataPacket (header, packet, payload, offset);

	g_warning ("MmsDownloader::ProcessPacket received a unknown packet type in an impossible manner.");

	return false;
}

bool
MmsDownloader::ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
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
		uint8_t stream_count, current_stream;

		// Count the number of streams
		stream_count = 0;
		for (int i = 1; i <= 127; i++) {
			if (parser->IsValidStream (i))
				stream_count++;
		}

		for (int i = 0; i < stream_count; i++) {
			while (current_stream <= 127 && !parser->IsValidStream (current_stream))
				current_stream++;

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
		this->Abort ();
		this->Send ();
	
		delete parser;

		return false;
	}
	this->asf_packet_size = parser->GetPacketSize ();
	this->header_size = header->length - sizeof (MmsDataPacket);

	delete parser;

	if (properties->file_size == this->header_size) {
		this->seekable = false;
	} else {
		this->seekable = true;
		pd->dl->NotifySize (properties->file_size);
	}

	// FIXME: Dont poll the requested position, notify over
	if (requested_position == -1);
		pd->dl->Write (payload, 0, this->header_size);

	return true;
}

bool
MmsDownloader::ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
	// CHECKME: It appears we used to parse the metadata for absolutely no reason
	// We skip the entire packet and header
	return true;
}

bool
MmsDownloader::ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *offset)
{
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
	pd->dl->Write (payload, (this->seekable ? (this->header_size + packet->packet.data.id*this->asf_packet_size) : header->length), this->asf_packet_size);

	return true;
}

void
MmsDownloader::Send ()
{
	PluginInstance *instance = GetPlugin ();

	if (instance == NULL || instance->GetBridge () == NULL)
		return;

	BrowserRequest *request = instance->GetBridge ()->CreateBrowserRequest ("GET", pd->GetUri ());

	request->SetHttpHeader ("User-Agent", "NSPlayer/11.1.0.3856");
	request->SetHttpHeader ("Pragma", "no-cache,xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}");

	if (this->described) {
		request->SetHttpHeader ("Pragma", "rate=1.000000,stream-offset=0:0,max-duration=0");
		request->SetHttpHeader ("Pragma", "xPlayStrm=1");
                if (requested_position != -1) {
			char *header = g_strdup_printf ("stream-time=%lld, packet-num=4294967295", requested_position / 10000);
			request->SetHttpHeader ("Pragma", header);
			g_free (header);
		}

		/* stream-switch-count && stream-switch-entry need to be on their own pragma lines
		 * we (ab)use SetBody for this*/
		char *stream_headers = g_strdup_printf ("Pragma: stream-switch-count=2\r\nPragma: stream-switch-entry=ffff:%i:0 ffff:%i:0\r\n\r\n", this->best_video_stream, this->best_audio_stream);
		request->SetBody (stream_headers, strlen (stream_headers));
		g_free (stream_headers);
	} else {
		request->SetHttpHeader ("Supported", "com.microsoft.wm.srvppair,com.microsoft.wm.sswitch,com.microsoft.wm.predstrm,com.microsoft.wm.startupprofile");
		request->SetHttpHeader ("Pragma", "packet-pair-experiment=1");
	}
	request->GetResponse (browser_downloader_started, browser_downloader_available, browser_downloader_finished, this);
}

void
MmsDownloader::Started ()
{
}

void
MmsDownloader::Finished ()
{
}
