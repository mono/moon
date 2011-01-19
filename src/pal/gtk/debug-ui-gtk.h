/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug-ui.h: debugging/inspection support for gtk+
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_DEBUG_UI_H__
#define __MOON_DEBUG_UI_H__

#include "window-gtk.h"

using namespace Moonlight;

G_BEGIN_DECLS

void show_debug (MoonWindowGtk *window);
void show_sources (MoonWindowGtk *window);
void debug_info (MoonWindowGtk *window);
#if OBJECT_TRACKING
void debug_media (MoonWindowGtk *window);
void dump_media_elements () __attribute__ ((used));
#endif

G_END_DECLS

#endif /* __MOON_DEBUG_UI_H__ */