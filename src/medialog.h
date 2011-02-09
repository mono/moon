/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * medialog.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOONLIGHT_MEDIALOG_H__
#define __MOONLIGHT_MEDIALOG_H__

#include <glib.h>

#include "eventargs.h"
#include "mutex.h"

namespace Moonlight {

class MediaLog {
	// every field access in this class must be protected by the mutex
	Mutex mutex;
	char *c_playerid;
	char *cs_url;
	char *cs_uri_stem;
	char *c_playerversion;
	char *cs_referrer;
	char *cs_user_agent;
	guint32 c_quality;
	guint64 filesize;
	guint64 filelength;
	guint64 x_duration;

public:
	MediaLog ();
	~MediaLog ();

	LogReadyRoutedEventArgs *CreateEventArgs ();

	void SetUrl (const char *value);
	void SetUriStem (const char *value);
	void SetPlayerId (const char *value);
	void SetPlayerVersion (const char *value);
	void SetUserAgent (const char *value);
	void SetFileLength (guint64 value);
	void SetFileSize (guint64 value);
	void SetQuality (guint32 value);
	void SetReferrer (const char *value);
	void SetDuration (guint64 duration);
};

};

#endif /* __MOONLIGHT_MEDIALOG_H__ */
