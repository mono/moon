/*
 * pipeline.cpp: Pipeline for the media
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"
#include "pipeline.h"
#include "pipeline-ffmpeg.h"
#include "uri.h"
#include "media.h"
#include "asf/asf.h"
#include "asf/asf-structures.h"

#define MAKE_CODEC_ID(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

#define CODEC_WMV1	MAKE_CODEC_ID ('W', 'M', 'V', '1')
#define CODEC_WMV2	MAKE_CODEC_ID ('W', 'M', 'V', '2')
#define CODEC_WMV3	MAKE_CODEC_ID ('W', 'M', 'V', '3')
#define CODEC_WMVA	MAKE_CODEC_ID ('W', 'M', 'V', 'A')
#define CODEC_WVC1	MAKE_CODEC_ID ('W', 'V', 'C', '1')
#define CODEC_MP3	0x55
#define CODEC_WMAV1 0x160
#define CODEC_WMAV2 0x161

/*
 * Media 
 */
DemuxerInfo* Media::registered_demuxers = NULL;
DecoderInfo* Media::registered_decoders = NULL;
ConverterInfo* Media::registered_converters = NULL;
 
Media::Media () :
		source (NULL), demuxer (NULL), markers (NULL),
		file_or_url (NULL), queued_requests (NULL),
		queue_closure (NULL)
{
}

Media::~Media ()
{
	DeleteQueue ();
		
	delete source;
	source = NULL;
	delete demuxer;
	demuxer = NULL;
	g_free (file_or_url);
	file_or_url = NULL;
	
	delete queue_closure;
	queue_closure = NULL;
	
	delete markers;
	markers = NULL;
}

List* 
Media::GetMarkers ()
{
	if (markers == NULL)
		markers = new List ();
	
	return markers;
}

void
Media::RegisterDemuxer (DemuxerInfo* info)
{
	printf ("Media::RegisterDecoder (%p)\n", info);
	info->next = NULL;
	if (registered_demuxers == NULL) {
		registered_demuxers = info;
	} else {
		MediaInfo* current = registered_demuxers;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
}

void
Media::RegisterConverter (ConverterInfo* info)
{
	printf ("Media::RegisterDecoder (%p)\n", info);
	info->next = NULL;
	if (registered_converters == NULL) {
		registered_converters = info;
	} else {
		MediaInfo* current = registered_converters;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
}

void
Media::RegisterDecoder (DecoderInfo* info)
{
	printf ("Media::RegisterDecoder (%p)\n", info);
	info->next = NULL;
	if (registered_decoders == NULL) {
		registered_decoders = info;
	} else {
		MediaInfo* current = registered_decoders;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
}

void
Media::Initialize ()
{
	// register stuff
	Media::RegisterDemuxer (new ASFDemuxerInfo ());
#ifdef INCLUDE_FFMPEG
	register_ffmpeg ();
#endif
}

void
Media::Shutdown ()
{
	MediaInfo* current;
	MediaInfo* next;
	
	current = registered_decoders;
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
	registered_decoders = NULL;
	
	current = registered_demuxers;
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
	registered_demuxers = NULL;
	
	
	current = registered_converters;
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
	registered_converters = NULL;
	
	
}

void
Media::AddMessage (MediaResult result, const char* msg)
{
	if (!MEDIA_SUCCEEDED (result)) {
		printf ("Media::AddMessage (%i, '%s').\n", result, msg);
	}
}

void
Media::AddMessage (MediaResult result, char* msg)
{
	AddMessage (result, (const char*) msg);
	g_free (msg);
}

MediaResult
Media::Seek (gint64 pts)
{
	return demuxer->Seek (pts);

	return MEDIA_SUCCESS;
}

MediaResult
Media::Open (const char* file_or_url)
{
	printf ("Media::Open ('%s').\n", file_or_url);
	
	Uri* uri = new Uri ();
	MediaResult result = MEDIA_FAIL;
	source = NULL;
	
	this->file_or_url = g_strdup (file_or_url);
	
	if (uri->Parse (file_or_url)) {
		printf ("Media::Open ('%s'): uri parsing succeeded, protocol: '%s'.\n", file_or_url, uri->protocol);
		if (uri->protocol == NULL) {
			result = MEDIA_INVALID_PROTOCOL;
		} else if (strcmp (uri->protocol, "mms") == 0) {
			source = new LiveSource (this);
			result = Open (source);
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("Media::Open ('%s'): live source failed, trying progressive source.\n", file_or_url);
				delete source;
				source = new ProgressiveSource (this);
				result = Open (source);
			}
		} else if (strcmp (uri->protocol, "http") == 0 || strcmp (uri->protocol, "https") == 0) {
			source = new ProgressiveSource (this);
			result = Open (source);
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("Media::Open ('%s'): progressive source failed, trying live source.\n", file_or_url);
				delete source;
				source = new LiveSource (this);
				result = Open (source);
			}
		} else if (strcmp (uri->protocol, "file") == 0) {
			source = new FileSource (this),
			result = Open (source);
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("Media::Open ('%s'): file source failed.\n", file_or_url);
			}
		} else {
			result = MEDIA_INVALID_PROTOCOL;
		}
	} else {
		// FIXME: Is it safe to assume that if the path cannot be parsed as an uri it is a filename?
		printf ("Media::Open ('%s'): uri parsing failed, assuming source is a filename.\n", file_or_url);
		source = new FileSource (this);	
		result = Open (source);
	}
	
	delete uri;
	
	if (!MEDIA_SUCCEEDED (result)) {
		printf ("Media::Open ('%s'): failed, result: %i.\n", file_or_url, result);
		delete source;
		source = NULL;
	} else {
		printf ("Media::Open ('%s'): succeeded.\n", file_or_url);
	}
	
	return result;
}

MediaResult
Media::Open (IMediaSource* source)
{
	printf ("Media::Open ().\n");
	
	MediaResult result = MEDIA_FAIL; // We return success if at least 1 stream could be opened correctly (has decoder and, if applicable, converter)

	if (source == NULL) {
		return MEDIA_INVALID_ARGUMENT;
	}
	
	// Select a demuxer
	uint8_t buffer [16];
	if (!source->Peek (&buffer [0], 16)) {
		return MEDIA_FAIL;
	}
	
	DemuxerInfo* current_demuxer = registered_demuxers;
	while (current_demuxer != NULL && !current_demuxer->Supports (&buffer [0], 16)) {
		printf ("Checking registered demuxer '%s' if it supports the media file '%s': no.\n", current_demuxer->GetName (), file_or_url);
		current_demuxer = (DemuxerInfo*) current_demuxer->next;
	}
		
	if (current_demuxer == NULL) {
		AddMessage (MEDIA_UNKNOWN_MEDIA_TYPE, g_strdup_printf ("No demuxers registered to handle the media file '%s'.", file_or_url));
		return MEDIA_UNKNOWN_MEDIA_TYPE;
	} else {
		printf ("Checking registered demuxer '%s' if it supports the media file '%s': yes.\n", current_demuxer->GetName (), file_or_url);
		demuxer = current_demuxer->Create (this);
	}
	
	// Found a demuxer
	result = demuxer->ReadHeader ();
	
	if (!MEDIA_SUCCEEDED (result)) {
		return result;
	}
	
	printf ("Media::Open (): Found %i streams in this source.\n", demuxer->GetStreamCount ());
	
	printf ("Media::Open (): Starting to select codecs...\n");
	
	result = MEDIA_FAIL; // Only set to SUCCESS if at least 1 stream can be used
	
	// Select codecs for each stream
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream* stream = demuxer->GetStream (i);
		if (stream == NULL)
			return MEDIA_INVALID_STREAM;
		
		const char* codec = stream->GetCodec ();
		IMediaDecoder* decoder = NULL;
		
		printf ("Media::Open (): Selecting codec for codec %s, id %i.\n", codec, stream->codec_id);
		
		DecoderInfo* current_decoder = registered_decoders;
		while (current_decoder != NULL && !current_decoder->Supports (codec)) {
			printf ("Checking registered decoder '%s' if it supports codec '%s': no.\n", current_decoder->GetName (), codec);
			current_decoder = (DecoderInfo*) current_decoder->next;
		}

		if (current_decoder == NULL) {
			AddMessage (MEDIA_UNKNOWN_CODEC, g_strdup_printf ("Unknown codec: '%s'.", codec));	
		} else {
			printf ("Checking registered decoder '%s' if it supports codec '%s': yes.\n", current_decoder->GetName (), codec);
			decoder = current_decoder->Create (this, stream);
		}

		if (decoder != NULL) {
			result = decoder->Open ();
			if (!MEDIA_SUCCEEDED (result)) {
				delete decoder;
				decoder = NULL;
			}
		}
		
		if (decoder != NULL) {
			// Select converter for this stream
			if (stream->GetType () == MediaTypeVideo && decoder->pixel_format != MoonPixelFormatRGB32) {
				VideoStream* vs = (VideoStream*) stream;
				IImageConverter* converter;
				
				ConverterInfo* current_conv = registered_converters;
				while (current_conv != NULL && !current_conv->Supports (decoder->pixel_format, MoonPixelFormatRGB32)) {
					printf ("Checking registered converter '%s' if it supports input '%i' and output '%i': no.\n", current_conv->GetName (), decoder->pixel_format, MoonPixelFormatRGB32);
					current_conv = (ConverterInfo*) current_conv->next;
				}

				if (current_conv == NULL) {
					AddMessage (MEDIA_UNKNOWN_CONVERTER, g_strdup_printf ("Can't convert from %i to %i: No converter found.", vs->decoder->pixel_format, MoonPixelFormatRGB32));	
				} else {
					printf ("Checking registered converter '%s' if it supports input '%i' and output '%i': yes.\n", current_conv->GetName (), decoder->pixel_format, MoonPixelFormatRGB32);
					converter = current_conv->Create (this, vs);
					converter->input_format = decoder->pixel_format;
					converter->output_format = MoonPixelFormatRGB32;
					if (!MEDIA_SUCCEEDED (converter->Open ())) {
						delete converter;
						converter = NULL;
					}
				}

				if (converter != NULL) {				
					vs->converter = converter;
				} else {
					delete decoder;
					decoder = NULL;
				}
			}
		}
		
		if (decoder != NULL) {
			stream->SetDecoder (decoder);
			result = MEDIA_SUCCESS;
		}
	}
	
	return result;
}

MediaResult
Media::GetNextFrame (MediaFrame* frame, int states)
{
	//printf ("Media::GetNextFrame (%p).\n", stream);
	
	if (frame == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "frame is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	MediaResult result = MEDIA_SUCCESS;
	
	if ((states & FRAME_DEMUXED) != FRAME_DEMUXED)
		return result; // Nothing to do?
		
	result = demuxer->ReadFrame (frame);
	if (!MEDIA_SUCCEEDED (result)) {
		return result;
	}
	
	if ((states & FRAME_DECODED) != FRAME_DECODED)
		return result;
	
	result = frame->stream->decoder->DecodeFrame (frame);
	if (!MEDIA_SUCCEEDED (result)) {
		return result;
	}
	
	//printf ("Media::GetNextFrame (%p) finished, size: %i.\n", stream, frame->uncompressed_size);
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::GetNextFrame (MediaFrame* frame)
{
	return GetNextFrame (frame, FRAME_DEMUXED | FRAME_DECODED);
}

void* 
Media::FrameReaderLoop (void* data)
{
	Media* media = (Media*) data;
	media->FrameReaderLoop ();
	return NULL;
}

#define LOG_FRAMEREADERLOOP(x, ...)// printf (x, __VA_ARGS__);

void
Media::FrameReaderLoop ()
{
	LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop ().\n");
	while (queued_requests != NULL) {
		Media::Node* node = NULL, *current = NULL;
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): entering mutex.\n");
		// Wait until we have something in the queue
		pthread_mutex_lock (&queue_mutex);
		while (queued_requests != NULL && queued_requests->IsEmpty ())
			pthread_cond_wait (&queue_condition, &queue_mutex);
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): got something.\n");
		if (queued_requests != NULL) {
			// Find the first audio node
			current = (Media::Node*) queued_requests->First ();
			while (current != NULL && current->frame->stream->GetType () != MediaTypeAudio) {
				current = (Media::Node*) current->next;
			}
			if (current != NULL && current->frame->stream->GetType () == MediaTypeAudio) {
				node = current;
			} else {
				// No audio node, just get the first node
				node = (Media::Node*) queued_requests->First ();
			}
			queued_requests->Unlink (node);
			LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): got a %s node, there are %i nodes left.\n", node->stream->GetType () == MediaTypeAudio ? "audio" : (node->stream->GetType () == MediaTypeVideo ? "video" : "unknown") , queued_requests->Length ());
		}
		pthread_mutex_unlock (&queue_mutex);
		
		if (node == NULL)
			continue; // Found nothing, continue waiting.
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): processing node.\n");
		
		// Now demux and decode what we found and send it to who asked for it
		MediaResult result = GetNextFrame (node->frame, node->states);
		if (MEDIA_SUCCEEDED (result)) {
			MediaClosure *closure = new MediaClosure ();
			memcpy (closure, queue_closure, sizeof (MediaClosure));
			closure->frame = node->frame;
			closure->Call ();
			delete closure;
		}
		delete node;
	}
	LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): exiting.\n");
}
#include <sched.h>

void
Media::GetNextFrameAsync (MediaFrame* frame)
{
	Media::GetNextFrameAsync (frame, FRAME_DEMUXED | FRAME_DECODED);
}

void
Media::GetNextFrameAsync (MediaFrame* frame, int states)
{
	//printf ("Media::GetNextFrameAsync (%p).\n", stream);
	if (queued_requests == NULL) {
		//printf ("Media::GetNextFrameAsync (%p). Creating threads.\n", stream);
		queued_requests = new List ();
		int result;
		pthread_attr_t attribs;
		result = pthread_attr_init (&attribs);
		result = pthread_attr_setdetachstate (&attribs, PTHREAD_CREATE_JOINABLE);
		result = pthread_mutex_init (&queue_mutex, NULL);
		result = pthread_cond_init (&queue_condition, NULL);
		result = pthread_create (&queue_thread, &attribs, FrameReaderLoop, this); 	
		result = pthread_attr_destroy (&attribs);		
	}
	
	pthread_mutex_lock (&queue_mutex);
	
	// Add another node to our queue.
	Media::Node* node = new Media::Node ();
	node->frame = frame;
	node->states = states;
	queued_requests->Append (node);
	
	pthread_cond_signal (&queue_condition);
	
	pthread_mutex_unlock (&queue_mutex);
}

void
Media::ClearQueue ()
{
	printf ("Media::ClearQueue ().\n");
	if (queued_requests != NULL) {
		pthread_mutex_lock (&queue_mutex);
		queued_requests->Clear (true);
		pthread_mutex_unlock (&queue_mutex);
	}
}

void
Media::DeleteQueue ()
{
	printf ("Media::DeleteQueue ().\n");
	if (queued_requests != NULL) {
		pthread_mutex_lock (&queue_mutex);
		queued_requests->Clear (true);
		delete queued_requests;
		queued_requests = NULL;
		pthread_cond_signal (&queue_condition);
		pthread_mutex_unlock (&queue_mutex);
		
		pthread_join (queue_thread, NULL);
		pthread_mutex_destroy (&queue_mutex);
		pthread_cond_destroy (&queue_condition);
		pthread_detach (queue_thread);
	}
}


/*
 * ASFDemuxer
 */

ASFDemuxer::ASFDemuxer (Media* media) : IMediaDemuxer (media),
	parser (NULL), reader (NULL)
{
}

ASFDemuxer::~ASFDemuxer ()
{
	delete reader;
	reader = NULL;
	
	delete parser;
	parser = NULL;
	
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;
}

MediaResult
ASFDemuxer::Seek (guint64 pts)
{
	if (reader == NULL) {
		reader = new ASFFrameReader (parser);
	}
	
	if (reader->Seek (0, pts))
		return MEDIA_SUCCESS;
	return MEDIA_FAIL;
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
	
	// Read the markers (if any)
	int i = -1;
	guint64 preroll = parser->file_properties->preroll;
	List* markers = media->GetMarkers ();
	
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
	
	i = -1;
	while (commands != NULL && commands [++i] != NULL) {
		asf_script_command_entry *entry = commands [i];
		int64_t pts = entry->pts; //(entry->pts - preroll) * 10000;
		char* text = entry->get_name ();
		const char* type = "";
		
		if (entry->type_index + 1 <= command->command_type_count) {
			type = command_types [entry->type_index];
		}
		
		markers->Append (new MediaMarker::Node (new MediaMarker (type, text, pts)));
		
		//printf ("MediaElement::ReadMarkers () Added script command at %llu (text: %s, type: %s)\n", pts, text, type);
		
		g_free (text);
	}
	
	
	// Read the MARKERs
	asf_marker *asf_marker;
	const asf_marker_entry* marker_entry;
	
	asf_marker = parser->marker;
	if (asf_marker != NULL) {
		for (i = 0; i < (int) asf_marker->marker_count; i++) {
			marker_entry = asf_marker->get_entry (i);
			int64_t pts = marker_entry->pts / 10000; // (marker_entry->pts - preroll * 10000);
			char* text = marker_entry->get_marker_description ();
			
			markers->Append (new MediaMarker::Node (new MediaMarker ("Name", text, pts)));
			
			//printf ("MediaElement::ReadMarkers () Added marker at %llu (text: %s, type: %s)\n", pts, text, "Name");
		
			g_free (text);
		}
	}
	
		
cleanup:
	if (command_types) {
		i = -1;
		while (command_types [++i] != NULL)
			g_free (command_types [i]);
		g_free (command_types);
	}
	
	g_free (commands);
}

MediaResult
ASFDemuxer::ReadHeader ()
{
	printf ("ASFDemuxer::ReadHeader ().\n");
	
	MediaResult result = MEDIA_SUCCESS;
	ASFSource* asf_source = new ASFMediaSource (NULL, GetMedia ()->GetSource ());
	ASFParser* asf_parser = new ASFParser (asf_source);
	gint32* stream_to_asf_index = NULL;
	IMediaStream** streams = NULL;
	int current_stream = 1;
	int stream_count = 0;
	
	asf_source->parser = asf_parser;
	
	if (!asf_parser->ReadHeader ()) {
		result = MEDIA_INVALID_MEDIA;
		GetMedia ()->AddMessage (MEDIA_INVALID_MEDIA, "asf_parser->ReadHeader () failed:");
		GetMedia ()->AddMessage (MEDIA_FAIL, asf_parser->GetLastError ());
		goto failure;
	}
	
	media->SetStartTime (asf_parser->file_properties->preroll);
			
	// Count the number of streams
	stream_count = 0;
	for (int i = 1; i <= 127; i++) {
		if (asf_parser->IsValidStream (i))
			stream_count++;
	}
	
	current_stream = 1;
	streams = (IMediaStream**) g_malloc0 (sizeof (IMediaStream*) * (stream_count + 1)); // End with a NULL element.
	stream_to_asf_index = (gint32*) g_malloc0 (sizeof (gint32) * (stream_count + 1)); 

	// Loop through all the streams and set stream-specific data	
	for (int i = 0; i < stream_count; i++) {
		while (current_stream <= 127 && !asf_parser->IsValidStream (current_stream))
			current_stream++;
		
		if (current_stream > 127) {
			result = MEDIA_INVALID_STREAM;
			GetMedia ()->AddMessage (result, "Couldn't find all the claimed streams in the file.");
			goto failure;
		}
		
		asf_stream_properties* stream_properties = asf_parser->GetStream (current_stream);
		IMediaStream* stream = NULL;
		
		if (stream_properties == NULL) {
			result = MEDIA_INVALID_STREAM;
			GetMedia ()->AddMessage (result, "Couldn't find all the claimed streams in the file.");
			goto failure;
		}
		
		if (stream_properties->is_audio ()) {
			AudioStream* audio = new AudioStream (GetMedia ());

			stream = audio;
			
			const WAVEFORMATEX* wave = stream_properties->get_audio_data ();
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

			if (video_data != NULL) {
				bmp = video_data->get_bitmap_info_header ();
				if (bmp != NULL) {
					video->width = bmp->image_width;
					video->height = bmp->image_height;
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
			}
		} else if (stream_properties->is_command ()) {
			MarkerStream* marker = new MarkerStream (GetMedia ());
			stream = marker;
			stream->codec = "asf-marker";
		} else {
			// Unknown stream, ignore it.
		}
		
		if (stream != NULL) {
			if (stream_properties->is_video () || stream_properties->is_audio ()) {
				switch (stream->codec_id) {
				case CODEC_WMV1: stream->codec = "wmv1"; break;
				case CODEC_WMV2: stream->codec = "wmv2"; break;
				case CODEC_WMV3: stream->codec = "wmv3"; break;
				case CODEC_WMVA: stream->codec = "wmva"; break;
				case CODEC_WVC1: stream->codec = "vc1"; break;
				case CODEC_MP3: stream->codec = "mp3"; break;
				case CODEC_WMAV1: stream->codec = "wmav1"; break;
				case CODEC_WMAV2: stream->codec = "wmav2"; break;
				default: stream->codec = "unknown"; break;
				}
			}
			stream->start_time = asf_parser->file_properties->preroll;
			streams [i] = stream;
			stream->index = i;			
			if (!asf_parser->file_properties->is_broadcast ()) {
				stream->duration = asf_parser->file_properties->play_duration / 10000 - stream->start_time;
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
	
	ReadMarkers ();
	
	return result;
	
failure:
	delete asf_parser;
	asf_parser = NULL;
	
	g_free (stream_to_asf_index);
	stream_to_asf_index = NULL;
	
	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++) {
			if (streams [i] != NULL) {
				delete streams [i];
				streams [i] = NULL;
			}
		}
		g_free (streams);
		streams = NULL;
	}
	
	return result;
}

MediaResult
ASFDemuxer::ReadFrame (MediaFrame* frame)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	
	if (reader == NULL) {
		reader = new ASFFrameReader (parser);
	}
	
	//printf ("ASFDemuxer::ReadFrame (%p) frame = ", frame);
	//frame->printf ();
	//printf ("\n");
	
	if (!reader->Advance (stream_to_asf_index [frame->stream->index])) {
		if (!reader->Eof ()) {
			media->AddMessage (MEDIA_DEMUXER_ERROR, "Error while advancing to the next frame.");
			return MEDIA_DEMUXER_ERROR;
		} else {
			media->AddMessage (MEDIA_NO_MORE_DATA, "Reached end of data.");
			return MEDIA_NO_MORE_DATA;
		}
	}
	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	frame->compressed_size = reader->Size ();
	frame->compressed_data = g_malloc0 (reader->Size () + frame->stream->min_padding);
	if (frame->compressed_data == NULL) {
		media->AddMessage (MEDIA_OUT_OF_MEMORY, "Could not allocate memory for next frame.");
		return MEDIA_OUT_OF_MEMORY;
	}
	//printf ("ASFDemuxer::ReadFrame (%p), min_padding = %i\n", frame, frame->stream->min_padding);
	if (frame->stream->min_padding > 0) {
		memset ((char*) frame->compressed_data + reader->Size (), 0, frame->stream->min_padding); 
	}
	if (!reader->Write (frame->compressed_data)) {
		media->AddMessage (MEDIA_DEMUXER_ERROR, "Error while copying the next frame.");
		return MEDIA_DEMUXER_ERROR;
	}
	
	frame->AddState (FRAME_DEMUXED);
	
	//printf ("ASFDemuxer::ReadFrame (%p) frame = ", frame);
	//frame->printf ();
	//printf ("\n");
	
	return MEDIA_SUCCESS;
}

/*
 * ASFDemuxerInfo
 */

bool
ASFDemuxerInfo::Supports (uint8_t* buffer, uint32_t length)
{
	if (length < 16)
		return false;
		
	bool result = asf_guid_compare (&asf_guids_header, (asf_guid*) buffer);

	printf ("ASFDemuxerInfo::Supports (%p, %u): probing result: %s\n", buffer, length, result ? "true" : "false");
	
	return result;
}

IMediaDemuxer*
ASFDemuxerInfo::Create (Media* media)
{
	return new ASFDemuxer (media);
} 

/*
 *	FileSource
 */

FileSource::FileSource (Media *media) :
	IMediaSource (media)
{
	filename = g_strdup (media->GetFileOrUrl ());
	fd = NULL;
}

FileSource::~FileSource ()
{
	g_free (filename);
	filename = NULL;
	if (fd) {
		fclose (fd);
		fd = NULL;
	}
}

bool
FileSource::Eof ()
{
	printf ("FileSource::Eof ().\n");
	printf ("FileSource::Eof (): Not implemented.\n");
	return false;
}

bool
FileSource::Read (void* buffer, guint32 size)
{
	//printf ("FileSource::Read (%p, %u).\n", buffer, size);
	
	size_t bytes_read;
	
	if (buffer == NULL) {
		media->AddMessage (MEDIA_INVALID_ARGUMENT, g_strdup_printf ("FileSource::Read (%p, %u): buffer\n", buffer, size));
		return false;
	}

	// Open the file if it hasn't been opened.
	if (!fd) {
		fd = fopen (filename, "rb");
		if (!fd) {
			media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("Could not open the file '%s': %s.\n", filename, strerror (errno)));
			return false;
		}
	}

	// Read 
	bytes_read = fread (buffer, 1, size, fd);
	
	// Report any errors
	if (bytes_read != size) {
		if (ferror (fd) != 0) {
			media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("Could not read from the file '%s': %s.\n", filename, strerror (errno)));
		} else if (feof (fd) != 0) {
			media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("Reached end of file prematurely of the file '%s'.\n", filename));
		} else {
			media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("Unspecified error while reading the file '%s'.\n", filename));
		}
		return false;
	}

	return true;
}

bool
FileSource::Peek (void* buffer, guint32 size)
{
	// Simple implementation of peek: save position, read bytes, restore position.
	gint64 position = GetPosition ();
	if (!Read (buffer, size))
		return false;
	return Seek (position, SEEK_SET);
}

guint64
FileSource::GetPosition ()
{
	//printf ("FileSource::GetPosition ().\n");
	return fd == NULL ? 0 : ftell (fd);
}

bool
FileSource::IsSeekable ()
{
	printf ("FileSource::IsSeekable ().\n");
	
	return true;
}

bool
FileSource::Seek (gint64 offset)
{
	//printf ("FileSource::Seek (%llu).\n", position);
	
	return Seek (offset, SEEK_CUR);
}

bool
FileSource::Seek (gint64 offset, int mode)
{
	//printf ("FileSource::Seek (%llu, %i).\n", offset, mode);
	
	int result = fseek (fd, offset, mode);
	if (result != 0) {
		media->AddMessage (MEDIA_SEEK_ERROR, g_strdup_printf ("Can't seek to offset %llu with mode %i in '%s': %s.\n", offset, mode, filename, strerror (errno)));
		return false;
	}
	//printf  ("fseek returned: %i, position: %llu\n", result, GetPosition ());
	return true;
}

/*
 * MediaClosure
 */ 

MediaClosure::MediaClosure () : 
	callback (NULL), frame (NULL), media (NULL), context (NULL)
{
}

MediaClosure::~MediaClosure ()
{
	delete frame;
	frame = NULL;
}

MediaResult
MediaClosure::Call ()
{
	if (callback)
		return callback (this);
		
	return MEDIA_NOCALLBACK;
}
/*
 * IMediaStream
 */

IMediaStream::IMediaStream (Media* media) : 
	extra_data (NULL), extra_data_size (NULL), codec_id (0), start_time (0),
	msec_per_frame (0), duration (0), decoder (NULL), codec (NULL), min_padding (0),
	index (-1), context (NULL)
{
}

IMediaStream::~IMediaStream ()
{
	delete decoder;
	decoder = NULL;
	
	g_free (extra_data);
	extra_data = NULL;
	extra_data_size = 0;
	
}

/*
 * IMediaDemuxer
 */ 
 
IMediaDemuxer::~IMediaDemuxer ()
{
	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++) {
			delete streams [i];
			streams [i] = NULL;
		}
		g_free (streams);
		streams = NULL;
		stream_count = 0;
	}
}

/*
 * MediaFrame
 */ 
 
MediaFrame::MediaFrame (IMediaStream* str) : 
	stream (str), decoder_specific_data (NULL), 
	pts (0), duration (0), state (0), compressed_size (0), compressed_data (NULL),
	uncompressed_size (0), uncompressed_data (NULL), srcSlideY (0), srcSlideH (0)
{
	for (int i = 0; i < 4; i++) {
		uncompressed_data_stride [i] = 0;  
		srcStride [i] = 0;
	}
}
 
MediaFrame::~MediaFrame ()
{
	g_free (compressed_data);
	compressed_data = NULL;
	g_free (uncompressed_data);
	uncompressed_data = NULL;
	
	if (decoder_specific_data != NULL) {
		if (stream != NULL && stream->decoder != NULL) { 
			stream->decoder->Cleanup (this);
		} else {
			printf ("MediaFrame::~MediaFrame (): Couldn't call decoder->Cleanup.\n");
		}
	}
}

/*
 * IMediaObject
 */
 
IMediaObject::IMediaObject (Media* med) : 
	media (med), callback (NULL)
{
}

IMediaObject::~IMediaObject ()
{
}

/*
 * IMediaSource
 */

/*
 * IMediaDemuxer
 */

void
IMediaDemuxer::SetStreams (IMediaStream** streams, int count)
{
	this->streams = streams;
	this->stream_count = count;
}

/*
 * IMediaDecoder
 */

IMediaDecoder::IMediaDecoder (Media* media, IMediaStream* stream)
{
	this->media = media;
	this->stream = stream;
}

/*
 * IImageConverter
 */

IImageConverter::IImageConverter (Media* med, VideoStream* str) : 
	media (med), stream (str), input_format (MoonPixelFormatNone), output_format (MoonPixelFormatNone)
{
}

/*
 * VideoStream
 */


VideoStream::VideoStream (Media* media) : IMediaStream (media),
	width (0), height (0), msec_per_frame (0), initial_pts (0),
	bits_per_sample (0), converter (NULL)
{
}

VideoStream::~VideoStream ()
{
	if (converter != NULL) {
		delete converter;
		converter = NULL;
	}
}

/*
 * MediaClosure
 */


/*
 * MediaMarker
 */ 

MediaMarker::MediaMarker (const char* type, const char* text, guint64 pts)
{
	this->type = g_strdup (type);
	this->text = g_strdup (text);
	this->pts = pts;
}

MediaMarker::~MediaMarker ()
{
	g_free (type);
	type = NULL;
	g_free (text);
	text = NULL;
}

/*
 * MarkerStream
 */
 
MarkerStream::MarkerStream (Media* media) 
	: IMediaStream (media),
	closure (NULL)
{
}

MarkerStream::~MarkerStream ()
{
	delete closure;
	closure = NULL;
}

void
MarkerStream::SetCallback (MediaClosure* cl)
{
	if (closure != NULL)
		delete closure;
	closure = cl;
}


