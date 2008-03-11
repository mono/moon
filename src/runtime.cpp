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
#define Region _joe_ball
#include <gdk/gdkx.h>

#include <gdk/gdkkeysyms.h>
 
#include <cairo-xlib.h>
#undef Visual
#undef Region

#include "runtime.h"
#include "canvas.h"
#include "control.h"
#include "color.h"
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
#include "dirty.h"
#include "fullscreen.h"
#include "utils.h"


//#define DEBUG_INVALIDATE 1
//#define RENDER_INDIVIDUALLY 1
#define DEBUG_REFCNT 0

#define CAIRO_CLIP 0
#define TIME_CLIP 0
#define TIME_REDRAW 1

int Surface::ResizeEvent = -1;
int Surface::FullScreenChangeEvent = -1;
int Surface::ErrorEvent = -1;

static bool inited = false;
static bool g_type_inited = false;
guint32 moonlight_flags = 0;

static struct {
	const char *override;
	guint32 flag;
	bool set;
} overrides[] = {
	{ "text=pango",        RUNTIME_INIT_PANGO_TEXT_LAYOUT,     true  },
	{ "text=silverlight",  RUNTIME_INIT_PANGO_TEXT_LAYOUT,     false },
	{ "codecs=microsoft",  RUNTIME_INIT_MICROSOFT_CODECS,      true  },
	{ "codecs=ffmpeg",     RUNTIME_INIT_MICROSOFT_CODECS,      false },
	{ "timesource=manual", RUNTIME_INIT_MANUAL_TIMESOURCE,     true  },
	{ "timesource=system", RUNTIME_INIT_MANUAL_TIMESOURCE,     false },
	{ "expose=show",       RUNTIME_INIT_SHOW_EXPOSE,           true  },
	{ "expose=hide",       RUNTIME_INIT_SHOW_EXPOSE,           false },
	{ "clipping=show",     RUNTIME_INIT_SHOW_CLIPPING,         true  },
	{ "clipping=hide",     RUNTIME_INIT_SHOW_CLIPPING,         false },
	{ "bbox=show",         RUNTIME_INIT_SHOW_BOUNDING_BOXES,   true  },
	{ "bbox=hide",         RUNTIME_INIT_SHOW_BOUNDING_BOXES,   false },
	{ "fps=show",          RUNTIME_INIT_SHOW_FPS,              true  },
	{ "fps=hide",          RUNTIME_INIT_SHOW_FPS,              false },
	{ "render=ftb",        RUNTIME_INIT_RENDER_FRONT_TO_BACK,  true  },
	{ "render=btf",        RUNTIME_INIT_RENDER_FRONT_TO_BACK,  false },
	{ "cache=show",	       RUNTIME_INIT_SHOW_CACHE_SIZE,       true  },
	{ "cache=hide",        RUNTIME_INIT_SHOW_CACHE_SIZE,       false },
	{ "converter=yuv",     RUNTIME_INIT_CONVERTER_YUV,         true  },
	{ "converter=ffmpeg",  RUNTIME_INIT_CONVERTER_YUV,         false },
	{ "shapecache=yes",    RUNTIME_INIT_USE_SHAPE_CACHE,	   true  },
	{ "shapecache=no",     RUNTIME_INIT_USE_SHAPE_CACHE,	   false }
};

#define RENDER_EXPOSE (moonlight_flags & RUNTIME_INIT_SHOW_EXPOSE)

static void
fps_report_default (Surface *surface, int nframes, float nsecs, void *user_data)
{
	printf ("Rendered %d frames in %.3fs = %.3f FPS\n", nframes, nsecs, nframes / nsecs);
}

static void
cache_report_default (Surface *surface, long bytes, void *user_data)
{
	printf ("Cache size is ~%.3f MB\n", bytes / 1048576.0);
}

static cairo_t *
runtime_cairo_create (GdkWindow *drawable, GdkVisual *visual)
{
	int width, height;
	cairo_surface_t *surface;
	cairo_t *cr;

	gdk_drawable_get_size (drawable, &width, &height);
	surface = cairo_xlib_surface_create (gdk_x11_drawable_get_xdisplay (drawable),
					     gdk_x11_drawable_get_xid (drawable),
					     GDK_VISUAL_XVISUAL (visual),
					     width, height);
      
	cr = cairo_create (surface);
	cairo_surface_destroy (surface);
			    
	return cr;
}

void
Surface::CreateSimilarSurface ()
{
	if (widget == NULL || widget->window == NULL)
		return;
	
	cairo_t *ctx = runtime_cairo_create (widget->window, gdk_drawable_get_visual (widget->window));
	
	if (cairo_xlib)
		cairo_destroy (cairo_xlib);
	
	cairo_surface_t *xlib_surface = cairo_surface_create_similar (
		cairo_get_target (ctx), 
		CAIRO_CONTENT_COLOR_ALPHA,
		width, height);
	
	cairo_destroy (ctx);
	
	cairo_xlib = cairo_create (xlib_surface);
	cairo_surface_destroy (xlib_surface);
}


Surface::Surface(int w, int h, bool windowless)
  : downloader_context (NULL),
    width (w), height (h), buffer (0), pixbuf (NULL),
    using_cairo_xlib_surface(0),
    cairo_buffer_surface (NULL), cairo_buffer(NULL),
    cairo_xlib(NULL), cairo (NULL), transparent(false),
    background_color(NULL),
    widget (NULL),
    widget_normal (NULL),
    widget_fullscreen (NULL),
    cursor (MouseCursorDefault),
    mouse_event (NULL)
{
	background_color = new Color (1, 1, 1, 0);

	if (!windowless) {
		widget = gtk_event_box_new ();
		gtk_widget_set_size_request (widget, width, height);
		InitializeWidget (widget);
	}
	
	buffer = NULL;

	normal_width = width;
	normal_height = height;

	toplevel = NULL;
	input_list = new List ();
	captured = false;

	Realloc ();
	
	full_screen = false;
	can_full_screen = false;

	time_manager = new TimeManager ();
	time_manager->Start ();

	full_screen_message = NULL;
	source_location = NULL;

	render = NULL;
	render_data = NULL;

	invalidate = NULL;
	invalidate_data = NULL;

	fps_report = fps_report_default;
	fps_data = NULL;

	fps_nframes = 0;
	fps_start = 0;

	cache_report = cache_report_default;
	cache_data = NULL;

	cache_size_in_bytes = 0;
	cache_size_ticker = 0;

	emittingMouseEvent = false;
	pendingCapture = NULL;
	pendingReleaseCapture = false;

#ifdef DEBUG
	debug_selected_element = NULL;
#endif

	up_dirty = new List ();
	down_dirty = new List ();
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
	time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);

	if (toplevel) {
		toplevel->SetSurface (NULL);
		toplevel->unref ();
		toplevel = NULL;
	}

	if (buffer) {
		free (buffer);
		buffer = NULL;
	}
	
#if DEBUG
	if (debug_selected_element) {
		debug_selected_element->unref ();
		debug_selected_element = NULL;
	}
#endif
	
	if (full_screen_message) {
		HideFullScreenMessage ();
	}

	delete input_list;
	input_list = NULL;

	if (source_location) {
		g_free (source_location);
		source_location = NULL;
	}
	
	cairo_destroy (cairo_buffer);
	cairo_buffer = NULL;

	cairo_surface_destroy (cairo_buffer_surface);
	cairo_buffer_surface = NULL;

	DestroyWidget (widget_fullscreen);
	widget_fullscreen = NULL;
	
	DestroyWidget (widget);
	widget = NULL;
	
	delete background_color;
	background_color = NULL;

	time_manager->unref ();
	time_manager = NULL;

	delete up_dirty;
	delete down_dirty;
}

void
Surface::SetCursor (MouseCursor new_cursor)
{
	if (new_cursor != cursor) {
		cursor = new_cursor;

		if (!widget)
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

		gdk_window_set_cursor (widget->window, c);
	}
}

void
Surface::ConnectEvents (bool realization_signals)
{
	if (!widget)
		return;

	gtk_signal_connect (GTK_OBJECT (widget), "expose_event",
			    G_CALLBACK (Surface::expose_event_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "motion_notify_event",
			    G_CALLBACK (motion_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "enter_notify_event",
			    G_CALLBACK (crossing_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "leave_notify_event",
			    G_CALLBACK (crossing_notify_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "key_press_event",
			    G_CALLBACK (key_press_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "key_release_event",
			    G_CALLBACK (key_release_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "button_press_event",
			    G_CALLBACK (button_press_callback), this);

	gtk_signal_connect (GTK_OBJECT (widget), "button_release_event",
			    G_CALLBACK (button_release_callback), this);

	if (realization_signals) {
		gtk_signal_connect (GTK_OBJECT (widget), "realize",
				    G_CALLBACK (realized_callback), this);

		gtk_signal_connect (GTK_OBJECT (widget), "unrealize",
				    G_CALLBACK (unrealized_callback), this);

		if (GTK_WIDGET_REALIZED (widget))
			realized_callback (widget, this);
	}
}

void
Surface::Attach (UIElement *element)
{
	bool first = FALSE;

	if (!element)
		return;

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

	// make sure we have a namescope at the toplevel so that names
	// can be registered/resolved properly.
	if (NameScope::GetNameScope (canvas) == NULL) {
		NameScope::SetNameScope (canvas, new NameScope());
	}

	canvas->SetSurface (this);
	toplevel = canvas;

	// First time we connect the surface, start responding to events
	if (first)
		ConnectEvents (true);

	canvas->OnLoaded ();

	bool change_size = false;
	//
	// If the did not get a size specified
	//
	if (normal_width == 0){
		Value *v = toplevel->GetValue (FrameworkElement::WidthProperty);

		if (v){
			normal_width = (int) v->AsDouble ();
			if (normal_width < 0)
				normal_width = 0;
			change_size = true;
		}
	}

	if (normal_height == 0){
		Value *v = toplevel->GetValue (FrameworkElement::HeightProperty);

		if (v){
			normal_height = (int) v->AsDouble ();
			if (normal_height < 0)
				normal_height = 0;
			change_size = true;
		}
	}

	if (!full_screen) {
		height = normal_height;
		width = normal_width;
	}

	if (change_size) {
		Realloc ();
		
		Emit (ResizeEvent);
	}

	canvas->UpdateBounds ();
	canvas->Invalidate ();
}

void
Surface::Invalidate (Rect r)
{
	if (invalidate)
		invalidate (this, r, invalidate_data);
	else if (widget)
		gtk_widget_queue_draw_area (widget,
					    (int) (widget->allocation.x + r.x), 
					    (int) (widget->allocation.y + r.y), 
					    (int) r.w, (int)r.h);
}


void
Surface::Paint (cairo_t *ctx, int x, int y, int width, int height)
{
	Rect r = Rect (x, y, width, height);
	Region region = Region (r);
	Paint (ctx, &region);
}

void
Surface::Paint (cairo_t *ctx, Region *region)
{
	if (!toplevel)
		return;

#if FRONT_TO_BACK_STATS
	uielements_rendered_front_to_back = 0;
	uielements_rendered_back_to_front = 0;
#endif

	if (IsAnythingDirty())
		ProcessDirtyElements ();

	bool did_front_to_back = false;

	List *render_list = new List ();
	Region *copy = new Region (region);

	if (moonlight_flags & RUNTIME_INIT_RENDER_FRONT_TO_BACK) {
		if (full_screen_message)
			full_screen_message->FrontToBack (copy, render_list);

		toplevel->FrontToBack (copy, render_list);

		if (render_list->IsEmpty()) {
			g_warning ("error building up render list - region corresponds to no UIElement intersections");
		}
		else {
			Region *empty_region = new Region ();

			RenderNode *node;
			while ((node = (RenderNode*)render_list->First())) {
				Region *r = node->region ? node->region : empty_region;
				UIElement *ui = node->uielement;
			 
#if FRONT_TO_BACK_STATS
				uielements_rendered_front_to_back ++;
#endif
 
				if (node->pre_render)
					node->pre_render (ctx, ui, r, true);

				if (node->render_element)
					ui->Render (ctx, r);

				if (node->post_render)
					node->post_render (ctx, ui, r, true);

				render_list->Remove (node);
			}

			did_front_to_back = true;
			delete empty_region;
		}

		delete render_list;
		delete copy;
	}

	if (!did_front_to_back) {
		toplevel->DoRender (ctx, region);

		if (full_screen_message) {
			full_screen_message->DoRender (ctx, region);
		}
	}

#if FRONT_TO_BACK_STATS
	printf ("UIElements rendered front-to-back: %d\n", uielements_rendered_front_to_back);
	printf ("UIElements rendered back-to-front: %d\n", uielements_rendered_back_to_front);
#endif

#ifdef DEBUG
		if (debug_selected_element) {
			Rect bounds = debug_selected_element->GetSubtreeBounds();
// 			printf ("debug_selected_element is %s\n", debug_selected_element->GetName());
// 			printf ("bounds is %g %g %g %g\n", bounds.x, bounds.y, bounds.w, bounds.h);
			cairo_save (ctx);
			//RenderClipPath (ctx);
			cairo_new_path (ctx);
			cairo_identity_matrix (ctx);
			cairo_set_source_rgba (ctx, 1.0, 0.5, 0.2, 1.0);
			cairo_set_line_width (ctx, 1);
			cairo_rectangle (ctx, bounds.x, bounds.y, bounds.w, bounds.h);
			cairo_stroke (ctx);
			cairo_restore (ctx);
		}
#endif
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
	gtk_widget_set_size_request (widget, width, height);
	
	if (!full_screen)
		Emit (ResizeEvent);
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

	if (toplevel)
		toplevel->UpdateBounds();
}

void
Surface::SetFullScreen (bool value)
{
	if (value && !can_full_screen) {
		g_warning ("You're not allowed to switch to fullscreen from where you're doing it.");
		return;
	}
	
	UpdateFullScreen (value);
	
}

bool
Surface::IsTopLevel (UIElement* top)
{
	if (top == NULL)
		return FALSE;
		
	return top == toplevel || top == full_screen_message;
}

void
Surface::ShowFullScreenMessage ()
{
	g_return_if_fail (full_screen_message == NULL);
	g_return_if_fail (toplevel && toplevel->Is (Type::PANEL));
	
	Type::Kind dummy;
	DependencyObject* message = xaml_create_from_str (NULL, FULLSCREEN_MESSAGE, false, &dummy);
	
	if (!message) {
		printf ("Unable to create fullscreen message.\n");
		return;
	}
	
	if (!message->Is (Type::CANVAS)) {
		printf ("Unable to create fullscreen message, got a %s, expected at least a UIElement.\n", message->GetTypeName ());
		message->unref ();
		return;
	}
	
	full_screen_message = (Canvas*) message;
	
	DependencyObject* message_object = full_screen_message->FindName ("message");
	DependencyObject* url_object = full_screen_message->FindName ("url");
	TextBlock* message_block = (message_object != NULL && message_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) message_object : NULL;
	TextBlock* url_block = (url_object != NULL && url_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) url_object : NULL;
	
	Value* tmp = full_screen_message->GetValue (UIElement::RenderTransformProperty);
	Transform* transform = tmp != NULL ? tmp->AsTransform () : NULL;// full_screen_message->uielement_get_render_transform (full_screen_message);
	
	double box_width = framework_element_get_width (full_screen_message);
	double box_height = framework_element_get_height (full_screen_message);
	
	// Set the url in the box
	if (url_block != NULL)  {
		char* url = NULL;
		if (g_str_has_prefix (source_location, "http://")) {
			char* path = strchr (source_location + 7, '/');
			if (path != NULL && path > source_location + 7) {
				url = g_strndup (source_location, path - source_location);  
			} else {
				url = g_strdup (source_location);
			}
		} else if (g_str_has_prefix (source_location, "file://")) {
			url = g_strdup ("file://");
		} else {
			url = g_strdup (source_location);
		}
		text_block_set_text (url_block, url ? url : (char*) "file://");
		g_free (url);
	}
	
	// The box is not made bigger if the url doesn't fit.
	// MS has an interesting text rendering if the url doesn't
	// fit: the text is overflown to the left.
	// Since only the server is shown, this shouldn't
	// happen on a regular basis though.
	
	// Center the url block
	if (url_block != NULL) {
		double url_width = url_block->GetActualWidth ();
		url_block->SetValue (Canvas::LeftProperty, (box_width - url_width) / 2);	
	}
	// Center the message block
	if (message_block != NULL) {
		double message_width = message_block->GetActualWidth ();
		message_block->SetValue (Canvas::LeftProperty, (box_width - message_width) / 2);
	}	

	// Put the box in the middle of the screen
	transform->SetValue (TranslateTransform::XProperty, (width - box_width) / 2);
	transform->SetValue (TranslateTransform::YProperty, (height - box_height) / 2);
	
	full_screen_message->SetSurface (this);
	full_screen_message->OnLoaded ();
	full_screen_message->Invalidate ();
}

void
Surface::SetSourceLocation (const char* location)
{
	if (source_location)
		g_free (source_location);
	source_location = g_strdup (location);
}

void 
Surface::HideFullScreenMessage ()
{
	if (full_screen_message) {
		full_screen_message->unref ();
		full_screen_message = NULL;
	}
}

void
Surface::UpdateFullScreen (bool value)
{
	if (value == full_screen)
		return;
	
	if (value) {
		widget_fullscreen = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		
		// Flip the drawing area
		widget_normal = widget;
		widget = widget_fullscreen;
		
		// Get the screen size
		int screen_width = gdk_screen_get_width (gdk_screen_get_default ());
		int screen_height = gdk_screen_get_height (gdk_screen_get_default ());
		
		//screen_width = (int) (screen_width * 0.8);
		//screen_height = (int) (screen_height * 0.8);
		
		width = screen_width;
		height = screen_height;
		
		gtk_widget_set_size_request (widget, width, height);
		gtk_window_fullscreen (GTK_WINDOW (widget));
		
		InitializeWidget (widget);
		
		ShowFullScreenMessage ();
		
		ConnectEvents (false);
	} else {
		HideFullScreenMessage ();
		
		// Flip back.
		widget = widget_normal;

		// Destroy the fullscreen widget.
		GtkWidget *fs = widget_fullscreen;
		widget_fullscreen = NULL;
		DestroyWidget (fs);
		
		width = normal_width;
		height = normal_height;
	}
	full_screen = value;
	
	Realloc ();

	time_manager->GetSource()->Stop();
	Emit (FullScreenChangeEvent);
	time_manager->GetSource()->Start();
}

void 
Surface::DestroyWidget (GtkWidget *widget)
{
	if (widget) {
		g_signal_handlers_disconnect_matched (widget,
						      (GSignalMatchType) G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL, this);
		gtk_widget_destroy (widget);
	}
}

void
Surface::InitializeWidget (GtkWidget *widget)
{
	// don't let gtk clear the window we'll do all the drawing.
	//gtk_widget_set_app_paintable (widget, TRUE);
	gtk_widget_set_double_buffered (widget, FALSE);
	
	//
	// Set to true, need to change that to FALSE later when we start
	// repainting again.   
	//
	if (GTK_IS_EVENT_BOX (widget))
		gtk_event_box_set_visible_window (GTK_EVENT_BOX (widget), FALSE);
	
	g_signal_connect (G_OBJECT (widget), "size-allocate",
			  G_CALLBACK (widget_size_allocate), this);
	g_signal_connect (G_OBJECT (widget), "destroy",
			  G_CALLBACK (widget_destroyed), this);
	
	gtk_widget_add_events (widget, 
			       GDK_POINTER_MOTION_MASK |
			       GDK_POINTER_MOTION_HINT_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK);

	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);

	gtk_widget_show (widget);

	// The window has to be realized for this call to work
	gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_CURSOR);
	/* we need to explicitly enable the devices */
	for (GList *l = gdk_devices_list(); l; l = l->next) {

#if THIS_NOLONGER_BREAKS_LARRYS_MOUSE
		GdkDevice *device = GDK_DEVICE(l->data);
		//if (!device->has_cursor)
		gdk_device_set_mode (device, GDK_MODE_SCREEN);
#endif
	}

	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
}

void
Surface::render_cb (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Surface *s = (Surface *) closure;
	int64_t now;

	GDK_THREADS_ENTER ();
	s->ProcessDirtyElements ();
	GDK_THREADS_LEAVE ();

	if ((moonlight_flags & RUNTIME_INIT_SHOW_FPS) && s->fps_start == 0)
		s->fps_start = get_now ();
	
	if (s->render)
		s->render (s, s->render_data);
	else if (s->widget)
		gdk_window_process_updates (GTK_WIDGET (s->widget)->window, FALSE);
	
	if ((moonlight_flags & RUNTIME_INIT_SHOW_FPS) && s->fps_report) {
		s->fps_nframes++;
		
		if ((now = get_now ()) > (s->fps_start + TIMESPANTICKS_IN_SECOND)) {
			float nsecs = (now - s->fps_start) / TIMESPANTICKS_IN_SECOND_FLOAT;
			
			s->fps_report (s, s->fps_nframes, nsecs, s->fps_data);
			s->fps_nframes = 0;
			s->fps_start = now;
		}
	}

	if ((moonlight_flags & RUNTIME_INIT_SHOW_CACHE_SIZE) && s->cache_report) {
		// By default we report cache status every 50 render's.
		// Should be enough for everybody, but syncing to ie. 1s sounds
		// better.
		if (s->cache_size_ticker == 50) {
			s->cache_report (s, s->cache_size_in_bytes, s->cache_data);
			s->cache_size_ticker = 0;
		} else
			s->cache_size_ticker++;
	}
}

void
Surface::update_input_cb (EventObject *sender, EventArgs *calldata, gpointer closure)
{
#if notyet
	Surface *s = (Surface *) closure;

	s->HandleMouseEvent (emit_MouseMove, true, true, false, s->mouse_event_state, s->mouse_event_x, s->mouse_event_y);
	s->UpdateCursorFromInputList ()
#endif
}

gboolean
Surface::realized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	s->CreateSimilarSurface ();
	s->cairo = s->cairo_xlib;

#ifdef USE_XRANDR
#if INTEL_DRIVERS_STOP_SUCKING
	// apparently the i965 drivers blank external screens when
	// getting the screen info (um, ugh?).  needless to say, this
	// annoyance is worse than not using the monitor's refresh as
	// the upper bound for our fps.
	//
	// http://lists.freedesktop.org/archives/xorg/2007-August/027616.html
	int event_base, error_base;
	GdkWindow *gdk_root = gtk_widget_get_root_window (widget);
	Display *dpy = GDK_WINDOW_XDISPLAY(gdk_root);
	Window root = GDK_WINDOW_XID (gdk_root);
	if (XRRQueryExtension (dpy, &event_base, &error_base)) {
		XRRScreenConfiguration *info = XRRGetScreenInfo (dpy,
								 root);
		short rate = XRRConfigCurrentRate (info);
		printf ("screen refresh rate = %d\n", rate);
 		time_manager->SetMaximumRefreshRate (rate);
		XRRFreeScreenConfigInfo (info);
	}
#endif
#endif
	
	s->time_manager->AddHandler (TimeManager::RenderEvent, render_cb, s);
	s->time_manager->AddHandler (TimeManager::UpdateInputEvent, update_input_cb, s);
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
	s->time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, s);
	s->time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, s);
	return TRUE;
}

gboolean
Surface::expose_to_drawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y)
{
	if (event->area.x > width || event->area.y > height)
		return TRUE;

#if TIME_REDRAW
	STARTTIMER (expose, "redraw");
#endif
	cairo = cairo_xlib;

	if (cairo) {
		//
		// BIG DEBUG BLOB
		// 
		if (cairo_status (cairo) != CAIRO_STATUS_SUCCESS){
			printf ("expose event: the cairo context has an error condition and refuses to paint: %s\n", 
				cairo_status_to_string (cairo_status (cairo)));
		}
	}

#ifdef DEBUG_INVALIDATE
	printf ("Got a request to repaint at %d %d %d %d\n", event->area.x, event->area.y, event->area.width, event->area.height);
#endif
	GdkPixmap *pixmap = NULL;
	if (widget) {
		/* create our own backbuffer if we're windowed.  in
		   the windowless case we assume we're drawing to the
		   backbuffer already, so no need for the additional
		   step. */
		pixmap = gdk_pixmap_new (drawable, MAX (event->area.width, 1), MAX (event->area.height, 1), -1);
	}
	cairo_t *ctx = runtime_cairo_create (widget ? pixmap : drawable, visual);
	Region *region = new Region (event->region);

	region->Offset (-off_x, -off_y);
	cairo_surface_set_device_offset (cairo_get_target (ctx),
					 off_x - event->area.x, 
					 off_y - event->area.y);

	region->Draw (ctx);
	cairo_clip (ctx);
	//
	// These are temporary while we change this to paint at the offset position
	// instead of using the old approach of modifying the topmost Canvas (a no-no),
	//
	// The flag "transparent" is here because I could not
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



	if (transparent) {
		if (widget) {
			cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
			region->Draw (ctx);
			cairo_paint (ctx);
		}

		cairo_set_source_rgba (ctx,
				       background_color->r,
				       background_color->g,
				       background_color->b,
				       background_color->a);
	}
	else {
		cairo_set_source_rgb (ctx,
				      background_color->r,
				      background_color->g,
				      background_color->b);
	}

	cairo_paint (ctx);


	cairo_save (ctx);
	cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
	Paint (ctx, region);

	if (RENDER_EXPOSE) {
		region->Draw (ctx);
		cairo_set_line_width (ctx, 2.0);
		cairo_set_source_rgb (ctx, (double)(frames % 2), (double)((frames + 1) % 2), (double)((frames / 3) % 2));
		cairo_stroke (ctx);
	}

	delete (region);

	if (widget) {
		GdkGC *gc = gdk_gc_new (pixmap);

		gdk_gc_set_clip_region (gc, event->region);

		gdk_draw_drawable (drawable, gc, pixmap,
				   0, 0,
				   event->area.x, event->area.y,
				   event->area.width, event->area.height);
	
		g_object_unref (pixmap);
		g_object_unref (gc);
	}
	cairo_destroy (ctx);

#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

	
	return TRUE;
}

gboolean
Surface::expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	Surface *s = (Surface *) data;

	s->frames++;

	if (widget == NULL)
		return TRUE;

	return s->expose_to_drawable (widget->window,
				      gdk_drawable_get_visual (widget->window),
				      event,
				      widget->allocation.x,
				      widget->allocation.y);
}

RenderNode::RenderNode (UIElement *el,
			Region *region,
			bool render_element,
			RenderFunc pre,
			RenderFunc post)

{
	uielement = el;
	uielement->ref();
	this->region = region;
	this->render_element = render_element;
	this->pre_render = pre;
	this->post_render = post;
}

RenderNode::~RenderNode ()
{
	if (uielement) {
		uielement->unref ();
		uielement = NULL;
	}

	if (region)
		delete region;
}

UIElementNode::UIElementNode (UIElement *el)
{
	uielement = el;
	uielement->ref();
}

UIElementNode::~UIElementNode ()
{
	uielement->unref();
	uielement = NULL;
}

// I really wish C++ had anonymous delegates...
static bool
emit_MouseLeftButtonDown (UIElement *element, GdkEvent *event)
{
	return element->EmitMouseLeftButtonDown (event);
}

static bool
emit_MouseLeftButtonUp (UIElement *element, GdkEvent *event)
{
	return element->EmitMouseLeftButtonUp (event);
}

static bool
emit_MouseMove (UIElement *element, GdkEvent *event)
{
	return element->EmitMouseMove (event);
}

static bool
emit_MouseEnter (UIElement *element, GdkEvent *event)
{
	return element->EmitMouseEnter (event);
}

static bool
emit_MouseLeave (UIElement *element, GdkEvent *)
{
	return element->EmitMouseLeave ();
}

void
Surface::PerformCapture (UIElement *capture)
{
	// "Capturing" the mouse pointer at an element forces us to
	// use the path up the hierarchy from that element to the root
	// as the input list, regardless of where the pointer actually
	// is.

	// XXX we should check if the input_list already starts at
	// @capture.
	List *new_input_list = new List();
	while (capture) {
		new_input_list->Append (new UIElementNode (capture));
		capture = capture->GetVisualParent();
	}

	delete input_list;
	input_list = new_input_list;
	captured = true;
	pendingCapture = NULL;
}

void
Surface::PerformReleaseCapture ()
{
	// These need to be set before calling HandleMouseEvent as
	// "captured" determines the input_list calculation, and
	// "pendingReleaseCapture", when set, causes an infinite
	// recursive loop.
	captured = false;
	pendingReleaseCapture = false;

	// this causes any new elements we're over to be Enter'ed.  MS
	// doesn't Leave the element that had the mouse captured,
	// though.
	HandleMouseEvent (NULL, false, true, false, mouse_event);
}

bool
Surface::SetMouseCapture (UIElement *capture)
{
	if (capture != NULL && (captured || pendingCapture))
		return false;

	// we delay the actual capture/release until the end of the
	// event bubbling.  need more testing on MS here to see what
	// happens if through the course of a single event bubbling up
	// to the root, one element captures and another releases.
	// Our current implementation has it so that any "release"
	// causes all captures to be ignored.
	if (capture == NULL) {
		if (emittingMouseEvent)
			pendingReleaseCapture = true;
		else
			PerformReleaseCapture ();
	}
	else {
		if (emittingMouseEvent) {
			pendingCapture = capture;
		}
		else {
			PerformCapture (capture);
		}
	}

	return true;
}

bool
Surface::EmitEventOnList (MoonlightEventEmitFunc emitter, List *list, GdkEvent *event, int end_idx)
{
	bool handled = false;

	if (emitter == NULL)
		return handled;

	int idx;
	UIElementNode *node;

	if (end_idx == -1)
		end_idx = list->Length();

	emittingMouseEvent = true;
	for (node = (UIElementNode*)list->First(), idx = 0; node && idx < end_idx; node = (UIElementNode*)node->next, idx++) {
		bool h = emitter (node->uielement, event);
		if (h)
			handled = true;
	}
	emittingMouseEvent = false;

	return handled;
}

void
Surface::FindFirstCommonElement (List *l1, int *index1,
				 List *l2, int *index2)
{
	// we exploit the fact that for a list with any common
	// elements, the lists will be identical from the first common
	// element to the end of the lists.  So, we start from the
	// last elements in both lists and walk backward to the start,
	// looking for the first elements that don't match.
	//
	// this algorithm is O(MAX(n,m)), but it's unclear whether or
	// not this is actually better than the O(n*m) approach, since
	// the O(n*m) approach will often find a match on the first
	// comparison (especially when the user is slowly moving the
	// mouse around in the same element), and skip the rest.
	int i1, i2;
	UIElementNode *ui1, *ui2;

	*index1 = -1;
	*index2 = -1;

	ui1 = (UIElementNode*)l1->Last();
	i1 = l1->Length() - 1;

	ui2 = (UIElementNode*)l2->Last();
	i2 = l2->Length() - 1;

	while (ui1 && ui2) {

		if (ui1->uielement == ui2->uielement) {
			*index1 = i1;
			*index2 = i2;
		}
		else {
			return;
		}

		ui1 = (UIElementNode*)ui1->prev;
		ui2 = (UIElementNode*)ui2->prev;
		i1--;
		i2--;
	}
}

bool
Surface::HandleMouseEvent (MoonlightEventEmitFunc emitter, bool emit_leave, bool emit_enter, bool force_emit, GdkEvent *event)
{
	bool handled = false;

	// we can end up here if mozilla pops up the JS timeout
	// dialog.  The problem is that JS might have registered a
	// handler for the event we're going to emit, so when we end
	// up tripping the timeout while in JS, mozilla pops up the
	// dialog, which causes a crossing-notify event to be emitted.
	// This causes HandleMouseEvent to be called, and the original
	// input_list is deleted.  the crossing-notify event is
	// handled, then we return to the event that tripped the
	// timeout, we crash.
	if (emittingMouseEvent)
		return false;

	if (toplevel == NULL || event == NULL)
		return false;

	// FIXME this should probably use mouse event args
	if (IsAnythingDirty())
		ProcessDirtyElements();

	if (captured) {
		// if the mouse is captured, the input_list doesn't ever
		// change, and we don't emit enter/leave events.  just emit
		// the event on the input_list.
		handled = EmitEventOnList (emitter, input_list, event, -1);
	}
	else {
		int surface_index;
		int new_index;

		// Accumulate a new input_list, which contains the
		// most deeply nested hit testable UIElement covering
		// the point (x,y), and all visual parents up the
		// hierarchy to the root.
		List *new_input_list = new List ();
		double x, y;

		gdk_event_get_coords (event, &x, &y);

		cairo_t *ctx = cairo;
		bool destroy_ctx = false;
		if (ctx == NULL) {
			// this will happen in the windowless case
			ctx = measuring_context_create ();
			destroy_ctx = true;
		}

		toplevel->HitTest (ctx, x, y, new_input_list);
		
		if (destroy_ctx)
			measuring_context_destroy (ctx);

		// for 2 lists:
		//   l1:  [a1, a2, a3, a4, ... ]
		//   l2:  [b1, b2, b3, b4, ... ]
		//
		// For identical lists:
		// 
		//   only the primary event is emitted for all
		//   elements of l2, in order.
		//
		// For lists that differ, Enter/Leave events must be
		// emitted.
		//
		//   If the first few nodes in each list differ, and,
		//   for instance bn == am, we know that [am...] ==
		//   [bn...]
		//
		//   when emitting a given event on b1, MS generally
		//   emits Leave events on [a1, a2, a3, ... am-1], and
		//   Enter events on [b1, b2, ... bn-1].
		//
		//   For most event types, that's all that happens if
		//   the lists differ.  For MouseLeftButtonDown (we
		//   also do it for MouseLeftButtonUp), we also emit
		//   the primary event on l2 after the enter/leave
		//   events.
		//
		FindFirstCommonElement (input_list, &surface_index,
					new_input_list, &new_index);

		if (emit_leave)
			handled = EmitEventOnList (emit_MouseLeave, input_list, event, surface_index);

		delete input_list;
		input_list = new_input_list;

		if (emit_enter)
			handled = EmitEventOnList (emit_MouseEnter, input_list, event, new_index) || handled;

		if ((surface_index == 0 && new_index == 0) || force_emit) {
			handled = EmitEventOnList (emitter, input_list, event, -1) || handled;
		}
	}

	// Perform any captures/releases that are pending after the
	// event is bubbled.
	if (pendingCapture)
		PerformCapture (pendingCapture);
	else if (pendingReleaseCapture)
		PerformReleaseCapture ();

	return handled;
}

void
Surface::UpdateCursorFromInputList ()
{
	MouseCursor new_cursor = MouseCursorDefault;
	
	// loop over the input list in order until we hit a node that
	// has its cursor set to the non-default.
	UIElementNode *node;
	for (node = (UIElementNode*)input_list->First(); node; node = (UIElementNode*)node->next) {
		new_cursor = (MouseCursor)node->uielement->GetValue (UIElement::CursorProperty)->AsInt32();
		if (new_cursor != MouseCursorDefault)
			break;
	}

	SetCursor (new_cursor);
}

gboolean
Surface::button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	if (button->button != 1)
		return FALSE;

	s->SetCanFullScreen (true);

	if (s->mouse_event)
		gdk_event_free (s->mouse_event);
	s->mouse_event = gdk_event_copy ((GdkEvent*)button);

	s->HandleMouseEvent (emit_MouseLeftButtonUp, true, true, true, s->mouse_event);

	s->UpdateCursorFromInputList ();
	s->SetCanFullScreen (false);

	// XXX MS appears to do this here, which is completely stupid.
	if (s->captured)
		s->PerformReleaseCapture ();

	return TRUE;
}

gboolean
Surface::button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	if (widget)
		gtk_widget_grab_focus (widget);

	if (button->button != 1)
		return FALSE;

	s->SetCanFullScreen (true);

	if (s->mouse_event)
		gdk_event_free (s->mouse_event);
	s->mouse_event = gdk_event_copy ((GdkEvent*)button);

	bool handled = s->HandleMouseEvent (emit_MouseLeftButtonDown, true, true, true, s->mouse_event);

	s->UpdateCursorFromInputList ();
	s->SetCanFullScreen (false);
	
	return handled;
}

gboolean
Surface::motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->mouse_event)
		gdk_event_free (s->mouse_event);
	s->mouse_event = gdk_event_copy ((GdkEvent*)event);

	bool handled = s->HandleMouseEvent (emit_MouseMove, true, true, false, s->mouse_event);

	if (event->is_hint) {
#if GTK_CHECK_VERSION(2,12,0)
	  if (gtk_check_version (2, 12, 0))
	  	gdk_event_request_motions (event);
	  else
#endif
	    {
		int ix, iy;
		GdkModifierType state;
		gdk_window_get_pointer (event->window, &ix, &iy, (GdkModifierType*)&state);
	    }    
	}

	s->UpdateCursorFromInputList ();

	return handled;
}

gboolean
Surface::crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	Surface *s = (Surface *) data;
	bool handled;

	if (event->type == GDK_ENTER_NOTIFY) {
		if (s->mouse_event)
			gdk_event_free (s->mouse_event);
		s->mouse_event = gdk_event_copy ((GdkEvent*)event);
		
		handled = s->HandleMouseEvent (emit_MouseMove, true, true, false, s->mouse_event);

		s->UpdateCursorFromInputList ();
	
	} else {
		// forceably emit MouseLeave on the current input
		// list..  the "new" list computed by HandleMouseEvent
		// should be the same as the current one since we pass
		// in the same x,y but I'm not sure that's something
		// we can rely on.
		handled = s->HandleMouseEvent (emit_MouseLeave, false, false, true, s->mouse_event);

		// MS specifies that mouse capture is lost when you mouse out of the control
		if (s->captured)
			s->PerformReleaseCapture ();

		// clear out the input list so we emit the right
		// events when the pointer reenters the control.
		delete s->input_list;
		s->input_list = new List();
	}

	return handled;
}

Key
Surface::gdk_keyval_to_key (guint keyval)
{
	switch (keyval) {
	case GDK_BackSpace:				return KeyBACKSPACE;
	case GDK_Tab:					return KeyTAB;
	case GDK_Return: case GDK_KP_Enter:		return KeyENTER;
	case GDK_Shift_L: case GDK_Shift_R:		return KeySHIFT;
	case GDK_Control_L: case GDK_Control_R:		return KeyCTRL;
	case GDK_Alt_L: case GDK_Alt_R:			return KeyALT;
	case GDK_Caps_Lock:				return KeyCAPSLOCK;
	case GDK_Escape:				return KeyESCAPE;
	case GDK_space: case GDK_KP_Space:		return KeySPACE;
	case GDK_Page_Up: case GDK_KP_Page_Up:		return KeyPAGEUP;
	case GDK_Page_Down: case GDK_KP_Page_Down:	return KeyPAGEDOWN;
	case GDK_End: case GDK_KP_End:			return KeyEND;
	case GDK_Home: case GDK_KP_Home:		return KeyHOME;
	case GDK_Left: case GDK_KP_Left:		return KeyLEFT;
	case GDK_Up: case GDK_KP_Up:			return KeyUP;
	case GDK_Right: case GDK_KP_Right:		return KeyRIGHT;
	case GDK_Down: case GDK_KP_Down:		return KeyDOWN;
	case GDK_Insert: case GDK_KP_Insert:		return KeyINSERT;
	case GDK_Delete: case GDK_KP_Delete:		return KeyINSERT;
	case GDK_0:					return KeyDIGIT0;
	case GDK_1:					return KeyDIGIT1;
	case GDK_2:					return KeyDIGIT2;
	case GDK_3:					return KeyDIGIT3;
	case GDK_4:					return KeyDIGIT4;
	case GDK_5:					return KeyDIGIT5;
	case GDK_6:					return KeyDIGIT6;
	case GDK_7:					return KeyDIGIT7;
	case GDK_8:					return KeyDIGIT8;
	case GDK_9:					return KeyDIGIT9;
	case GDK_a: case GDK_A:				return KeyA;
	case GDK_b: case GDK_B:				return KeyB;
	case GDK_c: case GDK_C:				return KeyC;
	case GDK_d: case GDK_D:				return KeyD;
	case GDK_e: case GDK_E:				return KeyE;
	case GDK_f: case GDK_F:				return KeyF;
	case GDK_g: case GDK_G:				return KeyG;
	case GDK_h: case GDK_H:				return KeyH;
	case GDK_i: case GDK_I:				return KeyI;
	case GDK_j: case GDK_J:				return KeyJ;
	case GDK_k: case GDK_K:				return KeyK;
	case GDK_l: case GDK_L:				return KeyL;
	case GDK_m: case GDK_M:				return KeyM;
	case GDK_n: case GDK_N:				return KeyN;
	case GDK_o: case GDK_O:				return KeyO;
	case GDK_p: case GDK_P:				return KeyP;
	case GDK_q: case GDK_Q:				return KeyQ;
	case GDK_r: case GDK_R:				return KeyR;
	case GDK_s: case GDK_S:				return KeyS;
	case GDK_t: case GDK_T:				return KeyT;
	case GDK_u: case GDK_U:				return KeyU;
	case GDK_v: case GDK_V:				return KeyV;
	case GDK_w: case GDK_W:				return KeyW;
	case GDK_x: case GDK_X:				return KeyX;
	case GDK_y: case GDK_Y:				return KeyY;
	case GDK_z: case GDK_Z:				return KeyZ;
	  
	case GDK_F1: case GDK_KP_F1:			return KeyF1;
	case GDK_F2: case GDK_KP_F2:			return KeyF2;
	case GDK_F3: case GDK_KP_F3:			return KeyF3;
	case GDK_F4: case GDK_KP_F4:			return KeyF4;
	case GDK_F5:					return KeyF5;
	case GDK_F6:					return KeyF6;
	case GDK_F7:					return KeyF7;
	case GDK_F8:					return KeyF8;
	case GDK_F9:					return KeyF9;
	case GDK_F10:					return KeyF10;
	case GDK_F11:					return KeyF11;
	case GDK_F12:					return KeyF12;
	  
	case GDK_KP_0:					return KeyNUMPAD0;
	case GDK_KP_1:					return KeyNUMPAD1;
	case GDK_KP_2:					return KeyNUMPAD2;
	case GDK_KP_3:					return KeyNUMPAD3;
	case GDK_KP_4:					return KeyNUMPAD4;
	case GDK_KP_5:					return KeyNUMPAD5;
	case GDK_KP_6:					return KeyNUMPAD6;
	case GDK_KP_7:					return KeyNUMPAD7;
	case GDK_KP_8:					return KeyNUMPAD8;
	case GDK_KP_9:					return KeyNUMPAD9;
	  
	case GDK_KP_Multiply:				return KeyMULTIPLY;
	case GDK_KP_Add:				return KeyADD;
	case GDK_KP_Subtract:				return KeySUBTRACT;
	case GDK_KP_Decimal:				return KeyDECIMAL;
	case GDK_KP_Divide:				return KeyDIVIDE;
	  
	default:
		return KeyKEYUNKNOWN;
	}
}

bool
Surface::FullScreenKeyHandled (GdkEventKey *key)
{
	if (!GetFullScreen ())
		return false;
		
	// If we're in fullscreen mode no key events are passed through.
	// We only handle Esc, to exit fullscreen mode.
	if (key->keyval == GDK_Escape) {
		SetFullScreen (false);
	}
	return true;
}

gboolean 
Surface::key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->FullScreenKeyHandled (key))
		return TRUE;

#if DEBUG_MARKER_KEY
	static int debug_marker_key_in = 0;
	if (key->keyval == GDK_d || key->keyval == GDK_D) {
		if (! debug_marker_key_in)
			printf ("<--- DEBUG MARKER KEY IN (%f) --->\n", get_now () / 10000000.0);
		else
			printf ("<--- DEBUG MARKER KEY OUT (%f) --->\n", get_now () / 10000000.0);
		debug_marker_key_in = ! debug_marker_key_in;
		return TRUE;
	}
#endif

	s->SetCanFullScreen (true);
	// key events are only ever delivered to the toplevel
	s->toplevel->EmitKeyDown (key->state, gdk_keyval_to_key (key->keyval), key->hardware_keycode);
				    
	s->SetCanFullScreen (false);
	
	return TRUE;
}

gboolean 
Surface::key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->FullScreenKeyHandled (key))
		return TRUE;

	s->SetCanFullScreen (true);

	// key events are only ever delivered to the toplevel
	s->toplevel->EmitKeyUp (key->state, gdk_keyval_to_key (key->keyval), key->hardware_keycode);
				    				    
	s->SetCanFullScreen (false);
	
	return TRUE;
}

void
Surface::widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	Surface *s = (Surface *) user_data;
	
	//printf ("Surface::size-allocate callback: current = %dx%d; new = %dx%d\n",
	//	s->width, s->height, allocation->width, allocation->height);
	
        if (s->width != allocation->width || s->height != allocation->height) {
                s->width = allocation->width;
                s->height = allocation->height;
		
		s->Realloc ();
		
		s->Emit (ResizeEvent);
	}
	
	// if x or y changed we need to recompute the presentation matrix
	// because the toplevel position depends on the allocation.
	if (s->toplevel)
		s->toplevel->UpdateBounds ();
}

void
Surface::widget_destroyed (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	// This is never called, why?
	printf ("------------------ WE ARE DESTROYED ---------------\n");
	if (s->widget_normal != NULL && s->widget_normal != widget) {
		// The fullscreen area have been destroyed.
		// If we are destroying it, widget_fullscreen is NULL.
		// If we're not, we have to call UpdateFullScreen to raise events,
		// change sizes, etc.
		if (s->widget_fullscreen != NULL) {
			s->UpdateFullScreen (false);
		}
	} else {
		s->widget = NULL;
	}
}



Surface *
surface_new (int width, int height)
{
	return new Surface (width, height);
}

void 
surface_destroy (Surface *s)
{
	s->unref ();
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
surface_get_widget (Surface *s)
{
	return s->GetWidget ();
}

TimeManager*
surface_get_time_manager (Surface* s)
{
	return s->GetTimeManager();
}

void
Surface::SetTrans (bool trans)
{
	transparent = trans;
	if (widget)
		gtk_widget_queue_draw (widget);
}

void
Surface::SetBackgroundColor (Color *color)
{
	//printf("YO");
	background_color = new Color (*color);
	if (widget)
		gtk_widget_queue_draw (widget);
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

void
runtime_init (guint32 flags)
{
	const char *env;
	
	if (inited)
		return;
	
	if (cairo_version () < CAIRO_VERSION_ENCODE(1,4,0)) {
		printf ("*** WARNING ***\n");
		printf ("*** Cairo versions < 1.4.0 should not be used for Moon.\n");
		printf ("*** Moon was configured to use Cairo version %d.%d.%d, but\n",
			CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
		printf ("*** is being run against version %s.\n", cairo_version_string ());
		printf ("*** Proceed at your own risk\n");
	}
	
	flags |= RUNTIME_INIT_SHOW_FPS;
	
	// Allow the user to override the flags via his/her environment
	if ((env = g_getenv ("MOONLIGHT_OVERRIDES"))) {
		const char *flag = env;
		const char *inptr;
		size_t n;
		uint i;
		
		while (*flag == ',')
			flag++;
		
		inptr = flag;
		
		while (*flag) {
			while (*inptr && *inptr != ',')
				inptr++;
			
			n = (inptr - flag);
			for (i = 0; i < G_N_ELEMENTS (overrides); i++) {
				if (n != strlen (overrides[i].override))
					continue;
				
				if (!strncmp (overrides[i].override, flag, n)) {
					if (!overrides[i].set)
						flags &= ~overrides[i].flag;
					else
						flags |= overrides[i].flag;
				}
			}
			
			while (*inptr == ',')
				inptr++;
			
			flag = inptr;
		}
	}
	
#if OBJECT_TRACKING
	if (EventObject::objects_created == EventObject::objects_destroyed) {
		printf ("Runtime created (no leaked objects so far).\n");
	} else {
		printf ("Runtime created. Object tracking summary:\n");
		printf ("\tObjects created: %i\n", EventObject::objects_created);
		printf ("\tObjects destroyed: %i\n", EventObject::objects_destroyed);
		printf ("\tDifference: %i\n", EventObject::objects_created - EventObject::objects_destroyed);
	}
#endif

	inited = true;
	
	if (!g_type_inited) {
		g_type_inited = true;
		g_type_init ();
	}
	
	moonlight_flags = flags;
	
	types_init ();
	namescope_init ();
	uielement_init ();
	collection_init ();
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

	/* lookup surface events */
	Type *t = Type::Find (Type::SURFACE);
	Surface::ResizeEvent = t->LookupEvent ("Resize");
	Surface::FullScreenChangeEvent = t->LookupEvent ("FullScreenChange");
	Surface::ErrorEvent = t->LookupEvent ("Error");
}

//
// These are the plugin-less versions of these methods
//
uint32_t
runtime_html_timer_timeout_add (int interval, GSourceFunc callback, gpointer data)
{
	return  g_timeout_add (interval, callback, data);
}

void 
runtime_html_timer_timeout_stop (uint32_t source_id)
{
	g_source_remove (source_id);
}

#if OBJECT_TRACKING
static int
IdComparer (gconstpointer base1, gconstpointer base2)
{
	int id1 = (*(EventObject **) base1)->id;
	int id2 = (*(EventObject **) base2)->id;

	int iddiff = id1 - id2;
	
	if (iddiff == 0)
		return 0;
	else if (iddiff < 0)
		return -1;
	else
		return 1;
}

static void
accumulate_last_n (gpointer key,
		   gpointer value,
		   gpointer user_data)
{
	GPtrArray *last_n = (GPtrArray*)user_data;

	g_ptr_array_insert_sorted (last_n, IdComparer, key);
}
#endif

void
runtime_shutdown (void)
{
	if (!inited)
		return;

	drain_unrefs ();
	
	animation_destroy ();
	text_destroy ();
	font_shutdown ();
	
	DependencyObject::Shutdown ();

#if OBJECT_TRACKING
	if (EventObject::objects_created == EventObject::objects_destroyed) {
		printf ("Runtime destroyed, no leaked objects.\n");
	} else {
		printf ("Runtime destroyed, with LEAKED EventObjects:\n");
		printf ("\tObjects created: %i\n", EventObject::objects_created);
		printf ("\tObjects destroyed: %i\n", EventObject::objects_destroyed);
		printf ("\tDifference: %i (%.1f%%)\n", EventObject::objects_created - EventObject::objects_destroyed, (100.0 * EventObject::objects_destroyed) / EventObject::objects_created);

		GPtrArray* last_n = g_ptr_array_new ();

		g_hash_table_foreach (EventObject::objects_alive, accumulate_last_n, last_n);

	 	uint counter = 10;
		counter = MIN(counter, last_n->len);
		if (counter) {
			printf ("\tOldest %d objects alive:\n", counter);
			for (uint i = 0; i < MIN (counter, last_n->len); i ++) {
				EventObject* obj = (EventObject *) last_n->pdata[i];
				printf ("\t\t%i = %s, refcount: %i\n", obj->id, obj->GetTypeName (), obj->GetRefCount ());
			}
		}

		g_ptr_array_free (last_n, true);
	}
#elif DEBUG
	if (EventObject::objects_created != EventObject::objects_destroyed) {
		printf ("Runtime destroyed, with %i LEAKED EventObjects.\n", EventObject::objects_created - EventObject::objects_destroyed);
	}
#endif

	Type::Shutdown ();
	inited = false;
	

}

gboolean
strcase_equal (gconstpointer  v1,
	       gconstpointer  v2)
{
	return !g_strcasecmp ((char*)v1, (char*)v2);
}

guint
strcase_hash (gconstpointer v)
{
	guint hash = 0;
	char *p = (char *) v;

	while (*p++)
		hash = (hash << 5) - (hash + g_ascii_tolower (*p));

	return hash;
}
