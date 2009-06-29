/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * http-streaming.cpp: 
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

#include <string.h>

#include "http-streaming.h"
#include "debug.h"


static const char * features [] = {"broadcast", "last", "live", "playlist", "reliable", "seekable", "skipbackwards", "skipforward", "stridable", NULL};

HttpStreamingFeatures
parse_http_streaming_features (const char *val)
{
	HttpStreamingFeatures result = HttpStreamingFeaturesNone;
	size_t length = 0;
	bool end = false;

	LOG_HTTPSTREAMING ("parse_http_streaming_features ('%s')\n", val);
		
	if (val == NULL)
		return result;
	
	if (val [0] == '"')
		val++;
	
	while (!end) {
		end = (val [length] == 0 || val [length] == '"');
		if (end || val [length] == ',') {
			
			//printf ("Checking feature: '%.*s'\n", length, val + start);
			
			for (int i = 0; features [i] != NULL; i++) {
				if (length != strlen (features [i]))
					continue;
					
				if (strncmp (val, features [i], length) == 0) {
					result = (HttpStreamingFeatures) (result | (1 << i));
					break;
				}
			}
			//printf ("Features: '%i'\n", results);
				
			val += length + 1;
			length = 0;
		} else {
			length++;
		}
	}
	
	return result;
}
