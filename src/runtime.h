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
#include "value.h"
#include "type.h"
#include "list.h"
#include "downloader.h"

#define TIMERS 0
#if TIMERS
#define STARTTIMER(id,str) TimeSpan id##_t_start = get_now(); printf ("timing of '%s' started at %lld\n", str, id##_t_start)
#define ENDTIMER(id,str) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended at %lld (%f seconds)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 1000000)
#else
#define STARTTIMER(id,str)
#define ENDTIMER(id,str)
#endif

enum RuntimeInitFlags {
	RUNTIME_INIT_PANGO_TEXT_LAYOUT = 0x1,
	RUNTIME_INIT_MICROSOFT_CODECS  = 0x2,
	RUNTIME_INIT_TIMESOURCE_MANUAL = 0x4,
};

#define RUNTIME_INIT_DESKTOP (RUNTIME_INIT_PANGO_TEXT_LAYOUT)
#define RUNTIME_INIT_BROWSER (RUNTIME_INIT_MICROSOFT_CODECS)


class Surface : public EventObject {
 public:
	Surface (int width, int height);

	virtual ~Surface ();

	// allows you to redirect painting of the surface to an
	// arbitrary cairo context.
	void Paint (cairo_t *ctx, int x, int y, int width, int height);

	void Attach (UIElement* toplevel);

	void SetCursor (MouseCursor cursor);

	bool SetMouseCapture (UIElement *capture);
	UIElement* GetMouseCapture () { return capture_element; }

	void Resize (int width, int height);
	int GetWidth () { return normal_width; }
	int GetHeight () { return normal_height; }

	void SetTrans (bool trans);
	bool GetTrans () { return transparent; }

	void SetBackgroundColor (Color *color);

	int GetFrameCount () { return frames; }
	void ResetFrameCount () { frames = 0; }

	void Invalidate (Rect r);

	GtkWidget* GetDrawingArea () { return drawing_area; }
	UIElement* GetToplevel() { return toplevel; }
	bool IsTopLevel (UIElement* top);

	UIElement* GetCapturedElement () { return capture_element; }

	static int ResizeEvent;
	static int FullScreenChangeEvent;
	
	bool GetFullScreen () { return full_screen; }
	void SetFullScreen (bool value);
	void SetCanFullScreen (bool value) { can_full_screen = value; }
	void SetSourceLocation (const char* location);
	bool FullScreenKeyHandled (GdkEventKey *key);
	int GetActualWidth () { return width; }
	int GetActualHeight () { return height; }

	ClockGroup* GetClockGroup () { return clock_group; }

	virtual Type::Kind GetObjectType () { return Type::SURFACE; };
	
	void SetDownloaderContext (gpointer context) { downloader_context = context; }
	gpointer GetDownloaderContext () { return downloader_context; }
	Downloader* CreateDownloader () 
	{
		Downloader *downloader = new Downloader ();
		downloader->SetContext (downloader_context);
		return downloader;
	}
	
	static Downloader* CreateDownloader (UIElement* element)
	{
		Surface* surface = NULL;
		if (element) {
			surface = element->GetSurface ();
		}
		if (surface) {
			return surface->CreateDownloader ();
		} else {
			printf ("Surface::CreateDownloader (%p, ID: %i): Unable to create contextual downloader.\n", element, GET_OBJ_ID (element));
			//print_stack_trace ();
			return new Downloader ();
		}
	}
	
private:
	gpointer downloader_context;
	
	int normal_width, normal_height;
	int screen_width, screen_height;
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

	// The widget where we draw.
	GtkWidget *drawing_area;

	// This currently can only be a canvas.
	UIElement *toplevel;

	// The element currently capturing the mouse
	UIElement *capture_element;

	// the currently shown cursor
	MouseCursor cursor;

	// Fullscreen support
	bool full_screen;
	Canvas* full_screen_message;
	char* source_location;
	// Should be set to true only while executing MouseLeftButtonDown, 
	// MouseLeftButtonUp, KeyDown, and KeyUp event handlers
	bool can_full_screen; 

	void UpdateFullScreen (bool value);

	// Here we keep a reference to the normal drawing area when
	// we are in fullscreen mode.
	GtkWidget *drawing_area_normal;

	// We set drawing_area to this whenever we are in
	// fullscreen mode.
	GtkWidget *drawing_area_fullscreen;

	// The clock group (toplevel clock) for this surface.
	// Registered with the TimeManager.  All storyboards created
	// within this surface are children of this ClockGroup.
	ClockGroup *clock_group;
	TimelineGroup *timeline;

	int frames;

	int last_event_state;
	double last_event_x, last_event_y;

	void ConnectEvents (bool realization_signals);
	void Realloc ();
	void InitializeDrawingArea (GtkWidget* drawing_area);
	void DestroyDrawingArea (GtkWidget* drawing_area);
	void ShowFullScreenMessage ();
	void HideFullScreenMessage ();

	void CreateSimilarSurface ();

	static Key gdk_keyval_to_key (guint keyval);

	static void render_cb (EventObject *sender, gpointer calldata, gpointer closure);
	static void update_input_cb (EventObject *sender, gpointer calldata, gpointer closure);
	static void drawing_area_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
	static void drawing_area_destroyed (GtkWidget *w, gpointer data);
	static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);
	static gboolean motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data);
	static gboolean crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data);
	static gboolean key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data);
	static gboolean key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data);
	static gboolean button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data);
	static gboolean button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data);
	static gboolean realized_callback (GtkWidget *widget, gpointer data);
	static gboolean unrealized_callback (GtkWidget *widget, gpointer data);
};

Surface *surface_new       (int width, int height);
void     surface_resize    (Surface *s, int width, int height);
void     surface_attach    (Surface *s, UIElement *element);
void     surface_init      (Surface *s, int width, int height);
void     surface_destroy   (Surface *s);
void     surface_set_trans (Surface *s, bool trans);
bool     surface_get_trans (Surface *s);
void     surface_paint     (Surface *s, cairo_t *ctx, int x, int y, int width, int height);

void    *surface_get_drawing_area (Surface *s);

cairo_t *measuring_context_create (void);
void     measuring_context_destroy (cairo_t *cr);

void runtime_init (guint32 flags);

uint32_t runtime_html_timer_timeout_add (int32_t interval, GSourceFunc callback, gpointer data);
void     runtime_html_timer_timeout_stop (uint32_t source_id);

void runtime_shutdown (void);

G_END_DECLS

#endif
