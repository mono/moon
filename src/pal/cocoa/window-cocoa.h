/* -*- Mode: C++; tab-width: 8; indent-tabs-: t; c-basic-offset: 8 -*- */
/*
 * window-cocoa.h: MoonWindow implementation using cocoa widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOW_COCOA_H__
#define __MOON_WINDOW_COCOA_H__

#include <glib.h>

#include "window.h"
#include "runtime.h"

namespace Moonlight {

/* @Namespace=System.Windows */
class MoonWindowCocoa : public MoonWindow {
public:
	MoonWindowCocoa (MoonWindowType windowType, int w = -1, int h = -1, MoonWindow* parent = NULL, Surface *surface = NULL);

	virtual ~MoonWindowCocoa ();

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

	/* @GeneratePInvoke */
	void *GetNativeWidget () { return NULL; }  // same as GetWidget, just without bleeding CocoaWidget into the cbindings
	
	virtual MoonClipboard *GetClipboard (MoonClipboardType clipboardType);

	virtual gpointer GetPlatformWindow ();

	/* These bleed across the public API :( */
	void ExposeEvent (Rect r);
	void MouseEnteredEvent (gpointer evt);
	void MouseExitedEvent (gpointer evt);
	void MotionEvent (gpointer evt);
	void ButtonPressEvent (gpointer evt);
	void ButtonReleaseEvent (gpointer evt);
	void KeyDownEvent (gpointer evt);
	void KeyUpEvent (gpointer evt);

private:
	unsigned char *backing_image_data;
	void *window;
	void *view;
	cairo_surface_t *native;
	CairoContext *ctx;

	cairo_surface_t *CreateCairoSurface ();
};

};
#endif /* __MOON_WINDOW_COCOA_H__ */
