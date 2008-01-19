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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pipeline.h"
#include "pipeline-ffmpeg.h"
#include "uri.h"
#include "media.h"
#include "asf/asf.h"
#include "asf/asf-structures.h"
#include "runtime.h"

#include <fcntl.h>
#include <pthread.h>

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
		queued_requests (NULL),	queue_closure (NULL), 
		file_or_url (NULL), source (NULL), 
		demuxer (NULL), markers (NULL),
		start_time (0), opened (false)
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

void
Media::SetFileOrUrl (const char *value)
{
	if (file_or_url)
		g_free (file_or_url);
	file_or_url = g_strdup (value);
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
	//printf ("Media::RegisterDemuxer (%p - %s)\n", info, info->GetName ());
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
	//printf ("Media::RegisterConverter (%p)\n", info);
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
	//printf ("Media::RegisterDecoder (%p)\n", info);
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
	Media::RegisterDemuxer (new Mp3DemuxerInfo ());
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
	if (!MEDIA_SUCCEEDED (result))
		printf ("Media::AddMessage (%i, '%s').\n", result, msg);
}

void
Media::AddMessage (MediaResult result, char *msg)
{
	AddMessage (result, (const char *) msg);
	g_free (msg);
}

MediaResult
Media::Seek (uint64_t pts)
{
	if (demuxer)
		return demuxer->Seek (pts);

	return MEDIA_FAIL;
}

MediaResult
Media::SeekAsync (uint64_t pts)
{
	if (demuxer == NULL)
		return MEDIA_FAIL;
	
	Media::WorkNode *work = new Media::WorkNode ();
	work->seek_pts = pts;
	EnqueueWork (work);
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::Initialize (const char* file_or_url)
{
	printf ("Media::Initialize ('%s').\n", file_or_url);
	
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
			result = source->Initialize ();
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("Media::Open ('%s'): live source failed, trying progressive source.\n", file_or_url);
				delete source;
				source = new ProgressiveSource (this);
				result = source->Initialize ();
			}
		} else if (strcmp (uri->protocol, "http") == 0 || strcmp (uri->protocol, "https") == 0) {
			source = new ProgressiveSource (this);
			result = source->Initialize ();
			if (!MEDIA_SUCCEEDED (result)) {
				printf ("Media::Open ('%s'): progressive source failed, trying live source.\n", file_or_url);
				delete source;
				source = new LiveSource (this);
				result = source->Initialize ();
			}
		} else if (strcmp (uri->protocol, "file") == 0) {
			source = new FileSource (this),
			result = source->Initialize ();
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
		result = source->Initialize ();
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
Media::Open ()
{
	if (source == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "Media::Initialize () hasn't been called (or didn't succeed).");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if (IsOpened ()) {
		AddMessage (MEDIA_FAIL, "Media::Open () has already been called.");
		return MEDIA_FAIL;
	}
	
	return Open (source);
}

MediaResult
Media::Open (IMediaSource* source)
{
	MediaResult result;
	
	printf ("Media::Open ().\n");
	
	if (source == NULL || IsOpened ()) // Initialize wasn't called (or didn't succeed) or already open.
		return MEDIA_INVALID_ARGUMENT;
	
	this->source = source;
	
	// Select a demuxer
	DemuxerInfo *demuxerInfo = registered_demuxers;
	while (demuxerInfo != NULL) {
		if (demuxerInfo->Supports (source))
			break;
		
		//printf ("Media::Open (): '%s' can't handle this media.\n", demuxerInfo->GetName ());
		demuxerInfo = (DemuxerInfo *) demuxerInfo->next;
	}
	
	if (demuxerInfo == NULL) {
		AddMessage (MEDIA_UNKNOWN_MEDIA_TYPE, g_strdup_printf ("No demuxers registered to handle the media file '%s'.", file_or_url));
		return MEDIA_UNKNOWN_MEDIA_TYPE;
	}
	
	// Found a demuxer
	demuxer = demuxerInfo->Create (this);
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
				VideoStream *vs = (VideoStream *) stream;
				IImageConverter *converter = NULL;
				
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
	
	opened = true;
	
	return result;
}

MediaResult
Media::GetNextFrame (MediaFrame *frame, uint16_t states)
{
	//printf ("Media::GetNextFrame (%p).\n", stream);
	
	if (frame == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "frame is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if (frame->stream == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "frame->stream is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	MediaResult result = MEDIA_SUCCESS;
	
	if ((states & FRAME_DEMUXED) != FRAME_DEMUXED)
		return result; // Nothing to do?
	
	result = demuxer->ReadFrame (frame);
	if (!MEDIA_SUCCEEDED (result))
		return result;
	
	if ((states & FRAME_DECODED) != FRAME_DECODED)
		return result;
	
	if (frame->event == 0) {
		result = frame->stream->decoder->DecodeFrame (frame);
		if (!MEDIA_SUCCEEDED (result))
			return result;
	} else {
		frame->AddState (FRAME_DECODED);
	}
	
	//printf ("Media::GetNextFrame (%p) finished, size: %i.\n", stream, frame->buflen);
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::GetNextFrame (MediaFrame *frame)
{
	return GetNextFrame (frame, FRAME_DEMUXED | FRAME_DECODED);
}

void * 
Media::FrameReaderLoop (void *data)
{
	Media *media = (Media *) data;
	
	media->FrameReaderLoop ();
	
	return NULL;
}

#define LOG_FRAMEREADERLOOP(x, ...)// printf (x, __VA_ARGS__);

void
Media::FrameReaderLoop ()
{
	LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop ().\n");
	while (queued_requests != NULL) {
		Media::WorkNode *node = NULL, *current = NULL;
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): entering mutex.\n");
		// Wait until we have something in the queue
		pthread_mutex_lock (&queue_mutex);
		while (queued_requests != NULL && queued_requests->IsEmpty ())
			pthread_cond_wait (&queue_condition, &queue_mutex);
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): got something.\n");
		
		if (queued_requests != NULL) {
			// Find the first audio node
			current = (Media::WorkNode*) queued_requests->First ();
			while (current != NULL && current->frame != NULL && current->frame->stream->GetType () != MediaTypeAudio) {
				current = (Media::WorkNode*) current->next;
			}
			
			if (current != NULL && current->frame != NULL && current->frame->stream->GetType () == MediaTypeAudio) {
				node = current;
			} else {
				// No audio node, just get the first node
				node = (Media::WorkNode*) queued_requests->First ();
			}
			
			queued_requests->Unlink (node);
			LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): got a %s node, there are %i nodes left.\n",
					     node->stream->GetType () == MediaTypeAudio ? "audio" :
					     (node->stream->GetType () == MediaTypeVideo ? "video" : "unknown"),
					     queued_requests->Length ());
		}
		pthread_mutex_unlock (&queue_mutex);
		
		if (node == NULL)
			continue; // Found nothing, continue waiting.
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): processing node.\n");
		
		if (node->frame != NULL) {
			// Now demux and decode what we found and send it to who asked for it
			MediaResult result = GetNextFrame (node->frame, node->states);
			if (MEDIA_SUCCEEDED (result)) {
				MediaClosure *closure = new MediaClosure ();
				memcpy (closure, queue_closure, sizeof (MediaClosure));
				closure->frame = node->frame;
				closure->Call ();
				delete closure;
			}
		} else { // Or seek if that was what was requested.
			Seek (node->seek_pts);
		}
		delete node;
	}
	LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): exiting.\n");
}

#include <sched.h>

void
Media::GetNextFrameAsync (MediaFrame *frame)
{
	Media::GetNextFrameAsync (frame, FRAME_DEMUXED | FRAME_DECODED);
}

void
Media::GetNextFrameAsync (MediaFrame *frame, uint16_t states)
{
	if (frame == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "frame is NULL.");
		return;
	}
	
	if (frame->stream == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "frame->stream is NULL.");
		return;
	}
	
	Media::WorkNode *node = new Media::WorkNode ();
	node->frame = frame;
	node->states = states;
	EnqueueWork (node);
}

void
Media::EnqueueWork (Media::WorkNode	 *work)
{
	//printf ("Media::EnqueueWork (%p).\n", stream);
	if (queued_requests == NULL) {
		//printf ("Media::EnqueueWork (%p). Creating threads.\n", stream);
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
	
	queued_requests->Append (work);
	
	pthread_cond_signal (&queue_condition);
	
	pthread_mutex_unlock (&queue_mutex);
}

void
Media::ClearQueue ()
{
	//printf ("Media::ClearQueue ().\n");
	if (queued_requests != NULL) {
		pthread_mutex_lock (&queue_mutex);
		queued_requests->Clear (true);
		pthread_mutex_unlock (&queue_mutex);
	}
	
	if (source->GetType () == MoonProgressiveSource) {
		ProgressiveSource* ps = (ProgressiveSource*) source;
		ps->CancelWait ();
	}
}

void
Media::DeleteQueue ()
{
	//printf ("Media::DeleteQueue ().\n");
	if (queued_requests != NULL) {
		pthread_mutex_lock (&queue_mutex);
		queued_requests->Clear (true);
		delete queued_requests;
		queued_requests = NULL;
		pthread_cond_signal (&queue_condition);
		pthread_mutex_unlock (&queue_mutex);
		
		if (source->GetType () == MoonProgressiveSource) {
			ProgressiveSource* ps = (ProgressiveSource*) source;
			ps->CancelWait ();
		}
		pthread_join (queue_thread, NULL);
		pthread_mutex_destroy (&queue_mutex);
		pthread_cond_destroy (&queue_condition);
		pthread_detach (queue_thread);
	}
	
	if (source != NULL && source->GetType () == MoonProgressiveSource) {
		ProgressiveSource* ps = (ProgressiveSource*) source;
		ps->CancelWait ();
	}
}


/*
 * ASFDemuxer
 */

ASFDemuxer::ASFDemuxer (Media *media) : IMediaDemuxer (media)
{
	stream_to_asf_index = NULL;
	reader = NULL;
	parser = NULL;
}

ASFDemuxer::~ASFDemuxer ()
{
	g_free (stream_to_asf_index);
	
	if (reader)
		delete reader;
	
	if (parser)
		delete parser;
}

MediaResult
ASFDemuxer::Seek (uint64_t pts)
{
	if (reader == NULL)
		reader = new ASFFrameReader (parser);
	
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
	List *markers = media->GetMarkers ();
	//guint64 preroll = parser->file_properties->preroll;
	const char *type;
	uint64_t pts;
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
	
	i = -1;
	while (commands != NULL && commands [++i] != NULL) {
		asf_script_command_entry *entry = commands [i];
		
		text = entry->get_name ();
		pts = entry->pts; //(entry->pts - preroll) * 10000;
		
		if (entry->type_index + 1 <= command->command_type_count)
			type = command_types [entry->type_index];
		else
			type = "";
		
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
			text = marker_entry->get_marker_description ();
			
			pts = marker_entry->pts / 10000; // (marker_entry->pts - preroll * 10000);
			
			markers->Append (new MediaMarker::Node (new MediaMarker ("Name", text, pts)));
			
			//printf ("MediaElement::ReadMarkers () Added marker at %llu (text: %s, type: %s)\n", pts, text, "Name");
		
			g_free (text);
		}
	}
	
		
cleanup:
	
	g_strfreev (command_types);
	g_free (commands);
}

MediaResult
ASFDemuxer::ReadHeader ()
{
	MediaResult result = MEDIA_SUCCESS;
	ASFSource *asf_source = new ASFMediaSource (NULL, GetMedia ()->GetSource ());
	ASFParser *asf_parser = new ASFParser (asf_source);
	int32_t *stream_to_asf_index = NULL;
	IMediaStream **streams = NULL;
	int current_stream = 1;
	int stream_count = 0;
	
	printf ("ASFDemuxer::ReadHeader ().\n");
	
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
	streams = (IMediaStream **) g_malloc0 (sizeof (IMediaStream *) * (stream_count + 1)); // End with a NULL element.
	stream_to_asf_index = (int32_t *) g_malloc0 (sizeof (int32_t) * (stream_count + 1)); 

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
ASFDemuxer::ReadFrame (MediaFrame *frame)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	
	if (reader == NULL)
		reader = new ASFFrameReader (parser);
	
	//printf ("ASFDemuxer::ReadFrame (%p) frame = ", frame);
	//frame->printf ();
	//printf ("\n");
	
	if (!reader->Advance (stream_to_asf_index [frame->stream->index])) {
		if (!reader->Eof ()) {
			media->AddMessage (MEDIA_DEMUXER_ERROR, "Error while advancing to the next frame.");
			return MEDIA_DEMUXER_ERROR;
		} else {
			media->AddMessage (MEDIA_NO_MORE_DATA, "Reached end of data.");
			frame->event = FrameEventEOF;
			return MEDIA_NO_MORE_DATA;
		}
	}
	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	frame->buflen = reader->Size ();
	frame->buffer = (uint8_t *) g_try_malloc (frame->buflen + frame->stream->min_padding);
	
	if (frame->buffer == NULL) {
		media->AddMessage (MEDIA_OUT_OF_MEMORY, "Could not allocate memory for next frame.");
		return MEDIA_OUT_OF_MEMORY;
	}
	
	//printf ("ASFDemuxer::ReadFrame (%p), min_padding = %i\n", frame, frame->stream->min_padding);
	if (frame->stream->min_padding > 0)
		memset (frame->buffer + frame->buflen, 0, frame->stream->min_padding); 
	
	if (!reader->Write (frame->buffer)) {
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
ASFDemuxerInfo::Supports (IMediaSource *source)
{
	uint8_t buffer[16];
	
	if (!source->Peek (buffer, 16))
		return false;
	
	bool result = asf_guid_compare (&asf_guids_header, (asf_guid *) buffer);
	
	printf ("ASFDemuxerInfo::Supports (%p): probing result: %s\n", source,
		result ? "true" : "false");
	
	return result;
}

IMediaDemuxer *
ASFDemuxerInfo::Create (Media *media)
{
	return new ASFDemuxer (media);
}



/*
 * MPEG Audio Demuxer
 */

struct MpegFrameHeader {
	uint8_t version:2;
	uint8_t layer:2;
	
	uint8_t copyright:1;
	uint8_t original:1;
	uint8_t padded:1;
	uint8_t prot:1;
	
	uint8_t channels:6;
	uint8_t intensity:1;
	uint8_t ms:1;
	
	int32_t bit_rate;
	int32_t sample_rate;
};

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

static int
mpeg_parse_bitrate (MpegFrameHeader *mpeg, uint8_t byte)
{
	int i = (byte & 0xf0) >> 4;
	
	if (i > 14)
		return -1;
	
	if (mpeg->version == 1)
		mpeg->bit_rate = mpeg1_bitrates[mpeg->layer - 1][i];
	else
		mpeg->bit_rate = mpeg2_bitrates[mpeg->layer - 1][i];
	
	return 0;
}

static int mpeg_samplerates[3][3] = {
	{ 44100, 48000, 32000 },  // version 1
	{ 22050, 24000, 16000 },  // version 2
	{ 11025, 12000,  8000 }   // version 2.5
};

static int
mpeg_parse_samplerate (MpegFrameHeader *mpeg, uint8_t byte)
{
	int i = byte & 0x0c;
	
	if (i > 2)
		return -1;
	
	mpeg->sample_rate = mpeg_samplerates[mpeg->version - 1][i];
	
	return 0;
}

static int
mpeg_parse_channels (MpegFrameHeader *mpeg, uint8_t byte)
{
	int mode = (byte >> 6) & 0x3;
	
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
	
	return 0;
}

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


static int
mpeg_parse_header (MpegFrameHeader *mpeg, const uint8_t *buffer)
{
	if (!is_mpeg_header (buffer))
		return -1;
	
	// extract the MPEG version
	switch ((buffer[1] >> 3) & 0x3) {
	case 0: /* MPEG Version 2.5 */
		mpeg->version = 3;
		break;
	case 1: /* reserved */
		return -1;
	case 2: /* MPEG Version 2 */
		mpeg->version = 2;
		break;
	case 3: /* MPEG Version 1 */
		mpeg->version = 1;
		break;
	}
	
	// extract the MPEG layer
	switch (buffer[1] & 0x06) {
	case 0x02:
		mpeg->layer = 3;
		break;
	case 0x04:
		mpeg->layer = 2;
		break;
	case 0x06:
		mpeg->layer = 1;
		break;
	default: /* 0x00 reserved */
		return -1;
	}
	
	// protection (via 16bit crc) bit
	mpeg->prot = (buffer[1] & 0x01) ? 1 : 0;
	
	// extract the bit rate
	if (mpeg_parse_bitrate (mpeg, buffer[2]) == -1)
		return -1;
	
	// extract the sample rate
	if (mpeg_parse_samplerate (mpeg, buffer[2]) == -1)
		return -1;
	
	// check if the frame is padded
	mpeg->padded = (buffer[2] & 0x2) ? 1 : 0;
	
	// extract the channel mode */
	if (mpeg_parse_channels (mpeg, buffer[3]) == -1)
		return -1;
	
	mpeg->copyright = (buffer[3] & 0x08) ? 1 : 0;
	mpeg->original = (buffer[3] & 0x04) ? 1 : 0;
	
	return 0;
}

static int mpeg_block_sizes[3][3] = {
	{ 384, 1152, 1152 },  // version 1
	{ 384, 1152,  576 },  // version 2
	{ 384, 1152,  576 }   // version 2.5
};

#define mpeg_block_size(mpeg) mpeg_block_sizes[(mpeg)->version - 1][(mpeg)->layer - 1]

static uint32_t
mpeg_frame_length (MpegFrameHeader *mpeg)
{
	uint32_t len;
	
	// calculate the frame length
	if (mpeg->layer == 1)
		len = (((12 * mpeg->bit_rate) / mpeg->sample_rate) + mpeg->padded) * 4;
	else
		len = ((144 * mpeg->bit_rate) / mpeg->sample_rate) + mpeg->padded;
	
	if (mpeg->prot) {
		// include 2 extra bytes for 16bit crc
		len += 2;
	}
	
	return len;
}

#define mpeg_frame_duration(mpeg) (48000000 / (mpeg)->sample_rate) * 8


#define MPEG_JUMP_TABLE_GROW_SIZE 16

Mp3FrameReader::Mp3FrameReader (IMediaSource *source, int64_t start)
{
	jmptab = g_new (MpegFrame, MPEG_JUMP_TABLE_GROW_SIZE);
	avail = MPEG_JUMP_TABLE_GROW_SIZE;
	used = 0;
	
	stream_start = start;
	stream = source;
	
	bit_rate = 0;
	cur_pts = 0;
}

Mp3FrameReader::~Mp3FrameReader ()
{
	g_free (jmptab);
}

void
Mp3FrameReader::AddFrameIndex (int64_t offset, uint64_t pts, uint32_t dur, int32_t bit_rate)
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

uint32_t
Mp3FrameReader::MpegFrameSearch (uint64_t pts)
{
	uint64_t start, end;
	uint32_t hi = used - 1;
	uint32_t m = hi >> 1;
	uint32_t lo = 0;
	
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

bool
Mp3FrameReader::Seek (uint64_t pts)
{
	int64_t offset = stream->GetPosition ();
	int32_t bit_rate = this->bit_rate;
	uint64_t cur_pts = this->cur_pts;
	uint32_t frame;
	
	if (pts == cur_pts)
		return true;
	
	if (pts == 0) {
		if (!stream->Seek (stream_start, SEEK_SET))
			goto exception;
		
		bit_rate = 0;
		cur_pts = 0;
		
		return true;
	}
	
	// if we are seeking to some place we've been, then we can use our jump table
	if (used > 0 && pts < (jmptab[used - 1].pts + jmptab[used - 1].dur)) {
		if (pts >= jmptab[used - 1].pts) {
			if (!stream->Seek (jmptab[used - 1].offset, SEEK_SET))
				goto exception;
			
			this->bit_rate = jmptab[used - 1].bit_rate;
			this->cur_pts = jmptab[used - 1].pts;
			
			return true;
		}
		
		// search for our requested pts
		frame = MpegFrameSearch (pts);
		
		g_assert (frame < used);
		
		if (!stream->Seek (jmptab[frame].offset, SEEK_SET))
			goto exception;
		
		this->bit_rate = jmptab[frame].bit_rate;
		this->cur_pts = jmptab[frame].pts;
		
		return true;
	}
	
	// keep skipping frames until we read to (or past) the requested pts
	while (this->cur_pts < pts) {
		if (!SkipFrame ())
			goto exception;
	}
	
	// pts requested is at the start of the next frame in the stream
	if (this->cur_pts == pts)
		return true;
	
	// pts requested was non-key frame, need to seek back to the most recent key frame
	if (!stream->Seek (jmptab[used - 1].offset, SEEK_SET))
		goto exception;
	
	this->bit_rate = jmptab[used - 1].bit_rate;
	this->cur_pts = jmptab[used - 1].pts;
	
	return true;
	
exception:
	
	// restore FrameReader to previous state
	stream->Seek (offset, SEEK_SET);
	this->bit_rate = bit_rate;
	this->cur_pts = cur_pts;
	
	return false;
}

bool
Mp3FrameReader::SkipFrame ()
{
	MpegFrameHeader mpeg;
	uint32_t duration;
	uint8_t buffer[4];
	int64_t offset;
	uint32_t len;
	
	offset = stream->GetPosition ();
	
	if (!stream->Read (buffer, 4))
		return false;
	
	memset ((void *) &mpeg, 0, sizeof (mpeg));
	if (mpeg_parse_header (&mpeg, buffer) == -1)
		return false;
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = mpeg_frame_length (&mpeg);
	
	if (!stream->Seek ((int64_t) (len - 4), SEEK_CUR))
		return false;
	
	cur_pts += duration;
	
	return true;
}

MediaResult
Mp3FrameReader::ReadFrame (MediaFrame *frame)
{
	MpegFrameHeader mpeg;
	uint32_t duration;
	uint8_t buffer[4];
	int64_t offset;
	uint32_t len;
	
	offset = stream->GetPosition ();
	
	if (!stream->Read (buffer, 4)) {
		frame->AddState (FRAME_DEMUXED);
		frame->event = FrameEventEOF;
		return MEDIA_NO_MORE_DATA;
	}
	
	memset ((void *) &mpeg, 0, sizeof (mpeg));
	if (mpeg_parse_header (&mpeg, buffer) == -1)
		return MEDIA_DEMUXER_ERROR;
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = mpeg_frame_length (&mpeg);
	frame->buflen = len;
	
	if (mpeg.layer != 1 && !mpeg.padded)
		frame->buffer = (uint8_t *) g_try_malloc (frame->buflen + 1);
	else
		frame->buffer = (uint8_t *) g_try_malloc (frame->buflen);
	
	if (frame->buffer == NULL)
		return MEDIA_OUT_OF_MEMORY;
	
	if (mpeg.layer != 1 && !mpeg.padded)
		frame->buffer[frame->buflen - 1] = 0;
	
	memcpy (frame->buffer, buffer, 4);
	
	if (!stream->Read (frame->buffer + 4, len - 4))
		frame->event = FrameEventEOF;
	
	frame->pts = cur_pts;
	frame->duration = duration;
	
	frame->AddState (FRAME_DEMUXED);
	
	cur_pts += duration;
	
	if (frame->event == FrameEventEOF)
		return MEDIA_NO_MORE_DATA;
	
	return MEDIA_SUCCESS;
}


/*
 * Mp3Demuxer
 */

Mp3Demuxer::Mp3Demuxer (Media *media) : IMediaDemuxer (media)
{
	reader = NULL;
}

Mp3Demuxer::~Mp3Demuxer ()
{
	if (reader)
		delete reader;
}

MediaResult
Mp3Demuxer::Seek (uint64_t pts)
{
	if (reader && reader->Seek (pts))
		return MEDIA_SUCCESS;
	
	return MEDIA_FAIL;
}

MediaResult
Mp3Demuxer::ReadHeader ()
{
	IMediaSource *source = GetMedia ()->GetSource ();
	IMediaStream **streams = NULL;
	int64_t stream_start;
	IMediaStream *stream;
	MpegFrameHeader mpeg;
	AudioStream *audio;
	uint8_t buffer[10];
	uint64_t duration;
	uint32_t size = 0;
	uint32_t nframes;
	int stream_count;
	uint32_t len;
	int64_t end;
	int i;
	
	if (!source->Read (buffer, 10))
		return MEDIA_INVALID_MEDIA;
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80)
				return false;
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		// skip over the ID3 tag
		if (!source->Seek ((int64_t) size, SEEK_SET))
			return MEDIA_INVALID_MEDIA;
		
		if (!source->Read (buffer, 4))
			return MEDIA_INVALID_MEDIA;
		
		stream_start = (int64_t) size;
	} else {
		stream_start = 0;
	}
	
	if (!source->Seek (0, SEEK_END))
		return MEDIA_INVALID_MEDIA;
	
	end = source->GetPosition ();
	
	if (!source->Seek (stream_start, SEEK_SET))
		return MEDIA_INVALID_MEDIA;
	
	memset ((void *) &mpeg, 0, sizeof (mpeg));
	if (mpeg_parse_header (&mpeg, buffer) == -1)
		return MEDIA_INVALID_MEDIA;
	
	// calculate the duration of the first frame
	duration = mpeg_frame_duration (&mpeg);
	
	// calculate the frame length
	len = mpeg_frame_length (&mpeg);
	
	// estimate the number of frames
	nframes = (end - stream_start) / len;
	
	reader = new Mp3FrameReader (source, stream_start);
	
	stream = audio = new AudioStream (GetMedia ());
	audio->codec_id = CODEC_MP3;
	audio->codec = "mp3";
	
	audio->duration = duration * nframes;
	audio->bit_rate = mpeg.bit_rate;
	audio->channels = mpeg.channels;
	audio->sample_rate = mpeg.sample_rate;
	audio->block_align = mpeg_block_size (&mpeg);
	audio->bits_per_sample = mpeg.layer == 1 ? 32 : 8;
	audio->extra_data = NULL;
	audio->extra_data_size = 0;
	
	stream->start_time = 0;
	
	streams = g_new (IMediaStream *, 2);
	streams[0] = stream;
	streams[1] = NULL;
	stream_count = 1;
	
	SetStreams (streams, stream_count);
	
	return MEDIA_SUCCESS;
}

MediaResult
Mp3Demuxer::ReadFrame (MediaFrame *frame)
{
	return reader->ReadFrame (frame);
}


/*
 * Mp3DemuxerInfo
 */

bool
Mp3DemuxerInfo::Supports (IMediaSource *source)
{
	uint8_t buffer[10];
	uint32_t size = 0;
	int i;
	
	// peek at the first 10 bytes which is enough to contain
	// either the mp3 frame header or an ID3 tag header
	if (!source->Peek (buffer, 10))
		return false;
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80)
				return false;
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		// skip over the ID3 tag
		if (!source->Seek (size, SEEK_SET))
			return false;
		
		// peek at the mp3 frame header
		if (!source->Read (buffer, 4))
			return false;
		
		if (!source->Seek (0, SEEK_SET))
			return false;
	}
	
	return is_mpeg_header (buffer);
}

IMediaDemuxer *
Mp3DemuxerInfo::Create (Media *media)
{
	return new Mp3Demuxer (media);
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

FileSource::FileSource (Media *media, const char *filename) :
	IMediaSource (media)
{
	this->filename = g_strdup (filename);
	fd = NULL;
}

FileSource::~FileSource ()
{
	g_free (filename);
	if (fd)
		fclose (fd);
}

MediaResult 
FileSource::Initialize ()
{
	if (fd) {
		media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("The file stream '%s' has already been initialized.", filename));
		return MEDIA_FILE_ERROR;
	}
	
	// Open the file
	if (!(fd = fopen (filename, "rb"))) {
		media->AddMessage (MEDIA_FILE_ERROR, g_strdup_printf ("Could not open the file '%s': %s.", filename, strerror (errno)));
		return MEDIA_FILE_ERROR;
	}
	
	return MEDIA_SUCCESS;
}

bool
FileSource::Eof ()
{
	if (fd)
		return feof (fd) != 0;
	
	return true;
}

bool
FileSource::Read (void *buffer, uint32_t n)
{
	size_t nread;
	
	//printf ("FileSource::Read (%p, %u).\n", buffer, size);
	
	if (buffer == NULL) {
		fprintf (stderr, "FileSource::Read (%p, %u): buffer\n", buffer, n);
		return false;
	}
	
	// Open the file if it hasn't been opened.
	if (!fd) {
		// Initialize should probably already have been called, but handle this case anyway.
		if (!MEDIA_SUCCEEDED (Initialize ()))
			return false;
	}
	
	// Read
	if ((nread = fread (buffer, 1, n, fd)) == n)
		return true;
	
	if (ferror (fd) != 0) {
		fprintf (stderr, "FileSource::Read (%p, %u): only managed to read %u bytes from `%s': %s.\n",
			 buffer, n, nread, filename, strerror (errno));
	} else if (feof (fd) != 0) {
		fprintf (stderr, "FileSource::Read (%p, %u): only managed to read %u bytes from `%s': end of file reached.\n",
			 buffer, n, nread, filename);
	} else {
		fprintf (stderr, "FileSource::Read (%p, %u): only managed to read %u bytes from `%s': unknown error.\n",
			 buffer, n, nread, filename);
	}
	
	return false;
}

bool
FileSource::Peek (void *buffer, uint32_t n)
{
	// Simple implementation of peek: save position, read bytes, restore position.
	uint64_t position = GetPosition ();
	
	if (!Read (buffer, n))
		return false;
	
	return Seek (position, SEEK_SET);
}

uint64_t
FileSource::GetPosition ()
{
	//printf ("FileSource::GetPosition ().\n");
	return fd == NULL ? 0 : ftell (fd);
}

bool
FileSource::IsSeekable ()
{
	return true;
}

bool
FileSource::Seek (int64_t offset)
{
	return Seek (offset, SEEK_CUR);
}

bool
FileSource::Seek (int64_t offset, int mode)
{
	if (fseek (fd, offset, mode) == 0)
		return true;
	
	fprintf (stderr, "FileSource::Seek (%lld, %d) on `%s' failed: %s.\n",
		 offset, mode, filename, strerror (errno));
	
	return false;
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

IMediaStream::IMediaStream (Media *media)
{
	context = NULL;
	
	extra_data_size = 0;
	extra_data = NULL;
	
	msec_per_frame = 0;
	start_time = 0;
	duration = 0;
	
	decoder = NULL;
	codec_id = 0;
	codec = NULL;
	
	min_padding = 0;
	index = -1;
}

IMediaStream::~IMediaStream ()
{
	delete decoder;
	
	g_free (extra_data);
}

/*
 * IMediaDemuxer
 */ 
 
IMediaDemuxer::~IMediaDemuxer ()
{
	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++)
			delete streams[i];
		
		g_free (streams);
	}
}

/*
 * MediaFrame
 */ 
 
MediaFrame::MediaFrame (IMediaStream *stream)
{
	decoder_specific_data = NULL;
	this->stream = stream;
	
	duration = 0;
	pts = 0;
	
	buffer = NULL;
	buflen = 0;
	state = 0;
	event = 0;
	
	for (int i = 0; i < 4; i++) {
		data_stride[i] = 0;  
		srcStride[i] = 0;
	}
	
	srcSlideY = 0;
	srcSlideH = 0;
}
 
MediaFrame::~MediaFrame ()
{
	g_free (buffer);
	
	if (decoder_specific_data != NULL) {
		if (stream != NULL && stream->decoder != NULL)
			stream->decoder->Cleanup (this);
	}
}

/*
 * IMediaObject
 */
 
IMediaObject::IMediaObject (Media *media)
{
	this->callback = NULL;
	this->media = media;
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

IMediaDecoder::IMediaDecoder (Media *media, IMediaStream *stream)
{
	this->media = media;
	this->stream = stream;
}

/*
 * IImageConverter
 */

IImageConverter::IImageConverter (Media *media, VideoStream *stream)
{
	output_format = MoonPixelFormatNone;
	input_format = MoonPixelFormatNone;
	this->stream = stream;
	this->media = media;
}

/*
 * VideoStream
 */

VideoStream::VideoStream (Media *media) : IMediaStream (media)
{
	converter = NULL;
	bits_per_sample = 0;
	msec_per_frame = 0;
	initial_pts = 0;
	height = 0;
	width = 0;
}

VideoStream::~VideoStream ()
{
	if (converter)
		delete converter;
}

/*
 * MediaClosure
 */


/*
 * MediaMarker
 */ 

MediaMarker::MediaMarker (const char *type, const char *text, uint64_t pts)
{
	this->type = g_strdup (type);
	this->text = g_strdup (text);
	this->pts = pts;
}

MediaMarker::~MediaMarker ()
{
	g_free (type);
	g_free (text);
}

/*
 * MarkerStream
 */
 
MarkerStream::MarkerStream (Media *media) : IMediaStream (media)
{
	closure = NULL;
}

MarkerStream::~MarkerStream ()
{
	delete closure;
}

void
MarkerStream::SetCallback (MediaClosure *closure)
{
	if (this->closure)
		delete this->closure;
	
	this->closure = closure;
}

/*
 * ProgressiveSource
 */

ProgressiveSource::ProgressiveSource (Media* media)
 : IMediaSource (media), filename (NULL), 
 	write_position (0), notified_size (0), wait_count (0),
 	size_notified (false), cancel_wait (false)
{
	pthread_mutex_init (&write_mutex, NULL);
	pthread_cond_init (&write_condition, NULL);

	Initialize ();
}

ProgressiveSource::~ProgressiveSource ()
{
	//printf ("ProgressiveSource::~ProgressiveSource ()\n");
	Close ();
	
	pthread_cond_destroy (&write_condition);
	pthread_mutex_destroy (&write_mutex);
}

bool
ProgressiveSource::IsWaiting ()
{
	return g_atomic_int_get (&wait_count) != 0;
}

bool
ProgressiveSource::WakeUp ()
{
	return WakeUp (true);
}

bool
ProgressiveSource::WakeUp (bool lock)
{
	if (lock)
		Lock ();
	pthread_cond_signal (&write_condition);
	if (lock)
		Unlock ();
	return true;
}

void
ProgressiveSource::Close ()
{
	
	if (file != 0) {
		close (file);
		file = 0;
	}
	
	if (filename != NULL) {
		unlink (filename);
		g_free (filename);
		filename = NULL;
	}
	
	write_position = 0;
	notified_size = 0;
	size_notified = false;
}

void
ProgressiveSource::Lock ()
{
	int result = pthread_mutex_lock (&write_mutex);
	if (result != 0)
		media->AddMessage (MEDIA_FAIL, g_strdup_printf ("Could not lock progressive file writer mutex: %s", strerror (result)));
}

void
ProgressiveSource::Unlock ()
{
	pthread_mutex_unlock (&write_mutex);
}

MediaResult
ProgressiveSource::Initialize ()
{
	// printf ("ProgressiveSource::StartDownload ()\n");
	Close ();
	
	char filename [L_tmpnam + 1];
	
	memset (filename, 0, L_tmpnam + 1);
	
	// Create the temporary file where we store the data
	do {
		char* name = tmpnam_r (filename);
		if (name == NULL) {
			if (media)
				media->AddMessage (MEDIA_FAIL, "Could not create local temporary file.");
			return false;
		}

		file = open (filename, O_RDWR | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR);
		if (file == -1) {
			if (errno == EMFILE) { // Too many open files. This would make for a nice hang test.
				return MEDIA_FAIL;
			}
		}
	} while (file == -1);
	
	this->filename = g_strdup (filename);
	
	// Ready to be written to
	
	return MEDIA_SUCCESS;
}

void
ProgressiveSource::Write (void *buf, int32_t offset, int32_t n)
{
	//printf ("ProgressiveSource::Write (%p, %i, %i)\n", buf, offset, n);
	guint64 read_position = 0;
	ssize_t written = 0;
	ssize_t total = 0;

	Lock ();

	if (file == 0) {
		media->AddMessage (MEDIA_FAIL, "Progressive source doesn't have a file to write the data to.");
		goto cleanup;
	}
	
	if (n == 0) {
		// We've got the entire file, update the size
		//printf ("ProgressiveSource::Write (%p, %i, %i): We've got the entire file, final size: %lld (earlier notified size: %lld)\n", buf, offset, n, write_position, notified_size);
		size_notified = true;
		notified_size = write_position;
		goto cleanup;
	}
	
	// Save the current position
	read_position = lseek (file, 0, SEEK_CUR);
	
	// Seek to the write position
	if (lseek (file, offset, SEEK_SET) != offset) {
		goto cleanup;
	}
	
	while ((written = ::write (file, total + (char*) buf, n - total)) > 0) {
		//printf ("ProgressiveSource::Write (%p, %i, %i): Wrote %i bytes.\n", buf, offset, n, written);
		total += written;
		if (total >= n)
			break;
	}
	
	write_position += total;
	
	if (written == -1) {
		media->AddMessage (MEDIA_FAIL, g_strdup_printf ("progressive source couldn't write more data: %s", strerror (errno)));
	}
	
	//printf ("ProgressiveSource::Write (%p, %i, %i): New write position: %lld\n", buf, offset, n, write_position);
	
	if (IsWaiting ()) {
		WakeUp (false);
	}
	
	// Restore the current position
	lseek (file, read_position, SEEK_SET);
	
cleanup:
	Unlock ();
}

void
ProgressiveSource::NotifySize (int64_t size)
{
	//printf ("ProgressiveSource::NotifySize (%llu)\n", size);
	notified_size = size;
	size_notified = true;
}

bool
ProgressiveSource::WaitForPosition (uint64_t position)
{
	bool result = false;
	
	g_atomic_int_inc (&wait_count);
	
	Lock ();
	while (true) {
		if (cancel_wait) {
			cancel_wait = false; // FIXME: This doesn't work if there are more than one thread waiting at the same time
			goto cleanup;
		}
		
		if (size_notified && position > notified_size) {
			goto cleanup;
		}
		
		if (write_position > position) {
			result = true;
			goto cleanup;
		}
		
		// By the time this method is called, we might not need to wait, so the wait goes
		// after all the checks.
		printf ("Going to sleep...\n");
		pthread_cond_wait (&write_condition, &write_mutex);
	}
	
cleanup:
	Unlock ();
	
	g_atomic_int_dec_and_test (&wait_count);
	
	return result;
}

void
ProgressiveSource::CancelWait ()
{
	Lock ();
	cancel_wait = true;
	WakeUp (false);
	Unlock ();
}

bool
ProgressiveSource::IsSeekable ()
{
	return true;
}

bool
ProgressiveSource::Seek (int64_t offset)
{
	return Seek (offset, SEEK_CUR);
}

bool
ProgressiveSource::Seek (gint64 offset, int mode)
{
	bool result = false;
	
	if (file == 0) {
		media->AddMessage (MEDIA_INVALID_ARGUMENT, "File has not been opened.");
		return false;
	}
	
	Lock ();
	
	// Calculate the position from the mode and offset.
	uint64_t position;
	switch (mode) {
	case SEEK_CUR: position = GetPosition () + offset; break;
	case SEEK_SET: position = offset; break;
	default:
		// Due to the fact that many times we do not know the size of the file, it does not make sense to support SEEK_END
		media->AddMessage (MEDIA_FAIL, g_strdup_printf ("Invalid seek mode: %i", mode));
		return false;
	}
	
	// Seeked beyond end of file?
	if (size_notified && notified_size < position) {
		media->AddMessage (MEDIA_FAIL, "Seek beyond eof.");
		result = false;
		goto cleanup;
	}
	
	// If the requested position isn't downloaded yet, wait for it to be.
	if (position > write_position) {
		printf ("ProgressiveSource::Seek (%lld): Seeked beyond written size (%lld)\n", position, write_position);
		Unlock ();
		if (!(WaitForPosition (position))) {
			return false;
		}
		Lock ();
	}
	
	// Finally seek to where we want to be
	result = lseek (file, position, SEEK_SET) == (off_t) position;

cleanup:
	Unlock ();
	return result;
}

bool
ProgressiveSource::Read (void* buffer, guint32 size)
{
	bool result = false;
	
	if (file == 0) {
		media->AddMessage (MEDIA_INVALID_ARGUMENT, "File has not been opened.");
		return false;
	}
	
	Lock ();
	
	// Check if requested more data than what has been downloaded
	uint64_t end = GetPosition () + size;
	//printf ("ProgressiveSource::Read (%.8p, %.4u): end: %.6llu, write_position: %.7llu, bytes before stop: %llu\n", buffer, size, end, write_position, write_position - end);
	if (end > write_position) {
		//printf ("ProgressiveSource::Read (%p, %u): Read beyond written size (%lld)\n", buffer, size, write_position);
		Unlock ();
		if (!(WaitForPosition (end))) {
			return false;
		}
		Lock ();		
	}
	
	result = read (file, buffer, size) == (ssize_t) size; 
	
	Unlock ();
	return result;
}

bool
ProgressiveSource::Peek (void* buffer, guint32 size)
{
	// Simple implementation of peek: save position, read bytes, restore position.
	uint64_t position = GetPosition ();
	
	if (!Read (buffer, size))
		return false;
	
	return Seek (position, SEEK_SET);
}

uint64_t
ProgressiveSource::GetPosition ()
{
	return lseek (file, 0, SEEK_CUR);
}

bool
ProgressiveSource::Eof ()
{
	printf ("ProgressiveSource::Eof ().\n");
	printf ("ProgressiveSource::Eof (): Not implemented.\n");
	return false;
}

uint64_t
ProgressiveSource::GetWritePosition ()
{
	int64_t result;
	Lock ();
	result = write_position;
	Unlock ();
	return result;
}




