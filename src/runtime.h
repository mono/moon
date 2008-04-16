/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * runtime.h: Core surface and canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __RUNTIME_H__
#define __RUNTIME_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include <gtk/gtk.h>

#include "point.h"
#include "uielement.h"
#include "dependencyobject.h"
#include "dirty.h"
#include "value.h"
#include "type.h"
#include "list.h"
#include "downloader.h"
#include "error.h"

#define MAXIMUM_CACHE_SIZE 6000000

#define FRONT_TO_BACK_STATS 0

#define TIMERS 0
#define DEBUG_MARKER_KEY 0
#if TIMERS
#define STARTTIMER(id,str) TimeSpan id##_t_start = get_now(); printf ("timing of '%s' started at %lld\n", str, id##_t_start)
#define ENDTIMER(id,str) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended at %lld (%f seconds)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 10000000)
#else
#define STARTTIMER(id,str)
#define ENDTIMER(id,str)
#endif

enum RuntimeInitFlags {
	RUNTIME_INIT_PANGO_TEXT_LAYOUT     = 1 << 0,
	RUNTIME_INIT_MICROSOFT_CODECS      = 1 << 1,
	RUNTIME_INIT_MANUAL_TIMESOURCE     = 1 << 2,
	RUNTIME_INIT_DISABLE_AUDIO         = 1 << 3,
	RUNTIME_INIT_SHOW_EXPOSE           = 1 << 4,
	RUNTIME_INIT_SHOW_CLIPPING         = 1 << 5,
	RUNTIME_INIT_SHOW_BOUNDING_BOXES   = 1 << 6,
	RUNTIME_INIT_SHOW_FPS              = 1 << 7,
	RUNTIME_INIT_RENDER_FRONT_TO_BACK  = 1 << 8,
	RUNTIME_INIT_SHOW_CACHE_SIZE	   = 1 << 9,
	RUNTIME_INIT_DISABLE_CONVERTER_YUV = 1 << 10,
	RUNTIME_INIT_USE_SHAPE_CACHE	   = 1 << 11,
};

#define RUNTIME_INIT_DESKTOP (RUNTIME_INIT_PANGO_TEXT_LAYOUT | RUNTIME_INIT_RENDER_FRONT_TO_BACK)
#define RUNTIME_INIT_BROWSER (RUNTIME_INIT_MICROSOFT_CODECS | RUNTIME_INIT_RENDER_FRONT_TO_BACK)

extern guint32 moonlight_flags;

class TimeManager;
class Surface;

typedef void (* MoonlightInvalidateFunc) (Surface *surface, Rect r, void *user_data);
typedef void (* MoonlightRenderFunc) (Surface *surface, void *user_data);
typedef void (* MoonlightFPSReportFunc) (Surface *surface, int nframes, float nsecs, void *user_data);
typedef void (* MoonlightCacheReportFunc) (Surface *surface, long size, void *user_data);
typedef bool (* MoonlightEventEmitFunc) (UIElement *element, GdkEvent *event);

class Surface : public EventObject {
 protected:
	virtual ~Surface();

 public:
	// if we're windowed, @d will be NULL.
	Surface (int width, int height, bool windowless = false);

	// allows you to redirect painting of the surface to an
	// arbitrary cairo context.
	void Paint (cairo_t *ctx, int x, int y, int width, int height);
	void Paint (cairo_t *ctx, Region *);

	void Attach (UIElement *toplevel);

	void SetCursor (MouseCursor cursor);

	bool SetMouseCapture (UIElement *capture);

	void Resize (int width, int height);
	int GetWidth () { return width; }
	int GetHeight () { return height; }

	void EmitError (ErrorEventArgs *args);
	void SetTrans (bool trans);
	bool GetTrans () { return transparent; }

	void SetBackgroundColor (Color *color);

	int GetFrameCount () { return frames; }
	void ResetFrameCount () { frames = 0; }

	void Invalidate (Rect r);

	GtkWidget *GetWidget () { return widget; }
	UIElement *GetToplevel() { return toplevel; }
	bool IsTopLevel (UIElement *top);

	const static int ResizeEvent;
	const static int FullScreenChangeEvent;
	const static int ErrorEvent;

	bool GetFullScreen () { return full_screen; }
	void SetFullScreen (bool value);
	void SetCanFullScreen (bool value) { can_full_screen = value; }
	void SetSourceLocation (const char *location);
	bool FullScreenKeyHandled (GdkEventKey *key);
	int GetActualWidth () { return width; }
	int GetActualHeight () { return height; }

	TimeManager *GetTimeManager () { return time_manager; }

	virtual Type::Kind GetObjectType () { return Type::SURFACE; };
	
	void SetDownloaderContext (gpointer context) { downloader_context = context; }
	gpointer GetDownloaderContext () { return downloader_context; }
	
	Downloader *CreateDownloader ();
	static Downloader *CreateDownloader (UIElement *element);

	void SetRenderFunc (MoonlightRenderFunc render, void *user_data);
	void SetInvalidateFunc (MoonlightInvalidateFunc invalidate, void *user_data);
	void SetFPSReportFunc (MoonlightFPSReportFunc report, void *user_data);
	void SetCacheReportFunc (MoonlightCacheReportFunc report, void *user_data);

	bool VerifyWithCacheSizeCounter (int w, int h);
	int64_t AddToCacheSizeCounter (int w, int h);
	void RemoveFromCacheSizeCounter (int64_t size);

#if FRONT_TO_BACK_STATS
	int uielements_rendered_front_to_back;
	int uielements_rendered_back_to_front;
#endif

#ifdef DEBUG
	UIElement *debug_selected_element;
#endif

	gboolean expose_to_drawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y);

	// widget callbacks
	static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);
	static gboolean motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data);
	static gboolean crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data);
	static gboolean key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data);
	static gboolean key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data);
	static gboolean button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data);
	static gboolean button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data);
	static gboolean realized_callback (GtkWidget *widget, gpointer data);
	static gboolean unrealized_callback (GtkWidget *widget, gpointer data);


	// bad, but these live in dirty.cpp, not runtime.cpp
	void AddDirtyElement (UIElement *element, DirtyType dirt);
	void RemoveDirtyElement (UIElement *element);
	void ProcessDirtyElements ();
	void PropagateDirtyFlagToChildren (UIElement *element, DirtyType dirt);
	bool IsAnythingDirty ();

	static pthread_t main_thread;
	static bool InMainThread () { return pthread_equal (main_thread, pthread_self ()); }
private:
	// bad, but these two live in dirty.cpp, not runtime.cpp
	void ProcessDownDirtyElements ();
	void ProcessUpDirtyElements ();

	List *down_dirty;
	List *up_dirty;

	gpointer downloader_context;
	
	int normal_width, normal_height;
	// the actual size of the drawing area, 
	// screen size in fullscreen mode,
	// otherwise normal size.
	int width, height;

	// The data lives here
	unsigned char *buffer;

	// The above buffer, as a pixbuf, for the software mode
	GdkPixbuf *pixbuf;
	
	bool using_cairo_xlib_surface;
	
	cairo_surface_t *cairo_buffer_surface;
	cairo_t         *cairo_buffer;
	cairo_t         *cairo_xlib;
	
	//
	// This is what code uses, and its equal to either:
	//    cairo_buffer: when the widget has not been realized
	//    cairo_xlib:   when the widget has been realized
	//
	cairo_t *cairo;		

	// The pixmap used for the backing storage for xlib_surface
	GdkPixmap *pixmap;

	bool transparent;

	Color *background_color;

	// The widget where we draw.  NULL if we're windowless
	GtkWidget *widget;

	// Here we keep a reference to the normal drawing area when
	// we are in fullscreen mode.
	GtkWidget *widget_normal;

	// We set widget to this whenever we are in fullscreen mode.
	GtkWidget *widget_fullscreen;

	// This currently can only be a canvas.
	UIElement *toplevel;

	// the list of elements (from most deeply nested to the
	// toplevel) we've most recently sent a mouse event to.
	List *input_list;

	// is the mouse captured?  if it is, it'll be by the first element in input_list.
	bool captured;
	UIElement *pendingCapture;
	bool pendingReleaseCapture;

	// are we currently emitting a mouse event?
	bool emittingMouseEvent;

	// the currently shown cursor
	MouseCursor cursor;

	// Fullscreen support
	bool full_screen;
	Canvas *full_screen_message;
	char *source_location;
	// Should be set to true only while executing MouseLeftButtonDown, 
	// MouseLeftButtonUp, KeyDown, and KeyUp event handlers
	bool can_full_screen; 

	void UpdateFullScreen (bool value);

	TimeManager *time_manager;

	int frames;

	GdkEvent *mouse_event;

	MoonlightInvalidateFunc invalidate;
	void *invalidate_data;

	MoonlightRenderFunc render;
	void *render_data;

	// Variables for reporting FPS
	MoonlightFPSReportFunc fps_report;
	int64_t fps_start;
	int fps_nframes;
	void *fps_data;

	// Variables for reporting cache size
	MoonlightCacheReportFunc cache_report;
	int64_t cache_size_in_bytes;
	int cache_size_ticker;
	void *cache_data;
	int cache_size_multiplier;
	
	void ConnectEvents (bool realization_signals);
	void Realloc ();
	void InitializeWidget (GtkWidget *widget);
	void DestroyWidget (GtkWidget *widget);
	void ShowFullScreenMessage ();
	void HideFullScreenMessage ();

	void CreateSimilarSurface ();

	static Key gdk_keyval_to_key (guint keyval);

	static void render_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void update_input_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
	static void widget_destroyed (GtkWidget *w, gpointer data);

	void FindFirstCommonElement (List *l1, int *index1,
				     List *l2, int *index2);
	bool EmitEventOnList (MoonlightEventEmitFunc emitter, List *list, GdkEvent *event, int end_idx);
	void UpdateCursorFromInputList ();
	bool HandleMouseEvent (MoonlightEventEmitFunc emitter, bool emit_leave, bool emit_enter, bool force_emit,
			       GdkEvent *event);
	void PerformCapture (UIElement *capture);
	void PerformReleaseCapture ();
};

/* for hit testing */
class UIElementNode : public List::Node {
public:
	UIElement *uielement;
		
	UIElementNode (UIElement *el);
	virtual ~UIElementNode ();
};

/* for rendering */
typedef void (*RenderFunc) (cairo_t *ctx, UIElement *uielement, Region *region, bool front_to_back);
class RenderNode : public List::Node {
public:
	UIElement *uielement;
	Region *region;
	bool render_element;
	RenderFunc pre_render;
	RenderFunc post_render;

	RenderNode (UIElement *el, Region *region, bool render_element, RenderFunc pre, RenderFunc post);

	virtual ~RenderNode ();
};


Surface *surface_new       (int width, int height);
void     surface_resize    (Surface *s, int width, int height);
void     surface_attach    (Surface *s, UIElement *element);
void     surface_init      (Surface *s, int width, int height);
void     surface_destroy   (Surface *s);
void     surface_set_trans (Surface *s, bool trans);
bool     surface_get_trans (Surface *s);
void     surface_paint     (Surface *s, cairo_t *ctx, int x, int y, int width, int height);

void    *surface_get_widget (Surface *s);
TimeManager* surface_get_time_manager (Surface* s);

cairo_t *measuring_context_create (void);
void     measuring_context_destroy (cairo_t *cr);

void runtime_init (guint32 flags);

uint32_t runtime_html_timer_timeout_add (int32_t interval, GSourceFunc callback, gpointer data);
void     runtime_html_timer_timeout_stop (uint32_t source_id);

void runtime_shutdown (void);

gboolean strcase_equal (gconstpointer  v1, gconstpointer  v2);
guint strcase_hash     (gconstpointer v);

G_END_DECLS

#endif
