/*
 * utils.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include "utils.h"



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


static int
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
	guint ins = bsearch (array, true, cmp, item);
	
	g_ptr_array_set_size (array, array->len + 1);
	
	if (ins < array->len) {
		uint8_t *dest = ((uint8_t *) array->pdata) + (sizeof (void *) * (ins + 1));
		uint8_t *src = ((uint8_t *) array->pdata) + (sizeof (void *) * ins);
		guint n = array->len - ins - 1;
		
		g_memmove (dest, src, (sizeof (void *) * n));
	}
	
	array->pdata[ins] = item;
}

static ssize_t
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
	
	return nwritten;
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
	
	if (nread != 0 || n == -1 || fsync (fd) == -1) {
		close (fd);
		
		return false;
	}
	
	close (fd);
	
	return true;
}


char *
make_tmpdir (char *tmpdir)
{
	char *path, *xxx;
	size_t n;
	
	if ((n = strlen (tmpdir)) < 6)
		return NULL;
	
	xxx = tmpdir + (n - 6);
	if (strcmp (xxx, "XXXXXX") != 0)
		return NULL;
	
	do {
		if (!(path = mktemp (tmpdir)))
			return NULL;
		
		if (g_mkdir_with_parents (tmpdir, 0700) != -1)
			return tmpdir;
		
		if (errno != EEXIST) {
			// don't bother trying again...
			return NULL;
		}
		
		// that path already exists, try a new one...
		strcpy (xxx, "XXXXXX");
	} while (1);
	
	return NULL;
}


static int
moon_rmdir_real (GString *path)
{
	struct dirent *dent;
	struct stat st;
	size_t len;
	DIR *dir;
	
	if (!(dir = opendir (path->str)))
		return -1;
	
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	while ((dent = readdir (dir))) {
		if (!strcmp (dent->d_name, ".") || !strcmp (dent->d_name, ".."))
			continue;
		
		g_string_truncate (path, len);
		g_string_append (path, dent->d_name);
		
		if (lstat (path->str, &st) == -1)
			continue;
		
		if (S_ISDIR (st.st_mode))
			moon_rmdir_real (path);
		else
			unlink (path->str);
	}
	
	closedir (dir);
	
	g_string_truncate (path, len - 1);
	
	return rmdir (path->str);
}

int
moon_rmdir (const char *dir)
{
	GString *path;
	int rv;
	
	path = g_string_new (dir);
	rv = moon_rmdir_real (path);
	g_string_free (path, true);
	
	return rv;
}

int
moon_copy_file (const char *filename, int fd)
{
	char buf[4096];
	ssize_t nread;
	int in;
	
	if ((in = open (filename, O_RDONLY)) == -1)
		return -1;
	
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
TextStream::Open (const char *filename, bool force)
{
	Encoding encoding = UNKNOWN;
	gunichar2 bom;
	ssize_t nread;
	
	if (fd != -1)
		Close ();
	
	if ((fd = open (filename, O_RDONLY)) == -1)
		return false;
	
	// prefetch the first chunk of data in order to determine encoding
	if ((nread = read_internal (fd, buffer, sizeof (buffer))) == -1) {
		close (fd);
		fd = -1;
		
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
			close (fd);
			fd = -1;
			
			return false;
		}
		
		encoding = UTF8;
	}
	
	if (encoding != UTF8 && (cd = g_iconv_open ("UTF-8", encoding_names[encoding])) == (GIConv) -1) {
		close (fd);
		fd = -1;
		
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
	
	if (fd == -1)
		return -1;
	
	do {
		if (cd != (GIConv) -1) {
			errno = 0;
			r = g_iconv (cd, &inbuf, &inleft, &outbuf, &outleft);
			if (r == (size_t) -1 && errno == E2BIG) {
				// not enough room left in outbuf
				break;
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
		if ((nread = read_internal (fd, inbuf, sizeof (buffer) - inleft)) <= 0) {
			eof = true;
			break;
		}
		
		inleft += nread;
		inbuf = buffer;
	} while (true);
	
	if (eof && cd != (GIConv) -1)
		g_iconv (cd, NULL, NULL, &outbuf, &outleft);
	
	buflen = inleft;
	bufptr = inbuf;
	
	return (outbuf - buf);
}
