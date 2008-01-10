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

#define MAKE_CODEC_ID(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

#define CODEC_WMV1	MAKE_CODEC_ID ('W', 'M', 'V', '1')
#define CODEC_WMV2	MAKE_CODEC_ID ('W', 'M', 'V', '2')
#define CODEC_WMV3	MAKE_CODEC_ID ('W', 'M', 'V', '3')
#define CODEC_WMVA	MAKE_CODEC_ID ('W', 'M', 'V', 'A')
#define CODEC_WVC1	MAKE_CODEC_ID ('W', 'V', 'C', '1')
#define CODEC_MP3	0x55
#define CODEC_WMAV1 0x160
#define CODEC_WMAV2 0x161
					
Media::Media (void* el) :
		source (NULL), demuxer (NULL), element (NULL),
		file_or_url (NULL), queued_requests (NULL),
		queue_closure (NULL)
{
	//if (element)
	//	element->ref ();
}

Media::~Media ()
{
	DeleteQueue ();
	//if (element)
	//	element->unref ();
		
	element = NULL;
	delete source;
	source = NULL;
	delete demuxer;
	demuxer = NULL;
	g_free (file_or_url);
	file_or_url = NULL;
	
	delete queue_closure;
	queue_closure = NULL;
}

void
Media::AddMessage (MediaResult result, const char* msg)
{
	printf ("Media::AddMessage (%i, '%s').\n", result, msg);
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
	
	MediaResult result = MEDIA_FAIL;

	if (source == NULL) {
		return MediaResult (MEDIA_INVALID_ARGUMENT);
	}
	
	// Determine the container format
	// Hard-code our own demuxer for the moment.
	demuxer = new ASFDemuxer (this);
	result = demuxer->ReadHeader ();
	
	if (!MEDIA_SUCCEEDED (result)) {
		return result;
	}
	
	printf ("Media::Open (): Found %i streams in this source.\n", demuxer->GetStreamCount ());
	
	printf ("Media::Open (): Starting to select codecs...\n");
	
	// Select codecs for each stream
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream* stream = demuxer->GetStream (i);
		if (stream == NULL)
			return MEDIA_INVALID_STREAM;
		
		const char* codec = stream->GetCodec ();
		IMediaDecoder* decoder = NULL;
		
		printf ("Media::Open (%p): Selecting codec for codec %s, id %i.\n", source, codec, stream->codec_id);
				
		if (decoder == NULL) {
			if (strcmp (codec, "wmv") == 0
				|| strcmp (codec, "wma") == 0
				|| strcmp (codec, "mp3") == 0
				|| strcmp (codec, "vc1") == 0
				|| strcmp (codec, "wmav2") == 0 
				) {
				decoder = new FfmpegDecoder (this, stream);
			} else if (strcmp (codec, "ASFMARKER") == 0) {
				decoder = new ASFMarkerDecoder (this, stream);
			} else {
				AddMessage (MEDIA_UNKNOWN_CODEC, g_strdup_printf ("Unknown codec: '%s'", codec));
			}
		}
		
		if (decoder != NULL) {
			result = decoder->Open ();
			if (!MEDIA_SUCCEEDED (result)) {
				delete decoder;
				decoder = NULL;
			}
		
		
		if (decoder != NULL) {
			stream->SetDecoder (decoder);
			
			if (stream->GetType () == MEDIA_VIDEO) {
				VideoStream* vs = (VideoStream*) stream;
				IImageConverter* converter;
				converter = new FfmpegConverter (this, vs);
				converter->input_format = vs->decoder->pixel_format;
				converter->output_format = PIXEL_FORMAT_RGB32;
				if (MEDIA_SUCCEEDED (converter->Open ())) {
					// FIXME: What should happen if there's no available converter? No video?
					vs->converter = converter;
				} else {
					delete converter;
					converter = NULL;
				}
			}
		}}
	}
	
	return MEDIA_SUCCESS;
}

MediaFrame*
Media::GetNextFrame (IMediaStream* stream)
{
	//printf ("Media::GetNextFrame (%p).\n", stream);
	
	MediaResult result = MEDIA_SUCCESS;
	MediaFrame* frame = new MediaFrame ();
	frame->stream = stream;
	
	result = demuxer->ReadFrame (frame);
	if (!MEDIA_SUCCEEDED (result)) {
		delete frame;
		return NULL;
	}
	
	result = stream->decoder->DecodeFrame (frame);
	if (!MEDIA_SUCCEEDED (result)) {
		delete frame;
		return NULL;
	}
	
	//printf ("Media::GetNextFrame (%p) finished, size: %i.\n", stream, frame->uncompressed_size);
	
	return frame;
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
			while (current != NULL && current->stream->GetType () != MEDIA_AUDIO) {
				current = (Media::Node*) current->next;
			}
			if (current != NULL && current->stream->GetType () == MEDIA_AUDIO) {
				node = current;
			} else {
				// No audio node, just get the first node
				node = (Media::Node*) queued_requests->First ();
			}
			queued_requests->Unlink (node);
			printf ("Media::FrameReaderLoop (): got a %s node, there are %i nodes left.\n", node->stream->GetType () == MEDIA_AUDIO ? "audio" : (node->stream->GetType () == MEDIA_VIDEO ? "video" : "unknown") , queued_requests->Length ());
		}
		pthread_mutex_unlock (&queue_mutex);
		
		if (node == NULL)
			continue; // Found nothing, continue waiting.
		
		LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): processing node.\n");
		
		// Now demux and decode what we found and send it to who asked for it
		MediaFrame* frame = GetNextFrame (node->stream);
		if (frame != NULL) {
			MediaClosure *closure = new MediaClosure ();
			memcpy (closure, queue_closure, sizeof (MediaClosure));
			closure->frame = frame;
			closure->Call ();
			delete closure;
		}
		delete node;
	}
	LOG_FRAMEREADERLOOP ("Media::FrameReaderLoop (): exiting.\n");
}
#include <sched.h>

void
Media::GetNextFrameAsync (IMediaStream* stream)
{
	//printf ("Media::GetNextFrameAsync (%p).\n", stream);
	if (queued_requests == NULL) {
		printf ("Media::GetNextFrameAsync (%p). Creating threads.\n", stream);
		queued_requests = new List ();
		int result;
		pthread_attr_t attribs;
		result = pthread_attr_init (&attribs);
		result = pthread_attr_setdetachstate (&attribs, PTHREAD_CREATE_JOINABLE);
		result = pthread_mutex_init (&queue_mutex, NULL);
		result = pthread_cond_init (&queue_condition, NULL);
		result = pthread_create (&queue_thread, &attribs, FrameReaderLoop, this); 	
		result = pthread_attr_destroy (&attribs);
		
		int policy = 2;//sched_getscheduler (0);
		int max = sched_get_priority_max (policy);
		int min = sched_get_priority_min (policy);
		printf ("Media::GetNextFrameAsync (%p). Policy: %i, max: %i, min: %i, fifi: %i, rr: %i, sporadic: %i, other: %i\n", stream, policy, max, min, SCHED_FIFO, SCHED_RR, -1, SCHED_OTHER);
		
	}
	
	pthread_mutex_lock (&queue_mutex);
	
	// Add another node to our queue.
	Media::Node* node = new Media::Node ();
	node->stream = stream;
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
			ASFMarkerStream* marker = new ASFMarkerStream (GetMedia ());
			stream = marker;
		} else {
			// Unknown stream, ignore it.
		}
		
		if (stream != NULL) {
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
		media->AddMessage (MEDIA_DEMUXER_ERROR, "Error while advancing to the next frame.");
		return MEDIA_DEMUXER_ERROR;
	}
	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	frame->compressed_size = reader->Size ();
	frame->compressed_data = g_malloc (reader->Size () + frame->stream->min_padding + 100);
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
	
	//printf ("ASFDemuxer::ReadFrame (%p) frame = ", frame);
	//frame->printf ();
	//printf ("\n");
	
	return MEDIA_SUCCESS;
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
	printf ("FileSource::Seek (%p, %u).\n", buffer, size);
	printf ("FileSource::Peek (%p, %u): Not implemented.\n", buffer, size);
	return false;
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
FileSource::Seek (gint64 position)
{
	//printf ("FileSource::Seek (%llu).\n", position);
	
	return Seek (position, SEEK_CUR);
}

bool
FileSource::Seek (gint64 position, int mode)
{
	//printf ("FileSource::Seek (%llu, %i).\n", position, mode);
	
	if (fseek (fd, position, mode) != 0) {
		media->AddMessage (MEDIA_SEEK_ERROR, g_strdup_printf ("Can't seek to offset %llu with mode %i in '%s': %s.\n", position, mode,  filename, strerror (errno)));
		return false;
	}
	return true;
}

/*
 * MediaClosure
 */ 
 
MediaClosure::~MediaClosure ()
{
	delete frame;
	frame = NULL;
}

/*
 * IMediaStream
 */

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
 
MediaFrame::~MediaFrame ()
{
	g_free (compressed_data);
	compressed_data = NULL;
	g_free (uncompressed_data);
	uncompressed_data = NULL;
	
	if (decoder_specific_data != NULL) {
		stream->decoder->Cleanup (this);
	}
}
