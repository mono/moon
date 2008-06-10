/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * utils.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __MOON_GARRAY_EXT_H__
#define __MOON_GARRAY_EXT_H__

#include <glib.h>
#include <cairo.h>

#include "zip/unzip.h"

G_BEGIN_DECLS

void g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item);

bool ExtractFile (unzFile zip, int fd);

char *CreateTempDir (const char *filename);

int RemoveDir (const char *dir);

int CopyFileTo (const char *filename, int fd);

cairo_t *measuring_context_create (void);
void     measuring_context_destroy (cairo_t *cr);

G_END_DECLS

class TextStream {
protected:
	char buffer[4096];
	size_t buflen;
	char *bufptr;
	GIConv cd;
	int fd;
	
	bool eof;
	
public:
	
	TextStream ();
	~TextStream ();
	
	bool Open (const char *filename, bool force);
	void Close ();
	
	bool Eof ();
	
	ssize_t Read (char *buf, size_t n);
};

#endif /* __MOON_GARRAY_EXT_H__ */
