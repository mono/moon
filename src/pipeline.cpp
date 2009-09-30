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
#include <fcntl.h>
#include <errno.h>

#include <dlfcn.h>

#include "audio.h"
#include "pipeline.h"
#include "codec-version.h"
#include "pipeline-ffmpeg.h"
#include "mp3.h"
#include "uri.h"
#include "media.h"
#include "mediaelement.h"
#include "asf/asf.h"
#include "asf/asf-structures.h"
#include "yuv-converter.h"
#include "runtime.h"
#include "mms-downloader.h"
#include "pipeline-ui.h"
#include "pipeline-asf.h"
#include "playlist.h"
#include "deployment.h"
#include "timesource.h"

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
	source = NULL;
	demuxer = NULL;
	markers = NULL;

	is_disposed = false;
	initialized = false;	
	opened = false;
	opening = false;
	stopping = false;
	error_reported = false;
	buffering_enabled = false;
	in_open_internal = false;
	http_retried = false;
	download_progress = 0.0;
	buffering_progress = 0.0;
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
	
	LOG_PIPELINE ("Media::Dispose (), id: %i\n", GET_OBJ_ID (this));

	mutex.Lock ();
	is_disposed = true;
	mutex.Unlock ();
	
	ClearQueue ();
		
	g_free (file);
	file = NULL;
	g_free (uri);
	uri = NULL;

	src = this->source;
	this->source = NULL;
	if (src) {
		src->Dispose ();
		src->unref ();
	}

	dmx = this->demuxer;
	this->demuxer = NULL;	
	if (dmx) {
		dmx->Dispose ();
		dmx->unref ();
	}
	
	delete markers;
	markers = NULL;
	
	IMediaObject::Dispose ();
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
	char *libmscodecs_path = NULL;
	const char *functions [] = {"register_codec_pack", NULL};
	const gchar *home = g_get_home_dir ();
	registering_ms_codecs = true;

	if (!(moonlight_flags & RUNTIME_INIT_ENABLE_MS_CODECS)) {
		LOG_CODECS ("Moonlight: mscodecs haven't been enabled.\n");
		return;
	}

	if (home != NULL)
		libmscodecs_path = g_build_filename (g_get_home_dir (), ".mozilla", "plugins", "moonlight", CODEC_LIBRARY_NAME, NULL);

	if (!(g_file_test (libmscodecs_path, G_FILE_TEST_EXISTS) && g_file_test (libmscodecs_path, G_FILE_TEST_IS_REGULAR))) {
		if (libmscodecs_path)
			g_free (libmscodecs_path);
		libmscodecs_path = g_strdup (CODEC_LIBRARY_NAME);
	}

	dl = dlopen (libmscodecs_path, RTLD_LAZY);
	if (dl != NULL) {
		LOG_CODECS ("Moonlight: Loaded mscodecs from: %s.\n", libmscodecs_path);
			
		for (int i = 0; functions [i] != NULL; i++) {
			reg = (register_codec) dlsym (dl, functions [i]);
			if (reg != NULL) {
				(*reg) (MOONLIGHT_CODEC_ABI_VERSION);
			} else {
				LOG_CODECS ("Moonlight: Cannot find %s in %s.\n", functions [i], libmscodecs_path);
			}
		}		
		registered_ms_codecs = true;
	} else {
		LOG_CODECS ("Moonlight: Cannot load %s: %s\n", libmscodecs_path, dlerror ());
	}
	g_free (libmscodecs_path);

	registering_ms_codecs = false;
}

void
Media::SetBufferingEnabled (bool value)
{
	buffering_enabled = value;
	WakeUp ();
}

void
Media::SetBufferingTime (guint64 buffering_time)
{
	mutex.Lock ();
	this->buffering_time = buffering_time;
	mutex.Unlock ();
	
	if (demuxer != NULL)
		demuxer->FillBuffers ();
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
	LOG_CODECS ("Moonlight: Codec has been registered: %s\n", info->GetName ());
}

void
Media::Initialize ()
{
	LOG_PIPELINE ("Media::Initialize ()\n");
	
	// demuxers
	Media::RegisterDemuxer (new ASFDemuxerInfo ());
	Media::RegisterDemuxer (new Mp3DemuxerInfo ());
	Media::RegisterDemuxer (new ASXDemuxerInfo ());

	// converters
	if (!(moonlight_flags & RUNTIME_INIT_FFMPEG_YUV_CONVERTER))
		Media::RegisterConverter (new YUVConverterInfo ());

	// decoders
	Media::RegisterDecoder (new ASFMarkerDecoderInfo ());
	if (moonlight_flags & RUNTIME_INIT_ENABLE_MS_CODECS) {
		RegisterMSCodecs ();
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

bool
Media::InMediaThread ()
{
	return MediaThreadPool::IsThreadPoolThread ();
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
		EmitSafe (BufferingProgressChangedEvent, new ProgressEventArgs (progress));
	}
}

void
Media::ReportDownloadProgress (double progress)
{
	LOG_PIPELINE ("Media::ReportDownloadProgress (%.3f), download_progress: %.3f\n", progress, download_progress);

	progress = MAX (MIN (progress, 1.0), 0.0);
	
	// download progress can only go up
	g_return_if_fail (progress >= download_progress);

	if (progress == download_progress)
		return;	

	if (progress > (download_progress + 0.005) || progress == 1.0 || progress == 0.0) {
		download_progress = progress;
		EmitSafe (DownloadProgressChangedEvent, new ProgressEventArgs (progress));
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
	LOG_PIPELINE ("Media::ReportSeekCompleted (%llu), id: %i\n", pts, GET_OBJ_ID (this));
	
	buffering_progress = 0;
	ClearQueue ();
	EmitSafe (SeekCompletedEvent);
}

void
Media::ReportOpenCompleted ()
{
	LOG_PIPELINE ("Media::ReportOpenCompleted (), id: %i\n", GET_OBJ_ID (this));
	
	EmitSafe (OpenCompletedEvent);
}

void
Media::ReportOpenDemuxerCompleted ()
{
	LOG_PIPELINE ("Media::ReportOpenDemuxerCompleted (), id: %i\n", GET_OBJ_ID (this));
	
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
		EmitSafe (MediaErrorEvent, args);
	}
}

void
Media::ReportErrorOccurred (const char *message)
{
	LOG_PIPELINE ("Media::ReportErrorOccurred (%s)\n", message);
	
	ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 3001, message)));
}

void
Media::ReportErrorOccurred (MediaResult result)
{
	char *msg = g_strdup_printf ("Media error: %i.", result);
	ReportErrorOccurred (msg);
	g_free (msg);
}

void
Media::PlayAsync ()
{
	LOG_PIPELINE ("Media::PlayAsync ()\n");
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
		InternalDownloader *idl = downloader->GetInternalDownloader ();
		MmsDownloader *mms_dl = (idl && idl->GetObjectType () == Type::MMSDOWNLOADER) ? (MmsDownloader *) idl : NULL;
		
		if (mms_dl == NULL) {
			ReportErrorOccurred ("We don't support using downloaders which haven't started yet.");
			return;
		}
		
		source = new MmsSource (this, downloader);
	} else {
		source = new FileSource (this, file);
	}
	
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
		ReportErrorOccurred (result);
		return;
	}
	
	initialized = true;
	this->source = source;
	this->source->ref ();
}

void
Media::Initialize (const char *uri)
{
	Downloader *dl;
	IMediaSource *source = NULL;
	
	LOG_PIPELINE ("Media::Initialize ('%s'), id: %i\n", uri, GET_OBJ_ID (this));	
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (file == NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (initialized == false);
	g_return_if_fail (error_reported == false);
	g_return_if_fail (source == NULL);
	g_return_if_fail (this->source == NULL);
	
	this->uri = g_strdup (uri);
	
	
	if (g_str_has_prefix (uri, "mms://") || g_str_has_prefix (uri, "rtsp://")  || g_str_has_prefix (uri, "rtsps://")) {
		dl = Surface::CreateDownloader (this);
		if (dl == NULL) {
			ReportErrorOccurred ("Couldn't create downloader.");
			return;
		}
		
		dl->Open ("GET", uri, StreamingPolicy);
		
		if (dl->GetFailedMessage () == NULL) {
			Initialize (dl, NULL);
		} else {
			ReportErrorOccurred (new ErrorEventArgs (MediaError,
								 MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR")));
		}
			
		dl->unref ();
		
		return;
	}
	
	source = new ProgressiveSource (this, uri);
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
Media::RetryHttp (ErrorEventArgs *args)
{
	char *http_uri = NULL;
	
	LOG_PIPELINE ("Media::RetryHttp (), current uri: '%s'\n", uri);
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (source != NULL);
	
	if (http_retried) {
		ReportErrorOccurred (args);
		return;
	}
	
	// CHECK: If the current protocolo is rtsps, should we retry http or https?
	
	if (g_str_has_prefix (uri, "mms://")) {
		http_uri = g_strdup_printf ("http://%s", uri + 6);
	} else if (g_str_has_prefix (uri, "rtsp://")) {
		http_uri = g_strdup_printf ("http://%s", uri + 7);
	} else if (g_str_has_prefix (uri, "rtsps://")) {
		http_uri = g_strdup_printf ("http://%s", uri + 8);
	} else {
		ReportErrorOccurred (args);
		return;
	}
	
	http_retried = true;
	
	LOG_PIPELINE ("Media::RetryHttp (), new uri: '%s'\n", http_uri);
	
	g_free (uri);
	uri = NULL;
	source->Dispose ();
	source->unref ();
	source = NULL;
	initialized = false;
	
	Initialize (http_uri);
	
	g_free (http_uri);
	
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
		
	demuxer->FillBuffers ();
		
	opened = true;
	opening = false;
	
	LOG_PIPELINE ("Media::OpenInteral (): opened successfully.\n");
	
	EmitSafe (OpenCompletedEvent);

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

bool
Media::SelectDemuxerAsync ()
{
	DemuxerInfo *demuxerInfo;
	MediaResult support;
	MediaResult result;
	bool eof;
	
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
	demuxer = source->CreateDemuxer (this);

	if (demuxer == NULL) { // No demuxer created, we need to find it ourselves.
		// Check if we have at least 1024 bytes or eof
		if (!source->IsPositionAvailable (16, &eof)) {
			if (!eof) {
				// We need to try again later.
				LOG_PIPELINE ("Media::SelectDemuxer (): We don't have enough data yet.\n");
				
				MediaClosure *closure = new MediaClosure (this, OpenInternal, this, "Media::OpenInternal");
				EnqueueWork (closure, false);
				closure->unref ();

				return false;
			}
		}

		// Select a demuxer
		demuxerInfo = registered_demuxers;
		while (demuxer == NULL && demuxerInfo != NULL) {
			LOG_PIPELINE ("Media::SelectDemuxer ): Checking if '%s' can handle the media.\n", demuxerInfo->GetName ());
			support = demuxerInfo->Supports (source);
			
			if (support == MEDIA_SUCCESS)
				break;
			
			result = support;
	
			if (result == MEDIA_NOT_ENOUGH_DATA) {
				LOG_PIPELINE ("Media::SelectDemuxer (): '%s' can't determine whether it can handle the media or not due to not enough data being available yet.\n", demuxerInfo->GetName ());
				
				MediaClosure *closure = new MediaClosure (this, OpenInternal, this, "Media::OpenInternal");
				EnqueueWork (closure, false);
				closure->unref ();
				
				return false;
			}
			
			LOG_PIPELINE ("Media::SelectDemuxer (): '%s' can't handle this media.\n", demuxerInfo->GetName ());
			demuxerInfo = (DemuxerInfo *) demuxerInfo->next;
		}
		
		if (demuxerInfo == NULL) {
			// No demuxer found, report an error
			const char *source_name = file ? file : uri;
		
			if (!source_name) {
				switch (source->GetType ()) {
				case MediaSourceTypeProgressive:
				case MediaSourceTypeFile:
					source_name = ((FileSource *) source)->GetFileName ();
					break;
				case MediaSourceTypeMms:
				case MediaSourceTypeMmsEntry:
					source_name = "live source";
					break;
				default:
					source_name = "unknown source";
					break;
				}
			}
			char *msg = g_strdup_printf ("No demuxers registered to handle the media source '%s'.", source_name);
			ReportErrorOccurred (new ErrorEventArgs (MediaError,
								 MoonError (MoonError::EXCEPTION, 3001, "AG_E_INVALID_FILE_FORMAT"),
								 MEDIA_UNKNOWN_CODEC, msg));
			g_free (msg);
			return false;
		}
		
		// Found a demuxer
		demuxer = demuxerInfo->Create (this, source);
	} else {
		LOG_PIPELINE ("Media::SelectDemuxer (): The source created the demuxer (%s).\n", demuxer->GetTypeName ());
	}
	
	if (demuxer->IsOpened ())
		return true;
	
	if (demuxer->IsOpening ())
		return false;
	
	LOG_PIPELINE ("Media::SelectDemuxer (), id: %i opening demuxer %i (%s)\n", GET_OBJ_ID (this), GET_OBJ_ID (demuxer), demuxer->GetTypeName ());
	
	demuxer->OpenDemuxerAsync ();
	
	LOG_PIPELINE ("Media::SelectDemuxer (), id: %i opening demuxer %i (%s) [Done]\n", GET_OBJ_ID (this), GET_OBJ_ID (demuxer), demuxer->GetTypeName ());
	
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
			Media::Warning (MEDIA_UNKNOWN_CODEC, "Unknown codec: '%s'.", codec);	
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
			ReportErrorOccurred (MEDIA_FAIL);
			return false;
		}
		
		if (decoder->IsOpening ()) {
			MediaClosure *closure = new MediaClosure (this, OpenInternal, this, "Media::OpenInternal");
			EnqueueWork (closure, false);
			closure->unref ();
			return false;
		}
		
		if (!decoder->IsOpened ()) {
			// After calling OpenDecoderAsync on a decoder, the decoder should either be opened, opening, or an error should have occurred.
			ReportErrorOccurred (MEDIA_FAIL);
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
			ReportErrorOccurred (MEDIA_FAIL);
			return false;
		}

		if (stream->GetType () != MediaTypeVideo)
			continue; // Only video streams need converters
			
		if (decoder->GetPixelFormat () == MoonPixelFormatRGB32 || decoder->GetPixelFormat () == MoonPixelFormatRGBA32)
			continue; // We need RGB32, so any stream already producing RGB32 doesn't need a converter.
			
		// Select converter for this stream
		VideoStream *vs = (VideoStream *) stream;
		
		ConverterInfo* current_conv = registered_converters;
		while (current_conv != NULL && !current_conv->Supports (decoder->GetPixelFormat (), MoonPixelFormatRGB32)) {
			LOG_PIPELINE ("Checking whether '%s' supports input '%d' and output '%d': no.\n",
				current_conv->GetName (), decoder->GetPixelFormat (), MoonPixelFormatRGB32);
			current_conv = (ConverterInfo*) current_conv->next;

		}
		
		if (current_conv == NULL) {
			ReportErrorOccurred (MEDIA_UNKNOWN_CONVERTER);
			//Media::Warning (MEDIA_UNKNOWN_CONVERTER, "Can't convert from %d to %d: No converter found.",
			//		decoder->GetPixelFormat (), MoonPixelFormatRGB32);
			return false;
		}	
		
		LOG_PIPELINE ("Checking whether '%s' supports input '%d' and output '%d': yes.\n",
			current_conv->GetName (), decoder->GetPixelFormat (), MoonPixelFormatRGB32);
		
		vs->converter = current_conv->Create (this, vs);
		vs->converter->input_format = decoder->GetPixelFormat ();
		vs->converter->output_format = MoonPixelFormatRGB32;
		if (!MEDIA_SUCCEEDED (vs->converter->Open ())) {
			vs->converter->unref ();
			vs->converter = NULL;
			ReportErrorOccurred (MEDIA_FAIL);
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
Media::EnqueueWork (MediaClosure *closure, bool wakeup)
{
	bool result = false;
	bool disposed;
	
	LOG_PIPELINE_EX ("Media::EnqueueWork (%p).\n", closure);

	g_return_val_if_fail (closure != NULL, false);
	g_return_val_if_fail (!IsDisposed (), false);
	
	mutex.Lock ();
	disposed = this->is_disposed;
	if (disposed) {
		result = false;
		LOG_PIPELINE ("Media::EnqueueWork (): disposed: %i, work not added\n", disposed);
	} else {
		MediaThreadPool::AddWork (closure, wakeup);
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
	if (!EnqueueWork (closure, true)) {
#if DEBUG && SANITY
		printf ("Media::DisposeObject (%p): Could not add callback to the media thread, calling Dispose directly.\n", obj);
#endif
		obj->Dispose ();
	}
	closure->unref ();
}

void
Media::WakeUp ()
{
	MediaThreadPool::WakeUp ();
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

ASXDemuxer::ASXDemuxer (Media *media, IMediaSource *source)
	: IMediaDemuxer (Type::ASXDEMUXER, media, source)
{
	playlist = NULL;
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
	IMediaDemuxer::Dispose ();
}

void
ASXDemuxer::OpenDemuxerAsyncInternal ()
{
	MediaResult result;
	PlaylistRoot *root;
	ErrorEventArgs *args = NULL;
	Media *media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	root = media->GetPlaylistRoot ();
	
	g_return_if_fail (root != NULL);

	PlaylistParser *parser = new PlaylistParser (root, source);

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
	} else if (args != NULL) {
		args->ref (); // calling ReportErrorOccurred with an event args will end up unreffing it
		ReportErrorOccurred (args);
	} else {
		ReportErrorOccurred (result);
	}
	if (args)
		args->unref ();
	
	media->unref ();
}

/*
 * ASXDemuxerInfo
 */

MediaResult
ASXDemuxerInfo::Supports (IMediaSource *source)
{
	if (PlaylistParser::IsASX2 (source) || PlaylistParser::IsASX3 (source)) {
		return MEDIA_SUCCESS;
	} else {
		return MEDIA_FAIL;
	}
}

IMediaDemuxer *
ASXDemuxerInfo::Create (Media *media, IMediaSource *source)
{
	return new ASXDemuxer (media, source);
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

gint32 
ManagedStreamSource::ReadInternal (void *buf, guint32 n)
{
	return stream.Read (stream.handle, buf, 0, n);
}

gint32 
ManagedStreamSource::PeekInternal (void *buf, guint32 n)
{
	int read;
	
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
	
/*
 * FileSource
 */

FileSource::FileSource (Media *media, const char *filename) : IMediaSource (Type::FILESOURCE, media)
{
	this->filename = g_strdup (filename);
	fd = NULL;
	size = 0;
	temp_file = false;
}

FileSource::FileSource (Media *media, bool temp_file) : IMediaSource (Type::FILESOURCE, media)
{
	filename = NULL;
	fd = NULL;
	size = 0;
	this->temp_file = temp_file;
}

FileSource::~FileSource ()
{
}

void
FileSource::Dispose ()
{
	g_free (filename);
	filename = NULL;
	if (fd != NULL) {
		fclose (fd);
		fd = NULL;
	}
	IMediaSource::Dispose ();
}

MediaResult 
FileSource::Initialize ()
{
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
			
		fd = g_fopen (filename, "r");
	}

	if (fd == NULL)
		return MEDIA_FILE_ERROR;

	UpdateSize ();
		
	return MEDIA_SUCCESS;
}

MediaResult
FileSource::Open (const char *filename)
{	
	g_return_val_if_fail (filename != NULL, MEDIA_FAIL);
	
	g_free (this->filename);
	this->filename = g_strdup (filename);
	
	if (fd != NULL) {
		fclose (fd);
		fd = NULL;
	}
	
	fd = fopen (filename, "r");
	
	if (fd == NULL)
		return MEDIA_FAIL;
		
	UpdateSize ();
	
	return MEDIA_SUCCESS;
}

void
FileSource::UpdateSize ()
{
	struct stat st;
	
	g_return_if_fail (fd != NULL);
	
	if (fstat (fileno (fd), &st) != -1) {
		size = st.st_size;
	} else {
		size = 0;
	}
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

	LOG_PIPELINE_EX ("FileSource::GetPositionInternal (): result: %lld\n", result);

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

	LOG_PIPELINE_EX ("FileSource::ReadInternal (0x????????, %i), nread: %i\n", (int) n, (int) nread);

	return nread;
}

gint32
FileSource::PeekInternal (void *buf, guint32 n)
{
	gint32 result;

	result = ReadSome (buf, n);
	
	Seek (-result, SEEK_CUR);

	LOG_PIPELINE_EX ("FileSource<%i>::PeekInternal (%p, %i), GetPosition (): %lld [Done]\n", GET_OBJ_ID (this), buf, n, GetPosition ());

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

ProgressiveSource::ProgressiveSource (Media *media, const char *uri) : FileSource (media, true)
{
	write_pos = 0;
	size = -1;
	write_fd = NULL;
	cancellable = NULL;
	this->uri = g_strdup (uri);
}

void
ProgressiveSource::Dispose ()
{	
	g_free (uri);
	uri = NULL;
	
	if (cancellable) {
		if (Surface::InMainThread ()) {
			delete_cancellable (this);
		} else {
			// we have to cancel/delete he cancellable on the main thread
			// it may end up doing a lot of stuff, including calling into
			// mozilla.
				
			// The tick call will ref us until the callback has been called.
			// Note that it may cause a warning to be printed
			// in ref () (reffing an object with a refcount of 0). 
			// TODO: find a way to avoid the warning in this case, imho this is
			// a valid case of reffing an object with a refcount of 0.
			AddTickCallSafe (delete_cancellable);
		}
	}
	
	CloseWriteFile ();
	
	FileSource::Dispose ();
}

void
ProgressiveSource::delete_cancellable (EventObject *data)
{
	ProgressiveSource *src = (ProgressiveSource *) data;
	if (src->cancellable) {
		src->cancellable->Cancel ();
		delete src->cancellable;
		src->cancellable = NULL;
	}
}

MediaResult
ProgressiveSource::Initialize ()
{
	MediaResult result = MEDIA_SUCCESS;
	Application *application;
	
	application = GetDeployment ()->GetCurrentApplication ();
	
	g_return_val_if_fail (application != NULL, MEDIA_FAIL);
	g_return_val_if_fail (filename == NULL, MEDIA_FAIL);
	g_return_val_if_fail (cancellable == NULL, MEDIA_FAIL);

	result = FileSource::Initialize ();

	if (!MEDIA_SUCCEEDED (result))
		return result;

	write_fd = g_fopen (filename, "w");
	if (write_fd == NULL) {
		char *msg = g_strdup_printf ("Could not open a write handle to the file '%s'\n", filename);
		ReportErrorOccurred (msg);
		g_free (msg);
		return MEDIA_FAIL;
	}

	// unlink the file right away so that it'll be deleted even if we crash.
	if (moonlight_flags & RUNTIME_INIT_KEEP_MEDIA) {
		printf ("Moonlight: The media file %s will not deleted.\n", filename);
	} else {
		g_unlink (filename);
	}
	
	cancellable = new Cancellable ();
	Uri *u = new Uri ();
	if (u->Parse (uri)) {
		application->GetResource (NULL, u, notify_func, data_write, MediaPolicy, cancellable, (gpointer) this);
	} else {
		result = MEDIA_FAIL;
		char *msg = g_strdup_printf ("Could not parse the uri '%s'", uri);
		ReportErrorOccurred (msg);
		g_free (msg);
	}
	delete u;
	
	return result;
}

void
ProgressiveSource::notify_func (NotifyType type, gint64 args, void *closure)
{
	g_return_if_fail (closure != NULL);
	((ProgressiveSource *) closure)->Notify (type, args);
}

void
ProgressiveSource::Notify (NotifyType type, gint64 args)
{
	LOG_PIPELINE ("ProgressiveSource::Notify (%i = %s, %" G_GINT64_FORMAT ")\n", 
		type, 
		type == ::NotifySize ? "NotifySize" : 
			(type == NotifyCompleted ? "NotifyCompleted" : 
			(type == NotifyFailed ? "NotifyFailed" : 
			(type == NotifyStarted ? "NotifyStarted" : 
			(type == NotifyProgressChanged ? "NotifyProgressChanged" : "unknown")))),
		args);
		
	switch (type) {
		case ::NotifySize:
			NotifySize (args);
			break;
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

void
ProgressiveSource::data_write (void *data, gint32 offset, gint32 n, void *closure)
{
	g_return_if_fail (closure != NULL);
	((ProgressiveSource *) closure)->DataWrite (data, offset, n);
}

void
ProgressiveSource::DataWrite (void *buf, gint32 offset, gint32 n)
{
	size_t nwritten;
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("ProgressiveSource::DataWrite (%p, %i, %i) media: %p, filename: %s\n", buf, offset, n, media, filename);
	
	g_return_if_fail (write_fd != NULL);
	
	if (n == 0) {
		// We've got the entire file, update the size
		size = write_pos; // Since this method is the only method that writes to write_pos, and we're not reentrant, there is no need to lock here.
		
		// Close our write handle, we won't write more now
		CloseWriteFile ();
				
		goto cleanup;
	}

	nwritten = fwrite (buf, 1, n, write_fd);
	fflush (write_fd);

	Lock ();
	write_pos += nwritten;
	Unlock ();

cleanup:
	if (media) {
		media->WakeUp ();
		media->ReportDownloadProgress ((double) (offset + n) / (double) size);
		media->unref ();
	}
}

void
ProgressiveSource::NotifySize (gint64 size)
{
	LOG_PIPELINE ("ProgressiveSource::NotifySize (%lld)\n", size);
	
	Lock ();
	this->size = size;
	Unlock ();
}

void
ProgressiveSource::DownloadComplete ()
{
	MediaResult result = MEDIA_SUCCESS;
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("ProgressiveSource::DownloadComplete ()\n");
	
	Lock ();
	if (write_pos != size && size != -1) { // what happend here?
		LOG_PIPELINE ("ProgressiveSource::DownloadComplete (): the downloaded size (%" G_GINT64_FORMAT ") != the reported size (%" G_GINT64_FORMAT	 ")\n", write_pos, size);
	}

	this->size = write_pos;
	
	// Close our write handle, we won't write more now
	CloseWriteFile ();
	
	Unlock ();
	
	if (!MEDIA_SUCCEEDED (result))
		ReportErrorOccurred (result);
	
	if (media) {
		media->ReportDownloadProgress (1.0);
		media->WakeUp ();
		media->unref ();
	}
}

void
ProgressiveSource::DownloadFailed ()
{
	LOG_PIPELINE ("ProgressiveSource::DownloadFailed ().\n");
	
	ReportErrorOccurred (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR")));
}

void
ProgressiveSource::CloseWriteFile ()
{
	if (write_fd == NULL)
		return;
		
	fclose (write_fd);
	write_fd = NULL;
}

/*
 * MemorySource
 */
 
MemorySource::MemorySource (Media *media, void *memory, gint32 size, gint64 start, bool owner)
	: IMediaSource (Type::MEMORYSOURCE, media)
{
	this->memory = memory;
	this->size = size;
	this->start = start;
	this->pos = 0;
	this->owner = owner;
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
 * MediaThreadPool
 */

pthread_mutex_t MediaThreadPool::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t MediaThreadPool::condition = PTHREAD_COND_INITIALIZER;
int MediaThreadPool::count = 0;
pthread_t MediaThreadPool::threads [max_threads];
Media *MediaThreadPool::medias [max_threads];
bool MediaThreadPool::shutting_down = false;
List MediaThreadPool::queue;
bool MediaThreadPool::valid [max_threads];

void
MediaThreadPool::AddWork (MediaClosure *closure, bool wakeup)
{
	pthread_attr_t attribs;
	int result = 0;
	
	pthread_mutex_lock (&mutex);
	
	if (shutting_down) {
		LOG_PIPELINE ("Moonlight: could not execute closure because we're shutting down.\n");
	} else {
		queue.Append (new MediaWork (closure));
		
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
			closure->GetDescription (), closure, closure->GetMedia (), GET_OBJ_ID (closure->GetMedia ()), closure->GetDeployment (), queue.Length ());
		
		if (wakeup)
			pthread_cond_signal (&condition);
	}
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
	current = queue.First ();
	while (current != NULL) {
		next = current->next; // retrieve next before Unlinking
		if (((MediaWork *) current)->closure->GetMedia () == media) {
			queue.Unlink (current);
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

void
MediaThreadPool::WakeUp ()
{
	LOG_FRAMEREADERLOOP ("MediaThreadPool::WakeUp ()\n");
	
	pthread_mutex_lock (&mutex);
	pthread_cond_signal (&condition);
	pthread_mutex_unlock (&mutex);
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
	return true;
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
	List::Node *current;
	List::Node *next;
	
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
	
	current = queue.First ();
	queue.Clear (false);
	
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
		
		node = (MediaWork *) queue.First ();
		
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
			queue.Unlink (node);
		}
		
		if (node != NULL)
			medias [self_index] = media;
		
		pthread_mutex_unlock (&mutex);
		
		if (node == NULL)
			continue;
		
		media->SetCurrentDeployment (true, true);

		LOG_FRAMEREADERLOOP ("MediaThreadLoop::WorkerLoop () %u: got %s %p for media %p on deployment %p, there are %d nodes left.\n", (int) pthread_self (), node->closure->GetDescription (), node, media, media->GetDeployment (), queue.Length ());
		
		node->closure->Call ();
		
		LOG_FRAMEREADERLOOP ("MediaThreadLoop::WorkerLoop () %u: processed node %p\n", (int) pthread_self (), node);
		
		delete node;
	}
	
	pthread_mutex_lock (&mutex);
	medias [self_index] = NULL;
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
 * IMediaStream
 */

IMediaStream::IMediaStream (Type::Kind kind, Media *media) : IMediaObject (kind, media)
{
	context = NULL;
	
	extra_data_size = 0;
	extra_data = NULL;
	
	duration = 0;
	
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
	case CODEC_WMV1: return g_strdup ("wmv1");
	case CODEC_WMV2: return g_strdup ("wmv2");
	case CODEC_WMV3: return g_strdup ("wmv3");
	case CODEC_WMVA: return g_strdup ("wmva");
	case CODEC_WVC1: return g_strdup ("vc1");
	case CODEC_RGBA: return g_strdup ("rgba");
	case CODEC_YV12: return g_strdup ("yv12");
	case CODEC_MP3: return g_strdup ("mp3");
	case CODEC_WMAV1: return g_strdup ("wmav1");
	case CODEC_WMAV2: return g_strdup ("wmav2");
	case CODEC_WMAV3: return g_strdup ("wmav3");
	case CODEC_PCM: return g_strdup ("pcm");
	default:
		g_warning ("IMediaStream::CreateCodec (%i): Not implemented.\n", codec_id);
		
		/* This algorithm needs testing.
		char *result;
		int size, current;
		int a = (codec_id & 0x000000FF);
		int b = (codec_id & 0x0000FF00) >> 8;
		int c = (codec_id & 0x00FF0000) >> 16;
		int d = (codec_id & 0xFF000000) >> 24;
		
		size = (a != 0) + (b != 0) + (c != 0) + (d != 0);
		
		g_return_val_if_fail (size >= 0 && size <= 4, g_strdup (""));
		
		result = (char *) g_malloc (size + 1);
		current = 0;
		if (a)
			result [current++] = (char) a;
		if (b)
			result [current++] = (char) b;
		if (c)
			result [current++] = (char) c;
		if (d)
			result [current++] = (char) d;
		result [current] = 0;
		*/
		return g_strdup ("<unknown>");
	}
	
}

bool
IMediaStream::IsQueueEmpty ()
{
	return queue.IsEmpty ();
}

const char *
IMediaStream::GetStreamTypeName ()
{
	switch (GetType ()) {
	case MediaTypeVideo: return "Video";
	case MediaTypeAudio: return "Audio";
	case MediaTypeMarker: return "Marker";
	default: return "Unknown";
	}
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
IMediaStream::GetDemuxer ()
{
	Media *media;
	IMediaDemuxer *result;
	
	if (IsDisposed ())
		return NULL;
	
	media = GetMediaReffed ();
		
	g_return_val_if_fail (media != NULL, NULL);
	
	result = media->GetDemuxer ();
	
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

	LOG_BUFFERING ("IMediaStream::GetBufferedSize (): id: %i, codec: %s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %llu ms, result: %llu ms\n",
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
		printf ("size: %.4llu, first: %.4lli, last popped: %.4lli, last enq: %.4lli, frames enq: %i>",
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
	Media *media;
	
	g_return_if_fail (Media::InMediaThread ());
	
	media = GetMediaReffed ();
	g_return_if_fail (media != NULL);
	
	queue.Lock ();
	if (first_pts == G_MAXUINT64)
		first_pts = frame->pts;

	LOG_PIPELINE ("IMediaStream::EnqueueFrame (%p) %s %" G_GUINT64_FORMAT " ms\n", frame, frame ? frame->stream->GetStreamTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : 0);

#if 0
	if (last_enqueued_pts > frame->pts && last_enqueued_pts != G_MAXUINT64 && frame->event != FrameEventEOF && frame->buflen > 0) {
		g_warning ("IMediaStream::EnqueueFrame (): codec: %.5s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %llu ms, "
		"buffer: %" G_GUINT64_FORMAT " ms, frame: %p, frame->buflen: %i, frame->pts: %" G_GUINT64_FORMAT " ms (the last enqueued frame's pts is below the current frame's pts)\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), frame, frame->buflen, MilliSeconds_FromPts (frame->pts));
	}
#endif

	last_enqueued_pts = frame->pts;
	first = queue.LinkedList ()->Length () == 0;
	queue.LinkedList ()->Append (new StreamNode (frame));
	queue.Unlock ();

	SetLastAvailablePts (frame->pts);

	if (first)
		EmitSafe (FirstFrameEnqueuedEvent);
	
	FrameEnqueued ();

	media->unref ();

	LOG_BUFFERING ("IMediaStream::EnqueueFrame (): codec: %.5s, first: %i, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %" G_GUINT64_FORMAT " ms, buffer: %" G_GUINT64_FORMAT " ms, frame: %p, frame->buflen: %i\n",
		codec, first, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts - first_pts), frame, frame->buflen);
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
	
	LOG_BUFFERING ("IMediaStream::PopFrame (): codec: %.5s, first_pts: %" G_GUINT64_FORMAT " ms, last_popped_pts: %" G_GUINT64_FORMAT " ms, last_enqueued_pts: %llu ms, buffer: %llu ms, frame: %p, frame->buflen: %i\n",
		codec, MilliSeconds_FromPts (first_pts), MilliSeconds_FromPts (last_popped_pts), MilliSeconds_FromPts (last_enqueued_pts), 
		MilliSeconds_FromPts (last_popped_pts != G_MAXUINT64 ? last_enqueued_pts - last_popped_pts : last_enqueued_pts), result, result ? result->buflen : 0);

	if (!input_ended && !output_ended && result != NULL) {
		IMediaDemuxer *demuxer = GetDemuxer ();
		if (demuxer != NULL)
			demuxer->FillBuffers ();
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
	Media *media = GetMediaReffed ();
	
	selected = value;
	if (media) {
		if (media->GetDemuxer ())
			media->GetDemuxer ()->UpdateSelected (this);
		media->unref ();
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
	opened = false;
	opening = false;
	pending_stream = NULL;
}

IMediaDemuxer::IMediaDemuxer (Type::Kind kind, Media *media)
	: IMediaObject (kind, media)
{
	source = NULL;
	stream_count = 0;
	streams = NULL;
	opened = false;
	opening = false;
	pending_stream = NULL;
}

void
IMediaDemuxer::Dispose ()
{
	if (streams != NULL) {
		IMediaStream **tmp = streams;
		int stream_count = this->stream_count;
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
	
	closure = new MediaClosure (media, OpenCallback, this, "IMediaDemuxer::OpenCallback");
	media->EnqueueWork (closure, false);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::ReportOpenDemuxerCompleted ()
{
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("IMediaDemuxer::ReportDemuxerOpenCompleted () media: %p\n", media);
	
	opened = true;
	opening = false;
	
	// Media might be null if we got disposed for some reason.
	if (!media)
		return;
		
	media->ReportOpenDemuxerCompleted ();
	media->unref ();
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
	LOG_PIPELINE ("IMediaDemuxer::ReportGetDiagnosticCompleted (%i, %lld)\n", kind, value);
}

void
IMediaDemuxer::ReportGetFrameCompleted (MediaFrame *frame)
{
	IMediaDecoder *decoder;
	
	g_return_if_fail (frame == NULL || (frame != NULL && frame->stream != NULL));
	g_return_if_fail (pending_stream != NULL); // we must be waiting for a frame ...
	
	LOG_PIPELINE ("IMediaDemuxer::ReportGetFrameCompleted (%p) %i %s %llu ms\n", frame, GET_OBJ_ID (this), frame ? frame->stream->GetStreamTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : (guint64) -1);
	
	if (frame == NULL) {
		LOG_PIPELINE ("IMediaDemuxer::ReportGetFrameCompleted (%p): input end signaled for %s stream.\n", frame, pending_stream->GetStreamTypeName ());
		// No more data for this stream
		pending_stream->SetInputEnded (true);
	} else if (!frame->stream->IsDisposed ()) {
		decoder = frame->stream->GetDecoder ();
		decoder->DecodeFrameAsync (frame);
	}
	
	pending_stream->unref ();
	pending_stream = NULL; // not waiting for anything more
	
	// enqueue some more 
	FillBuffers ();
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
	MediaClosure *closure = new MediaReportSeekCompletedClosure (media, ReportSeekCompletedCallback, this, pts);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::ReportSeekCompleted (guint64 pts)
{
	Media *media;

	LOG_PIPELINE ("IMediaDemuxer::ReportSeekCompleted (%llu)\n", pts);
	
	if (!Media::InMediaThread ()) {
		EnqueueReportSeekCompleted (pts);
		return;
	}
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	
	for (int i = 0; i < GetStreamCount (); i++) {
		IMediaStream *stream = GetStream (i);
		
		if (stream == NULL)
			continue;
		
		stream->ReportSeekCompleted ();
	}
	
	media->ReportSeekCompleted (pts);
	media->unref ();
	
	if (pending_stream) {
		pending_stream->unref ();
		pending_stream = NULL;
	}
	
	FillBuffers ();
	
	LOG_PIPELINE ("IMediaDemuxer::ReportSeekCompleted (%llu) [Done]\n", pts);
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
	MediaClosure *closure = new MediaGetFrameClosure (media, GetFrameCallback, this, stream);
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::GetFrameAsync (IMediaStream *stream)
{
	Media *media;
	
	LOG_PIPELINE ("IMediaDemuxer::GetFrameAsync (%p) %s InMediaThread: %i\n", stream, stream->GetStreamTypeName (), Media::InMediaThread ());
	
	if (!Media::InMediaThread ()) {
		EnqueueGetFrame (stream);
		return;
	}
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	if (stream != NULL && pending_stream == NULL) {
		pending_stream = stream;
		pending_stream->ref ();
		GetFrameAsyncInternal (stream);
	}

	media->unref ();
}

MediaResult
IMediaDemuxer::SeekCallback (MediaClosure *closure)
{
	MediaSeekClosure *seek = (MediaSeekClosure *) closure;
	seek->GetDemuxer ()->SeekAsyncInternal (seek->GetPts ());
	return MEDIA_SUCCESS;
}

void
IMediaDemuxer::EnqueueSeek (guint64 pts)
{
	Media *media = GetMediaReffed ();
	MediaSeekClosure *closure;
	
	g_return_if_fail (media != NULL);
	
	closure = new MediaSeekClosure (media, SeekCallback, this, pts);
	media->EnqueueWork (closure, true);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::SeekAsync (guint64 pts)
{
	LOG_PIPELINE ("IMediaDemuxer::SeekAsync (%llu)\n", pts);

	if (IsDisposed ())
		return;
		
	// we need to run this on the media thread, not the main thread.
	EnqueueSeek (pts);
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
	Media *media = GetMediaReffed ();
	MediaClosure *closure;
	
	g_return_if_fail (media != NULL);
	
	closure = new MediaClosure (media, FillBuffersCallback, this, "IMediaDemuxer::FillBuffersCallback");
	media->EnqueueWork (closure);
	closure->unref ();
	media->unref ();
}

void
IMediaDemuxer::FillBuffersInternal ()
{
	IMediaStream *stream;
	IMediaStream *request_stream = NULL;
	guint64 min_buffered_size = G_MAXUINT64;
	MediaResult result = MEDIA_SUCCESS;
	Media *media = GetMediaReffed ();
	guint64 buffering_time = 0;
	guint64 buffered_size = 0;
	int ended = 0;
	int media_streams = 0;
	
	LOG_BUFFERING ("IMediaDemuxer::FillBuffersInternal (), %i %s buffering time: %llu = %llu ms, pending_stream: %i %s\n", GET_OBJ_ID (this), GetTypeName (), buffering_time, media != NULL ? MilliSeconds_FromPts (media->GetBufferingTime ()) : -1, GET_OBJ_ID (pending_stream), pending_stream ? pending_stream->GetStreamTypeName () : "NULL");

	if (IsDisposed ())
		goto cleanup;

	// If we're waiting for something, there's nothing to do here.
	if (pending_stream != NULL)
		goto cleanup;

	// Find the stream with the smallest buffered size, and request a frame from that stream.
	g_return_if_fail (media != NULL);
	
	buffering_time = media->GetBufferingTime ();
	
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
		if (!stream->GetSelected ())
			continue;

		if (stream->GetType () != MediaTypeVideo && 
			stream->GetType () != MediaTypeAudio)
			continue;

		media_streams++;
		if (stream->GetOutputEnded ()) {
			ended++;
			continue; // this stream has ended.
		}
	
		decoder = stream->GetDecoder ();
		if (decoder == NULL) {
			fprintf (stderr, "IMediaDemuxer::FillBuffersInternal () %s stream has no decoder (id: %i refcount: %i)\n", stream->GetStreamTypeName (), GET_OBJ_ID (stream), stream->GetRefCount ());
			continue; // no decoder??
		}
	
		buffered_size = stream->GetBufferedSize ();
		min_buffered_size = MIN (min_buffered_size, buffered_size);
			
		if (buffered_size >= buffering_time)
			continue; // this stream has enough data buffered.
		
		if (!decoder->IsDecoderQueueEmpty ())
			continue; // this stream is waiting for data to be decoded.
			
		if (buffered_size <= min_buffered_size)
			request_stream = stream;
		
		LOG_BUFFERING ("IMediaDemuxer::FillBuffersInternal (): codec: %s, stream id: %i, result: %i, buffered size: %" G_GUINT64_FORMAT " ms, buffering time: %" G_GUINT64_FORMAT " ms, last popped time: %llu ms\n", 
				stream->codec, GET_OBJ_ID (stream), result, MilliSeconds_FromPts (buffered_size), MilliSeconds_FromPts (buffering_time), MilliSeconds_FromPts (stream->GetLastPoppedPts ()));
	}
	
	if (request_stream != NULL) {
		LOG_BUFFERING ("IMediaDemuxer::FillBuffersInternal (): requesting frame from %s stream (%i), min_buffered_size: %" G_GUINT64_FORMAT " ms\n", request_stream->GetStreamTypeName (), GET_OBJ_ID (stream), MilliSeconds_FromPts (min_buffered_size));
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

		if (stream->GetType () != MediaTypeVideo && stream->GetType () != MediaTypeAudio)
			continue;

		result = MIN (result, stream->GetBufferedSize ());
	}

	return result;
}

guint64
IMediaDemuxer::GetLastAvailablePts ()
{
	guint64 result = G_MAXUINT64;
	IMediaStream *stream;
	
	for (int i = 0; i < GetStreamCount (); i++) {
		stream = GetStream (i);

		if (stream == NULL || !stream->GetSelected ())
			continue;

		result = MIN (result, stream->GetLastAvailablePts ());
	}

	if (result == G_MAXUINT64)
		result = 0;

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
	: EventObject (Type::MEDIAFRAME)
{
	Initialize ();
	
	g_return_if_fail (stream != NULL);
	
	this->stream = stream;
	this->stream->ref ();
}

MediaFrame::MediaFrame (IMediaStream *stream, guint8 *buffer, guint32 buflen, guint64 pts, bool keyframe)
	: EventObject (Type::MEDIAFRAME)
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
		printf ("MediaFrame::MediaFrame () %s buffer: ", stream->GetStreamTypeName ());
		for (int i = 0; i < 4; i++)
			printf (" 0x%x", buffer [i]);
		printf ("\n");
	}
#endif

	if (keyframe)
		AddState (MediaFrameKeyFrame);
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
		
	if (events == NULL)
		goto cleanup;
		
	// Create a list of all the events to emit
	// don't keep the lock while emitting.
	event_mutex.Lock ();
	if (events != NULL) {
		ed = (EventData *) events->First ();
		while (ed != NULL) {
			if (ed->event_id == event_id) {
				emit = new EmitData (event_id, ed->handler, ed->context, args);
				if (ed->invoke_on_main_thread) {
					if (emit_on_main_thread == NULL)
						emit_on_main_thread = new List ();
					emit_on_main_thread->Append (emit);
				} else {
					if (emits == NULL)
						emits = new List ();
					emits->Append (emit);
				}
			}
			ed = (EventData *) ed->next;
		}
	}
	event_mutex.Unlock ();
	
	// emit the events to be emitted on this thread
	EmitList (emits);
	
	if (Surface::InMainThread ()) {
		// if we're already on the main thread, 
		// we can the events to be emitted
		// on the main thread
		List *tmp;
		event_mutex.Lock ();
		tmp = emit_on_main_thread;
		emit_on_main_thread = NULL;
		event_mutex.Unlock ();
		EmitList (tmp);
	} else {
		AddTickCallSafe (EmitListCallback);
	}	
	
cleanup:
	if (args)
		args->unref ();
}

void
IMediaObject::EmitListMain ()
{
	VERIFY_MAIN_THREAD;
	
	List *list;
	event_mutex.Lock ();
	list = emit_on_main_thread;
	emit_on_main_thread = NULL;
	event_mutex.Unlock ();
	EmitList (list);
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
	
	delete list;
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
IMediaObject::ReportErrorOccurred (MediaResult result)
{
	g_return_if_fail (media != NULL);
	
	media->ReportErrorOccurred (result);
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

	pthread_cond_init (&condition, NULL);
}

IMediaSource::~IMediaSource ()
{
	pthread_mutex_destroy (&mutex);
	pthread_cond_destroy (&condition);	
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

gint32
IMediaSource::ReadSome (void *buf, guint32 n)
{
	gint32 result;

	LOG_PIPELINE_EX ("IMediaSource<%i>::ReadSome (%p, %u)\n", GET_OBJ_ID (this), buf, n);

	Lock ();

	result = ReadInternal (buf, n);

	LOG_PIPELINE_EX ("IMediaSource<%i>::ReadSome (%p, %u) read %i, position: %lld\n", GET_OBJ_ID (this), buf, n, result, GetPosition ());

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
		FileSource *fs = NULL;
		
		if (GetType () == MediaSourceTypeFile)
			fs = (FileSource *) this;
		g_warning ("IMediaSource::ReadInternal (%i): Read failed, read %i bytes. available size: %lld, size: %lld, pos: %lld, prev pos: %lld, position not available: %lld, feof: %i, ferror: %i, strerror: %s\n", 
			n, read, avail, GetSize (), GetPosition (), prev, prev + n, fs ? feof (fs->fd) : -1, fs ? ferror (fs->fd) : -1, fs ? strerror (ferror (fs->fd)) : "<N/A>");
		print_stack_trace ();
	}
	
	LOG_PIPELINE_EX ("IMediaSource<%d>::ReadAll (%p, %u), read: %d [Done].\n", GET_OBJ_ID (this), buf, n, read);
	
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

	LOG_PIPELINE ("IMediaSource::Peek (%p, %u): peek result: %i, read %lld bytes.\n", buf, n, result, read);

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
IMediaSource::GetPositionInternal ()
{
	// This method should be overridden (or never called for the classes which doesn't override it).
	g_warning ("IMediaSource (%s)::GetPositionInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName ());
	print_stack_trace ();

	return -1;
}
bool
IMediaSource::SeekInternal (gint64 offset, int mode)
{
	g_warning ("IMediaSource (%s)::SeekInternal (%lld, %i): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName (), offset, mode);
	print_stack_trace ();

	return false;
}

gint32 
IMediaSource::ReadInternal (void *buffer, guint32 n)
{
	g_warning ("IMediaSource (%s)::ReadInternal (%p, %u): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName (), buffer, n);
	print_stack_trace ();
	
	return 0;
}

gint32
IMediaSource::PeekInternal (void *buffer, guint32 n)
{
	g_warning ("IMediaSource (%s)::PeekInternal (%p, %u): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName (), buffer, n);
	print_stack_trace ();
	
	return 0;
}

gint64
IMediaSource::GetSizeInternal ()
{
	g_warning ("IMediaSource (%s)::GetSizeInternal (): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", GetTypeName ());
	print_stack_trace ();
	
	return 0;
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

void
IMediaDecoder::ReportDecodeFrameCompleted (MediaFrame *frame)
{
	LOG_PIPELINE ("IMediaDecoder::ReportDecodeFrameCompleted (%p) %s %llu ms\n", frame, frame ? frame->stream->GetStreamTypeName () : "", frame ? MilliSeconds_FromPts (frame->pts) : 0);
	
	g_return_if_fail (frame != NULL);
	g_return_if_fail (frame->stream != NULL);
	
	frame->stream->EnqueueFrame (frame);
	frame->stream->GetDemuxer ()->FillBuffers ();
	
	if (input_ended && IsDecoderQueueEmpty ())
		InputEnded ();
}

MediaResult
IMediaDecoder::DecodeFrameCallback (MediaClosure *closure)
{
	
	IMediaDecoder *decoder = (IMediaDecoder *) closure->GetContext ();
	IMediaDecoder::FrameNode *node = (IMediaDecoder::FrameNode *) decoder->queue.Pop ();
	
	if (node != NULL) {
		decoder->DecodeFrameAsync (node->frame);
		delete node;
	}

	return MEDIA_SUCCESS;
}

void
IMediaDecoder::DecodeFrameAsync (MediaFrame *frame)
{
	Media *media;

	LOG_PIPELINE ("IMediaDecoder::DecodeFrameAsync (%p) %s\n", frame, (frame && frame->stream) ? frame->stream->GetStreamTypeName () : NULL);
	
	if (IsDisposed ())
		return;
	
	g_return_if_fail (frame != NULL);
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	if (!Media::InMediaThread ()) {
		MediaClosure *closure = new MediaClosure (media, DecodeFrameCallback, this, "IMediaDecoder::DecodeFrameCallback");
		queue.Push (new FrameNode (frame));
		media->EnqueueWork (closure);
		closure->unref ();
		return;
	}
	
	DecodeFrameAsyncInternal (frame);

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

void
IMediaDecoder::ReportOpenDecoderCompleted ()
{
	Media *media = GetMediaReffed ();
	
	LOG_PIPELINE ("IMediaDecoder::ReportOpenDecoderCompleted ()\n");
	
	opening = false;
	opened = true;
	
	g_return_if_fail (media != NULL);
	
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
	pts_per_frame = 0;
	initial_pts = 0;
	height = 0;
	width = 0;
}

VideoStream::VideoStream (Media *media, int codec_id, guint32 width, guint32 height, guint64 duration, gpointer extra_data, guint32 extra_data_size)
	: IMediaStream (Type::VIDEOSTREAM, media)
{
	converter = NULL;
	bits_per_sample = 0;
	pts_per_frame = 0;
	initial_pts = 0;
	this->height = height;
	this->width = width;
	this->duration = duration;
	this->codec_id = codec_id;
	this->codec = CreateCodec (codec_id);
	this->extra_data = extra_data;
	this->extra_data_size = extra_data_size;
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
	
	GetDecoder ()->DecodeFrameAsync (frame);
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

		frame->width = vs->width;
		frame->height = vs->height;

		frame->data_stride[0] = frame->buffer;
		frame->data_stride[1] = frame->buffer + (frame->width*frame->height);
		frame->data_stride[2] = frame->buffer + (frame->width*frame->height)+(frame->width/2*frame->height/2);
		frame->buffer = NULL;
		frame->srcStride[0] = frame->width;
		frame->srcSlideY = frame->width;
		frame->srcSlideH = frame->height;

		frame->AddState (MediaFramePlanar);
	}
	ReportDecodeFrameCompleted (frame);
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
	g_free (frame->buffer);
	frame->buflen = logo_size;
	frame->buffer = (guint8*) g_malloc (frame->buflen);
	memcpy (frame->buffer, logo, frame->buflen);
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
	g_free (frame->buffer);

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

	frame->buflen = data_size;
	frame->buffer = (guint8 *) g_malloc0 (frame->buflen);
	
	frame->AddState (MediaFrameDecoded);
	
	return MEDIA_SUCCESS;
}

void
NullDecoder::DecodeFrameAsyncInternal (MediaFrame *frame)
{
	MediaResult result = MEDIA_FAIL;
	IMediaStream *stream = GetStream ();
	
	if (stream->GetType () == MediaTypeAudio) {
		result = DecodeAudioFrame (frame);
	} else if (stream->GetType () == MediaTypeVideo) {
		result = DecodeVideoFrame (frame);
	}
	
	if (MEDIA_SUCCEEDED (result)) {
		ReportDecodeFrameCompleted (frame);
	} else {
		ReportErrorOccurred (result);
	}
}

void
NullDecoder::OpenDecoderAsyncInternal ()
{
	MediaResult result;
	IMediaStream *stream = GetStream ();
	
	if (stream->GetType () == MediaTypeAudio)
		result = OpenAudio ();
	else if (stream->GetType () == MediaTypeVideo)
		result = OpenVideo ();
	else
		result = MEDIA_FAIL;
		
	if (MEDIA_SUCCEEDED (result)) {
		ReportOpenDecoderCompleted ();
	} else {
		ReportErrorOccurred (result);
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
	g_return_if_fail (instance != NULL);
	g_return_if_fail (close_demuxer != NULL && get_diagnostic != NULL && get_sample != NULL && open_demuxer != NULL && seek != NULL && switch_media_stream != NULL);
	
	this->close_demuxer_callback = close_demuxer;
	this->get_diagnostic_async_callback = get_diagnostic;
	this->get_sample_async_callback = get_sample;
	this->open_demuxer_async_callback = open_demuxer;
	this->seek_async_callback = seek;
	this->switch_media_stream_async_callback = switch_media_stream;
	this->instance = instance;
}
	
void
ExternalDemuxer::Dispose ()
{	
	instance = NULL;
	close_demuxer_callback = NULL;
	get_diagnostic_async_callback = NULL;
	get_sample_async_callback = NULL;
	open_demuxer_async_callback = NULL;
	seek_async_callback = NULL;
	switch_media_stream_async_callback = NULL;
	
	IMediaDemuxer::Dispose ();
}

void
ExternalDemuxer::SetCanSeek (bool value)
{
	g_warning ("TODO: ExternalDemuxer::SetCanSeek ()");
}

gint32
ExternalDemuxer::AddStream (IMediaStream *stream)
{
	return IMediaDemuxer::AddStream (stream);
}

void 
ExternalDemuxer::CloseDemuxerInternal ()
{
	g_return_if_fail (close_demuxer_callback != NULL);
	
	close_demuxer_callback (instance);
}

void 
ExternalDemuxer::GetDiagnosticAsyncInternal (MediaStreamSourceDiagnosticKind diagnosticsKind)
{
	g_return_if_fail (get_diagnostic_async_callback != NULL);
	
	get_diagnostic_async_callback (instance, diagnosticsKind);
}

void 
ExternalDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	g_return_if_fail (get_sample_async_callback != NULL);
	g_return_if_fail (stream != NULL);
	
	get_sample_async_callback (instance, stream->GetStreamType ());
}

void 
ExternalDemuxer::OpenDemuxerAsyncInternal ()
{
	g_return_if_fail (open_demuxer_async_callback != NULL);
	
	open_demuxer_async_callback (instance, this);
}

void 
ExternalDemuxer::SeekAsyncInternal (guint64 seekToTime)
{
	g_return_if_fail (seek_async_callback != NULL);
	
	seek_async_callback (instance, seekToTime);
}

void 
ExternalDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *mediaStreamDescription)
{
	g_return_if_fail (switch_media_stream_async_callback != NULL);
	g_return_if_fail (mediaStreamDescription != NULL);
	
	switch_media_stream_async_callback (instance, mediaStreamDescription);
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
	this->codec_id = codec_id;
	this->codec = CreateCodec (codec_id);
	this->extra_data = extra_data;
	this->extra_data_size = extra_data_size;
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
		ExternalDecoder_DtorCallback dtor)
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
	GetStream ()->SetOutputEnded (true);
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
