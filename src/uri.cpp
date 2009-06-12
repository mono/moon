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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "uri.h"


/* see rfc1738, section 2.2 */
#define is_unsafe(c) (((unsigned char) c) <= 0x1f || ((unsigned char) c) == 0x7f)


Uri::Uri ()
{
	scheme = NULL;
	user = NULL;
	auth = NULL;
	passwd = NULL;
	host = NULL;
	port = 0;
	path = NULL;
	params = NULL;
	query = NULL;
	fragment = NULL;
	originalString = g_strdup("");
	isAbsolute = false;
}

Uri::Uri (const Uri& uri)
{
	scheme = NULL;
	user = NULL;
	auth = NULL;
	passwd = NULL;
	host = NULL;
	port = 0;
	path = NULL;
	g_datalist_init (&params);
	query = NULL;
	fragment = NULL;
	originalString = NULL;
	isAbsolute = false;

	Uri::Copy (&uri, this);
}

Uri::~Uri ()
{
	Free ();
}

void
Uri::Free ()
{
	g_free (scheme); scheme = NULL;
	g_free (user); user = NULL;
	g_free (auth); auth = NULL;
	g_free (passwd); passwd = NULL;
	g_free (host); host = NULL;
	g_free (path); path = NULL;
	g_datalist_clear (&params);
	g_datalist_init (&params);
	g_free (query); query = NULL;
	g_free (fragment); fragment = NULL;
	g_free (originalString); originalString = NULL;
	isAbsolute = false;
}

static void
clone_params (GQuark quark, gpointer data, gpointer user_data)
{
	gchar *str = (gchar *) data;
	GData **params = (GData **) user_data;
	g_datalist_id_set_data_full (params, quark, g_strdup (str), g_free);
}

void
Uri::Copy (const Uri *from, Uri *to)
{
	g_datalist_init (&to->params);
	
	if (from != NULL) {
		to->scheme = g_strdup (from->scheme);
		to->user = g_strdup (from->user);
		to->auth = g_strdup (from->auth);
		to->passwd = g_strdup (from->passwd);
		to->host = g_strdup (from->host);
		to->port = from->port;
		to->path = g_strdup (from->path);
		g_datalist_foreach ((GData**)&from->params, clone_params, &to->params);
		to->query = g_strdup (from->query);
		to->fragment = g_strdup (from->fragment);
		to->originalString = g_strdup (from->originalString);
		to->isAbsolute = from->isAbsolute;
	} else {
		to->scheme = NULL;
		to->user = NULL;
		to->auth = NULL;
		to->passwd = NULL;
		to->host = NULL;
		to->port = -1;
		to->path = NULL;
		to->query = NULL;
		to->fragment = NULL;
		to->originalString = NULL;
		to->isAbsolute = false;
	}
}

/* canonicalise a path */
static char *
canon_path (char *path, bool allow_root, bool allow_trailing_sep)
{
	register char *d, *inptr;
	
	d = inptr = path;
	
	while (*inptr) {
		if (inptr[0] == '/' && (inptr[1] == '/' || (inptr[1] == '\0' && !allow_trailing_sep)))
			inptr++;
		else
			*d++ = *inptr++;
	}
	
	if (!allow_root && (d == path + 1) && d[-1] == '/')
		d--;
	else if (allow_root && d == path && path[0] == '/')
		*d++ = '/';
	
	*d = '\0';
	
	return path[0] ? path : NULL;
}

#define HEXVAL(c) (isdigit ((int) ((unsigned char) c)) ? (c) - '0' : tolower ((unsigned char) c) - 'a' + 10)

#define is_xdigit(c) isxdigit ((int) ((unsigned char) c))

static void
url_decode (char *in, const char *url)
{
	register char *inptr, *outptr;
	
	inptr = outptr = in;
	while (*inptr) {
		if (*inptr == '%') {
			if (is_xdigit (inptr[1]) && is_xdigit (inptr[2])) {
				*outptr++ = HEXVAL (inptr[1]) * 16 + HEXVAL (inptr[2]);
				inptr += 3;
			} else {
				g_warning ("Invalid encoding in url: %s at %s", url, inptr);
				*outptr++ = *inptr++;
			}
		} else
			*outptr++ = *inptr++;
	}
	
	*outptr = '\0';
}

bool
Uri::Parse (const char *uri, bool allow_trailing_sep)
{
	char *name, *value, *scheme = NULL, *user = NULL, *auth = NULL, *passwd = NULL, *host = NULL, *path = NULL, *query = NULL, *fragment = NULL;
	register const char *start, *inptr;
	bool isAbsolute;
	bool parse_path = false;
	GData *params = NULL;
	int port = -1;
	size_t n;
	
	start = uri;

	isAbsolute = true;

	if (!*start) {
		isAbsolute = false;
		goto done;
	}

	inptr = start;
	while (*inptr && *inptr != ':' && *inptr != '/' && *inptr != '?' && *inptr != '#' && *inptr != '\\')
		inptr++;
	
	if (inptr > start && *inptr == ':') {
		scheme = g_ascii_strdown (start, inptr - start);
		
		inptr++;
		if (!*inptr) {
			isAbsolute = false;
			goto done;
		}
		
		if (!strncmp (inptr, "//", 2))
			inptr += 2;
		
		start = inptr;
		while (*inptr && *inptr != ';' && *inptr != ':' && *inptr != '@' && *inptr != '/')
			inptr++;
	} else {
		isAbsolute = !strncmp (inptr, "\\\\", 2);
		parse_path = true;
		scheme = NULL;
		inptr = uri;
	}
	
	switch (*inptr) {
	case ';': /* <user>;auth= */
	case ':': /* <user>:passwd or <host>:port */
	case '@': /* <user>@host */
		if (inptr - start) {
			user = g_strndup (start, inptr - start);
			url_decode (user, uri);
		}
		
		switch (*inptr) {
		case ';': /* ;auth= */
			if (!g_ascii_strncasecmp (inptr, ";auth=", 6)) {
				inptr += 6;
				start = inptr;
				while (*inptr && *inptr != ':' && *inptr != '@')
					inptr++;
				
				if (inptr - start) {
					auth = g_strndup (start, inptr - start);
					url_decode (auth, uri);
				}
				
				if (*inptr == ':') {
					inptr++;
					start = inptr;
					goto decode_passwd;
				} else if (*inptr == '@') {
					inptr++;
					start = inptr;
					goto decode_host;
				}
			}
			break;
		case ':': /* <user>:passwd@ or <host>:port */
			inptr++;
			start = inptr;
		decode_passwd:
			while (*inptr && *inptr != '@' && *inptr != '/')
				inptr++;
			
			if (*inptr == '@') {
				/* <user>:passwd@ */
				if (inptr - start) {
					passwd = g_strndup (start, inptr - start);
					url_decode (passwd, uri);
				}
				
				inptr++;
				start = inptr;
				goto decode_host;
			} else {
				/* <host>:port */
				host = user;
				user = NULL;
				inptr = start;
				goto decode_port;
			}
			
			break;
		case '@': /* <user>@host */
			inptr++;
			start = inptr;
		decode_host:
			while (*inptr && *inptr != ':' && *inptr != '/')
				inptr++;
			
			if (inptr > start) {
				n = inptr - start;
				while (n > 0 && start[n - 1] == '.')
					n--;
				
				if (n > 0)
					host = g_strndup (start, n);
			}
			
			if (*inptr == ':') {
				inptr++;
			decode_port:
				port = 0;
				
				while (*inptr >= '0' && *inptr <= '9' && port < 6554)
					port = (port * 10) + ((*inptr++) - '0');
				
				if (port > 65535) {
					/* chop off the last digit */
					port /= 10;
				}
				
				while (*inptr && *inptr != '/')
					inptr++;
			}
		}
		break;
	case '/': /* <host>/path or simply <host> */
	case '\0':
		if (inptr > start) {
			n = inptr - start;
			while (n > 0 && start[n - 1] == '.')
				n--;
			
			if (n > 0)
				host = g_strndup (start, n);
		}
		break;
	default:
		break;
	}
	
	if (parse_path || *inptr == '/') {
		/* look for params, query, or fragment */
		start = inptr;
		while (*inptr && *inptr != ';' && *inptr != '?' && *inptr != '#')
			inptr++;
		
		/* canonicalise and save the path component */
		if ((n = (inptr - start))) {
			value = g_strndup (start, n);
			url_decode (value, uri);
			
			if (!(path = canon_path (value, !host, allow_trailing_sep)))
				g_free (value);
		}
		
		switch (*inptr) {
		case ';':
			while (*inptr == ';') {
				while (*inptr == ';')
					inptr++;
				
				start = inptr;
				while (*inptr && *inptr != '=' && *inptr != ';' && *inptr != '?' && *inptr != '#')
					inptr++;
				
				name = g_strndup (start, inptr - start);
				url_decode (name, uri);
				
				if (*inptr == '=') {
					inptr++;
					start = inptr;
					while (*inptr && *inptr != ';' && *inptr != '?' && *inptr != '#')
						inptr++;
					
					value = g_strndup (start, inptr - start);
					url_decode (value, uri);
				} else {
					value = g_strdup ("");
				}
				
				g_datalist_set_data_full (&params, name, value, g_free);
				g_free (name);
			}
			
			if (*inptr == '#')
				goto decode_fragment;
			else if (*inptr != '?')
				break;
			
			/* fall thru */
		case '?':
			inptr++;
			start = inptr;
			while (*inptr && *inptr != '#')
				inptr++;
			
			query = g_strndup (start, inptr - start);
			url_decode (query, uri);
			
			if (*inptr != '#')
				break;
			
			/* fall thru */
		case '#':
		decode_fragment:
			fragment = g_strdup (inptr + 1);
			url_decode (fragment, uri);
			break;
		}
	}
	
done:
	
	Free ();

	// update the values
	
	this->scheme = scheme;
	this->user = user;
	this->auth = auth;
	this->passwd = passwd;
	this->host = host;
	this->port = port;
	this->path = path;
	this->params = params;
	this->query = query;
	this->fragment = fragment;
	this->originalString = g_strdup (uri);
	this->isAbsolute = isAbsolute;
	
	return true;
}

void
Uri::Combine (const char *relative_path)
{
	const char *filename;
	char *new_path;
	
	if (path && relative_path[0] != '/') {
		if (!(filename = strrchr (path, '/')))
			new_path = g_strdup (relative_path);
		else
			new_path = g_strdup_printf ("%.*s/%s", filename - path, path, g_str_has_prefix (relative_path, "../") ? relative_path+3 : relative_path);
		g_free (path);
		
		path = new_path;
	} else {
		g_free (path);
		
		path = g_strdup (relative_path);
	}
}

void
Uri::Combine (const Uri *relative_uri)
{
	if (relative_uri->isAbsolute)
		g_warning ("Uri::Combine (): Not a relative Uri");
	Combine (relative_uri->path);
}

static void
append_url_encoded (GString *string, const char *in, const char *extra)
{
	register const char *inptr = in;
	const char *start;
	
	while (*inptr) {
		start = inptr;
		while (*inptr && !is_unsafe (*inptr) && !strchr (extra, *inptr))
			inptr++;
		
		g_string_append_len (string, start, inptr - start);
		
		while (*inptr && (is_unsafe (*inptr) || strchr (extra, *inptr)))
			g_string_append_printf (string, "%%%.02hx", *inptr++);
	}
}

static void
append_param (GQuark key_id, gpointer value, gpointer user_data)
{
	GString *string = (GString *) user_data;
	
	g_string_append_c (string, ';');
	append_url_encoded (string, g_quark_to_string (key_id), "?=#");
	if (*((char *) value)) {
		g_string_append_c (string, '=');
		append_url_encoded (string, (const char *) value, "?;#");
	}
}

char *
Uri::ToString (UriToStringFlags flags) const
{
	GString *string;
	char *uri;
	
	string = g_string_new ("");
	
	if (this->host) {
		g_string_append (string, this->scheme);
		g_string_append (string, "://");
		
		if (this->user) {
			append_url_encoded (string, this->user, ":;@/");
			
			if (this->auth) {
				g_string_append (string, ";auth=");
				append_url_encoded (string, this->auth, ":@/");
			}
			
			if (this->passwd && !(flags & UriHidePasswd)) {
				g_string_append_c (string, ':');
				append_url_encoded (string, this->passwd, "@/");
			}
			
			g_string_append_c (string, '@');
		}
		
		g_string_append (string, this->host);
		
		if (this->port > 0)
			g_string_append_printf (string, ":%d", this->port);
	}
	
	if (this->path) {
		if (this->host && *this->path != '/')
			g_string_append_c (string, '/');
		
		append_url_encoded (string, this->path, ";?#");
	} else if (this->host && (this->params || this->query || this->fragment)) {
		g_string_append_c (string, '/');
	}
	
	if (this->params)
		g_datalist_foreach ((GData **) &this->params, append_param, string);
	
	if (this->query) {
		g_string_append_c (string, '?');
		append_url_encoded (string, this->query, "#");
	}
	
	if (this->fragment && !(flags & UriHideFragment)) {
		g_string_append_c (string, '#');
		append_url_encoded (string, this->fragment, "");
	}
	
	uri = string->str;
	g_string_free (string, false);
	
	return uri;
}

bool 
Uri::operator== (const Uri &v) const
{
	if (isAbsolute != v.isAbsolute)
		return false;
	if (port != v.port)
		return false;
	if (!!scheme != !!v.scheme
	    || (scheme && strcmp (scheme, v.scheme)))
		return false;
	if (!!user != !!v.user
	    || (user && strcmp (user, v.user)))
		return false;
	if (!!auth != !!v.auth
	    || (auth && strcmp (auth, v.auth)))
		return false;
	if (!!passwd != !!v.passwd
	    || (passwd && strcmp (passwd, v.passwd)))
		return false;
	if (!!host != !!v.host
	    || (host && strcmp (host, v.host)))
		return false;
	if (!!path != !!v.path
	    || (path && strcmp (path, v.path)))
		return false;
	if (!!query != !!v.query
	    || (query && strcmp (query, v.query)))
		return false;
	if (!!fragment != !!v.fragment
	    || (fragment && strcmp (fragment, v.fragment)))
		return false;
	
	// XXX we don't compare this->data vs v.data yet.
	
	// we intentionally don't compare original strings
	return true;
}

bool
Uri::Equals (const Uri *left, const Uri *right)
{
	if (!left && !right)
		return true;
	if (!left || !right)
		return false;
	return left->operator==(*right);
}

bool
Uri::IsNullOrEmpty (const Uri *uri)
{
	if (!uri || (uri->scheme == NULL && uri->user == NULL && uri->auth == NULL &&
	    uri->passwd == NULL && uri->host == NULL && uri->port == 0 && uri->path == NULL
	    && uri->params == NULL && uri->query == NULL && uri->fragment == NULL && 
	    strcmp (uri->originalString, "") == 0 && !uri->isAbsolute))
		return true;

	return false;
}

guint
Uri::GetHashCode ()
{
	char* str = ToString();
	guint hash = g_str_hash (str);
	g_free (str);
	return hash;
}

bool
Uri::IsScheme (const char *scheme)
{
	if (!!this->scheme != !!scheme)
		return false;

	if (this->scheme)
		return !g_ascii_strcasecmp (this->scheme, scheme);

	return true;
}

bool 
Uri::SameSiteOfOrigin (const Uri *left, const Uri *right)
{
	// works only on absolute URI
	if (!left || !left->isAbsolute || !right || !right->isAbsolute)
		return false;

	if (left->port != right->port)
		return false;

	if (!left->scheme || !right->scheme || (strcmp (left->scheme, right->scheme) != 0))
		return false;

	// comparing 2 file:/// URI where no hosts is present
	if (!left->host && !right->host && (strcmp (left->scheme, "file") == 0))
		return true;

	if (!left->host || !right->host || (strcmp (left->host, right->host) != 0))
		return false;

	return true;
}

