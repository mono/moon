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

#include <gtk/gtk.h>

#define Visual _XxVisual
#define Region _XxRegion
#include <gdk/gdkx.h>
#undef Visual
#undef Region

class MoonWindow;

#include "moonbuild.h"
#include "rect.h"
#include "enums.h"
#include "color.h"

class Surface;

/* @Namespace=System.Windows */
class MOON_API MoonWindow {
 public:
	MoonWindow (int w, int h, Surface *s = NULL) : width(w), height(h), surface(s), transparent(false) { }

	virtual ~MoonWindow () { }

	virtual void Resize (int width, int height) = 0;

	virtual void SetCursor (MouseCursor cursor) = 0;
	virtual void Invalidate (Rect r) = 0;
	virtual void Invalidate () { Invalidate (Rect (0, 0, width, height)); }
	virtual void ProcessUpdates () = 0;

	virtual gboolean HandleEvent (XEvent *event) = 0;

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

	virtual bool IsFullScreen () = 0;

	virtual GdkWindow* GetGdkWindow () = 0;

	void SetCurrentDeployment ();

 protected:
	int width;
	int height;
	Surface *surface;
	bool transparent;
};

#endif /* __MOON_WINDOW__ */
