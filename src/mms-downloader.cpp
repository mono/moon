/*
 * mms-downloader.cpp: MMS Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "mms-downloader.h"
#include "debug.h"
#include "timemanager.h"
#include "utils.h"

#define CLIENT_GUID "{c77e7400-738a-11d2-9add-0020af0a3278}"
#define CLIENT_SUPPORTED "com.microsoft.wm.srvppair, com.microsoft.wm.sswitch, com.microsoft.wm.startupprofile, com.microsoft.wm.predstrm"
#define CLIENT_USER_AGENT "NSPlayer/11.08.0005.0000"

static inline bool
is_valid_mms_header (MmsHeader *header)
{
	if (header->id != MMS_DATA && header->id != MMS_HEADER && header->id != MMS_METADATA && header->id != MMS_STREAM_C && header->id != MMS_END && header->id != MMS_PAIR_P)
		return false;

	return true;
}

/*
 * MmsDownloader
 */
MmsDownloader::MmsDownloader (Downloader *dl) : InternalDownloader (dl, Type::MMSDOWNLOADER)
{
	LOG_MMS ("MmsDownloader::MmsDownloader ()\n");

	uri = NULL;
	buffer = NULL;
	client_id = NULL;
	playlist_gen_id = NULL;

	size = 0;

	p_packet_count = 0;

	p_packet_times [0] = 0;
	p_packet_times [1] = 0;
	p_packet_times [2] = 0;
	
	max_bitrate = 0;
	is_playing = false;
	stream_switched = false;

	source = NULL;
	content_descriptions = NULL;

	requested_pts = 0;
	failure_reported = false;

	dl->SetRequireCustomHeaderSupport (true);
	dl->SetDisableCache (true);
}

MmsDownloader::~MmsDownloader ()
{
	LOG_MMS ("MmsDownloader::~MmsDownloader ()\n");

	g_free (uri);
	g_free (buffer);
	g_free (client_id);
	g_free (playlist_gen_id);
	delete content_descriptions;

	if (source)
		source->unref ();
}

void
MmsDownloader::SetSource (MmsSource *source)
{
	VERIFY_MAIN_THREAD;

	if (this->source)
		this->source->unref ();
	this->source = source;
	if (this->source)
		this->source->ref ();
}

void
MmsDownloader::SetRequestedPts (guint64 value)
{
	// thread safe
	LOG_MMS ("MmsDownloader::SetRequestedPts (%" G_GUINT64_FORMAT ")\n", value);

	request_mutex.Lock ();
	requested_pts = value;
	request_mutex.Unlock ();

	AddTickCallSafe (PlayCallback);
}

guint64
MmsDownloader::GetRequestedPts ()
{
	// thread safe
	guint64 result;

	request_mutex.Lock ();
	result = requested_pts;
	request_mutex.Unlock ();

	LOG_MMS ("MmsDownloader::GetRequestedPts (): %" G_GUINT64_FORMAT "\n", result);

	return result;
}

static void
set_common_dl_headers (Downloader *dl, MmsDownloader *mms, GString *pragma)
{
	dl->InternalSetHeader ("User-Agent", CLIENT_USER_AGENT);
	dl->InternalSetHeader ("Pragma", "no-cache");
	dl->InternalSetHeader ("Pragma", "xClientGUID=" CLIENT_GUID);
	dl->InternalSetHeader ("Supported", CLIENT_SUPPORTED);

	if (pragma != NULL && mms != NULL) {
		const char *playlist_gen_id = mms->GetPlaylistGenId ();
		const char *client_id = mms->GetClientId ();

#ifdef HAVE_CURL
		if (moonlight_flags & RUNTIME_INIT_CURL_BRIDGE) {
			if (playlist_gen_id != NULL) {
				g_string_printf (pragma, "playlist-gen-id=%s", playlist_gen_id);
				dl->InternalSetHeader ("Pragma", pragma->str);
			}
			if (client_id != NULL) {
				g_string_printf (pragma, "client-id=%s", client_id);
				dl->InternalSetHeader ("Pragma", pragma->str);
			}
		} else {
#endif

		if (playlist_gen_id != NULL)
			g_string_append_printf (pragma, "Pragma: playlist-gen-id=%s\r\n", playlist_gen_id);
		if (client_id != NULL)
			g_string_append_printf (pragma, "Pragma: client-id=%s\r\n", client_id);

#ifdef HAVE_CURL
		}
#endif

	}
}

static void
set_stream_selection_headers (Downloader *dl, MmsDownloader *mms, GString *pragma, MmsPlaylistEntry *entry)
{
	gint8 streams [128];
	int count = 0;

	/* 
	 * stream-switch-count && stream-switch-entry need to be on their own pragma lines
	 * we (ab)use SetBody for this
	 */

	g_return_if_fail (mms != NULL);
	g_return_if_fail (pragma != NULL);
	g_return_if_fail (entry != NULL);

	entry->GetSelectedStreams (mms->GetMaxBitrate (), streams);

	GString *line = g_string_new (NULL);
	g_string_printf (line, "stream-switch-entry=");
	for (int i = 1; i < 128; i++) {
		switch (streams [i]) {
		case -1: // invalid stream
			break;
		case 0: // not selected
			count++;
			g_string_append_printf (line, "%i:ffff:0 ", i);
			break;
		case 1: // selected
			count++;
			g_string_append_printf (line, "ffff:%i:0 ", i);
			break;
		default: // ?
			printf ("MmsDownloader: invalid stream selection value (%i).\n", streams [i]);
			break;
		}
	}


#ifdef HAVE_CURL
	if (moonlight_flags & RUNTIME_INIT_CURL_BRIDGE) {
		dl->InternalSetHeader ("Pragma", line->str);
		g_string_printf (line, "stream-switch-count=%i", count);
		dl->InternalSetHeader ("Pragma", line->str);
	} else {
#endif

		g_string_append_printf (pragma, "Pragma: %s\r\n", line->str);
		g_string_append_printf (pragma, "Pragma: stream-switch-count=%i\r\n", count);

#ifdef HAVE_CURL
	}
#endif

	g_string_free (line, true);
}

void
MmsDownloader::Open (const char *verb, const char *uri)
{
	int offset = 0;
	
	LOG_MMS ("MmsDownloader::Open ('%s', '%s')\n", verb, uri);

	VERIFY_MAIN_THREAD;

	g_return_if_fail (this->uri == NULL);
	g_return_if_fail (uri != NULL);
	
	if (strncmp (uri, "mms://", 6) == 0) {
		offset = 6;
	} else if (strncmp (uri, "rtsp://", 7) == 0) {
		offset = 7;
	} else if (strncmp (uri, "rtsps://", 8) == 0) {
		offset = 8;
	} else {
		fprintf (stderr, "Moonlight: streaming scheme must be either mms, rtsp or rtsps, got uri: %s\n", uri);
		return;
	}

	this->uri = g_strdup_printf ("http://%s", uri + offset);

	dl->OpenInitialize ();
	dl->SetRequireCustomHeaderSupport (true);
	dl->SetDisableCache (true);
	dl->InternalOpen (verb, this->uri);

	set_common_dl_headers (dl, this, NULL);
	dl->InternalSetHeader ("Pragma", "packet-pair-experiment=1");
	dl->SetResponseHeaderCallback (ProcessResponseHeaderCallback, this);
}

void
MmsDownloader::PlayCallback (EventObject *sender)
{
	((MmsDownloader *) sender)->Play ();
}

void
MmsDownloader::Play ()
{
	GString *pragma;
	MmsPlaylistEntry *entry;
	guint64 pts;

	request_mutex.Lock ();
	pts = requested_pts;
	requested_pts = 0;
	request_mutex.Unlock ();

	LOG_MMS ("MmsDownloader::Play () requested_pts: %" G_GUINT64_FORMAT "\n", pts);

	g_return_if_fail (source != NULL);

	g_free (buffer);
	buffer = NULL;
	size = 0;

	entry = source->GetCurrentReffed ();

	g_return_if_fail (entry != NULL);

	dl->InternalAbort ();

	dl->OpenInitialize ();
	dl->SetRequireCustomHeaderSupport (true);
	dl->SetDisableCache (true);
	dl->InternalOpen ("GET", uri);

	pragma = g_string_new (NULL);

	set_common_dl_headers (dl, this, pragma);

#ifdef HAVE_CURL
	if (moonlight_flags & RUNTIME_INIT_CURL_BRIDGE) {

		dl->InternalSetHeader ("Pragma", "rate=1.000000,stream-offset=0:0,max-duration=0");
		dl->InternalSetHeader ("Pragma", "xPlayStrm=1");
		dl->InternalSetHeader ("Pragma", "LinkBW=2147483647,rate=1.000, AccelDuration=20000, AccelBW=2147483647");

		g_string_printf (pragma, "stream-time=%" G_GINT64_FORMAT ", packet-num=4294967295", pts / 10000);
		dl->InternalSetHeader ("Pragma", pragma->str);

		set_stream_selection_headers (dl, this, pragma, entry);
	} else {
#endif
		g_string_append (pragma, "Pragma: rate=1.000000,stream-offset=0:0,max-duration=0\r\n");
		g_string_append (pragma, "Pragma: xPlayStrm=1\r\n");
		g_string_append (pragma, "Pragma: LinkBW=2147483647,rate=1.000, AccelDuration=20000, AccelBW=2147483647\r\n");
		g_string_append_printf (pragma, "Pragma: stream-time=%" G_GINT64_FORMAT ", packet-num=4294967295\r\n", pts / 10000);

		set_stream_selection_headers (dl, this, pragma, entry);

		g_string_append_printf (pragma, "\r\n"); // end of header

		dl->InternalSetBody (pragma->str, pragma->len);

#ifdef HAVE_CURL
	}
#endif

	dl->Send ();

	g_string_free (pragma, true);
	entry->unref ();

	is_playing = true;
}

void
MmsDownloader::ProcessResponseHeaderCallback (gpointer context, const char *header, const char *value)
{
	MmsDownloader *dl = (MmsDownloader *) context;
	g_return_if_fail (dl != NULL);
	dl->SetCurrentDeployment ();
	dl->ProcessResponseHeader (header, value);
}

void
MmsDownloader::ProcessResponseHeader (const char *header, const char *value)
{
	char *h;
	char *duped;

	LOG_MMS ("MmsDownloader::ProcessResponseHeader ('%s', '%s')\n", header, value);

	if (failure_reported)
		return;

	// check response code
	DownloaderResponse *response = this->dl->GetResponse ();
	if (response != NULL && response->GetResponseStatus () != 200) {
		fprintf (stderr, "Moonlight: The MmsDownloader could not load the uri '%s', got response status: %i (expected 200)\n", uri, response->GetResponseStatus ());
		failure_reported = true;
		if (source)
			source->ReportDownloadFailure ();
		return;
	}

	g_return_if_fail (header != NULL);
	g_return_if_fail (value != NULL);

	// we're only interested in the 'Pragma' header(s)

	if (strcmp (header, "Pragma") != 0)
		return;

	h = g_strdup (value);
	duped = h;

	while (h != NULL && *h != 0) {
		char *key = NULL;
		char *val = NULL;
		char c;
		char *left;

		key = parse_rfc_1945_token (h, &c, &left);

		if (key == NULL)
			break;

		h = left;

		if (key [0] == 0)
			continue;

		if (c == '=' && h != NULL) {
			if (*h == '"') {
				val = parse_rfc_1945_quoted_string (h + 1, &c, &left);
				h = left;
			} else if (*h != 0) {
				val = parse_rfc_1945_token (h, &c, &left);
				h = left;
			}
		}

		// printf ("MmsDownloader::ResponseHeader (). processing 'Pragma', key='%s', value='%s'\n", key, val);

		if (strcmp (key, "client-id") == 0) {
			if (client_id != NULL)
				g_free (client_id);
			client_id = g_strdup (val);
		}
	}

	g_free (duped);
}

void
MmsDownloader::Write (void *buf, gint32 off, gint32 n)
{
	LOG_MMS ("MmsDownloader::Write (%p, %i, %i)\n", buf, off, n);

	MmsHeader *header;
	MmsPacket *packet;
	char *payload;
	guint32 offset = 0;

	// Resize our internal buffer
	if (buffer == NULL) {
		buffer = (char *) g_malloc (n);
	} else {
		buffer = (char *) g_realloc (buffer, size + n);
	}

	// Populate the data into the buffer
	memcpy (buffer + size, buf, n);
	size += n;

	// Check if we have an entire packet available.
	while (size >= sizeof (MmsHeader)) {
		header = (MmsHeader *) buffer;

		if (!is_valid_mms_header (header)) {
			LOG_MMS ("MmsDownloader::Write (): invalid mms header\n");
			dl->Abort ();
			dl->NotifyFailed ("invalid mms source");
			return;
		}

		if (size < (header->length + sizeof (MmsHeader)))
			return;

		packet = (MmsPacket *) (buffer + sizeof (MmsHeader));
		payload = (buffer + sizeof (MmsHeader) + sizeof (MmsDataPacket));

		if (!ProcessPacket (header, packet, payload, &offset)) {
			LOG_MMS ("MmsDownloader::Write (): packet processing failed\n");
			break;
		}

		if (size > offset) {
			// FIXME: We should refactor this to increment the buffer pointer to the new position
			// but coalense the free / malloc / memcpy into batches to improve performance on big
			// streams
			char *new_buffer = (char *) g_malloc (size - offset);
			memcpy (new_buffer, buffer + offset, size - offset);
			g_free (buffer);

			buffer = new_buffer;
			size -= offset;
		} else {
			g_free (buffer);
			buffer = NULL;
			size = 0;
		}
	}
}

char *
MmsDownloader::GetDownloadedFilename (const char *partname)
{
	LOG_MMS ("MmsDownloader::GetDownloadedFilename ('%s')\n", partname);
	return NULL;
}

char *
MmsDownloader::GetResponseText (const char *partname, gint64 *size)
{
	LOG_MMS ("MmsDownloader::GetResponseText ('%s', %p)\n", partname, size);
	return NULL;
}

bool
MmsDownloader::ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessPacket (%p, %p, %p, %p) length: %i\n", header, packet, payload, offset, header->length);
	
	*offset = (header->length + sizeof (MmsHeader));
 
 	switch (header->id) {
	case MMS_HEADER:
		return ProcessHeaderPacket (header, packet, payload, offset);
	case MMS_METADATA:
		return ProcessMetadataPacket (header, packet, payload, offset);
	case MMS_PAIR_P:
		return ProcessPairPacket (header, packet, payload, offset);
	case MMS_DATA:
		return ProcessDataPacket (header, packet, payload, offset);
	case MMS_END:
		return ProcessEndPacket (header, packet, payload, offset);
	case MMS_STREAM_C:
		return ProcessStreamSwitchPacket (header, packet, payload, offset);
	}

	printf ("MmsDownloader::ProcessPacket received a unknown packet type %i.", (int) header->id);

	return false;
}

bool
MmsDownloader::ProcessStreamSwitchPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessStreamSwitchPacket ()\n");
	VERIFY_MAIN_THREAD;

	MmsHeaderReason *hr = (MmsHeaderReason *) header;

	g_return_val_if_fail (source != NULL, false);

	source->ReportStreamChange (hr->reason);
	stream_switched = true;

	return true;
}

bool
MmsDownloader::ProcessEndPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessEndPacket ()\n");
	VERIFY_MAIN_THREAD;

	MmsHeaderReason *hr = (MmsHeaderReason *) header;

	g_return_val_if_fail (source != NULL, false);

	g_free (playlist_gen_id);
	playlist_gen_id = NULL;
	g_free (client_id);
	client_id = NULL;

	source->NotifyFinished (hr->reason);

	// TODO: send log

	return true;
}

MmsPlaylistEntry *
MmsDownloader::GetCurrentEntryReffed ()
{
	VERIFY_MAIN_THREAD;

	g_return_val_if_fail (source != NULL, NULL);

	return source->GetCurrentReffed ();
}

bool
MmsDownloader::ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	bool success = true;
	MmsPlaylistEntry *entry;
	MediaResult result;

	LOG_MMS ("MmsDownloader::ProcessHeaderPacket () is_playing: %i\n", is_playing);

	entry = GetCurrentEntryReffed ();

	g_return_val_if_fail (entry != NULL, false);

	if (!entry->IsHeaderParsed ()) {
		result = entry->ParseHeader (payload, header->length - sizeof (MmsDataPacket));

		if (!MEDIA_SUCCEEDED (result)) {
			LOG_MMS ("MmsDownloader::ProcessHeaderPacket (): failed to parse the asf header.\n");
			success = false;
		} else if (!is_playing) {
			Play ();
		} else if (stream_switched) {
			MmsSecondDownloader *sdl = new MmsSecondDownloader (this);
			sdl->SendStreamSwitch ();
			sdl->SetKillTimeout (30 /* seconds */);
			sdl->unref ();
		}
	} else {
		// We've already parsed this header (in the Describe request).
		// TODO: handle the xResetStream when the playlist changes
		// TODO: can this be another header??
	}

	entry->unref ();

	return success;
}

bool
MmsDownloader::ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (%p, %p, %s, %p)\n", header, packet, payload, offset);

	const char *playlist_gen_id = NULL;
	const char *broadcast_id = NULL;
	HttpStreamingFeatures features = HttpStreamingFeaturesNone;
	
	char *start = payload;
	char *key = NULL, *value = NULL;
	char *state = NULL;

	g_return_val_if_fail (source != NULL, false);
	
	// format: key=value,key=value\0
	// example:
	// playlist-gen-id=1,broadcast-id=2,features="broadcast,seekable"\0
	
	// Make sure payload is null-terminated
	for (int i = 0; i < packet->packet.data.size; i++) {
		if (payload [i] == 0)
			break;
		if (i == packet->packet.data.size - 1)
			payload [i] = NULL;
	}
	
	// content description list
	int payload_strlen = strlen (payload);
	const char *cdl_start = NULL;
	int cdl_length;

	if (content_descriptions != NULL) {
		delete content_descriptions;
		content_descriptions = NULL;
	}

	if (packet->packet.data.size > payload_strlen + 1) {
		cdl_start = payload + payload_strlen + 1;
		cdl_length = packet->packet.data.size - payload_strlen - 2;

		// parse content description list here		
		content_descriptions = new ContentDescriptionList ();
		if (!content_descriptions->Parse (cdl_start, cdl_length)) {
			delete content_descriptions;
			content_descriptions = NULL;
		}
	}

	do {
		key = strtok_r (start, "=", &state);
		start = NULL;
		
		if (key == NULL)
			break;
			
		if (key [0] == ' ')
			key++;
		
		if (!strcmp (key, "features")) {
			value = strtok_r (NULL, "\"", &state);
		} else {
			value = strtok_r (NULL, ",", &state);
		}
		
		if (value == NULL)
			break;
			
		LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): %s=%s\n", key, value);
		
		if (!strcmp (key, "playlist-gen-id")) {
			playlist_gen_id = value;
		} else if (!strcmp (key, "broadcast-id")) {
			broadcast_id = value;
		} else if (!strcmp (key, "features")) {
			features = parse_http_streaming_features (value);
		} else {
			printf ("MmsDownloader::ProcessMetadataPacket (): Unexpected metadata: %s=%s\n", key, value);
		}
	} while (true);
	
	if (this->playlist_gen_id != NULL) 
		g_free (this->playlist_gen_id); 
	this->playlist_gen_id = g_strdup (playlist_gen_id);

	source->SetMmsMetadata (playlist_gen_id, broadcast_id, features);

	LOG_MMS ("MmsDownloader::ProcessMetadataPacket (): playlist_gen_id: '%s', broadcast_id: '%s', features: %i\n", playlist_gen_id, broadcast_id, features);
	
	return true;
}

bool
MmsDownloader::ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessPairPacket ()\n");
	
	if (p_packet_times [p_packet_count] == 0)
		p_packet_times [p_packet_count] = get_now ();

	// NOTE: If this is the 3rd $P packet, we need to increase the size reported in the header by
	// the value in the reason field.  This is a break from the normal behaviour of MMS packets
	// so we need to guard against this occurnace here and ensure we actually have enough data
	// buffered to consume
	if (p_packet_count == 2 && size < (header->length + sizeof (MmsHeader) + packet->packet.reason))
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

	p_packet_sizes [p_packet_count] = *offset;

	++p_packet_count;

	if (p_packet_times [0] == p_packet_times [2]) {
		max_bitrate = 0; // prevent /0
	} else {
		max_bitrate = (gint64) (((p_packet_sizes [1] + p_packet_sizes [2]) * 8) / ((double) ((p_packet_times [2] - p_packet_times [0]) / (double) 10000000)));
	}

	return true;
}

bool
MmsDownloader::ProcessDataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *offset)
{
	LOG_MMS ("MmsDownloader::ProcessDataPacket ()\n");
	
	g_return_val_if_fail (source != NULL, true);
	
	source->WritePacket (payload, header->length - sizeof (MmsDataPacket));
	
	return true;
}

/*
 * ContentDescriptionList
 */

bool
ContentDescriptionList::Parse (const char *input, gint32 length)
{
	bool result = false;
	char *str;
	char *duped;
	int str_length = length;
	char *end;

	//LOG_MMS ("ContentDescriptionList::Parse ('%*s', %i)\n", (int) length, input, length);

	// our input may contain embedded nulls or it may not have nulls at all
	// (not even one at the end).
	// since we use string parsing functions on the input, add a null at the 
	// end to not overrun anything.

	str = (char *) g_malloc (str_length + 1);
	memcpy (str, input, str_length);
	str [str_length] = 0; // null terminate
	duped = str; // save a copy of the allocated memory to free it later
	end = str + str_length;

	/*
	 * The format is:
	 *  <name length>,<name>,<value type>,<value length>,<value>
	 */

	char *str_name_length;
	char *str_name;
	char *str_value_type;
	char *str_value_length;
	void *value;
	char *comma;

	gint64 name_length;
	gint64 value_type;
	gint64 value_length;

	do {
		// name length
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_name_length = str;
		str = comma + 1;

		name_length = strtoull (str_name_length, NULL, 10);

		if (name_length < 0 || name_length > G_MAXINT32)
			goto cleanup;

		if (end - str < name_length + 1)
			goto cleanup;

		// name
		str_name = str;
		str_name [name_length] = 0; // null terminate
		str += name_length + 1;

		// value type
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_value_type = str;
		str = comma + 1;

		value_type = strtoull (str_value_type, NULL, 10);

		if (value_type < 0 || value_type > G_MAXINT32)
			goto cleanup;

		// value length
		comma = strchr (str, ',');
		if (comma == NULL)
			goto cleanup;

		*comma = 0; // null terminate
		str_value_length = str;
		str = comma + 1;

		value_length = strtoull (str_value_length, NULL, 10);

		if (value_length < 0 || value_length > G_MAXINT32)
			goto cleanup;

		if (end - str < value_length)
			goto cleanup;

		value = str; // can't null terminate, we don't necessarily have a string

		str += value_length;

		ContentDescription *cd = new ContentDescription ();
		cd->name = g_strndup (str_name, name_length);
		cd->value_type = (ContentDescription::ValueType) value_type;
		cd->value = g_malloc (value_length + 1);
		memcpy (cd->value, value, value_length);
		((char *) cd->value) [value_length] = 0;
		cd->value_length = value_length;
		list.Append (cd);

		// printf ("parsed: %*s = %*s\n", (int) name_length, str_name, (int) cd->value_length, (char *) cd->value);

		// trailing commas
		if (*str == ',') {
			str++;
		} else {
			break;
		}
	} while (str < end);

	result = true;

cleanup:

	g_free (duped);

	return result;
}

/*
 * ContentDescription
 */
 
ContentDescription::~ContentDescription ()
{
	g_free (name);
	g_free (value);
}

/*
 * MmsSecondDownloader
 */

MmsSecondDownloader::MmsSecondDownloader (MmsDownloader *mms)
{
	VERIFY_MAIN_THREAD;

	dl = NULL;
	this->mms = mms;
	this->mms->ref ();
	kill_timeout = 0;
}

void
MmsSecondDownloader::Dispose ()
{
	Deployment *deployment;
	Surface *surface;
	TimeManager *tm;

	VERIFY_MAIN_THREAD;

	if (dl != NULL) {
		dl->RemoveAllHandlers (this);
		dl->unref ();
		dl = NULL;
	}

	if (mms != NULL) {
		mms->unref ();
		mms = NULL;
	}

	if (kill_timeout != 0) {
		deployment = GetDeployment ();
		surface = deployment ? deployment->GetSurface () : NULL;
		tm = surface ? surface->GetTimeManager () : NULL;
		if (tm != NULL) {
			tm->RemoveTimeout (kill_timeout);
			kill_timeout = 0;
			unref ();
		}
	}

	EventObject::Dispose ();
}

void
MmsSecondDownloader::DownloadFailedHandler (EventObject *sender, EventArgs *args)
{
	LOG_MMS ("MmsLogger::DownloadFailedHandler ()\n");
	VERIFY_MAIN_THREAD;

	Dispose ();
}

void
MmsSecondDownloader::CompletedHandler (EventObject *sender, EventArgs *args)
{
	LOG_MMS ("MmsLogger::CompletedHandler ()\n");
	VERIFY_MAIN_THREAD;

	Dispose ();
}

void
MmsSecondDownloader::data_write (void *data, gint32 offset, gint32 n, void *closure)
{
	LOG_MMS ("((MmsLogger *) closure)->DataWrite (data = %p, offset = %i, n = %i);\n", data, offset, n);
}

void
MmsSecondDownloader::CreateDownloader ()
{
	Deployment *deployment;
	Surface *surface;

	VERIFY_MAIN_THREAD;

	deployment = GetDeployment ();

	g_return_if_fail (dl == NULL);
	g_return_if_fail (deployment != NULL);

	surface = deployment->GetSurface ();

	g_return_if_fail (surface != NULL);

	dl = surface->CreateDownloader ();

	dl->AddHandler (Downloader::DownloadFailedEvent, DownloadFailedCallback, this);
	dl->AddHandler (Downloader::CompletedEvent, CompletedCallback, this);
	dl->SetStreamFunctions (data_write, NULL, this);

	dl->SetRequireCustomHeaderSupport (true);
	// firefox will not download an uri equal to the mms uri simultaneously with the mms uri 
	// it tries to open a cache entry for writing, which fails since the cache entry is already in use
	// sp we disable the cace
	dl->SetDisableCache (true);		
	dl->Open ("POST", mms->GetUri (), NoPolicy);
}

void
MmsSecondDownloader::SetKillTimeout (guint seconds)
{
	Deployment *deployment;
	Surface *surface;
	TimeManager *tm;

	deployment = GetDeployment ();
	surface = deployment ? deployment->GetSurface () : NULL;
	tm = surface ? surface->GetTimeManager () : NULL;

	g_return_if_fail (tm != NULL);

	this->ref ();
	tm->AddTimeout (MOON_PRIORITY_IDLE, seconds * 1000, KillTimeoutCallback, this);
}

gboolean
MmsSecondDownloader::KillTimeoutCallback (gpointer context)
{
	((MmsSecondDownloader *) context)->KillTimeoutHandler ();
	return false;
}

void
MmsSecondDownloader::KillTimeoutHandler ()
{
	LOG_MMS ("MmsSecondDownloader::KillTimeoutHandler (), dl: %p\n", dl);
	kill_timeout = 0;
	SetCurrentDeployment ();
	unref ();
	Deployment::SetCurrent (NULL);
}

void
MmsSecondDownloader::SendStreamSwitch ()
{
	GString *pragma;
	MmsPlaylistEntry *entry;

	g_return_if_fail (mms != NULL);

	CreateDownloader ();

	g_return_if_fail (dl != NULL);

	entry = mms->GetCurrentEntryReffed ();

	pragma = g_string_new (NULL);

	set_common_dl_headers (dl, mms, pragma);
	set_stream_selection_headers (dl, mms, pragma, entry);

#ifdef HAVE_CURL
	if (moonlight_flags & RUNTIME_INIT_CURL_BRIDGE) {
	} else {
#endif
		g_string_append (pragma, "\r\n");
		dl->InternalSetBody (pragma->str, pragma->len);
#ifdef HAVE_CURL
	}
#endif
	dl->Send ();

	entry->unref ();
	g_string_free (pragma, true);

	LOG_MMS ("MmsSecondDownloader::SendStreamSwitch (): Sent.\n");
}

/*
 * Work in progress
 
void
MmsSecondDownloader::SendLog ()
{
	CreateDownloader ();

	g_return_if_fail (dl != NULL);

//
// POST /SSPLDrtOnDemandTest HTTP/1.0
// Host: moonlightmedia
// Content-Length: 2203
// User-Agent: NSPlayer/11.08.0005.0000
// Accept: * / *
// Accept-Language: en-us, *;q=0.1
// Connection: Keep-Alive
// Content-Type: application/x-wms-Logstats
// Pragma: client-id=3375607867
// Pragma: playlist-gen-id=2769
// Pragma: xClientGuid={00000000-0000-0000-0000-000000000000}
// Supported: com.microsoft.wm.srvppair, com.microsoft.wm.sswitch, com.microsoft.wm.startupprofile, com.microsoft.wm.predstrm
// 

	dl->InternalSetHeader ("User-Agent", "NSPlayer/11.08.0005.0000");
	dl->InternalSetHeader ("Content-Type", "application/x-wms-Logstats");

	GString *header = g_string_new (NULL);

	set_common_dl_headers (dl, mms, header);

	GString *all = g_string_new (NULL);
	GString *xml = g_string_new (NULL);
	GString *summary = g_string_new (NULL);

	// "<c-ip>0.0.0.0</c-ip>"
	g_string_append (summary, "0.0.0.0 ");
	g_string_append (xml, "<c-ip>0.0.0.0</c-ip>");

	// "<date>%.4i-%.2i-%.2i</date>" // yyyy-MM-dd
	tm now;
	time_t time_now = time (NULL);
	gmtime_r (&time_now, &now);
	g_string_append_printf (summary, "%.4i-%.2i-%.2i ", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
	g_string_append_printf (xml, "<date>%.4i-%.2i-%.2i</date>", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);

	// "<time>%.2i:%.2i:%.2i</time>" // HH:mm:ss
	g_string_append_printf (summary, "%.2i:%.2i:%.2i ", now.tm_hour, now.tm_min, now.tm_sec);
	g_string_append_printf (xml, "<time>%.2i:%.2i:%.2i</time>", now.tm_hour, now.tm_min, now.tm_sec);

	// "<c-dns>-</c-dns>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-dns>-</c-dns>");

	// "<cs-uri-stem>-</cs-uri-stem>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<cs-uri-stem>-</cs-uri-stem>");

	// "<c-starttime>0</c-starttime>"
	g_string_append_printf (summary, "0 ");
	g_string_append_printf (xml, "<c-starttime>0</c-starttime>");

	// "<x-duration>0</x-duration>"
	g_string_append_printf (summary, "0 ");
	g_string_append_printf (xml, "<x-duration>0</x-duration>");

	//"<c-rate>-</c-rate>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-rate>-</c-rate>");

	//"<c-status>200</c-status>"
	g_string_append_printf (summary, "200 ");
	g_string_append_printf (xml, "<c-status>200</c-status>");

	// "<c-playerid>" CLIENT_GUID "</c-playerid>"
	g_string_append_printf (summary, "%s ", CLIENT_GUID);
	g_string_append_printf (xml, "<c-playerid>%s</c-playerid>", CLIENT_GUID);

	// "<c-playerversion>-</c-playerversion>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-playerversion>-</c-playerversion>");

	// "<c-playerlanguage>-</c-playerlanguage>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-playerlanguage>-</c-playerlanguage>");

	// "<cs-User-Agent>%s</cs-User-Agent>"
	const char *user_agent = "Mozilla/5.0_(Windows;_U;_Windows_NT_5.1;_en-GB;_rv:1.9.0.9)_Gecko/2009040821_Firefox/3.0.9_(.NET_CLR_3.5.30729)_NSPlayer/11.08.0005.0000_Silverlight/2.0.40115.0"; //"Firefox";
	g_string_append_printf (summary, "%s ", user_agent);
	g_string_append_printf (xml, "<cs-User-Agent>%s</cs-User-Agent>", user_agent);

	// "<cs-Referer>%s</cs-Referer>"
	const char *referrer = "http://192.168.1.4:8080/media/video/test-server-side-playlist.html";//"http://example.com/";
	g_string_append_printf (summary, "%s ", referrer);
	g_string_append_printf (xml, "<cs-Referer>%s</cs-Referer>", referrer);

	// "<c-hostexe>-</c-hostexe>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-hostexe>-</c-hostexe>");

	// "<c-hostexever>-</c-hostexever>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-hostexever>-</c-hostexever>");

	// "<c-os>Linux</c-os>"
	g_string_append_printf (summary, "Windows_XP ");
	g_string_append_printf (xml, "<c-os>Windows_XP</c-os>");

	// "<c-osversion>-</c-osversion>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-osversion>-</c-osversion>");

	// "<c-cpu>-</c-cpu>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-cpu>-</c-cpu>");

	// "<filelength>-</filelength>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<filelength>-</filelength>");

	// "<filesize>-</filesize>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<filesize>-</filesize>");

	// "<avgbandwidth>-</avgbandwidth>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<avgbandwidth>-</avgbandwidth>");

	// "<protocol>http</protocol>"
	g_string_append_printf (summary, "http ");
	g_string_append_printf (xml, "<protocol>http</protocol>");

	// "<transport>TCP</transport>"
	g_string_append_printf (summary, "TCP ");
	g_string_append_printf (xml, "<transport>TCP</transport>");

	// "<audiocodec>-</audiocodec>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<audiocodec>-</audiocodec>");

	// "<videocodec>-</videocodec>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<videocodec>-</videocodec>");

	// "<c-channelURL>-</c-channelURL>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-channelURL>-</c-channelURL>");

	// "<sc-bytes>-</sc-bytes>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<sc-bytes>-</sc-bytes>");

	// "<c-bytes>-</c-bytes>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-bytes>-</c-bytes>");

	// "<s-pkts-sent>-</s-pkts-sent>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<s-pkts-sent>-</s-pkts-sent>");

	// "<c-pkts-received>-</c-pkts-received>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-received>-</c-pkts-received>");

	// "<c-pkts-lost-client>-</c-pkts-lost-client>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-lost-client>-</c-pkts-lost-client>");

	// "<c-pkts-lost-net>-</c-pkts-lost-net>"+
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-lost-net>-</c-pkts-lost-net>");

	// "<c-pkts-lost-cont-net>-</c-pkts-lost-cont-net>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-lost-cont-net>-</c-pkts-lost-cont-net>");

	// "<c-resendreqs>-</c-resendreqs>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-resendreqs>-</c-resendreqs>");

	// "<c-pkts-recovered-ECC>-</c-pkts-recovered-ECC>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-recovered-ECC>-</c-pkts-recovered-ECC>");

	// "<c-pkts-recovered-resent>-</c-pkts-recovered-resent>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-pkts-recovered-resent>-</c-pkts-recovered-resent>");

	// "<c-buffercount>-</c-buffercount>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-buffercount>-</c-buffercount>");

	// "<c-totalbuffertime>-</c-totalbuffertime>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-totalbuffertime>-</c-totalbuffertime>");

	// "<c-quality>-</c-quality>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<c-quality>-</c-quality>");

	// "<s-ip>-</s-ip><s-dns>-</s-dns>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<s-ip>-</s-ip><s-dns>-</s-dns>");

	// "<s-totalclients>-</s-totalclients>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<s-totalclients>-</s-totalclients>");

	// "<s-cpu-util>-</s-cpu-util>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<s-cpu-util>-</s-cpu-util>");

	// "<cs-url>%s</cs-url>"
	g_string_append_printf (summary, "%s ", mms->GetUri ());
	g_string_append_printf (xml, "<cs-url>%s</cs-url>", mms->GetUri ());

	// "<cs-media-name>-</cs-media-name>"
	g_string_append_printf (summary, "- ");
	g_string_append_printf (xml, "<cs-media-name>-</cs-media-name>");

	// "<cs-media-role>-</cs-media-role>"
	g_string_append_printf (summary, "-"); // skip the last space
	g_string_append_printf (xml, "<cs-media-role>-</cs-media-role>");

	g_string_append_printf (header, "Content-Length: %i\r\n", summary->len + xml->len + 11  + 19); // length of <XML></XML>  + <Summary/> 

	g_string_append_printf (header, "\r\n"); // end of header

	g_string_append (all, header->str);
	g_string_append (all, "<XML><Summary>");
	g_string_append (all, summary->str);
	g_string_append (all, "</Summary>");
	g_string_append (all, xml->str);
	g_string_append (all, "</XML>");

	dl->InternalSetBody (all->str, all->len);

	g_string_free (all, true);
	g_string_free (xml, true);
	g_string_free (summary, true);
	g_string_free (header, true);

	dl->Send ();

	LOG_MMS ("MmsDownloader: sent log.\n");
}

*/
