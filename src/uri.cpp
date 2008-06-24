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

#include <string.h>
#include <ctype.h>

#include "uri.h"


/* see rfc1738, section 2.2 */
#define is_unsafe(c) (((unsigned char) c) <= 0x1f || ((unsigned char) c) == 0x7f)


Uri::Uri ()
{
	protocol = NULL;
	user = NULL;
	auth = NULL;
	passwd = NULL;
	host = NULL;
	port = 0;
	path = NULL;
	params = NULL;
	query = NULL;
	fragment = NULL;
}

Uri::~Uri ()
{
	g_free (protocol);
	g_free (user);
	g_free (auth);
	g_free (passwd);
	g_free (host);
	g_free (path);
	g_datalist_clear (&params);
	g_free (query);
	g_free (fragment);
}

static void
clone_params (GQuark quark, gpointer data, gpointer user_data)
{
	gchar *str = (gchar *) data;
	GData **params = (GData **) user_data;
	g_datalist_id_set_data_full (params, quark, g_strdup (str), g_free);
}

Uri *
Uri::Clone ()
{
	Uri *uri = new Uri ();
	uri->protocol = g_strdup (protocol);
	uri->user = g_strdup (user);
	uri->auth = g_strdup (auth);
	uri->passwd = g_strdup (passwd);
	uri->host = g_strdup (host);
	uri->path = g_strdup (path);
	if (params)
		g_datalist_foreach (&params, clone_params, &uri->params);
	uri->query = g_strdup (query);
	uri->fragment = g_strdup (fragment);
	uri->port = port;
	return uri;
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
	char *name, *value, *protocol, *user = NULL, *auth = NULL, *passwd = NULL, *host = NULL, *path = NULL, *query = NULL, *fragment = NULL;
	register const char *start, *inptr;
	GData *params = NULL;
	int port = 0;
	size_t n;
	
	start = uri;
	if (!(inptr = strchr (start, ':'))) {
		protocol = g_strdup ("file");
		inptr = uri;
		
		goto decode_path;
	}
	
	protocol = g_ascii_strdown (start, inptr - start);
	
	inptr++;
	if (!*inptr)
		goto done;
	
	if (!strncmp (inptr, "//", 2))
		inptr += 2;
	
	start = inptr;
	while (*inptr && *inptr != ';' && *inptr != ':' && *inptr != '@' && *inptr != '/')
		inptr++;
	
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
	
	if (*inptr == '/') {
	decode_path:
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
	
	// update the values
	
	g_free (this->protocol);
	this->protocol = protocol;
	
	g_free (this->user);
	this->user = user;
	
	g_free (this->auth);
	this->auth = auth;
	
	g_free (this->passwd);
	this->passwd = passwd;
	
	g_free (this->host);
	this->host = host;
	
	this->port = port;
	
	g_free (this->path);
	this->path = path;
	
	g_datalist_clear (&this->params);
	this->params = params;
	
	g_free (this->query);
	this->query = query;
	
	g_free (this->fragment);
	this->fragment = fragment;
	
	return true;
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
Uri::ToString (UriToStringFlags flags)
{
	GString *string;
	char *uri;
	
	string = g_string_new ("");
	
	if (this->host) {
		g_string_append (string, this->protocol);
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
		g_datalist_foreach (&this->params, append_param, string);
	
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
