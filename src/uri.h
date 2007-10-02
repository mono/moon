/*
 * uri.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __URI_H__
#define __URI_H__

#include <glib.h>

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
	
	bool Parse (const char *uri);
	
	char *ToString ();
};

#endif /* __URI_H__ */
