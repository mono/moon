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
#include "playlist.h"

#define VIDEO_BITRATE_PERCENTAGE 75
#define AUDIO_BITRATE_PERCENTAGE 25

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
	g_return_if_fail (Media::InMediaThread ());
	
	result = reader->Seek (pts);
	
	if (MEDIA_SUCCEEDED (result)) {
		LOG_PIPELINE ("ASFDemuxer:Seek (%" G_GUINT64_FORMAT "): seek completed, reporting it\n", pts);
		ReportSeekCompleted (pts);
	} else if (result == MEDIA_NOT_ENOUGH_DATA) {
		LOG_PIPELINE ("ASFDemuxer:Seek (%" G_GUINT64_FORMAT "): not enough data\n", pts);
		EnqueueSeek ();
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
	Media *media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
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
	media->unref ();
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
	Media *media = GetMediaReffed ();
	int current_stream = 1;
	int stream_count = 0;
	int count;
	
	g_return_val_if_fail (media != NULL, MEDIA_FAIL);
	
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
	
	SetIsDrm (asf_parser->IsDrm ());

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
			AudioStream* audio = new AudioStream (media);

			stream = audio;
			
			const WAVEFORMATEX* wave = stream_properties->get_audio_data ();
			if (wave == NULL) {
				result = MEDIA_INVALID_STREAM;
				Media::Warning (result, "Couldn't find audio data in the file.");
				goto failure;
			}

			const WAVEFORMATEXTENSIBLE* wave_ex = wave->get_wave_format_extensible ();
			int data_size = stream_properties->size - sizeof (asf_stream_properties) - sizeof (WAVEFORMATEX);
			
			audio->SetChannels (wave->channels);
			audio->SetSampleRate (wave->samples_per_second);
			audio->SetBitRate (wave->bytes_per_second * 8);
			audio->SetBlockAlign (wave->block_alignment);
			audio->SetBitsPerSample (wave->bits_per_sample);
			audio->SetExtraData (NULL);
			audio->SetExtraDataSize (data_size > wave->codec_specific_data_size ? wave->codec_specific_data_size : data_size);
			audio->SetCodecId (wave->codec_id);
			
			if (wave_ex != NULL) {
				audio->SetBitsPerSample (wave_ex->Samples.valid_bits_per_sample);
				audio->SetExtraDataSize (audio->GetExtraDataSize () - (sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX)));
				audio->SetCodecId (*((guint32*) &wave_ex->sub_format));
			}

			// Fill in any extra codec data
			if (audio->GetExtraDataSize () > 0) {
				audio->SetExtraData (g_malloc0 (audio->GetExtraDataSize ()));
				char* src = ((char*) wave) + (wave_ex ? sizeof (WAVEFORMATEX) : sizeof (WAVEFORMATEX));
				memcpy (audio->GetExtraData (), src, audio->GetExtraDataSize ());
			}
		} else if (stream_properties->is_video ()) {
			VideoStream* video = new VideoStream (media);
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
			MarkerStream* marker = new MarkerStream (media);
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
	
	for (int i = 0; i < stream_count; i++)
		streams [i]->unref ();
	
	this->stream_to_asf_index = stream_to_asf_index;
	this->parser = asf_parser;
	
	reader = new ASFReader (parser, this);
			
	ReadMarkers ();
	
	media->unref ();
	
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
	
	media->unref ();
	
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

MediaResult
ASFDemuxer::GetFrameCallback (MediaClosure *c)
{
	MediaGetFrameClosure *closure = (MediaGetFrameClosure *) c;
	ASFDemuxer *demuxer = (ASFDemuxer *) closure->GetDemuxer ();
	demuxer->GetFrameAsyncInternal (closure->GetStream ());
	
	return MEDIA_SUCCESS;
}

void
ASFDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	//printf ("ASFDemuxer::ReadFrame (%p).\n", frame);
	ASFFrameReader *reader = NULL;
	MediaFrame *frame;
	MediaResult result;
	
	g_return_if_fail (this->reader != NULL);
	
	reader = this->reader->GetFrameReader (stream_to_asf_index [stream->index]);

	g_return_if_fail (reader != NULL);
	
	result = reader->Advance ();
	
	if (result == MEDIA_NO_MORE_DATA) {
		ReportGetFrameCompleted (NULL);
		return;
	}

	if (result == MEDIA_BUFFER_UNDERFLOW || result == MEDIA_NOT_ENOUGH_DATA) {
		Media *media = GetMediaReffed ();
		g_return_if_fail (media != NULL);
		MediaClosure *closure = new MediaGetFrameClosure (media, GetFrameCallback, this, stream);
		media->EnqueueWork (closure, false); // TODO: use a timeout here, no need to try again immediately.
		closure->unref ();
		media->unref ();
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
		frame->AddState (MediaFrameKeyFrame);
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
	
	frame->AddState (MediaFrameDemuxed);
	
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

	LOG_PIPELINE_ASF ("ASFDemuxerInfo::Supports (%p) pos: %" G_GINT64_FORMAT ", avail pos: %" G_GINT64_FORMAT "\n", source, source->GetPosition (), source->GetLastAvailablePosition ());

#if DEBUG
	bool eof = false;
	if (!source->GetPosition () == 0)
		fprintf (stderr, "ASFDemuxerInfo::Supports (%p): Trying to check if a media is supported, but the media isn't at position 0 (it's at position %" G_GINT64_FORMAT ")\n", source, source->GetPosition ());
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
 * MmsSource
 */
 
MmsSource::MmsSource (Media *media, Downloader *downloader)
	: IMediaSource (Type::MMSSOURCE, media)
{
	finished = false;
	write_count = 0;
	this->downloader = NULL;
	current = NULL;
	demuxer = NULL;
	
	g_return_if_fail (downloader != NULL);
	g_return_if_fail (downloader->GetInternalDownloader () != NULL);
	g_return_if_fail (downloader->GetInternalDownloader ()->GetObjectType () == Type::MMSDOWNLOADER);
	
	this->downloader = downloader;
	this->downloader->ref ();
	
	ReportStreamChange (0); // create the initial MmsPlaylistEntry
}

void
MmsSource::Dispose ()
{
	// thread safe method
	
	MmsPlaylistEntry *entry;
	IMediaDemuxer *demux;
	Downloader *dl;
		
	// don't lock during unref, only while nulling out the local field
	Lock ();
	entry = this->current;
	this->current = NULL;
	dl = this->downloader;
	this->downloader = NULL;
	demux = this->demuxer;
	this->demuxer = NULL;
	Unlock ();
	
	if (dl) {
		dl->RemoveAllHandlers (this);
		dl->unref ();
	}
	
	if (entry)
		entry->unref ();
		
	if (demux)
		demux->unref ();
	
	IMediaSource::Dispose ();
}

MediaResult
MmsSource::Initialize ()
{
	Downloader *dl;
	MmsDownloader *mms_dl;
	
	VERIFY_MAIN_THREAD;
	
	dl = GetDownloaderReffed ();
	
	g_return_val_if_fail (dl != NULL, MEDIA_FAIL);
	g_return_val_if_fail (!dl->Started (), MEDIA_FAIL);
	
	// We must call MmsDownloader::SetSource before the downloader
	// has actually received any data. Here we rely on the fact that
	// firefox needs a tick before returning any data.
	mms_dl = GetMmsDownloader (dl);
	if (mms_dl != NULL) {
		mms_dl->SetSource (this);
	} else {
		printf ("MmsSource::Initialize (): Could not get the MmsDownloader. Media won't play.\n");
	}
	
	dl->AddHandler (Downloader::DownloadFailedEvent, DownloadFailedCallback, this);
	dl->AddHandler (Downloader::CompletedEvent, DownloadCompleteCallback, this);
	dl->Send ();
	
	dl->unref ();
	
	return MEDIA_SUCCESS;
}

MmsDemuxer *
MmsSource::GetDemuxerReffed ()
{
	MmsDemuxer *result;
	Lock ();
	result = demuxer;
	if (result)
		result->ref ();
	Unlock ();
	return result;
}

Downloader *
MmsSource::GetDownloaderReffed ()
{
	Downloader *result;
	Lock ();
	result = downloader;
	if (downloader)
		downloader->ref ();
	Unlock ();
	return result;
}

MmsPlaylistEntry *
MmsSource::GetCurrentReffed ()
{
	MmsPlaylistEntry *result;
	
	// Thread safe
	
	Lock ();
	result = current;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

void
MmsSource::ReportDownloadFailure ()
{
	Media *media;
	
	LOG_MMS ("MmsSource::ReportDownloadFailure ()\n");
	VERIFY_MAIN_THREAD;
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	media->ReportErrorOccurred ("MmsDownloader failed");
	media->unref ();
}

void
MmsSource::ReportStreamChange (gint32 reason)
{
	Media *media;
	PlaylistRoot *root;
	Media *entry_media;
	
	LOG_MMS ("MmsSource::ReportStreamChange (reason: %i)\n", reason);
	
	VERIFY_MAIN_THREAD;
	
	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	root = media->GetPlaylistRoot ();

	g_return_if_fail (root != NULL);
		
	Lock ();
	if (current != NULL) {
		current->NotifyFinished ();
		current->unref ();
	}
		
	entry_media = new Media (root);
	current = new MmsPlaylistEntry (entry_media, this);
	entry_media->unref ();
	Unlock ();
	
	media->unref ();
}

void
MmsSource::SetMmsMetadata (const char *playlist_gen_id, const char *broadcast_id, HttpStreamingFeatures features)
{
	MmsPlaylistEntry *entry;
	
	LOG_MMS ("MmsSource::SetMmsMetadata ('%s', '%s', %i)\n", playlist_gen_id, broadcast_id, (int) features);
	
	VERIFY_MAIN_THREAD;
	
	entry = GetCurrentReffed ();
	
	g_return_if_fail (entry != NULL);
	
	entry->SetPlaylistGenId (playlist_gen_id);
	entry->SetBroadcastId (broadcast_id);
	entry->SetHttpStreamingFeatures (features);

	entry->unref ();
}

void
MmsSource::DownloadFailedHandler (Downloader *dl, EventArgs *args)
{
	Media *media = GetMediaReffed ();
	ErrorEventArgs *eea;
	VERIFY_MAIN_THREAD;
	
	g_return_if_fail (media != NULL);
	eea = new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR"));
	media->RetryHttp (eea);
	media->unref ();
	eea->unref ();
}

void
MmsSource::DownloadCompleteHandler (Downloader *dl, EventArgs *args)
{
	VERIFY_MAIN_THREAD;
}

void
MmsSource::NotifyFinished (guint32 reason)
{
	VERIFY_MAIN_THREAD;
	
	LOG_MMS ("MmsSource::NotifyFinished (%i)\n", reason);
	
	if (reason == 0) {
		// The server has finished streaming and no more 
		// Data packets will be transmitted until the next Play request
		finished = true;
	} else if (reason == 1) {
		// The server has finished streaming the current playlist entry. Other playlist
		// entries still remain to be streamed. The server will transmit a stream change packet
		// when it switches to the next entry.
		MmsPlaylistEntry *entry = GetCurrentReffed ();
		entry->NotifyFinished ();
		entry->unref ();
	} else {
		// ?
	}
}

MediaResult
MmsSource::SeekToPts (guint64 pts)
{
	// thread safe
	MediaResult result = true;
	Downloader *dl;
	MmsDownloader *mms_dl;
	
	LOG_PIPELINE_ASF ("MmsSource::SeekToPts (%" G_GUINT64_FORMAT ")\n", pts);

	dl = GetDownloaderReffed ();
	
	g_return_val_if_fail (dl != NULL, MEDIA_FAIL);

	mms_dl = GetMmsDownloader (dl);
	
	if (mms_dl) {
		mms_dl->SetRequestedPts (pts);
		finished = false;
		result = MEDIA_SUCCESS;
	} else {
		result = MEDIA_FAIL;
	}
	
	dl->unref ();
	
	return result;
}

MmsDownloader *
MmsSource::GetMmsDownloader (Downloader *dl)
{
	InternalDownloader *idl;
	
	g_return_val_if_fail (dl != NULL, NULL);
	
	idl = dl->GetInternalDownloader ();
	
	if (idl == NULL)
		return NULL;
	
	if (idl->GetObjectType () != Type::MMSDOWNLOADER)
		return NULL;
	
	return (MmsDownloader *) idl;
}

IMediaDemuxer *
MmsSource::CreateDemuxer (Media *media)
{
	// thread safe
	MmsDemuxer *result = NULL;
	
	g_return_val_if_fail (demuxer == NULL, NULL);
	
	Lock ();
	if (demuxer == NULL) {
		result = new MmsDemuxer (media, this);
		demuxer = result;
		demuxer->ref ();
	}
	Unlock ();
	
	return result;
}

void
MmsSource::WritePacket (void *buf, gint32 n)
{
	MmsPlaylistEntry *entry;
	
	VERIFY_MAIN_THREAD;
	
	entry = GetCurrentReffed ();
	
	g_return_if_fail (entry != NULL);
	
	entry->WritePacket (buf, n);
	entry->unref ();
}

ASFPacket *
MmsSource::Pop ()
{
	MmsPlaylistEntry *entry;
	ASFPacket *result;
	
	entry = GetCurrentReffed ();
	
	g_return_val_if_fail (entry != NULL, NULL);
	
	result = entry->Pop ();
	
	entry->unref ();
	
	return result;
}

bool
MmsSource::Eof ()
{
	// thread safe
	MmsPlaylistEntry *entry;
	bool result;
	
	if (!finished)
		return false;
	
	entry = GetCurrentReffed ();
	 	
	if (entry == NULL) {
	 	result = true;
	 } else {
		result = entry->Eof ();
		entry->unref ();
	}
	
	return result;
}

/*
 * MmsPlaylistEntry
 */

MmsPlaylistEntry::MmsPlaylistEntry (Media *media, MmsSource *source)
	: IMediaSource (Type::MMSPLAYLISTENTRY, media)
{
	finished = false;
	parent = source;
	parser = NULL;
	write_count = 0;
	demuxer = NULL;
	playlist_gen_id = NULL;
	broadcast_id = NULL;
	features = HttpStreamingFeaturesNone;
	
	g_return_if_fail (parent != NULL);
	parent->ref ();
}

MediaResult
MmsPlaylistEntry::Initialize ()
{
	return MEDIA_SUCCESS;
}

void
MmsPlaylistEntry::Dispose ()
{
	// thread safe
	MmsSource *mms_source;
	ASFParser *asf_parser;
	IMediaDemuxer *demux;
	
	Lock ();
	mms_source = this->parent;
	this->parent = NULL;
	asf_parser = this->parser;
	this->parser = NULL;
	demux = this->demuxer;
	this->demuxer = NULL;
	g_free (playlist_gen_id);
	playlist_gen_id = NULL;
	g_free (broadcast_id);
	broadcast_id = NULL;
	Unlock ();
	
	if (mms_source != NULL)
		mms_source->unref ();
	
	if (asf_parser != NULL)
		asf_parser->unref ();
		
	if (demux != NULL)
		demux->unref ();
	
	queue.Clear (true);
	// This is a bit weird - in certain
	// we can end up with a circular dependency between
	// Media and MmsPlaylistEntry, where Media::Dispose
	// isn't called. So if Media::Dispose hasn't been
	// called, do it here, and only do it after our
	// instance copy of the media is cleared out to 
	// prevent infinite loops.
	Media *m = GetMediaReffed ();
	
	IMediaSource::Dispose ();
	
	if (m != NULL) {
		if (!m->IsDisposed ())
			m->Dispose ();
		m->unref ();
	}
}

MediaResult
MmsPlaylistEntry::SeekToPts (guint64 pts)
{
	MmsSource *ms = GetParentReffed ();
	if (ms) {
		ms->SeekToPts (pts);
		ms->unref ();
		queue.Clear (true);
		return MEDIA_SUCCESS;
	} else {
		fprintf (stderr, "MmsPlaylistEntry::SeekToPts (%" G_GUINT64_FORMAT "): Could not seek to pts, no parent.\n", pts);
		return MEDIA_FAIL;
	}
}

void
MmsPlaylistEntry::NotifyFinished ()
{
	finished = true;
}

void
MmsPlaylistEntry::GetSelectedStreams (gint64 max_bitrate, gint8 streams [128])
{
	ASFParser *parser;
	asf_file_properties *properties;
	gint32 audio_bitrates [128];
	gint32 video_bitrates [128];
	
	memset (audio_bitrates, 0xff, 128 * 4);
	memset (video_bitrates, 0xff, 128 * 4);
	memset (streams, 0xff, 128); 
	
	parser = GetParserReffed ();
	
	g_return_if_fail (parser != NULL);
	
	properties = parser->GetFileProperties ();
	
	g_return_if_fail (properties != NULL);
	
	for (int i = 1; i < 127; i++) {
		int current_stream;
		if (!parser->IsValidStream (i)) {
			streams [i] = -1; // inexistent
			continue;
		}
		streams [i] = 0; // disabled
		current_stream = i;

		const asf_stream_properties *stream_properties = parser->GetStream (current_stream);
		const asf_extended_stream_properties *extended_stream_properties = parser->GetExtendedStream (current_stream);

		if (stream_properties == NULL) {
			printf ("MmsPlaylistEntry::GetSelectedStreams (): stream #%i doesn't have any stream properties.\n", current_stream);
			continue;
		}

		if (stream_properties->is_audio ()) {
			const WAVEFORMATEX* wave = stream_properties->get_audio_data ();
			audio_bitrates [current_stream] = wave->bytes_per_second * 8;
		} else if (stream_properties->is_video ()) {
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

			video_bitrates [current_stream] = bit_rate;
		} else if (stream_properties->is_command ()) {
			// we select all marker streams
			streams [current_stream] = 1;
		}
	}
	
	// select the video stream
	int video_stream = 0;
	int video_rate = 0;
	
	for (int i = 0; i < 128; i++) {
		int stream_rate = video_bitrates [i];

		if (stream_rate == -1)
			continue;

		if (video_rate == 0) {
			video_rate = stream_rate;
			video_stream = i;
		}

		if (stream_rate > video_rate && stream_rate < (max_bitrate * VIDEO_BITRATE_PERCENTAGE)) {
			video_rate = stream_rate;
			video_stream = i;
		}
	}
	streams [video_stream] = 1; // selected		
	LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams (): Selected video stream %i of rate %i\n", video_stream, video_rate);

	
	// select audio stream
	int audio_stream = 0;
	int audio_rate = 0;
	
	for (int i = 0; i < 128; i++) {
		int stream_rate = audio_bitrates [i];

		if (stream_rate == -1)
			continue;

		if (audio_rate == 0) {
			audio_rate = stream_rate;
			audio_stream = i;
		}

		if (stream_rate > audio_rate && stream_rate < (max_bitrate * AUDIO_BITRATE_PERCENTAGE)) {
			audio_rate = stream_rate;
			audio_stream = i;
		}
	}
	streams [audio_stream] = 1; // selected
	LOG_MMS ("MmsPlaylistEntry::GetSelectedStreams (): Selected audio stream %i of rate %i\n", audio_stream, audio_rate);
	
	parser->unref ();
}

bool
MmsPlaylistEntry::IsHeaderParsed ()
{
	bool result;
	Lock ();
	result = parser != NULL;
	Unlock ();
	return result;
}

IMediaDemuxer *
MmsPlaylistEntry::GetDemuxerReffed ()
{
	// thread safe
	IMediaDemuxer *result;
	
	Lock ();
	result = demuxer;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

ASFParser *
MmsPlaylistEntry::GetParserReffed ()
{
	// thread safe
	ASFParser *result;
	
	Lock ();
	result = parser;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

MmsSource *
MmsPlaylistEntry::GetParentReffed ()
{
	// thread safe
	MmsSource *result;
	
	Lock ();
	result = parent;
	if (result)
		result->ref ();
	Unlock ();
	
	return result;
}

IMediaDemuxer *
MmsPlaylistEntry::CreateDemuxer (Media *media)
{
	// thread safe
	ASFDemuxer *result;
	ASFParser *asf_parser;
	
	asf_parser = GetParserReffed ();
	
	g_return_val_if_fail (media != NULL, NULL);
	g_return_val_if_fail (asf_parser != NULL, NULL);
	g_return_val_if_fail (demuxer == NULL, NULL);
	
	result = new ASFDemuxer (media, this);
	result->SetParser (asf_parser);
	result->SetIsDrm (asf_parser->IsDrm ());

	Lock ();
	if (demuxer != NULL)
		demuxer->unref ();
	demuxer = result;
	demuxer->ref ();
	Unlock ();
	
	asf_parser->unref ();
	
	return result;
}

void 
MmsPlaylistEntry::SetPlaylistGenId (const char *value)
{
	// thread safe
	Lock ();
	g_free (playlist_gen_id);
	playlist_gen_id = g_strdup (value);
	Unlock ();
}

char *
MmsPlaylistEntry::GetPlaylistGenId ()
{
	// thread safe
	char *result;
	Lock ();
	result = g_strdup (playlist_gen_id);
	Unlock ();
	return result;
}

void
MmsPlaylistEntry::SetBroadcastId (const char *value)
{
	// thread safe
	Lock ();
	g_free (broadcast_id);
	broadcast_id = g_strdup (value);
	Unlock ();
}

char *
MmsPlaylistEntry::GetBroadcastId ()
{
	// thread safe
	char *result;
	Lock ();
	result = g_strdup (broadcast_id);
	Unlock ();
	return result;
}

void 
MmsPlaylistEntry::SetHttpStreamingFeatures (HttpStreamingFeatures value)
{
	features = value;
}

HttpStreamingFeatures
MmsPlaylistEntry::GetHttpStreamingFeatures ()
{
	return features;
}

void
MmsPlaylistEntry::AddEntry ()
{
	VERIFY_MAIN_THREAD;
	
	Media *media = GetMediaReffed ();
	Playlist *playlist;
	PlaylistEntry *entry;
	MmsDemuxer *mms_demuxer = NULL;
	
	g_return_if_fail (media != NULL);
	
	if (parent == NULL)
		goto cleanup;
	
	mms_demuxer = parent->GetDemuxerReffed ();
	
	if (mms_demuxer == NULL)
		goto cleanup;
	
	playlist = mms_demuxer->GetPlaylist ();
	
	if (playlist == NULL)
		goto cleanup;
	
	entry = new PlaylistEntry (playlist);
	entry->SetIsLive (features & HttpStreamingBroadcast);
	
	playlist->AddEntry (entry);
	
	entry->InitializeWithSource (this);

cleanup:
	if (media)
		media->unref ();
	if (mms_demuxer)
		mms_demuxer->unref ();
}

MediaResult
MmsPlaylistEntry::ParseHeader (void *buffer, gint32 size)
{
	VERIFY_MAIN_THREAD;
	
	LOG_MMS ("MmsPlaylistEntry::ParseHeader (%p, %i)\n", buffer, size);
	
	MediaResult result;
	MemorySource *asf_src = NULL;
	Media *media;
	ASFParser *asf_parser;
	
	// this method shouldn't get called more than once
	g_return_val_if_fail (parser == NULL, MEDIA_FAIL);
	
	media = GetMediaReffed ();
	g_return_val_if_fail (media != NULL, MEDIA_FAIL);
	
	media->ReportDownloadProgress (1.0);
	
	asf_src = new MemorySource (media, buffer, size, 0, false);
	asf_parser = new ASFParser (asf_src, media);
	result = asf_parser->ReadHeader ();	
	asf_src->unref ();
	media->unref ();
	
	if (MEDIA_SUCCEEDED (result)) {
		Lock ();
		if (parser)
			parser->unref ();
		parser = asf_parser;
		Unlock ();
		AddEntry ();
	} else {
		asf_parser->unref ();
	}
	
	return result;
}

ASFPacket *
MmsPlaylistEntry::Pop ()
{
	// thread safe
	//LOG_MMS ("MmsSource::Pop (), there are %i packets in the queue, of a total of %" G_GINT64_FORMAT " packets written.\n", queue.Length (), write_count);
	
	QueueNode *node;
	ASFPacket *result = NULL;
	ASFParser *parser;
	
trynext:
	node = (QueueNode *) queue.Pop ();
	
	if (node == NULL) {
		LOG_PIPELINE_ASF ("MmsSource::Pop (): No more packets (for now).\n");
		return NULL;
	}
	
	parser = GetParserReffed ();
	
	if (node->packet == NULL) {
		if (parser == NULL) {
			g_warning ("MmsSource::Pop (): No parser to parse the packet.\n");
			goto cleanup;
		}
		node->packet = new ASFPacket (parser, node->source);
		if (!MEDIA_SUCCEEDED (node->packet->Read ())) {
			LOG_PIPELINE_ASF ("MmsSource::Pop (): Error while parsing packet, getting a new packet\n");
			delete node;
			goto trynext;
		}
	}
	
	result = node->packet;
	result->ref ();

cleanup:				
	delete node;
	
	if (parser)
		parser->unref ();
	
	LOG_PIPELINE_ASF ("MmsSource::Pop (): popped 1 packet, there are %i packets left, of a total of %" G_GINT64_FORMAT " packets written\n", queue.Length (), write_count);
	
	return result;
}

bool
MmsPlaylistEntry::IsFinished ()
{
	bool result;
	return parent->IsFinished (); // REMOVE
	MmsSource *src = GetParentReffed ();
	
	g_return_val_if_fail (src != NULL, true);
	
	result = src->IsFinished ();
	src->unref ();
	
	return result;
}

void
MmsPlaylistEntry::WritePacket (void *buf, gint32 n)
{
	MemorySource *src;
	ASFPacket *packet;
	ASFParser *asf_parser;
	Media *media;
	bool added = false;
	
	LOG_PIPELINE_ASF ("MmsPlaylistEntry::WritePacket (%p, %i), write_count: %" G_GINT64_FORMAT "\n", buf, n, write_count + 1);
	VERIFY_MAIN_THREAD;

	media = GetMediaReffed ();
	
	g_return_if_fail (media != NULL);

	write_count++;
	
	asf_parser = GetParserReffed ();
	
	if (asf_parser != NULL) {
		src = new MemorySource (media, buf, n, 0, false);
		packet = new ASFPacket (asf_parser, src);
		if (!MEDIA_SUCCEEDED (packet->Read ())) {
			LOG_PIPELINE_ASF ("MmsPlaylistEntry::WritePacket (%p, %i): Error while parsing packet, dropping packet.\n", buf, n);
		} else {
			queue.Push (new QueueNode (packet));
			added = true;
		}
		packet->unref ();
		src->unref ();
	} else {
		src = new MemorySource (media, g_memdup (buf, n), n, 0);
		queue.Push (new QueueNode (src));
		src->unref ();
		added = true;
	}
	
	if (added) {
		IMediaDemuxer *demuxer = GetDemuxerReffed ();
		if (demuxer) {
			demuxer->FillBuffers ();
			demuxer->unref ();
		}		
	}
	if (asf_parser)
		asf_parser->unref ();
		
	if (media)
		media->unref ();
}

/*
 * MmsPlaylistEntry::QueueNode
 */

MmsPlaylistEntry::QueueNode::QueueNode (MemorySource *source)
{
	if (source)
		source->ref ();
	this->source = source;
	packet = NULL;
}

MmsPlaylistEntry::QueueNode::QueueNode (ASFPacket *packet)
{
	if (packet)
		packet->ref ();
	this->packet = packet;
	source = NULL;
}

MmsPlaylistEntry::QueueNode::~QueueNode ()
{
	if (packet)
		packet->unref ();
	if (source)
		source->unref ();
}

/*
 * MmsDemuxer
 */

MmsDemuxer::MmsDemuxer (Media *media, MmsSource *source)
	: IMediaDemuxer (Type::MMSDEMUXER, media, source)
{
	playlist = NULL;
	mms_source = source;
	if (mms_source)
		mms_source->ref ();
}

void 
MmsDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	printf ("MmsDemuxer::GetFrameAsyncInternal (%p): This method should never be called.\n", stream);
}

void 
MmsDemuxer::OpenDemuxerAsyncInternal ()
{
	PlaylistRoot *root;
	Media *media;
	
	LOG_MMS ("MmsDemuxer::OpenDemuxerAsyncInternal ().\n");
	
	media = GetMediaReffed ();
	root = media ? media->GetPlaylistRoot () : NULL;
	
	g_return_if_fail (playlist == NULL);
	g_return_if_fail (media != NULL);
	g_return_if_fail (root != NULL);
	
	playlist = new Playlist (root, mms_source);
	ReportOpenDemuxerCompleted ();
	media->unref ();
}

MediaResult
MmsDemuxer::SeekInternal (guint64 pts)
{
	g_warning ("MmsDemuxer::SeekInternal (%" G_GINT64_FORMAT "): You hit a bug in moonlight, please attach gdb, get a stack trace and file bug.", pts);
	print_stack_trace ();

	return MEDIA_FAIL;
}

void 
MmsDemuxer::SeekAsyncInternal (guint64 seekToTime)
{
	printf ("MmsDemuxer::SeekAsyncInternal (%" G_GUINT64_FORMAT "): Not implemented.\n", seekToTime);
}

void 
MmsDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	printf ("MmsDemuxer::SwitchMediaStreamAsyncInternal (%p): Not implemented.\n", stream);
}
	
void 
MmsDemuxer::Dispose ()
{
	Playlist *pl;
	MmsSource *src;
	
	mutex.Lock ();
	pl = this->playlist;
	this->playlist = NULL;
	src = this->mms_source;
	this->mms_source = NULL;
	mutex.Unlock ();
	
	if (pl)
		pl->unref ();
		
	if (src)
		src->unref ();
	
	IMediaDemuxer::Dispose ();
}
	
