/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-asf.cpp: ASF related parts of the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include "pipeline-asf.h"
#include "mms-downloader.h"
#include "debug.h"

/*
 * ASFDemuxer
 */

ASFDemuxer::ASFDemuxer (Media *media, IMediaSource *source) : IMediaDemuxer (Type::ASFDEMUXER, media, source)
{
	stream_to_asf_index = NULL;
	reader = NULL;
	parser = NULL;
}

void
ASFDemuxer::Dispose ()
{
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;
	
	delete reader;
	reader = NULL;

	if (parser) {
		parser->Dispose ();
		parser->unref ();
		parser = NULL;
	}
	
	IMediaDemuxer::Dispose ();
}

void
ASFDemuxer::UpdateSelected (IMediaStream *stream)
{
	if (reader)
		reader->SelectStream (stream_to_asf_index [stream->index], stream->GetSelected ());

	IMediaDemuxer::UpdateSelected (stream);
}

void
ASFDemuxer::SeekAsyncInternal (guint64 pts)
{
	MediaResult result;
	
	LOG_PIPELINE ("ASFDemuxer::Seek (%" G_GUINT64_FORMAT ")\n", pts);
	
	g_return_if_fail (reader != NULL);
	g_return_if_fail (media != NULL);
	g_return_if_fail (media->InMediaThread ());
	
	result = reader->Seek (pts);
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportSeekCompleted (pts);
	} else if (result == MEDIA_NOT_ENOUGH_DATA) {
		EnqueueSeek (pts);
	} else {
		ReportErrorOccurred (result);
	}
}

void
ASFDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	LOG_PIPELINE ("ASFDemuxer::SwitchMediaStreamAsyncInternal (%p). TODO.\n", stream);
}

void
ASFDemuxer::ReadMarkers ()
{
	/*
		We can get markers from several places:
			- The header of the file, read before starting to play
				- As a SCRIPT_COMMAND
				- As a MARKER
				They are both treated the same way, added into the timeline marker collection when the media is loaded.
			- As data in the file (a separate stream whose type is ASF_COMMAND_MEDIA)
				These markers show up while playing the file, and they don't show up in the timeline marker collection,
				they only get to raise the MarkerReached event.
				currently the demuxer will call the streamed_marker_callback when it encounters any of these.    
	*/
	
	// Hookup to the marker (ASF_COMMAND_MEDIA) stream
	MediaMarker *marker;
	
	// Read the markers (if any)
	List *markers = media->GetMarkers ();
	const char *type;
	guint64 pts;
	guint64 preroll_pts = MilliSeconds_ToPts (parser->GetFileProperties ()->preroll);
	char *text;
	int i = -1;
	
	// Read the SCRIPT COMMANDs
	char **command_types = NULL;
	asf_script_command_entry **commands = NULL;
	asf_script_command *command = parser->script_command;
	
	if (command != NULL) {
		commands = command->get_commands (parser, &command_types);
		
		if (command_types == NULL) {
			//printf ("MediaElement::ReadASFMarkers (): No command types.\n");
			goto cleanup;
		}
	}
	
	if (commands != NULL) {
		for (i = 0; commands[i]; i++) {
			asf_script_command_entry *entry = commands [i];
			
			text = entry->get_name ();
			pts = MilliSeconds_ToPts (entry->pts) - preroll_pts;
			
			if (entry->type_index + 1 <= command->command_type_count)
				type = command_types [entry->type_index];
			else
				type = "";

			marker = new MediaMarker (type, text, pts);
			markers->Append (new MediaMarker::Node (marker));
			marker->unref ();
			
			//printf ("MediaElement::ReadMarkers () Added script command at %" G_GUINT64_FORMAT " (text: %s, type: %s)\n", pts, text, type);
			
			g_free (text);
		}
	}
	
	// Read the MARKERs
	asf_marker *asf_marker;
	const asf_marker_entry* marker_entry;
	
	asf_marker = parser->marker;
	if (asf_marker != NULL) {
		for (i = 0; i < (int) asf_marker->marker_count; i++) {
			marker_entry = asf_marker->get_entry (i);
			text = marker_entry->get_marker_description ();
			
			pts = marker_entry->pts - preroll_pts;

			marker = new MediaMarker ("Name", text, pts);
			markers->Append (new MediaMarker::Node (marker));
			marker->unref ();
			
			//printf ("MediaElement::ReadMarkers () Added marker at %" G_GUINT64_FORMAT " (text: %s, type: %s)\n", pts, text, "Name");
		
			g_free (text);
		}
	}
	
		
cleanup:
	
	g_strfreev (command_types);
	g_free (commands);
}

void
ASFDemuxer::SetParser (ASFParser *parser)
{
	if (this->parser)
		this->parser->unref ();
	this->parser = parser;
	if (this->parser) {
		this->parser->ref ();
		this->parser->SetSource (source);
	}
}

void
ASFDemuxer::OpenDemuxerAsyncInternal ()
{
	MediaResult result;
	
	LOG_PIPELINE ("ASFDemuxer::OpenDemuxerAsyncInternal ()\n");
	
	g_return_if_fail (media != NULL);
	
	result = Open ();
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportOpenDemuxerCompleted ();
	} else if (result == MEDIA_NOT_ENOUGH_DATA) {
		EnqueueOpen ();
	} else {
		ReportErrorOccurred (result);
	}
}

MediaResult
ASFDemuxer::Open ()
{
	MediaResult result = MEDIA_SUCCESS;
	ASFParser *asf_parser = NULL;
	gint32 *stream_to_asf_index = NULL;
	IMediaStream **streams = NULL;
	int current_stream = 1;
	int stream_count = 0;
	int count;
	
	if (parser != NULL) {
		asf_parser = parser;
	} else {
		asf_parser = new ASFParser (source, media);
	}

	LOG_PIPELINE_ASF ("ASFDemuxer::ReadHeader ().\n");

	result = asf_parser->ReadHeader ();
	if (!MEDIA_SUCCEEDED (result)) {
		if (result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_PIPELINE_ASF ("ASFDemuxer::ReadHeader (): ReadHeader failed due to not enough data being available.\n");
		} else {
			Media::Warning (MEDIA_INVALID_MEDIA, "asf_parser->ReadHeader () failed:");
			Media::Warning (MEDIA_FAIL, "%s", asf_parser->GetLastErrorStr ());
		}
		goto failure;
	}
	
	// Count the number of streams
	for (int i = 1; i <= 127; i++) {
		if (asf_parser->IsValidStream (i))
			stream_count++;
	}
	
	current_stream = 1;
	streams = (IMediaStream **) g_malloc0 (sizeof (IMediaStream *) * (stream_count + 1)); // End with a NULL element.
	stream_to_asf_index = (gint32 *) g_malloc0 (sizeof (gint32) * (stream_count + 1)); 

	// keep count as a separate local since we can change its value (e.g. bad stream)
	count = stream_count;
	// Loop through all the streams and set stream-specific data	
	for (int i = 0; i < count; i++) {
		while (current_stream <= 127 && !asf_parser->IsValidStream (current_stream))
			current_stream++;
		
		if (current_stream > 127) {
			result = MEDIA_INVALID_STREAM;
			Media::Warning (result, "Couldn't find all the claimed streams in the file.");
			goto failure;
		}
		
		const asf_stream_properties* stream_properties = asf_parser->GetStream (current_stream);
		IMediaStream* stream = NULL;
		
		if (stream_properties == NULL) {
			result = MEDIA_INVALID_STREAM;
			Media::Warning (result, "Couldn't find all the claimed streams in the file.");
			goto failure;
		}
		
		if (stream_properties->is_audio ()) {
			AudioStream* audio = new AudioStream (GetMedia ());

			stream = audio;
			
			const WAVEFORMATEX* wave = stream_properties->get_audio_data ();
			if (wave == NULL) {
				result = MEDIA_INVALID_STREAM;
				Media::Warning (result, "Couldn't find audio data in the file.");
				goto failure;
			}

			const WAVEFORMATEXTENSIBLE* wave_ex = wave->get_wave_format_extensible ();
			int data_size = stream_properties->size - sizeof (asf_stream_properties) - sizeof (WAVEFORMATEX);
			
			audio->channels = wave->channels;
			audio->sample_rate = wave->samples_per_second;
			audio->bit_rate = wave->bytes_per_second * 8;
			audio->block_align = wave->block_alignment;
			audio->bits_per_sample = wave->bits_per_sample;
			audio->extra_data = NULL;
			audio->extra_data_size = data_size > wave->codec_specific_data_size ? wave->codec_specific_data_size : data_size;
			audio->codec_id = wave->codec_id;
			
			if (wave_ex != NULL) {
				audio->bits_per_sample = wave_ex->Samples.valid_bits_per_sample;
				audio->extra_data_size -= (sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX));
				audio->codec_id = *((guint32*) &wave_ex->sub_format);
			}

			// Fill in any extra codec data
			if (audio->extra_data_size > 0) {
				audio->extra_data = g_malloc0 (audio->extra_data_size);
				char* src = ((char*) wave) + (wave_ex ? sizeof (WAVEFORMATEX) : sizeof (WAVEFORMATEX));
				memcpy (audio->extra_data, src, audio->extra_data_size);
			}
		} else if (stream_properties->is_video ()) {
			VideoStream* video = new VideoStream (GetMedia ());
			stream = video;
			
			const asf_video_stream_data* video_data = stream_properties->get_video_data ();
			const BITMAPINFOHEADER* bmp;
			const asf_extended_stream_properties* aesp;

			if (video_data != NULL) {
				bmp = video_data->get_bitmap_info_header ();
				aesp = asf_parser->GetExtendedStream (current_stream);
				if (bmp != NULL) {
					video->width = bmp->image_width;
					video->height = bmp->image_height;

					// note: both height and width are unsigned
					if ((video->height > MAX_VIDEO_HEIGHT) || (video->width > MAX_VIDEO_WIDTH)) {
						result = MEDIA_INVALID_STREAM;
						Media::Warning (result, 
							"Video stream size (width: %d, height: %d) outside limits (%d, %d)", 
							video->height, video->width, MAX_VIDEO_HEIGHT, MAX_VIDEO_WIDTH);
						goto failure;
					}

					video->bits_per_sample = bmp->bits_per_pixel;
					video->codec_id = bmp->compression_id;
					video->extra_data_size = bmp->get_extra_data_size ();
					if (video->extra_data_size > 0) {
						video->extra_data = g_malloc0 (video->extra_data_size);
						memcpy (video->extra_data, bmp->get_extra_data (), video->extra_data_size);
					} else {
						video->extra_data = NULL;
					}
				}
				if (aesp != NULL) {
					video->bit_rate = aesp->data_bitrate;
					video->pts_per_frame = aesp->average_time_per_frame;
				} else {
					video->bit_rate = video->width*video->height;
					video->pts_per_frame = 0;
				} 
			}
		} else if (stream_properties->is_command ()) {
			MarkerStream* marker = new MarkerStream (GetMedia ());
			stream = marker;
			stream->codec = g_strdup ("asf-marker");
		} else {
			// Unknown stream, don't include it in the count since it's NULL
			stream_count--;
			// also adjust indexes so we don't create a hole in the streams array
			count--;
			i--;
		}
		
		if (stream != NULL) {
			if (stream_properties->is_video () || stream_properties->is_audio ()) {
				switch (stream->codec_id) {
				case CODEC_WMV1: stream->codec = g_strdup ("wmv1"); break;
				case CODEC_WMV2: stream->codec = g_strdup ("wmv2"); break;
				case CODEC_WMV3: stream->codec = g_strdup ("wmv3"); break;
				case CODEC_WMVA: stream->codec = g_strdup ("wmva"); break;
				case CODEC_WVC1: stream->codec = g_strdup ("vc1"); break;
				case CODEC_MP3: stream->codec = g_strdup ("mp3"); break;
				case CODEC_WMAV1: stream->codec = g_strdup ("wmav1"); break;
				case CODEC_WMAV2: stream->codec = g_strdup ("wmav2"); break;
				case CODEC_WMAV3: stream->codec = g_strdup ("wmav3"); break;
				default:
					char a = ((stream->codec_id & 0x000000FF));
					char b = ((stream->codec_id & 0x0000FF00) >> 8);
					char c = ((stream->codec_id & 0x00FF0000) >> 16);
					char d = ((stream->codec_id & 0xFF000000) >> 24);
					stream->codec = g_strdup_printf ("unknown (%c%c%c%c)", a ? a : ' ', b ? b : ' ', c ? c : ' ', d ? d : ' ');
					break;
				}
			}
			streams [i] = stream;
			stream->index = i;			
			if (!asf_parser->file_properties->is_broadcast ()) {
				stream->duration = asf_parser->file_properties->play_duration - MilliSeconds_ToPts (asf_parser->file_properties->preroll);
			}
			stream_to_asf_index [i] = current_stream;
		}
		
		current_stream++;
	}
	
	
	if (!MEDIA_SUCCEEDED (result)) {
		goto failure;
	}
	
	SetStreams (streams, stream_count);
	this->stream_to_asf_index = stream_to_asf_index;
	this->parser = asf_parser;
	
	reader = new ASFReader (parser, this);
			
	ReadMarkers ();
	
	return result;
	
failure:
	asf_parser->unref ();
	asf_parser = NULL;
	
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;
	
	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++) {
			if (streams [i] != NULL) {
				streams [i]->unref ();
				streams [i] = NULL;
			}
		}
		g_free (streams);
		streams = NULL;
	}
	
	return result;
}

IMediaStream *
ASFDemuxer::GetStreamOfASFIndex (gint32 asf_index)
{
	for (gint32 i = 0; i < GetStreamCount (); i++) {
		if (stream_to_asf_index [i] == asf_index)
			return GetStream (i);
	}
	return NULL;
}

void
ASFDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	ASFFrameReader *reader = this->reader->GetFrameReader (stream_to_asf_index [stream->index]);
	MediaFrame *frame;
	MediaResult result;
	
	g_return_if_fail (reader != NULL);
	
	result = reader->Advance ();
	
	if (result == MEDIA_NO_MORE_DATA) {
		ReportGetFrameCompleted (NULL);
		return;
	}

	if (result == MEDIA_BUFFER_UNDERFLOW) {
		EnqueueGetFrame (stream);
		return;
	}
	
	if (result == MEDIA_NOT_ENOUGH_DATA) {
		EnqueueGetFrame (stream);
		return;
	}
	
	if (!MEDIA_SUCCEEDED (result)) {
		ReportErrorOccurred ("Error while advancing to the next frame (%d)");
		return;
	}

	frame = new MediaFrame (stream);	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	if (reader->IsKeyFrame ())
		frame->AddState (FRAME_KEYFRAME);
	frame->buflen = reader->Size ();
	frame->buffer = (guint8 *) g_try_malloc (frame->buflen + frame->stream->min_padding);
	
	if (frame->buffer == NULL) {
		ReportErrorOccurred ( "Could not allocate memory for next frame.");
		return;
	}
	
	//printf ("ASFDemuxer::ReadFrame (%p), min_padding = %i\n", frame, frame->stream->min_padding);
	if (frame->stream->min_padding > 0)
		memset (frame->buffer + frame->buflen, 0, frame->stream->min_padding); 
	
	if (!reader->Write (frame->buffer)) {
		ReportErrorOccurred ("Error while copying the next frame.");
		return;
	}
	
	frame->AddState (FRAME_DEMUXED);
	
	ReportGetFrameCompleted (frame);
	
	frame->unref ();
}

/*
 * ASFMarkerDecoder
 */

ASFMarkerDecoder::ASFMarkerDecoder (Media *media, IMediaStream *stream)
	: IMediaDecoder (Type::ASFMARKERDECODER, media, stream)
{
}

void
ASFMarkerDecoder::OpenDecoderAsyncInternal ()
{
	ReportOpenDecoderCompleted ();
}

void
ASFMarkerDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	LOG_PIPELINE_ASF ("ASFMarkerDecoder::DecodeFrame ()\n");
	
	MediaResult result;
	char *text;
	char *type;
	gunichar2 *data;
	gunichar2 *uni_type = NULL;
	gunichar2 *uni_text = NULL;
	int text_length = 0;
	int type_length = 0;
	guint32 size = 0;
	
	if (frame->buflen % 2 != 0 || frame->buflen == 0 || frame->buffer == NULL) {
		ReportErrorOccurred (MEDIA_CORRUPTED_MEDIA);
		return;
	}

	data = (gunichar2 *) frame->buffer;
	uni_type = data;
	size = frame->buflen;
	
	// the data is two arrays of WCHARs (type and text), null terminated.
	// loop through the data, counting characters and null characters
	// there should be at least two null characters.
	int null_count = 0;
	
	for (guint32 i = 0; i < (size / sizeof (gunichar2)); i++) {
		if (uni_text == NULL) {
			type_length++;
		} else {
			text_length++;
		}
		if (*(data + i) == 0) {
			null_count++;
			if (uni_text == NULL) {
				uni_text = data + i + 1;
			} else {
				break; // Found at least two nulls
			}
		}
	}
	
	if (null_count >= 2) {
		text = wchar_to_utf8 (uni_text, text_length);
		type = wchar_to_utf8 (uni_type, type_length);
		
		LOG_PIPELINE_ASF ("ASFMarkerDecoder::DecodeFrame (): sending script command type: '%s', text: '%s', pts: '%" G_GUINT64_FORMAT "'.\n", type, text, frame->pts);

		frame->marker = new MediaMarker (type, text, frame->pts);
		
		g_free (text);
		g_free (type);
		result = MEDIA_SUCCESS;
	} else {
		LOG_PIPELINE_ASF ("ASFMarkerDecoder::DecodeFrame (): didn't find 2 null characters in the data.\n");
		result = MEDIA_CORRUPTED_MEDIA;
	}

	if (MEDIA_SUCCEEDED (result))  {
		ReportDecodeFrameCompleted (frame);
	} else {
		ReportErrorOccurred (result);
	}
}

/*
 * ASFMarkerDecoderInfo
 */

IMediaDecoder *
ASFMarkerDecoderInfo::Create (Media *media, IMediaStream *stream)
{
	return new ASFMarkerDecoder (media, stream);
}	

bool 
ASFMarkerDecoderInfo::Supports (const char *codec)
{
	return !strcmp (codec, "asf-marker");
}

const char *
ASFMarkerDecoderInfo::GetName ()
{
	return "ASFMarkerDecoder";
}

/*
 * ASFDemuxerInfo
 */

MediaResult
ASFDemuxerInfo::Supports (IMediaSource *source)
{
	guint8 buffer[16];
	bool result;

	LOG_PIPELINE_ASF ("ASFDemuxerInfo::Supports (%p) pos: %lld, avail pos: %lld\n", source, source->GetPosition (), source->GetLastAvailablePosition ());

#if DEBUG
	bool eof = false;
	if (!source->GetPosition () == 0)
		fprintf (stderr, "ASFDemuxerInfo::Supports (%p): Trying to check if a media is supported, but the media isn't at position 0 (it's at position %lld)\n", source, source->GetPosition ());
	if (!source->IsPositionAvailable (16, &eof)) // This shouldn't happen, we should have at least 1024 bytes (or eof).
		fprintf (stderr, "ASFDemuxerInfo::Supports (%p): Not enough data! eof: %i\n", source, eof);
#endif

	if (!source->Peek (buffer, 16)) {
		fprintf (stderr, "ASFDemuxerInfo::Supports (%p): Peek failed.\n", source);
		return MEDIA_FAIL;
	}
	
	result = asf_guid_compare (&asf_guids_header, (asf_guid *) buffer);
	
	//printf ("ASFDemuxerInfo::Supports (%p): probing result: %s %s\n", source, source->ToString (),
	//	result ? "true" : "false");
	
	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

IMediaDemuxer *
ASFDemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new ASFDemuxer (media, source);
}


/*
 * MemoryQueueSource::QueueNode
 */

MemoryQueueSource::QueueNode::QueueNode (MemorySource *source)
{
	if (source)
		source->ref ();
	this->source = source;
	packet = NULL;
}

MemoryQueueSource::QueueNode::QueueNode (ASFPacket *packet)
{
	if (packet)
		packet->ref ();
	this->packet = packet;
	source = NULL;
}

MemoryQueueSource::QueueNode::~QueueNode ()
{
	if (packet)
		packet->unref ();
	if (source)
		source->unref ();
}

/*
 * MemoryQueueSource
 */
 
MemoryQueueSource::MemoryQueueSource (Media *media, Downloader *downloader)
	: IMediaSource (Type::MEMORYQUEUESOURCE, media)
{
	finished = false;
	write_count = 0;
	parser = NULL;
	queue = new Queue ();
	this->downloader = NULL;
	
	g_return_if_fail (downloader != NULL);
	g_return_if_fail (downloader->GetInternalDownloader () != NULL);
	g_return_if_fail (downloader->GetInternalDownloader ()->GetType () == InternalDownloader::MmsDownloader);
	
	this->downloader = downloader;
	this->downloader->ref ();
	this->mms_downloader = (MmsDownloader *) downloader->GetInternalDownloader ();
}

void
MemoryQueueSource::Dispose ()
{
	if (parser) {
		parser->unref ();
		parser = NULL;
	}
	if (queue) {
		delete queue;
		queue = NULL;
	}
	if (downloader) {
		downloader->RemoveAllHandlers (this);
		downloader->unref ();
		downloader = NULL;
		mms_downloader = NULL;
	}
	
	
	IMediaSource::Dispose ();
}

ASFParser *
MemoryQueueSource::GetParser ()
{
	return parser;
}

MediaResult
MemoryQueueSource::Initialize ()
{
	g_return_val_if_fail (mms_downloader != NULL, MEDIA_FAIL);
	g_return_val_if_fail (downloader != NULL, MEDIA_FAIL);
	g_return_val_if_fail (!downloader->Started (), MEDIA_FAIL);
	
	mms_downloader->SetSource (this);
	
	downloader->AddHandler (Downloader::DownloadFailedEvent, DownloadFailedCallback, this);
	downloader->AddHandler (Downloader::CompletedEvent, DownloadCompleteCallback, this);
	downloader->Send ();
	
	return MEDIA_SUCCESS;
}

void
MemoryQueueSource::DownloadFailedHandler (Downloader *dl, EventArgs *args)
{
	g_return_if_fail (media != NULL);
	media->RetryHttp (new ErrorEventArgs (MediaError, 4001, "AG_E_NETWORK_ERROR"));
}

void
MemoryQueueSource::DownloadCompleteHandler (Downloader *dl, EventArgs *args)
{
}

void
MemoryQueueSource::SetParser (ASFParser *parser)
{
	if (this->parser)
		this->parser->unref ();
	this->parser = parser;
	if (this->parser)
		this->parser->ref ();
}

gint64
MemoryQueueSource::GetPositionInternal ()
{
	g_warning ("MemoryQueueSource::GetPositionInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.");
	print_stack_trace ();

	return -1;
}

Queue*
MemoryQueueSource::GetQueue ()
{
	QueueNode *node;
	QueueNode *next;

	if (!queue)
		return NULL;

	// Make sure all nodes have asf packets.
	queue->Lock ();
	node = (QueueNode *) queue->LinkedList ()->First ();
	while (node != NULL && node->packet == NULL) {
		next = (QueueNode *) node->next;
		
		node->packet = new ASFPacket (parser, node->source);
		if (!MEDIA_SUCCEEDED (node->packet->Read ())) {
			LOG_PIPELINE_ASF ("MemoryQueueSource::GetQueue (): Error while parsing packet, dropping packet.\n");
			queue->LinkedList ()->Remove (node);
		}
		
		node = next;
	}
	queue->Unlock ();
	
	return queue;
}

ASFPacket *
MemoryQueueSource::Pop ()
{
	//printf ("MemoryQueueSource::Pop (), there are %i packets in the queue, of a total of %lld packets written.\n", queue.Length (), write_count);
	
	QueueNode *node;
	ASFPacket *result = NULL;

	if (!queue)
		return NULL;

trynext:
	node = (QueueNode *) queue->Pop ();
	
	if (node == NULL) {
		LOG_PIPELINE_ASF ("MemoryQueueSource::Pop (): No more packets (for now).\n");
		return NULL;
	}
	
	if (node->packet == NULL) {
		if (parser == NULL) {
			g_warning ("MemoryQueueSource::Pop (): No parser to parse the packet.\n");
			goto cleanup;
		}
		node->packet = new ASFPacket (parser, node->source);
		if (!MEDIA_SUCCEEDED (node->packet->Read ())) {
			LOG_PIPELINE_ASF ("MemoryQueueSource::Pop (): Error while parsing packet, getting a new packet\n");
			delete node;
			goto trynext;
		}
	}
	
	result = node->packet;
	result->ref ();

cleanup:				
	delete node;
	
	LOG_PIPELINE_ASF ("MemoryQueueSource::Pop (): popped 1 packet, there are %i packets left, of a total of %lld packets written\n", queue->Length (), write_count);
	
	return result;
}

void
MemoryQueueSource::Write (void *buf, gint64 offset, gint32 n)
{
	// We should never get here
	// The MmsDownloader knowns about us and should call WritePacket.
	g_return_if_fail (false);
}

void
MemoryQueueSource::WritePacket (void *buf, gint32 n)
{
	MemorySource *src;
	ASFPacket *packet;
	
	LOG_PIPELINE_ASF ("MemoryQueueSource::WritePacket (%p, %i), write_count: %lld\n", buf, n, write_count + 1);

	if (!queue)
		return;

	write_count++;
	if (parser != NULL) {
		src = new MemorySource (media, buf, n, 0);
		src->SetOwner (false);
		packet = new ASFPacket (parser, src);
		if (!MEDIA_SUCCEEDED (packet->Read ())) {
			LOG_PIPELINE_ASF ("MemoryQueueSource::WritePacket (%p, %i): Error while parsing packet, dropping packet.\n", buf, n);
		} else {
			queue->Push (new QueueNode (packet));
		}
		packet->unref ();
		src->unref ();
	} else {
		src = new MemorySource (media, g_memdup (buf, n), n, 0);
		queue->Push (new QueueNode (src));
		src->unref ();
	}

	if (media) {
		media->ReportDownloadProgress (1.0);
		media->WakeUp ();
	}
}

bool
MemoryQueueSource::SeekInternal (gint64 offset, int mode)
{
	g_warning ("MemoryQueueSource::SeekInternal (%lld, %i): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", offset, mode);
	print_stack_trace ();

	return false;
}

gint32 
MemoryQueueSource::ReadInternal (void *buffer, guint32 n)
{
	g_warning ("MemoryQueueSource::ReadInternal (%p, %u): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", buffer, n);
	
	return 0;
}

gint32
MemoryQueueSource::PeekInternal (void *buffer, guint32 n)
{
	g_warning ("MemoryQueueSource::PeekInternal (%p, %u): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", buffer, n);
	
	return 0;
}

gint64
MemoryQueueSource::GetLastAvailablePositionInternal ()
{
	g_warning ("MemoryQueueSource::GetLastAvailablePositionInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.");
	
	return 0;
}

void
MemoryQueueSource::NotifySize (gint64 size)
{
	// We don't care.
}

void
MemoryQueueSource::NotifyFinished ()
{
	Lock ();
	this->finished = true;
	Unlock ();
}

gint64
MemoryQueueSource::GetSizeInternal ()
{
	g_warning ("MemoryQueueSource::GetSizeInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.");
	
	return 0;
}

MediaResult
MemoryQueueSource::SeekToPts (guint64 pts)
{
	MediaResult result = true;

	LOG_PIPELINE_ASF ("MemoryQueueSource::SeekToPts (%" G_GUINT64_FORMAT ")\n", pts);

	if (queue) {
		queue->Clear (true);
		mms_downloader->SetRequestedPts (pts);
		finished = false;
		result = MEDIA_SUCCESS;
	} else {
		result = MEDIA_FAIL;
	}
	
	return result;
}
