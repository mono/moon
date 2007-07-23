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
#include <glib.h>
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
Surface::CreateSimilarSurface ()
{
    cairo_t *ctx = gdk_cairo_create (drawing_area->window);

    if (cairo_xlib){
	cairo_destroy (cairo_xlib);
    }

    cairo_surface_t *xlib_surface = cairo_surface_create_similar (
	   cairo_get_target (ctx), 
	   CAIRO_CONTENT_COLOR_ALPHA,
	   width, height);

    cairo_destroy (ctx);

    cairo_xlib = cairo_create (xlib_surface);
    cairo_surface_destroy (xlib_surface);
}


Surface::Surface(int w, int h)
  : cb_surface_resize(NULL),

    width (w), height (h), buffer (0), pixbuf (NULL),
    using_cairo_xlib_surface(0),
    cairo_buffer_surface (NULL), cairo_buffer(NULL),
    cairo_xlib(NULL), cairo (NULL), transparent(false),
    cursor (MouseCursorDefault)
{

	drawing_area = gtk_event_box_new ();

	// don't let gtk clear the window we'll do all the drawing.
	gtk_widget_set_app_paintable (drawing_area, TRUE);

	//
	// Set to true, need to change that to FALSE later when we start
	// repainting again.   
	//
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (drawing_area), TRUE);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "size_allocate",
			    G_CALLBACK(drawing_area_size_allocate), this);
	gtk_signal_connect (GTK_OBJECT (drawing_area), "destroy",
			    G_CALLBACK(drawing_area_destroyed), this);

	gtk_widget_add_events (drawing_area, 
			       GDK_POINTER_MOTION_MASK |
			       GDK_POINTER_MOTION_HINT_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK);
	GTK_WIDGET_SET_FLAGS (drawing_area, GTK_CAN_FOCUS);
	//gtk_widget_set_double_buffered (drawing_area, FALSE);

	gtk_widget_show (drawing_area);

	gtk_widget_set_size_request (drawing_area, width, height);
	buffer = NULL;

	toplevel = NULL;
	capture_element = NULL;

	Realloc ();
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
	TimeManager::Instance()->RemoveHandler (TimeManager::Instance()->RenderEvent, render_cb, this);
	TimeManager::Instance()->RemoveHandler (TimeManager::Instance()->UpdateInputEvent, update_input_cb, this);

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

bool
Surface::SetMouseCapture (UIElement *capture)
{
	if (capture != NULL && capture_element != NULL)
		return false;

	capture_element = capture;

	if (capture_element == NULL) {
		// generate the proper Enter/Leave events here for the object
		// under the mouse pointer
	}

	return true;
}

void
Surface::SetCursor (MouseCursor new_cursor)
{
	if (new_cursor != cursor) {
		cursor = new_cursor;

		if (drawing_area == NULL)
			return;

		GdkCursor *c = NULL;
		switch (cursor) {
		case MouseCursorDefault:
			c = NULL;
			break;
		case MouseCursorArrow:
			c = gdk_cursor_new (GDK_LEFT_PTR);
			break;
		case MouseCursorHand:
			c = gdk_cursor_new (GDK_HAND1);
			break;
		case MouseCursorWait:
			c = gdk_cursor_new (GDK_WATCH);
			break;
		case MouseCursorIBeam:
			c = gdk_cursor_new (GDK_XTERM);
			break;
		case MouseCursorStylus:
			c = gdk_cursor_new (GDK_CROSSHAIR); // ??
			break;
		case MouseCursorEraser:
			c = gdk_cursor_new (GDK_PENCIL); // ??
			break;
		case MouseCursorNone:
			// XXX nothing yet.  create a pixmap cursor with no pixel data
			break;
		}

		gdk_window_set_cursor (drawing_area->window, c);
	}
}

void
Surface::ConnectEvents ()
{
	gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
			    G_CALLBACK (Surface::expose_event_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "motion_notify_event",
			    G_CALLBACK (motion_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "enter_notify_event",
			    G_CALLBACK (crossing_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "leave_notify_event",
			    G_CALLBACK (crossing_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "key_press_event",
			    G_CALLBACK (key_press_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "key_release_event",
			    G_CALLBACK (key_release_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
			    G_CALLBACK (button_press_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "button_release_event",
			    G_CALLBACK (button_release_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "realize",
			    G_CALLBACK (realized_callback), this);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "unrealize",
			    G_CALLBACK (unrealized_callback), this);

	if (GTK_WIDGET_REALIZED (drawing_area)){
		realized_callback (drawing_area, this);
	}
}

void
Surface::Attach (UIElement *element)
{
	bool first = FALSE;

	if (!Type::Find (element->GetObjectType())->IsSubclassOf (Type::CANVAS)) {
		printf ("Unsupported toplevel\n");
		return;
	}
	
	if (toplevel) {
		toplevel->Invalidate ();
		toplevel->unref ();
	} else 
		first = TRUE;

	Canvas *canvas = (Canvas *) element;
	canvas->ref ();

	canvas->surface = this;
	toplevel = canvas;

	// First time we connect the surface, start responding to events
	if (first)
		ConnectEvents ();

	canvas->OnLoaded ();

	bool change_size = false;
	//
	// If the did not get a size specified
	//
	if (width == 0){
		Value *v = toplevel->GetValue (FrameworkElement::WidthProperty);

		if (v){
			width = (int) v->AsDouble ();
			if (width < 0)
				width = 0;
			change_size = true;
		}
	}

	if (height == 0){
		Value *v = toplevel->GetValue (FrameworkElement::HeightProperty);

		if (v){
			height = (int) v->AsDouble ();
			if (height < 0)
				height = 0;
			change_size = true;
		}
	}

	if (change_size)
		Realloc ();

	canvas->UpdateBounds ();
	canvas->Invalidate ();
}

void
Surface::Paint (cairo_t *ctx, int x, int y, int width, int height)
{
	toplevel->DoRender (ctx, x, y, width, height);
}

//
// This will resize the surface (merely a convenience function for
// resizing the widget area that we have.
//
// This will not change the Width and Height properties of the 
// toplevel canvas, if you want that, you must do that yourself
//
void
Surface::Resize (int width, int height)
{
	gtk_widget_set_size_request (drawing_area, width, height);
}

void
Surface::Realloc ()
{
	if (buffer)
		free (buffer);

	int size = width * height * 4;
	buffer = (unsigned char *) malloc (size);

	cairo_buffer_surface = cairo_image_surface_create_for_data (
		buffer, CAIRO_FORMAT_ARGB32, width, height, width * 4);

	cairo_buffer = cairo_create (cairo_buffer_surface);

	if (cairo_xlib == NULL) {
		cairo = cairo_buffer;
	}
	else {
		CreateSimilarSurface ();
		cairo = cairo_xlib;
	}
}

void
Surface::RegisterEvents (callback_plain_event surface_resize)
{
	this->cb_surface_resize = surface_resize;
}


void
Surface::render_cb (EventObject *sender, gpointer calldata, gpointer closure)
{
	Surface *s = (Surface*)closure;
	gdk_window_process_updates (GTK_WIDGET (s->drawing_area)->window, FALSE);
}

void
Surface::update_input_cb (EventObject *sender, gpointer calldata, gpointer closure)
{
	Surface *s = (Surface*)closure;

	MouseCursor new_cursor = MouseCursorDefault;

	UIElement *input_element = s->capture_element ? s->capture_element : s->toplevel;
	input_element->HandleMotion (s->cairo, s->last_event_state, s->last_event_x, s->last_event_y, &new_cursor);
	s->SetCursor (new_cursor);
}

gboolean
Surface::realized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	s->CreateSimilarSurface ();
	s->cairo = s->cairo_xlib;

	TimeManager::Instance()->AddHandler (TimeManager::Instance()->RenderEvent, render_cb, s);
	TimeManager::Instance()->AddHandler (TimeManager::Instance()->UpdateInputEvent, update_input_cb, s);
	return TRUE;
}

gboolean
Surface::unrealized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->cairo_xlib) {
		cairo_destroy (s->cairo_xlib);
		s->cairo_xlib = NULL;
	}

	s->cairo = s->cairo_buffer;
	TimeManager::Instance()->RemoveHandler (TimeManager::Instance()->RenderEvent, render_cb, s);
	TimeManager::Instance()->RemoveHandler (TimeManager::Instance()->UpdateInputEvent, update_input_cb, s);
	return TRUE;
}

gboolean
Surface::expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
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
	s->Paint (ctx, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_destroy (ctx);

#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

	return TRUE;
}

gboolean
Surface::motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Surface *s = (Surface *) data;
	GdkModifierType state;
	double x, y;

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

	s->last_event_x = x;
	s->last_event_y = y;
	s->last_event_state = state;

	MouseCursor new_cursor = MouseCursorDefault;
	UIElement *input_element = s->capture_element ? s->capture_element : s->toplevel;
	input_element->HandleMotion (s->cairo, state, x, y, &new_cursor);
	s->SetCursor (new_cursor);

	return TRUE;
}

gboolean
Surface::crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	Surface *s = (Surface *) data;

	if (event->type == GDK_ENTER_NOTIFY){
		double x = event->x;
		double y = event->y;

		if (GTK_WIDGET_NO_WINDOW (widget)){
			x -= widget->allocation.x;
			y -= widget->allocation.y;
		}
		
		MouseCursor new_cursor = MouseCursorDefault;

		if (s->capture_element) {
			s->capture_element->HandleMotion (s->cairo, event->state, x, y, &new_cursor);
		}
		else {
		  	s->toplevel->HandleMotion (s->cairo, event->state, x, y, &new_cursor);
			s->toplevel->Enter (s->cairo, event->state, x, y);
		}

		s->SetCursor (new_cursor);

		s->last_event_x = x;
		s->last_event_y = y;
		s->last_event_state = event->state;
	
	} else {
		if (s->capture_element == NULL)
			s->toplevel->Leave ();
	}

	return TRUE;
}

gboolean 
Surface::key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
#if notyet
	Surface *s = (Surface *) data;

	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keydown (s->toplevel, key->state, key->keyval, key->hardware_keycode);
#endif
	return FALSE;
}

gboolean 
Surface::key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
#if notyet
	Surface *s = (Surface *) data;

	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keyup (s->toplevel, key->state, key->keyval, key->hardware_keycode);
#endif
	return FALSE;
}

gboolean
Surface::button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	if (button->button != 1)
		return FALSE;

	double x = button->x;
	double y = button->y;
	if (GTK_WIDGET_NO_WINDOW (widget)){
		x -= widget->allocation.x;
		y -= widget->allocation.y;
	}
	UIElement *input_element = s->capture_element ? s->capture_element : s->toplevel;
	input_element->HandleButtonRelease (s->cairo, button->state, x, y);
	
	return TRUE;
}

gboolean
Surface::button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	gtk_widget_grab_focus (widget);

	if (button->button != 1)
		return FALSE;

	double x = button->x;
	double y = button->y;
	if (GTK_WIDGET_NO_WINDOW (widget)){
		x -= widget->allocation.x;
		y -= widget->allocation.y;
	}
	UIElement *input_element = s->capture_element ? s->capture_element : s->toplevel;
	input_element->HandleButtonPress (s->cairo, button->state, x, y);
	
	return FALSE;
}


void
Surface::drawing_area_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	Surface *s = (Surface *) user_data;

        if (s->width != allocation->width || s->height != allocation->height){
                s->width = allocation->width;
                s->height = allocation->height;

		s->Realloc ();
	}
	
	// if x or y changed we need to recompute the presentation matrix
	// because the toplevel position depends on the allocation.
	if (s->toplevel)
		s->toplevel->UpdateBounds ();
}

void
Surface::drawing_area_destroyed (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	// This is never called, why?
	printf ("------------------ WE ARE DESTROYED ---------------\n");
	s->drawing_area = NULL;
}



Surface *
surface_new (int width, int height)
{
	return new Surface (width, height);
}

void 
surface_destroy (Surface *s)
{
	delete s;
}

void
surface_resize (Surface *s, int width, int height)
{
	s->Resize (width, height);
}

void
surface_attach (Surface *surface, UIElement *toplevel)
{
	surface->Attach (toplevel);
}

void
surface_paint (Surface *s, cairo_t *ctx, int x, int y, int width, int height)
{
	s->Paint (ctx, x, y, width, height);
}

void *
surface_get_drawing_area (Surface *s)
{
	return s->GetDrawingArea ();
}

void
surface_register_events (Surface *s,
			 callback_plain_event surface_resize)
{
	s->RegisterEvents (surface_resize);
}

void
Surface::SetTrans (bool trans)
{
	transparent = trans;
	if (drawing_area)
		gtk_widget_queue_draw (drawing_area);
}


void 
surface_set_trans (Surface *s, bool trans)
{
	s->SetTrans (trans);
}

bool
surface_get_trans (Surface *s)
{
	return s->GetTrans ();
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
