/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-android.h: MoonWindow implementation using android widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOW_ANDROID_H__
#define __MOON_WINDOW_ANDROID_H__

#include <glib.h>

#include "window.h"
#include "runtime.h"

#ifdef USE_GALLIUM
struct pipe_screen;
#endif

namespace Moonlight {

class GLXSurface;
class GLXContext;

/* @Namespace=System.Windows */
class MoonWindowAndroid : public MoonWindow {
public:
	MoonWindowAndroid (MoonWindowType windowType, int w = -1, int h = -1, MoonWindow* parent = NULL, Surface *surface = NULL);

	virtual ~MoonWindowAndroid ();

	virtual void ConnectToContainerPlatformWindow (gpointer container_window);

	virtual void Resize (int width, int height);
	virtual void SetCursor (CursorType cursor);
	virtual void SetBackgroundColor (Color *color);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();
	virtual gboolean HandleEvent (gpointer platformEvent);
	virtual void Show ();
	virtual void Hide ();
	virtual void EnableEvents (bool first);
	virtual void DisableEvents ();

	virtual void SetLeft (double left);
	virtual double GetLeft ();

	virtual void SetTop (double top);
	virtual double GetTop ();

	virtual void SetWidth (double width);

	virtual void SetHeight (double height);

	virtual void SetTitle (const char *title);

	virtual void SetIconFromPixbuf (MoonPixbuf *pixbuf);

	virtual void SetStyle (WindowStyle style);

	virtual void GrabFocus ();
	virtual bool HasFocus ();

	virtual MoonClipboard *GetClipboard (MoonClipboardType clipboardType);

	virtual gpointer GetPlatformWindow ();

#ifdef USE_GALLIUM
	void SetGalliumScreen (pipe_screen *gscreen) { screen = gscreen; }
#endif

private:
	cairo_surface_t *native;

#ifdef USE_GALLIUM
	pipe_screen *screen;
	Context *gctx;
	static int gctxn;
#endif

	int left;
	int top;
};

};
#endif /* __MOON_WINDOW_ANDROID_H__ */
