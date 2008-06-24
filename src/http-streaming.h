/*
 * http-streaming.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_HTTP_STREAMING__
#define __MOON_HTTP_STREAMING__

#include <glib.h>

enum HttpStreamingFeatures {
	HttpStreamingFeaturesNone	= 0,
	HttpStreamingBroadcast		= 1 << 0,
	HttpStreamingLast			= 1 << 1,
	HttpStreamingLive			= 1 << 2,
	HttpStreamingPlaylist		= 1 << 3,
	HttpStreamingReliable		= 1 << 4,
	HttpStreamingSeekable		= 1 << 5,
	HttpStreamingSkipbackward	= 1 << 6,
	HttpStreamingSkipforward	= 1 << 7,
	HttpStreamingStridable		= 1 << 8,
};

G_BEGIN_DECLS

HttpStreamingFeatures parse_http_streaming_features (const char *value);

G_END_DECLS

#endif // __MOON_HTTP_STREAMING__
