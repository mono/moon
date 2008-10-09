/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline.cpp: Pipeline for the media
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
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
#include <sched.h>

#include <dlfcn.h>

#include "pipeline.h"
#include "pipeline-ffmpeg.h"
#include "mp3.h"
#include "uri.h"
#include "media.h"
#include "asf/asf.h"
#include "asf/asf-structures.h"
#include "yuv-converter.h"
#include "runtime.h"
#include "mms-downloader.h"
#include "pipeline-ui.h"

#define LOG_PIPELINE(...)// printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR(...) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR_CONDITIONAL(x, ...) if (x) printf (__VA_ARGS__);
#define LOG_FRAMEREADERLOOP(...)// printf (__VA_ARGS__);
#define LOG_BUFFERING(...)// printf (__VA_ARGS__);

/*
 * MediaNode
 */

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

bool Media::registering_ms_codecs = false;
bool Media::registered_ms_codecs = false;
DemuxerInfo *Media::registered_demuxers = NULL;
DecoderInfo *Media::registered_decoders = NULL;
ConverterInfo *Media::registered_converters = NULL;
Queue *Media::media_objects = NULL;

Media::Media (MediaElement *element, Downloader *dl)
{
	LOG_PIPELINE ("Media::Media (%p <id:%i>), id: %i\n", element, GET_OBJ_ID (element), GET_OBJ_ID (this));

	// Add ourselves to the global list of medias
	media_objects->Push (new MediaNode (this));

	pthread_attr_t attribs;
	
	this->element = element;
	this->SetSurface (element->GetSurface ());

	downloader = dl;
	if (downloader)
		downloader->ref ();

	queued_requests = new List ();
	
	file_or_url = NULL;
	source = NULL;
	
	demuxer = NULL;
	markers = NULL;
	buffering_time = 0;
	
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
	
	if (downloader)
		downloader->unref ();
	
	if (!stopped)
		pthread_join (queue_thread, NULL);
	pthread_mutex_destroy (&queue_mutex);
	pthread_cond_destroy (&queue_condition);
	
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

bool
Media::IsMSCodecsInstalled ()
{
	return registered_ms_codecs;
}

void
Media::RegisterMSCodecs (void)
{
	register_codec reg;
	void *dl;
	MoonlightConfiguration config;
	char *libmscodecs_path = config.GetStringValue ("Codecs", "MSCodecsPath");
	const char *functions [] = {"register_mswma", "register_mswmv", "register_msmp3"};
	registering_ms_codecs = true;

	if (libmscodecs_path == NULL || !(g_file_test (libmscodecs_path, G_FILE_TEST_EXISTS) && g_file_test (libmscodecs_path, G_FILE_TEST_IS_REGULAR)))
		libmscodecs_path = g_strdup ("libmscodecs.so");
	
	dl = dlopen (libmscodecs_path, RTLD_LAZY);
	if (dl != NULL) {
		for (int i = 0; i < 3; i++) {
			reg = (register_codec) dlsym (dl, functions [i]);
			if (reg != NULL) {
				(*reg) (MOONLIGHT_CODEC_ABI_VERSION);
			} else if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG) {
				printf ("Moonlight: Cannot find %s in %s.\n", functions [i], libmscodecs_path);
			}
		}		
		registered_ms_codecs = true;
	} else if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG) {
		printf ("Moonlight: Cannot load %s: %s\n", libmscodecs_path, dlerror ());
	}
	g_free (libmscodecs_path);

	registering_ms_codecs = false;
}

void
Media::SetBufferingTime (guint64 buffering_time)
{
	pthread_mutex_lock (&queue_mutex);
	this->buffering_time = buffering_time;
	pthread_mutex_unlock (&queue_mutex);
}

guint64
Media::GetBufferingTime ()
{
	guint64 result;
	pthread_mutex_lock (&queue_mutex);
	result = buffering_time;
	pthread_mutex_unlock (&queue_mutex);
	return result;
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
	MediaInfo *current;
	
	//printf ("Media::RegisterDecoder (%p)\n", info);
	info->next = NULL;
	if (registered_decoders == NULL) {
		registered_decoders = info;
	} else {
		if (registering_ms_codecs) {
			// MS codecs might get registered after all other codecs (right after installing them), 
			// which means after the null codecs so if they don't get special treatment, they won't
			// get used until the next browser restart (when they're registered normally).
			// So instead of appending them, we prepend them.
			info->next = registered_decoders;
			registered_decoders = info;
		} else {
			current = registered_decoders;
			while (current->next != NULL)
				current = current->next;
			current->next = info;
		}
	}
	if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG)
		printf ("Moonlight: Codec has been registered: %s\n", info->GetName ());
}

void
Media::Initialize ()
{
	LOG_PIPELINE ("Media::Initialize ()\n");
	
	media_objects = new Queue ();	
	
	// demuxers
	Media::RegisterDemuxer (new ASFDemuxerInfo ());
	Media::RegisterDemuxer (new Mp3DemuxerInfo ());
	Media::RegisterDemuxer (new ASXDemuxerInfo ());

	// converters
	if (!(moonlight_flags & RUNTIME_INIT_FFMPEG_YUV_CONVERTER))
		Media::RegisterConverter (new YUVConverterInfo ());

	// decoders
	Media::RegisterDecoder (new ASFMarkerDecoderInfo ());
	if (!(moonlight_flags & RUNTIME_INIT_DISABLE_MS_CODECS)) {
		RegisterMSCodecs ();
	}
#ifdef INCLUDE_FFMPEG
	if (!(moonlight_flags & RUNTIME_INIT_DISABLE_FFMPEG_CODECS)) {
		register_ffmpeg ();
	}
#endif
	
	Media::RegisterDecoder (new NullDecoderInfo ());
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
		node->media->ref ();
		node->media->StopThread ();
		node->media->unref ();
		node = (MediaNode *) node->next;
	}
	
	media_objects->Unlock ();

	delete media_objects;
	media_objects = NULL;

	LOG_PIPELINE ("Media::Shutdown () [Done]\n");
}

void
Media::Warning (MediaResult result, const char *format, ...)
{
	va_list args;
	
	if (MEDIA_SUCCEEDED (result))
		return;
	
	fprintf (stderr, "Moonlight: MediaResult = %d; ", result);
	
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	
	fputc ('\n', stderr);
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
Media::Seek (guint64 pts)
{
	if (demuxer)
		return demuxer->Seek (pts);
	
	return MEDIA_FAIL;
}

MediaResult
Media::SeekAsync (guint64 pts, MediaClosure *closure)
{
	LOG_PIPELINE ("Media::SeekAsync (%llu, %p), id: %i\n", pts, closure, GET_OBJ_ID (this));

	if (demuxer == NULL)
		return MEDIA_FAIL;
	
	EnqueueWork (new MediaWork (closure, pts));
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::Open ()
{
	LOG_PIPELINE ("Media::Open (), id: %i\n", GET_OBJ_ID (this));

	if (source == NULL) {
		Media::Warning (MEDIA_INVALID_ARGUMENT, "Media::Initialize () hasn't been called (or didn't succeed).");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if (IsOpened ()) {
		Media::Warning (MEDIA_FAIL, "Media::Open () has already been called.");
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
	LOG_PIPELINE ("Media::Open (%p <id:%i>), id: %i, downloader: %p\n", source, GET_OBJ_ID (source), GET_OBJ_ID (this), downloader);

	MediaResult result;
	MediaResult support;
	MmsDownloader *mms_dl = NULL;
	ASFParser *asf_parser = NULL;
	
	LOG_PIPELINE ("Media::Open ().\n");
	
	if (source == NULL || IsOpened ()) // Initialize wasn't called (or didn't succeed) or already open.
		return MEDIA_INVALID_ARGUMENT;
	
	if (downloader != NULL && downloader->GetInternalDownloader () != NULL && downloader->GetInternalDownloader ()->GetType () == InternalDownloader::MmsDownloader) {
		// The internal downloader doesn't get deleted until the Downloader itself is destructed, which won't happen 
		// because we have a ref to it. Which means that it's safe to access the internal dl here.
		mms_dl = (MmsDownloader *) downloader->GetInternalDownloader ();
		if ((asf_parser = mms_dl->GetASFParser ()) == NULL) {
			if (stopped || stopping)
				return MEDIA_FAIL;
				
			if (downloader->IsAborted ())
				return MEDIA_READ_ERROR;
	
			if (source->Eof ())
				return MEDIA_READ_ERROR;						

			return MEDIA_NOT_ENOUGH_DATA;
		}
		
		demuxer = new ASFDemuxer (this, source);
		((ASFDemuxer *) demuxer)->SetParser (asf_parser);
		asf_parser->SetSource (source);
		LOG_PIPELINE ("Media::Open (): Using parser from MmsDownloader, source: %s.\n", source->ToString ());
	}
	
	
	// Select a demuxer
	DemuxerInfo *demuxerInfo = registered_demuxers;
	while (demuxer == NULL && demuxerInfo != NULL) {
		support = demuxerInfo->Supports (source);
		
		if (support == MEDIA_SUCCESS)
			break;
		
		result = support;

		if (result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_PIPELINE ("Media::Open (): '%s' can't determine whether it can handle the media or not due to not enough data being available yet.\n", demuxerInfo->GetName ());
			return result;
		}
		
		LOG_PIPELINE ("Media::Open (): '%s' can't handle this media.\n", demuxerInfo->GetName ());
		demuxerInfo = (DemuxerInfo *) demuxerInfo->next;
	}
	
	if (demuxer == NULL && demuxerInfo == NULL) {
		const char *source_name = file_or_url;
		
		if (!source_name) {
			switch (source->GetType ()) {
			case MediaSourceTypeProgressive:
			case MediaSourceTypeFile:
				source_name = ((FileSource *) source)->GetFileName ();
				break;
			case MediaSourceTypeQueueMemory:
				source_name = "live source";
				break;
			default:
				source_name = "unknown source";
				break;
			}
		}
		
		Media::Warning (MEDIA_UNKNOWN_MEDIA_TYPE, "No demuxers registered to handle the media source `%s'.", source_name);
		
		return MEDIA_UNKNOWN_MEDIA_TYPE;
	}
	
	// Found a demuxer
	if (demuxer == NULL)
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
		
		if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG)
			printf ("Moonlight: Searching registered decoders for a decoder which supports '%s'\n", codec);
		
		DecoderInfo *current_decoder = registered_decoders;
		while (current_decoder != NULL && !current_decoder->Supports (codec)) {
			if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG)
				printf ("Moonlight: Checking if registered decoder '%s' supports codec '%s': no.\n", current_decoder->GetName (), codec);
			current_decoder = (DecoderInfo*) current_decoder->next;
		}

		if (current_decoder == NULL) {
			Media::Warning (MEDIA_UNKNOWN_CODEC, "Unknown codec: '%s'.", codec);	
		} else {
			if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG)
				printf ("Moonlight: Checking if registered decoder '%s' supports codec '%s': yes.\n", current_decoder->GetName (), codec);
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
					Media::Warning (MEDIA_UNKNOWN_CONVERTER, "Can't convert from %d to %d: No converter found.",
							decoder->pixel_format, MoonPixelFormatRGB32);	
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
	guint16 states= work->data.frame.states;	
	IMediaStream *stream = work->data.frame.stream;
	MediaResult result = MEDIA_SUCCESS;
	
	//printf ("Media::GetNextFrame (%p).\n", stream);
	
	if (work == NULL) {
		Media::Warning (MEDIA_INVALID_ARGUMENT, "work is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if (stream == NULL) {
		Media::Warning (MEDIA_INVALID_ARGUMENT, "work->stream is NULL.");
		return MEDIA_INVALID_ARGUMENT;
	}
	
	if ((states & FRAME_DEMUXED) != FRAME_DEMUXED)
		return result; // Nothing to do?

	do {
		if (frame) {
			LOG_PIPELINE ("Media::GetNextFrame (): delayed a frame\n");
			delete frame;
		}
		// TODO: Before popping any frames, we need to check if the decoder still has data available
		frame = stream->PopFrame ();
		//printf ("Media::GetNextFrame () popped a frame: %p, pts: %llu\n", frame, frame ? MilliSeconds_FromPts (frame->pts) : 0);
		if (frame == NULL) {
			result = MEDIA_BUFFER_UNDERFLOW;
			break;
		}
		
		if ((states & FRAME_DECODED) != FRAME_DECODED) {
			// We weren't requested to decode the frame
			// This might cause some errors on delayed codecs (such as the wmv ones).
			break;
		}
		
		if (frame->event != 0)
			break;
	
		result = stream->decoder->DecodeFrame (frame);
		//printf ("Media::GetNextFrame () decoded a frame: %p, pts: %llu\n", frame, frame ? MilliSeconds_FromPts (frame->pts) : 0);
	} while (result == MEDIA_CODEC_DELAYED);

	work->closure->frame = frame;

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
		if (queued_requests != NULL && !stopping && queued_requests->IsEmpty ())
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
		
		if (demuxer != NULL && (node == NULL || node->type != WorkTypeSeek)) {
			// Fill buffers if we have a demuxer and as long as we don't have a pending seek.
			demuxer->FillBuffers ();
		}

		if (node == NULL)
			continue; // Found nothing, continue waiting.
		
		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): processing node %p with type %i.\n", node, node->type);
		
		switch (node->type) {
		case WorkTypeSeek:
			LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): Seeking, current count: %d\n", queued_requests->Length ());
			result = Seek (node->data.seek.seek_pts);
			if (result == MEDIA_NOT_ENOUGH_DATA) {
				bool putback = false;
				// The seek failed because we don't have enough data.
				// Put the seek request back again.
				pthread_mutex_lock (&queue_mutex);
				// There is a race condition here, somebody might have called ClearQueue now to
				// empty the queue, and we'll enter new items in it (a bad thing to do). 
				// We handle a second seek request correctly here (which would be one reason for a 
				// ClearQueue), but there might be other cases.
				// The proper fix would be to store the seek request on the media itself, and only
				// clear it if the seek succeeded (or failed, but not because of missing data).
				putback = (queued_requests->First () == NULL || ((MediaWork *) queued_requests->First ())->type != WorkTypeSeek);
				pthread_mutex_unlock (&queue_mutex);

				if (putback) {
					LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): putting back seek request.\n");
					SeekAsync (node->data.seek.seek_pts, node->closure);
					node->closure = NULL;
				}
			}
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

		if (node->closure != NULL) {
			node->closure->result = result;
			node->closure->Call ();
		}
		
		LOG_FRAMEREADERLOOP ("Media::WorkerLoop (): processed node %p with type %i, result: %i.\n",
				     node, node->type, result);
		
		delete node;
	}
	
	stopped = true;

	LOG_PIPELINE ("Media::WorkerLoop (): exiting.\n");
}

void
Media::GetNextFrameAsync (MediaClosure *closure, IMediaStream *stream, guint16 states)
{
	MoonWorkType type;
	
	if (stream == NULL) {
		Media::Warning (MEDIA_INVALID_ARGUMENT, "stream is NULL.");
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
		Media::Warning (MEDIA_INVALID_ARGUMENT, "The frame's stream is of an unknown type.");
		return;
	}
	
	EnqueueWork (new MediaWork (closure, stream, states));
}

void
Media::EnqueueWork (MediaWork *work)
{
	MediaWork *current;
	
	//printf ("Media::EnqueueWork (%p), type: %i.\n", work, work->type);
	
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
Media::WakeUp ()
{
	pthread_mutex_lock (&queue_mutex);
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
	
	delete reader;

	if (parser)
		parser->unref ();
}

void
ASFDemuxer::UpdateSelected (IMediaStream *stream)
{
	if (reader)
		reader->SelectStream (stream_to_asf_index [stream->index], stream->GetSelected ());

	IMediaDemuxer::UpdateSelected (stream);
}

MediaResult
ASFDemuxer::SeekInternal (guint64 pts)
{
	//printf ("ASFDemuxer::Seek (%llu)\n", pts);
	
	if (reader == NULL)
		return MEDIA_FAIL;
	
	return reader->Seek (pts);
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
	MarkerStream *marker_stream;
	ASFFrameReader *reader;
	MediaMarker *marker;
	
	for (int i = 0; i < GetStreamCount (); i++) {
		if (GetStream (i)->GetType () == MediaTypeMarker) {
			marker_stream = (MarkerStream *) GetStream (i);
			this->reader->SelectStream (stream_to_asf_index [marker_stream->index], true);
			reader = this->reader->GetFrameReader (stream_to_asf_index [marker_stream->index]);
			reader->SetMarkerStream (marker_stream);
			printf ("ASFDemuxer::ReadMarkers (): Hooked up marker stream.\n");
		}
	}
		
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

			marker = new MediaMarker ("Name", text, pts);
			markers->Append (new MediaMarker::Node (marker));
			marker->unref ();
			
			//printf ("MediaElement::ReadMarkers () Added marker at %llu (text: %s, type: %s)\n", pts, text, "Name");
		
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
	if (this->parser)
		this->parser->ref ();
}

MediaResult
ASFDemuxer::ReadHeader ()
{
	MediaResult result = MEDIA_SUCCESS;
	ASFParser *asf_parser = NULL;
	gint32 *stream_to_asf_index = NULL;
	IMediaStream **streams = NULL;
	int current_stream = 1;
	int stream_count = 0;
	
	if (parser != NULL) {
		asf_parser = parser;
	} else {
		asf_parser = new ASFParser (source, media);
	}

	LOG_PIPELINE ("ASFDemuxer::ReadHeader ().\n");

	result = asf_parser->ReadHeader ();
	if (!MEDIA_SUCCEEDED (result)) {
		if (result == MEDIA_NOT_ENOUGH_DATA) {
			LOG_PIPELINE ("ASFDemuxer::ReadHeader (): ReadHeader failed due to not enough data being available.\n");
		} else {
			Media::Warning (MEDIA_INVALID_MEDIA, "asf_parser->ReadHeader () failed:");
			Media::Warning (MEDIA_FAIL, "%s", asf_parser->GetLastErrorStr ());
		}
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
	stream_to_asf_index = (gint32 *) g_malloc0 (sizeof (gint32) * (stream_count + 1)); 

	// Loop through all the streams and set stream-specific data	
	for (int i = 0; i < stream_count; i++) {
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
				} else {
					video->bit_rate = video->width*video->height;
				} 
			}
		} else if (stream_properties->is_command ()) {
			MarkerStream* marker = new MarkerStream (GetMedia ());
			stream = marker;
			stream->codec = g_strdup ("asf-marker");
		} else {
			// Unknown stream, ignore it.
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

MediaResult
ASFDemuxer::TryReadFrame (IMediaStream *stream, MediaFrame **f)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	ASFFrameReader *reader = this->reader->GetFrameReader (stream_to_asf_index [stream->index]);
	MediaFrame *frame;
	MediaResult result;
	
	result = reader->Advance ();
	if (result == MEDIA_NO_MORE_DATA) {
		//Media::Warning (MEDIA_NO_MORE_DATA, "Reached end of data.");
		return MEDIA_NO_MORE_DATA;
	}

	if (result == MEDIA_BUFFER_UNDERFLOW)
		return result;
	
	if (!MEDIA_SUCCEEDED (result)) {
		Media::Warning (MEDIA_DEMUXER_ERROR, "Error while advancing to the next frame (%d)", result);
		return result;
	}

	frame = new MediaFrame (stream);
	*f = frame;
	
	frame->pts = reader->Pts ();
	//frame->duration = reader->Duration ();
	if (reader->IsKeyFrame ())
		frame->AddState (FRAME_KEYFRAME);
	frame->buflen = reader->Size ();
	frame->buffer = (guint8 *) g_try_malloc (frame->buflen + frame->stream->min_padding);
	
	if (frame->buffer == NULL) {
		Media::Warning (MEDIA_OUT_OF_MEMORY, "Could not allocate memory for next frame.");
		return MEDIA_OUT_OF_MEMORY;
	}
	
	//printf ("ASFDemuxer::ReadFrame (%p), min_padding = %i\n", frame, frame->stream->min_padding);
	if (frame->stream->min_padding > 0)
		memset (frame->buffer + frame->buflen, 0, frame->stream->min_padding); 
	
	if (!reader->Write (frame->buffer)) {
		Media::Warning (MEDIA_DEMUXER_ERROR, "Error while copying the next frame.");
		return MEDIA_DEMUXER_ERROR;
	}
	
	frame->AddState (FRAME_DEMUXED);
	
	return MEDIA_SUCCESS;
}

/*
 * ASFMarkerDecoder
 */

MediaResult 
ASFMarkerDecoder::DecodeFrame (MediaFrame *frame)
{
	LOG_PIPELINE ("ASFMarkerDecoder::DecodeFrame ()\n");
	
	MediaResult result;
	char *text;
	char *type;
	gunichar2 *data;
	gunichar2 *uni_type = NULL;
	gunichar2 *uni_text = NULL;
	int text_length = 0;
	int type_length = 0;
	guint32 size = 0;
	
	if (frame->buflen % 2 != 0 || frame->buflen == 0 || frame->buffer == NULL)
		return MEDIA_CORRUPTED_MEDIA;

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
		
		LOG_PIPELINE ("ASFMarkerDecoder::DecodeFrame (): sending script command type: '%s', text: '%s', pts: '%llu'.\n", type, text, frame->pts);

		frame->marker = new MediaMarker (type, text, frame->pts);
		
		g_free (text);
		g_free (type);
		result = MEDIA_SUCCESS;
	} else {
		LOG_PIPELINE ("ASFMarkerDecoder::DecodeFrame (): didn't find 2 null characters in the data.\n");
		result = MEDIA_CORRUPTED_MEDIA;
	}

	return result;
}


/*
 * ASFDemuxerInfo
 */

MediaResult
ASFDemuxerInfo::Supports (IMediaSource *source)
{
	guint8 buffer[16];
	bool result;

	LOG_PIPELINE ("ASFDemuxerInfo::Supports (%p) pos: %lld, avail pos: %lld\n", source, source->GetPosition (), source->GetLastAvailablePosition ());

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

	if (MEDIA_SUCCEEDED (parser->Parse ())) {
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

MediaResult
ASXDemuxerInfo::Supports (IMediaSource *source)
{
	char buffer[4];
	bool result;
	
	if (!source->Peek ((guint8 *) buffer, 4))
		return MEDIA_FAIL;
	
	result = !g_ascii_strncasecmp (buffer, "<asx", 4) ||
		!g_ascii_strncasecmp (buffer, "[Ref", 4);

	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

IMediaDemuxer *
ASXDemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new ASXDemuxer (media, source);
}

/*
 * ManagedStreamSource
 */

#if SL_2_0

ManagedStreamSource::ManagedStreamSource (Media *media, ManagedStreamCallbacks *stream) : IMediaSource (media)
{
	memcpy (&this->stream, stream, sizeof (this->stream));
}

ManagedStreamSource::~ManagedStreamSource ()
{
	stream.handle = NULL;
}

gint32 
ManagedStreamSource::ReadInternal (void *buf, guint32 n)
{
	return stream.Read (stream.handle, buf, 0, n);
}

gint32 
ManagedStreamSource::PeekInternal (void *buf, guint32 n)
{
	int read;
	gint64 position;
	
	read = stream.Read (stream.handle, buf, 0, n);
	stream.Seek (stream.handle, -read, 1 /* SeekOrigin.Current */);
	return read;
}

bool 
ManagedStreamSource::SeekInternal (gint64 offset, int mode)
{
	stream.Seek (stream.handle, offset, mode /* FIXME: check if mode values matches SeekOrigin values */);
	return true;
}

gint64
ManagedStreamSource::GetPositionInternal ()
{
	return stream.Position (stream.handle);
}

gint64 
ManagedStreamSource::GetSizeInternal ()
{
	return stream.Length (stream.handle);
}
	
#endif

/*
 * FileSource
 */

FileSource::FileSource (Media *media, const char *filename) : IMediaSource (media)
{
	this->filename = g_strdup (filename);
	fd = NULL;
	size = 0;
	temp_file = false;
}

FileSource::FileSource (Media *media, bool temp_file) : IMediaSource (media)
{
	filename = NULL;
	fd = NULL;
	size = 0;
	this->temp_file = temp_file;
}

FileSource::~FileSource ()
{
	g_free (filename);
	if (fd != NULL)
		fclose (fd);
}

MediaResult 
FileSource::Initialize ()
{
	struct stat st;
	int tmp_fd;

	LOG_PIPELINE ("FileSource::Initialize ()\n");

	if (fd != NULL)
		return MEDIA_SUCCESS;
	
	if (temp_file) {
		if (filename != NULL)
			return MEDIA_FILE_ERROR;
	
		filename = g_build_filename (g_get_tmp_dir (), "MoonlightProgressiveStream.XXXXXX", NULL);
		
		if ((tmp_fd = g_mkstemp (filename)) == -1) {
			g_free (filename);
			filename = NULL;
			
			return MEDIA_FAIL;
		}

		fd = fdopen (tmp_fd, "r");

		setvbuf (fd, buffer, _IOFBF, sizeof (buffer));
	} else {
		if (filename == NULL)
			return MEDIA_FILE_ERROR;
			
		fd = fopen (filename, "r");
	}

	if (fd == NULL)
		return MEDIA_FILE_ERROR;

	size = 0;
	if (fstat (fileno (fd), &st) != -1)
		size = st.st_size;
		
	return MEDIA_SUCCESS;
}

gint64
FileSource::GetSizeInternal ()
{
	return size;
}

gint64
FileSource::GetPositionInternal ()
{
	gint64 result;
	
	if (fd == NULL)
		return -1;
	
	result = ftell (fd);

	LOG_PIPELINE ("FileSource::GetPositionInternal (): result: %lld\n", result);

	return result;
}

bool
FileSource::SeekInternal (gint64 offset, int mode)
{
	gint64 n;
	
	if (fd == NULL)
		return false;

	LOG_PIPELINE ("FileSource::SeekInternal (%lld, %i)\n", offset, mode);
	
	clearerr (fd);
	n = fseek (fd, offset, mode);

	return n != -1;
}

gint32
FileSource::ReadInternal (void *buf, guint32 n)
{
	ssize_t nread = 0;

	if (fd == NULL) {
		errno = EINVAL;
		LOG_PIPELINE_ERROR ("FileSource::ReadInternal (%p, %u): File not open.\n", buf, n);
		return -1;
	}

	clearerr (fd);
	nread = fread (buf, 1, n, fd);

	LOG_PIPELINE ("FileSource::ReadInternal (0x????????, %i), prev pos: %i, post pos: %i, nread: %i\n", (int) n, (int) current_position, (int) after_position, (int) nread);

	return nread;
}

gint32
FileSource::PeekInternal (void *buf, guint32 n)
{
	gint32 result;

	result = ReadSome (buf, n);
	
	Seek (-result, SEEK_CUR);

	LOG_PIPELINE ("FileSource<%i>::PeekInternal (%p, %i), GetPosition (): %lld [Done]\n", GET_OBJ_ID (this), buf, n, GetPosition ());

	return result;
}

bool
FileSource::Eof ()
{
	if (fd == NULL)
		return false;
	
	return feof (fd);
}

/*
 * ProgressiveSource
 */

ProgressiveSource::ProgressiveSource (Media *media) : FileSource (media, true)
{
	write_pos = 0;
	size = -1;
	write_fd = NULL;
}

ProgressiveSource::~ProgressiveSource ()
{
	if (write_fd != NULL)
		fclose (write_fd);
}

MediaResult
ProgressiveSource::Initialize ()
{
	MediaResult result;
	
	if (filename != NULL)
		return MEDIA_SUCCESS;

	result = FileSource::Initialize ();

	if (!MEDIA_SUCCEEDED (result))
		return result;

	write_fd = fopen (filename, "w");
	if (write_fd == NULL) {
		fprintf (stderr, "Moonlight: Could not open a write handle to the file '%s'\n", filename);
		return MEDIA_FAIL;
	}

	// unlink the file right away so that it'll be deleted even if we crash.
	if (moonlight_flags & RUNTIME_INIT_KEEP_MEDIA) {
		printf ("Moonlight: The media file %s will not deleted.\n", filename);
	} else {
		unlink (filename);
	}
	
	return MEDIA_SUCCESS;
}

void
ProgressiveSource::Write (void *buf, gint64 offset, gint32 n)
{
	size_t nwritten;
	
	LOG_PIPELINE ("ProgressiveSource::Write (%p, %lld, %i) media: %p, filename: %s\n", buf, offset, n, media, filename);
	if (write_fd == NULL) {
		Media::Warning (MEDIA_FAIL, "Progressive source doesn't have a file to write the data to.");
		return;
	}
	
	if (n == 0) {
		// We've got the entire file, update the size
		size = write_pos; // Since this method is the only method that writes to write_pos, and we're not reentrant, there is no need to lock here.
		goto cleanup;
	}

	nwritten = fwrite (buf, 1, n, write_fd);
	fflush (write_fd);

	Lock ();
	write_pos += nwritten;
	Unlock ();


cleanup:
	if (media)
		media->WakeUp ();

}

void
ProgressiveSource::NotifySize (gint64 size)
{
	Lock ();
	this->size = size;
	Unlock ();
}

void
ProgressiveSource::NotifyFinished ()
{
	Lock ();
	this->size = write_pos;
	Unlock ();
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
 * MemoryNestedSource
 */

MemoryNestedSource::MemoryNestedSource (MemorySource *src) : MemorySource (src->GetMedia (), src->GetMemory (), src->GetSize (), src->GetStart ())
{
	src->ref ();
	this->src = src;
}

MemoryNestedSource::~MemoryNestedSource ()
{
	src->unref ();
}

/*
 * MemoryQueueSource
 */
 
MemorySource::MemorySource (Media *media, void *memory, gint32 size, gint64 start)
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
MemorySource::SeekInternal (gint64 offset, int mode)
{
	gint64 real_offset;

	switch (mode) {
	case SEEK_SET:
		real_offset = offset - start;
		if (real_offset < 0 || real_offset >= size)
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

gint32 
MemorySource::ReadInternal (void *buffer, guint32 n)
{
	guint32 k = MIN (n, size - pos);
	memcpy (buffer, ((char*) memory) + pos, k);
	pos += k;
	return k;
}

gint32
MemorySource::PeekInternal (void *buffer, guint32 n)
{
	gint64 start = this->start + pos;

	if (this->start > start)
		return 0;

	if ((this->start + size) < (start + n))
		return 0;

	memcpy (buffer, ((char*) memory) + this->start - start, n);
	return n;
}

/*
 * MemoryQueueSource
 */
 
MemoryQueueSource::MemoryQueueSource (Media *media)
	: IMediaSource (media)
{
	finished = false;
	write_count = 0;
	parser = NULL;
	queue = new Queue ();
}

MemoryQueueSource::~MemoryQueueSource ()
{
}

void
MemoryQueueSource::Dispose ()
{
	IMediaSource::Dispose ();
	if (parser) {
		parser->unref ();
		parser = NULL;
	}
	if (queue) {
		delete queue;
		queue = NULL;
	}
}

ASFParser *
MemoryQueueSource::GetParser ()
{
	return parser;
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
			LOG_PIPELINE_ERROR ("MemoryQueueSource::GetQueue (): Error while parsing packet, dropping packet.\n");
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
		LOG_PIPELINE ("MemoryQueueSource::Pop (): No more packets (for now).\n");
		return NULL;
	}
	
	if (node->packet == NULL) {
		if (parser == NULL) {
			g_warning ("MemoryQueueSource::Pop (): No parser to parse the packet.\n");
			goto cleanup;
		}
		node->packet = new ASFPacket (parser, node->source);
		if (!MEDIA_SUCCEEDED (node->packet->Read ())) {
			LOG_PIPELINE_ERROR ("MemoryQueueSource::Pop (): Error while parsing packet, getting a new packet\n");
			delete node;
			goto trynext;
		}
	}
	
	result = node->packet;
	result->ref ();

cleanup:				
	delete node;
	
	LOG_PIPELINE ("MemoryQueueSource::Pop (): popped 1 packet, there are %i packets left, of a total of %lld packets written\n", queue->Length (), write_count);
	
	return result;
}

void
MemoryQueueSource::Write (void *buf, gint64 offset, gint32 n)
{
	MemorySource *src;
	ASFPacket *packet;
	
	LOG_PIPELINE ("MemoryQueueSource::Write (%p, %lld, %i), write_count: %lld\n", buf, offset, n, write_count + 1);

	if (!queue)
		return;

	write_count++;
	if (parser != NULL) {
		src = new MemorySource (NULL, buf, n, offset);
		src->SetOwner (false);
		packet = new ASFPacket (parser, src);
		if (!MEDIA_SUCCEEDED (packet->Read ())) {
			LOG_PIPELINE_ERROR ("MemoryQueueSource::Write (%p, %lld, %i): Error while parsing packet, dropping packet.\n", buf, offset, n);
		} else {
			queue->Push (new QueueNode (packet));
		}
		packet->unref ();
		src->unref ();
	} else {
		src = new MemorySource (NULL, g_memdup (buf, n), n, offset);
		queue->Push (new QueueNode (src));
		src->unref ();
	}

	if (media)
		media->WakeUp ();
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

	LOG_PIPELINE ("MemoryQueueSource::SeekToPts (%llu)\n", pts);

	if (queue) {
		queue->Clear (true);
		Downloader *dl = media->GetDownloader ();
		InternalDownloader *idl = dl->GetInternalDownloader ();
		MmsDownloader *mms;
		if (idl->GetType () == InternalDownloader::MmsDownloader) {
			mms = (MmsDownloader *) idl;
			mms->SetRequestedPts (pts);
		} else {
			fprintf (stderr, "Moonlight: media assert failure (downloader's internal downloader isn't a mms downloader)\n");
		}
		finished = false;
		result = MEDIA_SUCCESS;
	} else {
		result = MEDIA_FAIL;
	}
	
	return result;
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
	context_refcounted = true;
}

MediaClosure::~MediaClosure ()
{
	delete frame;

	if (context_refcounted && context)
		context->unref ();
	if (media)
		media->unref ();
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
MediaClosure::SetContextUnsafe (EventObject *context)
{
	if (this->context && context_refcounted)
		this->context->unref ();
		
	this->context = context;
	context_refcounted = false;
}

void
MediaClosure::SetContext (EventObject *context)
{
	if (this->context && context_refcounted)
		this->context->unref ();
	this->context = context;
	if (this->context)
		this->context->ref ();
	context_refcounted = true;
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
	queue = new Queue ();
	
	first_pts = G_MAXUINT64; // The first pts in the stream, initialized to G_MAXUINT64
	last_popped_pts = G_MAXUINT64; // The pts of the last frame returned, initialized to G_MAXUINT64
	last_enqueued_pts = G_MAXUINT64; // The pts of the last frame enqueued, initialized to G_MAXUINT64
}

IMediaStream::~IMediaStream ()
{
	if (decoder)
		decoder->unref ();
	
	g_free (extra_data);
	g_free (codec);
	delete queue;
}

guint64
IMediaStream::GetBufferedSize ()
{
	guint64 result;
	
	queue->Lock ();
	if (first_pts == G_MAXUINT64 || last_enqueued_pts == G_MAXUINT64)
		result = 0;
	else if (last_popped_pts == G_MAXUINT64)
		result = last_enqueued_pts - first_pts;
	else
		result = last_enqueued_pts - last_popped_pts;
	queue->Unlock ();

	LOG_BUFFERING ("IMediaStream::GetBufferedSize (): codec: %s, first_pts: %llu ms, last_popped_pts: %llu ms, last_enqueued_pts: %llu ms, result: %llu ms\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), MilliSeconds_FromPts (result));

	return result;
}


#if DEBUG
#define TO_MS(x) (MilliSeconds_FromPts (x) == 1844674407370955ULL ? -1 : MilliSeconds_FromPts (x))

void
IMediaStream::PrintBufferInformation ()
{
	guint64 buffer_size = GetBufferedSize ();
	
	printf (" <%s: ", codec);
	
	if (GetSelected ()) {
		printf ("size: %.4llu, first: %.4lli, last popped: %.4lli, last enq: %.4lli, frames enq: %i>",
			TO_MS (buffer_size), TO_MS (first_pts), TO_MS (last_popped_pts), 
			TO_MS (last_enqueued_pts), queue ? queue->Length () : -1);
	} else {
		printf ("(not selected) >");
	}
}
#endif

void
IMediaStream::EnqueueFrame (MediaFrame *frame)
{
	queue->Lock ();
	if (first_pts == G_MAXUINT64)
		first_pts = frame->pts;

#if 0
	if (last_enqueued_pts > frame->pts && last_enqueued_pts != G_MAXUINT64 && frame->event != FrameEventEOF && frame->buflen > 0) {
		g_warning ("IMediaStream::EnqueueFrame (): codec: %.5s, first_pts: %llu ms, last_popped_pts: %llu ms, last_enqueued_pts: %llu ms, "
		"buffer: %llu ms, frame: %p, frame->buflen: %i, frame->pts: %llu ms (the last enqueued frame's pts is below the current frame's pts)\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), frame, frame->buflen, MilliSeconds_FromPts (frame->pts));
	}
#endif

	if (frame->event != FrameEventEOF)
		last_enqueued_pts = frame->pts;
	queue->LinkedList ()->Append (new StreamNode (frame));
	queue->Unlock ();

	LOG_BUFFERING ("IMediaStream::EnqueueFrame (): codec: %.5s, first_pts: %llu ms, last_popped_pts: %llu ms, last_enqueued_pts: %llu ms, buffer: %llu ms, frame: %p, frame->buflen: %i\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts - first_pts), frame, frame->buflen);
}

MediaFrame *
IMediaStream::PopFrame ()
{
	MediaFrame *result = NULL;
	StreamNode *node = NULL;

	// We use the queue lock to synchronize access to
	// last_popped_pts/last_enqueued_pts/first_pts

	queue->Lock ();
	node = (StreamNode *) queue->LinkedList ()->First ();
	if (node != NULL) {
		result = node->frame;
		node->frame = NULL;
		
		queue->LinkedList ()->Remove (node);
		last_popped_pts = result->pts;
	}
	queue->Unlock ();
	
	LOG_BUFFERING ("IMediaStream::PopFrame (): codec: %.5s, first_pts: %llu ms, last_popped_pts: %llu ms, last_enqueued_pts: %llu ms, buffer: %llu ms, frame: %p, frame->buflen: %i\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), result, result ? result->buflen : 0);

	return result;
}

void
IMediaStream::ClearQueue ()
{
	LOG_BUFFERING ("IMediaStream::ClearQueue ()\n");
	if (queue) {
		queue->Lock ();
		queue->LinkedList ()->Clear (true);
		first_pts = G_MAXUINT64;
		last_popped_pts = G_MAXUINT64;
		last_enqueued_pts = G_MAXUINT64;
		queue->Unlock ();
	}
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

void
IMediaDemuxer::FillBuffers ()
{
	IMediaStream *stream;
	MediaFrame *frame;
	MediaResult result = MEDIA_SUCCESS;
	guint64 buffering_time = media->GetBufferingTime ();
	
	LOG_BUFFERING ("IMediaDemuxer::FillBuffers ()\n");

	for (int i = 0; i < GetStreamCount (); i++) {
		stream = GetStream (i);
		if (!stream->GetSelected ())
			continue;

		if (stream->GetType () != MediaTypeVideo && 
			stream->GetType () != MediaTypeAudio)
			continue;

		if (stream->GetBufferedSize () >= buffering_time)
			continue;

		while (stream->GetBufferedSize () < buffering_time) {
			LOG_BUFFERING ("IMediaDemuxer::FillBuffers (): codec: %s, result: %i, buffered size: %llu ms, buffering time: %llu ms\n",
				stream->codec, result, MilliSeconds_FromPts (stream->GetBufferedSize ()), MilliSeconds_FromPts (buffering_time));

			frame = NULL;
			result = TryReadFrame (stream, &frame);
			if (MEDIA_SUCCEEDED (result)) {
				stream->EnqueueFrame (frame);
			} else if (result == MEDIA_NO_MORE_DATA) {
				if (frame != NULL) {
					g_warning ("IMediaDemuxer::FillBuffers (): we shouldn't get a frame back when there's no more data.\n");
					delete frame;
				}
				frame = new MediaFrame (stream);
				frame->event = FrameEventEOF;
				stream->EnqueueFrame (frame);
				break;
			} else {
				if (frame != NULL)
					delete frame;
				break;
			}
		}
		
		LOG_BUFFERING ("IMediaDemuxer::FillBuffers (): codec: %s, result: %i, buffered size: %llu ms, buffering time: %llu ms, last popped time: %llu ms\n", 
				stream->codec, result, MilliSeconds_FromPts (stream->GetBufferedSize ()), MilliSeconds_FromPts (buffering_time), MilliSeconds_FromPts (stream->GetLastPoppedPts ()));
	}
	
	LOG_BUFFERING ("IMediaDemuxer::FillBuffers () [Done]. BufferedSize: %llu ms\n", GetBufferedSize ());
}

guint64
IMediaDemuxer::GetBufferedSize ()
{
	guint64 result = G_MAXUINT64;
	IMediaStream *stream;
	
	for (int i = 0; i < GetStreamCount (); i++) {
		stream = GetStream (i);
		if (!stream->GetSelected ())
			continue;

		result = MIN (result, stream->GetBufferedSize ());
	}

	return result;
}

#if DEBUG
void
IMediaDemuxer::PrintBufferInformation ()
{
	printf ("Buffer: %lld", MilliSeconds_FromPts (GetBufferedSize ()));
	for (int i = 0; i < GetStreamCount (); i++) {
		GetStream (i)->PrintBufferInformation ();
	}
	printf ("\n");
}
#endif

guint64
IMediaDemuxer::GetDuration ()
{
	guint64 result = 0;
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
	this->marker = NULL;
	
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
	if (decoder_specific_data != NULL) {
		if (stream != NULL && stream->decoder != NULL)
			stream->decoder->Cleanup (this);
	}
	g_free (buffer);
	if (marker)
		marker->unref ();
}

/*
 * IMediaObject
 */
 
IMediaObject::IMediaObject (Media *media)
{
	this->media = media;
	if (this->media)
		this->media->ref ();
}

void
IMediaObject::Dispose ()
{
	EventObject::Dispose ();
	if (media) {
		media->unref ();
		media = NULL;
	}
}

void
IMediaObject::SetMedia (Media *value)
{
	if (media)
		media->unref ();
	media = value;
	if (media)
		media->ref ();
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

gint32
IMediaSource::ReadSome (void *buf, guint32 n)
{
	gint32 result;

	LOG_PIPELINE ("IMediaSource<%i>::ReadSome (%p, %u)\n", GET_OBJ_ID (this), buf, n);

	Lock ();

	result = ReadInternal (buf, n);

	LOG_PIPELINE ("IMediaSource<%i>::ReadSome (%p, %u) read %i, position: %lld\n", GET_OBJ_ID (this), buf, n, result, GetPosition ());

	Unlock ();

	return result;
}

bool
IMediaSource::ReadAll (void *buf, guint32 n)
{
	gint32 read;
	gint64 prev = GetPosition ();
	gint64 avail = GetLastAvailablePosition ();
	
	//printf ("IMediaSource::ReadAll (%p, %u), position: %lld\n", buf, n, prev);
	
	read = ReadSome (buf, n);
	
	if ((gint64) read != (gint64) n) {
		FileSource *fs = (FileSource *) this;
		g_warning ("IMediaSource::ReadInternal (%i): Read failed, read %i bytes. available size: %lld, size: %lld, pos: %lld, prev pos: %lld, position not available: %lld, feof: %i, ferror: %i, strerror: %s\n", 
			n, read, avail, GetSize (), GetPosition (), prev, prev + n, feof (fs->fd), ferror (fs->fd), strerror (ferror (fs->fd)));
		print_stack_trace ();
		exit (-1);
	}
	
	LOG_PIPELINE ("IMediaSource<%d>::ReadAll (%p, %u), read: %d [Done].\n",
		      GET_OBJ_ID (this), buf, n, read);
	
	return (gint64) read == (gint64) n;
}

bool
IMediaSource::Peek (void *buf, guint32 n)
{
	bool result;
	gint64 read;
	
	Lock ();

	read = PeekInternal (buf, n);
	result = read == (gint64) n;

	Unlock ();

	if (!result)
		printf ("IMediaSource::Peek (%p, %u): peek failed, read %lld bytes.\n", buf, n, read);

	return result;
}

bool
IMediaSource::Seek (gint64 offset, int mode)
{
	LOG_PIPELINE ("IMediaSource<%d> (%s)::Seek (%lld, %d = %s)\n",
		      GET_OBJ_ID (this), ToString (), offset, mode, mode == SEEK_SET ? "SEEK_SET"
		      : (mode == SEEK_CUR ? "SEEK_CUR" : (mode == SEEK_END ? "SEEK_END" : "<invalid value>")));
	
	bool result;
	Lock ();
	result = SeekInternal (offset, mode);
	Unlock ();
	return result;
}

bool
IMediaSource::IsPositionAvailable (gint64 position, bool *eof)
{
	gint64 available = GetLastAvailablePosition ();
	gint64 size = GetSize ();

	*eof = false;

	if (size != -1 && size < position) {
		// Size is known and smaller than the requested position
		*eof = true;
		return false;
	}

	if (available != -1 && available < position) {
		// Not everything is available and the available position is smaller than the requested position
		*eof = false;
		return false;
	}

	if (size == -1 && available == -1) {
		// Size is not known, but everything is available??
		// This is probably due to a bug in the derived *Source class
		*eof = false;
		fprintf (stderr, "Moonlight: media assert error (invalid source size), media playback errors will probably occur\n");
		return false;
	}

	return true;
}

gint64
IMediaSource::GetLastAvailablePosition ()
{
	gint64 result;
	Lock ();
	result = GetLastAvailablePositionInternal ();
	Unlock ();
	return result;
}

gint64
IMediaSource::GetPosition ()
{
	gint64 result;
	Lock ();
	result = GetPositionInternal ();
	Unlock ();
	return result;
}

gint64
IMediaSource::GetSize ()
{
	gint64 result;
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

MediaResult
IMediaDemuxer::Seek (guint64 pts)
{
	MediaResult result;
	
	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaStream *stream = GetStream (i);
		stream->ClearQueue ();
		if (stream->decoder != NULL)
			stream->decoder->CleanState ();
	}

	LOG_PIPELINE ("IMediaDemuxer::Seek (%llu)\n", MilliSeconds_FromPts (pts));

	result = SeekInternal (pts);

#if DEBUG
	for (int i = 0; i < GetStreamCount (); i++) {
		if (GetStream (i)->PopFrame () != NULL) {
			g_warning ("IMediaDemuxer::Seek (): We got frames while we were seeking.\n");
		}
	}
#endif

	return result;
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

MediaMarker::MediaMarker (const char *type, const char *text, guint64 pts)
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
	closure = NULL;
}

void
MarkerStream::MarkerFound (MediaFrame *frame)
{
	MediaResult result;
	
	if (decoder == NULL) {
		LOG_PIPELINE ("MarkerStream::MarkerFound (): Got marker, but there's no decoder for the marker.\n");
		return;
	}
	
	result = decoder->DecodeFrame (frame);
	
	if (!MEDIA_SUCCEEDED (result)) {
		LOG_PIPELINE ("MarkerStream::MarkerFound (): Error while decoding marker: %i\n", result);
		return;
	}
	
	if (closure == NULL) {
		LOG_PIPELINE ("MarkerStream::MarkerFound (): Got decoded marker, but nobody is waiting for it.\n");
		return;
	}
	
	closure->marker = frame->marker;
	if (closure->marker)
		closure->marker->ref ();
	closure->Call ();
	if (closure->marker)
		closure->marker->unref ();
	closure->marker = NULL;
}

void
MarkerStream::SetCallback (MediaClosure *closure)
{
	this->closure = closure;
}

/*
 * MediaWork
 */ 
MediaWork::MediaWork (MediaClosure *closure, IMediaStream *stream, guint16 states)
{
	switch (stream->GetType ()) {
	case MediaTypeVideo:
		type = WorkTypeVideo; break;
	case MediaTypeAudio:
		type = WorkTypeAudio; break;
	case MediaTypeMarker:
		type = WorkTypeMarker; break;
	default:
		fprintf (stderr, "MediaWork::MediaWork (%p, %p, %i): Invalid stream type %u\n",
			 closure, stream, (uint) states, stream->GetType ());
		break;
	}
	this->closure = closure;
	this->data.frame.states = states;
	this->data.frame.stream = stream;
	this->data.frame.stream->ref ();
}

MediaWork::MediaWork (MediaClosure *closure, guint64 seek_pts)
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
		if (data.open.source)
			data.open.source->unref ();
		break;
	case WorkTypeVideo:
	case WorkTypeAudio:
	case WorkTypeMarker:
		if (data.frame.stream)
			data.frame.stream->unref ();
		break;
	case WorkTypeSeek:
		break; // Nothing to do
	}
	delete closure;

#if DEBUG
	memset (&data, 0, sizeof (data));
#endif
}

/*
 * NullDecoderInfo
 */

bool
NullDecoderInfo::Supports (const char *codec)
{
	const char *video_fourccs [] = { "wmv1", "wmv2", "wmv3", "wmva", "vc1", NULL };
	const char *audio_fourccs [] = { "wmav1","wmav2", "mp3", NULL};
	
	for (int i = 0; video_fourccs [i] != NULL; i++)
		if (!strcmp (codec, video_fourccs [i]))
			return true;

	for (int i = 0; audio_fourccs [i] != NULL; i++)
		if (!strcmp (codec, audio_fourccs [i]))
			return true;


	return false;
}

/*
 * NullDecoder
 */

NullDecoder::NullDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream)
{
	logo = NULL;
	logo_size = 0;
	prev_pts = G_MAXUINT64;
}

NullDecoder::~NullDecoder ()
{
	g_free (logo);
}

MediaResult
NullDecoder::DecodeVideoFrame (MediaFrame *frame)
{
	// free encoded buffer and alloc a new one for our image
	g_free (frame->buffer);
	frame->buflen = logo_size;
	frame->buffer = (guint8*) g_malloc (frame->buflen);
	memcpy (frame->buffer, logo, frame->buflen);
	frame->AddState (FRAME_DECODED);
	
	//printf ("NullVideoDecoder::DecodeFrame () pts: %llu, w: %i, h: %i\n", frame->pts, w, h);
	
	return MEDIA_SUCCESS;
}

MediaResult
NullDecoder::DecodeAudioFrame (MediaFrame *frame)
{
	AudioStream *as = (AudioStream *) stream;
	guint32 samples;
	guint32 data_size;
	guint64 diff_pts;
	
	// discard encoded data
	g_free (frame->buffer);

	// We have no idea here how long the encoded audio data is
	// for the first frame we use 0.1 seconds, for the rest
	// we calculate the time since the last frame

	if (prev_pts == G_MAXUINT64 || frame->pts <= prev_pts) {
		samples = as->sample_rate / 10; // start off sending 0.1 seconds of audio
	} else {
		diff_pts = frame->pts - prev_pts;
		samples = (float) as->sample_rate / (TIMESPANTICKS_IN_SECOND_FLOAT / (float) diff_pts);
	}
	prev_pts = frame->pts;

	data_size  = samples * as->channels * 2 /* 16 bit audio */;

	frame->buflen = data_size;
	frame->buffer = (guint8 *) g_malloc0 (frame->buflen);
	
	frame->AddState (FRAME_DECODED);
	
	return MEDIA_SUCCESS;
}

MediaResult
NullDecoder::DecodeFrame (MediaFrame *frame)
{
	if (stream->GetType () == MediaTypeAudio)
		return DecodeAudioFrame (frame);
	else if (stream->GetType () == MediaTypeVideo)
		return DecodeVideoFrame (frame);
	else
		return MEDIA_FAIL;
}

MediaResult
NullDecoder::Open ()
{
	if (stream->GetType () == MediaTypeAudio)
		return OpenAudio ();
	else if (stream->GetType () == MediaTypeVideo)
		return OpenVideo ();
	else
		return MEDIA_FAIL;
}

MediaResult
NullDecoder::OpenAudio ()
{
	return MEDIA_SUCCESS;
}

MediaResult
NullDecoder::OpenVideo ()
{
	VideoStream *vs = (VideoStream *) stream;
	guint32 dest_height = vs->height;
	guint32 dest_width = vs->width;
	guint32 dest_i = 0;
	
	// We assume that the input image is a 24 bit bitmap (bmp), stored bottum up and flipped vertically.
	extern const char moonlight_logo [];
	const char *image = moonlight_logo;

	guint32 img_offset = *((guint32*)(image + 10));
	guint32 img_width  = *((guint32*)(image + 18));
	guint32 img_height = *((guint32*)(image + 22));
	guint32 img_stride = (img_width * 3 + 3) & ~3; // in bytes
	guint32 img_i, img_h, img_w;
	
	LOG_PIPELINE ("offset: %i, width: 0x%x = %i, height: 0x%x = %i, stride: %i\n", img_offset, img_width, img_width, img_height, img_height, img_stride);
	
	// create the buffer for our image
	logo_size = dest_height * dest_width * 4;
	logo = (guint8*) g_malloc (logo_size);

	// write our image tiled into the destination rectangle, flipped horizontally
	dest_i = 4;
	for (guint32 dest_h = 0; dest_h < dest_height; dest_h++) {
		for (guint32 dest_w = 0; dest_w < dest_width; dest_w++) {
			img_h = dest_h % img_height;
			img_w = dest_w % img_width;
			img_i = img_h * img_stride + img_w * 3;
			
			logo [logo_size - dest_i + 0] = image [img_offset + img_i + 0];
			logo [logo_size - dest_i + 1] = image [img_offset + img_i + 1];
			logo [logo_size - dest_i + 2] = image [img_offset + img_i + 2];
			logo [logo_size - dest_i + 3] = 255;
			
			dest_i += 4;
		}
	}
	
	// Flip the image vertically
	for (guint32 dest_h = 0; dest_h < dest_height; dest_h++) {
		for (guint32 dest_w = 0; dest_w < dest_width / 2; dest_w++) {
			guint32 tmp;
			guint32 a = (dest_h * dest_width + dest_w) * 4;
			guint32 b = (dest_h * dest_width + dest_width - dest_w) * 4 - 4;
			for (guint32 c = 0; c < 3; c++) {
				tmp = logo [a + c];
				logo [a + c] = logo [b + c];
				logo [b + c] = tmp;
			}
		}
	}

	pixel_format = MoonPixelFormatRGB32;
	
	return MEDIA_SUCCESS;
}
