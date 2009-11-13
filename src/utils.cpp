/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * utils.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib/gstdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "utils.h"
#include "application.h"
#include "deployment.h"

static gpointer
managed_stream_open (gpointer context, const char *filename, int mode)
{
	// minizip expects to get a FILE* here, we'll just shuffle our context around.

	return context;
}

static unsigned long
managed_stream_read (gpointer context, gpointer stream, void *buf, unsigned long size)
{
	ManagedStreamCallbacks *s = (ManagedStreamCallbacks *) context;
	unsigned long left = size;
	unsigned long nread = 0;
	int n;
	
	do {
		if ((n = s->Read (s->handle, (char *) buf + nread, 0, MIN (left, G_MAXINT32))) <= 0)
			break;
		
		nread += n;
		left -= n;
	} while (nread < size);
	
	return nread;
}

static unsigned long
managed_stream_write (gpointer context, gpointer stream, const void *buf, unsigned long size)
{
	ManagedStreamCallbacks *s = (ManagedStreamCallbacks *) context;
	unsigned long nwritten = 0;
	unsigned long left = size;
	int n;
	
	do {
		n = MIN (left, G_MAXINT32);
		s->Write (s->handle, (char *) buf + nwritten, 0, n);
		nwritten += n;
		left -= n;
	} while (nwritten < size);
	
	return nwritten;
}

static long
managed_stream_tell (gpointer context, gpointer stream)
{
	ManagedStreamCallbacks *s = (ManagedStreamCallbacks *) context;

	return s->Position (s->handle);
}

static long
managed_stream_seek (gpointer context, gpointer stream, unsigned long offset, int origin)
{
	ManagedStreamCallbacks *s = (ManagedStreamCallbacks *) context;

	s->Seek (s->handle, offset, origin);

	return 0;
}

static int
managed_stream_close (gpointer context, gpointer stream)
{
	return 0;
}

static int
managed_stream_error (gpointer context, gpointer stream)
{
	return 0;
}


gboolean
managed_unzip_stream_to_stream_first_file (ManagedStreamCallbacks *source, ManagedStreamCallbacks *dest)
{
	zlib_filefunc_def funcs;
	unzFile zipFile;
	gboolean ret;

	ret = FALSE;

	funcs.zopen_file = managed_stream_open;
	funcs.zread_file = managed_stream_read;
	funcs.zwrite_file = managed_stream_write;
	funcs.ztell_file = managed_stream_tell;
	funcs.zseek_file = managed_stream_seek;
	funcs.zclose_file = managed_stream_close;
	funcs.zerror_file = managed_stream_error;
	funcs.opaque = source;

	zipFile = unzOpen2 (NULL, &funcs);

	if (!zipFile)
		return FALSE;

	if (unzGoToFirstFile (zipFile) != UNZ_OK)
		goto cleanup;	

	if (unzOpenCurrentFile (zipFile) != UNZ_OK)
		goto cleanup;

	ret = managed_unzip_extract_to_stream (zipFile, dest);

cleanup:
	unzCloseCurrentFile (zipFile);
	unzClose (zipFile);

	return ret;
}

gboolean
managed_unzip_stream_to_stream (ManagedStreamCallbacks *source, ManagedStreamCallbacks *dest, const char *partname)
{
	zlib_filefunc_def funcs;
	unzFile zipFile;
	gboolean ret;

	ret = FALSE;

	funcs.zopen_file = managed_stream_open;
	funcs.zread_file = managed_stream_read;
	funcs.zwrite_file = managed_stream_write;
	funcs.ztell_file = managed_stream_tell;
	funcs.zseek_file = managed_stream_seek;
	funcs.zclose_file = managed_stream_close;
	funcs.zerror_file = managed_stream_error;
	funcs.opaque = source;

	zipFile = unzOpen2 (NULL, &funcs);

	if (!zipFile)
		return FALSE;

	if (unzLocateFile (zipFile, partname, 2) != UNZ_OK)
		goto cleanup;	

	if (unzOpenCurrentFile (zipFile) != UNZ_OK)
		goto cleanup;

	ret = managed_unzip_extract_to_stream (zipFile, dest);

cleanup:
	unzCloseCurrentFile (zipFile);
	unzClose (zipFile);

	return ret;
}

gboolean
managed_unzip_extract_to_stream (unzFile zipFile, ManagedStreamCallbacks *dest)
{
	char buf[4096];
	int nread;

	do {
		if ((nread = unzReadCurrentFile (zipFile, buf, sizeof (buf))) > 0) {
			dest->Write (dest->handle, buf, 0, nread);
		}
	} while (nread > 0);

	return TRUE;
}

/**
 * MID:
 * @lo: the low bound
 * @hi: the high bound
 *
 * Finds the midpoint between positive integer values, @lo and @hi.
 *
 * Notes: Typically expressed as '(@lo + @hi) / 2', this is incorrect
 * when @lo and @hi are sufficiently large enough that combining them
 * would overflow their integer type. To work around this, we use the
 * formula, '@lo + ((@hi - @lo) / 2)', thus preventing this problem
 * from occuring.
 *
 * Returns the midpoint between @lo and @hi (rounded down).
 **/
#define MID(lo, hi) (lo + ((hi - lo) >> 1))


static guint
bsearch (GPtrArray *array, bool stable, GCompareFunc cmp, void *item)
{
	register guint lo, hi;
	guint m;
	int c;
	
	if (array->len == 0)
		return 0;
	
	lo = 0, hi = array->len;
	
	do {
		m = MID (lo, hi);
		if ((c = cmp (&item, &array->pdata[m])) > 0) {
			lo = m + 1;
			m = lo;
		} else if (c < 0) {
			hi = m;
		} else if (stable) {
			lo = m + 1;
			m = lo;
		} else {
			break;
		}
	} while (lo < hi);
	
	return m;
}

void
g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item)
{
	guint index = bsearch (array, true, cmp, item);
	
	g_ptr_array_insert (array, index, item);
}

void
g_ptr_array_insert (GPtrArray *array, guint index, void *item)
{
	guint8 *dest, *src;
	guint n;
	
	if (index >= array->len) {
		g_ptr_array_add (array, item);
		return;
	}
	
	g_ptr_array_set_size (array, array->len + 1);
	
	dest = ((guint8 *) array->pdata) + (sizeof (void *) * (index + 1));
	src = ((guint8 *) array->pdata) + (sizeof (void *) * index);
	n = array->len - index - 1;
	
	memmove (dest, src, (sizeof (void *) * n));
	array->pdata[index] = item;
}

int
write_all (int fd, char *buf, size_t len)
{
	size_t nwritten = 0;
	ssize_t n;
	
	do {
		do {
			n = write (fd, buf + nwritten, len - nwritten);
		} while (n == -1 && errno == EINTR);
		
		if (n == -1)
			return -1;
		
		nwritten += n;
	} while (nwritten < len);
	
	return 0;
}

static bool
is_dll_or_mdb (const char *filename, int n)
{
	return n > 4 && (!g_ascii_strcasecmp (filename + (n - 4), ".dll") || !g_ascii_strcasecmp (filename + (n - 4), ".mdb"));
}

const char *
CanonicalizeFilename (char *filename, int n, CanonMode mode)
{
	char *inptr = filename;
	char *inend;
	
	if (n < 0)
		n = strlen (filename);
	
	inend = inptr + n;
	
	if (mode == CanonModeXap && is_dll_or_mdb (filename, n)) {
		// Note: We don't want to change the casing of the dll's
		// basename since Mono requires the basename to match the
		// expected case.
		
		inend -= 5;
		while (inend > inptr && *inend != '\\' && *inend != '/')
			inend--;
		
		if (*inend == '\\') {
			// include the trailing '\\'
			inend++;
		}
	}
	
	while (inptr < inend) {
		if (*inptr != '\\') {
			if (mode != CanonModeNone)
				*inptr = g_ascii_tolower (*inptr);
		} else
			*inptr = G_DIR_SEPARATOR;
		
		inptr++;
	}
	
	return filename;
}

bool
ExtractFile (unzFile zip, int fd)
{
	char buf[4096];
	int nread;
	ssize_t n;
	
	do {
		n = 0;
		if ((nread = unzReadCurrentFile (zip, buf, sizeof (buf))) > 0) {
			if ((n = write_all (fd, buf, nread)) == -1)
				break;
		}
	} while (nread > 0);
	
	if (nread != 0 || n == -1 /*|| fsync (fd) == -1*/) {
		close (fd);
		
		return false;
	}
	
	close (fd);
	
	return true;
}

bool
ExtractAll (unzFile zip, const char *dir, CanonMode mode)
{
	char *filename, *dirname, *path, *altpath;
	unz_file_info info;
	int fd;
	
	if (unzGoToFirstFile (zip) != UNZ_OK)
		return false;
	
	do {
		unzGetCurrentFileInfo (zip, &info, NULL, 0, NULL, 0, NULL, 0);
		if (info.external_fa & (1 << 4))
			continue;
		
		if (!(filename = (char *) g_malloc (info.size_filename + 1)))
			return false;
		
		unzGetCurrentFileInfo (zip, NULL, filename, info.size_filename + 1, NULL, 0, NULL, 0);
		
		CanonicalizeFilename (filename, info.size_filename, mode);
		
		path = g_build_filename (dir, filename, NULL);
		
		dirname = g_path_get_dirname (path);
		if (g_mkdir_with_parents (dirname, 0700) == -1 && errno != EEXIST) {
			g_free (filename);
			g_free (dirname);
			g_free (path);
			return false;
		}
		
		g_free (dirname);
		
		if ((fd = g_open (path, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1) {
			g_free (filename);
			g_free (path);
			return false;
		}
		
		if (unzOpenCurrentFile (zip) != UNZ_OK) {
			g_free (filename);
			g_free (path);
			close (fd);
			return false;
		}
		
		if (!ExtractFile (zip, fd)) {
			unzCloseCurrentFile (zip);
			g_free (filename);
			g_free (path);
			return false;
		}
		
		unzCloseCurrentFile (zip);
		
		if (mode == CanonModeXap && is_dll_or_mdb (filename, info.size_filename)) {
			CanonicalizeFilename (filename, info.size_filename, CanonModeResource);
			altpath = g_build_filename (dir, filename, NULL);
			if (strcmp (path, altpath) != 0)
				symlink (path, altpath);
			g_free (altpath);
		}
		
		g_free (filename);
		g_free (path);
	} while (unzGoToNextFile (zip) == UNZ_OK);
	
	return true;
}

char *
MakeTempDir (char *tmpdir)
{
	char *xxx;
	int attempts = 0;
	size_t n;
	
	if ((n = strlen (tmpdir)) < 6) {
		errno = EINVAL;
		return NULL;
	}
	
	xxx = tmpdir + (n - 6);
	if (strcmp (xxx, "XXXXXX") != 0) {
		errno = EINVAL;
		return NULL;
	}
	
	do {

		if (!mktemp (tmpdir))
			return NULL;
		
		if (g_mkdir (tmpdir, 0700) != -1)
			return tmpdir;
		
		if (errno != EEXIST) {
			// don't bother trying again...
			return NULL;
		}
		
		// that path already exists, try a new one...
		strcpy (xxx, "XXXXXX");
		attempts++;
	} while (attempts < 100);
	
	return NULL;
}


static int
rmdir_real (GString *path)
{
	const gchar *dirname;
	struct stat st;
	size_t len;
	GDir *dir;
	
	if (!(dir = g_dir_open (path->str, 0, NULL)))
		return -1;
	
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	while ((dirname = g_dir_read_name (dir))) {
		if (!strcmp (dirname, ".") || !strcmp (dirname, ".."))
			continue;
		
		g_string_truncate (path, len);
		g_string_append (path, dirname);
		
		if (g_lstat (path->str, &st) == -1)
			continue;
		
		if (S_ISDIR (st.st_mode))
			rmdir_real (path);
		else
			g_unlink (path->str);
	}
	
	g_dir_close (dir);
	
	g_string_truncate (path, len - 1);
	
	return g_rmdir (path->str);
}

//
// Creates a temporary directory, based on the @filename template
//
// Returns: a g-allocated string name that points to the created
// directory, or NULL on failure
//
char *
CreateTempDir (const char *filename)
{
	const char *name;
	char *path, *buf;
	
	if (!(name = strrchr (filename, '/')))
		name = filename;
	else
		name++;
	
	buf = g_strdup_printf ("%s.XXXXXX", name);

	if (Application::GetCurrent())
		path = g_build_filename (Application::GetCurrent()->GetResourceRoot(), buf, NULL);
	else {
		path = g_build_filename (g_get_tmp_dir (), buf, NULL);
		Deployment::GetCurrent()->TrackPath (path);
	}
	g_free (buf);
	
	if (!MakeTempDir (path)) {
		g_free (path);
		return NULL;
	}
	
	return path;
}

int
RemoveDir (const char *dir)
{
	GString *path;
	int rv;
	
	path = g_string_new (dir);
	rv = rmdir_real (path);
	g_string_free (path, true);
	
	return rv;
}

int
CopyFileTo (const char *filename, int fd)
{
	char buf[4096];
	ssize_t nread;
	int in;
	
	if ((in = g_open (filename, O_RDONLY)) == -1) {
		close (fd);
		return -1;
	}
	
	do {
		do {
			nread = read (in, buf, sizeof (buf));
		} while (nread == -1 && errno == EINTR);
		
		if (nread == -1)
			goto exception;
		
		if (nread == 0)
			break;
		
		if (write_all (fd, buf, nread) == -1)
			goto exception;
	} while (true);
	
	if (fsync (fd) == -1)
		goto exception;
	
	close (in);
	close (fd);
	
	return 0;
	
exception:
	
	close (in);
	close (fd);
	
	return -1;
}

cairo_t*
measuring_context_create (void)
{
	cairo_surface_t* surf = cairo_image_surface_create (CAIRO_FORMAT_A1, 1, 1);
	return cairo_create (surf);
}

void
measuring_context_destroy (cairo_t *cr)
{
	cairo_surface_destroy (cairo_get_target (cr));
	cairo_destroy (cr);
}


static ssize_t
read_internal (int fd, char *buf, size_t n)
{
	ssize_t nread;
	
	do {
		nread = read (fd, buf, n);
	} while (nread == -1 && errno == EINTR);
	
	return nread;
}


TextStream::TextStream ()
{
	cd = (GIConv) -1;
	bufptr = buffer;
	buflen = 0;
	fd = -1;
	
	eof = true;
}

TextStream::~TextStream ()
{
	if (fd != -1)
		close (fd);
	
	if (cd != (GIConv) -1) {
		g_iconv_close (cd);
		cd = (GIConv) -1;
	}
}

#define BOM ((gunichar2) 0xFEFF)
#define ANTIBOM ((gunichar2) 0xFFFE)

enum Encoding {
	UTF16_BE,
	UTF16_LE,
	UTF32_BE,
	UTF32_LE,
	UTF8,
	UNKNOWN,
};

static const char *encoding_names[] = { "UTF-16BE", "UTF-16LE", "UTF-32BE", "UTF-32LE", "UTF-8" };


bool
TextStream::OpenBuffer (const char *buf, int size)
{
	fmode = false;

	textbufptr = textbuf = (char *) buf;
	textbufsize = size;	

	if (size > 0)
		eof = false;

	return ReadBOM (false);
}

bool
TextStream::OpenFile (const char *filename, bool force)
{
	fmode = true;

	if (fd != -1)
		Close ();
	
	if ((fd = g_open (filename, O_RDONLY)) == -1)
		return false;

	return ReadBOM (force);
}

bool
TextStream::ReadBOM (bool force)
{
	Encoding encoding = UNKNOWN;
	gunichar2 bom;
	ssize_t nread;
	
	// prefetch the first chunk of data in order to determine encoding
	if ((nread = ReadInternal (buffer, sizeof (buffer))) == -1) {
		Close ();
		
		return false;
	}
	
	bufptr = buffer;
	buflen = nread;
	
	if (nread >= 2) {
		memcpy (&bom, buffer, 2);
		switch (bom) {
		case ANTIBOM:
			encoding = UTF16_BE;
			buflen -= 2;
			bufptr += 2;
			break;
		case BOM:
			encoding = UTF16_LE;
			buflen -= 2;
			bufptr += 2;
			break;
		case 0:
			if (nread >= 4) {
				memcpy (&bom, buffer + 2, 2);
				if (bom == ANTIBOM) {
					encoding = UTF32_BE;
					buflen -= 4;
					bufptr += 4;
				} else if (bom == BOM) {
					encoding = UTF32_LE;
					buflen -= 4;
					bufptr += 4;
				}
			}
			break;
		default:
			encoding = UTF8;
			break;
		}
	} else {
		// assume utf-8
		encoding = UTF8;
	}
	
	if (encoding == UNKNOWN) {
		if (!force) {
			Close ();
			
			return false;
		}
		
		encoding = UTF8;
	}
	
	if (encoding != UTF8 && (cd = g_iconv_open ("UTF-8", encoding_names[encoding])) == (GIConv) -1) {
		Close ();
		
		return false;
	}
	
	eof = false;
	
	return true;
}

void
TextStream::Close ()
{
	if (fd != -1) {
		close (fd);
		fd = -1;
	}
	
	if (cd != (GIConv) -1) {
		g_iconv_close (cd);
		cd = (GIConv) -1;
	}
	
	bufptr = buffer;
	buflen = 0;
	eof = true;
}

bool
TextStream::Eof ()
{
	return eof && buflen == 0;
}

ssize_t
TextStream::Read (char *buf, size_t n)
{
	size_t inleft = buflen;
	char *inbuf = bufptr;
	char *outbuf = buf;
	size_t outleft = n;
	ssize_t nread;
	size_t r;
	
	do {
		if (cd != (GIConv) -1) {
			if (g_iconv (cd, &inbuf, &inleft, &outbuf, &outleft) == (size_t) -1) {
				switch (errno) {
				case E2BIG:
					// not enough space available in the output buffer
					goto out;
				case EINVAL:
					// incomplete multibyte character sequence
					goto out;
				case EILSEQ:
					// illegal multibyte sequence
					return -1;
				default:
					// unknown error, fail
					return -1;
				}
			}
		} else {
			r = MIN (inleft, outleft);
			memcpy (outbuf, inbuf, r);
			outleft -= r;
			outbuf += r;
			inleft -= r;
			inbuf += r;
		}
		
		if (outleft == 0 || eof)
			break;
		
		// buffer more data
		if (inleft > 0)
			memmove (buffer, inbuf, inleft);
		
		inbuf = buffer + inleft;
		if ((nread = ReadInternal (inbuf, sizeof (buffer) - inleft)) <= 0) {
			eof = true;
			break;
		}
		
		inleft += nread;
		inbuf = buffer;
	} while (true);
	
	if (eof && cd != (GIConv) -1)
		g_iconv (cd, NULL, NULL, &outbuf, &outleft);
	
out:
	
	buflen = inleft;
	bufptr = inbuf;
	
	return (outbuf - buf);
}

ssize_t
TextStream::ReadInternal (char *buffer, ssize_t size)
{
	if (fmode) {
		return read_internal (fd, buffer, size);
	} else {
		ssize_t nread = size;

		if (eof)
			return -1;

		if (textbufptr + size > textbuf + textbufsize) {
			eof = true;
			nread = textbuf + textbufsize - textbufptr;
		}
		memcpy (buffer, textbufptr, nread);

		textbufptr += nread;
				
		return nread;
	}
}


GArray *double_garray_from_str (const char *s, gint max)
{
	char *next = (char *)s;
	GArray *values = g_array_sized_new (false, true, sizeof (double), max > 0 ? max : 16);
	double coord = 0.0;
	guint end = max > 0 ? max : G_MAXINT;

	while (next && values->len < end) {
		while (g_ascii_isspace (*next) || *next == ',')
			next = g_utf8_next_char (next);
		
		if (next) {
			errno = 0;
			char *prev = next;
			coord = g_ascii_strtod (prev, &next);
			if (errno != 0 || next == prev)
				goto error;

			g_array_append_val (values, coord);
		}
	}

error:
	while (values->len < (guint) max) {
		coord = 0.0;
		g_array_append_val (values, coord);
	}

	return values;
}

char * parse_rfc_1945_quoted_string (char *input, char *c, char **end)
{
	char *start;
	
	if (input == NULL || input [0] != '"')
		return NULL;
	
	*end = NULL;

	input++;
	start = input;
	
	do {
/*
 *	We're parsing a quoted-string according to RFC 1945:
 *	
 *       quoted-string  = ( <"> *(qdtext) <"> )
 *
 *       qdtext         = <any CHAR except <"> and CTLs,
 *                        but including LWS>
 *
 *       LWS            = [CRLF] 1*( SP | HT )
 *       CTL            = <any US-ASCII control character
 *                        (octets 0 - 31) and DEL (127)>
 *
 */
		char h = *input;
		*c = h;
				
		// LWS
		if (h == 10 || h == 13 || h == ' ' || h == 9)
			continue;
		
		// CTL
		if (h == 0)
			return start;
		
		if ((h > 0 && h <= 31) || h == 127) {
			*end = input + 1;
			*input = 0;
			return start;
		}
		
		// quote
		if (h == '"') {
			*end = input + 1;
			*input = 0;
			return start;
		}
		
	} while (*(input++));
	
	return start;
}

char *parse_rfc_1945_token (char *input, char *c, char **end)
{
	char *start = input;
	bool first = false;
	
	if (input == NULL || c == NULL || end == NULL)
		return NULL;
		
	*c = 0;
	*end = NULL;
	
	
	do {
/*
 *	We're parsing a token according to RFC 1945:
 *	
 *       token          = 1*<any CHAR except CTLs or tspecials>
 *
 *       tspecials      = "(" | ")" | "<" | ">" | "@"
 *                      | "," | ";" | ":" | "\" | <">
 *                      | "/" | "[" | "]" | "?" | "="
 *                      | "{" | "}" | SP | HT
 *
 *       CTL            = <any US-ASCII control character
 *                        (octets 0 - 31) and DEL (127)>
 *       SP             = <US-ASCII SP, space (32)>
 *       HT             = <US-ASCII HT, horizontal-tab (9)>
 *
 */
		char h = *input;
		*c = h;
		
		// CTL
		if (h == 0)
			return start;
			
		if ((h > 0 && h <= 31) || h == 127) {
			if (!first) {
				start = input + 1;
				continue;
			}
			*end = input + 1;
			*input = 0;
			continue;
		}
		
		// tspecials
		switch (h) {
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':
		case '{':
		case '}':
		case 32:
		case 9:
			if (!first) {
				start = input + 1;
				continue;
			}
			*end = input + 1;
			*input = 0;
			return start;		
		}
		first = true;
	} while (*(input++));
	
	
	// reached the end of the input, only one token
	return start;
}

Cancellable::Cancellable ()
{
	cancel_cb = NULL;
	downloader = NULL;
}

Cancellable::~Cancellable ()
{
	if (downloader)
		downloader->unref ();
}

void
Cancellable::Cancel ()
{
	if (cancel_cb)
		cancel_cb (downloader, context);
}

void
Cancellable::SetCancelFuncAndData (CancelCallback cb, Downloader *downloader, void *_context)
{
	cancel_cb = cb;
	context = _context;
	if (this->downloader)
		this->downloader->unref ();
	this->downloader = downloader;
	if (this->downloader)
		this->downloader->ref ();
}
