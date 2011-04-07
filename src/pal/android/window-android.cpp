/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-gtk.cpp: MoonWindow implementation using gtk widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "window-android.h"
#include "clipboard-android.h"
#include "pixbuf-android.h"
#include "deployment.h"
#include "timemanager.h"
#include "enums.h"
#include "context-cairo.h"
#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#endif

// Gallium context cache size.
//
#define CONTEXT_CACHE_SIZE 1

using namespace Moonlight;

#ifdef USE_GALLIUM
int MoonWindowAndroid::gctxn = 0;
#endif

MoonWindowAndroid::MoonWindowAndroid (MoonWindowType windowType, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (windowType, w, h, parent, surface)
{
}

MoonWindowAndroid::~MoonWindowAndroid ()
{
}

void
MoonWindowAndroid::ConnectToContainerPlatformWindow (gpointer container_window)
{
}

MoonClipboard*
MoonWindowAndroid::GetClipboard (MoonClipboardType clipboardType)
{
	return new MoonClipboardAndroid (this, clipboardType);
}

gpointer
MoonWindowAndroid::GetPlatformWindow ()
{
	// FIXME
	return NULL;
}

void
MoonWindowAndroid::Resize (int width, int height)
{
	// FIXME
}

void
MoonWindowAndroid::SetBackgroundColor (Color *color)
{
	// FIXME
}

void
MoonWindowAndroid::SetCursor (CursorType cursor)
{
	// FIXME
}

void
MoonWindowAndroid::Invalidate (Rect r)
{
	// FIXME
}

void
MoonWindowAndroid::ProcessUpdates ()
{
	// FIXME
}

gboolean
MoonWindowAndroid::HandleEvent (gpointer platformEvent)
{
	// FIXME
	return TRUE;
}

void
MoonWindowAndroid::Show ()
{
	if (surface) {
		surface->HandleUIWindowUnavailable ();
		surface->HandleUIWindowAvailable ();
	}
}

void
MoonWindowAndroid::Hide ()
{
	if (surface)
		surface->HandleUIWindowAvailable ();
}

void
MoonWindowAndroid::EnableEvents (bool first)
{
	// FIXME
}

void
MoonWindowAndroid::DisableEvents ()
{
	// FIXME
}

void
MoonWindowAndroid::GrabFocus ()
{
	// FIXME
}

bool
MoonWindowAndroid::HasFocus ()
{
	// FIXME
	return false;
}

void
MoonWindowAndroid::SetLeft (double left)
{
	// FIXME
}

double
MoonWindowAndroid::GetLeft ()
{
	return left;
}

void
MoonWindowAndroid::SetTop (double top)
{
	// FIXME
}

double
MoonWindowAndroid::GetTop ()
{
	return top;
}

void
MoonWindowAndroid::SetWidth (double width)
{
	// FIXME
}

void
MoonWindowAndroid::SetHeight (double height)
{
	// FIXME
}

void
MoonWindowAndroid::SetTitle (const char *title)
{
	// FIXME
}

void
MoonWindowAndroid::SetIconFromPixbuf (MoonPixbuf *pixbuf)
{
	// FIXME
}

void
MoonWindowAndroid::SetStyle (WindowStyle style)
{
	// FIXME
}
