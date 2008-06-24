/*
 * uri.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __URI_H__
#define __URI_H__

#include <glib.h>

enum UriToStringFlags {
	UriHidePasswd   = 1 << 0,
	UriHideFragment = 1 << 1,
};

class Uri {
public:
	char *protocol;
	char *user;
	char *auth;
	char *passwd;
	char *host;
	int port;
	char *path;
	GData *params;
	char *query;
	char *fragment;
	
	Uri ();
	~Uri ();
	
	bool Parse (const char *uri, bool allow_trailing_sep = false);
	
	char *ToString (UriToStringFlags flags);
	char *ToString () { return ToString ((UriToStringFlags) 0); }
	Uri *Clone ();
};

#endif /* __URI_H__ */
