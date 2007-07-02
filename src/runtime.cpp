/*
 * runtime.cpp: Core surface and canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gtk/gtk.h>
#define Visual _XVisual
#include <gdk/gdkx.h>

#include <cairo-xlib.h>
#undef Visual
#include "runtime.h"
#include "canvas.h"
#include "control.h"
#include "shape.h"
#include "transform.h"
#include "animation.h"
#include "downloader.h"
#include "frameworkelement.h"
#include "stylus.h"
#include "rect.h"
#include "text.h"
#include "panel.h"
#include "value.h"
#include "namescope.h"
#include "xaml.h"

//#define DEBUG_INVALIDATE 1
#define DEBUG_REFCNT 0

#define CAIRO_CLIP 0
#define TIME_CLIP 0
#define TIME_REDRAW 1

void
draw_grid (cairo_t *cairo)
{
	cairo_set_line_width (cairo, 1.0);
	cairo_set_source_rgba (cairo, 0, 1, 0, 1.0);
	for (int col = 100; col < 1024; col += 100){
		cairo_move_to (cairo, col, 0);
		cairo_line_to (cairo, col, 1024);
		cairo_stroke (cairo);
	}

	for (int row = 0; row < 1024; row += 100){
		cairo_move_to (cairo, 0, row);
		cairo_line_to (cairo, 1024, row);
		cairo_stroke (cairo);
	}
}

void
create_similar (Surface *s, GtkWidget *widget)
{
    cairo_t *ctx = gdk_cairo_create (widget->window);

    if (s->cairo_xlib){
	cairo_destroy (s->cairo_xlib);
    }

    cairo_surface_t *xlib_surface = cairo_surface_create_similar (
	   cairo_get_target (ctx), 
	   CAIRO_CONTENT_COLOR_ALPHA,
	   s->width, s->height);

    cairo_destroy (ctx);

    s->cairo_xlib = cairo_create (xlib_surface);
    cairo_surface_destroy (xlib_surface);
}



static void
surface_realloc (Surface *s)
{
	if (s->buffer)
		free (s->buffer);

	int size = s->width * s->height * 4;
	s->buffer = (unsigned char *) malloc (size);

	s->cairo_buffer_surface = cairo_image_surface_create_for_data (
		s->buffer, CAIRO_FORMAT_ARGB32, s->width, s->height, s->width * 4);

	s->cairo_buffer = cairo_create (s->cairo_buffer_surface);

	if (s->cairo_xlib == NULL) {
		s->cairo = s->cairo_buffer;
	}
	else {
		create_similar (s, s->drawing_area);
		s->cairo = s->cairo_xlib;
	}
}

void 
surface_destroy (Surface *s)
{
	delete s;
}

static void
render_surface (gpointer data)
{
	Surface *s = (Surface*)data;
	gdk_window_process_updates (GTK_WIDGET (s->drawing_area)->window, FALSE);
}

gboolean
realized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	create_similar (s, widget);
	s->cairo = s->cairo_xlib;

	TimeManager::Instance()->AddHandler ("render", render_surface, s);
	return TRUE;
}

gboolean
unrealized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->cairo_xlib) {
		cairo_destroy (s->cairo_xlib);
		s->cairo_xlib = NULL;
	}

	s->cairo = s->cairo_buffer;
	TimeManager::Instance()->RemoveHandler ("render", render_surface, s);
	return TRUE;
}

gboolean
expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	Surface *s = (Surface *) data;

	s->frames++;

	if (event->area.x > s->width || event->area.y > s->height)
		return TRUE;

#if TIME_REDRAW
	STARTTIMER (expose, "redraw");
#endif
	s->cairo = s->cairo_xlib;

	//
	// BIG DEBUG BLOB
	// 
	if (cairo_status (s->cairo) != CAIRO_STATUS_SUCCESS){
		printf ("expose event: the cairo context has an error condition and refuses to paint: %s\n", 
			cairo_status_to_string (cairo_status (s->cairo)));
	}

#ifdef DEBUG_INVALIDATE
	printf ("Got a request to repaint at %d %d %d %d\n", event->area.x, event->area.y, event->area.width, event->area.height);
#endif

	cairo_t *ctx = gdk_cairo_create (widget->window);
	gdk_cairo_region (ctx, event->region);
	cairo_clip (ctx);

	//
	// These are temporary while we change this to paint at the offset position
	// instead of using the old approach of modifying the topmost Canvas (a no-no),
	//
	// The flag "s->transparent" is here because I could not
	// figure out what is painting the background with white now.
	// The change that made the white painting implicit instead of
	// explicit is patch 80632.   I would appreciate any help in tracking down
	// the proper way of making the background white when not running in 
	// "transparent" mode.    
	//
	// Either exposing surface_set_trans to turn the next code is a hack, 
	// or it is normal to request all code that wants to paint to manually
	// clear the background to white beforehand.    For now am going with
	// making this an explicit surface API.
	//
	// The second part is for coping with the future: when we support being 
	// windowless
	//
	if (s->transparent && !GTK_WIDGET_NO_WINDOW (widget)){
		cairo_set_operator (ctx, CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba (ctx, 1, 1, 1, 0);
		cairo_paint (ctx);
	}

	cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
	surface_paint (s, ctx, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_destroy (ctx);

#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

	return TRUE;
}

void 
surface_set_trans (Surface *s, bool trans)
{
	s->transparent = trans;
	if (s->drawing_area)
		gtk_widget_queue_draw (s->drawing_area);
}

bool
surface_get_trans (Surface *s)
{
	return s->transparent;
}

static gboolean
motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Surface *s = (Surface *) data;
	GdkModifierType state;
	double x, y;

	if (!s->cb_motion)
		return FALSE;

	if (event->is_hint) {
		int ix, iy;
		gdk_window_get_pointer (event->window, &ix, &iy, &state);
		x = ix;
		y = iy;
	} else {
		x = event->x;
		y = event->y;

		if (GTK_WIDGET_NO_WINDOW (widget)){
			x -= widget->allocation.x;
			y -= widget->allocation.y;
		}

		state = (GdkModifierType)event->state;
	}

	s->toplevel->HandleMotion (s, state, x, y);
	return TRUE;
}

static gboolean
crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->cb_enter == NULL || s->cb_mouse_leave == NULL)
		return FALSE;

	if (event->type == GDK_ENTER_NOTIFY){
		double x = event->x;
		double y = event->y;

		if (GTK_WIDGET_NO_WINDOW (widget)){
			x -= widget->allocation.x;
			y -= widget->allocation.y;
		}
		
		s->toplevel->HandleMotion (s, event->state, x, y);
		s->toplevel->Enter (s, event->state, x, y);
	} else {
		s->toplevel->Leave (s);
	}
	
	return TRUE;
}

static gboolean 
key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_keydown)
		return FALSE;

	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keydown (s->toplevel, key->state, key->keyval, key->hardware_keycode);
}

static gboolean 
key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_keyup)
		return FALSE;
	
	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keyup (s->toplevel, key->state, key->keyval, key->hardware_keycode);

}

static gboolean
button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_up)
		return FALSE;
	
	if (button->button != 1)
		return FALSE;

	double x = button->x;
	double y = button->y;
	if (GTK_WIDGET_NO_WINDOW (widget)){
		x -= widget->allocation.x;
		y -= widget->allocation.y;
	}
	s->toplevel->HandleButton (s, s->cb_up, button->state, x, y);
	
	return TRUE;
}

static gboolean
button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	gtk_widget_grab_focus (widget);

	if (!s->cb_down)
		return FALSE;

	if (button->button != 1)
		return FALSE;

	double x = button->x;
	double y = button->y;
	if (GTK_WIDGET_NO_WINDOW (widget)){
		x -= widget->allocation.x;
		y -= widget->allocation.y;
	}
	s->toplevel->HandleButton (s, s->cb_down, button->state, x, y);
	
	return FALSE;
}


void 
clear_drawing_area (GtkObject *obj, gpointer data)
{
	Surface *s = (Surface *) data;

	s->drawing_area = NULL;
}

void
surface_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	Surface *s = (Surface *) user_data;

        if (s->width != allocation->width || s->height != allocation->height){
                s->width = allocation->width;
                s->height = allocation->height;

		surface_realloc (s);
	}
	
	// if x or y changed we need to recompute the presentation matrix
	// because the toplevel position depends on the allocation.
	if (s->toplevel)
		s->toplevel->UpdateBounds ();
}

static void
surface_drawing_area_destroyed (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	// This is never called, why?
	printf ("------------------ WE ARE DESTROYED ---------------\n");
	s->drawing_area = NULL;
}

Surface *
surface_new (int width, int height)
{
	Surface *s = new Surface ();

	s->drawing_area = gtk_event_box_new ();

	// don't let gtk clear the window we'll do all the drawing.
	gtk_widget_set_app_paintable (s->drawing_area, TRUE);

	//
	// Set to true, need to change that to FALSE later when we start
	// repainting again.   
	//
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (s->drawing_area), TRUE);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "size_allocate",
			    G_CALLBACK(surface_size_allocate), s);
	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "destroy",
			    G_CALLBACK(surface_drawing_area_destroyed), s);

	gtk_widget_add_events (s->drawing_area, 
			       GDK_POINTER_MOTION_MASK |
			       GDK_POINTER_MOTION_HINT_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK);
	GTK_WIDGET_SET_FLAGS (s->drawing_area, GTK_CAN_FOCUS);
	//gtk_widget_set_double_buffered (s->drawing_area, FALSE);

	gtk_widget_show (s->drawing_area);

	gtk_widget_set_size_request (s->drawing_area, width, height);
	s->buffer = NULL;

	s->width = width;
	s->height = height;
	s->toplevel = NULL;

	surface_realloc (s);

	return s;
}

//
// This will resize the surface (merely a convenience function for
// resizing the widget area that we have.
//
// This will not change the Width and Height properties of the 
// toplevel canvas, if you want that, you must do that yourself
//
void
surface_resize (Surface *s, int width, int height)
{
	gtk_widget_set_size_request (s->drawing_area, width, height);
}

Surface::~Surface ()
{
	//
	// This removes one source of problems: the unrealize handler is not
	// being called when Mozilla destroys our window, so we remove it here.
	//
	// This is easy to trigger, open two clocks and try to load a different
	// page on one of the pages (that keeps the timer ticking, and eventually
	// kills this
	//
	// There is still another problem: sometimes we are getting:
	//     The error was 'RenderBadPicture (invalid Picture parameter)'.
	//
	// And I have yet to track what causes this, the stack trace is not 
	// very useful
	//
	TimeManager::Instance()->RemoveHandler ("render", render_surface, this);

	if (toplevel) {
		toplevel->unref ();
		toplevel = NULL;
	}

	if (buffer) {
		free (buffer);
		buffer = NULL;
	}

	cairo_destroy (cairo_buffer);
	cairo_buffer = NULL;

	cairo_surface_destroy (cairo_buffer_surface);
	cairo_buffer_surface = NULL;

	if (drawing_area != NULL){
		g_signal_handlers_disconnect_matched (drawing_area,
						      (GSignalMatchType) G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL, this);

		gtk_widget_destroy (drawing_area);
		drawing_area = NULL;
	}
}

void
surface_connect_events (Surface *s)
{
	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "expose_event",
			    G_CALLBACK (expose_event_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "motion_notify_event",
			    G_CALLBACK (motion_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "enter_notify_event",
			    G_CALLBACK (crossing_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "leave_notify_event",
			    G_CALLBACK (crossing_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "key_press_event",
			    G_CALLBACK (key_press_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "key_release_event",
			    G_CALLBACK (key_release_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "button_press_event",
			    G_CALLBACK (button_press_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "button_release_event",
			    G_CALLBACK (button_release_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "realize",
			    G_CALLBACK (realized_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "unrealize",
			    G_CALLBACK (unrealized_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "destroy",
			    G_CALLBACK (clear_drawing_area), s);

	if (GTK_WIDGET_REALIZED (s->drawing_area)){
		realized_callback (s->drawing_area, s);
	}
}

void
surface_attach (Surface *surface, UIElement *toplevel)
{
	bool first = FALSE;

	if (!Type::Find (toplevel->GetObjectType())->IsSubclassOf (Type::CANVAS)) {
		printf ("Unsupported toplevel\n");
		return;
	}
	
	if (surface->toplevel) {
		surface->toplevel->Invalidate ();
		surface->toplevel->unref ();
	} else 
		first = TRUE;

	Canvas *canvas = (Canvas *) toplevel;
	canvas->ref ();

	canvas->surface = surface;
	surface->toplevel = canvas;

	// First time we connect the surface, start responding to events
	if (first)
		surface_connect_events (surface);

	canvas->OnLoaded ();

	bool change_size = false;
	//
	// If the did not get a size specified
	//
	if (surface->width == 0){
		Value *v = toplevel->GetValue (FrameworkElement::WidthProperty);

		if (v){
			surface->width = (int) v->AsDouble ();
			if (surface->width < 0)
				surface->width = 0;
			change_size = true;
		}
	}

	if (surface->height == 0){
		Value *v = toplevel->GetValue (FrameworkElement::HeightProperty);

		if (v){
			surface->height = (int) v->AsDouble ();
			if (surface->height < 0)
				surface->height = 0;
			change_size = true;
		}
	}

	if (change_size)
		surface_realloc (surface);

	canvas->UpdateBounds ();
	canvas->Invalidate ();
}

void
surface_paint (Surface *s, cairo_t *ctx, int x, int y, int width, int height)
{
        cairo_t *temp = s->cairo;
	s->cairo = ctx;
	s->toplevel->DoRender (s->cairo, x, y, width, height);
	s->cairo = temp;
}

void *
surface_get_drawing_area (Surface *s)
{
	return s->drawing_area;
}

void surface_register_events (Surface *s,
			      callback_mouse_event motion, callback_mouse_event down, callback_mouse_event up,
			      callback_mouse_event enter,
			      callback_plain_event got_focus, callback_plain_event lost_focus,
			      callback_plain_event loaded, callback_plain_event mouse_leave, callback_plain_event surface_resize,
			      callback_keyboard_event keydown, callback_keyboard_event keyup)
{
	s->cb_motion = motion;
	s->cb_down = down;
	s->cb_up = up;
	s->cb_enter = enter;
	s->cb_got_focus = got_focus;
	s->cb_lost_focus = lost_focus;
	s->cb_loaded = loaded;
	s->cb_mouse_leave = mouse_leave;
	s->cb_keydown = keydown;
	s->cb_keyup = keyup;
	s->cb_surface_resize = surface_resize;
}


cairo_t*
measuring_context_create (void)
{
	cairo_surface_t* surf = cairo_image_surface_create (CAIRO_FORMAT_A1, 1, 1);
	return cairo_create (surf);
}

void
measuring_context_destroy (cairo_t *cr)
{
	cairo_surface_destroy (cairo_get_target (cr));
	cairo_destroy (cr);
}

static bool inited = false;

void
runtime_init (void)
{
	if (inited)
		return;
	
	if (cairo_version () < CAIRO_VERSION_ENCODE(1,4,0)) {
		printf ("*** WARNING ***\n");
		printf ("*** Cairo versions < 1.4.0 should not be used for Moon.\n");
		printf ("*** Moon was configured to use Cairo version %d.%d.%d, but\n", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
		printf ("*** is being run against version %s.\n", cairo_version_string ());
		printf ("*** Proceed at your own risk\n");
	}

	inited = true;

	g_type_init ();

	TimeManager::Instance()->Start();

	types_init ();
	namescope_init ();
	uielement_init ();
	framework_element_init ();
	canvas_init ();
	dependencyobject_init();
	event_trigger_init ();
	transform_init ();
	animation_init ();
	brush_init ();
	shape_init ();
	geometry_init ();
	xaml_init ();
	clock_init ();
	text_init ();
	downloader_init ();
	media_init ();
	panel_init ();
	stylus_init ();
}

void
runtime_shutdown ()
{
	if (!inited)
		return;

	TimeManager::Instance()->Shutdown ();
	Type::Shutdown ();
	DependencyObject::Shutdown ();

	inited = false;
}
