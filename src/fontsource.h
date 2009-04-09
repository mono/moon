/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontsource.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_SOURCE_H__
#define __FONT_SOURCE_H__

#include <glib.h>
#include <string.h>

#include "utils.h"

/* @IncludeInKinds */
struct FontSource {
	ManagedStreamCallbacks *stream;
};

#endif /* __FONT_SOURCE_H__ */
