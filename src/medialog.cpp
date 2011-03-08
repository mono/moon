/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * medialog.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "medialog.h"
#include "factory.h"
#include "deployment.h"
#include "runtime.h"
#include "debug.h"

namespace Moonlight {

/*
 * MediaLog
 */

MediaLog::MediaLog ()
{
	c_playerid = g_strdup ("{3300AD50-2C39-46C0-AE0A-000000000000}");
	cs_url = g_strdup ("");
	cs_uri_stem = g_strdup ("");
	c_playerversion = g_strdup ("");
	cs_referrer = g_strdup ("");
	cs_user_agent = g_strdup ("");
	c_quality = 100;
	x_duration = 0;
	filelength = g_strdup ("");
	filesize = g_strdup ("");
}

MediaLog::~MediaLog ()
{
	g_free (c_playerid);
	g_free (cs_url);
	g_free (cs_uri_stem);
	g_free (c_playerversion);
	g_free (cs_referrer);
	g_free (cs_user_agent);
	g_free (filelength);
	g_free (filesize);
}

LogReadyRoutedEventArgs *
MediaLog::CreateEventArgs ()
{
	LogReadyRoutedEventArgs *args;

	args = MoonUnmanagedFactory::CreateLogReadyRoutedEventArgs ();
	args->GiveLog (GetLog (true));

	return args;
}

char *
MediaLog::GetLog (bool include_user_name)
{
	GPtrArray *keys = g_ptr_array_new ();
	GPtrArray *values = g_ptr_array_new ();
	Deployment *deployment = Deployment::GetCurrent ();
	const Uri *source_location;
	char *result;

	VERIFY_MAIN_THREAD;

	source_location = deployment->GetSourceLocation (NULL);
	if (source_location != NULL)
		SetReferrer (source_location->ToString ());
	SetPlayerVersion (deployment->GetRuntimeVersion ());
	SetUserAgent (deployment->GetUserAgent ());

	mutex.Lock ();

	// "<c-ip>0.0.0.0</c-ip>"
	g_ptr_array_add (keys, (void *) "c-ip");
	g_ptr_array_add (values, g_strdup ("0.0.0.0"));

	// "<date>%.4i-%.2i-%.2i</date>" // yyyy-MM-dd
	tm now;
	time_t time_now = time (NULL);
	gmtime_r (&time_now, &now);
	g_ptr_array_add (keys, (void *) "date");
	g_ptr_array_add (values, g_strdup_printf ("%.4i-%.2i-%.2i", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday));

	// "<time>%.2i:%.2i:%.2i</time>" // HH:mm:ss
	g_ptr_array_add (keys, (void *) "time");
	g_ptr_array_add (values, g_strdup_printf ("%.2i:%.2i:%.2i", now.tm_hour, now.tm_min, now.tm_sec));

	// "<c-dns>-</c-dns>"
	g_ptr_array_add (keys, (void *) "c-dns");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<cs-uri-stem>-</cs-uri-stem>"
	g_ptr_array_add (keys, (void *) "cs-uri-stem");
	g_ptr_array_add (values, g_strdup (cs_uri_stem));

	// "<c-starttime>0</c-starttime>"
	g_ptr_array_add (keys, (void *) "c-starttime");
	g_ptr_array_add (values, g_strdup ("0"));

	// "<x-duration>0</x-duration>"
	g_ptr_array_add (keys, (void *) "x-duration");
	g_ptr_array_add (values, g_strdup_printf ("%" G_GUINT64_FORMAT, x_duration));

	//"<c-rate>-</c-rate>"
	g_ptr_array_add (keys, (void *) "c-rate");
	g_ptr_array_add (values, g_strdup ("-"));

	//"<c-status>200</c-status>"
	g_ptr_array_add (keys, (void *) "c-status");
	g_ptr_array_add (values, g_strdup ("200"));

	// "<c-playerid>" CLIENT_GUID "</c-playerid>"
	g_ptr_array_add (keys, (void *) "c-playerid");
	g_ptr_array_add (values, g_strdup (c_playerid));

	// "<c-playerversion>-</c-playerversion>"
	g_ptr_array_add (keys, (void *) "c-playerversion");
	g_ptr_array_add (values, g_strdup (c_playerversion));

	// "<c-playerlanguage>-</c-playerlanguage>"
	g_ptr_array_add (keys, (void *) "c-playerlanguage");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<cs-User-Agent>%s</cs-User-Agent>"
	g_ptr_array_add (keys, (void *) "cs-User-Agent");
	g_ptr_array_add (values, g_strdup (cs_user_agent));

	// "<cs-Referer>%s</cs-Referer>"
	g_ptr_array_add (keys, (void *) "cs-Referer");
	g_ptr_array_add (values, g_strdup (cs_referrer));

	// "<c-hostexe>-</c-hostexe>"
	g_ptr_array_add (keys, (void *) "c-hostexe");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-hostexever>-</c-hostexever>"
	g_ptr_array_add (keys, (void *) "c-hostexever");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-os>Linux</c-os>"
	g_ptr_array_add (keys, (void *) "c-os");
	g_ptr_array_add (values, g_strdup ("Linux"));

	// "<c-osversion>-</c-osversion>"
	g_ptr_array_add (keys, (void *) "c-osversion");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-cpu>-</c-cpu>"
	g_ptr_array_add (keys, (void *) "c-cpu");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<filelength>-</filelength>"
	g_ptr_array_add (keys, (void *) "filelength");
	g_ptr_array_add (values, g_strdup (filelength));

	// "<filesize>-</filesize>"
	g_ptr_array_add (keys, (void *) "filesize");
	g_ptr_array_add (values, g_strdup (filesize));

	// "<avgbandwidth>-</avgbandwidth>"
	g_ptr_array_add (keys, (void *) "avgbandwidth");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<protocol>http</protocol>"
	g_ptr_array_add (keys, (void *) "protocol");
	g_ptr_array_add (values, g_strdup ("http"));

	// "<transport>TCP</transport>"
	g_ptr_array_add (keys, (void *) "transport");
	g_ptr_array_add (values, g_strdup ("TCP"));

	// "<audiocodec>-</audiocodec>"
	g_ptr_array_add (keys, (void *) "audiocodec");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<videocodec>-</videocodec>"
	g_ptr_array_add (keys, (void *) "videocodec");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-channelURL>-</c-channelURL>"
	g_ptr_array_add (keys, (void *) "c-channelURL");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<sc-bytes>-</sc-bytes>"
	g_ptr_array_add (keys, (void *) "sc-bytes");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-bytes>-</c-bytes>"
	g_ptr_array_add (keys, (void *) "c-bytes");
	g_ptr_array_add (values, g_strdup ("0"));

	// "<s-pkts-sent>-</s-pkts-sent>"
	g_ptr_array_add (keys, (void *) "s-pkts-sent");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-received>-</c-pkts-received>"
	g_ptr_array_add (keys, (void *) "c-pkts-received");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-lost-client>-</c-pkts-lost-client>"
	g_ptr_array_add (keys, (void *) "c-pkts-lost-client");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-lost-net>-</c-pkts-lost-net>"
	g_ptr_array_add (keys, (void *) "c-pkts-lost-net");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-lost-cont-net>-</c-pkts-lost-cont-net>"
	g_ptr_array_add (keys, (void *) "c-pkts-lost-cont-net");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-resendreqs>-</c-resendreqs>"
	g_ptr_array_add (keys, (void *) "c-resendreqs");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-recovered-ECC>-</c-pkts-recovered-ECC>"
	g_ptr_array_add (keys, (void *) "c-pkts-recovered-ECC");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-pkts-recovered-resent>-</c-pkts-recovered-resent>"
	g_ptr_array_add (keys, (void *) "c-pkts-recovered-resent");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-buffercount>-</c-buffercount>"
	g_ptr_array_add (keys, (void *) "c-buffercount");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-totalbuffertime>-</c-totalbuffertime>"
	g_ptr_array_add (keys, (void *) "c-totalbuffertime");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<c-quality>-</c-quality>"
	g_ptr_array_add (keys, (void *) "c-quality");
	g_ptr_array_add (values, g_strdup_printf ("%i", c_quality));

	// "<s-ip>-</s-ip>"
	g_ptr_array_add (keys, (void *) "s-ip");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<s-dns>-</s-dns>"
	g_ptr_array_add (keys, (void *) "s-dns");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<s-totalclients>-</s-totalclients>"
	g_ptr_array_add (keys, (void *) "s-totalclients");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<s-cpu-util>-</s-cpu-util>"
	g_ptr_array_add (keys, (void *) "s-cpu-util");
	g_ptr_array_add (values, g_strdup ("-"));

	if (include_user_name) {
		// "<cs-user-name>-</cs-user-name>"
		g_ptr_array_add (keys, (void *) "cs-user-name");
		g_ptr_array_add (values, g_strdup ("-"));
	}

	// "<cs-url>%s</cs-url>"
	g_ptr_array_add (keys, (void *) "cs-url");
	g_ptr_array_add (values, g_strdup (cs_url));

	// "<cs-media-name>-</cs-media-name>"
	g_ptr_array_add (keys, (void *) "cs-media-name");
	g_ptr_array_add (values, g_strdup ("-"));

	// "<cs-media-role>-</cs-media-role>"
	g_ptr_array_add (keys, (void *) "cs-media-role");
	g_ptr_array_add (values, g_strdup ("-"));

	g_ptr_array_add (keys, NULL);
	g_ptr_array_add (values, NULL);

	mutex.Unlock ();

	result = Deployment::GetCurrent ()->CreateMediaLogXml ((const char **) keys->pdata, (const char **) values->pdata);

	g_ptr_array_free (keys, true);
	for (unsigned int i = 0; i < values->len; i++)
		g_free (g_ptr_array_index (values, i));
	g_ptr_array_free (values, true);

	return result;
}

void
MediaLog::SetUrl (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, cs_url) != 0) {
		g_free (cs_url);
		cs_url = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetUriStem (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, cs_uri_stem) != 0) {
		g_free (cs_uri_stem);
		cs_uri_stem = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetPlayerId (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, c_playerid) != 0) {
		g_free (c_playerid);
		c_playerid = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetPlayerVersion (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, c_playerversion) != 0) {
		g_free (c_playerversion);
		c_playerversion = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetUserAgent (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, cs_user_agent) != 0) {
		g_free (cs_user_agent);
		cs_user_agent = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetFileLength (guint64 value)
{
	char *tmp = g_strdup_printf ("%" G_GUINT64_FORMAT, value);
	SetFileLength (tmp);
	g_free (tmp);
}

void
MediaLog::SetFileLength (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, filelength) != 0) {
		g_free (filelength);
		filelength = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetFileSize (guint64 value)
{
	char *tmp = g_strdup_printf ("%" G_GUINT64_FORMAT, value);
	SetFileSize (tmp);
	g_free (tmp);
}

void
MediaLog::SetFileSize (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, filesize) != 0) {
		g_free (filesize);
		filesize = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetQuality (guint32 value)
{
	mutex.Lock ();
	c_quality = value;
	mutex.Unlock ();
}

void
MediaLog::SetReferrer (const char *value)
{
	if (value == NULL)
		return;

	mutex.Lock ();
	if (strcmp (value, cs_referrer) != 0) {
		g_free (cs_referrer);
		cs_referrer = g_strdup (value);
	}
	mutex.Unlock ();
}

void
MediaLog::SetDuration (guint64 value)
{
	mutex.Lock ();
	x_duration = value;
	mutex.Unlock ();
}

};
