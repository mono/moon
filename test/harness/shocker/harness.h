/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * harness.h: Interface with our managed test harness
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


#ifndef __HARNESS_H__
#define __HARNESS_H__

#include <glib.h>

bool send_harness_message (const char *msg, guint8 **buffer, guint32 *output_length);

#endif /* __HARNESS_H__ */

