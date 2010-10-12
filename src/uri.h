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

#include "value.h"
#include "gchandle.h"

namespace Moonlight {

enum UriKind {
	UriKindRelativeOrAbsolute,
	UriKindAbsolute,
	UriKindRelative,
};

typedef void *(* System_Uri_Ctor_1) (const char *uri_string);
typedef void *(* System_Uri_Ctor_2) (const char *uri_string, UriKind uri_kind);
typedef void *(* System_Uri_Ctor_3) (const void *base_uri, const char *relative_uri);
typedef void *(* System_Uri_Ctor_4) (const void *base_uri, const void *relative_uri);
typedef char *(* System_Uri_GetStringProperty) (const void *instance);
typedef gint32 (* System_Uri_GetInt32Property) (const void *instance);
typedef bool (* System_Uri_GetBooleanProperty) (const void *instance);
typedef char *(* System_Uri_ToString) (const void *instance);
typedef bool (* System_Uri_Equals) (const void *a, const void *b);
typedef void *(* System_Uri_CloneWithScheme) (const void *instance, const char *scheme);

struct UriFunctions {
	System_Uri_Ctor_1 ctor_1;
	System_Uri_Ctor_2 ctor_2;
	System_Uri_Ctor_3 ctor_3;
	System_Uri_Ctor_4 ctor_4;
	System_Uri_GetStringProperty get_scheme;
	System_Uri_GetStringProperty get_host;
	System_Uri_GetInt32Property get_port;
	System_Uri_GetStringProperty get_fragment;
	System_Uri_GetStringProperty get_path;
	System_Uri_GetStringProperty get_query;
	System_Uri_GetStringProperty get_original_string;
	System_Uri_GetBooleanProperty get_is_absolute;
	System_Uri_ToString tostring;
	System_Uri_Equals equals;
	System_Uri_CloneWithScheme clone_with_scheme;
	System_Uri_ToString get_http_request_string;
};

/* @IncludeInKinds */
/* @SkipValue */
/* @Namespace=System */
class Uri {
public:
	Uri ();
	/* @GenerateCBinding,GeneratePInvoke */
	Uri (GCHandle gc_handle);
	~Uri ();

	/* @GenerateCBinding,GeneratePInvoke */
	GCHandle GetGCHandle () const;

	/* Managed API. Note that we don't use a ctor because we can't throw exceptions like managed code can. */
	static Uri *Create (const char *uri_string);
	static Uri *Create (const char *uri_string, UriKind uri_kind);
	static Uri *Create (const Uri *base_uri, const char *relative_uri);
	static Uri *Create (const Uri *base_uri, const Uri *relative_uri);

	/* Clone a uri. We just make another gc handle to the managed uri */
	static Uri *Clone (const Uri *uri_to_clone);
	/* Clone a uri, but change the scheme. */
	static Uri *CloneWithScheme (const Uri *uri_to_clone, const char *scheme);

	static Uri *CombineWithSourceLocation (Deployment *deployment, const Uri *base_uri, const Uri *relative_uri, bool allow_escape = false);

	const char *ToString () const;
	const char *GetHttpRequestString () const;

	bool operator== (const Uri &v) const;

	static bool Equals (const Uri *left, const Uri *right);
	static bool IsNullOrEmpty (const Uri *uri);
	static bool SameSiteOfOrigin (const Uri *left, const Uri *right);
	static bool SameScheme (const Uri *uri1, const Uri *uri2);
	static bool SameDomain (const Uri *uri1, const Uri *uri2);

	bool IsScheme (const char *scheme) const;
	bool IsAbsolute () const;
	
	bool IsInvalidPath () const;
	bool IsUncPath () const;

	const char *GetScheme () const;
	const char *GetHost () const;
	const char *GetFragment () const;
	const char *GetPath () const;
	const char *GetQuery () const;
	const char *GetOriginalString () const;
	int GetPort () const;

private:
	Deployment *deployment;
	GCHandle gchandle;
	/* Since Uri is immutable, we can cache properties to only do the native->managed transition once per property */
	mutable char *scheme;
	mutable char *host;
	mutable char *path;
	mutable char *query;
	mutable char *fragment;
	mutable char *original_string;
	mutable char *tostring;
	mutable char *http_request_string;
	mutable bool is_absolute;
	mutable int port;

	mutable bool scheme_fetched:1;
	mutable bool host_fetched:1;
	mutable bool path_fetched:1;
	mutable bool query_fetched:1;
	mutable bool fragment_fetched:1;
	mutable bool original_string_fetched:1;
	mutable bool tostring_fetched:1;
	mutable bool http_request_string_fetched:1;
	mutable bool is_absolute_fetched:1;
	mutable bool port_fetched:1;

	void Init ();
};

};
#endif /* __URI_H__ */
