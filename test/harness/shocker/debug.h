/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __SHOCKER_DEBUG_H__
#define __SHOCKER_DEBUG_H__

#include <glib.h>

extern guint32 shocker_flags;

enum ShockerFlags {
	SHOCKER_DEBUG_HARNESS  = 1 << 0,
	SHOCKER_DEBUG_CAPTURE  = 1 << 1,
	SHOCKER_DEBUG_INPUT    = 1 << 2,
	SHOCKER_DEBUG_PLUGIN   = 1 << 3,
	SHOCKER_DEBUG_SHUTDOWN = 1 << 4,
	SHOCKER_DEBUG_CLIPBOARD = 1 << 5,
};

#define LOG_HARNESS(...)	if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_HARNESS))  printf (__VA_ARGS__);
#define LOG_CAPTURE(...)	if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_CAPTURE))  printf (__VA_ARGS__);
#define LOG_INPUT(...)		if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_INPUT))    printf (__VA_ARGS__);
#define LOG_PLUGIN(...)		if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_PLUGIN))   printf (__VA_ARGS__);
#define LOG_SHUTDOWN(...)	if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_SHUTDOWN)) printf (__VA_ARGS__);
#define LOG_CLIPBOARD(...)  if (G_UNLIKELY (shocker_flags & SHOCKER_DEBUG_CLIPBOARD)) printf (__VA_ARGS__);

void shocker_debug_initialize () __attribute__ ((constructor));

#endif /* __SHOCKER_DEBUG_H__ */
