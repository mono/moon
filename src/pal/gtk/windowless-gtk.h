/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless-gtk.h: Windowless (NPAPI) implementation for gtk using browsers.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOWLESS_GTK_H__
#define __MOON_WINDOWLESS_GTK_H__

#include "window-gtk.h"
#include "moonlight.h"
#include "runtime.h"
#include "plugin.h"

using namespace Moonlight;

/* @Namespace=System.Windows */
class MoonWindowlessGtk : public MoonWindowGtk {
public:
	MoonWindowlessGtk (int w = -1, int h = -1, PluginInstance *plugin = NULL);

	virtual ~MoonWindowlessGtk ();

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
	VisualID visualid;
	int x;
	int y;
	
	void UpdateWindowInfo ();
};

#endif // __MOON_WINDOWLESS_GTK_H__
