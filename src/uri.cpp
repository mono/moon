/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * uri.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "deployment.h"
#include "uri.h"
#include "debug.h"

namespace Moonlight {

Uri::Uri ()
{
	Init ();
}

Uri::Uri (void *gchandle)
{
	Init ();
	this->gchandle = gchandle;
}

Uri::~Uri ()
{
	g_free (scheme); scheme = NULL;
	g_free (host); host = NULL;
	g_free (path); path = NULL;
	g_free (query); query = NULL;
	g_free (fragment); fragment = NULL;
	g_free (original_string); original_string = NULL;
	g_free (tostring); tostring = NULL;
	g_free (http_request_string); http_request_string = NULL;
	if (gchandle)
		deployment->FreeGCHandle (gchandle);
	gchandle = NULL;
	deployment = NULL;
	is_absolute = false;
}

void
Uri::Init ()
{
	deployment = Deployment::GetCurrent ();
	gchandle = NULL;
	scheme = NULL;
	host = NULL;
	port = 0;
	path = NULL;
	query = NULL;
	fragment = NULL;
	original_string = NULL;
	tostring = NULL;
	http_request_string = NULL;
	is_absolute = false;

	scheme_fetched = false;
	host_fetched = false;
	path_fetched = false;
	query_fetched = false;
	fragment_fetched = false;
	original_string_fetched = false;
	http_request_string_fetched = false;
	tostring_fetched = false;
	is_absolute_fetched = false;
	port_fetched = false;
}

void *
Uri::GetGCHandle () const
{
	return gchandle;
}

Uri *
Uri::Create (const char *uri_string)
{
	void *gchandle;

	gchandle = Deployment::GetCurrent ()->GetUriFunctions ()->ctor_2 (uri_string, UriKindRelativeOrAbsolute);

	return gchandle == NULL ? NULL : new Uri (gchandle);
}

Uri *
Uri::Create (const char *uri_string, UriKind uri_kind)
{
	void *gchandle;

	gchandle = Deployment::GetCurrent ()->GetUriFunctions ()->ctor_2 (uri_string, uri_kind);

	return gchandle == NULL ? NULL : new Uri (gchandle);
}

Uri *
Uri::Create (const Uri *base_uri, const char *relative_uri)
{
	void *gchandle;

	gchandle = Deployment::GetCurrent ()->GetUriFunctions ()->ctor_3 (base_uri->GetGCHandle (), relative_uri);

	return gchandle == NULL ? NULL : new Uri (gchandle);
}

Uri *
Uri::Create (const Uri *base_uri, const Uri *relative_uri)
{
	void *gchandle;

	gchandle = Deployment::GetCurrent ()->GetUriFunctions ()->ctor_4 (base_uri->GetGCHandle (), relative_uri->GetGCHandle ());

	return gchandle == NULL ? NULL : new Uri (gchandle);
}

Uri *
Uri::Clone (const Uri *uri_to_clone)
{
	void *gchandle = NULL;

	if (uri_to_clone != NULL && uri_to_clone->GetGCHandle () != NULL)
		gchandle = uri_to_clone->deployment->GetUriFunctions ()->clone (uri_to_clone->GetGCHandle ());

	return new Uri (gchandle);
}

Uri *
Uri::CloneWithScheme (const Uri *uri_to_clone, const char *scheme)
{
	void *gchandle = NULL;

	if (uri_to_clone != NULL && uri_to_clone->GetGCHandle () != NULL)
		gchandle = uri_to_clone->deployment->GetUriFunctions ()->clone_with_scheme (uri_to_clone->GetGCHandle (), scheme);

	return new Uri (gchandle);
}

Uri *
Uri::CombineWithSourceLocation (Deployment *deployment, const Uri *base_uri, const Uri *relative_uri)
{
	Uri *absolute_dummy;
	Uri *absolute_base;
	Uri *absolute_uri;
	Uri *result;

	/*
	 * We must combine base_uri and relative_uri, having in mind that base_uri can't get out of the
	 * source location using ".."
	 * So we create a dummy absolute uri (with an empty path), and combine it with the base_uri => absolute_base.
	 * This will remove any ".."'s we don't want.
	 * Then we combine absolute_base with the relative_uri => absolute_uri.
	 * Finally we take the source location and combine it with the path of absolute_uri => result.
	 */

	LOG_DOWNLOADER ("Uri::CombineWithSourceLocation (%s, %s, %s)\n", deployment->GetSourceLocation (NULL)->ToString (), base_uri ? base_uri->ToString () : NULL, relative_uri->ToString ());
	absolute_dummy = Uri::Create ("http://www.mono-project.com/");
	if (base_uri != NULL && base_uri->GetOriginalString () != NULL) {
		absolute_base = Uri::Create (absolute_dummy, base_uri);
	} else {
		absolute_base = Uri::Clone (absolute_dummy);
	}
	delete absolute_dummy;

	if (absolute_base == NULL)
		return NULL;

	LOG_DOWNLOADER ("Uri::CombineWithSourceLocation (): absolute base uri with dummy root: '%s'\n", absolute_base->ToString ());

	/* Combine the absolute base uri with the uri of the resource */
	absolute_uri = Uri::Create (absolute_base, relative_uri);
	delete absolute_base;

	if (absolute_uri == NULL)
		return NULL;

	LOG_DOWNLOADER ("Uri::CombineWithSourceLocation (): absolute base with dummy root and uri: '%s'\n", absolute_uri->ToString ());

	const char *path = absolute_uri->GetPath ();
	if (path != NULL && path [0] == '/') {
		/* Skip over any '/' so that the absolute uri's path is not resolved against the root of the source location */
		path++;
	}
	result = Uri::Create (deployment->GetSourceLocation (NULL), path);
	delete absolute_uri;

	LOG_DOWNLOADER ("Uri::CombineWithSourceLocation () final uri: '%s' (path: '%s')\n", absolute_uri->ToString (), path);

	return result;
}

const char *
Uri::GetScheme () const
{
	if (!scheme_fetched && gchandle != NULL) {
		scheme = deployment->GetUriFunctions ()->get_scheme (gchandle);
		scheme_fetched = true;
	}
	return scheme;
}

const char *
Uri::GetHost () const
{
	if (!host_fetched && gchandle != NULL) {
		host = deployment->GetUriFunctions ()->get_host (gchandle);
		host_fetched = true;
	}
	return host;
}

int
Uri::GetPort () const
{
	if (!port_fetched && gchandle != NULL) {
		port = deployment->GetUriFunctions ()->get_port (gchandle);
		port_fetched = true;
	}
	return port;
}

const char *
Uri::GetFragment () const
{
	if (!fragment_fetched && gchandle != NULL) {
		fragment = deployment->GetUriFunctions ()->get_fragment (gchandle);
		fragment_fetched = true;
	}
	return fragment;
}

const char *
Uri::GetPath () const
{
	if (!path_fetched && gchandle != NULL) {
		path = deployment->GetUriFunctions ()->get_path (gchandle);
		path_fetched = true;
	}
	return path;
}

const char *
Uri::GetQuery () const
{
	if (!query_fetched && gchandle != NULL) {
		query = deployment->GetUriFunctions ()->get_query (gchandle);
		query_fetched = true;
	}
	return query;
}

const char *
Uri::GetOriginalString () const
{
	if (!original_string_fetched && gchandle != NULL) {
		original_string = deployment->GetUriFunctions ()->get_original_string (gchandle);
		original_string_fetched = true;
	}
	return original_string;
}

const char *
Uri::ToString () const
{
	if (!tostring_fetched && gchandle != NULL) {
		tostring = deployment->GetUriFunctions ()->tostring (gchandle);
		tostring_fetched = true;
	}
	return tostring;
}

const char *
Uri::GetHttpRequestString () const
{
	if (!http_request_string_fetched && gchandle != NULL) {
		http_request_string = deployment->GetUriFunctions ()->get_http_request_string (gchandle);
		http_request_string_fetched = true;
	}
	return http_request_string;
}

bool
Uri::IsAbsolute () const
{
	if (!is_absolute_fetched && gchandle != NULL) {
		is_absolute = deployment->GetUriFunctions ()->get_is_absolute (gchandle);
		is_absolute_fetched = true;
	}
	return is_absolute;
}

bool 
Uri::operator== (const Uri &v) const
{
	return Equals (this, &v);
}

bool
Uri::Equals (const Uri *left, const Uri *right)
{
	if (!left && !right)
		return true;
	if (!left || !right)
		return false;
	if (left == right || left->GetGCHandle () == right->GetGCHandle ())
		return true;
	return left->deployment->GetUriFunctions ()->equals (left->GetGCHandle (), right->GetGCHandle ());
}

bool
Uri::IsNullOrEmpty (const Uri *uri)
{
	if (uri == NULL)
		return true;

	return uri->GetOriginalString () == NULL || uri->GetOriginalString () [0] == 0;
}

bool
Uri::IsScheme (const char *scheme) const
{
	if (!!this->GetScheme () != !!scheme)
		return false;

	if (this->GetScheme ())
		return !g_ascii_strcasecmp (this->GetScheme (), scheme);

	return true;
}

bool 
Uri::SameSiteOfOrigin (const Uri *left, const Uri *right)
{
	// works only on absolute URI
	if (!left || !left->IsAbsolute () || !right || !right->IsAbsolute ())
		return false;

	if (left->GetPort () != right->GetPort ())
		return false;

	if (!left->IsScheme (right->GetScheme ()))
		return false;

	// comparing 2 file:/// URI where no hosts is present
	if (left->IsScheme ("file"))
		return true;

	if (!Uri::SameDomain (left, right))
		return false;

	return true;
}

bool
Uri::SameScheme (const Uri *uri1, const Uri *uri2)
{
	return uri1->IsScheme (uri2->GetScheme ());
}

bool
Uri::SameDomain (const Uri *uri1, const Uri *uri2)
{
	const char *host1 = uri1->GetHost ();
	const char *host2 = uri2->GetHost ();
	
	if (host1 && host2)
		return g_ascii_strcasecmp (host1, host2) == 0;
	
	if (!host1 && !host2)
		return true;
	
	return false;
}

bool
Uri::IsInvalidPath () const
{
	const char *path = GetOriginalString ();
	return path && (path [0] == '\\' || (path [0] == '.' && path [1] == '\\'));
}

bool
Uri::IsUncPath () const
{
	const char *path = GetOriginalString ();
	return path && path [0] == '\\';
}

};
