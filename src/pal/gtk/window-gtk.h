/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-gtk.h: MoonWindow implementation using gtk widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOW_GTK_H__
#define __MOON_WINDOW_GTK_H__

#include <glib.h>

#if GLIB_SIZEOF_VOID_P == 8
#define GDK_NATIVE_WINDOW_POINTER 1
#endif

#include <gtk/gtk.h>

#include "window.h"
#include "runtime.h"

#ifdef USE_GALLIUM
struct sw_winsys;
struct pipe_screen;
#endif

namespace Moonlight {

/* @Namespace=System.Windows */
class MoonWindowGtk : public MoonWindow {
public:
	MoonWindowGtk (MoonWindowType windowType, int w = -1, int h = -1, MoonWindow* parent = NULL, Surface *surface = NULL);

	virtual ~MoonWindowGtk ();

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

	GtkWidget* GetWidget() { return widget; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void *GetNativeWidget () { return GetWidget (); }  // same as GetWidget, just without bleeding GtkWidget into the cbindings
	
	virtual MoonClipboard *GetClipboard (MoonClipboardType clipboardType);

	virtual gpointer GetPlatformWindow ();

protected:
	unsigned char *backing_image_data;
	int backing_store_width;
	int backing_store_height;

	cairo_surface_t* CreateCairoSurface (GdkWindow *drawable, GdkVisual *visual, bool native, int width, int height);

	void PaintToDrawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y, bool transparent, bool clear_transparent);

	static gboolean container_button_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

private:
	GdkPixmap *backing_store;
	GdkGC *backing_store_gc;
	
	GtkWidget *widget;

	GtkWidget *container;

	Context *ctx;
	cairo_surface_t *native;

#ifdef USE_GALLIUM
	sw_winsys   *winsys;
	pipe_screen *screen;
#endif

	int left;
	int top;

	gboolean ExposeEvent (GtkWidget *w, GdkEventExpose *event);
	static gboolean expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
	static gboolean motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
	static gboolean crossing_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
	static gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
	static gboolean key_release (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
	static gboolean button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	static gboolean button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	static gboolean scroll (GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	static gboolean focus_in (GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
	static gboolean focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
	static gboolean realized (GtkWidget *widget, gpointer user_data);
	static gboolean unrealized (GtkWidget *widget, gpointer user_data);

	static void widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
	static void widget_destroyed (GtkWidget *widget, gpointer user_data);

	void InitializeFullScreen (MoonWindow *parent);
	void InitializeDesktop (MoonWindow *parent);
	void InitializePlugin ();
	void InitializeCommon ();

	void RightClickMenu ();
	void ShowMoonlightDialog ();

	static void show_moonlight_dialog (MoonWindowGtk *window);
	static void uninstall_application (MoonWindowGtk *window);
	static void install_application (MoonWindowGtk *window);
};

};
#endif /* __MOON_WINDOW_GTK_H__ */
