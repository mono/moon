/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#ifndef __MOON_WINDOW__
#define __MOON_WINDOW__

#include "pal.h"
#include "rect.h"
#include "enums.h"
#include "color.h"
#include "deployment.h"

class MoonEvent;
class MoonWindowingSystem;

/* @Namespace=System.Windows */
class MoonWindow {
 public:
	// FIXME: do something with parentWindow here.
	MoonWindow (bool fullscreen, int width = -1, int height = -1, MoonWindow *parentWindow = NULL, Surface *s = NULL)
	: width(width), height(height), surface(s), fullscreen (fullscreen), transparent(false), windowingSystem(NULL) { }

	MoonWindow (int width = -1, int height = -1, PluginInstance *plugin = NULL)
	: width(width), height(height), surface(NULL), fullscreen (false), transparent(false), windowingSystem(NULL) { }

	virtual ~MoonWindow () { }

	virtual void ConnectToContainerPlatformWindow (gpointer container_window) = 0;

	virtual void Resize (int width, int height) = 0;

	virtual void SetCursor (CursorType cursor) = 0;
	virtual void Invalidate (Rect r) = 0;
	virtual void Invalidate () { Invalidate (Rect (0, 0, width, height)); }
	virtual void ProcessUpdates () = 0;

	virtual gboolean HandleEvent (gpointer platformEvent) = 0;

	virtual void Show () = 0;
	virtual void Hide () = 0;

	virtual void EnableEvents (bool first) = 0;
	virtual void DisableEvents () = 0;
	
	virtual void GrabFocus () = 0;
	virtual bool HasFocus () = 0;

	int GetWidth () { return width; }
	int GetHeight () { return height; }

	virtual void SetSurface (Surface* s) { surface = s; }
	Surface *GetSurface () { return surface; }

	/* @GenerateCBinding,GeneratePInvoke */
	void SetTransparent (bool flag) { if (transparent != flag) Invalidate (); transparent = flag; }
	
	virtual void SetBackgroundColor (Color *color) { Invalidate (); };
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetTransparent () { return transparent; }

	bool IsFullScreen () { return fullscreen; }

	void SetCurrentDeployment ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual MoonClipboard *GetClipboard (MoonClipboardType clipboardType) = 0;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual gpointer GetPlatformWindow () = 0;

 protected:
	int width;
	int height;
	Surface *surface;
	bool fullscreen;
	bool transparent;

	MoonWindowingSystem* windowingSystem;
};

#endif /* __MOON_WINDOW__ */
