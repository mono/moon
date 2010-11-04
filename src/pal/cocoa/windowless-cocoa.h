/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless-cocoa.h: Windowless (NPAPI) implementation for cocoa using browsers.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOWLESS_COCOA_H__
#define __MOON_WINDOWLESS_COCOA_H__

#include "window-cocoa.h"
#include "moonlight.h"
#include "runtime.h"
#include "plugin.h"

using namespace Moonlight;

/* @Namespace=System.Windows */
class MoonWindowlessCocoa : public MoonWindowCocoa {
public:
	MoonWindowlessCocoa (int w = -1, int h = -1, PluginInstance *plugin = NULL);

	virtual ~MoonWindowlessCocoa ();

	void ConnectToContainerPlatformWindow (gpointer container_window);

	virtual void Resize (int width, int height);

	virtual void SetCursor (CursorType cursor);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

	virtual gboolean HandleEvent (gpointer platformEvent);

	virtual void Show ();
	virtual void Hide ();

	virtual void EnableEvents (bool first);
	virtual void DisableEvents ();
	
	virtual void GrabFocus ();
	virtual bool HasFocus ();

	virtual void SetSurface (Surface *s);

	virtual bool IsFullScreen () { return false; }

	virtual gpointer GetPlatformWindow ();

private:
	PluginInstance *plugin;
	int x;
	int y;
	
	void UpdateWindowInfo ();
};

#endif // __MOON_WINDOWLESS_COCOA_H__
