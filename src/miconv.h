/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * miconv.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MICONV_H__
#define __MICONV_H__

#include <glib.h>
#include <sys/types.h>

namespace Moonlight {

G_BEGIN_DECLS

typedef struct _miconv_t *miconv_t;

int miconv (miconv_t cd, char **inbytes, size_t *inbytesleft,
	    char **outbytes, size_t *outbytesleft);
miconv_t miconv_open (const char *to, const char *from);
int miconv_close (miconv_t cd);

G_END_DECLS

};

#endif /* __MICONV_H__ */
