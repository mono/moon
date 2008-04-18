/*
 * pipeline.cpp: Pipeline for the media
 *
 * Authors: Rolf Bjarne Kvinge (RKvinge@novell.com)
 *          Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#include <pthread.h>
#include <sched.h>

#include "pipeline.h"
#include "pipeline-ffmpeg.h"
#include "uri.h"
#include "media.h"
#include "asf/asf.h"
#include "asf/asf-structures.h"
#include "yuv-converter.h"
#include "runtime.h"


#define MAKE_CODEC_ID(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

#define CODEC_WMV1	MAKE_CODEC_ID ('W', 'M', 'V', '1')
#define CODEC_WMV2	MAKE_CODEC_ID ('W', 'M', 'V', '2')
#define CODEC_WMV3	MAKE_CODEC_ID ('W', 'M', 'V', '3')
#define CODEC_WMVA	MAKE_CODEC_ID ('W', 'M', 'V', 'A')
#define CODEC_WVC1	MAKE_CODEC_ID ('W', 'V', 'C', '1')
#define CODEC_MP3	0x55
#define CODEC_WMAV1 0x160
#define CODEC_WMAV2 0x161

#define LOG_PIPELINE(...)// printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR(...) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR_CONDITIONAL(x, ...) if (x) printf (__VA_ARGS__);
#define LOG_FRAMEREADERLOOP(...)// printf (__VA_ARGS__);

class MediaNode : public List::Node {
public:
	Media *media;
	MediaNode (Media *media)
	{
		this->media = media;
	}
};

/*
 * Media 
 */
DemuxerInfo *Media::registered_demuxers = NULL;
DecoderInfo *Media::registered_decoders = NULL;
ConverterInfo *Media::registered_converters = NULL;
Queue *Media::media_objects = NULL;

Media::Media (MediaElement *element)
{
	LOG_PIPELINE ("Media::Media (%p <id:%i>), id: %i\n", element, GET_OBJ_ID (element), GET_OBJ_ID (this));

	// Add ourselves to the global list of medias
	media_objects->Push (new MediaNode (this));

	pthread_attr_t attribs;
	
	this->element = element;
	this->SetSurface (element->GetSurface ());

	queued_requests = new List ();
	
	file_or_url = NULL;
	source = NULL;
	
	demuxer = NULL;
	markers = NULL;
	
	opened = false;
	stopping = false;
	stopped = false;
	
	pthread_attr_init (&attribs);
	pthread_attr_setdetachstate (&attribs, PTHREAD_CREATE_JOINABLE);
	
	pthread_mutex_init (&queue_mutex, NULL);
	pthread_cond_init (&queue_condition, NULL);
	
	pthread_create (&queue_thread, &attribs, WorkerLoop, this); 	
	pthread_attr_destroy (&attribs);
}

Media::~Media ()
{
	MediaNode *node;

	LOG_PIPELINE ("Media::~Media (), id: %i\n", GET_OBJ_ID (this));

	pthread_mutex_lock (&queue_mutex);
	queued_requests->Clear (true);
	delete queued_requests;
	queued_requests = NULL;
	pthread_cond_signal (&queue_condition);
	pthread_mutex_unlock (&queue_mutex);
	
	if (source)
		source->Abort ();

	if (!stopped)
		pthread_join (queue_thread, NULL);
	pthread_mutex_destroy (&queue_mutex);
	pthread_cond_destroy (&queue_condition);
	pthread_detach (queue_thread);
	
	g_free (file_or_url);
	if (source)
		source->unref ();
	if (demuxer)
		demuxer->unref ();
	delete markers;

	// Remove ourselves from the global list of medias
	// media_objects might be NULL if Media::Shutdown has been called already
	if (media_objects) {
		media_objects->Lock ();
		node = (MediaNode *) media_objects->LinkedList ()->First ();
		while (node != NULL) {
			if (node->media == this) {
				media_objects->LinkedList ()->Remove (node);
				break;
			}
			node = (MediaNode *) node->next;
		}
		media_objects->Unlock ();
	}
}

void
Media::SetSource (IMediaSource *source)
{
	LOG_PIPELINE ("Media::SetSource (%p <id:%i>)\n", source, GET_OBJ_ID (source));

	if (this->source)
		this->source->unref ();
	this->source = source;
	if (this->source)
		this->source->ref ();
}

IMediaSource *
Media::GetSource ()
{
	return source;
}

void
Media::SetFileOrUrl (const char *value)
{
	LOG_PIPELINE ("Media::SetFileOrUrl ('%s')\n", value);

	if (file_or_url)
		g_free (file_or_url);
	file_or_url = g_strdup (value);
}

List * 
Media::GetMarkers ()
{
	if (markers == NULL)
		markers = new List ();
	
	return markers;
}

void
Media::RegisterDemuxer (DemuxerInfo *info)
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
Media::RegisterConverter (ConverterInfo *info)
{
	//printf ("Media::RegisterConverter (%p)\n", info);
	info->next = NULL;
	if (registered_converters == NULL) {
		registered_converters = info;
	} else {
		MediaInfo *current = registered_converters;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
}

void
Media::RegisterDecoder (DecoderInfo *info)
{
	//printf ("Media::RegisterDecoder (%p)\n", info);
	info->next = NULL;
	if (registered_decoders == NULL) {
		registered_decoders = info;
	} else {
		MediaInfo *current = registered_decoders;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
}

void
Media::Initialize ()
{
	LOG_PIPELINE ("Media::Initialize ()\n");
	
	media_objects = new Queue ();	
	
	// register stuff
	Media::RegisterDemuxer (new ASFDemuxerInfo ());
	Media::RegisterDemuxer (new Mp3DemuxerInfo ());
	Media::RegisterDemuxer (new ASXDemuxerInfo ());
	if (!(moonlight_flags & RUNTIME_INIT_FFMPEG_YUV_CONVERTER))
		Media::RegisterConverter (new YUVConverterInfo ());
#ifdef INCLUDE_FFMPEG
	register_ffmpeg ();
#else
	Media::RegisterDecoder (new NullMp3DecoderInfo ());
#endif
	
	AudioPlayer::Initialize ();
}

void
Media::Shutdown ()
{
	LOG_PIPELINE ("Media::Shutdown ()\n");

	MediaInfo *current;
	MediaInfo *next;
	MediaNode *node;
	
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

	// Make sure all threads are stopped
	AudioPlayer::Shutdown ();

	media_objects->Lock ();
	node = (MediaNode *) media_objects->LinkedList ()->First ();
	while (node != NULL) {
		node->media->StopThread ();
		node = (MediaNode *) node->next;
	}
	
	media_objects->Unlock ();

	delete media_objects;
	media_objects = NULL;

	LOG_PIPELINE ("Media::Shutdown () [Done]\n");
}

void
Media::AddMessage (MediaResult result, const char *msg)
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

void
Media::AddError (MediaErrorEventArgs *args)
{
	LOG_PIPELINE ("Media::AddError (%p), message: %s, code: %i\n", args, args->error_message, args->error_code);

	//TODO: We should probably reaise MediaFailed when errors occur,
	// but it will need some testing to see what MS does (especially
	// with corrupt media during playback).
	//
	//if (element) {
	//	element->MediaFailed (args);
	//} else {
		fprintf (stderr, "Media error: %s\n", args->error_message);
	//}
}

MediaResult
Media::Seek (uint64_t pts)
{
	if (demuxer)
		return demuxer->Seek (pts);
	
	return MEDIA_FAIL;
}

MediaResult
Media::SeekAsync (uint64_t pts, MediaClosure *closure)
{
	LOG_PIPELINE ("Media::SeekAsync (%llu, %p), id: %i\n", pts, closure, GET_OBJ_ID (this));

	if (demuxer == NULL)
		return MEDIA_FAIL;
	
	EnqueueWork (new MediaWork (closure, pts));
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::Initialize (const char *file_or_url)
{
	LOG_PIPELINE ("Media::Initialize ('%s'), id: %i\n", file_or_url, GET_OBJ_ID (this));
	
	Uri* uri = new Uri ();
	MediaResult result = MEDIA_FAIL;
	SetSource (NULL);
	
	this->file_or_url = g_strdup (file_or_url);
	
	if (uri->Parse (file_or_url)) {
		LOG_PIPELINE ("Media::Open ('%s'): uri parsing succeeded, protocol: '%s'.\n", file_or_url, uri->protocol);
		if (uri->protocol == NULL) {
			result = MEDIA_INVALID_PROTOCOL;
/*
		} else if (strcmp (uri->protocol, "mms") == 0) {
			source = new LiveSource (this);
			result = source->Initialize ();
			if (!MEDIA_SUCCEEDED (result)) {
				LOG_PIPELINE ("Media::Open ('%s'): live source failed, trying progressive source.\n", file_or_url);
				source->unref ();
				source = new ProgressiveSource (this, true);
				result = source->Initialize ();
			}
*/
		} else if (strcmp (uri->protocol, "http") == 0 || strcmp (uri->protocol, "https") == 0) {
			source = new ProgressiveSource (this, false);
			result = source->Initialize ();
/*
			if (!MEDIA_SUCCEEDED (result)) {
				LOG_PIPELINE ("Media::Open ('%s'): progressive source failed, trying live source.\n", file_or_url);
				source->unref ();
				source = new LiveSource (this);
				result = source->Initialize ();
			}
*/
		} else if (strcmp (uri->protocol, "file") == 0) {
			source = new FileSource (this),
			result = source->Initialize ();
			if (!MEDIA_SUCCEEDED (result)) {
				LOG_PIPELINE ("Media::Open ('%s'): file source failed.\n", file_or_url);
			}
		} else {
			result = MEDIA_INVALID_PROTOCOL;
		}
	} else {
		// FIXME: Is it safe to assume that if the path cannot be parsed as an uri it is a filename?
		LOG_PIPELINE ("Media::Open ('%s'): uri parsing failed, assuming source is a filename.\n", file_or_url);
		source = new FileSource (this);	
		result = source->Initialize ();
	}
	
	delete uri;
	
	if (!MEDIA_SUCCEEDED (result)) {
		LOG_PIPELINE ("Media::Open ('%s'): failed, result: %i.\n", file_or_url, result);
		source->unref ();
		source = NULL;
	} else {
		LOG_PIPELINE ("Media::Open ('%s'): succeeded.\n", file_or_url);
	}
	
	return result;
}

MediaResult
Media::Open ()
{
	LOG_PIPELINE ("Media::Open (), id: %i\n", GET_OBJ_ID (this));

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
Media::OpenAsync (IMediaSource *source, MediaClosure *closure)
{
	LOG_PIPELINE ("Media::OpenAsync (%p <id:%i>, %p), id: %i\n", source, GET_OBJ_ID (source), closure, GET_OBJ_ID (this));

	closure->SetMedia (this);

	EnqueueWork (new MediaWork (closure, source));
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::Open (IMediaSource *source)
{
	LOG_PIPELINE ("Media::Open (%p <id:%i>), id: %i\n", source, GET_OBJ_ID (source), GET_OBJ_ID (this));

	MediaResult result;
	
	LOG_PIPELINE ("Media::Open ().\n");
	
	if (source == NULL || IsOpened ()) // Initialize wasn't called (or didn't succeed) or already open.
		return MEDIA_INVALID_ARGUMENT;
	
	// Select a demuxer
	DemuxerInfo *demuxerInfo = registered_demuxers;
	while (demuxerInfo != NULL) {
		if (demuxerInfo->Supports (source))
			break;
		
		LOG_PIPELINE ("Media::Open (): '%s' can't handle this media.\n", demuxerInfo->GetName ());
		demuxerInfo = (DemuxerInfo *) demuxerInfo->next;
	}
	
	if (demuxerInfo == NULL) {
		const char *source_name = file_or_url;
		
		if (!source_name) {
			switch (source->GetType ()) {
			case MediaSourceTypeProgressive:
			case MediaSourceTypeFile:
				source_name = ((FileSource *) source)->GetFileName ();
				break;
			case MediaSourceTypeLive:
				source_name = "live source";
				break;
			default:
				source_name = "unknown source";
				break;
			}
		}
		
		AddMessage (MEDIA_UNKNOWN_MEDIA_TYPE,
			    g_strdup_printf ("No demuxers registered to handle the media source `%s'.",
					     source_name));
		return MEDIA_UNKNOWN_MEDIA_TYPE;
	}
	
	// Found a demuxer
	demuxer = demuxerInfo->Create (this, source);
	result = demuxer->ReadHeader ();
	
	if (!MEDIA_SUCCEEDED (result))
		return result;
	
	LOG_PIPELINE ("Media::Open (): Found %i streams in this source.\n", demuxer->GetStreamCount ());
	
	LOG_PIPELINE ("Media::Open (): Starting to select codecs...\n");
	
	// If the demuxer has no streams (ASXDemuxer for instance)
	// then just return success.
	if (demuxer->GetStreamCount () == 0)
		return result;

	result = MEDIA_FAIL; // Only set to SUCCESS if at least 1 stream can be used
	
	// Select codecs for each stream
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		if (stream == NULL)
			return MEDIA_INVALID_STREAM;
		
		const char *codec = stream->GetCodec ();
		IMediaDecoder *decoder = NULL;
		
		LOG_PIPELINE ("Media::Open (): Selecting codec for codec %s, id %i.\n", codec, stream->codec_id);
		
		DecoderInfo *current_decoder = registered_decoders;
		while (current_decoder != NULL && !current_decoder->Supports (codec)) {
			LOG_PIPELINE ("Checking registered decoder '%s' if it supports codec '%s': no.\n",
				current_decoder->GetName (), codec);
			current_decoder = (DecoderInfo*) current_decoder->next;
		}

		if (current_decoder == NULL) {
			AddMessage (MEDIA_UNKNOWN_CODEC, g_strdup_printf ("Unknown codec: '%s'.", codec));	
		} else {
			LOG_PIPELINE ("Checking registered decoder '%s' if it supports codec '%s': yes.\n",
				current_decoder->GetName (), codec);
			decoder = current_decoder->Create (this, stream);
		}
		
		if (decoder != NULL) {
			result = decoder->Open ();
			if (!MEDIA_SUCCEEDED (result)) {
				decoder->unref ();
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
					LOG_PIPELINE ("Checking whether '%s' supports input '%d' and output '%d': no.\n",
						current_conv->GetName (), decoder->pixel_format, MoonPixelFormatRGB32);
					current_conv = (ConverterInfo*) current_conv->next;
				}
				
				if (current_conv == NULL) {
					AddMessage (MEDIA_UNKNOWN_CONVERTER,
						    g_strdup_printf ("Can't convert from %d to %d: No converter found.",
								     decoder->pixel_format, MoonPixelFormatRGB32));	
				} else {
					LOG_PIPELINE ("Checking whether '%s' supports input '%d' and output '%d': yes.\n",
						current_conv->GetName (), decoder->pixel_format, MoonPixelFormatRGB32);
					converter = current_conv->Create (this, vs);
					converter->input_format = decoder->pixel_format;
					converter->output_format = MoonPixelFormatRGB32;
					if (!MEDIA_SUCCEEDED (converter->Open ())) {
						converter->unref ();
						converter = NULL;
					}
				}
				
				if (converter != NULL) {
					vs->converter = converter;
				} else {
					decoder->unref ();
					decoder = NULL;
				}
			}
		}
		
		if (decoder != NULL) {
			stream->SetDecoder (decoder);
			result = MEDIA_SUCCESS;
		}
	}
	
	if (result == MEDIA_SUCCESS) {
		SetSource (source);
		opened = true;
	}
	
	LOG_PIPELINE ("Media::Open (): result = %s\n", MEDIA_SUCCEEDED (result) ? "true" : "false");

	return result;
}

MediaResult
Media::GetNextFrame (MediaWork *work)
{
	MediaFrame *frame = NULL;
	uint16_t states= work->data.frame.states;	
	IMediaStream *stream = work->data.frame.stream;
	MediaResult result = MEDIA_SUCCESS;
	
	//printf ("Media::GetNextFrame (%p).\n", stream);
	
	if (work == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "work is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if (stream == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "work->stream is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if ((states & FRAME_DEMUXED) != FRAME_DEMUXED)
		return result; // Nothing to do?

	do {
		if (frame) {
			LOG_PIPELINE ("Media::GetNextFrame (): delayed a frame\n");
			delete frame;
		}
		frame = new MediaFrame (stream);
		frame->AddState (states);

		// TODO: get the last frame out of delayed codecs (when the demuxer returns NO_MORE_DATA)

		result = demuxer->ReadFrame (frame);
		if (!MEDIA_SUCCEEDED (result))
			break;
		
		if ((states & FRAME_DECODED) != FRAME_DECODED) {
			// We weren't requested to decode the frame
			// This might cause some errors on delayed codecs (such as the wmv ones).
			break;
		}
		
		if (frame->event != 0)
			break;
	
		result = stream->decoder->DecodeFrame (frame);
	} while (result == MEDIA_CODEC_DELAYED);

	work->closure->frame = frame;

	//printf ("Media::GetNextFrame (%p) finished, size: %i.\n", stream, frame->buflen);
	
	return result;
}

void * 
Media::WorkerLoop (void *data)
{
	Media *media = (Media *) data;
	
	media->WorkerLoop ();
	
	return NULL;
}

void
Media::WorkerLoop ()
{
	MediaResult result;
	
	LOG_PIPELINE ("Media::WorkerLoop ().\n");

	while (queued_requests != NULL && !stopping) {
		MediaWork *node = NULL;
		
		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): entering mutex.\n");
		
		// Wait until we have something in the queue
		pthread_mutex_lock (&queue_mutex);
		while (queued_requests != NULL && !stopping && queued_requests->IsEmpty ())
			pthread_cond_wait (&queue_condition, &queue_mutex);
		
		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): got something.\n");
		
		if (queued_requests != NULL) {
			// Find the first audio node
			node = (MediaWork *) queued_requests->First ();
			
			if (node != NULL)
				queued_requests->Unlink (node);
			
			LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): got a node, there are %d nodes left.\n",
					     queued_requests->Length ());
		}
		
		pthread_mutex_unlock (&queue_mutex);
		
		if (node == NULL)
			continue; // Found nothing, continue waiting.
		
		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): processing node %p with type %i.\n", node, node->type);
		
		switch (node->type) {
		case WorkTypeSeek:
			//printf ("Media::WorkerLoop (): Seeking, current count: %d\n", queued_requests->Length ());
			result = Seek (node->data.seek.seek_pts);
			break;
		case WorkTypeAudio:
		case WorkTypeVideo:
		case WorkTypeMarker:
			// Now demux and decode what we found and send it to who asked for it
			result = GetNextFrame (node);
			break;
		case WorkTypeOpen:
			result = Open (node->data.open.source);
			break;
		}
		
		node->closure->result = result;
		node->closure->Call ();

		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): processed node %p with type %i, result: %i.\n", node, node->type, result);

		delete node;
	}
	
	stopped = true;

	LOG_PIPELINE ("Media::WorkerLoop (): exiting.\n");
}

void
Media::GetNextFrameAsync (MediaClosure *closure, IMediaStream *stream, uint16_t states)
{
	MoonWorkType type;
	
	if (stream == NULL) {
		AddMessage (MEDIA_INVALID_ARGUMENT, "stream is NULL.");
		return;
	}
	
	switch (stream->GetType ()) {
	case MediaTypeAudio:
		type = WorkTypeAudio;
		break;
	case MediaTypeVideo:
		type = WorkTypeVideo;
		break;
	case MediaTypeMarker:
		type = WorkTypeMarker;
		break;
	case MediaTypeNone:
	default:
		AddMessage (MEDIA_INVALID_ARGUMENT, "The frame's stream is of an unknown type.");
		return;
	}
	
	EnqueueWork (new MediaWork (closure, stream, states));
}

void
Media::EnqueueWork (MediaWork *work)
{
	MediaWork *current;
	
	//printf ("Media::EnqueueWork (%p).\n", stream);
	
	pthread_mutex_lock (&queue_mutex);
	
	if (queued_requests->First ()) {
		switch (work->type) {
		case WorkTypeSeek:
			// Only have one seek request in the queue, and make
			// sure to have it first.
			current = (MediaWork *) queued_requests->First ();
			if (current->type == WorkTypeSeek)
				queued_requests->Remove (current);
			
			queued_requests->Prepend (work);
			break;
		case WorkTypeAudio:
		case WorkTypeVideo:
		case WorkTypeMarker:
			// Insert the work just before work with less priority.
			current = (MediaWork *) queued_requests->First ();
			while (current != NULL && work->type >= current->type)
				current = (MediaWork *) current->next;
			
			if (current)
				queued_requests->InsertBefore (work, current);
			else
				queued_requests->Append (work);
			break;
		case WorkTypeOpen:
			queued_requests->Prepend (work);
			break;
		}
	} else {
		queued_requests->Append (work);
	}
	
	//printf ("Media::EnqueueWork (%p), count: %i\n", work, queued_requests->Length ());

	pthread_cond_signal (&queue_condition);
	
	pthread_mutex_unlock (&queue_mutex);
}

void
Media::ClearQueue ()
{
	LOG_PIPELINE ("Media::ClearQueue ().\n");
	if (queued_requests != NULL) {
		pthread_mutex_lock (&queue_mutex);
		queued_requests->Clear (true);
		pthread_mutex_unlock (&queue_mutex);
	}
	
	if (source)
		source->Abort ();
}

void
Media::StopThread ()
{
	LOG_PIPELINE ("Media::ClearQueue ().\n");

	if (stopped)
		return;

	stopping = true;
	ClearQueue ();
	pthread_mutex_lock (&queue_mutex);
	pthread_cond_signal (&queue_condition);
	pthread_mutex_unlock (&queue_mutex);
	pthread_join (queue_thread, NULL);

	LOG_PIPELINE ("Media::ClearQueue () [Done]\n");
}

/*
 * ASFDemuxer
 */

ASFDemuxer::ASFDemuxer (Media *media, IMediaSource *source) : IMediaDemuxer (media, source)
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
uint64_t 
ASFDemuxer::GetLastAvailablePts ()
{
	if (reader == NULL)
		return 0;

	return reader->GetLastAvailablePts ();
}

int64_t
ASFDemuxer::EstimatePtsPosition (uint64_t pts)
{
	int64_t result = -1;
	
	for (int i = 0; i < GetStreamCount (); i++) {
		if (!GetStream (i)->GetSelected ())
			continue;

		result = MAX (result, reader->GetFrameReader (stream_to_asf_index [i])->EstimatePtsPosition(pts));
	}
	
	return result;
}

void
ASFDemuxer::UpdateSelected (IMediaStream *stream)
{
	if (reader)
		reader->SelectStream (stream_to_asf_index [stream->index], stream->GetSelected ());

	IMediaDemuxer::UpdateSelected (stream);
}

MediaResult
ASFDemuxer::Seek (uint64_t pts)
{
	//printf ("ASFDemuxer::Seek (%llu)\n", pts);
	
	if (reader == NULL)
		return MEDIA_FAIL;
	
	return reader->Seek (pts) ? MEDIA_SUCCESS : MEDIA_FAIL;
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
	const char *type;
	uint64_t pts;
	uint64_t preroll_pts = MilliSeconds_ToPts (parser->GetFileProperties ()->preroll);
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
			
			markers->Append (new MediaMarker::Node (new MediaMarker (type, text, pts)));
			
			//printf ("MediaElement::ReadMarkers () Added script command at %llu (text: %s, type: %s)\n", pts, text, type);
			
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
	ASFParser *asf_parser = new ASFParser (source, media);
	int32_t *stream_to_asf_index = NULL;
	IMediaStream **streams = NULL;
	int current_stream = 1;
	int stream_count = 0;
	
	//printf ("ASFDemuxer::ReadHeader ().\n");
	
	if (!asf_parser->ReadHeader ()) {
		result = MEDIA_INVALID_MEDIA;
		GetMedia ()->AddMessage (MEDIA_INVALID_MEDIA, "asf_parser->ReadHeader () failed:");
		GetMedia ()->AddMessage (MEDIA_FAIL, asf_parser->GetLastErrorStr ());
		goto failure;
	}
	
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
		
		const asf_stream_properties* stream_properties = asf_parser->GetStream (current_stream);
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
	delete asf_parser;
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

MediaResult
ASFDemuxer::ReadFrame (MediaFrame *frame)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	ASFFrameReader *reader = this->reader->GetFrameReader (stream_to_asf_index [frame->stream->index]);
	MediaResult result;
	
	result = reader->Advance ();
	if (result == MEDIA_NO_MORE_DATA) {
		media->AddMessage (MEDIA_NO_MORE_DATA, "Reached end of data.");
		frame->event = FrameEventEOF;
		return MEDIA_NO_MORE_DATA;
	}
	
	if (!MEDIA_SUCCEEDED (result)) {
		media->AddMessage (MEDIA_DEMUXER_ERROR, g_strdup_printf ("Error while advancing to the next frame (%i)", result));
		return result;
	}
	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	if (reader->IsKeyFrame ())
		frame->AddState (FRAME_KEYFRAME);
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
	
	return MEDIA_SUCCESS;
}

/*
 * ASFDemuxerInfo
 */

bool
ASFDemuxerInfo::Supports (IMediaSource *source)
{
	uint8_t buffer[16];
	
#if DEBUG
	if (!source->GetPosition () == 0) {
		fprintf (stderr, "ASFDemuxerInfo::Supports (%p): Trying to check if a media is supported, but the media isn't at position 0 (it's at position %lld)\n", source, source->GetPosition ());
	}
#endif

	if (!source->Peek (buffer, 16))
		return false;
	
	bool result = asf_guid_compare (&asf_guids_header, (asf_guid *) buffer);
	
	//printf ("ASFDemuxerInfo::Supports (%p): probing result: %s\n", source,
	//	result ? "true" : "false");
	
	return result;
}

IMediaDemuxer *
ASFDemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new ASFDemuxer (media, source);
}


/*
 * ASXDemuxer
 */

ASXDemuxer::ASXDemuxer (Media *media, IMediaSource *source) : IMediaDemuxer (media, source)
{
	playlist = NULL;
}

ASXDemuxer::~ASXDemuxer ()
{
	if (playlist)
		playlist->unref ();
}

MediaResult
ASXDemuxer::ReadHeader ()
{
	MediaResult result;

	PlaylistParser *parser = new PlaylistParser (media->GetElement (), source);

	if (parser->Parse ()) {
		result = MEDIA_SUCCESS;
		playlist = parser->GetPlaylist ();
		playlist->ref ();
	} else {
		result = MEDIA_FAIL;
	}

	delete parser;

	return result;
}

/*
 * ASXDemuxerInfo
 */

bool
ASXDemuxerInfo::Supports (IMediaSource *source)
{
	uint8_t buffer[4];
	
	if (!source->Peek (buffer, 4))
		return false;
	
	return (buffer [0] == '<' && 
			(buffer [1] == 'A' || buffer [1] == 'a') && 
			(buffer [2] == 'S' || buffer [2] == 's') &&
			(buffer [3] == 'X' || buffer [3] == 'x') ||
		(buffer [0] == '[' && 
			 buffer [1] == 'R' && buffer [2] == 'e' && buffer [3] == 'f'));
}

IMediaDemuxer *
ASXDemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new ASXDemuxer (media, source);
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

enum MpegVBRHeaderType {
	MpegNoVBRHeader,
	MpegXingHeader,
	MpegVBRIHeader
};

struct MpegVBRHeader {
	MpegVBRHeaderType type;
	uint32_t nframes;
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

static bool
mpeg_parse_bitrate (MpegFrameHeader *mpeg, uint8_t byte)
{
	int i = (byte & 0xf0) >> 4;
	
	if (i > 14)
		return false;
	
	if (mpeg->version == 1)
		mpeg->bit_rate = mpeg1_bitrates[mpeg->layer - 1][i];
	else
		mpeg->bit_rate = mpeg2_bitrates[mpeg->layer - 1][i];
	
	return true;
}

static uint8_t
mpeg_encode_bitrate (MpegFrameHeader *mpeg, int bit_rate)
{
	int i;
	
	if (mpeg->version == 1) {
		for (i = 1; i < 15; i++) {
			if (mpeg1_bitrates[mpeg->layer - 1][i] == bit_rate)
				break;
		}
	} else {
		for (i = 1; i < 15; i++) {
			if (mpeg2_bitrates[mpeg->layer - 1][i] == bit_rate)
				break;
		}
	}
	
	if (i == 15)
		return 0;
	
	return ((i << 4) & 0xf0);
}

static int mpeg_samplerates[3][3] = {
	{ 44100, 48000, 32000 },  // version 1
	{ 22050, 24000, 16000 },  // version 2
	{ 11025, 12000,  8000 }   // version 2.5
};

static bool
mpeg_parse_samplerate (MpegFrameHeader *mpeg, uint8_t byte)
{
	int i = (byte >> 2) & 0x03;
	
	if (i > 2)
		return false;
	
	mpeg->sample_rate = mpeg_samplerates[mpeg->version - 1][i];
	
	return true;
}

static bool
mpeg_parse_channels (MpegFrameHeader *mpeg, uint8_t byte)
{
	int mode = (byte >> 6) & 0x03;
	
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
	
	return true;
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


static bool
mpeg_parse_header (MpegFrameHeader *mpeg, const uint8_t *buffer)
{
	if (!is_mpeg_header (buffer))
		return false;
	
	// extract the MPEG version
	switch ((buffer[1] >> 3) & 0x03) {
	case 0: /* MPEG Version 2.5 */
		mpeg->version = 3;
		break;
	case 1: /* reserved */
		return false;
	case 2: /* MPEG Version 2 */
		mpeg->version = 2;
		break;
	case 3: /* MPEG Version 1 */
		mpeg->version = 1;
		break;
	}
	
	// extract the MPEG layer
	switch ((buffer[1] >> 1) & 0x03) {
	case 1:
		mpeg->layer = 3;
		break;
	case 2:
		mpeg->layer = 1;
		break;
	case 3:
		mpeg->layer = 2;
		break;
	default:
		// invalid layer
		return false;
	}
	
	// protection (via 16bit crc) bit
	mpeg->prot = (buffer[1] & 0x01) ? 1 : 0;
	
	// extract the bit rate
	if (!mpeg_parse_bitrate (mpeg, buffer[2]))
		return false;
	
	// extract the sample rate
	if (!mpeg_parse_samplerate (mpeg, buffer[2]))
		return false;
	
	// check if the frame is padded
	mpeg->padded = (buffer[2] & 0x02) ? 1 : 0;
	
	// extract the channel mode */
	if (!mpeg_parse_channels (mpeg, buffer[3]))
		return false;
	
	mpeg->copyright = (buffer[3] & 0x08) ? 1 : 0;
	mpeg->original = (buffer[3] & 0x04) ? 1 : 0;
	
	return true;
}

static int mpeg_block_sizes[3][3] = {
	{ 384, 1152, 1152 },  // version 1
	{ 384, 1152,  576 },  // version 2
	{ 384, 1152,  576 }   // version 2.5
};

#define mpeg_block_size(mpeg) mpeg_block_sizes[(mpeg)->version - 1][(mpeg)->layer - 1]

static uint32_t
mpeg_frame_length (MpegFrameHeader *mpeg, bool xing)
{
	uint32_t len;
	
	// calculate the frame length
	if (mpeg->layer == 1)
		len = (((12 * mpeg->bit_rate) / mpeg->sample_rate) + mpeg->padded) * 4;
	else if (mpeg->version == 1)
		len = ((144 * mpeg->bit_rate) / mpeg->sample_rate) + mpeg->padded;
	else
		len = ((72 * mpeg->bit_rate) / mpeg->sample_rate) + mpeg->padded;
	
	if (false && mpeg->prot && !xing) {
		// include 2 extra bytes for 16bit crc
		len += 2;
	}
	
	return len;
}

#define MPEG_FRAME_LENGTH_MAX ((((144 * 160000) / 8000) + 1) + 2)

#define mpeg_frame_size(mpeg) (((mpeg)->bit_rate * (mpeg)->channels * mpeg_block_size (mpeg)) / (mpeg)->sample_rate)

#define mpeg_frame_duration(mpeg) (48000000 / (mpeg)->sample_rate) * 8

#if 0
static void
mpeg_print_info (MpegFrameHeader *mpeg)
{
	const char *version;
	
	switch (mpeg->version) {
	case 1:
		version = "1";
		break;
	case 2:
		version = "2";
		break;
	default:
		version = "2.5";
		break;
	}
	
	printf ("MPEG-%s Audio Layer %d; %d Hz, %d ch, %d kbit\n",
		version, mpeg->layer, mpeg->sample_rate, mpeg->channels,
		mpeg->bit_rate / 1000);
	
	printf ("\t16bit crc=%s; padded=%s\n", mpeg->prot ? "true" : "false",
		mpeg->padded ? "true" : "false");
	
	printf ("\tframe length = %u bytes\n", mpeg_frame_length (mpeg, false));
}
#endif

static int
mpeg_xing_header_offset (MpegFrameHeader *mpeg)
{
	if (mpeg->version == 1)
		return mpeg->channels == 1 ? 21 : 36;
	else
		return mpeg->channels == 1 ? 13 : 21;
}

#define mpeg_vbri_header_offset 36

static bool
mpeg_check_vbr_headers (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, IMediaSource *source, int64_t pos)
{
	uint32_t nframes = 0, size = 0, len;
	uint8_t buffer[24], *bufptr;
	int64_t offset;
	int i;
	
	// first, check for a Xing header
	offset = mpeg_xing_header_offset (mpeg);
	if (!source->Seek (pos + offset, SEEK_SET))
		return false;
	
	if (!source->Peek (buffer, 16))
		return false;
	
	if (!strncmp ((const char *) buffer, "Xing", 4)) {
		bufptr = buffer + 8;
		if (buffer[7] & 0x01) {
			// decode the number of frames
			for (i = 0; i < 4; i++)
				nframes = (nframes << 8) | *bufptr++;
		} if (buffer[7] & 0x02) {
			for (i = 0, size = 0; i < 4; i++)
				size = (size << 8) | *bufptr++;
			
			// calculate the frame length
			len = mpeg_frame_length (mpeg, true);
			
			// estimate the number of frames
			nframes = size / len;
		}
		
		vbr->type = MpegXingHeader;
		vbr->nframes = nframes;
		
		return true;
	}
	
	// check for a Fraunhofer VBRI header
	offset = mpeg_vbri_header_offset;
	if (!source->Seek (pos + offset, SEEK_SET))
		return false;
	
	if (!source->Peek (buffer, 24))
		return false;
	
	if (!strncmp ((const char *) buffer, "VBRI", 4)) {
		// decode the number of frames
		bufptr = buffer + 14;
		for (i = 0; i < 4; i++)
			nframes = (nframes << 8) | *bufptr++;
		
		vbr->type = MpegVBRIHeader;
		vbr->nframes = nframes;
		
		return true;
	}
	
	return false;
}


#define MPEG_JUMP_TABLE_GROW_SIZE 16

Mp3FrameReader::Mp3FrameReader (IMediaSource *source, int64_t start, uint32_t frame_len, uint32_t frame_duration, bool xing)
{
	jmptab = g_new (MpegFrame, MPEG_JUMP_TABLE_GROW_SIZE);
	avail = MPEG_JUMP_TABLE_GROW_SIZE;
	used = 0;
	
	this->frame_dur = frame_duration;
	this->frame_len = frame_len;
	this->xing = xing;
	
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

int64_t
Mp3FrameReader::EstimatePtsPosition (uint64_t pts)
{
	uint32_t frame;
	int64_t pos;
	uint64_t n;
	
	if (pts == cur_pts)		
		return stream->GetPosition ();
	
	// if the pts requested is some place we've been, then we can use our jump table
	if (used > 0) {
		if (pts < (jmptab[used - 1].pts + jmptab[used - 1].dur)) {
			if (pts >= jmptab[used - 1].pts)
				return jmptab[used - 1].offset;
			
			// search for our requested pts
			frame = MpegFrameSearch (pts);
			
			return jmptab[frame - 1].offset;
		}
		
		// estimate based on last known pts/offset
		n = (pts - jmptab[used - 1].pts) / frame_dur;
		pos = jmptab[used - 1].offset + (frame_len * n);
	} else {
		// estimate based on ctor frame len/duration
		pos = stream_start + (frame_len * (pts / frame_dur));
	}
	
	// FIXME: Xing mp3's sometimes have a pts table encoded in the
	// first frame... should use that data if available.
	
	return pos;
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
	
	if (!stream->Peek (buffer, 4))
		return false;
	
	if (!mpeg_parse_header (&mpeg, buffer))
		return false;
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = mpeg_frame_length (&mpeg, xing);
	
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
	
	if (!stream->ReadAll (buffer, 4)) {
		frame->AddState (FRAME_DEMUXED);
		frame->event = FrameEventEOF;
		return MEDIA_NO_MORE_DATA;
	}
	
	if (!mpeg_parse_header (&mpeg, buffer))
		return MEDIA_DEMUXER_ERROR;
	
	if (mpeg.bit_rate == 0) {
		// use the most recently specified bit rate
		mpeg.bit_rate = bit_rate;
		
		// re-encode the bitrate into the header
		buffer[2] |= mpeg_encode_bitrate (&mpeg, bit_rate);
	}
	
	bit_rate = mpeg.bit_rate;
	
	duration = mpeg_frame_duration (&mpeg);
	
	if (used == 0 || offset > jmptab[used - 1].offset)
		AddFrameIndex (offset, cur_pts, duration, bit_rate);
	
	len = mpeg_frame_length (&mpeg, xing);
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
	
	if (!stream->ReadAll (frame->buffer + 4, len - 4))
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

Mp3Demuxer::Mp3Demuxer (Media *media, IMediaSource *source) : IMediaDemuxer (media, source)
{
	reader = NULL;
	xing = false;
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

int64_t
Mp3Demuxer::EstimatePtsPosition (uint64_t pts)
{
	if (reader != NULL)
		return reader->EstimatePtsPosition (pts);
	
	return -1;
}

static int64_t
FindMpegHeader (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, IMediaSource *source, int64_t start)
{
	uint8_t buf[4096], hdr[4], *inbuf, *inend;
	int64_t pos, offset = start;
	register uint8_t *inptr;
	MpegFrameHeader next;
	int32_t n = 0;
	uint32_t len;
	
	if (!source->Seek (start, SEEK_SET))
		return -1;
	
	inbuf = buf;
	
	do {
		if ((n = source->ReadSome (inbuf, sizeof (buf) - n)) <= 0)
			return -1;
		
		inend = inbuf + n;
		inptr = buf;
		
		if ((inend - inptr) < 4)
			return -1;
		
		do {
			/* mpeg audio sync header begins with a 0xff */
			while (inptr < inend && *inptr != 0xff) {
				offset++;
				inptr++;
			}
			
			if (inptr == inend)
				break;
			
			/* found a 0xff byte... could be a frame header */
			if ((inptr + 3) < inend) {
				if (mpeg_parse_header (mpeg, inptr) && mpeg->bit_rate) {
					/* validate that this is really an MPEG frame header by calculating the
					 * position of the next frame header and checking that it looks like a
					 * valid frame header too */
					len = mpeg_frame_length (mpeg, false);
					pos = source->GetPosition ();
					
					if (vbr && mpeg_check_vbr_headers (mpeg, vbr, source, offset)) {
						if (vbr->type == MpegXingHeader)
							len = mpeg_frame_length (mpeg, true);
						
						return offset + len;
					}
					
					if (source->Seek (offset + len, SEEK_SET) && source->Peek (hdr, 4)) {
						if (mpeg_parse_header (&next, hdr)) {
							/* everything checks out A-OK */
							return offset;
						}
					}
					
					/* restore state */
					if (pos == -1 || !source->Seek (pos, SEEK_SET))
						return -1;
				}
				
				/* not an mpeg audio sync header */
				offset++;
				inptr++;
			} else {
				/* not enough data to check */
				break;
			}
		} while (inptr < inend);
		
		if ((n = (inend - inptr)) > 0) {
			/* save the remaining bytes */
			memmove (buf, inptr, n);
		}
		
		/* if we scan more than 'MPEG_FRAME_LENGTH_MAX' bytes, this is unlikely to be an mpeg audio stream */
	} while ((offset - start) < MPEG_FRAME_LENGTH_MAX);
	
	return -1;
}

MediaResult
Mp3Demuxer::ReadHeader ()
{
	IMediaStream **streams = NULL;
	int64_t stream_start;
	IMediaStream *stream;
	MpegFrameHeader mpeg;
	AudioStream *audio;
	uint8_t buffer[10];
	MpegVBRHeader vbr;
	uint64_t duration;
	uint32_t size = 0;
	uint32_t nframes;
	int stream_count;
	uint32_t len;
	int64_t end;
	int i;
	
	if (!source->Peek (buffer, 10))
		return MEDIA_INVALID_MEDIA;
	
	// Check for a leading ID3 tag
	if (!strncmp ((const char *) buffer, "ID3", 3)) {
		for (i = 0; i < 4; i++) {
			if (buffer[6 + i] & 0x80)
				return MEDIA_INVALID_MEDIA;
			
			size = (size << 7) | buffer[6 + i];
		}
		
		if ((buffer[5] & (1 << 4))) {
			// add additional 10 bytes for footer
			size += 20;
		} else
			size += 10;
		
		// MPEG stream data starts at the end of the ID3 tag
		stream_start = (int64_t) size;
	} else {
		stream_start = 0;
	}
	
	// There can be an "arbitrary" amount of garbage at the
	// beginning of an mp3 stream, so we need to find the first
	// MPEG sync header by scanning.
	vbr.type = MpegNoVBRHeader;
	if ((stream_start = FindMpegHeader (&mpeg, &vbr, source, stream_start)) == -1)
		return MEDIA_INVALID_MEDIA;
	
	if (!source->Seek (stream_start, SEEK_SET))
		return MEDIA_INVALID_MEDIA;
	
	if (vbr.type == MpegNoVBRHeader) {
		// calculate the frame length
		len = mpeg_frame_length (&mpeg, false);
		
		if ((end = source->GetSize ()) != -1) {
			// estimate the number of frames
			nframes = (end - stream_start) / len;
		} else {
			nframes = 0;
		}
	} else {
		if (vbr.type == MpegXingHeader)
			xing = true;
		
		// calculate the frame length
		len = mpeg_frame_length (&mpeg, xing);
		nframes = vbr.nframes;
	}
	
	// calculate the duration of the first frame
	duration = mpeg_frame_duration (&mpeg);
	
	reader = new Mp3FrameReader (source, stream_start, len, duration, xing);
	
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
	int64_t stream_start = 0;
	MpegFrameHeader mpeg;
	uint8_t buffer[10];
	uint32_t size = 0;
	MpegVBRHeader vbr;
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
		stream_start = (int64_t) size;
	}
	
	stream_start = FindMpegHeader (&mpeg, &vbr, source, stream_start);
	
	source->Seek (0, SEEK_SET);
	
	return stream_start != -1;
}

IMediaDemuxer *
Mp3DemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new Mp3Demuxer (media, source);
}


/*
 * NullMp3Decoder
 */
MediaResult
NullMp3Decoder::DecodeFrame (MediaFrame *frame)
{
	MpegFrameHeader mpeg;
	
	mpeg_parse_header (&mpeg, frame->buffer);
	g_free (frame->buffer);
	
	frame->buflen = mpeg_frame_size (&mpeg);
	frame->buffer = (uint8_t *) g_malloc0 (frame->buflen);
	
	frame->AddState (FRAME_DECODED);
	
	return MEDIA_SUCCESS;
}


/*
 * FileSource
 */

FileSource::FileSource (Media *media) : IMediaSource (media)
{
	filename = g_strdup (media->GetFileOrUrl ());
	bufptr = buffer;
	buflen = 0;
	pos = -1;
	fd = -1;
	
	eof = true;
}

FileSource::FileSource (Media *media, const char *filename) : IMediaSource (media)
{
	this->filename = g_strdup (filename);
	bufptr = buffer;
	buflen = 0;
	pos = -1;
	fd = -1;
	
	eof = true;
}

FileSource::~FileSource ()
{
	g_free (filename);
	if (fd != -1)
		close (fd);
}

MediaResult 
FileSource::Initialize ()
{
	if (fd != -1)
		return MEDIA_SUCCESS;
	
	if (filename == NULL)
		return MEDIA_FILE_ERROR;
	
	if ((fd = open (filename, O_LARGEFILE | O_RDONLY)) == -1)
		return MEDIA_FILE_ERROR;
	
	eof = false;
	pos = 0;
	
	return MEDIA_SUCCESS;
}

int64_t
FileSource::GetSizeInternal ()
{
	struct stat st;
	
	if (fd == -1 || fstat (fd, &st) == -1)
		return -1;
	
	return st.st_size;
}

int64_t
FileSource::GetPositionInternal ()
{
	return pos;
}

bool
FileSource::SeekInternal (int64_t offset, int mode)
{
	int64_t n;
	
	if (fd == -1)
		return false;
	
	switch (mode) {
	case SEEK_CUR:
		if (offset == 0)
			return true;
		
		/* convert to SEEK_SET */
		if ((offset = pos + offset) < 0)
			return false;
		
		/* fall thru... */
	case SEEK_SET:
		if (pos == offset)
			return true;
		
		if (offset < 0)
			return false;
		
		n = (offset - pos);
		if ((n >= 0 && n <= buflen) || (n < 0 && (-n) <= (bufptr - buffer))) {
			/* the position is within our pre-buffered data */
			pos = offset;
			bufptr += n;
			buflen -= n;
			
			return true;
		}
		
		/* we are now forced to do an actual seek */
		if (lseek (fd, offset, SEEK_SET) == -1)
			return false;
		
		pos = offset;
		eof = false;
		
		bufptr = buffer;
		buflen = 0;
		
		return true;
	case SEEK_END:
		/* I doubt this code path ever gets hit, so we'll just
		 * do things the easy way... to hell with any
		 * optimizations. */
		if (lseek (fd, offset, SEEK_END) == -1)
			return false;
		
		pos = offset;
		eof = false;
		
		bufptr = buffer;
		buflen = 0;
		
		return true;
	default:
		// invalid 'whence' argument
		return false;
	}
}

/* non-interruptable read() */
static ssize_t
noint_read (int fd, char *buf, size_t n)
{
	ssize_t nread;
	
	do {
		nread = read (fd, buf, n);
	} while (nread == -1 && errno == EINTR);
	
	return nread;
}

int32_t
FileSource::ReadInternal (void *buf, uint32_t n)
{
	ssize_t r, nread = 0;
	
	if (fd == -1) {
		errno = EINVAL;
		LOG_PIPELINE_ERROR ("FileSource::ReadInternal (%p, %u): File not open.\n", buf, n);
		return -1;
	}
	
	while (n > 0) {
		if ((r = MIN (buflen, n)) > 0) {
			/* copy what we can from our existing buffer */
			memcpy (((char *) buf) + nread, bufptr, r);
			bufptr += r;
			buflen -= r;
			nread += r;
			pos += r;
			n -= r;
		}
		
		if (n >= sizeof (buffer)) {
			/* bypass intermediate buffer */
			bufptr = buffer;
			if ((r = noint_read (fd, ((char *) buf) + nread, n)) > 0) {
				nread += r;
				pos += r;
				n -= r;
			}
		} else if (n > 0) {
			/* buffer more data */
			if ((r = noint_read (fd, buffer, sizeof (buffer))) > 0)
				buflen = (uint32_t) r;
			
			bufptr = buffer;
		}
		
		if (r == -1 && nread == 0) {
			LOG_PIPELINE_ERROR ("FileSource<%d>::ReadInternal ('%s', %p, %u): Some error occured during reading, r: %d, nread: %d, errno: %d = %s.\n",
					    GET_OBJ_ID (this), filename, buf, n, r, nread, errno, strerror (errno));
			return -1;
		}
		
		if (r == 0) {
			LOG_PIPELINE_ERROR ("FileSource<%d>::ReadInternal ('%s', %p, %u): Could not read all the data, eof reached. Current position: %lld\n",
					    GET_OBJ_ID (this), filename, buf, n, GetPositionInternal ());
			eof = true;
			break;
		}
	}
	
	return nread;
}

int32_t
FileSource::PeekInternal (void *buf, uint32_t n, int64_t start)
{
	int32_t result;
	int64_t current_pos;
	int64_t initial_pos;

	if (start == -1) {
		initial_pos = GetPosition ();
	} else {
		initial_pos = start;		
	}

	current_pos = GetPosition ();
	
	// First try to peek in the buffer.
	if (initial_pos == current_pos && PeekInBuffer (buf, n))
		return n;
	
	// We could not peek in the buffer, use a very simple fallback algorithm.
	LOG_PIPELINE ("FileSource<%i>::PeekInternal (%p, %i, %lld), initial_pos: %lld, current_pos: %lld, GetPosition (): %lld\n", GET_OBJ_ID (this), buf, n, start, initial_pos, current_pos, GetPosition ());

	if (initial_pos != current_pos)
		Seek (initial_pos, SEEK_SET);

	result = ReadSome (buf, n);
	
	Seek (current_pos, SEEK_SET);

	LOG_PIPELINE ("FileSource<%i>::PeekInternal (%p, %i, %lld), initial_pos: %lld, current_pos: %lld, GetPosition (): %lld [Done]\n", GET_OBJ_ID (this), buf, n, start, initial_pos, current_pos, GetPosition ());

	return result;
}

bool
FileSource::PeekInBuffer (void *buf, uint32_t n)
{
	uint32_t need, used, avail, shift;
	ssize_t r;
	
	if (fd == -1)
		return false;
	
	if (n > sizeof (buffer)) {
		/* we can't handle a peek of this size */
		return false;
	}
	
	if (buflen < n) {
		/* need to buffer more data */
		if (bufptr > buffer) {
			used = (bufptr + buflen) - buffer;
			avail = sizeof (buffer) - used;
			need = n - buflen;
			
			if (avail < need) {
				/* make room for 'need' more bytes */
				shift = need - avail;
				memmove (buffer, buffer + shift, used - shift);
				bufptr -= shift;
			} else {
				/* request 'avail' more bytes so we
				 * can hopefully fill our buffer */
				need = avail;
			}
		} else {
			/* nothing in our buffer, fill 'er up */
			need = sizeof (buffer);
		}
		
		do {
			if ((r = noint_read (fd, bufptr + buflen, need)) == 0) {
				eof = true;
				break;
			} else if (r == -1)
				break;
			
			buflen += r;
			need -= r;
		} while (buflen < n);
	}
	
	if (buflen < n) {
		/* can't peek as much data as requested... */
		return false;
	}
	
	memcpy (((char *) buf), bufptr, n);
	
	return true;
}

/*
 * ProgressiveSource
 */

ProgressiveSource::ProgressiveSource (Media *media, bool is_live) : FileSource (media, NULL)
{
	last_requested_pts = UINT64_MAX;
	write_pos = 0;
	wait_pos = 0;
	first_write_pos = 0;
	size = -1;
	requested_pts = UINT64_MAX;
	this->is_live = is_live;
}

ProgressiveSource::~ProgressiveSource ()
{
}

MediaResult
ProgressiveSource::Initialize ()
{
	if (filename != NULL)
		return MEDIA_SUCCESS;
	
	filename = g_build_filename (g_get_tmp_dir (), "MoonlightProgressiveStream.XXXXXX", NULL);
	if ((fd = g_mkstemp (filename)) == -1) {
		g_free (filename);
		filename = NULL;
		
		return MEDIA_FAIL;
	}
	
	// unlink the file right away so that it'll be deleted even if we crash.
	unlink (filename);

	wait_pos = 0;
	eof = false;
	pos = 0;
	
	return MEDIA_SUCCESS;
}

static ssize_t
write_all (int fd, char *buf, size_t len)
{
	size_t nwritten = 0;
	ssize_t n;
	
	do {
		do {
			n = write (fd, buf + nwritten, len - nwritten);
		} while (n == -1 && (errno == EINTR || errno == EAGAIN));
		
		if (n > 0)
			nwritten += n;
	} while (n != -1 && nwritten < len);
	
	if (n == -1)
		return -1;
	
	return nwritten;
}

void
ProgressiveSource::Write (void *buf, int64_t offset, int32_t n)
{
	ssize_t nwritten;

	// printf ("ProgressiveSource::Write (%p, %lld, %i)\n", buf, offset, n);
	if (fd == -1) {
		media->AddMessage (MEDIA_FAIL, "Progressive source doesn't have a file to write the data to.");
		return;
	}
	
	Lock ();

	if (n == 0) {
		// We've got the entire file, update the size
		size = write_pos;
		goto cleanup;
	}

	// Seek to the write position
	if (lseek (fd, offset, SEEK_SET) != offset)
		goto cleanup;
	
	if ((nwritten = write_all (fd, (char *) buf, n)) > 0)
		write_pos = offset + nwritten;

	
	// Restore the current position
	if (write_pos != -1) {
		lseek (fd, pos + buflen, SEEK_SET);
	} else {
		first_write_pos = offset;
		FileSource::Seek (offset, SEEK_SET);
		buflen = 0;
	}
	
cleanup:
	if (IsWaiting ())
		Signal ();
	
	Unlock ();
}

void
ProgressiveSource::NotifySize (int64_t size)
{
	Lock ();
	this->size = size;
	Unlock ();
}

bool
ProgressiveSource::SeekToPts (uint64_t pts)
{

	if (last_requested_pts == pts)
		return true;

	if (pts == 0 && Seek (0, SEEK_SET))
		return true;

	Lock ();
	requested_pts = pts;
	write_pos = -1;

	StartWaitLoop ();
	while (!Aborted () && requested_pts != UINT64_MAX)
		Wait ();
	EndWaitLoop ();

	Unlock ();

	return !Aborted ();
}

void
ProgressiveSource::RequestPosition (int64_t *pos)
{
	Lock ();
	if (requested_pts != UINT64_MAX && requested_pts != last_requested_pts) {
		*pos = requested_pts;
		last_requested_pts = requested_pts;
		requested_pts = UINT64_MAX;
		Signal ();
	}
	Unlock ();
}

/*
 *
 */
MemorySource::MemorySource (Media *media, void *memory, int32_t size, int64_t start)
	: IMediaSource (media)
{
	this->memory = memory;
	this->size = size;
	this->start = start;
	this->pos = 0;
	this->owner = true;
}

MemorySource::~MemorySource ()
{
	if (owner)
		g_free (memory);
}

bool
MemorySource::SeekInternal (int64_t offset, int mode)
{
	int64_t real_offset;

	switch (mode) {
	case SEEK_SET:
		real_offset = offset - start;
		if (real_offset < 0 || real_offset > size)
			return false;
		pos = real_offset;
		return true;
	case SEEK_CUR:
		if (pos + offset > size || pos + offset < 0)
			return false;
		pos += offset;
		return true;
	case SEEK_END:
		if (size - offset > size || size - offset < 0)
			return false;
		pos = size - offset;
		return true;
	default:
		return false;
	}
	return true;
}

int32_t 
MemorySource::ReadInternal (void *buffer, uint32_t n)
{
	uint32_t k = MIN (n, size - pos);
	memcpy (buffer, ((char*) memory) + pos, k);
	pos += k;
	return k;
}

int32_t
MemorySource::PeekInternal (void *buffer, uint32_t n, int64_t start)
{
	if (start == -1)
		start = pos;

	if (n > size - start)
		return 0;
	memcpy (buffer, ((char*) memory) + start, n);
	return n;
}

/*
 * MediaClosure
 */ 

MediaClosure::MediaClosure (MediaCallback *cb)
{
	callback = cb;
	frame = NULL;
	media = NULL;
	context = NULL;
	result = 0;
}

MediaClosure::~MediaClosure ()
{
	delete frame;

	base_unref (context);
	base_unref (media);
}

MediaResult
MediaClosure::Call ()
{
	if (callback)
		return callback (this);
		
	return MEDIA_NO_CALLBACK;
}

void
MediaClosure::SetMedia (Media *media)
{
	if (this->media)
		this->media->unref ();
	this->media = media;
	if (this->media)
		this->media->ref ();
}

Media *
MediaClosure::GetMedia ()
{
	return media;
}

void
MediaClosure::SetContext (EventObject *context)
{
	if (this->context)
		this->context->unref ();
	this->context = context;
	if (this->context)
		this->context->ref ();
}

EventObject *
MediaClosure::GetContext ()
{
	return context;
}

MediaClosure *
MediaClosure::Clone ()
{
	MediaClosure *closure = new MediaClosure (callback);
	closure->SetContext (context);
	closure->SetMedia (media);
	closure->frame = frame;
	closure->marker = marker;
	closure->result = result;
	return closure;
}

/*
 * IMediaStream
 */

IMediaStream::IMediaStream (Media *media) : IMediaObject (media)
{
	context = NULL;
	
	extra_data_size = 0;
	extra_data = NULL;
	
	msec_per_frame = 0;
	duration = 0;
	
	decoder = NULL;
	codec_id = 0;
	codec = NULL;
	
	min_padding = 0;
	index = -1;
	selected = false;
}

IMediaStream::~IMediaStream ()
{
	if (decoder)
		decoder->unref ();
	
	g_free (extra_data);
}

void
IMediaStream::SetSelected (bool value)
{
	selected = value;
	if (media && media->GetDemuxer ())
		media->GetDemuxer ()->UpdateSelected (this);
}

/*
 * IMediaDemuxer
 */ 
 
IMediaDemuxer::IMediaDemuxer (Media *media, IMediaSource *source) : IMediaObject (media)
{
	this->source = source;
	this->source->ref ();
	stream_count = 0;
	streams = NULL;
}

IMediaDemuxer::~IMediaDemuxer ()
{
	if (streams != NULL) {
		for (int i = 0; i < stream_count; i++)
			streams[i]->unref ();
		
		g_free (streams);
	}
	source->unref ();
}

uint64_t
IMediaDemuxer::GetDuration ()
{
	uint64_t result = 0;
	for (int i = 0; i < GetStreamCount (); i++)
		result = MAX (result, GetStream (i)->duration);
	return result;
}

IMediaStream*
IMediaDemuxer::GetStream (int index)
{
	return (index < 0 || index >= stream_count) ? NULL : streams [index];
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
	this->media = media;
}

IMediaObject::~IMediaObject ()
{
}

/*
 * IMediaSource
 */

IMediaSource::IMediaSource (Media *media)
	: IMediaObject (media)
{
	pthread_mutexattr_t attribs;
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&mutex, &attribs);
	pthread_mutexattr_destroy (&attribs);

	pthread_cond_init (&condition, NULL);

	wait_count = 0;
	aborted = false;
}

IMediaSource::~IMediaSource ()
{
	pthread_mutex_destroy (&mutex);
	pthread_cond_destroy (&condition);	
}

void
IMediaSource::Lock ()
{
	pthread_mutex_lock (&mutex);
}

void
IMediaSource::Unlock ()
{
	pthread_mutex_unlock (&mutex);
}

void
IMediaSource::Signal ()
{
	pthread_cond_signal (&condition);
}

void
IMediaSource::Wait ()
{
	if (aborted)
		return;

	g_atomic_int_inc (&wait_count);
	pthread_cond_wait (&condition, &mutex);
	g_atomic_int_dec_and_test (&wait_count);
}

void
IMediaSource::StartWaitLoop ()
{
	g_atomic_int_inc (&wait_count);
}

void
IMediaSource::EndWaitLoop ()
{
	if (g_atomic_int_dec_and_test (&wait_count))
		aborted = false;
}

void
IMediaSource::Abort ()
{
	LOG_PIPELINE ("IMediaSource::Abort ()\n");
	//LOG_PIPELINE_ERROR_CONDITIONAL (GetLastAvailablePosition () != -1, "IMediaSource::Abort (): Any pending operations will now be aborted (probably due to a seek) and you may see a few reading errors. This is completely normal.\n")

	// There's no need to lock here, since aborted can only be set to true.
	aborted = true;
	while (IsWaiting ()) {
		Signal ();
	}
}

bool
IMediaSource::IsWaiting ()
{
	return g_atomic_int_get (&wait_count) != 0;
}

void
IMediaSource::WaitForPosition (bool block, int64_t position)
{
	LOG_PIPELINE ("IMediaSource<%i>::WaitForPosition (%i, %lld) last available pos: %lld, aborted: %i\n", GET_OBJ_ID (this), block, position, GetLastAvailablePositionInternal (), aborted);

	StartWaitLoop ();

	if (block && GetLastAvailablePositionInternal () != -1) {
		while (!aborted) {
			// Check if the entire file is available.
			if (GetSizeInternal () >= 0 && GetSizeInternal () <= GetLastAvailablePositionInternal ()) {
				LOG_PIPELINE ("IMediaSource<%i>::WaitForPosition (%i, %lld): GetSize (): %lld, GetLastAvailablePositionInternal (): %lld.\n", GET_OBJ_ID (this), block, position, GetSizeInternal (), GetLastAvailablePositionInternal ());	
				break;
			}

			// Check if we got the position we want
			if (GetLastAvailablePositionInternal () >= position) {
				LOG_PIPELINE ("IMediaSource<%i>::WaitForPosition (%i, %lld): GetLastAvailablePositionInternal (): %lld.\n", GET_OBJ_ID (this), block, position, GetLastAvailablePositionInternal ());	
				break;
			}
			
			Wait ();
		}
	}

	EndWaitLoop ();

	LOG_PIPELINE ("IMediaSource<%i>::WaitForPosition (%i, %lld): aborted: %i\n", GET_OBJ_ID (this), block, position, aborted);
}

int32_t
IMediaSource::ReadSome (void *buf, uint32_t n, bool block, int64_t start)
{
	int32_t result;

	LOG_PIPELINE ("IMediaSource<%i>::ReadSome (%p, %i, %s, %lld)\n", GET_OBJ_ID (this), buf, n, block ? "true" : "false", start);
	
	Lock ();
	
	if (start == -1)
		start = GetPositionInternal ();
	else if (start != GetPositionInternal ())
		SeekInternal (start, SEEK_SET);

	WaitForPosition (block, start + n);

	result = ReadInternal (buf, n);

	Unlock ();

	return result;
}

bool
IMediaSource::ReadAll (void *buf, uint32_t n, bool block, int64_t start)
{
	int32_t read;

	LOG_PIPELINE ("IMediaSource<%d>::ReadAll (%p, %u, %s, %lld).\n", GET_OBJ_ID (this), buf, n, block ? "true" : "false", start);

	read = ReadSome (buf, n, block, start);

	LOG_PIPELINE_ERROR_CONDITIONAL ((int64_t) read != (int64_t) n, "IMediaSource<%d>::ReadAll (%p, %u, %s, %lld): Could only read %i bytes.\n",
					GET_OBJ_ID (this), buf, n, block ? "true" : "false", start, read);
	LOG_PIPELINE ("IMediaSource<%d>::ReadAll (%p, %u, %s, %lld), read: %d [Done].\n", GET_OBJ_ID (this), buf, n, block ? "true" : "false", start, read);

	return (int64_t) read == (int64_t) n;
}

bool
IMediaSource::Peek (void *buf, uint32_t n, bool block, int64_t start)
{
	bool result;

	Lock ();

	if (start == -1)
		start = GetPositionInternal ();

	WaitForPosition (block, start + n);

	result = (int64_t) PeekInternal (buf, n, start) == (int64_t) n;

	Unlock ();

	return result;
}

bool
IMediaSource::Seek (int64_t offset, int mode)
{
	LOG_PIPELINE ("IMediaSource<%i> (%s)::Seek (%lld, %i = %s)\n", GET_OBJ_ID (this), ToString (), offset, mode, mode == SEEK_SET ? "SEEK_SET" : (mode == SEEK_CUR ? "SEEK_CUR" : (mode == SEEK_END ? "SEEK_END" : "<invalid value>")));

	bool result;
	Lock ();
	result = SeekInternal (offset, mode);
	Unlock ();
	return result;
}

int64_t
IMediaSource::GetLastAvailablePosition ()
{
	int64_t result;
	Lock ();
	result = GetLastAvailablePositionInternal ();
	Unlock ();
	return result;
}

int64_t
IMediaSource::GetPosition ()
{
	int64_t result;
	Lock ();
	result = GetPositionInternal ();
	Unlock ();
	return result;
}

int64_t
IMediaSource::GetSize ()
{
	int64_t result;
	Lock ();
	result = GetSizeInternal ();
	Unlock ();
	return result;
}

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

IMediaDecoder::IMediaDecoder (Media *media, IMediaStream *stream) : IMediaObject (media)
{
	this->stream = stream;
}

/*
 * IImageConverter
 */

IImageConverter::IImageConverter (Media *media, VideoStream *stream) : IMediaObject (media)
{
	output_format = MoonPixelFormatNone;
	input_format = MoonPixelFormatNone;
	this->stream = stream;
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
		converter->unref ();
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
 * MediaWork
 */ 
MediaWork::MediaWork (MediaClosure *closure, IMediaStream *stream, uint16_t states)
{
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		type = WorkTypeVideo; break;
	case MediaTypeAudio:
		type = WorkTypeAudio; break;
	case MediaTypeMarker:
		type = WorkTypeMarker; break;
	default:
		fprintf (stderr, "MediaWork::MediaWork (%p, %p, %i): Invalid stream type %u\n", closure, stream, (uint) states, stream->GetType ());
		break;
	}
	this->closure = closure;
	this->data.frame.states = states;
	this->data.frame.stream = stream;
	this->data.frame.stream->ref ();
}

MediaWork::MediaWork (MediaClosure *closure, uint64_t seek_pts)
{
	type = WorkTypeSeek;
	this->closure = closure;
	this->data.seek.seek_pts = seek_pts;
}

MediaWork::MediaWork (MediaClosure *closure, IMediaSource *source)
{
	type = WorkTypeOpen;
	this->closure = closure;
	this->data.open.source = source;
	this->data.open.source->ref ();
}

MediaWork::~MediaWork ()
{
	switch (type) {
	case WorkTypeOpen:
		base_unref (data.open.source);
		break;
	case WorkTypeVideo:
	case WorkTypeAudio:
	case WorkTypeMarker:
		base_unref (data.frame.stream);
		break;
	case WorkTypeSeek:
		break; // Nothing to do
	}
	delete closure;

#if DEBUG
	memset (&data, 0, sizeof (data));
#endif
}
