/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless-cocoa.cpp: Windowsless Surface subclass for Cocoa
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <glib.h>

#include "windowless-cocoa.h"

#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

MoonWindowlessCocoa::MoonWindowlessCocoa (int w, int h, PluginInstance *plugin)
  : MoonWindowCocoa (MoonWindowType_Plugin, w, h)
{
	this->plugin = plugin;

	UpdateWindowInfo ();
}

MoonWindowlessCocoa::~MoonWindowlessCocoa ()
{
}

void
MoonWindowlessCocoa::ConnectToContainerPlatformWindow (gpointer container_window)
{
	// do nothing here
}

void
MoonWindowlessCocoa::UpdateWindowInfo ()
{
	// It appears opera doesn't do a good job of keeping the NPWindow members valid
	// between SetWindow calls so we have to work around this by copying the members
	// we need.  This is really ugly.

	NPWindow *window = plugin->GetWindow();
	NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct*)window->ws_info;
	x = window->x;
	y = window->y;
}

void
MoonWindowlessCocoa::Resize (int width, int height)
{
	bool emit_resize = false;

	UpdateWindowInfo ();

        if (this->width != width || this->height != height) {
		this->width = width;
		this->height = height;

		emit_resize = true;
	}

	surface->HandleUIWindowAllocation (emit_resize);
}

void
MoonWindowlessCocoa::SetCursor (CursorType cursor)
{
#if (NP_VERSION_MINOR >= NPVERS_HAS_WINDOWLESS_CURSORS)
	NPCursor npcursor;
	switch (cursor) {
	case CursorTypeDefault:
		npcursor = NPCursorAuto;
		break;
	case CursorTypeArrow:
		npcursor = NPCursorPointer;
		break;
	case CursorTypeWait:
		npcursor = NPCursorWait;
		break;
	case CursorTypeIBeam:
		npcursor = NPCursorText;
		break;
	case CursorTypeStylus:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case CursorTypeEraser:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case CursorTypeNone:
		// Silverlight display no cursor if the enumeration value is invalid (e.g. -1)
	default:
		npcursor = NPCursorNone;
		break;
	}

	MOON_NPN_SetValue (plugin->GetInstance(), NPPVcursor, (void*)npcursor);
#endif
}

void
MoonWindowlessCocoa::Invalidate (Rect r)
{
	NPRect nprect;

	// Mozilla gets seriously confused about invalidations 
	// outside the windowless bounds.
	r = r.Intersection (Rect (0, 0, GetWidth(), GetHeight())).RoundOut ();

	nprect.left = (uint16_t)r.x;
	nprect.top = (uint16_t)r.y;
	nprect.right = (uint16_t)(r.x + r.width);
	nprect.bottom = (uint16_t)(r.y + r.height);

	MOON_NPN_InvalidateRect (plugin->GetInstance(), &nprect);
}

void
MoonWindowlessCocoa::ProcessUpdates ()
{
	//MOON_NPN_ForceRedraw (plugin->GetInstance());
}

gboolean
MoonWindowlessCocoa::HandleEvent (gpointer platformEvent)
{
	g_assert_not_reached ();
}

void
MoonWindowlessCocoa::Show ()
{
	// nothing needed here
}

void
MoonWindowlessCocoa::Hide ()
{
	// nothing needed here
}

void
MoonWindowlessCocoa::EnableEvents (bool first)
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowlessCocoa::DisableEvents ()
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowlessCocoa::GrabFocus ()
{
	// we can't grab focus - the browser handles that.
}

bool
MoonWindowlessCocoa::HasFocus ()
{
	// XXX maybe we should track the focus in/out events?
	return false;
}

void
MoonWindowlessCocoa::SetSurface (Surface *s)
{
	MoonWindowCocoa::SetSurface (s);
	s->HandleUIWindowAvailable ();
}

gpointer
MoonWindowlessCocoa::GetPlatformWindow ()
{
	g_assert_not_reached ();
}
