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

/* @IncludeInKinds */
/* @SkipValue */
/* @Namespace=System */
struct Uri {
public:
	bool isAbsolute;

	char *scheme;
	char *user;
	char *auth;
	char *passwd;
	char *host;
	int port;
	char *path;
	GData *params;
	char *query;
	char *fragment;

	char *originalString;

	Uri ();
	Uri (const Uri& uri);

	~Uri ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool Parse (const char *uri, bool allow_trailing_sep = false);

	/* @GenerateCBinding,GeneratePInvoke */
	void Free ();

	char *ToString (UriToStringFlags flags);
	char *ToString () { return ToString ((UriToStringFlags) 0); }

	static void Copy (const Uri *from, Uri *to);

	bool operator== (const Uri &v) const;

	/* @GenerateCBinding */
	static bool Equals (const Uri *left, const Uri *right);

	bool IsScheme (const char *scheme);
};
#endif /* __URI_H__ */
