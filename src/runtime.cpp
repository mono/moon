/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * runtime.cpp: Core surface
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#define Visual _XxVisual
#define Region _XxRegion
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <cairo-xlib.h>
#undef Visual
#undef Region

#include "runtime.h"
#include "canvas.h"
#include "color.h"
#include "shape.h"
#include "transform.h"
#include "animation.h"
#include "downloader.h"
#include "frameworkelement.h"
#include "textblock.h"
#include "media.h"
#include "stylus.h"
#include "rect.h"
#include "panel.h"
#include "value.h"
#include "namescope.h"
#include "xaml.h"
#include "dirty.h"
#include "fullscreen.h"
#include "incomplete-support.h"
#include "drm.h"
#include "utils.h"
#include "window-gtk.h"
#include "timemanager.h"

#include "contentcontrol.h"
#include "usercontrol.h"
#include "deployment.h"
#include "grid.h"
#include "cbinding.h"
#include "tabnavigationwalker.h"

//#define DEBUG_INVALIDATE 1
//#define RENDER_INDIVIDUALLY 1
#define DEBUG_REFCNT 0

#define CAIRO_CLIP 0
#define TIME_CLIP 0
#define TIME_REDRAW 1

#define NO_EVENT_ID -1

bool Surface::main_thread_inited = false;
pthread_t Surface::main_thread = 0;

static bool inited = false;
static bool g_type_inited = false;
static GList* surface_list = NULL;
guint32 moonlight_flags = 0;
#if DEBUG || LOGGING
guint32 debug_flags_ex = 0;
guint32 debug_flags = 0;
#endif


struct env_options {
	const char *name;
	guint64 flag;
	bool set;
};

static struct env_options overrides[] = {
	// There's no "ms-codecs=yes" option to not allow enabling them from the command line.
	{ "ms-codecs=no",      RUNTIME_INIT_ENABLE_MS_CODECS,      false },
	{ "ffmpeg-codecs=no",  RUNTIME_INIT_DISABLE_FFMPEG_CODECS, true  },
	{ "ffmpeg-codecs=yes", RUNTIME_INIT_DISABLE_FFMPEG_CODECS, false },
	{ "timesource=manual", RUNTIME_INIT_MANUAL_TIMESOURCE,     true  },
	{ "timesource=system", RUNTIME_INIT_MANUAL_TIMESOURCE,     false },
	{ "expose=show",       RUNTIME_INIT_SHOW_EXPOSE,           true  },
	{ "expose=hide",       RUNTIME_INIT_SHOW_EXPOSE,           false },
	{ "clipping=show",     RUNTIME_INIT_SHOW_CLIPPING,         true  },
	{ "clipping=hide",     RUNTIME_INIT_SHOW_CLIPPING,         false },
	{ "bbox=show",         RUNTIME_INIT_SHOW_BOUNDING_BOXES,   true  },
	{ "bbox=hide",         RUNTIME_INIT_SHOW_BOUNDING_BOXES,   false },
	{ "textbox=show",      RUNTIME_INIT_SHOW_TEXTBOXES,        true  },
	{ "textbox=hide",      RUNTIME_INIT_SHOW_TEXTBOXES,        false },
	{ "fps=show",          RUNTIME_INIT_SHOW_FPS,              true  },
	{ "fps=hide",          RUNTIME_INIT_SHOW_FPS,              false },
	/* FIXME disabled for layout clipping */
	{ "render=ftb",        RUNTIME_INIT_RENDER_FRONT_TO_BACK,  false },
	{ "render=btf",        RUNTIME_INIT_RENDER_FRONT_TO_BACK,  true  },
	{ "cache=show",	       RUNTIME_INIT_SHOW_CACHE_SIZE,       true  },
	{ "cache=hide",        RUNTIME_INIT_SHOW_CACHE_SIZE,       false },
	{ "converter=default", RUNTIME_INIT_FFMPEG_YUV_CONVERTER,  false },
	{ "converter=ffmpeg",  RUNTIME_INIT_FFMPEG_YUV_CONVERTER,  true  },
	{ "shapecache=yes",    RUNTIME_INIT_USE_SHAPE_CACHE,	   true  },
	{ "shapecache=no",     RUNTIME_INIT_USE_SHAPE_CACHE,	   false },
	{ "updatepos=yes",     RUNTIME_INIT_USE_UPDATE_POSITION,   true  },
	{ "updatepos=no",      RUNTIME_INIT_USE_UPDATE_POSITION,   false },
	{ "windowless=yes",    RUNTIME_INIT_ALLOW_WINDOWLESS,      true  },
	{ "windowless=no",     RUNTIME_INIT_ALLOW_WINDOWLESS,      false },
	{ "audio=alsa",        RUNTIME_INIT_AUDIO_ALSA,            true  },
	{ "audio=alsa-mmap",   RUNTIME_INIT_AUDIO_ALSA_MMAP,       true  },
	{ "audio=alsa-rw",     RUNTIME_INIT_AUDIO_ALSA_RW,         true  },
	{ "audio=pulseaudio",  RUNTIME_INIT_AUDIO_PULSE,           true  },
	{ "idlehint=yes",      RUNTIME_INIT_USE_IDLE_HINT,         false },
	{ "idlehint=no",       RUNTIME_INIT_USE_IDLE_HINT,         true  },
	/* default to the image backend until cairo is actually faster that way */
	{ "backend=xlib",      RUNTIME_INIT_USE_BACKEND_XLIB,      false },
	{ "backend=image",     RUNTIME_INIT_USE_BACKEND_XLIB,      true  },
	{ "keepmedia=no",      RUNTIME_INIT_KEEP_MEDIA,            false },
	{ "keepmedia=yes",     RUNTIME_INIT_KEEP_MEDIA,            true  },
	{ "allimages=no",      RUNTIME_INIT_ALL_IMAGE_FORMATS,     false },
	{ "allimages=yes",     RUNTIME_INIT_ALL_IMAGE_FORMATS,     true  },
	{ "emulatekeycodes=yes", RUNTIME_INIT_EMULATE_KEYCODES,	   true  },
	{ "emulatekeycodes=no",  RUNTIME_INIT_EMULATE_KEYCODES,	   false },
	{ "curlbridge=yes",     RUNTIME_INIT_CURL_BRIDGE,          true  },
	{ "curlbridge=no",     RUNTIME_INIT_CURL_BRIDGE,          false },

};

#define RUNTIME_INIT_DESKTOP (RUNTIME_INIT_PANGO_TEXT_LAYOUT | RUNTIME_INIT_RENDER_FRONT_TO_BACK | RUNTIME_INIT_USE_UPDATE_POSITION | RUNTIME_INIT_USE_SHAPE_CACHE | RUNTIME_INIT_USE_IDLE_HINT | RUNTIME_INIT_USE_BACKEND_XLIB | RUNTIME_INIT_ALL_IMAGE_FORMATS | RUNTIME_INIT_OUT_OF_BROWSER | RUNTIME_INIT_DESKTOP_EXTENSIONS)
#define RUNTIME_INIT_BROWSER (RUNTIME_INIT_RENDER_FRONT_TO_BACK | RUNTIME_INIT_USE_UPDATE_POSITION | RUNTIME_INIT_USE_SHAPE_CACHE | RUNTIME_INIT_ALLOW_WINDOWLESS | RUNTIME_INIT_USE_IDLE_HINT | RUNTIME_INIT_USE_BACKEND_XLIB | RUNTIME_INIT_ENABLE_MS_CODECS | RUNTIME_INIT_CREATE_ROOT_DOMAIN)

#if DEBUG || LOGGING
static struct env_options debugs[] = {
	{ "alsa",              RUNTIME_DEBUG_ALSA,             true },
	{ "audio",             RUNTIME_DEBUG_AUDIO,            true },
	{ "pulse",             RUNTIME_DEBUG_PULSE,            true },
	{ "httpstreaming",     RUNTIME_DEBUG_HTTPSTREAMING,    true },
	{ "markers",           RUNTIME_DEBUG_MARKERS,          true },
	{ "mms",               RUNTIME_DEBUG_MMS,              true },
	{ "mediaplayer",       RUNTIME_DEBUG_MEDIAPLAYER,      true },
	{ "pipeline",          RUNTIME_DEBUG_PIPELINE,         true },
	{ "pipeline-error",    RUNTIME_DEBUG_PIPELINE_ERROR,   true },
	{ "framereaderloop",   RUNTIME_DEBUG_FRAMEREADERLOOP,  true },
	{ "ui",                RUNTIME_DEBUG_UI,               true },
	{ "ffmpeg",            RUNTIME_DEBUG_FFMPEG,           true },
	{ "codecs",            RUNTIME_DEBUG_CODECS,           true },
	{ "dependencyobject",  RUNTIME_DEBUG_DP,               true },
	{ "downloader",        RUNTIME_DEBUG_DOWNLOADER,       true },
	{ "font",              RUNTIME_DEBUG_FONT,             true },
	{ "layout",            RUNTIME_DEBUG_LAYOUT,           true },
	{ "media",             RUNTIME_DEBUG_MEDIA,            true },
	{ "mediaelement",      RUNTIME_DEBUG_MEDIAELEMENT,     true },
	{ "msi",	       RUNTIME_DEBUG_MSI,	       true },
	{ "buffering",         RUNTIME_DEBUG_BUFFERING,        true },
	{ "asf",               RUNTIME_DEBUG_ASF,              true },
	{ "playlist",          RUNTIME_DEBUG_PLAYLIST,         true },
	{ "text",              RUNTIME_DEBUG_TEXT,             true },
	{ "xaml",              RUNTIME_DEBUG_XAML,             true },
	{ "deployment",        RUNTIME_DEBUG_DEPLOYMENT,       true },
	{ "mp3",               RUNTIME_DEBUG_MP3,              true },
	{ "asf",               RUNTIME_DEBUG_ASF,              true },
	{ "value",             RUNTIME_DEBUG_VALUE,            true },
	{ NULL, 0, false }
};

static struct env_options debug_extras[] = {
	{ "alsa-ex",           RUNTIME_DEBUG_ALSA_EX,          true },
	{ "audio-ex",          RUNTIME_DEBUG_AUDIO_EX,         true },
	{ "pulse-ex",          RUNTIME_DEBUG_PULSE_EX,         true },
	{ "markers-ex",        RUNTIME_DEBUG_MARKERS_EX,       true },
	{ "mediaplayer-ex",    RUNTIME_DEBUG_MEDIAPLAYER_EX,   true },
	{ "mediaelement-ex",   RUNTIME_DEBUG_MEDIAELEMENT_EX,  true },
	{ "playlist-ex",       RUNTIME_DEBUG_PLAYLIST_EX,      true },
	{ "pipeline-ex",       RUNTIME_DEBUG_PIPELINE_EX,      true },
	{ NULL, 0, false }
};
#endif



#define RENDER_EXPOSE (moonlight_flags & RUNTIME_INIT_SHOW_EXPOSE)

static bool
running_on_nvidia ()
{
	int event, error, opcode;

	Display *display = XOpenDisplay (NULL);
	bool result = XQueryExtension (display, "NV-GLX", &opcode, &event, &error);
	XCloseDisplay (display);

	return result;
}

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

GList *
runtime_get_surface_list (void)
{
	if (!Surface::InMainThread ()) {
		g_warning ("This method can be only called from the main thread!\n");
		return NULL;
	}
	
	return surface_list;
}

static cairo_t *
runtime_cairo_create (GdkWindow *drawable, GdkVisual *visual, bool native)
{
	int width, height;
	cairo_surface_t *surface;
	cairo_t *cr;

	gdk_drawable_get_size (drawable, &width, &height);

	if (native)
		surface = cairo_xlib_surface_create (gdk_x11_drawable_get_xdisplay (drawable),
						     gdk_x11_drawable_get_xid (drawable),
						     GDK_VISUAL_XVISUAL (visual),
						     width, height);
	else 
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

	cr = cairo_create (surface);
	cairo_surface_destroy (surface);
			    
	return cr;
}

static gboolean
flags_can_be_modifed (void)
{
	if (g_list_length (surface_list) != 0) {
		g_warning ("Flags can be dynamically modified only when there are no surfaces created!");
		return FALSE;
	} else if (inited == FALSE) {
		g_warning ("Runtime has not been initialized yet, your flags will be overriden!");
		return FALSE;
	} else 
		return TRUE;
}

void
runtime_flags_set_manual_timesource (gboolean flag)
{
	if (flags_can_be_modifed ())
		moonlight_flags |= RUNTIME_INIT_MANUAL_TIMESOURCE;
}

void
runtime_flags_set_use_shapecache (gboolean flag)
{
	if (flags_can_be_modifed ())
		moonlight_flags |= RUNTIME_INIT_USE_SHAPE_CACHE;
}

void
runtime_flags_set_show_fps (gboolean flag)
{
	if (flags_can_be_modifed ())
		moonlight_flags |= RUNTIME_INIT_SHOW_FPS;
}

/* FIXME More flag setters here */

Surface::Surface (MoonWindow *window)
{
	SetObjectType (Type::SURFACE);

	GetDeployment ()->SetSurface (this);

	main_thread = pthread_self ();
	main_thread_inited = true;
	
	zombie = false;
	downloader_context = NULL;
	downloaders = NULL;
	relaxed_media_mode = false;
	background_color = NULL;
	cursor = MouseCursorDefault;
	mouse_event = NULL;
	
	background_color = new Color (1, 1, 1, 0);

	time_manager = new TimeManager ();
	time_manager->Start ();
	ticked_after_attach = false;

	fullscreen_window = NULL;
	normal_window = active_window = window;
	if (active_window->IsFullScreen())
		g_warning ("Surfaces cannot be initialized with fullscreen windows.");
	window->SetSurface (this);
	
	layers = new HitTestCollection ();
	toplevel = NULL;
	input_list = new List ();
	captured = NULL;
	
	focused_element = NULL;
	focus_changed_events = new Queue ();

	full_screen = false;
	first_user_initiated_event = false;
	user_initiated_event = false;
	user_initiated_monotonic_counter = 0;
	
	zoom_factor = 1.0;
	
	incomplete_support_message = NULL;
	drm_message = NULL;
	full_screen_message = NULL;
	source_location = NULL;

	fps_report = fps_report_default;
	fps_data = NULL;

	frames = 0;
	fps_nframes = 0;
	fps_start = 0;

	cache_report = cache_report_default;
	cache_data = NULL;

	cache_size_in_bytes = 0;
	cache_size_ticker = 0;
	cache_size_multiplier = -1;

	expose_handoff = NULL;
	expose_handoff_data = NULL;
	expose_handoff_last_timespan = G_MAXINT64; 

	emittingMouseEvent = false;
	pendingCapture = NULL;
	pendingReleaseCapture = false;

#ifdef DEBUG
	debug_selected_element = NULL;
#endif

	up_dirty = new DirtyLists (true);
	down_dirty = new DirtyLists (false);
	
	surface_list = g_list_append (surface_list, this);
}

Surface::~Surface ()
{
	time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
		
	if (toplevel) {
		toplevel->SetSurface (NULL);
		toplevel->unref ();
	}
	
#if DEBUG
	if (debug_selected_element) {
		debug_selected_element->unref ();
		debug_selected_element = NULL;
	}
#endif
	
	HideFullScreenMessage ();
	
	delete input_list;
	
	g_free (source_location);

	if (fullscreen_window)
		delete fullscreen_window;
	
	if (normal_window)
		delete normal_window;
	
	delete background_color;
	
	time_manager->unref ();
	
	delete up_dirty;
	delete down_dirty;
	
	delete downloaders;
	layers->unref ();
	
	surface_list = g_list_remove (surface_list, this);
}

void
Surface::Dispose ()
{
	if (toplevel) {
		toplevel->SetSurface (NULL);
		toplevel->Dispose ();
	}
	
	EventObject::Dispose ();
}

void
Surface::Zombify ()
{
	time_manager->Shutdown ();
	DetachDownloaders ();
	zombie = true;
}

MoonWindow *
Surface::DetachWindow ()
{
	MoonWindow *result;
	
	/* We only detach the normal window. TODO: Testing requires to see what happens if DetachWindow is called (changing plugin.source) in fullscreen mode. */
	if (active_window == normal_window)
		active_window = NULL;
	result = normal_window;
	normal_window = NULL;
	
	return result;
}

void
Surface::SetCursor (MouseCursor new_cursor)
{
	if (new_cursor != cursor) {
		cursor = new_cursor;

		active_window->SetCursor (cursor);
	}
}

void
Surface::AutoFocus ()
{
	GenerateFocusChangeEvents ();
}

void
Surface::AutoFocusAsync (EventObject *sender)
{
	((Surface *)sender)->AutoFocus ();
}

void
Surface::Attach (UIElement *element)
{
	bool first = false;

#if DEBUG
	// Attach must be called with NULL to clear out the old
	// element before attaching element, otherwise the new element
	// might get loaded with data from the old element (when
	// parsing xaml ticks will get added to the timemanager of the
	// surface, if the old element isn't gone when the new element
	// is parsed, the ticks will be added to the old timemanager).
	if (toplevel != NULL && element != NULL)
		g_warning ("Surface::Attach (NULL) should be called to clear out the old toplevel before adding a new element.");
#endif

#if SANITY
	if (element != NULL && element->GetDeployment () != GetDeployment ()) 
		g_warning ("Surface::Attach (%p): trying to attach an object created on the deployment %p on a surface whose deployment is %p\n", element, element->GetDeployment (), GetDeployment ());
	if (GetDeployment () != Deployment::GetCurrent ())
		g_warning ("Surface::Attach (%p): current deployment is %p, surface deployment is %p\n", element, GetDeployment (), Deployment::GetCurrent ());
#endif


	if (toplevel) {
		toplevel->RemoveHandler (UIElement::LoadedEvent, toplevel_loaded, this);
		DetachLayer (toplevel);
		time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
		time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
		time_manager->Stop ();
		int maxframerate = time_manager->GetMaximumRefreshRate ();
		toplevel->unref ();
		time_manager->unref ();
		time_manager = new TimeManager ();
		time_manager->AddHandler (TimeManager::RenderEvent, render_cb, this);
		time_manager->AddHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
		time_manager->SetMaximumRefreshRate (maxframerate);
		time_manager->NeedRedraw ();
		time_manager->Start ();
	} else 
		first = true;

	if (!element) {
		DetachDownloaders ();

		if (first)
			active_window->EnableEvents (first);

		if (active_window)
			active_window->Invalidate();

		toplevel = NULL;
		return;
	}

	if (!element->Is (Type::UIELEMENT)) {
		printf ("Surface::Attach Unsupported toplevel %s\n", element->GetTypeName ());
		return;
	}

	UIElement *new_toplevel = element;
	new_toplevel->ref ();

	// make sure we have a namescope at the toplevel so that names
	// can be registered/resolved properly.
	if (NameScope::GetNameScope (new_toplevel) == NULL) {
		NameScope::SetNameScope (new_toplevel, new NameScope());
	}

	// First time we connect the surface, start responding to events
	if (first && active_window)
		active_window->EnableEvents (first);

	if (zombie)
		return;

	toplevel = new_toplevel;

	this->ref ();
	toplevel->AddHandler (UIElement::LoadedEvent, toplevel_loaded, this, (GDestroyNotify)event_object_unref);

	AttachLayer (toplevel);

	ticked_after_attach = false;
	time_manager->RemoveTickCall (tick_after_attach_reached, this);
	time_manager->AddTickCall (tick_after_attach_reached, this);

	const char *runtime_version = GetDeployment()->GetRuntimeVersion ();
	
	if (first && runtime_version
	    && (!strncmp ("3.", runtime_version, 2)
		|| !strncmp ("4.", runtime_version, 2))) {
		// we're running a SL app, let's warn the user about
		// moonlight's incomplete support.
		ShowIncompleteSilverlightSupportMessage ();
	}
}

void
Surface::tick_after_attach_reached (EventObject *data)
{
	Surface *surface = (Surface*)data;

	surface->ticked_after_attach = true;
	surface->Emit (Surface::LoadEvent);
}

void
Surface::toplevel_loaded (EventObject *sender, EventArgs *args, gpointer closure)
{
	((Surface*)closure)->ToplevelLoaded ((UIElement*)sender);
}

void
Surface::ToplevelLoaded (UIElement *element)
{
	if (element == toplevel) {
		toplevel->RemoveHandler (UIElement::LoadedEvent, toplevel_loaded, this);

		// FIXME: If the element is supposed to be focused, FocusElement (element)
		// should be used. I think this is unnecessary anyway.
		//if (active_window && active_window->HasFocus())
		//	element->EmitGotFocus ();
	
		//
		// If the did not get a size specified
		//
		if (normal_window && normal_window->GetWidth() == 0 && normal_window->GetHeight() == 0 && toplevel) {
			/*
			 * this should only be hit in the nonplugin case ans is
			 * simply here to give a reasonable default size
			 */
			Value *vh, *vw;
			vw = toplevel->GetValue (FrameworkElement::WidthProperty);
			vh = toplevel->GetValue (FrameworkElement::HeightProperty);
			if (vh || vw)
				normal_window->Resize (MAX (vw ? (int)vw->AsDouble () : 0, 0),
						       MAX (vh ? (int)vh->AsDouble () : 0, 0));
		}

		Emit (ResizeEvent);

		element->UpdateTotalRenderVisibility ();
		element->UpdateTotalHitTestVisibility ();
		element->FullInvalidate (true);

		// we call this two here so that the layout pass proceeds when
		// we next process the dirty list.
		element->InvalidateMeasure ();
	}
}

void
Surface::AttachLayer (UIElement *layer)
{
	if (layer == toplevel)
		layers->Insert (0, Value(layer));
	else
		layers->Add (Value (layer));

	layer->SetSurface (this);
	layer->FullInvalidate (true);
	layer->InvalidateMeasure ();
	layer->WalkTreeForLoadedHandlers (NULL, false, false);
	Deployment::GetCurrent()->PostLoaded ();
}

void
Surface::DetachLayer (UIElement *layer)
{
	layers->Remove (Value (layer));
	layer->SetSurface (NULL);
	if (active_window)
		Invalidate (layer->GetBounds ());
}

void
Surface::Invalidate (Rect r)
{
	active_window->Invalidate (r);
}

void
Surface::ProcessUpdates ()
{
	active_window->ProcessUpdates();
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
	for (int i = 0; i < layers->GetCount (); i++) {
		UIElement *layer = layers->GetValueAt (i)->AsUIElement ();
		layer->Paint (ctx, region, NULL);
	}

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
		cairo_rectangle (ctx, bounds.x, bounds.y, bounds.width, bounds.height);
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
// toplevel element, if you want that, you must do that yourself
//
void
Surface::Resize (int width, int height)
{
	if (width == normal_window->GetWidth()
	    && height == normal_window->GetHeight())
		return;

	normal_window->Resize (width, height);
}

void
Surface::EmitSourceDownloadComplete ()
{
	Emit (SourceDownloadCompleteEvent, NULL);
}

void
Surface::EmitSourceDownloadProgressChanged (DownloadProgressEventArgs *args)
{
	Emit (SourceDownloadProgressChangedEvent, args);
}

void
Surface::EmitError (ErrorEventArgs *args)
{
	Emit (ErrorEvent, args);
}

void
Surface::EmitError (int number, int code, const char *message)
{
	EmitError (new ErrorEventArgs ((ErrorEventArgsType)number,
				       MoonError (MoonError::EXCEPTION, code, message)));
}

void
Surface::Realloc ()
{
	for (int i = 0; i < layers->GetCount (); i++) {
		UIElement *layer = layers->GetValueAt (i)->AsUIElement ();

		layer->InvalidateMeasure ();
		//layer->UpdateBounds();
	}
}

void
Surface::SetFullScreen (bool value)
{
	if (value && !IsUserInitiatedEvent ()) {
		g_warning ("You're not allowed to switch to fullscreen from where you're doing it.");
		return;
	}
	
	UpdateFullScreen (value);
}

void
Surface::SetZoomFactor (double value)
{
	// FIXME: implement surface zooming
	zoom_factor = value;
	
	Emit (ZoomedEvent, new EventArgs ());
}

void
Surface::SetUserInitiatedEvent (bool value)
{
	GenerateFocusChangeEvents ();
	first_user_initiated_event = first_user_initiated_event | value;
	user_initiated_event = value;
	user_initiated_monotonic_counter++;
}

bool
Surface::IsTopLevel (UIElement* top)
{
	if (top == NULL)
		return false;

	bool ret = top == full_screen_message;

	for (int i = 0; i < layers->GetCount () && !ret; i++)
		ret = layers->GetValueAt (i)->AsUIElement () == top;

	return ret;
}

void
Surface::ShowIncompleteSilverlightSupportMessage ()
{
	g_return_if_fail (incomplete_support_message == NULL);

	Type::Kind dummy;
	XamlLoader *loader = new XamlLoader (NULL, INCOMPLETE_SUPPORT_MESSAGE, this);
	DependencyObject* message = loader->CreateDependencyObjectFromString (INCOMPLETE_SUPPORT_MESSAGE, false, &dummy);
	delete loader;

	if (!message) {
		g_warning ("Unable to create incomplete support message.\n");
		return;
	}
	
	if (!message->Is (Type::FRAMEWORKELEMENT)) {
		g_warning ("Unable to create incomplete support message, got a %s, expected at least a FrameworkElement.\n", message->GetTypeName ());
		message->unref ();
		return;
	}

	incomplete_support_message = (Panel *) message;
	AttachLayer (incomplete_support_message);

	DependencyObject* message_object = incomplete_support_message->FindName ("message");
	TextBlock* message_block = (message_object != NULL && message_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) message_object : NULL;

	
	char *message_text = g_strdup_printf ("You are running a Silverlight %c application.  You may experience incompatabilities as Moonlight does not have full support for this runtime yet.", GetDeployment()->GetRuntimeVersion()[0]);
	message_block->SetValue (TextBlock::TextProperty, message_text);
	g_free (message_text);

	DependencyObject* storyboard_object = incomplete_support_message->FindName ("FadeOut");
	Storyboard* storyboard = (storyboard_object != NULL && storyboard_object->Is (Type::STORYBOARD)) ? (Storyboard*) storyboard_object : NULL;

	storyboard->AddHandler (Timeline::CompletedEvent, HideIncompleteSilverlightSupportMessageCallback, this);

	// make the message take up the full width of the window
	message->SetValue (FrameworkElement::WidthProperty, Value (active_window->GetWidth()));
}

void
Surface::HideIncompleteSilverlightSupportMessageCallback (EventObject *sender, EventArgs *args, gpointer closure)
{
	((Surface*)closure)->HideIncompleteSilverlightSupportMessage ();
}

void 
Surface::HideIncompleteSilverlightSupportMessage ()
{
	if (incomplete_support_message) {
	        DetachLayer (incomplete_support_message);
		incomplete_support_message->unref ();
		incomplete_support_message = NULL;
		// Since we're removing a layer the dirty list might get confused
		active_window->Invalidate ();
	}
}

void
Surface::ShowDrmMessage ()
{
	if (drm_message != NULL) {
		return; /* We're already showing it */
	}

	Type::Kind dummy;
	XamlLoader *loader = new XamlLoader (NULL, DRM_MESSAGE, this);
	DependencyObject* message = loader->CreateDependencyObjectFromString (DRM_MESSAGE, false, &dummy);
	delete loader;

	if (!message) {
		g_warning ("Unable to create drm message.\n");
		return;
	}
	
	if (!message->Is (Type::PANEL)) {
		g_warning ("Unable to create drm message, got a %s, expected at least a FrameworkElement.\n", message->GetTypeName ());
		message->unref ();
		return;
	}

	drm_message = (Panel *) message;
	AttachLayer (drm_message);

	/* Hide the message when clicked */
	drm_message->AddHandler (UIElement::MouseLeftButtonDownEvent, HideDrmMessageCallback, this);

	// make the message take up the full width of the window
	drm_message->SetValue (FrameworkElement::WidthProperty, Value (active_window->GetWidth()));
}

void
Surface::HideDrmMessageCallback (EventObject *sender, EventArgs *args, gpointer closure)
{
	((Surface *) closure)->HideDrmMessage ();
}

void 
Surface::HideDrmMessage ()
{
	if (drm_message) {
		if (focused_element == drm_message)
			focused_element = NULL;
		DetachLayer (drm_message);
		drm_message->unref ();
		drm_message = NULL;
		// Since we're removing a layer the dirty list might get confused
		active_window->Invalidate ();
	}
}

void
Surface::ShowFullScreenMessage ()
{
	g_return_if_fail (full_screen_message == NULL);
	
	Type::Kind dummy;
	XamlLoader *loader = new XamlLoader (NULL, FULLSCREEN_MESSAGE, this);
	DependencyObject* message = loader->CreateDependencyObjectFromString (FULLSCREEN_MESSAGE, false, &dummy);
	delete loader;
	
	if (!message) {
		g_warning ("Unable to create fullscreen message.\n");
		return;
	}
	
	full_screen_message = (Panel *) message;
	AttachLayer (full_screen_message);
	
	DependencyObject* message_object = full_screen_message->FindName ("message");
	DependencyObject* url_object = full_screen_message->FindName ("url");
	TextBlock* message_block = (message_object != NULL && message_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) message_object : NULL;
	TextBlock* url_block = (url_object != NULL && url_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) url_object : NULL;
	
	// Set the url in the box
	if (url_block != NULL)  {
		char *url = NULL;
		
		if (source_location) {
			if (g_str_has_prefix (source_location, "http://")) {
				const char *path = strchr (source_location + 7, '/');
				
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
		}
		
		url_block->SetValue (TextBlock::TextProperty, url ? url : (char *) "file://");
		g_free (url);
	}
	
	DependencyObject* storyboard_object = full_screen_message->FindName ("FadeOut");
	Storyboard* storyboard = (storyboard_object != NULL && storyboard_object->Is (Type::STORYBOARD)) ? (Storyboard*) storyboard_object : NULL;

	storyboard->AddHandler (Timeline::CompletedEvent, HideFullScreenMessageCallback, this);
}

void
Surface::HideFullScreenMessageCallback (EventObject *sender, EventArgs *args, gpointer closure)
{
	((Surface*)closure)->HideFullScreenMessage ();
}

void 
Surface::HideFullScreenMessage ()
{
	if (full_screen_message) {
	        DetachLayer (full_screen_message);
		full_screen_message->unref ();
		full_screen_message = NULL;
		// Since we're removing a layer the dirty list might get confused
		active_window->Invalidate ();
	}
}

const char* 
Surface::GetSourceLocation ()
{
	return source_location;
}

void
Surface::SetSourceLocation (const char* location)
{
	g_free (source_location);
	source_location = g_strdup (location);
}

void
Surface::UpdateFullScreen (bool value)
{
	if (value == full_screen)
		return;

	if (value) {
		fullscreen_window = new MoonWindowGtk (true, -1, -1, normal_window, this);

		active_window = fullscreen_window;
		
		ShowFullScreenMessage ();

		fullscreen_window->EnableEvents (false);
	} else {
		active_window = normal_window;

		HideFullScreenMessage ();

		delete fullscreen_window;
		fullscreen_window = NULL;
	}

	full_screen = value;
	
	Realloc ();

	time_manager->GetSource()->Stop();
	Emit (FullScreenChangeEvent);

	if (!value)
		Emit (ResizeEvent);
	time_manager->GetSource()->Start();
}

void
Surface::render_cb (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Surface *s = (Surface *) closure;
	gint64 now;
	bool dirty = false;

	if (s->active_window == NULL)
		return; /* no active window to render to */

	GDK_THREADS_ENTER ();
	if (s->zombie) {
		s->up_dirty->Clear (true);
		s->down_dirty->Clear (true);
	} else {
		dirty = s->ProcessDirtyElements ();
	}

	if (s->expose_handoff) {
		TimeSpan time = s->GetTimeManager ()->GetCurrentTime ();
		if (time != s->expose_handoff_last_timespan) {
			s->expose_handoff (s, time , s->expose_handoff_data);
			s->expose_handoff_last_timespan = time;
		}
	}

	GDK_THREADS_LEAVE ();

	if ((moonlight_flags & RUNTIME_INIT_SHOW_FPS) && s->fps_start == 0)
		s->fps_start = get_now ();
	
	if (dirty) {
		s->ProcessUpdates ();
	}

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

	s->HandleMouseEvent (UIElement::MouseMoveEvent, true, true, false, s->mouse_event_state, s->mouse_event_x, s->mouse_event_y);
	s->UpdateCursorFromInputList ()
#endif
}

void
Surface::HandleUIWindowAvailable ()
{
	time_manager->AddHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->AddHandler (TimeManager::UpdateInputEvent, update_input_cb, this);

	time_manager->NeedRedraw ();
}

void
Surface::HandleUIWindowUnavailable ()
{
	time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
}

void
Surface::PaintToDrawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y, bool transparent, bool clear_transparent)
{
	frames++;

	LOG_UI ("Surface::PaintToDrawable (%p, %p, (%d,%d %d,%d), %d, %d, %d, %d)\n",
		drawable, visual, event->area.x, event->area.y, event->area.width, event->area.height,
		off_x, off_y, transparent, clear_transparent);
	
	if (event->area.x > (off_x + active_window->GetWidth()) || event->area.y > (off_y + active_window->GetHeight()))
		return;

	SetCurrentDeployment ();

#if TIME_REDRAW
	STARTTIMER (expose, "redraw");
#endif
	if (cache_size_multiplier == -1)
		cache_size_multiplier = gdk_drawable_get_depth (drawable) / 8 + 1;

#ifdef DEBUG_INVALIDATE
	printf ("Got a request to repaint at %d %d %d %d\n", event->area.x, event->area.y, event->area.width, event->area.height);
#endif
	cairo_t *ctx = runtime_cairo_create (drawable, visual, moonlight_flags & RUNTIME_INIT_USE_BACKEND_XLIB);
	Region *region = new Region (event->region);

	region->Offset (-off_x, -off_y);
	cairo_surface_set_device_offset (cairo_get_target (ctx),
					 off_x - event->area.x, 
					 off_y - event->area.y);
	region->Draw (ctx);
	//
	// These are temporary while we change this to paint at the offset position
	// instead of using the old approach of modifying the topmost UIElement (a no-no),
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
	cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);

	if (transparent) {
		if (clear_transparent) {
			cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
			cairo_fill_preserve (ctx);
			cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
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

	cairo_fill_preserve (ctx);
	cairo_clip (ctx);

	cairo_save (ctx);
	Paint (ctx, region);
	cairo_restore (ctx);

	if (RENDER_EXPOSE) {
		cairo_new_path (ctx);
		region->Draw (ctx);
		cairo_set_line_width (ctx, 2.0);
		cairo_set_source_rgb (ctx, (double)(abs (frames) % 2), (double)((abs (frames) + 1) % 2), (double)((abs (frames) / 3) % 2));
		cairo_stroke (ctx);
	}

	if (!(moonlight_flags & RUNTIME_INIT_USE_BACKEND_XLIB)) {
		cairo_surface_flush (cairo_get_target (ctx));
		cairo_t *native = runtime_cairo_create (drawable, visual, true);

		cairo_surface_set_device_offset (cairo_get_target (native),
						 0, 0);
		cairo_surface_set_device_offset (cairo_get_target (ctx),
						 0, 0);

		cairo_set_source_surface (native, cairo_get_target (ctx),
					  0, 0);

		region->Offset (off_x, off_y);
		region->Offset (-event->area.x, -event->area.y);
		region->Draw (native);

		cairo_fill (native);
		cairo_destroy (native);
	}
	cairo_destroy (ctx);

	delete region;


#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

}

/* for emitting focus changed events */
class FocusChangedNode : public List::Node {
public:
	UIElement *lost_focus;
	UIElement *got_focus;
	
	FocusChangedNode (UIElement *lost_focus, UIElement *got_focus);
	virtual ~FocusChangedNode ();
};

FocusChangedNode::FocusChangedNode (UIElement *lost_focus, UIElement *got_focus)
{
	this->lost_focus = lost_focus;
	this->got_focus = got_focus;
	
	if (lost_focus)
		lost_focus->ref ();
	if (got_focus)
		got_focus->ref ();
}

FocusChangedNode::~FocusChangedNode ()
{
	if (lost_focus)
		lost_focus->unref ();
	if (got_focus)
		got_focus->unref ();
}

RenderNode::RenderNode (UIElement *el,
			Region *region,
			bool render_element,
			RenderFunc pre,
			RenderFunc post)

{
	uielement = el;
	uielement->ref();
	this->region = region ? region : new Region ();
	this->render_element = render_element;
	this->pre_render = pre;
	this->post_render = post;
}

void
RenderNode::Render (cairo_t *ctx)
{
	bool front_to_back = uielement->UseBackToFront ();

	if (pre_render)
		pre_render (ctx, uielement, region, front_to_back);

	if (render_element)
		uielement->Render (ctx, region);
	
	if (post_render)
		post_render (ctx, uielement, region, front_to_back);
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

void
Surface::PerformCapture (UIElement *capture)
{
	// "Capturing" the mouse pointer at an element forces us to
	// use the path up the hierarchy from that element to the root
	// as the input list, regardless of where the pointer actually
	// is.

	captured = capture;
	List *new_input_list = new List();
	while (capture) {
		new_input_list->Append (new UIElementNode (capture));
		capture = capture->GetVisualParent();
	}

	delete input_list;
	input_list = new_input_list;
	pendingCapture = NULL;
}

void
Surface::PerformReleaseCapture ()
{
	// These need to be set before calling HandleMouseEvent as
	// "captured" determines the input_list calculation, and
	// "pendingReleaseCapture", when set, causes an infinite
	// recursive loop.
	UIElement *old_captured = captured;
	captured = NULL;
	pendingReleaseCapture = false;

	old_captured->EmitLostMouseCapture ();

	// this causes any new elements we're over to be Enter'ed.  MS
	// doesn't Leave the element that had the mouse captured,
	// though.
	HandleMouseEvent (NO_EVENT_ID, false, true, false, mouse_event);
}

void
Surface::ReleaseMouseCapture (UIElement *capture)
{
	// Mouse capture is only released when the element owning the capture
	// requests it
	if (capture != captured && capture != pendingCapture)
		return;

	if (emittingMouseEvent)
		pendingReleaseCapture = true;
	else
		PerformReleaseCapture ();
}

bool
Surface::SetMouseCapture (UIElement *capture)
{
	if (captured || pendingCapture)
		return capture == captured || capture == pendingCapture;

	if (!emittingMouseEvent)
		return false;

	pendingCapture = capture;
	return true;
}

EventArgs*
Surface::CreateArgsForEvent (int event_id, GdkEvent *event)
{
	if (event_id ==UIElement::InvalidatedEvent
	    || event_id ==UIElement::GotFocusEvent
	    || event_id ==UIElement::LostFocusEvent)
		return new RoutedEventArgs ();
	else if (event_id == UIElement::MouseLeaveEvent
		 || event_id ==UIElement::MouseMoveEvent
		 || event_id ==UIElement::MouseEnterEvent)
		return new MouseEventArgs(event);
	else if (event_id ==UIElement::MouseLeftButtonMultiClickEvent
		 || event_id ==UIElement::MouseLeftButtonDownEvent
		 || event_id ==UIElement::MouseLeftButtonUpEvent
		 || event_id ==UIElement::MouseRightButtonDownEvent
		 || event_id ==UIElement::MouseRightButtonUpEvent)
		return new MouseButtonEventArgs(event);
	else if (event_id == UIElement::MouseWheelEvent)
		return new MouseWheelEventArgs(event);
	else if (event_id == UIElement::KeyDownEvent
		 || event_id == UIElement::KeyUpEvent)
		return new KeyEventArgs((GdkEventKey*)event);
	else {
		g_warning ("Unknown event id %d\n", event_id);
		return new EventArgs();
	}
}

bool
Surface::EmitEventOnList (int event_id, List *element_list, GdkEvent *event, int end_idx)
{
	bool handled = false;

	int idx;
	UIElementNode *node;

	if (element_list->IsEmpty() || end_idx == 0)
		return handled;

	if (end_idx == -1)
		end_idx = element_list->Length();

	EmitContext** emit_ctxs = g_new (EmitContext*, end_idx + 1);
	for (node = (UIElementNode*)element_list->First(), idx = 0; node && idx < end_idx; node = (UIElementNode*)node->next, idx++) {
		emit_ctxs[idx] = node->uielement->StartEmit (event_id);
	}

	EventArgs *args = CreateArgsForEvent(event_id, event);
	bool args_are_routed = args->Is (Type::ROUTEDEVENTARGS);

	if (args_are_routed && element_list->First())
		((RoutedEventArgs*)args)->SetSource(((UIElementNode*)element_list->First())->uielement);

	for (node = (UIElementNode*)element_list->First(), idx = 0; node && idx < end_idx; node = (UIElementNode*)node->next, idx++) {
		args->ref ();
		bool h = node->uielement->DoEmit (event_id, args);
		if (h)
			handled = true;
		if (zombie) {
			handled = false;
			break;
		}
		
		if (args_are_routed && ((RoutedEventArgs*)args)->GetHandled())
			break;
	}

	args->unref();

	for (node = (UIElementNode*)element_list->First(), idx = 0; node && idx < end_idx; node = (UIElementNode*)node->next, idx++) {
		node->uielement->FinishEmit (event_id, emit_ctxs[idx]);
	}
	g_free (emit_ctxs);

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

static List*
copy_input_list_from_node (List *input_list, UIElementNode* node)
{
	List *list = new List ();

	while (node) {
		list->Append (new UIElementNode (node->uielement));
		node = (UIElementNode*) node->next;
	}

	return list;
}

bool
Surface::HandleMouseEvent (int event_id, bool emit_leave, bool emit_enter, bool force_emit, GdkEvent *event)
{
	bool handled = false;
	bool mouse_down =	event_id == UIElement::MouseLeftButtonDownEvent ||
						event_id == UIElement::MouseRightButtonDownEvent;

	if ((moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) == 0 && 
	    ((event_id == UIElement::MouseRightButtonDownEvent) || (event_id == UIElement::MouseRightButtonDownEvent)))
		event_id = NO_EVENT_ID;
		
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

	if (zombie)
		return false;

	if (toplevel == NULL || event == NULL)
		return false;

	emittingMouseEvent = true;

	if (captured) {
		// if the mouse is captured, the input_list doesn't ever
		// change, and we don't emit enter/leave events.  just emit
		// the event on the input_list.
		if (event_id != NO_EVENT_ID)
			handled = EmitEventOnList (event_id, input_list, event, -1);
	}
	else {
		// FIXME this should probably use mouse event args
		ProcessDirtyElements();

		int surface_index;
		int new_index;

		// Accumulate a new input_list, which contains the
		// most deeply nested hit testable UIElement covering
		// the point (x,y), and all visual parents up the
		// hierarchy to the root.
		List *new_input_list = new List ();
		double x, y;

		gdk_event_get_coords (event, &x, &y);

		Point p (x,y);

		cairo_t *ctx = measuring_context_create ();
		for (int i = layers->GetCount () - 1; i >= 0 && new_input_list->IsEmpty (); i--)
			layers->GetValueAt (i)->AsUIElement ()->HitTest (ctx, p, new_input_list);

		if (mouse_down) {
			GenerateFocusChangeEvents ();
			if (!GetFocusedElement ()) {
				int last = layers->GetCount () - 1;
				for (int i = last; i >= 0; i--) {
					if (TabNavigationWalker::Focus (layers->GetValueAt (i)->AsUIElement (), true))
						break;
				}
				if (!GetFocusedElement () && last != -1)
					FocusElement (layers->GetValueAt (last)->AsUIElement ());
			}
			GenerateFocusChangeEvents ();
		}
		
		
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
			handled = EmitEventOnList (UIElement::MouseLeaveEvent, input_list, event, surface_index);

		if (emit_enter)
			handled = EmitEventOnList (UIElement::MouseEnterEvent, new_input_list, event, new_index) || handled;

		if (event_id != NO_EVENT_ID && ((surface_index == 0 && new_index == 0) || force_emit))
			handled = EmitEventOnList (event_id, new_input_list, event, -1) || handled;
		
		// We need to remove from the new_input_list the events which have just 
		// became invisible/unhittable as the result of the event. 
		// (ie. element visibility was changed in the mouse enter).

		if (handled) {
			UIElementNode *node;

			for (node = (UIElementNode*)new_input_list->Last(); node; node = (UIElementNode*)node->prev) {
				if (! node->uielement->GetRenderVisible () ||
				    ! node->uielement->GetHitTestVisible ()) {
					// Ooops, looks like something changed.
					// We need to copy the list with some elements removed.
					List *list = copy_input_list_from_node (new_input_list, (UIElementNode*)node->next);
					delete new_input_list;
					new_input_list = list;
					break;
				}
			}
		}

		measuring_context_destroy (ctx);

		delete input_list;
		input_list = new_input_list;
	}

#define SPEW_INPUT_LIST 0

#if SPEW_INPUT_LIST
	{
		printf ("input_list: ");
		UIElementNode *node;
		for (node = (UIElementNode*)input_list->First(); node; node = (UIElementNode*)node->next) {
			if (node != input_list->First())
				printf ("->");
			printf ("(%s)", node->uielement->GetName());
		}
		printf ("\n");
	}
#endif

	// Perform any captures/releases that are pending after the
	// event is bubbled.
	if (pendingCapture)
		PerformCapture (pendingCapture);
	if (pendingReleaseCapture || (captured && !captured->CanCaptureMouse ()))
		PerformReleaseCapture ();
	emittingMouseEvent = false;
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
		new_cursor = node->uielement->GetCursor ();
		if (new_cursor != MouseCursorDefault)
			break;
	}

	SetCursor (new_cursor);
}

void
Surface::SetFPSReportFunc (MoonlightFPSReportFunc report, void *user_data)
{
	fps_report = report;
	fps_data = user_data;
}

void
Surface::SetCacheReportFunc (MoonlightCacheReportFunc report, void *user_data)
{
	cache_report = report;
	cache_data = user_data;
}

void 
Surface::SetExposeHandoffFunc (MoonlightExposeHandoffFunc func, void *user_data)
{
	expose_handoff = func;
	expose_handoff_data = user_data;
	expose_handoff_last_timespan = G_MAXINT64; 
}

class DownloaderNode : public List::Node {
public:
	Downloader *downloader;
	DownloaderNode (Downloader *dl) { downloader = dl; }		
};

void
Surface::DetachDownloaders ()
{
	DownloaderNode *node;
	if (downloaders == NULL)
		return;
		
	node = (DownloaderNode *) downloaders->First ();
	while (node != NULL) {
		node->downloader->RemoveHandler (Downloader::DestroyedEvent, OnDownloaderDestroyed, this);
		node->downloader->SetSurface (NULL);
		node = (DownloaderNode *) node->next;
	}
	downloaders->Clear (true);
}

void
Surface::OnDownloaderDestroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	DownloaderNode *node;
	Surface *surface = (Surface *) closure;
	List *downloaders = surface->downloaders;
	
	if (downloaders == NULL) {
		printf ("Surface::OnDownloaderDestroyed (): The list of downloaders is empty.\n");
		return;
	}
	
	node = (DownloaderNode *) downloaders->First ();
	while (node != NULL) {
		if (node->downloader == sender) {
			downloaders->Remove (node);
			return;
		}
		node = (DownloaderNode *) node->next;
	}
	
	printf ("Surface::OnDownloaderDestroyed (): Couldn't find the downloader %p in the list of downloaders\n", sender);
}

Downloader *
Surface::CreateDownloader (void) 
{
	if (zombie) {
		g_warning ("Surface::CreateDownloader (): Trying to create a downloader on a zombified surface.\n");
		return NULL;
	}
	
	Downloader *downloader = new Downloader ();
	downloader->SetSurface (this);
	downloader->SetContext (downloader_context);
	downloader->AddHandler (Downloader::DestroyedEvent, OnDownloaderDestroyed, this);
	if (downloaders == NULL)
		downloaders = new List ();
	downloaders->Append (new DownloaderNode (downloader));
	
	return downloader;
}

Downloader *
Surface::CreateDownloader (EventObject *obj)
{
	Surface *surface = obj ? obj->GetSurface () : NULL;

	if (surface == NULL)
		surface = Deployment::GetCurrent ()->GetSurface ();
	
	if (surface)
		return surface->CreateDownloader ();
	
	g_warning ("Surface::CreateDownloader (%p, ID: %i): Unable to create contextual downloader.\n",
		   obj, GET_OBJ_ID (obj));
	
	return NULL;
}

bool 
Surface::VerifyWithCacheSizeCounter (int w, int h)
{
	if (! (moonlight_flags & RUNTIME_INIT_USE_SHAPE_CACHE))
		return false;

	if (cache_size_multiplier == -1)
		return false;

	if (cache_size_in_bytes + (w * h * cache_size_multiplier) < MAXIMUM_CACHE_SIZE)
		return true;
	else
		return false;
}

gint64
Surface::AddToCacheSizeCounter (int w, int h)
{
	gint64 new_size = w * h * cache_size_multiplier;
	cache_size_in_bytes += new_size;
	return new_size;
}

void 
Surface::RemoveFromCacheSizeCounter (gint64 size)
{
	cache_size_in_bytes -= size;
}

bool
Surface::FullScreenKeyHandled (GdkEventKey *key)
{
	if (!GetFullScreen ())
		return false;
		
	// If we're in fullscreen mode no key events are passed through.
	// We only handle Esc, to exit fullscreen mode.
	if (key->keyval == GDK_Escape)
		SetFullScreen (false);
	
	switch (key->keyval) {
		case GDK_Down:
		case GDK_Up:
		case GDK_Left:
		case GDK_Right:
		case GDK_KP_Space:
		case GDK_space:
		case GDK_Tab:
		case GDK_Page_Down:
		case GDK_Page_Up:
		case GDK_Home:
		case GDK_End:
		case GDK_Return:
		case GDK_KP_Enter:
			return false;
			
		// Explicitly listing GDK_Escape here as it should never bubble up
		case GDK_Escape:
		default:
			return true;
	}
}

gboolean
Surface::HandleUIFocusIn (GdkEventFocus *event)
{
	if (IsZombie ())
		return false;

	time_manager->InvokeTickCalls();

	if (GetFocusedElement ()) {
		List *focus_to_root = ElementPathToRoot (GetFocusedElement ());
		EmitEventOnList (UIElement::GotFocusEvent, focus_to_root, (GdkEvent*)event, -1);
		delete focus_to_root;
	}

	return false;
}

gboolean
Surface::HandleUIFocusOut (GdkEventFocus *event)
{
	if (IsZombie ())
		return false;

	time_manager->InvokeTickCalls();

	if (GetFocusedElement ()) {
		List *focus_to_root = ElementPathToRoot (GetFocusedElement ());
		EmitEventOnList (UIElement::LostFocusEvent, focus_to_root, (GdkEvent*)event, -1);
		delete focus_to_root;
	}

	return false;
}

gboolean
Surface::HandleUIButtonRelease (GdkEventButton *event)
{
	time_manager->InvokeTickCalls();

	if (event->button != 1 && event->button != 3) {
		return false;
	}

	SetUserInitiatedEvent (true);
	
	if (mouse_event)
		gdk_event_free (mouse_event);
	
	mouse_event = gdk_event_copy ((GdkEvent *) event);

	HandleMouseEvent (event->button == 1 ? UIElement::MouseLeftButtonUpEvent : UIElement::MouseRightButtonUpEvent,
			  true, true, true, mouse_event);

	UpdateCursorFromInputList ();
	SetUserInitiatedEvent (false);

	// XXX MS appears to do this here, which is completely stupid.
	if (captured)
		PerformReleaseCapture ();

	return !((moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) == 0 && event->button == 3);
}

gboolean
Surface::HandleUIButtonPress (GdkEventButton *event)
{
	bool handled;
	int event_id;
	
	active_window->GrabFocus ();
	
	time_manager->InvokeTickCalls();

	if (event->button != 1 && event->button != 3)
		return false;

	SetUserInitiatedEvent (true);

	if (mouse_event)
		gdk_event_free (mouse_event);
	
	mouse_event = gdk_event_copy ((GdkEvent *) event);
	
	switch (event->type) {
	case GDK_3BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
		if (event->button != 1) {
			SetUserInitiatedEvent (false);
			return false;
		}
		
		handled = HandleMouseEvent (UIElement::MouseLeftButtonMultiClickEvent, false, false, true, mouse_event);
		break;
	default:
		if (event->button == 1)
			event_id = UIElement::MouseLeftButtonDownEvent;
		else
			event_id = UIElement::MouseRightButtonDownEvent;
		
		handled = HandleMouseEvent (event_id, true, true, true, mouse_event);
		break;
	}
	
	UpdateCursorFromInputList ();
	SetUserInitiatedEvent (false);

	return handled;
}

gboolean
Surface::HandleUIScroll (GdkEventScroll *event)
{
	time_manager->InvokeTickCalls();

	if (mouse_event)
		gdk_event_free (mouse_event);
	
	mouse_event = gdk_event_copy ((GdkEvent *) event);

	bool handled = false;

	handled = HandleMouseEvent (UIElement::MouseWheelEvent, true, true, true, mouse_event);

	UpdateCursorFromInputList ();

	return handled;
}

gboolean
Surface::HandleUIMotion (GdkEventMotion *event)
{
	time_manager->InvokeTickCalls();

	if (mouse_event)
		gdk_event_free (mouse_event);
	
	mouse_event = gdk_event_copy ((GdkEvent *) event);

	bool handled = false;

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
		((GdkEventMotion *) mouse_event)->x = ix;
		((GdkEventMotion *) mouse_event)->y = iy;
	    }    
	}

	handled = HandleMouseEvent (UIElement::MouseMoveEvent, true, true, true, mouse_event);
	UpdateCursorFromInputList ();

	return handled;
}

gboolean
Surface::HandleUICrossing (GdkEventCrossing *event)
{
	bool handled;

	time_manager->InvokeTickCalls();

	/* FIXME Disabling this for now... causes issues in ink journal
	GdkWindow *active_gdk_window = active_window->GetGdkWindow ();

	if (event->window && event->window != active_window->GetGdkWindow ()) {
		g_object_unref (active_gdk_window);
		return TRUE;
	} else
		g_object_unref (active_gdk_window);
	*/

	if (event->type == GDK_ENTER_NOTIFY) {
		if (mouse_event)
			gdk_event_free (mouse_event);
		mouse_event = gdk_event_copy ((GdkEvent *) event);
		
		handled = HandleMouseEvent (UIElement::MouseMoveEvent, true, true, false, mouse_event);

		UpdateCursorFromInputList ();
	
	} else {
		// forceably emit MouseLeave on the current input
		// list..  the "new" list computed by HandleMouseEvent
		// should be the same as the current one since we pass
		// in the same x,y but I'm not sure that's something
		// we can rely on.
		handled = HandleMouseEvent (UIElement::MouseLeaveEvent, false, false, true, mouse_event);

		// MS specifies that mouse capture is lost when you mouse out of the control
		if (captured)
			PerformReleaseCapture ();

		// clear out the input list so we emit the right
		// events when the pointer reenters the control.
		if (!emittingMouseEvent) {
			delete input_list;
			input_list = new List();
		}
	}

	return handled;
}

void
Surface::GenerateFocusChangeEvents()
{
	while (!focus_changed_events->IsEmpty ()) {
		FocusChangedNode *node = (FocusChangedNode *) focus_changed_events->Pop ();
	
		List *el_list;
		if (node->lost_focus) {
			el_list = ElementPathToRoot (node->lost_focus);
			EmitEventOnList (UIElement::LostFocusEvent, el_list, NULL, -1);
			delete (el_list);
		}
	
		if (node->got_focus) {
			el_list = ElementPathToRoot (node->got_focus);
			EmitEventOnList (UIElement::GotFocusEvent, el_list, NULL, -1);
			delete (el_list);
		}
		delete node;
	}
}

bool
Surface::FocusElement (UIElement *focused)
{
	if (focused == focused_element)
		return true;

	focus_changed_events->Push (new FocusChangedNode (focused_element, focused));
	focused_element = focused;

	if (FirstUserInitiatedEvent ())
		AddTickCall (Surface::AutoFocusAsync);
	return true;
}

List*
Surface::ElementPathToRoot (UIElement *source)
{
	List *list = new List();
	while (source) {
		list->Append (new UIElementNode (source));
		source = source->GetVisualParent();
	}
	return list;
}

gboolean 
Surface::HandleUIKeyPress (GdkEventKey *event)
{
	time_manager->InvokeTickCalls();

	Key key = Keyboard::MapKeyValToKey (event->keyval);
	
	if (Keyboard::IsKeyPressed (key)) {
		// If we are running an SL 1.0 application, then key repeats are dropped
		Deployment *deployment = Deployment::GetCurrent ();
		if (!deployment->IsLoadedFromXap ())
			return true;
	} else if (FullScreenKeyHandled (event)) {
		return true;
	}
	
#if DEBUG_MARKER_KEY
	static int debug_marker_key_in = 0;
	if (event->keyval == GDK_d || event->keyval == GDK_D) {
		if (!debug_marker_key_in)
			printf ("<--- DEBUG MARKER KEY IN (%f) --->\n", get_now () / 10000000.0);
		else
			printf ("<--- DEBUG MARKER KEY OUT (%f) --->\n", get_now () / 10000000.0);
		debug_marker_key_in = ! debug_marker_key_in;
		return true;
	}
#endif
	
	SetUserInitiatedEvent (true);
	bool handled = false;

	Keyboard::OnKeyPress (key);
	
	if (focused_element) {
		List *focus_to_root = ElementPathToRoot (focused_element);
		handled = EmitEventOnList (UIElement::KeyDownEvent, focus_to_root, (GdkEvent*)event, -1);
		delete focus_to_root;
	}
	else if (toplevel){
		// in silverlight 1.0, key events are only ever delivered to the toplevel
		toplevel->EmitKeyDown (event);
		handled = true;
	}

	SetUserInitiatedEvent (false);
	
	return handled;
}

gboolean 
Surface::HandleUIKeyRelease (GdkEventKey *event)
{
	time_manager->InvokeTickCalls();

	if (FullScreenKeyHandled (event))
		return true;

	SetUserInitiatedEvent (true);
	bool handled = false;

	Key key = Keyboard::MapKeyValToKey (event->keyval);
	Keyboard::OnKeyRelease (key);

	if (focused_element) {
		List *focus_to_root = ElementPathToRoot (focused_element);
		handled = EmitEventOnList (UIElement::KeyUpEvent, focus_to_root, (GdkEvent*)event, -1);
		delete focus_to_root;
	}
	else if (toplevel) {
		// in silverlight 1.0, key events are only ever delivered to the toplevel
		toplevel->EmitKeyUp (event);
		handled = true;
	}
	
	SetUserInitiatedEvent (false);
	
	return handled;
}

void
Surface::HandleUIWindowAllocation (bool emit_resize)
{
	Realloc ();
	if (emit_resize)
		Emit (ResizeEvent);
}

void
Surface::HandleUIWindowDestroyed (MoonWindow *window)
{
	if (window == fullscreen_window) {
		// switch out of fullscreen mode, as something has
		// destroyed our fullscreen window.
		UpdateFullScreen (false);
	}
	else if (window == normal_window) {
		// something destroyed our normal window
		normal_window = NULL;
	}

	if (window == active_window)
		active_window = NULL;
}

void
Surface::SetBackgroundColor (Color *color)
{
	if (background_color)
		delete background_color;
		
	background_color = new Color (*color);

	active_window->SetBackgroundColor (color);
	active_window->Invalidate ();
}

Color *
Surface::GetBackgroundColor ()
{
	return background_color;
}

bool
Surface::IsVersionSupported (const char *version_list)
{
	/* we support all 0.*, 1.0.*, 1.1.* and 2.0.* versions. */
	bool supported = true;
	gchar **versions;
	char *version = NULL;
	gint64 numbers [4];

	if (version_list == NULL)
		return false;

	versions = g_strsplit (version_list, ".", 4);

	supported = versions [0] != NULL && versions [1] != NULL;
	
	if (supported) {
		for (int k = 0; k < 4; k++) {
			numbers [k] = 0;
			version = versions [k];
			
			if (version == NULL)
				break;
			
			if (version [0] == 0) {
				supported = false;
				break;
			}
			
			// Only allow ascii 0-9 characters in the numbers
			for (int i = 0; version [i] != 0; i++) {
				if (version [i] < '0' || version [i] > '9') {
					supported = false;
					break;
				}
			}
			
			numbers [k] = atoll (version);
		}
		
		switch (numbers [0]) {
		case 0: // We support all versions of the format "0.*" and "1.*"
		case 1: 
			break;
		case 2:
			supported &= numbers [1] == 0; // 2.0.*
			break;
		case 3:
			supported &= numbers [1] == 0; // 3.0.*
			break;
		default:
			supported = false;
			break;
		}
	}
	
	g_strfreev (versions);

	return supported;
}

void
runtime_init_browser (const char *plugin_dir)
{
	runtime_init (plugin_dir, RUNTIME_INIT_BROWSER);
}

void
runtime_init_desktop ()
{
	runtime_init (NULL, RUNTIME_INIT_DESKTOP);
}

bool
runtime_is_running_out_of_browser ()
{
	return (moonlight_flags & RUNTIME_INIT_OUT_OF_BROWSER) != 0;
}

static gint32
get_flags (gint32 def, const char *envname, struct env_options options[])
{
	gint32 flags = def;
	const char *env;
	
	if (envname && (env = g_getenv (envname))) {
		printf ("%s = %s\n", envname, env);

		const char *flag = env;
		const char *inptr;
		bool all = !strcmp ("all", env);
		size_t n;
		uint i;
		
		while (*flag == ',')
			flag++;
		
		inptr = flag;
		
		while (*flag) {
			while (*inptr && *inptr != ',')
				inptr++;
			
			n = (inptr - flag);
			for (i = 0; options[i].name != NULL; i++) {
				if (all) {
					flags |= options[i].flag;
					continue;
				}
				
				if (n != strlen (options[i].name))
					continue;
				
				if (!strncmp (options[i].name, flag, n)) {
					if (!options[i].set)
						flags &= ~options[i].flag;
					else
						flags |= options[i].flag;
				}
			}
			
			while (*inptr == ',')
				inptr++;
			
			flag = inptr;
		}
	}
	return flags;
}

void
runtime_init (const char *platform_dir, guint32 flags)
{
	if (inited)
		return;

	if (cairo_version () < CAIRO_VERSION_ENCODE(1,4,0)) {
		printf ("*** WARNING ***\n");
		printf ("*** Cairo versions < 1.4.0 should not be used for Moon.\n");
		printf ("*** Moon is being run against version %s.\n", cairo_version_string ());
		printf ("*** Proceed at your own risk\n");
	}

	if (running_on_nvidia ()) {
		printf ("Moonlight: Forcing client-side rendering because we detected binary drivers which are known to suffer performance problems.\n");
		flags &= ~RUNTIME_INIT_USE_BACKEND_XLIB;
	}

	// Allow the user to override the flags via his/her environment
	flags = get_flags (flags, "MOONLIGHT_OVERRIDES", overrides);
#if DEBUG || LOGGING
	debug_flags_ex = get_flags (0, "MOONLIGHT_DEBUG", debug_extras);
	debug_flags = get_flags (0, "MOONLIGHT_DEBUG", debugs);
#endif

	inited = true;
	
	if (!g_type_inited) {
		g_type_inited = true;
		g_type_init ();
	}
	
	moonlight_flags = flags;

	Deployment::Initialize (platform_dir, (flags & RUNTIME_INIT_CREATE_ROOT_DOMAIN) != 0);

	xaml_init ();
	downloader_init ();
	Media::Initialize ();
}

void
runtime_shutdown (void)
{
	if (!inited)
		return;

	Media::Shutdown ();
	
	inited = false;
}
