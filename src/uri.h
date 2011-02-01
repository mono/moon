/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "moonbuild.h"

enum UriToStringFlags {
	UriHidePasswd   = 1 << 0,
	UriHideFragment = 1 << 1,
	UriHideQuery    = 1 << 2,
	UriShowFileScheme    = 1 << 3,
};

/* @IncludeInKinds */
/* @SkipValue */
/* @Namespace=System */
struct MOON_API Uri {
public:
	Uri ();
	Uri (const Uri& uri);

	~Uri ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool Parse (const char *uri, bool allow_trailing_sep = false);
	void Combine (const char *relative_path);
	void Combine (const Uri *relative_uri);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Free ();

	char *ToString (UriToStringFlags flags) const;
	char *ToString () const { return ToString ((UriToStringFlags) 0); }

	static void Copy (const Uri *from, Uri *to);

	bool operator== (const Uri &v) const;

	/* @GenerateCBinding */
	static bool Equals (const Uri *left, const Uri *right);
	static bool IsNullOrEmpty (const Uri *uri);
	static bool SameSiteOfOrigin (const Uri *left, const Uri *right);

	/* @GenerateCBinding */
	guint GetHashCode ();

	bool IsScheme (const char *scheme) const;
	bool IsAbsolute () const { return isAbsolute; }
	
	bool IsInvalidPath () { return path && (path[0] == '.' || path[0] == '\\') && path[1] == '\\'; }
	bool IsUncPath () { return path && path[0] == '\\' && path[1] == '\\'; }
	
	const char *GetScheme () const { return scheme; }
	const char *GetHost () const { return host; }
	int GetPort () const { return port; }
	const char *GetUser () const { return user; }
	const char *GetAuth () const { return auth; }
	const char *GetPasswd () const { return passwd; }
	const char *GetFragment () const { return fragment; }
	const char *GetPath () const { return path; }
	const char *GetQuery () const { return query; }
	
	struct Param {
		Param *next;
		char *value;
		char *name;
	};
	
	bool isAbsolute;

	char *scheme;
	char *user;
	char *auth;
	char *passwd;
	char *host;
	int port;
	char *path;
	Param *params;
	char *query;
	char *fragment;

	char *originalString;
};

#endif /* __URI_H__ */
