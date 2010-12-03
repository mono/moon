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

#include <glib/gstdio.h>
#include <errno.h>

#include "audio.h"
#include "pipeline.h"
#include "pipeline-ffmpeg.h"
#include "mp3.h"
#include "uri.h"
#include "media.h"
#include "mediaelement.h"
#include "yuv-converter.h"
#include "runtime.h"
#include "pipeline-asf.h"
#include "playlist.h"
#include "deployment.h"
#include "timesource.h"
#include "pipeline-mp4.h"
#include "factory.h"

#if CODECS_SUPPORTED
#include "pipeline-ui.h"
#endif

extern const char moonlight_logo [];

namespace Moonlight {

/*
 * Media
 */

bool Media::registering_ms_codecs = false;
bool Media::registered_ms_codecs = false;

DemuxerInfo *Media::registered_demuxers = NULL;
DecoderInfo *Media::registered_decoders = NULL;
ConverterInfo *Media::registered_converters = NULL;

Media::Media (PlaylistRoot *root)
	: IMediaObject (Type::MEDIA, this)
{
	LOG_PIPELINE ("Media::Media (), id: %i\n", GET_OBJ_ID (this));

	playlist = root;
	buffering_time = 0;
	file = NULL;
	uri = NULL;
	resource_base = NULL;
	source = NULL;
	demuxer = NULL;
	markers = NULL;

	is_disposed = false;
	initialized = false;	
	opened = false;
	opening = false;
	stopped = false;
	error_reported = false;
	in_open_internal = false;
	http_retried = false;
	download_progress = 0.0;
	buffering_progress = 0.0;
	target_pts = 0;
	start_time = 0;
	
	if (!GetDeployment ()->RegisterMedia (this))
		Dispose ();
}

Media::~Media ()
{
	LOG_PIPELINE ("Media::~Media (), id: %i\n", GET_OBJ_ID (this));
}

void
Media::Dispose ()
{
	IMediaSource *src;
	IMediaDemuxer *dmx;
	bool was_disposed = false;
	
	LOG_PIPELINE ("Media::Dispose (), id: %i\n", GET_OBJ_ID (this));

	mutex.Lock ();
	was_disposed = is_disposed;
	is_disposed = true;
	mutex.Unlock ();
	
	/* 
	 * Don't run our dispose code more than once, we may end up deleted on the main thread
	 * which will cause Dispose to be called on the main thread too. Since Dispose must already
	 * have been called on the media thread, we're safe.
	 */
	if (was_disposed) {
		IMediaObject::Dispose ();
		return;
	}

#if SANITY
	if (!MediaThreadPool::IsThreadPoolThread ()) {
		g_warning ("Media::Dispose (): Not in thread-pool thread, and we haven't been disposed already.\n");
	}
#endif

	ClearQueue ();
		
	/* 
	 * We're on a media thread, and there is no other work in the queue: we can ensure that nothing
	 * more will ever execute on the media thread related to this Media instance.
	 */

	g_free (file);
	file = NULL;
	delete uri;
	uri = NULL;
	delete resource_base;
	resource_base = NULL;

	src = this->source;
	this->source = NULL;
	if (src) {
		src->Dispose ();
		src->unref ();
	}

	mutex.Lock ();
	dmx = this->demuxer;
	this->demuxer = NULL;	
	mutex.Unlock ();
	if (dmx) {
		dmx->Dispose ();
		dmx->unref ();
	}
	
	delete markers;
	markers = NULL;
	
	IMediaObject::Dispose ();

	GetDeployment ()->UnregisterMedia (this);
}

void
Media::SetStartTime (TimeSpan value)
{
	VERIFY_MAIN_THREAD;
	g_return_if_fail (!initialized);
	start_time = value;
}

bool
Media::IsMSCodecsInstalled ()
{
	return registered_ms_codecs;
}

void
Media::InstallMSCodecs (bool is_user_initiated)
{
#if CODECS_SUPPORTED
	CodecDownloader::ShowUI (Deployment::GetCurrent ()->GetSurface (), is_user_initiated);
#else
	runtime_get_windowing_system ()->ShowCodecsUnavailableMessage ();
#endif
}

void
Media::SetBufferingTime (guint64 buffering_time)
{
	mutex.Lock ();
	this->buffering_time = buffering_time;
	mutex.Unlock ();
}

guint64
Media::GetBufferingTime ()
{
	guint64 result;
	mutex.Lock ();
	result = buffering_time;
	mutex.Unlock ();
	return result;
}

PlaylistRoot *
Media::GetPlaylistRoot ()
{
	return playlist;
}

IMediaDemuxer *
Media::GetDemuxerReffed ()
{
	IMediaDemuxer *result;
	mutex.Lock ();
	result = this->demuxer;
	if (result)
		result->ref ();
	mutex.Unlock ();
	return result;
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
	info->next = NULL;
	if (registered_demuxers == NULL) {
		registered_demuxers = info;
	} else {
		MediaInfo* current = registered_demuxers;
		while (current->next != NULL)
			current = current->next;
		current->next = info;
	}
	LOG_DEMUXERS ("Moonlight: Demuxer has been registered: %s\n", info->GetName ());
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
	LOG_CODECS ("Moonlight: Codec has been registered: %s\n", info->GetName ());
}

void
Media::Initialize ()
{
	LOG_PIPELINE ("Media::Initialize ()\n");
	
	// demuxers
	Media::RegisterDemuxer (new ASFDemuxerInfo ());
	Media::RegisterDemuxer (new Mp4DemuxerInfo ());
	Media::RegisterDemuxer (new Mp3DemuxerInfo ());
	Media::RegisterDemuxer (new ASXDemuxerInfo ());

	// converters
	if (!(moonlight_flags & RUNTIME_INIT_FFMPEG_YUV_CONVERTER))
		Media::RegisterConverter (new YUVConverterInfo ());

	// decoders
	Media::RegisterDecoder (new ASFMarkerDecoderInfo ());
	if (moonlight_flags & RUNTIME_INIT_ENABLE_MS_CODECS) {
#if CODECS_SUPPORTED
		RegisterMSCodecs ();
#endif
	}
#ifdef INCLUDE_FFMPEG
	if (!(moonlight_flags & RUNTIME_INIT_DISABLE_FFMPEG_CODECS)) {
		register_ffmpeg ();
	}
#endif
	
	Media::RegisterDecoder (new PassThroughDecoderInfo ());
	Media::RegisterDecoder (new NullDecoderInfo ());
	
	MediaThreadPool::Initialize ();
}

void
Media::Shutdown ()
{
	LOG_PIPELINE ("Media::Shutdown ()\n");

	MediaInfo *current;
	MediaInfo *next;
	
	// Make sure all threads are stopped
	AudioPlayer::Shutdown ();
	MediaThreadPool::Shutdown ();
	
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

	LOG_PIPELINE ("Media::Shutdown () [Done]\n");
}

bool
Media::InMediaThread ()
{
	return MediaThreadPool::IsThreadPoolThread ();
}

MediaResult
Media::ClearBufferingProgressCallback (MediaClosure *closure)
{
	closure->GetMedia ()->ClearBufferingProgress ();
	return MEDIA_SUCCESS;
}

void
Media::ClearBufferingProgress ()
{
	/* To avoid having to lock around more variables, just marshal this to the media thread */
	if (!InMediaThread ()) {
		MediaClosure *closure = new MediaClosure (this, ClearBufferingProgressCallback, this, "Media::ClearBufferingProgress");
		EnqueueWork (closure);
		closure->unref ();
		return;
	}

	/* We can't just set buffering_progress to 0 and hope that we haven't reached the end of the media, which
	 * would cause us to never restart playback. We can't just emit BufferingProgressChangedEvent either, we may
	 * still have a long way to 100% buffer. So check if we have a stream that hasn't finished, in which case
	 * reset the buffering progress to 0, otherwise emit the (last) BufferingProgressChanged event */
	 if (demuxer != NULL) {
		 for (int i = 0; i < demuxer->GetStreamCount (); i++) {
			 IMediaStream *stream = demuxer->GetStream (i);

			 if (stream == NULL || !stream->GetSelected ())
				continue;

			 if (stream->GetOutputEnded () == false) {
				 LOG_PIPELINE ("Media::ClearBufferingProgress () %s hasn't ended, we can clear buffering_progress\n", stream->GetTypeName ());
				 buffering_progress = 0.0;
				 demuxer->FillBuffers ();
				 return;
			 }
		 }
	 }

	/* All streams have ended their output, buffering_progress won't change anymore. Emit the last 
	 * BufferingProgressChanged event */
	LOG_PIPELINE ("Media::ClearBufferingProgress (): All streams have ended, emit BufferingProgressChangedEvent (%.2f).\n", buffering_progress);
	EmitSafe (BufferingProgressChangedEvent, new ProgressEventArgs (buffering_progress, 0.0));
}

void
Media::ReportBufferingProgress (double progress)
{
	LOG_BUFFERING ("Media::ReportBufferingProgress (%.3f), buffering_progress: %.3f\n", progress, buffering_progress);
	
	progress = MAX (MIN (progress, 1.0), 0.0);
	
	if (progress == buffering_progress)
		return;
	
	if (progress < buffering_progress || progress > (buffering_progress + 0.005) || progress == 1.0 || progress == 0.0) {
		buffering_progress = progress;
		EmitSafe (BufferingProgressChangedEvent, new ProgressEventArgs (progress, 0.0));
	}
}

void
Media::ReportDownloadProgress (double progress, double offset, bool force)
{
	LOG_PIPELINE ("Media::ReportDownloadProgress (%.3f, %i), download_progress: %.3f\n", progress, force, download_progress);

	progress = MAX (MIN (progress, 1.0), 0.0);
	
	if (progress <= download_progress && !force) {
		/*
		 * Download progress percentage can actually go down - if the file size
		 * goes up. Yes, the file size can go up.
		 */
		return;
	}

	if (progress > (download_progress + 0.005) || progress == 1.0 || progress == 0.0) {
		download_progress = progress;
		EmitSafe (DownloadProgressChangedEvent, new ProgressEventArgs (progress, offset));
	}
}

void
Media::SeekAsync (guint64 pts)
{
	LOG_PIPELINE ("Media::SeekAsync (%" G_GUINT64_FORMAT "), id: %i\n", pts, GET_OBJ_ID (this));

	if (demuxer == NULL) {
		ReportErrorOccurred ("Media::SeekAsync was called, but there is no demuxer to seek on.\n");
		return;
	}

	demuxer->SeekAsync (pts);
}

void
Media::ReportSeekCompleted (guint64 pts)
{
	LOG_PIPELINE ("Media::ReportSeekCompleted (%" G_GUINT64_FORMAT "), id: %i\n", pts, GET_OBJ_ID (this));
	
	buffering_progress = 0;
	ClearQueue ();
	EmitSafe (SeekCompletedEvent);
}

void
Media::ReportOpenDemuxerCompleted ()
{
	LOG_PIPELINE ("Media::ReportOpenDemuxerCompleted (), id: %i\n", GET_OBJ_ID (this));
	VERIFY_MEDIA_THREAD;
	OpenInternal ();
}

void
Media::ReportOpenDecoderCompleted (IMediaDecoder *decoder)
{
	LOG_PIPELINE ("Media::ReportOpenDecoderCompleted (%p), id: %i\n", decoder, GET_OBJ_ID (this));
	
	g_return_if_fail (decoder != NULL);
	
	OpenInternal ();
}

void
Media::ReportErrorOccurred (ErrorEventArgs *args)
{
	LOG_PIPELINE ("Media::ReportErrorOccurred (%p %s)\n", args, args == NULL ? NULL : args->GetErrorMessage());
	
	if (args) {
		fprintf (stderr, "Moonlight: %s %i %s %s\n", enums_int_to_str ("ErrorType", args->GetErrorType()), args->GetErrorCode(), args->GetErrorMessage(), args->GetExtendedMessage());
	} else {
		fprintf (stderr, "Moonlight: Unspecified media error.\n");
	}
	
	if (!error_reported) {
		error_reported = true;
		args->ref ();
		EmitSafe (MediaErrorEvent, args);
	}

	args->unref ();
}

void
Media::ReportErrorOccurred (const char *message)
{
	LOG_PIPELINE ("Media::ReportErrorOccurred (%s)\n", message);
	
	ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 3001, message)));
}

void
Media::PlayAsync ()
{
	LOG_PIPELINE ("Media::PlayAsync ()\n");
	
	MediaClosure *closure = new MediaClosure (this, PlayCallback, this, "Media::PlayAsync");
	EnqueueWork (closure);
	closure->unref ();
}

void
Media::PauseAsync ()
{
	LOG_PIPELINE ("Media::PauseAsync ()\n");
}

void
Media::StopAsync ()
{
	LOG_PIPELINE ("Media::StopAsync ()\n");
	
	MediaClosure *closure = new MediaClosure (this, StopCallback, this, "Media::StopAsync");
	EnqueueWork (closure);
	closure->unref ();

	// seek to the beginning
	SeekAsync (start_time);
}

MediaResult
Media::StopCallback (MediaClosure *closure)
{
	closure->GetMedia ()->Stop ();
	return MEDIA_SUCCESS;
}

MediaResult
Media::PlayCallback (MediaClosure *closure)
{
	closure->GetMedia ()->Play ();
	return MEDIA_SUCCESS;
}

void
Media::Stop ()
{
	LOG_PIPELINE ("Media::Stop () ID: %i\n", GET_OBJ_ID (this));
	
	g_return_if_fail (MediaThreadPool::IsThreadPoolThread ());
	
	stopped = true;
	
	/* This can't be done, if PlayAsync was called right after StopAsync, we might actually remove the request to start playing again */
	/* ClearQueue (); */
	
	if (demuxer != NULL)
		demuxer->ClearBuffers ();
}

void
Media::Play ()
{
	LOG_PIPELINE ("Media::Play () ID: %i\n", GET_OBJ_ID (this));
	
	g_return_if_fail (MediaThreadPool::IsThreadPoolThread ());
	
	stopped = false;
	if (demuxer != NULL)
		demuxer->FillBuffers ();
}

void
Media::Initialize (Downloader *downloader, const char *PartName)
{
	IMediaSource *source;
	
	LOG_PIPELINE ("Media::Initialize (%p, '%s'), id: %i\n", downloader, PartName, GET_OBJ_ID (this));
	
	g_return_if_fail (downloader != NULL);
	g_return_if_fail (file == NULL);
	g_return_if_fail (uri != NULL || PartName != NULL);
	g_return_if_fail (initialized == false);
	g_return_if_fail (error_reported == false);
	g_return_if_fail (this->source == NULL);
	
	if (downloader->Completed ()) {
		file = downloader->GetDownloadedFilename (PartName);
		
		if (file == NULL) {
			ReportErrorOccurred ("Couldn't get downloaded filename.");
			return;
		}
	}
	
	if (file == NULL && PartName != NULL && PartName [0] != 0) {
		ReportErrorOccurred ("We don't support using media in zip files which haven't been downloaded yet (i.e. calling MediaElement.SetSource (dl, 'foo') with a dl which hasn't downloaded the file yet)");
		return;
	}
	
	if (file == NULL) {
		ReportErrorOccurred ("We don't support using downloaders for mms streams.");
		return;
	}

	source = new FileSource (this, file);
	Initialize (source);
	source->unref ();
}

void
Media::Initialize (IMediaSource *source)
{
	MediaResult result;
	
	LOG_PIPELINE ("Media::Initialize (%p), id: %i\n", source, GET_OBJ_ID (this));
	
	g_return_if_fail (source != NULL);
	g_return_if_fail (this->source == NULL);
	g_return_if_fail (initialized == false);
	
	result = source->Initialize ();
	if (!MEDIA_SUCCEEDED (result)) {
		ReportErrorOccurred ("Unspecified error while initializing source");
		return;
	}
	
	initialized = true;
	this->source = source;
	this->source->ref ();
}

void
Media::Initialize (const Uri *resource_base, const Uri *uri)
{
	IMediaSource *source = NULL;
	
	LOG_PIPELINE ("Media::Initialize ('%s'), id: %i\n", uri ? uri->GetOriginalString () : NULL, GET_OBJ_ID (this));	
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (file == NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (initialized == false);
	g_return_if_fail (error_reported == false);
	g_return_if_fail (source == NULL);
	g_return_if_fail (this->source == NULL);
	
	this->uri = Uri::Clone (uri);
	this->resource_base = Uri::Clone (resource_base);
	
	if (this->uri->IsAbsolute () && (this->uri->IsScheme ("mms") || this->uri->IsScheme ("rtsp") || this->uri->IsScheme ("rtsps"))) {
		// We ignore resource base for mms sources, they have to live on a server
		source = new MmsSource (this, this->uri);
	} else {
		source = new ProgressiveSource (this, this->resource_base, this->uri);
	}

	Initialize (source);
	source->unref ();
}

void
Media::Initialize (IMediaDemuxer *demuxer)
{
	LOG_PIPELINE ("Media::Initialize (%p), id: %i\n", demuxer, GET_OBJ_ID (this));
	
	g_return_if_fail (demuxer != NULL);
	g_return_if_fail (this->demuxer == NULL);
	g_return_if_fail (initialized == false);
	
	this->demuxer = demuxer;
	this->demuxer->ref ();
	
	initialized = true;
}

void
Media::SetTargetPts (guint64 pts)
{
	mutex.Lock ();
	target_pts = pts;
	mutex.Unlock ();
}

guint64
Media::GetTargetPts ()
{
	guint64 result;
	mutex.Lock ();
	result = target_pts;
	mutex.Unlock ();
	return result;
}

void
Media::RetryHttp (ErrorEventArgs *args)
{
	Uri *resource_base = NULL;
	Uri *http_uri = NULL;
	
	LOG_PIPELINE ("Media::RetryHttp (), current uri: '%s'\n", uri->ToString ());
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (source != NULL);
	
	if (http_retried) {
		ReportErrorOccurred (args);
		return;
	}
	
	// CHECK: If the current protocolo is rtsps, should we retry http or https?
	
	if (uri->IsScheme ("mms") || uri->IsScheme ("rtsp") || uri->IsScheme ("rtsps")) {
		http_uri = Uri::CloneWithScheme (uri, "http");
	} else {
		ReportErrorOccurred (args);
		return;
	}
	
	http_retried = true;
	
	LOG_PIPELINE ("Media::RetryHttp (), new uri: '%s'\n", http_uri->ToString ());
	
	delete uri;
	uri = NULL;
	resource_base = this->resource_base;
	this->resource_base = NULL;

	/* this method is called on the main thread, ensure Dispose is called on the source on the media thread  */
	DisposeObject (source);
	source->unref ();
	source = NULL;
	initialized = false;
	error_reported = false;
	
	Initialize (resource_base, http_uri);
	delete resource_base; // Initialize has cloned the resource base now, we can delete it
	
	delete http_uri;
	
	if (!error_reported)
		OpenAsync ();
}

void
Media::OpenAsync ()
{
	LOG_PIPELINE ("Media::OpenAsync (), id: %i\n", GET_OBJ_ID (this));
	
	g_return_if_fail (initialized == true);
	
	EmitSafe (OpeningEvent);
	
	MediaClosure *closure = new MediaClosure (this, OpenInternal, this, "Media::OpenAsync");
	EnqueueWork (closure);
	closure->unref ();
}

void
Media::OpenInternal ()
{
	LOG_PIPELINE ("Media::OpenInternal (), id: %i\n", GET_OBJ_ID (this));

	g_return_if_fail (initialized == true);

	if (opened) {
		// This may happen due to the recursion detection below
		// Example: we try open a demuxer, the demuxer opens successfully 
		// right away and calls ReportDemuxerOpenComplete which will call
		// us. Due to the recursion detection we'll enqueue a call to
		// OpenInternal, while the first OpenInternal may succeed and
		// set opened to true.
		LOG_PIPELINE ("Media::OpenInteral (): already opened.\n");
		return;
	}
	
	// detect recursive calls.

	if (in_open_internal) {
		LOG_PIPELINE ("Media::OpenInteral (): recursive.\n");
		MediaClosure *closure = new MediaClosure (this, OpenInternal, this, "Media::OpenInternal");
		EnqueueWork (closure);
		closure->unref ();
		return;
	}

	in_open_internal = true; 

	if (error_reported)
		goto cleanup;
		
	if (!SelectDemuxerAsync ()) {
		LOG_PIPELINE ("Media::OpenInteral (): no demuxer yet.\n");
		goto cleanup;
	}
		
	if (error_reported)
		goto cleanup;
		
	if (!SelectDecodersAsync ()) {
		LOG_PIPELINE ("Media::OpenInteral (): no decoders yet.\n");
		goto cleanup;
	}
		
	opened = true;
	opening = false;
	
	LOG_PIPELINE ("Media::OpenInteral (): opened successfully.\n");
	
	EmitSafe (OpenCompletedEvent);

	SeekAsync (start_time);

cleanup:
	in_open_internal = false;
}

MediaResult
Media::OpenInternal (MediaClosure *closure)
{
	Media *media = (Media *) closure->GetContext ();
	
	g_return_val_if_fail (media != NULL, MEDIA_FAIL);
	
	media->OpenInternal ();
	
	return MEDIA_SUCCESS;
}

MediaResult
Media::SelectDemuxerReadCallback (MediaClosure *c)
{
	LOG_PIPELINE ("Media::SelectDemuxerReadCallback (%p)\n", c);
	MediaReadClosure *closure = (MediaReadClosure *) c;
	closure->GetMedia ()->SelectDemuxerAsync (closure);
	return MEDIA_SUCCESS;
}

bool
Media::SelectDemuxerAsync ()
{
	return SelectDemuxerAsync (NULL);
}

bool
Media::SelectDemuxerAsync (MediaReadClosure *closure)
{
	DemuxerInfo *demuxerInfo;
	MediaResult support;
	MediaResult result;
	
	LOG_PIPELINE ("Media::SelectDemuxer () id: %i, demuxer: %p, IsOpened: %i, IsOpening: %i\n", GET_OBJ_ID (this), demuxer, demuxer ? demuxer->IsOpened () : -1, demuxer ? demuxer->IsOpening () : -1);
	
	g_return_val_if_fail (error_reported == false, false);
	g_return_val_if_fail (initialized == true, false);
	
	// Check if demuxer already is open
	if (demuxer != NULL) {
		if (demuxer->IsOpened ())
			return true;
		if (!demuxer->IsOpening ())
			demuxer->OpenDemuxerAsync ();
		return demuxer->IsOpened ();
	}
	
	g_return_val_if_fail (source != NULL, false);
	
	// Check if the source knows how to create the demuxer
	demuxer = source->CreateDemuxer (this, NULL);

	if (demuxer == NULL) { // No demuxer created, we need to find it ourselves.
		if (closure == NULL) {
			/* No data yet, request a read */
			MediaReadClosure *read_closure = new MediaReadClosure (this, SelectDemuxerReadCallback, this, 0, 1024);
			source->ReadAsync (read_closure);
			read_closure->unref ();
			return false;
		}

		// Select a demuxer
		bool any_not_enough_data = false;
		MemoryBuffer *data = closure->GetData ();
		demuxerInfo = registered_demuxers;
		while (demuxer == NULL && demuxerInfo != NULL) {
			LOG_PIPELINE ("Media::SelectDemuxer ): Checking if '%s' can handle the media.\n", demuxerInfo->GetName ());
			data->SeekSet (0);
			LOG_DEMUXERS ("Media::SelectDemuxer ): Checking if '%s' can handle the media.\n", demuxerInfo->GetName ());
			support = demuxerInfo->Supports (data);
			
			if (support == MEDIA_SUCCESS)
				break;
			
			result = support;
	
			if (result == MEDIA_NOT_ENOUGH_DATA) {
				LOG_DEMUXERS ("Media::SelectDemuxer (): '%s' can't determine whether it can handle the media or not due to not enough data being available yet.\n", demuxerInfo->GetName ());
				
				if (closure->GetCount () > 1024 * 1024 * 10) {
					/* 10 MB should be enough to determine if a demuxer can handle the source */
				} else if (closure->GetCount () != data->GetSize ()) {
					/* We reached the end of the available data */
				} else {
					any_not_enough_data = true;
				}
			} else {
				LOG_DEMUXERS ("Media::SelectDemuxer (): '%s' can't handle this media.\n", demuxerInfo->GetName ());
			}
			
			demuxerInfo = (DemuxerInfo *) demuxerInfo->next;
		}
		
		if (demuxerInfo == NULL && any_not_enough_data) {
			/* Read another 1024 bytes and try again */
			MediaReadClosure *read_closure = new MediaReadClosure (this, SelectDemuxerReadCallback, this, 0, closure->GetCount () + 1024);
			source->ReadAsync (read_closure);
			read_closure->unref ();

			return false;
		}
		
		if (demuxerInfo == NULL) {
			// No demuxer found, report an error
			char *source_name = file ? g_strdup (file) : (uri ? g_strdup (uri->ToString ()) : NULL);
		
			if (!source_name) {
				switch (source->GetObjectType ()) {
				case Type::PROGRESSIVESOURCE: {
					ProgressiveSource *ps = (ProgressiveSource *) source;
					source_name = g_strdup_printf ("%s (%s)", ps->GetUri ()->ToString (), ps->GetFileName ());
					break;
				}
				case Type::FILESOURCE:
					source_name = g_strdup (((FileSource *) source)->GetFileName ());
					break;
				case Type::MMSSOURCE:
				case Type::MMSPLAYLISTENTRY:
					source_name = g_strdup ("live source");
					break;
				default:
					source_name = g_strdup_printf ("unknown source: %s", source->GetTypeName ());
					break;
				}
			}
			char *msg = g_strdup_printf ("No demuxers registered to handle the media source '%s'.", source_name);
			ReportErrorOccurred (new ErrorEventArgs (MediaError,
											 MoonError (MoonError::EXCEPTION, 3001, "AG_E_INVALID_FILE_FORMAT"),
											 MEDIA_UNKNOWN_CODEC, msg));
			g_free (msg);
			g_free (source_name);
			return false;
		}

		// Found a demuxer
		data->SeekSet (0);

		demuxer = demuxerInfo->Create (this, source, data);
	} else {
		LOG_DEMUXERS ("Media::SelectDemuxer (): The source created the demuxer (%s).\n", demuxer->GetTypeName ());
	}
	
	if (demuxer->IsOpened ())
		return true;
	
	if (demuxer->IsOpening ())	
		return false;
	
	LOG_DEMUXERS ("Media::SelectDemuxer (), id: %i opening demuxer %i (%s)\n", GET_OBJ_ID (this), GET_OBJ_ID (demuxer), demuxer->GetTypeName ());
	
	demuxer->OpenDemuxerAsync ();
	
	LOG_DEMUXERS ("Media::SelectDemuxer (), id: %i opening demuxer %i (%s) [Done]\n", GET_OBJ_ID (this), GET_OBJ_ID (demuxer), demuxer->GetTypeName ());
	
	return demuxer != NULL && demuxer->IsOpened ();
}

bool
Media::SelectDecodersAsync ()
{	
	LOG_PIPELINE ("Media::SelectDecodersAsync () id: %i.\n", GET_OBJ_ID (this));
		
	g_return_val_if_fail (error_reported == false, false);
	g_return_val_if_fail (initialized == true, false);
	
	if (demuxer == NULL) {
		ReportErrorOccurred ("No demuxer to select decoders from.");
		return false;
	}
	
	// If the demuxer has no streams (ASXDemuxer for instance)
	// then just return success.
	if (demuxer->GetStreamCount () == 0)
		return true;

	LOG_PIPELINE ("Media::SelectDecodersAsync (): Selecting decoders.\n");
	
	// Select codecs for each stream
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);

		if (stream == NULL) {
			ReportErrorOccurred ("MEDIA_INVALID_STREAM");
			return false;
		}
		
		if (stream->GetDecoder () != NULL)
			continue;
		
		const char *codec = stream->GetCodec ();
		IMediaDecoder *decoder = NULL;
		
		LOG_CODECS ("Moonlight: Searching registered decoders for a decoder which supports '%s'\n", codec);
		
		DecoderInfo *current_decoder = registered_decoders;
		while (current_decoder != NULL && !current_decoder->Supports (codec)) {
			LOG_CODECS ("Moonlight: Checking if registered decoder '%s' supports codec '%s': no.\n", current_decoder->GetName (), codec);
			current_decoder = (DecoderInfo*) current_decoder->next;
		}

		if (current_decoder == NULL) {
			printf ("Moonlight: Unknown codec: %s\n", codec);
			continue;
		}
		
		LOG_CODECS ("Moonlight: Checking if registered decoder '%s' supports codec '%s': yes.\n", current_decoder->GetName (), codec);
		decoder = current_decoder->Create (this, stream);
		
		stream->SetDecoder (decoder);
		decoder->unref ();
	}
	
	if (error_reported)
		return false;
		
	// Open the codecs
	LOG_PIPELINE ("Media::SelectDecodersAsync (): Opening decoders.\n");
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		IMediaDecoder *decoder;
		
		if (stream == NULL)
			continue;
		
		decoder = stream->GetDecoder ();
		
		if (decoder == NULL) {
			ReportErrorOccurred (new ErrorEventArgs (MediaError,
											 MoonError (MoonError::EXCEPTION, 3001, "AG_E_INVALID_FILE_FORMAT")));
			return false;
		}
		
		if (decoder->IsOpening () || decoder->IsOpened ())
			continue;
			
		decoder->OpenDecoderAsync ();
	}
	
	if (error_reported)
		return false;
	
	// Wait until all the codecs have opened
	LOG_PIPELINE ("Media::SelectDecodersAsync (): Waiting for decoders to open.\n");
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		IMediaDecoder *decoder;
		
		if (stream == NULL)
			continue;
		
		decoder = stream->GetDecoder ();
		
		if (decoder == NULL) {
			ReportErrorOccurred ("Stream does not have a decoder after selecting one");
			return false;
		}
		
		if (decoder->IsOpening ()) {
			/* Wait until the decoder has been opened */
			return false;
		}
		
		if (!decoder->IsOpened ()) {
			// After calling OpenDecoderAsync on a decoder, the decoder should either be opened, opening, or an error should have occurred.
			ReportErrorOccurred ("Stream does not have a decoder after selecting one");
			return false;
		}

	}

	// All the codecs have been opened now.
	// Find converters for each of them (whenever required).
	
	LOG_PIPELINE ("Media::SelectDecodersAsync (): Selecting converters.\n");
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		IMediaDecoder *decoder;
		
		if (stream == NULL)
			continue;
		
		decoder = stream->GetDecoder ();
		
		if (decoder == NULL) {
			ReportErrorOccurred ("Stream does not have a decoder after selecting one");
			return false;
		}

		if (!stream->IsVideo ())
			continue; // Only video streams need converters
			
		if (decoder->GetPixelFormat () == MoonPixelFormatRGB32 || decoder->GetPixelFormat () == MoonPixelFormatRGBA32)
			continue; // We need RGB32, so any stream already producing RGB32 doesn't need a converter.
			
		// Select converter for this stream
		VideoStream *vs = (VideoStream *) stream;
		
		ConverterInfo* current_conv = registered_converters;
		while (current_conv != NULL && !current_conv->Supports (decoder->GetPixelFormat (), MoonPixelFormatRGB32)) {
			LOG_PIPELINE ("Checking whether '%s' supports input '%s' (%d) and output '%s' (%d): no.\n",
				current_conv->GetName (), enums_int_to_str ("MoonPixelFormat", decoder->GetPixelFormat ()), decoder->GetPixelFormat (), enums_int_to_str ("MoonPixelFormat", MoonPixelFormatRGB32), MoonPixelFormatRGB32);
			current_conv = (ConverterInfo*) current_conv->next;
		}
		
		if (current_conv == NULL) {
			char *msg = g_strdup_printf ("Can't convert from %d to %d: No converter found", decoder->GetPixelFormat (), MoonPixelFormatRGB32);
			ReportErrorOccurred (msg);
			g_free (msg);
			return false;
		}	
		
		LOG_PIPELINE ("Checking whether '%s' supports input '%s' (%d) and output '%s' (%d): yes.\n",
			current_conv->GetName (), enums_int_to_str ("MoonPixelFormat", decoder->GetPixelFormat ()), decoder->GetPixelFormat (), enums_int_to_str ("MoonPixelFormat", MoonPixelFormatRGB32), MoonPixelFormatRGB32);
		
		vs->SetImageConverter (current_conv->Create (this, vs));
		vs->GetImageConverter ()->unref (); /* the stream has a ref now */
		vs->GetImageConverter ()->input_format = decoder->GetPixelFormat ();
		vs->GetImageConverter ()->output_format = MoonPixelFormatRGB32;
		if (!vs->GetImageConverter ()->Open ()) {
			vs->SetImageConverter (NULL);
			return false;
		}
	}
	
	// Loop through all the streams, return true if at least one has a codec.
	
	for (int i = 0; i < demuxer->GetStreamCount (); i++) {
		IMediaStream *stream = demuxer->GetStream (i);
		
		if (stream == NULL)
			continue;
		
		if (stream->GetDecoder () != NULL)
			return true;
	}
	
	// No codecs found for no stream, report an error.
	ReportErrorOccurred ("Didn't find any codecs for any stream.");
	return false;
}

bool
Media::EnqueueWork (MediaClosure *closure)
{
	bool result = false;
	bool disposed;
	
	LOG_PIPELINE_EX ("Media::EnqueueWork (%p).\n", closure);

	g_return_val_if_fail (closure != NULL, false);

	if (IsDisposed ())
		return false;
	
	mutex.Lock ();
	disposed = this->is_disposed;
	if (disposed) {
		result = false;
		LOG_PIPELINE ("Media::EnqueueWork (): disposed: %i, work not added\n", disposed);
	} else {
		MediaThreadPool::AddWork (closure);
		result = true;
	}
	mutex.Unlock ();
		
	return result;
}

MediaResult
Media::DisposeObjectInternal (MediaClosure *closure)
{
	closure->GetContext ()->Dispose ();
	return MEDIA_SUCCESS;
}

void
Media::DisposeObject (EventObject *obj)
{
	MediaDisposeObjectClosure *closure = new MediaDisposeObjectClosure (this, DisposeObjectInternal, obj);
	if (!EnqueueWork (closure)) {
		LOG_PIPELINE ("Media::DisposeObject (%p): Could not add callback to the media thread, calling Dispose directly.\n", obj);
		obj->Dispose ();
	}
	closure->unref ();
}

void
Media::ClearQueue ()
{
	LOG_PIPELINE ("Media::ClearQueue ().\n");
	MediaThreadPool::RemoveWork (this);
}

/*
 * ASXDemuxer
 */

ASXDemuxer::ASXDemuxer (Media *media, IMediaSource *source, MemoryBuffer *buffer)
	: IMediaDemuxer (Type::ASXDEMUXER, media, source)
{
	playlist = NULL;
	this->buffer = buffer;
	this->buffer->ref ();
}

ASXDemuxer::~ASXDemuxer ()
{
}

void
ASXDemuxer::Dispose ()
{
	if (playlist) {
		playlist->unref ();
		playlist = NULL;
	}
	if (buffer) {
		buffer->unref ();
		buffer = NULL;
	}
	IMediaDemuxer::Dispose ();
}

MediaResult
ASXDemuxer::DataCallback (MediaClosure *c)
{
	MediaReadClosure *closure = (MediaReadClosure *) c;
	ASXDemuxer *demuxer = (ASXDemuxer *) closure->GetContext ();
	if (demuxer->buffer)
		demuxer->buffer->unref ();
	demuxer->buffer = closure->GetData ();
	demuxer->buffer->ref ();
	demuxer->OpenDemuxerAsyncInternal ();
	return MEDIA_SUCCESS;
}

void
ASXDemuxer::OpenDemuxerAsyncInternal ()
{
	MediaResult result;
	PlaylistRoot *root;
	ErrorEventArgs *args = NULL;
	Media *media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);

	if (source->GetSize () != buffer->GetSize ()) {
		/* If the source doesn't know its size, it means it hasn't been fully downloaded, and we also get into this branch */
		MediaReadClosure *closure = new MediaReadClosure (media, DataCallback, this, 0, source->GetSize () > 0 ? source->GetSize () : buffer->GetSize () * 2);
		source->ReadAsync (closure);
		closure->unref ();
		media->unref ();
		return;
	}

	root = media->GetPlaylistRoot ();
	
	g_return_if_fail (root != NULL);

	PlaylistParser *parser = new PlaylistParser (root, buffer);

	if (MEDIA_SUCCEEDED (parser->Parse ())) {
		result = MEDIA_SUCCESS;
		playlist = parser->GetPlaylist ();
		playlist->ref ();
	} else {
		result = MEDIA_FAIL;
		args = parser->GetErrorEventArgs ();
		if (args != NULL)
			args->ref ();
	}

	delete parser;

	if (MEDIA_SUCCEEDED (result)) {
		ReportOpenDemuxerCompleted ();
	} else if (result == MEDIA_NOT_ENOUGH_DATA) {
		EnqueueOpen ();
	} else if (args != NULL) {
		args->ref (); // calling ReportErrorOccurred with an event args will end up unreffing it
		ReportErrorOccurred (args);
	} else {
		ReportErrorOccurred ("Unspecified error while opening ASXDemuxer");
	}
	if (args)
		args->unref ();
	
	media->unref ();
}

/*
 * ASXDemuxerInfo
 */

MediaResult
ASXDemuxerInfo::Supports (MemoryBuffer *source)
{
	if (PlaylistParser::IsASX2 (source)) {
		LOG_PLAYLIST ("ASXDemuxerInfo::Supports (): ASX2.\n");
		return MEDIA_SUCCESS;
	}

	source->SeekSet (0);

	if (PlaylistParser::IsASX3 (source)) {
		LOG_PLAYLIST ("ASXDemuxerInfo::Supports (): ASX3.\n");
		return MEDIA_SUCCESS;
	}

	return MEDIA_FAIL;
}

IMediaDemuxer *
ASXDemuxerInfo::Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
{
	return new ASXDemuxer (media, source, initial_buffer);
}

/*
 * ManagedStreamSource
 */

ManagedStreamSource::ManagedStreamSource (Media *media, ManagedStreamCallbacks *stream) : IMediaSource (Type::MANAGEDSTREAMSOURCE, media)
{
	memcpy (&this->stream, stream, sizeof (this->stream));
}

ManagedStreamSource::~ManagedStreamSource ()
{
	stream.handle = NULL;
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

void
ManagedStreamSource::ReadAsyncInternal (MediaReadClosure *closure)
{
	Media *media;

	if (closure->GetOffset () >= G_MAXINT32 || closure->GetCount () >= G_MAXINT32) {
		fprintf (stderr, "Moonlight: stream read overflow, offset: %" G_GUINT64_FORMAT ", count: %" G_GUINT32_FORMAT "\n", closure->GetOffset (), closure->GetCount ());
		return;
	}

	media = GetMediaReffed ();
	if (media == NULL) {
		LOG_PIPELINE ("ManagedStreamSource::ReadAsyncInternal (): no media, disposed?\n");
		return;
	}

	if (stream.CanSeek (stream.handle) && GetPositionInternal () != closure->GetOffset ()) {
		stream.Seek (stream.handle, closure->GetOffset (), SEEK_SET);
	}

	void *buf = g_malloc (closure->GetCount ());
	gint32 read = stream.Read (stream.handle, buf, 0, closure->GetCount ());
	MemoryBuffer *src = new MemoryBuffer (media, buf, read, true);
	closure->SetData (src);
	media->EnqueueWork (closure);
	media->unref ();
	src->unref ();
}
	
/*
 * FileSource
 */

FileSource::FileSource (Media *media, const char *filename) : IMediaSource (Type::FILESOURCE, media)
{
	this->filename = g_strdup (filename);
	fd = NULL;
	size = 0;
}

FileSource::~FileSource ()
{
	g_free (filename);
	filename = NULL;
}

void
FileSource::Dispose ()
{
	if (fd != NULL) {
		fclose (fd);
		fd = NULL;
	}
	IMediaSource::Dispose ();
}

MediaResult 
FileSource::Initialize ()
{
	struct stat st;
	
	LOG_PIPELINE ("FileSource::Initialize () filename: %s\n", filename);

	g_return_val_if_fail (fd == NULL, MEDIA_FAIL);
	g_return_val_if_fail (filename != NULL, MEDIA_FAIL);

	fd = g_fopen (filename, "r");

	if (fd == NULL) {
		char *msg = g_strdup_printf ("Could not open the file: %s\n", filename);
		ReportErrorOccurred (msg);
		g_free (msg);
		return MEDIA_FAIL;
	}

	if (fstat (fileno (fd), &st) != -1) {
		size = st.st_size;
	} else {
		size = 0;
	}
		
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

	LOG_PIPELINE_EX ("FileSource::GetPositionInternal (): result: %" G_GINT64_FORMAT "\n", result);

	return result;
}

void
FileSource::ReadAsyncInternal (MediaReadClosure *closure)
{
	ReadFD (fd, closure);
}

bool
FileSource::Eof ()
{
	if (fd == NULL)
		return false;
	
	return feof (fd);
}

/*
 * MediaReadClosureNore
 */
class MediaReadClosureNode : public List::Node {
private:
	MediaReadClosure *closure;

public:
	MediaReadClosureNode (MediaReadClosure *closure)
	{
		this->closure = closure;
		this->closure->ref ();
	}
	virtual ~MediaReadClosureNode ()
	{
		this->closure->unref ();
		this->closure = NULL;
	}
	MediaReadClosure *GetClosure () { return closure; }
};

/*
 * Ranges
 */

Ranges::Ranges ()
{
	ranges = NULL;
	count = 0;
}

Ranges::~Ranges ()
{
	g_free (ranges);
}

void
Ranges::Add (guint64 offset, guint32 length)
{
	bool extended = false;

	LOG_PIPELINE_EX ("Ranges::Add (%" G_GUINT64_FORMAT ", %" G_GUINT32_FORMAT ")\n", offset, length);

	/* Check if we can extend an existing range */
	for (int i = 0; i < count; i++) {
		guint64 r_offset = ranges [i].offset;
		guint64 r_length = ranges [i].length;
		if (r_offset <= offset && r_offset + r_length >= offset) {
			/* We can extend an existing range */
			ranges [i].length = MAX ((offset + length) - r_offset, r_length);
			extended = true;
			break;
		} else if (r_offset > offset && offset + length >= r_offset) {
			/* We can extend 'backwards' an existing range */
			ranges [i].offset = offset;
			ranges [i].length = MAX ((r_offset + r_length) - offset, length);
			extended = true;
			break;
		} else if (r_offset > offset) {
			/* No need to look further since the ranges array is ordered */
			break;
		}
	}

	if (!extended) {
		/* We could not extend an existing range, append/insert a new one */
		count++;
		ranges = (Range *) g_realloc (ranges, count * sizeof (Range));
		ranges [count - 1].offset = 0;
		ranges [count - 1].length = 0;

		/* We need to make sure we insert the new range at the right index */
		gint32 insert_index = count - 1;
		for (int i = 0; i < count - 1; i++) {
			guint64 r_offset = ranges [i].offset;
			if (r_offset > offset) {
				/* We need to insert, not append. Move the data one step ahead */
				memmove (ranges + i + 1, ranges + i, (count - 1 - i) * sizeof (Range));
				insert_index = i;
				break;
			}
		}
		ranges [insert_index].offset = offset;
		ranges [insert_index].length = length;
	}

	/* Now we need to check for overlapping/redundant ranges */
	bool found_redundant;
	do {
		found_redundant = false;
		for (int i = 0; i < count - 1; i++) {
			guint64 r_offset = ranges [i].offset;
			guint64 r_length = ranges [i].length;
			guint64 n_offset = ranges [i + 1].offset;
			guint64 n_length = ranges [i + 1].length;
			if (r_offset + r_length >= n_offset) {
				/* the current range overlaps the next range, join them */
				ranges [i].length = (n_offset + n_length) - r_offset;
				if (count < i + 2) {
					/* move back the next ranges */
					memmove (ranges + i + 1, ranges + i + 2, (count - (i + 2)) * sizeof (Range));
				} else {
					/* last entry joined into the second last entry, no need to move memory back */
				}
				count--;
				found_redundant = true;
				break;
			}
		}
	} while (found_redundant);

	LOG_PIPELINE_EX ("Final ranges: ");
	for (int i = 0; i < count; i++)
		LOG_PIPELINE_EX ("[%" G_GUINT64_FORMAT ",%" G_GUINT64_FORMAT "] ", ranges [i].offset, ranges [i].length);
	LOG_PIPELINE_EX ("\n");
}

bool
Ranges::Contains (guint64 offset, guint64 length)
{
	for (int i = 0; i < count; i++) {
		if (ranges [i].Contains (offset, length)) {
			LOG_PIPELINE_EX ("Ranges::Contains (%" G_GUINT64_FORMAT ",%" G_GUINT64_FORMAT "): YES\n", offset, length);
			return true;
		}
	}
	LOG_PIPELINE_EX ("Ranges::Contains (%" G_GUINT64_FORMAT ",%" G_GUINT64_FORMAT "): NO\n", offset, length);
	return false;
}

guint64
Ranges::GetFirstNotInRange (guint64 offset, guint64 length)
{
	for (int i = 0; i < count; i++) {
		/* We know that the list of ranges is sorted and it does not have overlapping ranges */
		if (!ranges [i].Contains (offset, length)) {
			if (ranges [i].offset > offset) {
				/* We want a position before the first we have */
				return offset;
			} else {
				/* We want a length bigger than what we have */
				return ranges [i].offset + ranges [i].length;
			}
		}
	}

	return G_MAXUINT64;
}

/*
 * ProgressiveSource
 */

ProgressiveSource::ProgressiveSource (Media *media, const Uri *resource_base, const Uri *uri)
	: IMediaSource (Type::PROGRESSIVESOURCE, media)
{
	complete = false;
	error_occurred = false;
	write_pos = 0;
	size = -1;
	write_fd = NULL;
	read_fd = NULL;
	cancellable = NULL;
	filename = NULL;
	bytes_received = 0;
	first_reception = 0;
	current_request = 0;
	current_request_received = 0;
	range_request = NULL;
	brr_enabled = 0;

	this->uri = Uri::Clone (uri);
	this->resource_base = Uri::Clone (resource_base);
}

ProgressiveSource::~ProgressiveSource ()
{
	if (write_fd) {
		fclose (write_fd);
		write_fd = NULL;
	}
	if (read_fd) {
		fclose (read_fd);
		read_fd = NULL;
	}
	delete uri;
	uri = NULL;
	delete resource_base;
	resource_base = NULL;
	g_free (filename);
	filename = NULL;
}

void
ProgressiveSource::Dispose ()
{
	bool delete_requests;

	mutex.Lock ();
	delete_requests = cancellable != NULL || range_request != NULL;
	mutex.Unlock ();
	
	if (delete_requests) {
		if (Surface::InMainThread ()) {
			DeleteCancellable (this);
		} else {
			// we have to cancel/delete he cancellable on the main thread
			// it may end up doing a lot of stuff, including calling into
			// mozilla.
			Resurrect (); /* Don't call ref, since that will cause a warning to be printed */
			AddTickCall (DeleteCancellable);
			unref (); /* The tick call still has a ref */
		}
	}
	
	IMediaSource::Dispose ();
}
gint64
ProgressiveSource::GetPositionInternal ()
{
	printf ("ProgressiveSource::GetPositionInternal (): this should method should not have been called.\n");
	return 0;
}
void
ProgressiveSource::DeleteCancellable (EventObject *data)
{
	ProgressiveSource *src = (ProgressiveSource *) data;
	Cancellable *cancellable = NULL;

	src->mutex.Lock ();
	if (src->cancellable) {
		cancellable = src->cancellable;
		src->cancellable = NULL;
	}
	src->mutex.Unlock ();

	/* Do work with the mutex unlocked */
	if (cancellable != NULL) {
		cancellable->Cancel ();
		delete cancellable;
	}
	if (src->range_request != NULL)
		src->SetRangeRequest (NULL);
}

bool
ProgressiveSource::Eof ()
{
	if (!complete)
		return false;

	if (read_fd == NULL)
		return false;

	return feof (read_fd);
}

MediaResult
ProgressiveSource::Initialize ()
{
	MediaResult result = MEDIA_SUCCESS;
	Application *application;
	int tmp_fd;

	if (uri == NULL) {
		/* Error message has already been shown from the ctor */
		return MEDIA_FAIL;
	}

	application = GetDeployment ()->GetCurrentApplication ();
	
	g_return_val_if_fail (application != NULL, MEDIA_FAIL);
	g_return_val_if_fail (filename == NULL, MEDIA_FAIL);
	g_return_val_if_fail (cancellable == NULL, MEDIA_FAIL);
	g_return_val_if_fail (write_fd == NULL, MEDIA_FAIL);
	g_return_val_if_fail (read_fd == NULL, MEDIA_FAIL);

	filename = g_build_filename (g_get_tmp_dir (), "MoonlightProgressiveStream.XXXXXX", NULL);
	
	if ((tmp_fd = g_mkstemp (filename)) == -1) {
		ReportErrorOccurred ("Could not create temporary filename for media file.\n");
		g_free (filename);
		filename = NULL;
		return MEDIA_FAIL;
	}
	
	LOG_PIPELINE ("ProgressiveSource::Initialize (): Created temporary file %s for %s\n", filename, uri->ToString ());
	
	/* Open the read file descriptor */
	read_fd = fdopen (tmp_fd, "r");
	if (read_fd == NULL) {
		char *msg = g_strdup_printf ("Could not open temporary file: %s", strerror (errno));
		ReportErrorOccurred (msg);
		g_free (msg);
		result = MEDIA_FAIL;
		goto delete_file;
	}

	/* Open the write file descriptor */
	write_fd = g_fopen (filename, "w");
	if (write_fd == NULL) {
		char *msg = g_strdup_printf ("Could not open a write handle to the file '%s'\n", filename);
		ReportErrorOccurred (msg);
		g_free (msg);
		result = MEDIA_FAIL;
		goto delete_file;
	}

	/* Disable buffering for the write file descriptor, this ensures that our table of file ranges is correctly readable
	 * using the read file descriptor. */
	setvbuf (write_fd, NULL, _IONBF, 0);

	/* Disable buffering for the read file descriptor too, with http seeking enabled we might have empty chunks in
	 * the file now that could get buffered */
	setvbuf (read_fd, NULL, _IONBF, 0);

	/* Unlink the file right away so that it'll be deleted even if we crash. */
	if (moonlight_flags & RUNTIME_INIT_KEEP_MEDIA) {
		printf ("Moonlight: The media file %s will not deleted (uri: %s).\n", filename, uri->ToString ());
	} else {
		g_unlink (filename);
	}
	
	cancellable = new Cancellable ();
	if (!application->GetResource (resource_base, uri, NotifyCallback, DataWriteCallback, MediaPolicy,
		 (HttpRequest::Options) (HttpRequest::DisableFileStorage), cancellable, (gpointer) this)) {
		result = MEDIA_FAIL;
		char *msg = g_strdup_printf ("invalid path found in uri '%s'", uri->ToString ());
		ReportErrorOccurred (msg);
		g_free (msg);
	}

	if (error_occurred)
		result = MEDIA_FAIL;
	
	return result;

delete_file:
	if (read_fd != NULL) {
		fclose (read_fd);
		read_fd = NULL;
	}
	if (write_fd != NULL) {
		fclose (write_fd);
		write_fd = NULL;
	}
	if (filename != NULL) {
		g_unlink (filename);
		g_free (filename);
		filename = NULL;
	}
	
	return result;
}

void
ProgressiveSource::NotifyCallback (NotifyType type, gint64 args, void *closure)
{
	((ProgressiveSource *) closure)->Notify (type, args);
}

void
ProgressiveSource::Notify (NotifyType type, gint64 args)
{
	LOG_PIPELINE ("ProgressiveSource::Notify (%i = %s, %" G_GINT64_FORMAT ") current range request: %" G_GINT64_FORMAT "\n",
		type, 
			(type == NotifyCompleted ? "NotifyCompleted" :
			(type == NotifyFailed ? "NotifyFailed" : 
			(type == NotifyStarted ? "NotifyStarted" : 
			(type == NotifyProgressChanged ? "NotifyProgressChanged" : "unknown")))),
		args, current_request);
		
	if (current_request != 0) {
		/* We have range requests going on and we've cancelled the main http request: do nothing */
		return;
	}

	switch (type) {
		case NotifyCompleted:
			DownloadComplete ();
			break;
		case NotifyFailed:
			DownloadFailed ();
			break;
		case NotifyStarted:
		case NotifyProgressChanged:
		default:
			break;
	}
}

double
ProgressiveSource::GetDownloadProgressOffset ()
{
	double result = 0;
	if (current_request != 0 && size != 0)
		result = (double) current_request / (double) size;
	LOG_PIPELINE ("ProgressiveSource::GetDownloadProgressOffset (): %f current_request: %" G_GUINT64_FORMAT " size: %" G_GUINT64_FORMAT "\n", result, current_request, size);
	return result;
}

gint32
ProgressiveSource::CalculateDownloadSpeed ()
{
	TimeSpan now = get_now ();
	double time = MAX (1.0, TimeSpan_ToSecondsFloat (now - first_reception));
	LOG_PIPELINE ("ProgressiveSource::CalculateDownloadSpeed () now: %" G_GUINT64_FORMAT " first: %" G_GUINT64_FORMAT " diff: %.2fs bytes received: %" G_GUINT64_FORMAT " result: %i\n",
		now, first_reception, time, bytes_received, (gint32) (bytes_received / time));
	return (gint32) (bytes_received / time);
}

void
ProgressiveSource::DataWriteCallback (EventObject *sender, EventArgs *calldata, void *closure)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) calldata;
	((ProgressiveSource *) closure)->DataWrite (args->GetData (), args->GetOffset (), args->GetCount ());
}

void
ProgressiveSource::DataWrite (void *buf, gint32 offset, gint32 n)
{
	size_t nwritten;
	Media *media = NULL;

	LOG_PIPELINE ("ProgressiveSource::DataWrite (%p, %i, %i) media: %p, filename: %s\n", buf, offset, n, media, filename);

	if (IsDisposed ())
		return;
	
	g_return_if_fail (write_fd != NULL);
	
	if (first_reception == 0) {
		first_reception = get_now ();
		/* Ensure brr_enabled is set */
		AddTickCall (CheckReadRequestsCallback);
	}
	bytes_received += n;

	media = GetMediaReffed ();
	
	if (n == 0) {
		// We've got the entire file, update the size
		size = MAX (size, write_pos); // Since this method is the only method that writes to write_pos, and we're not reentrant, there is no need to lock here.

		/* Don't close the write handle, we might get seeks to parts of the file that hasn't been downloaded */
		goto cleanup;
	} else {
		if (cancellable != NULL && cancellable->GetRequest () != NULL)
			size = cancellable->GetRequest ()->GetNotifiedSize ();
	}

	if (offset != -1 && fseek (write_fd, offset, SEEK_SET) != 0) {
		char *msg = g_strdup_printf ("Could not seek to offset %i: %s", offset, strerror (errno));
		ReportErrorOccurred (msg);
		g_free (msg);
	} else {
		nwritten = fwrite (buf, 1, n, write_fd);
		fflush (write_fd);

		if (nwritten > 0) {
			mutex.Lock ();
			if (offset != -1) {
				write_pos = offset + nwritten;
				ranges.Add (offset, nwritten);
			} else {
				ranges.Add (write_pos, nwritten);
				write_pos += nwritten;
			}
			mutex.Unlock ();

			CheckPendingReads ();
		}
	}


cleanup:
	if (media) {
		if (size != -1 && offset != -1) {
			if (current_request <= offset) {
				double progress = (double) (offset + n) / (double) size;
				double progress_offset = GetDownloadProgressOffset ();
				LOG_PIPELINE ("ProgressiveSource::DataWrite () current_request: %" G_GUINT64_FORMAT " = %.4f offset: %i = %.2f n: %i size: %" G_GUINT64_FORMAT " about to report progress: %f\n",
					current_request, progress_offset, offset, (double) offset / (double) size, n, size, (double) (offset + n) / (double) size);
				media->ReportDownloadProgress (progress, progress_offset, false);
			} else {
				/* We got data from a request that is about to be cancelled */
				LOG_PIPELINE ("ProgressiveSource::DataWrite () current_request: %" G_GUINT64_FORMAT " = %.4f offset: %i = %.2f n: %i size: %" G_GUINT64_FORMAT " not reporting progress (before latest brr request)\n",
					current_request, (double) current_request / (double) size, offset, (double) offset / (double) size, n, size);
			}
		}
		media->unref ();
	}
}

void
ProgressiveSource::CheckReadRequestsCallback (EventObject *obj)
{
	((ProgressiveSource *) obj)->CheckReadRequests ();
}

void
ProgressiveSource::CheckReadRequests ()
{
	MediaReadClosureNode *node;
	guint64 request_range = 0;
	gint64 write_pos;
	VERIFY_MAIN_THREAD;

	LOG_PIPELINE ("ProgressiveSource::CheckReadRequests (): brr_enabled: %i size: %" G_GINT64_FORMAT "\n", brr_enabled, size);

	if (size == -1) {
		/* We can't do byte-range-requests if we don't have a size */
		printf ("ProgressiveSource::CheckReadRequests (): can't do brr because size is not known\n");
		return;
	}

	if (brr_enabled == 0) {
		/* Check if we should do byte rante requests (if the server sent a 'Accept-Ranges: bytes' header) */
		if (cancellable != NULL && cancellable->GetRequest () != NULL && cancellable->GetRequest ()->GetResponse () != NULL) {
			if (!cancellable->GetRequest ()->GetResponse ()->ContainsHeader ("Accept-Ranges", "bytes")) {
				LOG_PIPELINE ("ProgressiveSource::CheckReadRequests (): brr disabled\n");
				brr_enabled = 2; /* Disabled */
			} else {
				LOG_PIPELINE ("ProgressiveSource::CheckReadRequests (): brr enabled\n");
				brr_enabled = 1; /* Enabled */
			}
		}
	}

	if (brr_enabled == 2) {
		/* Byte range requests are disabled, do nothing */
		return;
	}

	mutex.Lock ();
	node = (MediaReadClosureNode *) read_closures.First ();
	write_pos = this->write_pos;
	while (node != NULL) {
		MediaReadClosure *closure = node->GetClosure ();
		if (closure->GetOffset () < current_request) {
			/* A position before the last requested position: we need to seek */
			request_range = closure->GetOffset ();
		} else if (write_pos > current_request && write_pos >= closure->GetOffset ()) {
			/* We're currently downloading what's been requested: wait */
		} else if (write_pos > current_request && write_pos < closure->GetOffset () && CalculateDownloadSpeed () * 5 > (closure->GetOffset () - write_pos)) {
			/* It would take less than 5 seconds to reach the requested position if we just wait: wait */
		} else {
			/* It would take more than 5 seconds to reach the requested position if we just wait: seek */
			request_range = closure->GetOffset ();
		}

		if (request_range != 0)
			break;

		node = (MediaReadClosureNode *) node->next;
	}
	mutex.Unlock ();

	if (request_range != 0) {
		current_request = request_range;
		current_request_received = 0;
		HttpRequest *r;
		r = GetDeployment ()->CreateHttpRequest ((HttpRequest::Options) (HttpRequest::CustomHeaders | HttpRequest::DisableFileStorage | HttpRequest::DisableCache | HttpRequest::DisableAsyncSend));
		if (r != NULL) {
			SetRangeRequest (r);
			r->unref ();
		} else {
			fprintf (stderr, "Moonlight: Failed to create http request to execute a byte range request for url: %s", uri == NULL ? "?" : uri->ToString ());
		}
	}
}

void
ProgressiveSource::SetRangeRequest (HttpRequest *value)
{
	HttpRequest *rr;
	Media *media;

	LOG_PIPELINE ("ProgressiveSource::SetRangeRequest (%p), range_request: %p current_request: %" G_GUINT64_FORMAT "\n", value, range_request, current_request);
	VERIFY_MAIN_THREAD;

	rr = range_request;
	range_request = NULL;
	if (rr != NULL) {
		rr->RemoveAllHandlers (this);
		rr->Abort ();
		rr->unref ();
		rr = NULL;
	}

	if (value == NULL)
		return;

	range_request = value;
	range_request->ref ();

	LOG_PIPELINE ("ProgressiveSource::SetRangeRequest (): requesting restart at %" G_GINT64_FORMAT "\n", current_request);
	range_request->AddHandler (HttpRequest::StartedEvent, RangeStartedCallback, this);
	range_request->AddHandler (HttpRequest::WriteEvent, RangeWriteCallback, this);
	range_request->AddHandler (HttpRequest::StoppedEvent, RangeStoppedCallback, this);
	range_request->Open ("GET", uri, resource_base, MediaPolicy);
	range_request->SetHeaderFormatted ("Range", g_strdup_printf ("bytes=%" G_GINT64_FORMAT "-", current_request), false);
	range_request->Send ();

	media = GetMediaReffed ();
	if (media) {
		media->ReportDownloadProgress (0.0, 0.0, true);
		media->unref ();
	}
}

void
ProgressiveSource::RangeStartedHandler (HttpRequest *sender, EventArgs *args)
{
	Cancellable *cancellable;
	HttpResponse *response;

	LOG_PIPELINE ("HttpRequest::RangeStartedHandler ()\n");

	response = sender->GetResponse ();
	if (response->GetResponseStatus () == 206) {
		/* partial content, cancel the first (normal) request, we don't need it anymore */
		mutex.Lock ();
		cancellable = this->cancellable;
		this->cancellable = NULL;
		mutex.Unlock ();
		/* Do work with the mutex unlocked */
		if (cancellable != NULL) {
			cancellable->Cancel ();
			delete cancellable;
			cancellable = NULL;
		}
	} else {
		/* not partial content, just disable brr and forget about it. We'll continue to get the original data we requested. */
		brr_enabled = 2; /* disabled */
		sender->Abort ();
	}
}

void
ProgressiveSource::RangeWriteHandler (HttpRequest *sender, HttpRequestWriteEventArgs *args)
{
	LOG_PIPELINE ("ProgressiveSource::RangeWriteHandler (%p, %p) is_current: %i current_request: %" G_GUINT64_FORMAT " received: %" G_GUINT64_FORMAT " count: %i\n", sender, args, range_request == sender, current_request, current_request_received, args->GetCount ());

	if (range_request == sender) {
		DataWrite (args->GetData (), current_request + current_request_received, args->GetCount ());
		current_request_received += args->GetCount ();
	}
}

void
ProgressiveSource::RangeStoppedHandler (HttpRequest *sender, HttpRequestStoppedEventArgs *args)
{
	LOG_PIPELINE ("HttpRequest::RangeStoppedHandler (%p, %p) success: %i error: %s range_request: %p\n", sender, args, args->IsSuccess (), args->GetErrorMessage (), range_request);

	if (range_request == sender) {
		SetRangeRequest (NULL);
		/* Start up a new request from the latest position we don't have */
		if (size > 0) {
			guint64 start = ranges.GetFirstNotInRange (0, size);
			if (start != G_MAXUINT64) {
				/* There are parts we haven't downloaded yet */
				LOG_PIPELINE ("HttpRequest::RangeStoppedHandler (%p, %p): there are parts we haven't downloaded yet, requesting a brr at offset %" G_GUINT64_FORMAT " = %.4f\n",
					sender, args, start, (double) start / (double) size);

				current_request = start;
				current_request_received = 0;
				HttpRequest *r;
				r = GetDeployment ()->CreateHttpRequest ((HttpRequest::Options) (HttpRequest::CustomHeaders | HttpRequest::DisableFileStorage | HttpRequest::DisableCache | HttpRequest::DisableAsyncSend));
				if (r != NULL) {
					SetRangeRequest (r);
					r->unref ();
				} else {
					fprintf (stderr, "Moonlight: Failed to create http request to execute a byte range request for url: %s", uri == NULL ? "?" : uri->ToString ());
				}
			} else {
				LOG_PIPELINE ("HttpRequest::RangeStoppedHandler (%p, %p): entire file downloaded!\n", sender, args);
			}
		} else {
			LOG_PIPELINE ("HttpRequest::RangeStoppedHandler (%p, %p): no size! %" G_GUINT64_FORMAT "\n", sender, args, size);
		}
	}
}

MediaResult
ProgressiveSource::CheckPendingReadsCallback (MediaClosure *closure)
{
	((ProgressiveSource *) closure->GetContext ())->CheckPendingReads ();
	return MEDIA_SUCCESS;
}

void
ProgressiveSource::CheckPendingReads ()
{
	MediaReadClosureNode *node;
	MediaReadClosureNode *next = NULL;
	bool checked_for_media_thread = false;
	List pending_reads;
	bool ready;
	int c = 0;

	/* This method is thread-safe */
	LOG_PIPELINE ("ProgressiveSource::CheckPendingReads () %i closures to check\n", read_closures.Length ());

	mutex.Lock ();
	/* Check the list of read closures for read requests we can satisfy.
	 * Store those requests in a separate list, and execute the reads with the mutex unlocked */
	node = (MediaReadClosureNode *) read_closures.First ();
	while (node != NULL) {
		MediaReadClosure *closure = node->GetClosure ();
		next = (MediaReadClosureNode *) node->next;
		
		if (size > 0 && closure->GetOffset () + closure->GetCount () > size) {
			/* Requested data after the ended of the file, fixup the count so that it matches the end of the file */
			LOG_PIPELINE ("ProgressiveSource::CheckPendingReads () invalid request (offset %" G_GUINT64_FORMAT " + count %u > size %" G_GUINT64_FORMAT"): "
				"fixing count to %" G_GUINT64_FORMAT "\n", closure->GetOffset (), closure->GetCount (), size, size - closure->GetOffset ());
			closure->SetCount (size - closure->GetOffset ());
		}

		if (complete && brr_enabled != 1 /* enabled */) {
			ready = true;
		} else {
			ready = ranges.Contains (closure->GetOffset (), closure->GetCount ());
		}

		LOG_PIPELINE ("ProgressiveSource::CheckPendingReads () closure #%i: %i (complete: %i brr_enabled: %i offset: %" G_GINT64_FORMAT " count: %i)\n", ++c, ready, complete, brr_enabled, closure->GetOffset (), closure->GetCount ());
		
		if (ready) {
			if (!checked_for_media_thread) {
				if (!Media::InMediaThread ()) {
					/* Only read on media thread, which means marshal this call to the media thread */
					Media *media = GetMediaReffed ();
					if (media) {
						MediaClosure *closure = new MediaClosure (media, CheckPendingReadsCallback, this, "ProgressiveSource::CheckPendingReadsCallback");
						media->EnqueueWork (closure);
						media->unref ();
						closure->unref ();
					}
					break;
				}
				/* We're on the media thread, we can read */
				checked_for_media_thread = true;
			}
			read_closures.Unlink (node);
			pending_reads.Append (node);
		}
		node = next;
	}
	mutex.Unlock ();

	/* Loop over the read closures we've collected and do the actual read */
	node = (MediaReadClosureNode *) pending_reads.First ();
	while (node != NULL) {
		ReadFD (read_fd, node->GetClosure ());
		/* The list (and all the nodes) will be deleted at function exit */
		node = (MediaReadClosureNode *) node->next;
	}
}

void
ProgressiveSource::ReadAsyncInternal (MediaReadClosure *closure)
{
	VERIFY_MEDIA_THREAD;

	LOG_PIPELINE ("ProgressiveSource::ReadAsyncInternal (offset: %" G_GINT64_FORMAT " count: %u)\n", closure->GetOffset (), closure->GetCount ());	
		
	mutex.Lock ();
	read_closures.Append (new MediaReadClosureNode (closure));
	mutex.Unlock ();
	
	CheckPendingReads ();
	AddTickCall (CheckReadRequestsCallback);
}

#if OBJECT_TRACKING
guint32
ProgressiveSource::GetPendingReadRequestCount ()
{
	guint32 result;
	mutex.Lock ();
	result = read_closures.Length ();
	mutex.Unlock ();
	return result;
}
#endif

void
ProgressiveSource::DownloadComplete ()
{
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("ProgressiveSource::DownloadComplete () size: %" G_GINT64_FORMAT " write_pos: %" G_GINT64_FORMAT "\n", size, write_pos);
	
	mutex.Lock ();
	complete = true;
#if SANITY
	if (write_pos != size && size != -1) { // what happend here?
		printf ("ProgressiveSource::DownloadComplete (): the downloaded size (%" G_GINT64_FORMAT ") != the reported size (%" G_GINT64_FORMAT	 ")\n", write_pos, size);
	}
#endif
	this->size = write_pos;
	mutex.Unlock ();

	/* We don't close the write handle here: we might get seeks to positions in the file we didn't have causing more writes */

	CheckPendingReads ();

	if (media) {
		media->ReportDownloadProgress (1.0, GetDownloadProgressOffset (), false);
		media->unref ();
	}
}

void
ProgressiveSource::DownloadFailed ()
{
	LOG_PIPELINE ("ProgressiveSource::DownloadFailed ().\n");
	
	error_occurred = true;
	ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR")));
}

/*
 * MemoryBuffer
 */
 
MemoryBuffer::MemoryBuffer (Media *media, void *memory, gint32 size, bool owner)
	: IMediaObject (Type::MEMORYBUFFER, media)
{
	this->memory = memory;
	this->size = size;
	this->pos = 0;
	this->owner = owner;
}

MemoryBuffer::~MemoryBuffer ()
{
	if (owner)
		g_free (memory);
}

bool
MemoryBuffer::Peek (void *buffer, guint32 count)
{
	if (GetRemainingSize () < count)
		return false;
	memcpy (buffer, ((guint8 *) memory) + pos, count);
	return true;
}

bool
MemoryBuffer::Read (void *buffer, guint32 count)
{
	if (!Peek (buffer, count))
		return false;
	pos += count;
	return true;
}

void
MemoryBuffer::SeekOffset (gint32 offset)
{
	g_return_if_fail (pos + offset >= 0 && pos + offset <= size);

	pos += offset;
}

void
MemoryBuffer::SeekSet (gint32 position)
{
	g_return_if_fail (position >= 0 && position <= size);

	pos = position;
}

/*
 * MediaThreadPool
 */

pthread_mutex_t MediaThreadPool::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t MediaThreadPool::condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t MediaThreadPool::completed_condition = PTHREAD_COND_INITIALIZER;
int MediaThreadPool::count = 0;
pthread_t MediaThreadPool::threads [max_threads];
Media *MediaThreadPool::medias [max_threads];
Deployment *MediaThreadPool::deployments [max_threads];
bool MediaThreadPool::shutting_down = false;
List *MediaThreadPool::queue = NULL;
bool MediaThreadPool::valid [max_threads];

void
MediaThreadPool::AddWork (MediaClosure *closure)
{
	pthread_attr_t attribs;
	int result = 0;
	
	pthread_mutex_lock (&mutex);
	
	if (shutting_down) {
		LOG_PIPELINE ("Moonlight: could not execute closure because we're shutting down.\n");
	} else {
		if (queue == NULL)
			queue = new List ();
		queue->Append (new MediaWork (closure));
		
		// check if all threads are busy with other Media objects
		bool spawn = true;
		if (count == 0) {
			spawn = true;
		} else if (count < max_threads) {
			Media *media = closure->GetMedia ();
			for (int i = 0; i < count; i++) {
				if (medias [i] == NULL || medias [i] == media) {
					spawn = false; // there is a thread working on this media or not working at all.
					break;
				}
			}
		} else {
			spawn = false;
		}
		
		if (spawn) {
			int prev_count = count;
			
			count++; // start up another thread.
			
			LOG_FRAMEREADERLOOP ("MediaThreadPool::AddWork (): spawning a new thread (we'll now have %i thread(s))\n", count);
			
			for (int i = prev_count; i < count && result == 0; i++) {
				valid [i] = false;
				medias [i] = NULL;
				deployments [i] = NULL;
				
				pthread_attr_init (&attribs);
				pthread_attr_setdetachstate (&attribs, PTHREAD_CREATE_JOINABLE);
				result = pthread_create (&threads [i], &attribs, WorkerLoop, NULL);
				pthread_attr_destroy (&attribs);
				
				if (result != 0) {
					fprintf (stderr, "Moonlight: could not create media thread: %s (%i)\n", strerror (result), result);
				} else {
					valid [i] = true;
				}
			}
		}

		LOG_FRAMEREADERLOOP ("MediaThreadLoop::AddWork () got %s %p for media %p (%i) on deployment %p, there are %d nodes left.\n", 
			closure->GetDescription (), closure, closure->GetMedia (), GET_OBJ_ID (closure->GetMedia ()), closure->GetDeployment (), queue ? queue->Length () : -1);
		
		pthread_cond_signal (&condition);
	}
	pthread_mutex_unlock (&mutex);
}

void
MediaThreadPool::WaitForCompletion (Deployment *deployment)
{
	bool waiting = false;
	MediaWork *current = NULL;
	
	LOG_PIPELINE ("MediaThreadPool::WaitForCompletion (%p)\n", deployment);
	
	VERIFY_MAIN_THREAD;
	
	pthread_mutex_lock (&mutex);
	do {
		waiting = false;
		
		/* check if the deployment is being worked on */
		for (int i = 0; i < count; i++) {
			if (deployments [i] == deployment) {
				waiting = true;
				break;
			}
		}
		/* check if the deployment is in the queue */
		if (!waiting && queue != NULL) {
			current = (MediaWork *) queue->First ();	
			while (current != NULL) {
				if (current->closure->GetDeployment () == deployment) {
					waiting = true;
					break;
				}
				current = (MediaWork *) current->next;
			}
		}
		if (waiting) {
			timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100000000; /* 0.1 seconds = 100 milliseconds = 100.000.000 nanoseconds */
			pthread_cond_timedwait (&completed_condition, &mutex, &ts);
		}
	} while (waiting);
	pthread_mutex_unlock (&mutex);
}

void
MediaThreadPool::RemoveWork (Media *media)
{
	LOG_PIPELINE ("MediaThreadPool::RemoveWork (%p = %i)\n", media, GET_OBJ_ID (media));
	
	List::Node *next;
	List::Node *first = NULL;
	List::Node *last = NULL;
	List::Node *current = NULL;
	int counter = 0;
	
	pthread_mutex_lock (&mutex);

	// create a list of nodes to delete
	current = queue != NULL ? queue->First () : NULL;
	while (current != NULL) {
		next = current->next; // retrieve next before Unlinking
		MediaWork *mw = (MediaWork *) current;
		if (mw->closure->GetMedia () == media) {
			queue->Unlink (current);
			if (first == NULL) {
				first = current;
			} else {
				last->next = current;
			}
			last = current;
			counter++;
			break;
		}
		current = next;
	}
	
	pthread_mutex_unlock (&mutex);

	// We have to delete the list nodes with the
	// queue mutex unlocked, due to refcounting
	// (our node's (MediaWork) dtor will cause unrefs,
	// which may cause other dtors to be called,
	// eventually ending up wanting to lock the mutex
	// again).
	
	current = first;
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
}

bool
MediaThreadPool::IsThreadPoolThread ()
{
	bool result = false;
	pthread_mutex_lock (&mutex);
	for (int i = 0; i < count; i++) {
		if (pthread_equal (pthread_self (), threads [i])) {
			result = true;
			break;
		}
	}
	pthread_mutex_unlock (&mutex);
	return result;
}

void
MediaThreadPool::Initialize ()
{
	LOG_PIPELINE ("MediaThreadPool::Initialize ()\n");
	VERIFY_MAIN_THREAD;
	
	shutting_down = false; // this may be true if the user closed a moonlight-tab (we'd shutdown), then opened another moonlight-tab.
}

void
MediaThreadPool::Shutdown ()
{
	List::Node *current = NULL;
	List::Node *next = NULL;
	
	LOG_PIPELINE ("MediaThreadPool::Shutdown (), we have %i thread(s) to shut down\n", count);
	
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (!shutting_down);
	
	pthread_mutex_lock (&mutex);
	
	shutting_down = true;
	pthread_cond_broadcast (&condition);
	
	for (int i = 0; i < count; i++) {
		if (!valid [i])
			continue;
		
		pthread_mutex_unlock (&mutex);
		pthread_join (threads [i], NULL);
		pthread_mutex_lock (&mutex);
	}
	
	if (queue != NULL) {
		current = queue->First ();
		queue->Clear (false);
		delete queue;
		queue = NULL;
	}
	count = 0;
	
	pthread_mutex_unlock (&mutex);
	
	// deleting a node can have side-effects, so we first copy the list of nodes, 
	// clear the original and loop over the copy while deleting the nodes.
	// this prevents any reentering issues while deleting nodes.
	while (current != NULL) {
		next = current->next;
		delete current;
		current = next;
	}
	
	LOG_PIPELINE ("MediaThreadPool::Shutdown () [Completed]\n");	
}

void *
MediaThreadPool::WorkerLoop (void *data)
{
	MediaWork *node = NULL;
	Media *media = NULL;
	int self_index = -1;
	
	/*
	 * Unblock any signals. We inherit the blocked signals from the thread that
	 * created us, and if that thread happens to be a thread that has signals
	 * blocked, we might end up deadlocking in the gc (since the gc delivers
	 * a suspend signal, this thread never gets it because the signal is blocked,
	 * and the gc waits for us to handle the suspend signal).
	 * The pulseaudio thread is one example of a thread that has all signals
	 * blocked, causing this issue if we create a new thread from the
	 * pulseaudio thread.
	 */
	
	sigset_t signal_set;
	int err = 0;
	if ((err = sigemptyset (&signal_set)) != 0) {
		fprintf (stderr, "Moonlight: Media thread pool was unable to create an empty set of signals: %s (%i).\n", strerror (err), err);
	} else if ((err = pthread_sigmask (SIG_SETMASK, &signal_set, NULL)) != 0) {
		fprintf (stderr, "Moonlight: Media thread pool was unable to unblock all signals: %s (%i).\n", strerror (err), err);
	}
	if (err != 0) {
		/* Something failed. Check if all signals are unblocked, if not, exit
		 * the thread. Exiting the thread might cause media playback to fail,
		 * while continuing with blocked signals will probably end up
		 * deadlocking the gc.*/
		bool any_blocked_signals = false;
		 
		if (pthread_sigmask (SIG_BLOCK, NULL, &signal_set) != 0) {
			any_blocked_signals = true; /* Assume the worst */
#ifdef HAVE_SIGISEMPTYSET
		} else if (!sigisemptyset (&signal_set)) {
			any_blocked_signals = true;
#endif
		}

		if (any_blocked_signals) {
			fprintf (stderr, "Moonlight: A media thread was started with blocked signals and could not unblock them. The media thread will exit (this may cause media playback to fail).\n");
			return NULL;
		}
	}
	
	pthread_mutex_lock (&mutex);
	for (int i = 0; i < count; i++) {
		if (pthread_equal (threads [i], pthread_self ())) {
			self_index = i;
			break;
		}
	}
	pthread_mutex_unlock (&mutex);
	
	LOG_PIPELINE ("MediaThreadPool::WorkerLoop () %u: Started thread with index %i.\n", (int) pthread_self (), self_index);
	
	g_return_val_if_fail (self_index >= 0, NULL);
	
	while (!shutting_down) {
		pthread_mutex_lock (&mutex);
		
		medias [self_index] = NULL;
		deployments [self_index] = NULL;
		/* if anybody was waiting for us to finish working, notify them */
		if (media != NULL)
			pthread_cond_signal (&completed_condition);

		media = NULL;		
		node = (MediaWork *) (queue != NULL ? queue->First () : NULL);
		
		while (node != NULL) {
			media = node->closure->GetMedia ();
			
			for (int i = 0; i < count; i++) {
				if (medias [i] == media) {
					// another thread is working for the same media object.
					// we need to find something else to do.
					media = NULL;
					break;
				}
			}
			
			if (media != NULL)
				break;
			
			node = (MediaWork *) node->next;
		}
		
		if (node == NULL) {
			pthread_cond_wait (&condition, &mutex);
		} else {
			queue->Unlink (node);
		}
		
		if (node != NULL) {
			medias [self_index] = media;
			/* At this point the current deployment might be wrong, so avoid
			 * the warnings in GetDeployment. Do not move the call to SetCurrenDeployment
			 * here, since it might end up doing a lot of work with the mutex
			 * locked. */
			deployments [self_index] = media->GetUnsafeDeployment ();
		}
		
		pthread_mutex_unlock (&mutex);
		
		if (node == NULL)
			continue;
		
		media->SetCurrentDeployment (true, true);

		LOG_FRAMEREADERLOOP ("MediaThreadLoop::WorkerLoop () %u: got %s %p for media %p on deployment %p, there are %d nodes left.\n", (int) pthread_self (), node->closure->GetDescription (), node, media, media->GetDeployment (), queue ? queue->Length () : -1);
		
		node->closure->Call ();
		
		LOG_FRAMEREADERLOOP ("MediaThreadLoop::WorkerLoop () %u: processed node %p\n", (int) pthread_self (), node);
		
		delete node;
	}
	
	pthread_mutex_lock (&mutex);
	deployments [self_index] = NULL;
	medias [self_index] = NULL;
	/* if anybody was waiting for us to finish working, notify them */
	if (media != NULL)
		pthread_cond_signal (&completed_condition);
	pthread_mutex_unlock (&mutex);
	
	LOG_PIPELINE ("MediaThreadPool::WorkerLoop () %u: Exited (index: %i).\n", (int) pthread_self (), self_index);
	
	return NULL;
}


/*
 * MediaClosure
 */ 

MediaClosure::MediaClosure (Media *media, MediaCallback *callback, EventObject *context, const char *description)
	: EventObject (Type::MEDIACLOSURE, true)
{
	Init (media, callback, context);
	this->description = description;
}

MediaClosure::MediaClosure (Type::Kind object_kind, Media *media, MediaCallback *callback, EventObject *context)
	: EventObject (object_kind, true)
{
	Init (media, callback, context);
}

void
MediaClosure::Init (Media *media, MediaCallback *callback, EventObject *context)
{
	result = MEDIA_INVALID;
	description = NULL;
	this->callback = callback;
	this->context = context;
	if (this->context)
		this->context->ref ();
	this->media = media;
	if (this->media)
		this->media->ref ();
	
	// put checks at the end so that fields are still initialized, since we can't abort construction.
	g_return_if_fail (callback != NULL);
	g_return_if_fail (media != NULL);
}

void
MediaClosure::Dispose ()
{
	if (context) {
		context->unref ();
		context = NULL;
	}
	
	if (media) {
		media->unref ();
		media = NULL;
	}
	
	callback = NULL;
	
	EventObject::Dispose ();
}

void
MediaClosure::Call ()
{
	if (callback) {
		result = callback (this);
	} else {
		result = MEDIA_NO_CALLBACK;
	}
}

/*
 * MediaDisposeObjectClosure
 */
MediaDisposeObjectClosure::MediaDisposeObjectClosure (Media *media, MediaCallback *callback, EventObject *context)
	: MediaClosure (Type::MEDIADISPOSEOBJECTCLOSURE, media, callback, context)
{
}

void
MediaDisposeObjectClosure::Dispose ()
{
	if (!CallExecuted ()) {
		// we haven't been executed. do it now.
#if SANITY && DEBUG
		LOG_PIPELINE ("MediaDisposeObjectClosure::~MediaDisposeObjectClosure (): callback hasn't been executed, we'll do it now.\n");
#endif
		Call ();
	}
	
	MediaClosure::Dispose ();
}

/*
 * MediaSeekClosure
 */
MediaSeekClosure::MediaSeekClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, guint64 pts)
	: MediaClosure (Type::MEDIASEEKCLOSURE, media, callback, context)
{
	this->pts = pts;
}

/*
 * MediaReportSeekCompletedClosure
 */

MediaReportSeekCompletedClosure::MediaReportSeekCompletedClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, guint64 pts)
	: MediaClosure (Type::MEDIAREPORTSEEKCOMPLETEDCLOSURE, media, callback, context)
{
	g_return_if_fail (context != NULL);
	
	this->pts = pts;
}

MediaReportSeekCompletedClosure::~MediaReportSeekCompletedClosure ()
{
}

void
MediaReportSeekCompletedClosure::Dispose ()
{
	MediaClosure::Dispose ();
}

/*
 * MediaGetFrameClosure
 */

MediaGetFrameClosure::MediaGetFrameClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, IMediaStream *stream)
	: MediaClosure (Type::MEDIAGETFRAMECLOSURE, media, callback, context)
{
	this->stream = NULL;
	
	g_return_if_fail (context != NULL);
	g_return_if_fail (stream != NULL);
	
	this->stream = stream;
	// this->stream->ref ();
	
	//fprintf (stderr, "MediaGetFrameClosure::MediaGetFrameClosure ()  id: %i\n", GetId ());
}

MediaGetFrameClosure::~MediaGetFrameClosure ()
{
	//fprintf (stderr, "MediaGetFrameClosure::~MediaGetFrameClosure () id: %i\n", GetId ()); 
}

void
MediaGetFrameClosure::Dispose ()
{
	if (stream) {
	//	stream->unref ();
		stream = NULL;
	}
	
	MediaClosure::Dispose ();
	//fprintf (stderr, "MediaGetFrameClosure::Dispose () id: %i\n", GetId ());
}

/*
 * MediaReportFrameCompletedClosure
 */

MediaReportFrameCompletedClosure::MediaReportFrameCompletedClosure (Media *media, MediaCallback *callback, IMediaDemuxer *context, MediaFrame *frame)
	: MediaClosure (Type::MEDIAGETFRAMECLOSURE, media, callback, context)
{
	this->frame = NULL;
	
	g_return_if_fail (context != NULL);
	
	this->frame = frame;
	if (this->frame)
		this->frame->ref ();
}

void
MediaReportFrameCompletedClosure::Dispose ()
{
	if (frame) {
		frame->unref ();
		frame = NULL;
	}
	
	MediaClosure::Dispose ();
}

/*
 * MediaReportDecodeFrameCompletedClosure
 */

MediaReportDecodeFrameCompletedClosure::MediaReportDecodeFrameCompletedClosure (Media *media, MediaCallback *callback, IMediaDecoder *context, MediaFrame *frame)
	: MediaClosure (Type::MEDIAREPORTDECODEFRAMECOMPLETEDCLOSURE, media, callback, context)
{
	this->frame = NULL;
	
	g_return_if_fail (context != NULL);
	
	this->frame = frame;
	if (this->frame)
		this->frame->ref ();
}

void
MediaReportDecodeFrameCompletedClosure::Dispose ()
{
	if (frame) {
		frame->unref ();
		frame = NULL;
	}

	MediaClosure::Dispose ();
}

/*
 * IMediaStream
 */

IMediaStream::IMediaStream (Type::Kind kind, Media *media) : IMediaObject (kind, media)
{
	context = NULL;
	
	extra_data_size = 0;
	extra_data = NULL;
	
	duration = 0;
	pts_per_frame = 0;
	
	decoder = NULL;
	codec_id = 0;
	codec = NULL;
	
	min_padding = 0;
	index = -1;
	selected = false;
	input_ended = false;
	output_ended = false;
	
	first_pts = G_MAXUINT64; // The first pts in the stream, initialized to G_MAXUINT64
	last_popped_pts = G_MAXUINT64; // The pts of the last frame returned, initialized to G_MAXUINT64
	last_enqueued_pts = G_MAXUINT64; // The pts of the last frame enqueued, initialized to G_MAXUINT64
	last_available_pts = 0; // The pts of the last available frame, initialized to 0
}

void
IMediaStream::Dispose ()
{
	if (decoder) {
		IMediaDecoder *d = decoder;
		decoder = NULL;
		d->Dispose ();
		d->unref ();
	}
	g_free (extra_data);
	extra_data = NULL;
	g_free (codec);
	codec = NULL;

	ClearQueue ();
	IMediaObject::Dispose ();
}

char *
IMediaStream::CreateCodec (int codec_id)
{
	switch (codec_id) {
	case CODEC_WMV1:  return g_strdup ("wmv1");
	case CODEC_WMV2:  return g_strdup ("wmv2");
	case CODEC_WMV3:  return g_strdup ("wmv3");
	case CODEC_WMVA:  return g_strdup ("wmva");
	case CODEC_WVC1:  return g_strdup ("vc1");
	case CODEC_RGBA:  return g_strdup ("rgba");
	case CODEC_YV12:  return g_strdup ("yv12");
	case CODEC_MP3:   return g_strdup ("mp3");
	case CODEC_WMAV1: return g_strdup ("wmav1");
	case CODEC_WMAV2: return g_strdup ("wmav2");
	case CODEC_WMAV3: return g_strdup ("wmav3");
	case CODEC_PCM:   return g_strdup ("pcm");
	case CODEC_MPEG_RAW_AAC:
	case CODEC_mp4a:
	case CODEC_MP4A:  return g_strdup ("aac");
	case CODEC_H264:
	case CODEC_avc1:
	case CODEC_AVC1:  return g_strdup ("h264");
	case CODEC_ASF_MARKER: return g_strdup ("asf-marker");
	default:
		char result [5];
		int size;
		int a = (codec_id & 0x000000FF);
		int b = (codec_id & 0x0000FF00) >> 8;
		int c = (codec_id & 0x00FF0000) >> 16;
		int d = (codec_id & 0xFF000000) >> 24;

		size = 0;
		if (a != 0)
			result [size++] = (char) a;
		if (b != 0)
			result [size++] = (char) b;
		if (c != 0)
			result [size++] = (char) c;
		if (d != 0)
			result [size++] = (char) d;
		result [size] = 0;

		return g_strdup (result);
	}
	
}

bool
IMediaStream::IsQueueEmpty ()
{
	return queue.IsEmpty ();
}

void
IMediaStream::ReportSeekCompleted ()
{
	LOG_PIPELINE ("IMediaStream::ReportSeekCompleted ()\n");
	input_ended = false;
	output_ended = false;
	ClearQueue ();
	if (decoder != NULL)
		decoder->ReportSeekCompleted ();
}

IMediaDemuxer *
IMediaStream::GetDemuxerReffed ()
{
	Media *media;
	IMediaDemuxer *result;
	
	if (IsDisposed ())
		return NULL;
	
	media = GetMediaReffed ();
		
	g_return_val_if_fail (media != NULL, NULL);
	
	result = media->GetDemuxerReffed ();
	
	media->unref ();
	
	return result;
}

IMediaDecoder *
IMediaStream::GetDecoder ()
{
	return decoder;
}

void
IMediaStream::SetDecoder (IMediaDecoder *value)
{
	if (decoder)
		decoder->unref ();
	decoder = value;
	if (decoder)
		decoder->ref ();
}

void
IMediaStream::SetCodecId (int value)
{
	codec_id = value;
	if (codec != NULL)
		g_free (codec);
	codec = CreateCodec (codec_id);
}

bool
IMediaStream::GetOutputEnded ()
{
	return output_ended;
}

void
IMediaStream::SetOutputEnded (bool value)
{
	output_ended = value;
}

bool
IMediaStream::GetInputEnded ()
{
	return input_ended;
}

void
IMediaStream::SetInputEnded (bool value)
{
	input_ended = value;
	if (GetDecoder () != NULL)
		GetDecoder ()->ReportInputEnded ();
}

guint64
IMediaStream::GetBufferedSize ()
{
	guint64 result;
	
	queue.Lock ();
	if (first_pts == G_MAXUINT64 || last_enqueued_pts == G_MAXUINT64)
		result = 0;
	else if (last_popped_pts == G_MAXUINT64)
		result = last_enqueued_pts - first_pts;
	else
		result = last_enqueued_pts - last_popped_pts;
	queue.Unlock ();

	LOG_BUFFERING ("IMediaStream::GetBufferedSize (): id: %i, codec: %s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %" G_GUINT64_FORMAT " ms, result: %" G_GUINT64_FORMAT " ms\n",
		GET_OBJ_ID (this), codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), MilliSeconds_FromPts (result));

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
		printf ("size: %.4" G_GINT64_FORMAT ", first: %.4" G_GINT64_FORMAT ", last popped: %.4" G_GINT64_FORMAT ", last enq: %.4" G_GINT64_FORMAT ", frames enq: %i>",
			TO_MS (buffer_size), TO_MS (first_pts), TO_MS (last_popped_pts), 
			TO_MS (last_enqueued_pts), queue.Length ());
	} else {
		printf ("(not selected) >");
	}
}
#endif

void
IMediaStream::EnqueueFrame (MediaFrame *frame)
{
	bool first = false;
	guint64 seeked_to_pts = 0;
	Media *media;
	IMediaDemuxer *demuxer = NULL;
	/* Add nodes to be deleted here, they'll automaticall be deleted when the method exits. */
	/* The reason for doing this at method exit is to not do deletion (with unrefs, etc) with a mutex held */
	List trash;
	
	g_return_if_fail (Media::InMediaThread ());
	
	media = GetMediaReffed ();
	g_return_if_fail (media != NULL);
	
	if (media->IsStopped ()) {
		/* We need to enqueue one frame so that we can render the first frame for a stopped media element */
		if (!IsQueueEmpty ()) {
			LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p): stopped, not enqueuing frame (we already have at least one frame).\n", frame);
			goto cleanup;
		} else {
			LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p): stopped, but enqueing since we're empty.\n", frame);
		}
	}
	
	if (frame->GetBuffer () == NULL && !frame->IsPlanar ()) {
		/* for some reason there is no output from the decoder, possibly because it needs more data from the demuxer before outputting anything */
		LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p): No data in frame, not storing it.\n", frame);
		goto cleanup;
	}
	
	demuxer = GetDemuxerReffed ();
	if (demuxer == NULL) {
		LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p): No demuxer.\n", frame);
		goto cleanup;
	}
	seeked_to_pts = demuxer->GetSeekedToPts ();
	demuxer->unref ();
	
	queue.Lock ();
	if (first_pts == G_MAXUINT64)
		first_pts = frame->pts;

	LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p) %s %" G_GUINT64_FORMAT " ms\n", frame, frame ? frame->stream->GetTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : 0);

#if 0
	if (last_enqueued_pts > frame->pts && last_enqueued_pts != G_MAXUINT64 && frame->event != FrameEventEOF && frame->buflen > 0) {
		g_warning ("IMediaStream::EnqueueFrame (): codec: %.5s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %" G_GUINT64_FORMAT " ms, "
		"buffer: %" G_GUINT64_FORMAT " ms, frame: %p, frame->buflen: %i, frame->pts: %" G_GUINT64_FORMAT " ms (the last enqueued frame's pts is below the current frame's pts)\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), frame, frame->buflen, MilliSeconds_FromPts (frame->pts));
	}
#endif

	last_enqueued_pts = frame->pts;
	first = queue.LinkedList ()->Length () == 0;
	queue.LinkedList ()->Append (new StreamNode (frame));

	if (seeked_to_pts != G_MAXUINT64 && first_pts < seeked_to_pts 
		&& (GetObjectType () == Type::AUDIOSTREAM || GetObjectType () == Type::VIDEOSTREAM)) {
		StreamNode *last_key_frame = NULL;
		StreamNode *node;
		StreamNode *n;

		/* we need to remove any frames before the last key frame before the seeked-to pts */

		/* find the last key frame below the seeked-to pts */
		node = (StreamNode *) queue.LinkedList ()->First ();
		while (node != NULL && node->GetFrame ()->pts < seeked_to_pts) {
			if (GetObjectType () == Type::AUDIOSTREAM || node->GetFrame ()->IsKeyFrame ())
				last_key_frame = node;

			node = (StreamNode *) node->next;
		};

		if (last_key_frame != NULL) {
			/* remove any frames before that last key frame */
			node = (StreamNode *) last_key_frame->prev;
			while (node != NULL) {
				n = (StreamNode *) node->prev;
				queue.LinkedList ()->Unlink (node);
				LOG_PIPELINE ("%s::EnqueueFrame (): removing node with pts %" G_GUINT64_FORMAT "\n", GetTypeName (), node->GetFrame ()->pts);
				trash.Append (node);
				node = n;
			}
	
			/* update the first pts to point to the real first pts */
			node = (StreamNode *) queue.LinkedList ()->First ();
			guint64 next_first_pts = node == NULL ? G_MAXUINT64 : node->GetFrame ()->pts;
			LOG_PIPELINE ("%s::EnqueueFrame (): setting first_pts to: %" G_GUINT64_FORMAT ", from %" G_GUINT64_FORMAT " (demuxer first pts: %" G_GUINT64_FORMAT ")\n",
				GetTypeName (), first_pts, next_first_pts, seeked_to_pts);
			first_pts = next_first_pts;
		}
	}
	
	queue.Unlock ();

	if (first)
		EmitSafe (FirstFrameEnqueuedEvent);
	
	FrameEnqueued ();

cleanup:
	media->unref ();

	LOG_BUFFERING ("IMediaStream::EnqueueFrame (): codec: %.5s, first: %i, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %" G_GUINT64_FORMAT " ms, buffer: %" G_GUINT64_FORMAT " ms, frame: %p, frame->buflen: %i\n",
		codec, first, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts - first_pts), frame, frame->GetBufLen ());
}

MediaFrame *
IMediaStream::PopFrame ()
{
	MediaFrame *result = NULL;
	StreamNode *node = NULL;

	// We use the queue lock to synchronize access to
	// last_popped_pts/last_enqueued_pts/first_pts

	queue.Lock ();
	node = (StreamNode *) queue.LinkedList ()->First ();
	if (node != NULL) {
		result = node->GetFrame ();
		result->ref ();
		queue.LinkedList ()->Remove (node);
		last_popped_pts = result->pts;
	}
	queue.Unlock ();
	
	LOG_BUFFERING ("IMediaStream::PopFrame (): codec: %.5s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %" G_GUINT64_FORMAT " ms, buffer: %" G_GUINT64_FORMAT " ms, frame: %p, frame->buflen: %i\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), result, result ? result->GetBufLen () : 0);

	if (!input_ended && !output_ended && result != NULL) {
		IMediaDemuxer *demuxer = GetDemuxerReffed ();
		if (demuxer != NULL) {
			demuxer->FillBuffers ();
			demuxer->unref ();
		}
	}

	return result;
}

void
IMediaStream::ClearQueue ()
{
	LOG_BUFFERING ("IMediaStream::ClearQueue ()\n");
	queue.Lock ();
	queue.LinkedList ()->Clear (true);
	first_pts = G_MAXUINT64;
	last_popped_pts = G_MAXUINT64;
	last_enqueued_pts = G_MAXUINT64;
	queue.Unlock ();
}

void
IMediaStream::SetSelected (bool value)
{
	IMediaDemuxer *demuxer;
	
	selected = value;

	demuxer = GetDemuxerReffed ();

	if (demuxer != NULL) {
		demuxer->UpdateSelected (this);
		demuxer->FillBuffers ();
		demuxer->unref ();
	}
}

/*
 * IMediaStream.StreamNode
 */

IMediaStream::StreamNode::StreamNode (MediaFrame *f)
{ 
	frame = f;
	frame->ref ();
}

IMediaStream::StreamNode::~StreamNode ()
{
	frame->unref ();
	frame = NULL;
}
/*
 * IMediaDemuxer
 */ 
 
IMediaDemuxer::IMediaDemuxer (Type::Kind kind, Media *media, IMediaSource *source) : IMediaObject (kind, media)
{
	this->source = source;
	this->source->ref ();
	stream_count = 0;
	streams = NULL;
	drm = false;
	opened = false;
	opening = false;
	seeking = false;
	seek_pending = false;
	pending_stream = NULL;
	pending_fill_buffers = false;
	seeked_to_pts = G_MAXUINT64;
}

IMediaDemuxer::IMediaDemuxer (Type::Kind kind, Media *media)
	: IMediaObject (kind, media)
{
	source = NULL;
	stream_count = 0;
	streams = NULL;
	drm = false;
	opened = false;
	opening = false;
	seeking = false;
	seek_pending = false;
	pending_stream = NULL;
	pending_fill_buffers = false;
	seeked_to_pts = G_MAXUINT64;
}

void
IMediaDemuxer::Dispose ()
{
	if (streams != NULL) {
		IMediaStream **tmp = streams;
		int stream_count = this->stream_count;
		this->stream_count = 0;
		streams = NULL;
		for (int i = 0; i < stream_count; i++) {
			tmp [i]->Dispose ();
			tmp [i]->unref ();
		}
		g_free (tmp);
	}
	if (source) {
		source->unref ();
		source = NULL;
	}
	if (pending_stream != NULL) {
		pending_stream->unref ();
		pending_stream = NULL;
	}
	opened = false;
	IMediaObject::Dispose ();
}

MediaResult
IMediaDemuxer::OpenCallback (MediaClosure *closure)
{
	IMediaDemuxer *demuxer;
	
	LOG_PIPELINE ("IMediaDemuxer::OpenCallback (%p)\n", closure);
	
	demuxer = (IMediaDemuxer *) closure->GetContext ();
	demuxer->OpenDemuxerAsync ();
	
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::EnqueueOpen ()
{
	MediaClosure *closure;
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("IMediaDemuxer::EnqueueOpen ()\n");
	
	if (media == NULL)
		return;

	closure = new MediaClosure (media, OpenCallback, this, "IMediaDemuxer::OpenCallback");
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::EnqueueReportOpenDemuxerCompleted ()
{
	MediaClosure *closure;
	Media *media = GetMediaReffed ();

	LOG_PIPELINE ("IMediaDemuxer::EnqueueReportOpenDemuxerCompleted ()\n");

	if (media == NULL)
		return;

	closure = new MediaClosure (media, ReportOpenDemuxerCompletedCallback, this, "IMediaDemuxer::ReportOpenDemuxerCompletedCallback");
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

MediaResult
IMediaDemuxer::ReportOpenDemuxerCompletedCallback (MediaClosure *closure)
{
	((IMediaDemuxer *) closure->GetContext ())->ReportOpenDemuxerCompleted ();
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::ReportOpenDemuxerCompleted ()
{
	Media *media;
	
	LOG_PIPELINE ("IMediaDemuxer::ReportDemuxerOpenCompleted ()\n");

	/* Ensure we're on a media thread */
	if (!Media::InMediaThread ()) {
		EnqueueReportOpenDemuxerCompleted ();
		return;
	}

	/* Media might be null if we got disposed for some reason. */
	media = GetMediaReffed ();
	if (media == NULL)
		return;

	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaStream *stream = GetStream (i);
		VideoStream *vs;
		if (stream == NULL || !stream->IsVideo ())
			continue;
		vs = (VideoStream *)  stream;
		if ((vs->GetHeight () > MAX_VIDEO_HEIGHT) || (vs->GetWidth () > MAX_VIDEO_WIDTH)) {
			char *msg = g_strdup_printf ("%s: Video stream size (width: %d, height: %d) outside limits (%d, %d)", 
				GetTypeName (), vs->GetHeight (), vs->GetWidth (), MAX_VIDEO_HEIGHT, MAX_VIDEO_WIDTH);
			ReportErrorOccurred (msg);
			g_free (msg);
			return;
		}

	}

	opened = true;
	opening = false;
	
	media->ReportOpenDemuxerCompleted ();
	media->unref ();

	EmitSafe (IMediaDemuxer::OpenedEvent);
}

void
IMediaDemuxer::ReportGetFrameProgress (double progress)
{
	LOG_PIPELINE ("IMediaDemuxer::ReportGetFrameProgress (%f)\n", progress);
}

void
IMediaDemuxer::ReportSwitchMediaStreamCompleted (IMediaStream *stream)
{
	LOG_PIPELINE ("IMediaDemuxer::ReportSwitchMediaStreamCompleted (%p)\n", stream);
}

void
IMediaDemuxer::ReportGetDiagnosticCompleted (MediaStreamSourceDiagnosticKind kind, gint64 value)
{
	LOG_PIPELINE ("IMediaDemuxer::ReportGetDiagnosticCompleted (%i, %" G_GINT64_FORMAT ")\n", kind, value);
}

void
IMediaDemuxer::EnqueueReportGetFrameCompleted (MediaFrame *frame)
{
	Media *media = GetMediaReffed ();

	if (media == NULL)
		return;

	MediaClosure *closure = new MediaReportFrameCompletedClosure (media, ReportGetFrameCompletedCallback, this, frame);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

MediaResult
IMediaDemuxer::ReportGetFrameCompletedCallback (MediaClosure *closure)
{
	MediaReportFrameCompletedClosure *c = (MediaReportFrameCompletedClosure *) closure;
	
	g_return_val_if_fail (c != NULL, MEDIA_FAIL);
	g_return_val_if_fail (c->GetDemuxer () != NULL, MEDIA_FAIL);
	
	c->GetDemuxer ()->ReportGetFrameCompleted (c->GetFrame ());
	
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::ReportGetFrameCompleted (MediaFrame *frame)
{
	Media *media = NULL;
	
	g_return_if_fail (frame == NULL || (frame != NULL && frame->stream != NULL));
	g_return_if_fail (pending_stream != NULL);
	g_return_if_fail (frame == NULL || pending_stream == frame->stream);

	/* Ensure we're on a media thread */
	if (!Media::InMediaThread ()) {
		EnqueueReportGetFrameCompleted (frame);
		return;
	}

	/* Media might be null if we got disposed somehow */
	media = GetMediaReffed ();
	if (media == NULL) {
		return;
	}

	LOG_PIPELINE ("IMediaDemuxer::ReportGetFrameCompleted (%p) %i %s %" G_GUINT64_FORMAT " ms\n", frame, GET_OBJ_ID (this), frame ? frame->stream->GetTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : (guint64) -1);
	
	if (frame == NULL) {
		LOG_PIPELINE ("IMediaDemuxer::ReportGetFrameCompleted (%p): input end signaled for %s stream.\n", frame, pending_stream->GetTypeName ());
		// No more data for this stream
		pending_stream->SetInputEnded (true);
	} else if (!frame->stream->IsDisposed ()) {
		IMediaDecoder *decoder = frame->stream->GetDecoder ();
		if (decoder != NULL)
			decoder->DecodeFrameAsync (frame, true /* always enqueue */);
	}
	
	pending_stream->unref ();
	pending_stream = NULL; // not waiting for anything more
	
	// enqueue some more 
	FillBuffers ();

	if (media)
		media->unref ();
}

MediaResult
IMediaDemuxer::ReportSeekCompletedCallback (MediaClosure *c)
{
	MediaReportSeekCompletedClosure *closure = (MediaReportSeekCompletedClosure *) c;
	IMediaDemuxer *demuxer;
	
	g_return_val_if_fail (closure != NULL, MEDIA_FAIL);
	g_return_val_if_fail (closure->GetContext () != NULL, MEDIA_FAIL);
	
	demuxer = (IMediaDemuxer *) closure->GetContext ();
	demuxer->ReportSeekCompleted (closure->GetPts ());
	
	return MEDIA_SUCCESS;
}

void 
IMediaDemuxer::EnqueueReportSeekCompleted (guint64 pts)
{
	Media *media = GetMediaReffed ();

	if (media == NULL)
		return;

	MediaClosure *closure = new MediaReportSeekCompletedClosure (media, ReportSeekCompletedCallback, this, pts);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::ReportSeekCompleted (guint64 pts)
{
	Media *media;

	LOG_PIPELINE ("IMediaDemuxer::ReportSeekCompleted (%" G_GUINT64_FORMAT ")\n", pts);
	
	g_return_if_fail (seeking);
	g_return_if_fail (seek_pending);
	
	if (!Media::InMediaThread ()) {
		EnqueueReportSeekCompleted (pts);
		return;
	}
	
#if SANITY
	if (pending_stream != NULL)
		printf ("IMediaDemuxer::ReportSeekCompleted (%" G_GUINT64_FORMAT "): we can't be waiting for a frame now.\n", pts);
#endif

	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	/* We need to call ReportSeekCompleted once for every time SeekAsync(pts) was called */
	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaStream *stream = GetStream (i);
		
		if (stream == NULL)
			continue;
		
		stream->ReportSeekCompleted ();
	}
		
	mutex.Lock ();
	seeks.RemoveAt (0);
	seeking = !seeks.IsEmpty ();
	mutex.Unlock ();
	
	seek_pending = false;
	
	media->ReportSeekCompleted (pts);
	media->unref ();
	
	if (!seeking) {
		seeked_to_pts = pts;
		pending_fill_buffers = false;
		FillBuffers ();
	} else {
		LOG_PIPELINE ("IMediaDemuxer::ReportSeekCompleted (%" G_GUINT64_FORMAT "): still pending seeks, enqueuing another seek.\n", pts);
		EnqueueSeek ();
	}
	
	LOG_PIPELINE ("IMediaDemuxer::ReportSeekCompleted (%" G_GUINT64_FORMAT ") [Done]\n", pts);
}

void
IMediaDemuxer::OpenDemuxerAsync ()
{
	g_return_if_fail (opened == false);
	
	opening = true;
	opened = false;
	OpenDemuxerAsyncInternal ();
}

MediaResult
IMediaDemuxer::GetFrameCallback (MediaClosure *c)
{
	MediaGetFrameClosure *closure = (MediaGetFrameClosure *) c;
	IMediaDemuxer *demuxer;
	
	g_return_val_if_fail (closure != NULL, MEDIA_FAIL);
	g_return_val_if_fail (closure->GetStream () != NULL, MEDIA_FAIL);
	g_return_val_if_fail (closure->GetContext () != NULL, MEDIA_FAIL);
	
	demuxer = (IMediaDemuxer *) closure->GetContext ();
	demuxer->GetFrameAsync (closure->GetStream ());
	
	return MEDIA_SUCCESS;
}

void 
IMediaDemuxer::EnqueueGetFrame (IMediaStream *stream)
{
	g_return_if_fail (pending_stream == NULL); // we can't be waiting for another frame.
	
	Media *media = GetMediaReffed ();

	if (media == NULL)
		return;

	MediaClosure *closure = new MediaGetFrameClosure (media, GetFrameCallback, this, stream);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::GetFrameAsync (IMediaStream *stream)
{
	Media *media = NULL;
	
	LOG_PIPELINE ("IMediaDemuxer::GetFrameAsync (%p) %s InMediaThread: %i\n", stream, stream->GetTypeName (), Media::InMediaThread ());
	
	if (!Media::InMediaThread ()) {
		EnqueueGetFrame (stream);
		return;
	}
	
	if (seeking) {
		LOG_PIPELINE ("IMediaDemuxer::GetFrameAsync (): delayed since we're waiting for a seek.\n");
		goto cleanup;
	}
	
	if (pending_stream != NULL) {
		/* we're already waiting for a frame */
		goto cleanup;
	}
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	if (stream != NULL) {
		pending_stream = stream;
		pending_stream->ref ();
		GetFrameAsyncInternal (stream);
	}

cleanup:
	if (media)
		media->unref ();
}

MediaResult
IMediaDemuxer::SeekCallback (MediaClosure *closure)
{
	MediaSeekClosure *seek = (MediaSeekClosure *) closure;
	seek->GetDemuxer ()->SeekAsync ();
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::EnqueueSeek ()
{
	Media *media = GetMediaReffed ();
	MediaSeekClosure *closure;
	
	g_return_if_fail (media != NULL);
	
	closure = new MediaSeekClosure (media, SeekCallback, this, 0);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::SeekAsync ()
{
	guint64 pts = G_MAXUINT64;
	
	LOG_PIPELINE ("IMediaDemuxer::SeekAsync (), seeking: %i\n", seeking);
	
	g_return_if_fail (Media::InMediaThread ());
	
	if (seek_pending) {
		/* We're already seeking, wait until that seek has finished */
		/* ReportSeekCompleted will call EnqueueSeek if we still need to seek when the current seek has finished */
		LOG_PIPELINE ("IMediaDemuxer::SeekAsync (): already seeking, wait until the current seek has finished.\n");
		return;
	}
	
	seeking = true; /* this ensures that we stop demuxing frames asap */
	
	if (pending_stream != NULL) {
		/* we're waiting for the decoder to decode a frame, wait a bit with the seek */
		LOG_PIPELINE ("IMediaDemuxer::SeekAsync (): %i waiting for a frame, postponing seek\n", GET_OBJ_ID (this));
		EnqueueSeek ();
		return;
	}
	
	mutex.Lock ();
	if (!seeks.IsEmpty ())
		pts = ((PtsNode *) seeks.First ())->pts;
	mutex.Unlock ();
	
	if (pts == G_MAXUINT64) {
		LOG_PIPELINE ("IMediaDemuxer.:SeekAsync (): %i no pending seek?\n", GET_OBJ_ID (this));
		seeking = false;
		return;
	}

	/* Ask the demuxer to seek */
	/* at this point the pipeline shouldn't be doing anything else (for this media) */
	LOG_PIPELINE ("IMediaDemuxer::SeekAsync (): %i seeking to %" G_GUINT64_FORMAT "\n", GET_OBJ_ID (this), pts);
	Media *media = GetMediaReffed ();
	if (media) {
		media->EmitSafe (Media::SeekingEvent);
		media->unref ();
	}
	seek_pending = true;
	SeekAsyncInternal (pts);
}

void
IMediaDemuxer::SeekAsync (guint64 pts)
{
	LOG_PIPELINE ("IMediaDemuxer::SeekAsync (%" G_GUINT64_FORMAT ")\n", pts);
	// Can be called both on main and media thread

	if (IsDisposed ())
		return;
		
	mutex.Lock ();
	seeks.Append (new PtsNode (pts));
	mutex.Unlock ();

	EnqueueSeek ();
}

void
IMediaDemuxer::ClearBuffers ()
{
	pending_fill_buffers = false;
	
	/* Clear all the buffer queues */
	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaStream *stream = GetStream (i);
		
		if (stream == NULL)
			continue;
		
		stream->ClearQueue ();
	}
}

MediaResult
IMediaDemuxer::FillBuffersCallback (MediaClosure *closure)
{
	IMediaDemuxer *demuxer = (IMediaDemuxer *) closure->GetContext ();
	demuxer->FillBuffersInternal ();
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::FillBuffers ()
{
	Media *media = NULL;
	MediaClosure *closure;
	bool enqueue = true;
	
	mutex.Lock ();
	if (pending_fill_buffers) {
		// there's already a FillBuffers request enqueued
		enqueue = false;
	} else {
		media = GetMediaReffed ();
		if (media == NULL) {
			enqueue = false;
		} else {
			enqueue = true;
			pending_fill_buffers = true;
		}
	}
	mutex.Unlock ();
	
	if (enqueue) {
		closure = new MediaClosure (media, FillBuffersCallback, this, "IMediaDemuxer::FillBuffersCallback");
		media->EnqueueWork (closure);
		closure->unref ();
	}
	
	if (media != NULL)
		media->unref ();
}

void
IMediaDemuxer::FillBuffersInternal ()
{
	IMediaStream *stream;
	IMediaStream *request_stream = NULL;
	guint64 min_buffered_size = G_MAXUINT64;
	Media *media = GetMediaReffed ();
	guint64 buffering_time = 0;
	guint64 buffered_size = 0;
	guint64 last_enqueued_pts = 0;
	guint64 p_last_enqueued_pts = 6666666LL;
	guint64 target_pts;
	int ended = 0;
	int media_streams = 0;
	const char *c = NULL;
	const char *pc = NULL;
	
	LOG_PIPELINE ("IMediaDemuxer::FillBuffersInternal (), %i %s buffering time: %" G_GUINT64_FORMAT " = %" G_GUINT64_FORMAT " ms, pending_stream: %i %s\n", GET_OBJ_ID (this), GetTypeName (), buffering_time, media != NULL ? MilliSeconds_FromPts (media->GetBufferingTime ()) : -1, GET_OBJ_ID (pending_stream), pending_stream ? pending_stream->GetTypeName () : "NULL");

	mutex.Lock ();
	pending_fill_buffers = false;
	mutex.Unlock ();

	if (IsDisposed ())
		goto cleanup;

	// If we're waiting for something, there's nothing to do here.
	if (pending_stream != NULL)
		goto cleanup;

	/*
	 * Find the stream with the smallest buffered size, and request a frame from that stream.
	 * Here we define buffered size as the time between Media's target_pts and the last enqueued pts on the
	 * stream (assuming that there is at least one frame in the stream's buffer, otherwise buffered size is
	 * hard coded to 0). Note that this can give a negative buffer (if target_ps > last_enqueued_pts for a
	 * stream) - this can happen if we have audio but need a video frame. Treat this too as a buffered size
	 * of 0.
	 */
	g_return_if_fail (media != NULL);
	
	buffering_time = media->GetBufferingTime ();
	target_pts = media->GetTargetPts ();
	target_pts = target_pts == G_MAXUINT64 ? 0 : target_pts;
	
	if (buffering_time == 0) {
		// Play as soon as possible.
		// However we still need something in the buffer, at least one frame, oherwise the buffering progress
		// will stay at 0%, so up the buffering time to 1 ms. This way we'll reach 100% buffering progress when
		// all streams have 1 frame queued.
		buffering_time = 1;
	}

	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaDecoder *decoder = NULL;
		
		stream = GetStream (i);
		if (!stream->GetSelected ()) {
			LOG_PIPELINE ("IMediaDemuxer::FillBuffersInternal (): stream %i (%s) isn't selected.\n", i, stream->GetTypeName ());
			continue;
		}

		if (!stream->IsAudioOrVideo ())
			continue;

		media_streams++;
		if (stream->GetOutputEnded ()) {
			ended++;
			continue; // this stream has ended.
		}
	
		decoder = stream->GetDecoder ();
		if (decoder == NULL) {
			fprintf (stderr, "IMediaDemuxer::FillBuffersInternal () %s stream has no decoder (id: %i refcount: %i)\n", stream->GetTypeName (), GET_OBJ_ID (stream), stream->GetRefCount ());
			continue; // no decoder??
		}
	
		if (!decoder->IsDecoderQueueEmpty ())
			continue; // this stream is waiting for data to be decoded.
		
		c = NULL;
		last_enqueued_pts = stream->GetLastEnqueuedPts ();

		if (stream->GetQueueLength () == 0) {
			buffered_size = 0;
			c = "Zero length queue";
		} else if (last_enqueued_pts == G_MAXUINT64) {
			buffered_size = 0;
			c = "No last enqueued pts";
		} else if (last_enqueued_pts <= target_pts) {
			buffered_size = 0;
			c = "Last enqueued pts <= target_pts";
		} else {
			buffered_size = last_enqueued_pts - target_pts;
		}

		if (buffered_size >= buffering_time) {
			/* This stream has enough data buffered. */
			LOG_BUFFERING ("%s::FillBuffersInternal (): %s has enough data buffered (%" G_GUINT64_FORMAT " ms)\n", GetTypeName (), stream->GetTypeName (), MilliSeconds_FromPts (buffered_size));
			continue;
		}

		if (buffered_size <= min_buffered_size) {
			min_buffered_size = buffered_size;
			request_stream = stream;
			pc = c == NULL ? "buffered size smaller than min buffered size" : c;
			p_last_enqueued_pts = last_enqueued_pts;
		}
	}
	
	if (request_stream != NULL) {
		if (media->IsStopped ()) {
			if (!request_stream->IsQueueEmpty ()) {
				LOG_PIPELINE ("IMediaDemuxer::FillBuffersInternal (): stopped, and we have frames in the buffer.\n");
				goto cleanup;
			} else {
				LOG_PIPELINE ("IMediaDemuxer::FillBuffersInternal (): stopped, but the buffer is empty, continuing\n");
			}
		}
		
		LOG_BUFFERING ("%s::FillBuffersInternal (): requesting frame from %s stream, TargetPts: %" G_GUINT64_FORMAT " ms LastEnqueuedPts: %" G_GUINT64_FORMAT " ms MinBufferedSize: %" G_GUINT64_FORMAT " ms: %s\n", 
			GetTypeName (), request_stream->GetTypeName (), MilliSeconds_FromPts (target_pts), MilliSeconds_FromPts (p_last_enqueued_pts), MilliSeconds_FromPts (min_buffered_size), pc);
		GetFrameAsync (request_stream);
	}
	
	if (media_streams > 0) {
		if (ended == media_streams) {
			media->ReportBufferingProgress (1.0);
		} else {
			if (min_buffered_size > 0 && buffering_time > 0) {
				double progress = ((double) min_buffered_size / (double) buffering_time);
				media->ReportBufferingProgress (progress);
			}
		}
	}

cleanup:
	if (media)
		media->unref ();
	
	LOG_BUFFERING ("IMediaDemuxer::FillBuffersInternal () [Done]. BufferedSize: %" G_GUINT64_FORMAT " ms\n", MilliSeconds_FromPts (GetBufferedSize ()));
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

		if (!stream->IsAudioOrVideo ())
			continue;

		result = MIN (result, stream->GetBufferedSize ());
	}

	return result;
}

#if DEBUG
void
IMediaDemuxer::PrintBufferInformation ()
{
	printf ("Buffer: %" G_GINT64_FORMAT "", MilliSeconds_FromPts (GetBufferedSize ()));
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
		result = MAX (result, GetStream (i)->GetDuration ());
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
	: EventObject (Type::MEDIAFRAME, true)
{
	Initialize ();
	
	g_return_if_fail (stream != NULL);
	
	this->stream = stream;
	this->stream->ref ();
}

MediaFrame::MediaFrame (IMediaStream *stream, guint8 *buffer, guint32 buflen, guint64 pts, bool keyframe)
	: EventObject (Type::MEDIAFRAME, true)
{
	Initialize ();
	
	g_return_if_fail (stream != NULL);
	
	this->stream = stream;
	this->stream->ref ();
	this->buffer = buffer;
	this->buflen = buflen;
	this->pts = pts;
	
#if 0	
	if (buflen > 4 && false) {
		printf ("MediaFrame::MediaFrame () %s buffer: ", stream->GetTypeName ());
		for (int i = 0; i < 4; i++)
			printf (" 0x%x", buffer [i]);
		printf ("\n");
	}
#endif

	if (keyframe)
		AddState (MediaFrameKeyFrame);
}

bool
MediaFrame::AllocateBuffer (guint32 size)
{
	g_return_val_if_fail (buffer == NULL, false);
	g_return_val_if_fail (stream != NULL, false);

	buflen = size;
	buffer = (guint8 *) g_try_malloc (buflen + stream->GetMinPadding ());
	if (buffer == NULL) {
		stream->ReportErrorOccurred ("Moonlight: Could not allocate memory for next frame");
		return false;
	}

	memset (buffer + buflen, 0, stream->GetMinPadding ());

	return true;
}

bool
MediaFrame::FetchData (guint32 size, void *data)
{
	if (!AllocateBuffer (size))
		return false;

	memcpy (buffer, data, buflen);
	return true;
}

bool
MediaFrame::PrependData (guint32 size, void *data)
{
	g_return_val_if_fail (buffer != NULL, false);
	g_return_val_if_fail (stream != NULL, false);

	buffer = (guint8 *) g_realloc (buffer, buflen + stream->GetMinPadding () + size);
	if (buffer == NULL) {
		stream->ReportErrorOccurred ("Moonlight: Could not realloacte memory for current frame");
		return false;
	}

	memmove (buffer + size, buffer, buflen);
	memcpy (buffer, data, size);
	buflen += size;
	return true;
}

void
MediaFrame::Initialize ()
{
	decoder_specific_data = NULL;
	stream = NULL;
	marker = NULL;
	
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
	width = 0;
	height = 0;
	demuxer_width = 0;
	demuxer_height = 0;
}

MediaFrame::~MediaFrame ()
{
}

void
MediaFrame::Dispose ()
{
	IMediaDecoder *decoder;
	
#if SANITY
	// We can be called either on the main thread just before destruction
	// (in which case there are no races since the code which unreffed us
	// is the only code which knows about us), or at any time from the
	// media thread.
	
	if (GetRefCount () != 0 && stream != NULL) {
		if (!Media::InMediaThread ()) {
			// if refcount != 0 we're not being called just before destruction, in which case we should
			// only be on the media thread.
			printf ("MediaFrame::Dispose (): this method should only be called from the media thread.\n");
		}
	}
#endif
	
	if (decoder_specific_data != NULL && stream != NULL) {
		decoder = stream->GetDecoder ();
		if (decoder != NULL)
			decoder->Cleanup (this);
	}
	g_free (buffer);
	buffer = NULL;
	if (marker) {
		marker->unref ();
		marker = NULL;
	}
	if (stream) {
		stream->unref ();
		stream = NULL;
	}
	
	EventObject::Dispose ();
}

void
MediaFrame::SetSrcSlideY (int value)
{
	srcSlideY = value;
}

void
MediaFrame::SetSrcSlideH (int value)
{
	srcSlideH = value;
}

void
MediaFrame::SetSrcStride (int a, int b, int c, int d)
{
	srcStride [0] = a;
	srcStride [1] = b;
	srcStride [2] = c;
	srcStride [3] = d;
}

void
MediaFrame::SetDataStride (guint8* a, guint8* b, guint8* c, guint8* d)
{
	data_stride [0] = a;
	data_stride [1] = b;
	data_stride [2] = c;
	data_stride [3] = d;
}

/*
 * IMediaObject.EventData
 */

IMediaObject::EventData::EventData (int event_id, EventHandler handler, EventObject *context, bool invoke_on_main_thread)
{
	this->event_id = event_id;
	this->handler = handler;
	this->context = context;
	this->context->ref ();
	this->invoke_on_main_thread = invoke_on_main_thread;
}

IMediaObject::EventData::~EventData ()
{
	context->unref ();
	context = NULL;
}

/*
 * IMediaObject.EmitData
 */

IMediaObject::EmitData::EmitData (int event_id, EventHandler handler, EventObject *context, EventArgs *args)
{
	this->event_id = event_id;
	this->handler = handler;
	this->context = context;
	this->context->ref ();
	this->args = args;
	if (this->args)
		this->args->ref ();
}

IMediaObject::EmitData::~EmitData ()
{
	context->unref ();
	context = NULL;
	if (args) {
		args->unref ();
		args = NULL;
	}
}

/*
 * IMediaObject
 */
 
IMediaObject::IMediaObject (Type::Kind kind, Media *media)
	: EventObject (kind, true)
{
	this->media = media;
	if (this->media)
		this->media->ref ();
	g_return_if_fail (media != NULL);
	events = NULL;
	emit_on_main_thread = NULL;
}

void
IMediaObject::Dispose ()
{

#if SANITY
	// We can be called either on the main thread just before destruction
	// (in which case there are no races since the code which unreffed us
	// is the only code which knows about us), or at any time from the
	// media thread.
	if (GetRefCount () != 0 && !Media::InMediaThread ()) {
		// if refcount != 0 we're not being called just before destruction, in which case we should
		// only be on the media thread.
		LOG_PIPELINE ("IMediaObject::Dispose (): this method should only be called from the media thread.\n");
	}
#endif

	media_mutex.Lock ();
	if (media) {
		media->unref ();
		media = NULL;
	}
	media_mutex.Unlock ();
	
	event_mutex.Lock ();
	delete events;
	events = NULL;
	if (emit_on_main_thread != NULL) {
		delete emit_on_main_thread;
		emit_on_main_thread = NULL;
	}
	event_mutex.Unlock ();
	
	EventObject::Dispose ();
}

void
IMediaObject::AddSafeHandler (int event_id, EventHandler handler, EventObject *context, bool invoke_on_main_thread)
{
	LOG_PIPELINE ("IMediaObject::AddSafeHandler (%i, %p, %p, %i)\n", event_id, handler, context, invoke_on_main_thread);
	EventData *ed;
	
	if (!IsDisposed ()) {
		ed = new EventData (event_id, handler, context, invoke_on_main_thread);
		event_mutex.Lock ();
		if (events == NULL)
			events = new List ();
		events->Append (ed);
		event_mutex.Unlock ();
	}
}

void
IMediaObject::RemoveSafeHandlers (EventObject *context)
{
	EventData *ed;
	EventData *next;
	
	event_mutex.Lock ();
	if (events != NULL) {
		ed = (EventData *) events->First ();
		while (ed != NULL) {
			next = (EventData *) ed->next;
			if (ed->context == context)
				events->Remove (ed);
			ed = next;
		}
	}
	event_mutex.Unlock ();
}

void
IMediaObject::EmitSafe (int event_id, EventArgs *args)
{
	List *emits = NULL; // The events to emit on this thread.
	EventData *ed;
	EmitData *emit;
	PendingEmitList *emit_list = NULL; // The events to emit on the main thread
	bool add_tick_call = false;
	bool in_main_thread;
		
	if (events == NULL)
		goto cleanup;
		
	in_main_thread = Surface::InMainThread ();

	// Create a list of all the events to emit
	// don't keep the lock while emitting.
	event_mutex.Lock ();
	if (events != NULL) {
		ed = (EventData *) events->First ();
		while (ed != NULL) {
			if (ed->event_id == event_id) {
				emit = new EmitData (event_id, ed->handler, ed->context, args);
				if (ed->invoke_on_main_thread && !in_main_thread) {
					if (emit_list == NULL)
						emit_list = new PendingEmitList ();
					emit_list->list.Append (emit);
				} else {
					if (emits == NULL)
						emits = new List ();
					emits->Append (emit);
				}
			}
			ed = (EventData *) ed->next;
		}
	}
	if (emit_list != NULL) {
		if (emit_on_main_thread == NULL)
			emit_on_main_thread = new List ();
		emit_on_main_thread->Append (emit_list);
		add_tick_call = true;
	}
	event_mutex.Unlock ();
	
	// emit the events to be emitted on this thread
	EmitList (emits);
	delete emits;
	
	if (add_tick_call)
		AddTickCall (EmitListCallback);
	
cleanup:
	if (args)
		args->unref ();
}

void
IMediaObject::EmitListMain ()
{
	PendingEmitList *emit_list = NULL;

	VERIFY_MAIN_THREAD;
	
	event_mutex.Lock ();
	if (emit_on_main_thread != NULL) {
		emit_list = (PendingEmitList *) emit_on_main_thread->First ();
		if (emit_list != NULL)
			emit_on_main_thread->Unlink (emit_list);
	}
	event_mutex.Unlock ();

	if (emit_list != NULL) {
		EmitList (&emit_list->list);
		delete emit_list;
	}
}

void
IMediaObject::EmitListCallback (EventObject *obj)
{
	IMediaObject *media_obj = (IMediaObject *) obj;
	media_obj->EmitListMain ();
}

void
IMediaObject::EmitList (List *list)
{
	EmitData *emit;
	
	if (list == NULL)
		return;
	
	emit = (EmitData *) list->First ();
	while (emit != NULL) {
		emit->handler (this, emit->args, emit->context);
		emit = (EmitData *) emit->next;
	}
}

Media *
IMediaObject::GetMediaReffed ()
{
	Media *result;
	media_mutex.Lock ();
	result = media;
	if (result)
		result->ref ();
	media_mutex.Unlock ();
	return result;
}

void
IMediaObject::ReportErrorOccurred (char const *message)
{
	g_return_if_fail (media != NULL);
	
	media->ReportErrorOccurred (message);
}

void
IMediaObject::ReportErrorOccurred (ErrorEventArgs *args)
{
	g_return_if_fail (media != NULL);
	
	media->ReportErrorOccurred (args);
}

void
IMediaObject::SetMedia (Media *value)
{
	media_mutex.Lock ();
	if (media)
		media->unref ();
	media = value;
	if (media)
		media->ref ();
	media_mutex.Unlock ();
}

/*
 * IMediaSource
 */

IMediaSource::IMediaSource (Type::Kind kind, Media *media)
	: IMediaObject (kind, media)
{
	pthread_mutexattr_t attribs;
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&mutex, &attribs);
	pthread_mutexattr_destroy (&attribs);
}

IMediaSource::~IMediaSource ()
{
	pthread_mutex_destroy (&mutex);
}

void
IMediaSource::Dispose ()
{
	IMediaObject::Dispose ();
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
IMediaSource::ReadFD (FILE *read_fd, MediaReadClosure *closure)
{
	Media *media;
	size_t read;
	void *buffer;
	MemoryBuffer *mem;
	
	VERIFY_MEDIA_THREAD;

	LOG_PIPELINE ("IMediaSource::ReadFD (%p, %p offset: %" G_GINT64_FORMAT " count: %u)\n", read_fd, closure, closure->GetOffset (), closure->GetCount ());

	g_return_if_fail (read_fd != NULL);

	if (ftell (read_fd) != closure->GetOffset ()) {
		if (0 != fseek (read_fd, closure->GetOffset (), SEEK_SET)) {
			char *msg = g_strdup_printf ("ProgressiveSource: could not seek to %" G_GINT64_FORMAT ": %s\n", closure->GetOffset (), strerror (errno));
			ReportErrorOccurred (msg);
			g_free (msg);
			return;
		}
	}
	
	media = GetMediaReffed ();
	if (media == NULL) {
		/* We're most likely disposed */
		LOG_PIPELINE ("IMediaSource::ReadFD (): no media, disposed?\n");
		return;
	}
		
	if (closure->GetCount () > 0) {
		buffer = g_try_malloc (closure->GetCount ());
		if (buffer == NULL) {
			media->unref ();
			ReportErrorOccurred ("ProgressiveSource: could not allocate read buffer");
			return;
		}

		read = fread (buffer, 1, closure->GetCount (), read_fd);
	} else {
		buffer = NULL;
		read = 0;
	}

	mem = new MemoryBuffer (media, buffer, read, true);
	closure->SetData (mem);
	mem->unref ();
	
	media->EnqueueWork (closure);
	
	media->unref ();
}

void
IMediaSource::ReadAsync (MediaReadClosure *closure)
{
	Lock ();
	
	ReadAsyncInternal (closure);
	
	Unlock ();
}

gint64
IMediaSource::GetPositionInternal ()
{
	// This method should be overridden (or never called for the classes which doesn't override it).
	g_warning ("IMediaSource (%s)::GetPositionInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName ());
	print_stack_trace ();

	return -1;
}

void
IMediaSource::ReadAsyncInternal (MediaReadClosure *closure)
{
	g_warning ("IMediaSource (%s)::ReadAsyncInternal ():  You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName ());
	print_stack_trace ();
}

gint64
IMediaSource::GetSizeInternal ()
{
	g_warning ("IMediaSource (%s)::GetSizeInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName ());
	print_stack_trace ();
	
	return 0;
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
	
	for (int i = 0; i < count; i++)
		this->streams [i]->ref ();
}

gint32
IMediaDemuxer::AddStream (IMediaStream *stream)
{
	g_return_val_if_fail (stream != NULL, -1);
	
	stream_count++;
	streams = (IMediaStream **) g_realloc (streams, stream_count * sizeof (IMediaStream *));
	streams [stream_count - 1] = stream;
	stream->ref ();
	
	return stream_count - 1;
}

/*
 * IMediaDecoder
 */

IMediaDecoder::IMediaDecoder (Type::Kind kind, Media *media, IMediaStream *stream) : IMediaObject (kind, media)
{
	pixel_format = MoonPixelFormatNone;
	this->stream = NULL;
	
	g_return_if_fail (stream != NULL);
	
	this->stream = stream;
	this->stream->ref ();
	
	opening = false;
	opened = false;
	input_ended = false;
}

void
IMediaDecoder::Dispose ()
{
	if (stream != NULL) {
		IMediaStream *s = stream;
		stream = NULL;
		s->Dispose ();
		s->unref ();
		s = NULL;
	}

	queue.Clear (true);

	IMediaObject::Dispose ();
}

void
IMediaDecoder::ReportSeekCompleted ()
{
	queue.Clear (true);
	input_ended = false;
	CleanState ();
}

void
IMediaDecoder::ReportInputEnded ()
{
	input_ended = true;
	if (IsDecoderQueueEmpty ()) {
		InputEnded ();
	}
}

MediaResult
IMediaDecoder::ReportDecodeFrameCompletedCallback (MediaClosure *closure)
{
	MediaReportDecodeFrameCompletedClosure *c = (MediaReportDecodeFrameCompletedClosure *) closure;
	
	g_return_val_if_fail (c != NULL, MEDIA_FAIL);
	g_return_val_if_fail (c->GetDecoder () != NULL, MEDIA_FAIL);

	c->GetDecoder ()->ReportDecodeFrameCompleted (c->GetFrame ());
	
	return MEDIA_SUCCESS;
}

void
IMediaDecoder::ReportDecodeFrameCompleted (MediaFrame *frame)
{
	IMediaDemuxer *demuxer;
	IMediaStream *stream;
	Media *media = NULL;
	bool demuxer_size_failure = false;

	LOG_PIPELINE ("IMediaDecoder::ReportDecodeFrameCompleted (%p) %s %" G_GUINT64_FORMAT " ms\n", frame, frame ? frame->stream->GetTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : 0);
	
	g_return_if_fail (frame != NULL);
	
	media = GetMediaReffed ();
	g_return_if_fail (media != NULL);
	
	if (!Media::InMediaThread ()) {
		LOG_PIPELINE ("IMediaDecoder::ReportOpenDecoderCompleted (): Not in media thread, marshalling...\n");
		MediaClosure *closure = new MediaReportDecodeFrameCompletedClosure (media, ReportDecodeFrameCompletedCallback, this, frame);
		media->EnqueueWork (closure);
		closure->unref ();
		goto cleanup;
	}

	stream = frame->stream;
	if (stream == NULL)
		goto cleanup;
	
	frame->stream->EnqueueFrame (frame);

	/* Check if the size set by the demuxer is valid. DRT #7008. */
	if (frame->stream->IsVideo ()) {
		if (frame->GetDemuxerWidth () != 0) {
			if (frame->GetWidth () != 0) {
				demuxer_size_failure |= frame->GetWidth () != frame->GetDemuxerWidth ();
			} else {
				demuxer_size_failure |= ((VideoStream *) frame->stream)->GetWidth () != (guint32) frame->GetDemuxerWidth ();
			}
		}
		if (frame->GetDemuxerHeight () != 0) {
			if (frame->GetHeight () != 0) {
				demuxer_size_failure |= frame->GetHeight () != frame->GetDemuxerHeight ();
			} else {
				demuxer_size_failure |= ((VideoStream *) frame->stream)->GetHeight () != (guint32) frame->GetDemuxerHeight ();
			}
		}
	}

	if (demuxer_size_failure) {
		ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
	} else {
		demuxer = stream->GetDemuxerReffed ();
		if (demuxer != NULL) {
			demuxer->FillBuffers ();
			demuxer->unref ();
		}
	}
	
	if (input_ended && IsDecoderQueueEmpty ())
		InputEnded ();

cleanup:
	if (media)
		media->unref ();
}

MediaResult
IMediaDecoder::DecodeFrameCallback (MediaClosure *closure)
{
	
	IMediaDecoder *decoder = (IMediaDecoder *) closure->GetContext ();
	IMediaDecoder::FrameNode *node = (IMediaDecoder::FrameNode *) decoder->queue.Pop ();
	
	if (node != NULL) {
		decoder->DecodeFrameAsync (node->frame, false);
		delete node;
	}

	return MEDIA_SUCCESS;
}

void
IMediaDecoder::DecodeFrameAsync (MediaFrame *frame, bool enqueue_always)
{
	Media *media;

	LOG_PIPELINE ("IMediaDecoder::DecodeFrameAsync (%p) %s\n", frame, (frame && frame->stream) ? frame->stream->GetTypeName () : NULL);
	
	if (IsDisposed ())
		return;
	
	g_return_if_fail (frame != NULL);
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	if (enqueue_always || !Media::InMediaThread ()) {
		MediaClosure *closure = new MediaClosure (media, DecodeFrameCallback, this, "IMediaDecoder::DecodeFrameCallback");
		queue.Push (new FrameNode (frame));
		media->EnqueueWork (closure);
		closure->unref ();
		goto cleanup;
	}
	
	DecodeFrameAsyncInternal (frame);

cleanup:
	media->unref ();
}

void
IMediaDecoder::OpenDecoderAsync ()
{
	LOG_PIPELINE ("IMediaDecoder::OpenDecoderAsync ()\n");
	
	g_return_if_fail (opening == false);
	g_return_if_fail (opened == false);
	
	opening = true;	
	OpenDecoderAsyncInternal ();
}

MediaResult
IMediaDecoder::ReportOpenDecoderCompletedCallback (MediaClosure *closure)
{
	((IMediaDecoder *) closure->GetContext ())->ReportOpenDecoderCompleted ();
	return MEDIA_SUCCESS;
}

void
IMediaDecoder::ReportOpenDecoderCompleted ()
{
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("IMediaDecoder::ReportOpenDecoderCompleted ()\n");
	
	/* Media might be null if we've been disposed */
	if (media == NULL)
		return;

	if (!Media::InMediaThread ()) {
		LOG_PIPELINE ("IMediaDecoder::ReportOpenDecoderCompleted (): Not in media thread, marshalling...\n");
		MediaClosure *closure = new MediaClosure (media, ReportOpenDecoderCompletedCallback, this, "IMediaDecoder::ReportOpenDecoderCompletedCallback");
		media->EnqueueWork (closure);
		closure->unref ();
		media->unref ();
		return;
	}

	opening = false;
	opened = true;
	
	media->ReportOpenDecoderCompleted (this);
	media->unref ();
}

/*
 * IImageConverter
 */

IImageConverter::IImageConverter (Type::Kind kind, Media *media, VideoStream *stream) : IMediaObject (kind, media)
{
	output_format = MoonPixelFormatNone;
	input_format = MoonPixelFormatNone;
	this->stream = stream;
}

/*
 * VideoStream
 */

VideoStream::VideoStream (Media *media) : IMediaStream (Type::VIDEOSTREAM, media)
{
	converter = NULL;
	bits_per_sample = 0;
	initial_pts = 0;
	height = 0;
	width = 0;
	bit_rate = 0;
}

VideoStream::VideoStream (Media *media, int codec_id, guint32 width, guint32 height, guint64 duration, gpointer extra_data, guint32 extra_data_size)
	: IMediaStream (Type::VIDEOSTREAM, media)
{
	converter = NULL;
	bits_per_sample = 0;
	initial_pts = 0;
	bit_rate = 0;
	this->height = height;
	this->width = width;
	this->SetDuration (duration);
	this->SetCodecId (codec_id);
	this->SetExtraData (extra_data);
	this->SetExtraDataSize (extra_data_size);
}

VideoStream::~VideoStream ()
{
}

void
VideoStream::Dispose ()
{
	if (converter) {
		converter->Dispose ();
		converter->unref ();
		converter = NULL;
	}
	IMediaStream::Dispose ();
}

/*
 * MediaMarkerFoundClosure
 */

MediaMarkerFoundClosure::MediaMarkerFoundClosure (Media *media, MediaCallback *callback, MediaElement *context)
	: MediaClosure (Type::MEDIAMARKERFOUNDCLOSURE, media, callback, context)
{
	marker = NULL;
}

void
MediaMarkerFoundClosure::Dispose ()
{
	if (marker) {
		marker->unref ();
		marker = NULL;
	}
	MediaClosure::Dispose ();
}

void
MediaMarkerFoundClosure::SetMarker (MediaMarker *marker)
{
	if (this->marker)
		this->marker->unref ();
	this->marker = marker;
	if (this->marker)
		this->marker->ref ();
}

/*
 * MediaMarker
 */ 

MediaMarker::MediaMarker (const char *type, const char *text, guint64 pts)
	: EventObject (Type::MEDIAMARKER)
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
 
MarkerStream::MarkerStream (Media *media) : IMediaStream (Type::MARKERSTREAM, media)
{
	closure = NULL;
}

void
MarkerStream::Dispose ()
{
	if (closure) {
		closure->unref ();
		closure = NULL;
	}
	
	IMediaStream::Dispose ();
}

void
MarkerStream::MarkerFound (MediaFrame *frame)
{
	LOG_PIPELINE ("MarkerStream::MarkerFound ().\n");
	
	if (GetDecoder () == NULL) {
		LOG_PIPELINE ("MarkerStream::MarkerFound (): Got marker, but there's no decoder for the marker.\n");
		return;
	}
	
	GetDecoder ()->DecodeFrameAsync (frame, false);
}

void
MarkerStream::FrameEnqueued ()
{
	MediaFrame *frame;
	
	LOG_PIPELINE ("MarkerStream::FrameEnqueued ().\n");
	
	frame = PopFrame ();
	
	if (frame == NULL) {
		LOG_PIPELINE ("MarkerStream::FrameEnqueued (): No frame.\n");
		return;
	}
	
	if (closure != NULL) {
		closure->SetMarker (frame->marker);
		closure->Call ();
		closure->SetMarker (NULL);
	} else {
		LOG_PIPELINE ("MarkerStream::FrameEnqueued (): No callback.\n");
		mutex.Lock ();
		list.Append (new MediaMarker::Node (frame->marker));
		mutex.Unlock ();
	}
	
	frame->unref ();
}

MediaMarker *
MarkerStream::Pop ()
{
	MediaMarker *result = NULL;
	MediaMarker::Node *node;
	
	mutex.Lock ();
	node = (MediaMarker::Node *) list.First ();
	if (node != NULL) {
		result = node->marker;
		result->ref ();
		list.Remove (node);
	}
	mutex.Unlock ();
	
	return result;
}

void
MarkerStream::SetCallback (MediaMarkerFoundClosure *closure)
{
	if (this->closure)
		this->closure->unref ();
	this->closure = closure;
	if (this->closure)
		this->closure->ref ();
}

/*
 * MediaWork
 */ 
MediaWork::MediaWork (MediaClosure *c)
{
	g_return_if_fail (c != NULL);
	
	closure = c;
	closure->ref ();
}

MediaWork::~MediaWork ()
{
	g_return_if_fail (closure != NULL);
	
	closure->unref ();
	closure = NULL;
}

/*
 * PassThroughDecoderInfo
 */

bool
PassThroughDecoderInfo::Supports (const char *codec)
{
	const char *video_fourccs [] = { "yv12", "rgba", NULL };
	const char *audio_fourccs [] = { "pcm", NULL };
	
	for (int i = 0; video_fourccs [i] != NULL; i++)
		if (!strcmp (codec, video_fourccs [i]))
			return true;

	for (int i = 0; audio_fourccs [i] != NULL; i++)
		if (!strcmp (codec, audio_fourccs [i]))
			return true;

	return false;
}

/*
 * PassThroughDecoder
 */

PassThroughDecoder::PassThroughDecoder (Media *media, IMediaStream *stream)
	: IMediaDecoder (Type::PASSTHROUGHDECODER, media, stream)
{
}

void
PassThroughDecoder::Dispose ()
{
	IMediaDecoder::Dispose ();
}

void
PassThroughDecoder::OpenDecoderAsyncInternal ()
{
	const char *fourcc = GetStream ()->GetCodec ();
	
	if (!strcmp (fourcc, "yv12")) {
		SetPixelFormat (MoonPixelFormatYUV420P);
	} else if (!strcmp (fourcc, "rgba")) {
		SetPixelFormat (MoonPixelFormatRGBA32);
	} else if (!strcmp (fourcc, "pcm")) {
		// nothing to do here
	} else {
		ReportErrorOccurred (g_strdup_printf ("Unknown fourcc: %s", fourcc));
		return;
	}
	
	ReportOpenDecoderCompleted ();
}

void
PassThroughDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	frame->AddState (MediaFrameDecoded);
	if (GetPixelFormat () == MoonPixelFormatYUV420P) {
		VideoStream *vs = (VideoStream *) GetStream ();

		frame->width = vs->GetWidth ();
		frame->height = vs->GetHeight ();

		frame->data_stride [0] = frame->GetBuffer ();
		frame->data_stride [1] = frame->GetBuffer () + (frame->width*frame->height);
		frame->data_stride [2] = frame->GetBuffer () + (frame->width*frame->height)+(frame->width/2*frame->height/2);
		frame->SetBuffer (NULL);
		frame->srcStride[0] = frame->width;
		frame->srcSlideY = frame->width;
		frame->srcSlideH = frame->height;

		frame->AddState (MediaFramePlanar);
	}
	ReportDecodeFrameCompleted (frame);
}

void
PassThroughDecoder::InputEnded ()
{
	GetStream ()->SetOutputEnded (true);
}

/*
 * NullDecoderInfo
 */

bool
NullDecoderInfo::Supports (const char *codec)
{
	const char *video_fourccs [] = { "wmv1", "wmv2", "wmv3", "wmva", "vc1", NULL };
	const char *audio_fourccs [] = { "wmav1","wmav2", "wmav3", "mp3", NULL};
	
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

NullDecoder::NullDecoder (Media *media, IMediaStream *stream) : IMediaDecoder (Type::NULLDECODER, media, stream)
{
	logo = NULL;
	logo_size = 0;
	prev_pts = G_MAXUINT64;
}

void
NullDecoder::Dispose ()
{
	g_free (logo);
	logo = NULL;
	
	IMediaDecoder::Dispose ();
}

MediaResult
NullDecoder::DecodeVideoFrame (MediaFrame *frame)
{
	// free encoded buffer and alloc a new one for our image
	g_free (frame->GetBuffer ());
	frame->SetBuffer (NULL);
	if (!frame->FetchData (logo_size, logo)) {
		return MEDIA_FAIL;
	}
	frame->AddState (MediaFrameDecoded);
	
	//printf ("NullVideoDecoder::DecodeFrame () pts: %" G_GUINT64_FORMAT ", w: %i, h: %i\n", frame->pts, w, h);
	
	return MEDIA_SUCCESS;
}

MediaResult
NullDecoder::DecodeAudioFrame (MediaFrame *frame)
{
	AudioStream *as = (AudioStream *) GetStream ();
	guint32 samples;
	guint32 data_size;
	guint64 diff_pts;
	
	// discard encoded data
	g_free (frame->GetBuffer ());

	// We have no idea here how long the encoded audio data is
	// for the first frame we use 0.1 seconds, for the rest
	// we calculate the time since the last frame

	if (prev_pts == G_MAXUINT64 || frame->pts <= prev_pts) {
		samples = as->GetSampleRate () / 10; // start off sending 0.1 seconds of audio
	} else {
		diff_pts = frame->pts - prev_pts;
		samples = (float) as->GetSampleRate () / (TIMESPANTICKS_IN_SECOND_FLOAT / (float) diff_pts);
	}
	prev_pts = frame->pts;

	data_size  = samples * as->GetChannels () * 2 /* 16 bit audio */;

	frame->SetBufLen (data_size);
	frame->SetBuffer ((guint8 *) g_malloc0 (data_size));
	
	frame->AddState (MediaFrameDecoded);
	
	return MEDIA_SUCCESS;
}

void
NullDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	MediaResult result = MEDIA_FAIL;
	IMediaStream *stream = GetStream ();
	
	if (stream->IsAudio ()) {
		result = DecodeAudioFrame (frame);
	} else if (stream->IsVideo ()) {
		result = DecodeVideoFrame (frame);
	}
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportDecodeFrameCompleted (frame);
	} else {
		ReportErrorOccurred ("Unspecified error while NullDecoder was decoding a frame");
	}
}

void
NullDecoder::OpenDecoderAsyncInternal ()
{
	MediaResult result;
	IMediaStream *stream = GetStream ();
	
	if (stream->IsAudio ()) {
		result = OpenAudio ();
	} else if (stream->IsVideo ()) {
		result = OpenVideo ();
	} else {
		result = MEDIA_FAIL;
	}
		
	if (MEDIA_SUCCEEDED (result)) {
		ReportOpenDecoderCompleted ();
	} else {
		ReportErrorOccurred ("Unspecified error while opening null decoder");
	}
}

MediaResult
NullDecoder::OpenAudio ()
{
	return MEDIA_SUCCESS;
}

MediaResult
NullDecoder::OpenVideo ()
{
	VideoStream *vs = (VideoStream *) GetStream ();
	guint32 dest_height = vs->GetHeight ();
	guint32 dest_width = vs->GetWidth ();
	guint32 dest_i = 0;
	
	// We assume that the input image is a 24 bit bitmap (bmp), stored bottum up and flipped vertically.
	const char *image = moonlight_logo;

	guint32 img_offset = *((guint32*)(image + 10));
	guint32 img_width  = *((guint32*)(image + 18));
	guint32 img_height = *((guint32*)(image + 22));
	guint32 img_stride = (img_width * 3 + 3) & ~3; // in bytes
	guint32 img_i, img_h, img_w;
	guint32 start_w = (dest_width-img_width)/2;
	guint32 end_w = start_w + img_width;	
	guint32 start_h = (dest_height-img_height)/2;
	guint32 end_h = start_h + img_height;	
	
	LOG_PIPELINE ("offset: %i, width: 0x%x = %i, height: 0x%x = %i, stride: %i\n", img_offset, img_width, img_width, img_height, img_height, img_stride);
	
	// create the buffer for our image
	logo_size = dest_height * dest_width * 4;
	logo = (guint8*) g_malloc (logo_size);
	memset (logo, 0x00, logo_size);

	// write our image centered into the destination rectangle, flipped horizontally
	dest_i = 4;
	for (guint32 dest_h = 0; dest_h < dest_height; dest_h++) {
		for (guint32 dest_w = 0; dest_w < dest_width; dest_w++) {
			if (dest_w >= start_w && dest_w < end_w && dest_h >= start_h && dest_h < end_h) {
				img_h = (dest_h - start_h) % img_height;
				img_w = (dest_w - start_w) % img_width;
				img_i = img_h * img_stride + img_w * 3;

				logo [logo_size - dest_i + 0] = image [img_offset + img_i + 0];
				logo [logo_size - dest_i + 1] = image [img_offset + img_i + 1];
				logo [logo_size - dest_i + 2] = image [img_offset + img_i + 2];
			}
			logo [logo_size - dest_i + 3] = 0xff;

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

	SetPixelFormat (MoonPixelFormatRGB32);
	
	return MEDIA_SUCCESS;
}

/*
 * ExternalDemuxer
 */

ExternalDemuxer::ExternalDemuxer (Media *media, void *instance, CloseDemuxerCallback close_demuxer, 
		GetDiagnosticAsyncCallback get_diagnostic, GetFrameAsyncCallback get_sample, OpenDemuxerAsyncCallback open_demuxer, 
		SeekAsyncCallback seek, SwitchMediaStreamAsyncCallback switch_media_stream)
	: IMediaDemuxer (Type::EXTERNALDEMUXER, media)
{
	this->close_demuxer_callback = close_demuxer;
	this->get_diagnostic_async_callback = get_diagnostic;
	this->get_sample_async_callback = get_sample;
	this->open_demuxer_async_callback = open_demuxer;
	this->seek_async_callback = seek;
	this->switch_media_stream_async_callback = switch_media_stream;
	this->instance = instance;
	
	can_seek = true;
	pthread_rwlock_init (&rwlock, NULL);
	
	g_return_if_fail (instance != NULL);
	g_return_if_fail (close_demuxer != NULL && get_diagnostic != NULL && get_sample != NULL && open_demuxer != NULL && seek != NULL && switch_media_stream != NULL);
}

ExternalDemuxer::~ExternalDemuxer ()
{
	pthread_rwlock_destroy (&rwlock);
}

void
ExternalDemuxer::Dispose ()
{	
	ClearCallbacks ();
	IMediaDemuxer::Dispose ();
}

void
ExternalDemuxer::ClearCallbacks ()
{
	pthread_rwlock_wrlock (&rwlock);
	close_demuxer_callback = NULL;
	get_diagnostic_async_callback = NULL;
	get_sample_async_callback = NULL;
	open_demuxer_async_callback = NULL;
	seek_async_callback = NULL;
	switch_media_stream_async_callback = NULL;
	instance = NULL;
	pthread_rwlock_unlock (&rwlock);
}

void
ExternalDemuxer::SetCanSeek (bool value)
{
	can_seek = value;
}

gint32
ExternalDemuxer::AddStream (IMediaStream *stream)
{
	return IMediaDemuxer::AddStream (stream);
}

void 
ExternalDemuxer::CloseDemuxerInternal ()
{
	pthread_rwlock_rdlock (&rwlock);
	if (close_demuxer_callback != NULL) {
		close_demuxer_callback (instance);
	} else {
#if SANITY
		printf ("ExternalDemuxer::CloseDemuxerInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}

void 
ExternalDemuxer::GetDiagnosticAsyncInternal (MediaStreamSourceDiagnosticKind diagnosticsKind)
{
	pthread_rwlock_rdlock (&rwlock);
	if (get_diagnostic_async_callback != NULL) {	
		get_diagnostic_async_callback (instance, diagnosticsKind);
	} else {
#if SANITY
		printf ("ExternalDemuxer::GetDiagnosticsAsyncInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}

void 
ExternalDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	g_return_if_fail (stream != NULL);
	
	pthread_rwlock_rdlock (&rwlock);
	if (get_sample_async_callback != NULL) {
		get_sample_async_callback (instance, stream->GetStreamType ());
	} else {
#if SANITY
		printf ("ExternalDemuxer::GetFrameAsyncInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}

void 
ExternalDemuxer::OpenDemuxerAsyncInternal ()
{
	pthread_rwlock_rdlock (&rwlock);
	if (open_demuxer_async_callback != NULL) {	
		open_demuxer_async_callback (instance, this);
	} else {
#if SANITY
		printf ("ExternalDemuxer::OpenDemuxerAsyncInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}

void 
ExternalDemuxer::SeekAsyncInternal (guint64 seekToTime)
{
	pthread_rwlock_rdlock (&rwlock);
	if (seek_async_callback != NULL) {
		seek_async_callback (instance, seekToTime);
	} else {
#if SANITY
		printf ("ExternalDemuxer::SeekAsyncInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}

void 
ExternalDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *mediaStreamDescription)
{
	g_return_if_fail (mediaStreamDescription != NULL);
	
	pthread_rwlock_rdlock (&rwlock);
	if (switch_media_stream_async_callback != NULL) {
		switch_media_stream_async_callback (instance, mediaStreamDescription);
	} else {
#if SANITY
		printf ("ExternalDemuxer::SwitchMediaStreamAsyncInternal (): no function pointer.\n");
#endif
	}
	pthread_rwlock_unlock (&rwlock);
}
	
	
/*
 * AudioStream
 */

AudioStream::AudioStream (Media *media)
	: IMediaStream (Type::AUDIOSTREAM, media)
{
}

AudioStream::AudioStream (Media *media, int codec_id, int bits_per_sample, int block_align, int sample_rate, int channels, int bit_rate, gpointer extra_data, guint32 extra_data_size)
	: IMediaStream (Type::AUDIOSTREAM, media)
{
	this->SetCodecId (codec_id);
	this->SetExtraData (extra_data);
	this->SetExtraDataSize (extra_data_size);
	input_bits_per_sample = bits_per_sample;
	output_bits_per_sample = bits_per_sample;
	input_block_align = block_align;
	output_block_align = block_align;
	input_sample_rate = sample_rate;
	output_sample_rate = sample_rate;
	input_channels = channels;
	output_channels = channels;
	input_bit_rate = bit_rate;
	output_bit_rate = bit_rate;
}

/*
 * ExternalDecoder
 */

ExternalDecoder::ExternalDecoder (Media *media, IMediaStream *stream, void *instance, const char *name,
		ExternalDecoder_DecodeFrameAsyncCallback decode_frame_async,
		ExternalDecoder_OpenDecoderAsyncCallback open_decoder_async,
		ExternalDecoder_CleanupCallback cleanup,
		ExternalDecoder_CleanStateCallback clean_state,
		ExternalDecoder_HasDelayedFrameCallback has_delayed_frame,
		ExternalDecoder_DisposeCallback dispose,
		ExternalDecoder_DtorCallback dtor,
		ExternalDecoder_InputEndedCallback input_ended)
	: IMediaDecoder (Type::EXTERNALDECODER, media, stream)
{
	this->instance = instance;
	this->name = g_strdup (name);
	this->decode_frame_async = decode_frame_async;
	this->open_decoder_async = open_decoder_async;
	this->cleanup = cleanup;
	this->clean_state = clean_state;
	this->has_delayed_frame = has_delayed_frame;
	this->dispose = dispose;
	this->dtor = dtor;
	this->input_ended = input_ended;
}
	
ExternalDecoder::~ExternalDecoder ()
{
	dtor (instance);
	g_free (name);
}
	
void
ExternalDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	decode_frame_async (instance, frame);
}

void
ExternalDecoder::OpenDecoderAsyncInternal ()
{
	open_decoder_async (instance);
}
	
void
ExternalDecoder::Dispose ()
{
	dispose (instance);
	
	IMediaDecoder::Dispose ();
}

void
ExternalDecoder::Cleanup (MediaFrame *frame)
{
	cleanup (instance, frame);
}

void
ExternalDecoder::CleanState ()
{
	clean_state (instance);
}

bool
ExternalDecoder::HasDelayedFrame ()
{
	return has_delayed_frame (instance);
}

void
ExternalDecoder::InputEnded ()
{
	input_ended (instance);
}

/*
 * ExternalDecoderInfo
 */

ExternalDecoderInfo::ExternalDecoderInfo (void *instance, const char *name, ExternalDecoderInfo_SupportsCallback supports, ExternalDecoderInfo_Create create, ExternalDecoderInfo_dtor dtor)
{
	this->instance = instance;
	this->supports = supports;
	this->create = create;
	this->dtor = dtor;
	this->name = g_strdup (name);
}

bool 
ExternalDecoderInfo::Supports (const char *codec)
{
	return supports (instance, codec);
}

IMediaDecoder *
ExternalDecoderInfo::Create (Media *media, IMediaStream *stream)
{
	return create (instance, media, stream);
}

ExternalDecoderInfo::~ExternalDecoderInfo ()
{
	if (dtor != NULL)
		dtor (instance);
	g_free (name);
}

/*
 * MediaReadClosure
 */

MediaReadClosure::MediaReadClosure (Media *media, MediaCallback *callback, EventObject *context, gint64 offset, guint32 count)
	: MediaClosure (Type::MEDIAREADCLOSURE, media, callback, context)
{
	this->data = NULL;
	this->offset = offset;
	this->count = count;
	this->cancelled = false;
}

void
MediaReadClosure::Dispose ()
{
	if (this->data) {
		this->data->unref ();
		this->data = NULL;
	}
	MediaClosure::Dispose ();
}

void
MediaReadClosure::SetData (MemoryBuffer *data)
{
	if (this->data)
		this->data->unref ();
	this->data = data;
	if (this->data)
		this->data->ref ();
}



};
