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

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "debug.h"
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
#include "framerate-display.h"
#include "drm.h"
#include "utils.h"
#include "timemanager.h"

#include "contentcontrol.h"
#include "usercontrol.h"
#include "deployment.h"
#include "grid.h"
#include "cbinding.h"
#include "tabnavigationwalker.h"
#include "window.h"
#if PAL_GTK_WINDOWING
#include "pal-gtk.h"
#endif
#if PAL_GLIB_MESSAGING
#include "pal/messaging/glib/pal-glib-msg.h"
#endif
#if PAL_LINUX_CAPTURE
#include "pal/capture/pal-linux-capture.h"
#endif
#if PAL_DBUS_NETWORKAVAILABILITY
#include "pal/network/dbus/pal-dbus-network.h"
#endif

#include "pipeline.h"
#include "effect.h"

//#define DEBUG_INVALIDATE 1
//#define RENDER_INDIVIDUALLY 1
#define DEBUG_REFCNT 0

#define CAIRO_CLIP 0
#define TIME_CLIP 0
#define TIME_REDRAW 1

#define NO_EVENT_ID -1

bool Surface::main_thread_inited = false;
pthread_t Surface::main_thread = 0;

static MoonWindowingSystem *windowing_system = NULL;
static MoonInstallerService *installer_service = NULL;
static MoonMessagingService *messaging_service = NULL;
static MoonCaptureService *capture_service = NULL;
static MoonNetworkService *network_service = NULL;

bool inited = false;
bool g_type_inited = false;
GList* surface_list = NULL;
guint32 moonlight_flags = 0;
#if DEBUG || LOGGING
guint32 debug_flags_ex = 0;
guint32 debug_flags = 0;
#endif

static MoonlightRuntimeOption options [] = {

	/* flag                               name                 enable        disable   runtime changable  description */
	// There's no "ms-codecs=yes" option to not allow enablin  g them from the command line.
	{ RUNTIME_INIT_ENABLE_MS_CODECS,      "ms-codecs",         NULL,         "no" },
	{ RUNTIME_INIT_DISABLE_FFMPEG_CODECS, "ffmpeg-codecs",     "no",         "yes" },
	{ RUNTIME_INIT_MANUAL_TIMESOURCE,     "timesource",        "manual",     "system" },
	{ RUNTIME_INIT_SHOW_EXPOSE,           "expose",            "show",       "hide",   true,            "Show expose regions" },
	{ RUNTIME_INIT_SHOW_CLIPPING,         "clipping",          "show",       "hide",   true,            "Show clipping rectangles" },
	{ RUNTIME_INIT_SHOW_BOUNDING_BOXES,   "bbox",              "show",       "hide",   true,            "Show UIElement bounding boxes" },
	{ RUNTIME_INIT_SHOW_TEXTBOXES,        "textbox",           "show",       "hide",   true,            "Show TextBox bounds" },
	{ RUNTIME_INIT_SHOW_FPS,              "fps",               "show",       "hide",   true,            "Show Frames Per Second" },
	{ RUNTIME_INIT_OCCLUSION_CULLING,     "occlusion-culling", "yes",        "no",    true,             "Enable Occlusion Culling" },
	{ RUNTIME_INIT_SHOW_CACHE_SIZE,       "cache",             "show",       "hide",   true,            "Show cache size" },
	{ RUNTIME_INIT_FFMPEG_YUV_CONVERTER,  "converter",         "ffmpeg",     "default" },
	{ RUNTIME_INIT_USE_SHAPE_CACHE,       "shapecache",        "yes",        "no" },
	{ RUNTIME_INIT_USE_UPDATE_POSITION,   "updatepos",         "yes",        "no" },
	{ RUNTIME_INIT_ALLOW_WINDOWLESS,      "windowless",        "yes",        "no" },
	{ RUNTIME_INIT_AUDIO_PULSE,           "audio",             "pulseaudio", "alsa" },
	{ RUNTIME_INIT_AUDIO_ALSA,            "alsa",              "mmap",       "rw" },
	{ RUNTIME_INIT_USE_IDLE_HINT,         "idlehint",          "yes",        "no" },
	{ RUNTIME_INIT_USE_BACKEND_IMAGE,     "backend",           "image",      "native", true,            "Use client side surface rasterizing" },
	{ RUNTIME_INIT_KEEP_MEDIA,            "keepmedia",         "yes",        "no",     true,            "Don't remove media files from /tmp after download" },
	{ RUNTIME_INIT_ALL_IMAGE_FORMATS,     "allimages",         "yes",        "no" },
	{ RUNTIME_INIT_EMULATE_KEYCODES,      "emulatekeycodes",   "yes",        "no",     true,            "Emulate Windows PlatformKeyCodes" },
	{ RUNTIME_INIT_ENABLE_EFFECTS,        "effects",           "yes",        "no",     true,            "Enable Pixel Effects" },
	{ RUNTIME_INIT_CURL_BRIDGE,           "curlbridge",        "yes",        "no",     true,            "Prefer Curl bridge" },
	{ RUNTIME_INIT_ENABLE_TOGGLEREFS,     "togglerefs",        "yes",        "no" },
	{ RUNTIME_INIT_OOB_LAUNCHER_FIREFOX,  "ooblauncher",       "firefox",    "default" , true,          "Use firefox to execute out-of-browser applications" },
	{ (RuntimeInitFlag)0 }

};

const MoonlightRuntimeOption *
moonlight_get_runtime_options ()
{
	return options;
}

void
moonlight_set_runtime_option (RuntimeInitFlag flag, bool set)
{
	int index = -1;
	for (int i = 0; options [i].name != NULL; i++) {
		if (options [i].flag == flag) {
			index = i;
			break;
		}
	}

	if (index == -1)
		return;

	if (options[index].runtime_changeable) {
		printf ("Moonlight: setting option %s to %i\n", index == -1 ? "?" : options [index].name, set);
		if (set) {
			moonlight_flags |= flag;
		} else {
			moonlight_flags &= ~flag;
		}
	}
}


bool
moonlight_get_runtime_option (RuntimeInitFlag flag)
{
	return moonlight_flags & flag;
}

#define RUNTIME_INIT_DESKTOP (RuntimeInitFlag)(RUNTIME_INIT_OCCLUSION_CULLING | RUNTIME_INIT_USE_UPDATE_POSITION | RUNTIME_INIT_USE_SHAPE_CACHE | RUNTIME_INIT_USE_IDLE_HINT | RUNTIME_INIT_USE_BACKEND_IMAGE | RUNTIME_INIT_ALL_IMAGE_FORMATS | RUNTIME_INIT_DESKTOP_EXTENSIONS | RUNTIME_INIT_ENABLE_EFFECTS | RUNTIME_INIT_ENABLE_TOGGLEREFS )
#define RUNTIME_INIT_BROWSER (RuntimeInitFlag)(RUNTIME_INIT_OCCLUSION_CULLING | RUNTIME_INIT_USE_UPDATE_POSITION | RUNTIME_INIT_USE_SHAPE_CACHE | RUNTIME_INIT_ALLOW_WINDOWLESS | RUNTIME_INIT_USE_IDLE_HINT | RUNTIME_INIT_USE_BACKEND_IMAGE | RUNTIME_INIT_ENABLE_MS_CODECS | RUNTIME_INIT_CREATE_ROOT_DOMAIN | RUNTIME_INIT_ENABLE_EFFECTS | RUNTIME_INIT_ENABLE_TOGGLEREFS )

#if DEBUG || LOGGING
static struct MoonlightDebugOption debugs[] = {
	{ "alsa",              RUNTIME_DEBUG_ALSA },
	{ "asf",               RUNTIME_DEBUG_ASF },
	{ "audio",             RUNTIME_DEBUG_AUDIO },
	{ "buffering",         RUNTIME_DEBUG_BUFFERING },
	{ "codecs",            RUNTIME_DEBUG_CODECS },
	{ "curl",              RUNTIME_DEBUG_CURL },
	{ "demuxers",          RUNTIME_DEBUG_DEMUXERS },
	{ "dependencyobject",  RUNTIME_DEBUG_DP },
	{ "deployment",        RUNTIME_DEBUG_DEPLOYMENT },
	{ "downloader",        RUNTIME_DEBUG_DOWNLOADER },
	{ "effect",            RUNTIME_DEBUG_EFFECT },
	{ "ffmpeg",            RUNTIME_DEBUG_FFMPEG },
	{ "font",              RUNTIME_DEBUG_FONT },
	{ "framereaderloop",   RUNTIME_DEBUG_FRAMEREADERLOOP },
	{ "layout",            RUNTIME_DEBUG_LAYOUT },
	{ "markers",           RUNTIME_DEBUG_MARKERS },
	{ "media",             RUNTIME_DEBUG_MEDIA },
	{ "mediaelement",      RUNTIME_DEBUG_MEDIAELEMENT },
	{ "mediaplayer",       RUNTIME_DEBUG_MEDIAPLAYER },
	{ "mms",               RUNTIME_DEBUG_MMS },
	{ "mp3",               RUNTIME_DEBUG_MP3 },
	{ "mp4",               RUNTIME_DEBUG_MP4 },
	{ "msi",               RUNTIME_DEBUG_MSI },
	{ "oob",               RUNTIME_DEBUG_OOB },
	{ "pipeline",          RUNTIME_DEBUG_PIPELINE },
	{ "pipeline-error",    RUNTIME_DEBUG_PIPELINE_ERROR },
	{ "playlist",          RUNTIME_DEBUG_PLAYLIST },
	{ "pulse",             RUNTIME_DEBUG_PULSE },
	{ "text",              RUNTIME_DEBUG_TEXT },
	{ "ui",                RUNTIME_DEBUG_UI },
	{ "value",             RUNTIME_DEBUG_VALUE },
	{ "xaml",              RUNTIME_DEBUG_XAML },
	{ NULL }
};

static struct MoonlightDebugOption debug_extras[] = {
	{ "alsa-ex",           RUNTIME_DEBUG_ALSA_EX },
	{ "audio-ex",          RUNTIME_DEBUG_AUDIO_EX },
	{ "markers-ex",        RUNTIME_DEBUG_MARKERS_EX },
	{ "mediaelement-ex",   RUNTIME_DEBUG_MEDIAELEMENT_EX },
	{ "mediaplayer-ex",    RUNTIME_DEBUG_MEDIAPLAYER_EX },
	{ "mms-ex",            RUNTIME_DEBUG_MMS_EX },
	{ "pipeline-ex",       RUNTIME_DEBUG_PIPELINE_EX },
	{ "playlist-ex",       RUNTIME_DEBUG_PLAYLIST_EX },
	{ "pulse-ex",          RUNTIME_DEBUG_PULSE_EX },
	{ NULL }
};

const MoonlightDebugOption *
moonlight_get_debug_options ()
{
	return debugs;
}

void
moonlight_set_debug_option (guint32 flag, bool set)
{
	int index = -1;
	for (int i = 0; debugs [i].name != NULL; i++) {
		if (debugs [i].flag == flag) {
			index = i;
			break;
		}
	}
	printf ("Moonlight: setting option %s to %i\n", index == -1 ? "?" : debugs [index].name, set);
	if (set) {
		debug_flags |= flag;
	} else {
		debug_flags &= ~flag;
	}
}

bool
moonlight_get_debug_option (guint32 flag)
{
	return debug_flags & flag;
}

const MoonlightDebugOption *
moonlight_get_debug_ex_options ()
{
	return debug_extras;
}

void
moonlight_set_debug_ex_option (guint32 flag, bool set)
{
	int index = -1;
	for (int i = 0; debug_extras [i].name != NULL; i++) {
		if (debug_extras [i].flag == flag) {
			index = i;
			break;
		}
	}
	printf ("Moonlight: setting debug-ex option %s to %i\n", index == -1 ? "?" : debug_extras [index].name, set);
	if (set) {
		debug_flags_ex |= flag;
	} else {
		debug_flags_ex &= ~flag;
	}
}

bool
moonlight_get_debug_ex_option (guint32 flag)
{
	return debug_flags_ex & flag;
}
#endif

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
	: EventObject (Type::SURFACE)
{
	GetDeployment ()->SetSurface (this);

	main_thread = pthread_self ();
	main_thread_inited = true;
	
	zombie = false;
	background_color = NULL;
	cursor = CursorTypeDefault;
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
	focus_changed_events = new List ();

	full_screen = false;
	first_user_initiated_event = false;
	user_initiated_event = false;
	user_initiated_monotonic_counter = 0;
	
	zoom_factor = 1.0;
	
	incomplete_support_message = NULL;
	drm_message = NULL;
	full_screen_message = NULL;
	source_location = NULL;

	framerate_counter_display = NULL;
	framerate_textblock = NULL;
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
	
	enable_redraw_regions = false;
	
	emittingMouseEvent = false;
	pendingCapture = NULL;
	pendingReleaseCapture = false;

#ifdef DEBUG
	debug_selected_element = NULL;
#endif

	up_dirty = new DirtyLists (true);
	down_dirty = new DirtyLists (false);
	
	surface_list = g_list_append (surface_list, this);

	SetRuntimeOptions (moonlight_flags);
}

Surface::~Surface ()
{
	time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
		
	if (toplevel) {
		toplevel->SetIsAttached (false);
		toplevel->unref ();
	}
	
#if DEBUG
	if (debug_selected_element) {
		debug_selected_element->unref ();
		debug_selected_element = NULL;
	}
#endif
	
	HideFullScreenMessage ();
	
	delete focus_changed_events;
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
	
	layers->unref ();
	
	surface_list = g_list_remove (surface_list, this);
}

void
Surface::Dispose ()
{
	if (toplevel) {
		toplevel->SetIsAttached (false);
		toplevel->Dispose ();
	}
	
	EventObject::Dispose ();
}

void
Surface::Zombify ()
{
	time_manager->Shutdown ();
	zombie = true;
}

guint32
Surface::GetRuntimeOptions ()
{
	return surface_flags;
}

void
Surface::SetRuntimeOptions (guint32 flags)
{
	surface_flags = flags;
	for (int i = 0; i < 32; i ++)
		SetRuntimeOption ((RuntimeInitFlag)(1 << i), (flags & (1 << i)) != 0);
}

bool
Surface::GetRuntimeOption (RuntimeInitFlag flag)
{
	return (surface_flags & flag) != 0;
}

void
Surface::SetRuntimeOption (RuntimeInitFlag flag, bool value)
{
	if (value)
		surface_flags |= (1 << flag);
	else
		surface_flags &= ~(1 << flag);

	switch (flag) {
	case RUNTIME_INIT_SHOW_EXPOSE:
		SetEnableRedrawRegions (value);
		break;

	case RUNTIME_INIT_SHOW_FPS:
		SetEnableFrameRateCounter (value);
		break;

	// FIXME: these flags just modify the global settings, but
	// many shouldn't
	case RUNTIME_INIT_ENABLE_EFFECTS:
	case RUNTIME_INIT_SHOW_CACHE_SIZE:
	case RUNTIME_INIT_SHOW_CLIPPING:
	case RUNTIME_INIT_SHOW_BOUNDING_BOXES:
	case RUNTIME_INIT_SHOW_TEXTBOXES:
	case RUNTIME_INIT_OCCLUSION_CULLING:
	case RUNTIME_INIT_USE_BACKEND_IMAGE:
	case RUNTIME_INIT_KEEP_MEDIA:
	case RUNTIME_INIT_CURL_BRIDGE:
	case RUNTIME_INIT_EMULATE_KEYCODES:
	default:
		moonlight_set_runtime_option (flag, value);
		break;
	}
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
Surface::SetCursor (CursorType new_cursor)
{
	if (new_cursor != cursor) {
		cursor = new_cursor;

		active_window->SetCursor (cursor);
	}
}

TimeManager *
Surface::GetTimeManagerReffed ()
{
	TimeManager *result;
	time_manager_mutex.Lock ();
	result = time_manager;
	if (result)
		result->ref ();
	time_manager_mutex.Unlock ();
	return result;
}

void
Surface::EmitFocusChangeEventsAsync (EventObject *sender)
{
	((Surface *)sender)->EmitFocusChangeEvents ();
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
		time_manager_mutex.Lock ();
		time_manager->unref ();
		time_manager = new TimeManager ();
		time_manager_mutex.Unlock ();
		time_manager->AddHandler (TimeManager::RenderEvent, render_cb, this);
		time_manager->AddHandler (TimeManager::UpdateInputEvent, update_input_cb, this);
		time_manager->SetMaximumRefreshRate (maxframerate);
		time_manager->NeedRedraw ();
		time_manager->Start ();
	} else 
		first = true;

	if (!element) {
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
		NameScope *ns = new NameScope ();
		NameScope::SetNameScope (new_toplevel, ns);
		ns->unref ();
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

	layer->SetIsAttached (true);
	layer->FullInvalidate (true);
	layer->InvalidateMeasure ();
	layer->WalkTreeForLoadedHandlers (NULL, false, false);
	Deployment::GetCurrent()->PostLoaded ();
}

void
Surface::DetachLayer (UIElement *layer)
{
	// if the layer contained the last UIElement receiving mouse input, clear the input list.
	if (!input_list->IsEmpty() && ((UIElementNode*)input_list->Last())->uielement == layer) {
		delete input_list;
		input_list = new List ();
	}

	// if the layer contained the last focused UIElement, clear that as well.
	if (focused_element) {
		bool in_this_layer = false;
		UIElement *f = focused_element;
		while (f != NULL) {
			if (f == layer) {
				in_this_layer = true;
				break;
			}
			f = f->GetVisualParent();
		}
		if (in_this_layer)
			FocusElement (NULL);
	}

	// XXX should we also clear out the focus_changed_events list?

	layers->Remove (Value (layer));
	layer->SetIsAttached (false);
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

// arbitrary cairo context.
void
Surface::Paint (cairo_t *ctx, Region *region)
{
	Paint (ctx, region, false, false);
}

#if 0
static void
dump_render_list (List *render_list)
{
	RenderNode *node;
	int indent = 0;

	node = (RenderNode*)render_list->First();
	while (node) {
		if (node->pre_render) {
			Rect r = node->region->ClipBox();
			Rect bounds = node->uielement->GetBounds();

			for (int i = 0; i < indent; i ++) putc (' ', stdout);
			printf ("<Render Name=\"%s\" Type=\"%s\" RenderExtents=\"%g %g %g %g\" Bounds=\"%g %g %g %g\" ",
				node->uielement->GetName(), node->uielement->GetTypeName(),
				r.x, r.y, r.width, r.height,
				bounds.x, bounds.y, bounds.width, bounds.height);
			printf ("RenderSelf=\"%s\" ", node->render_element ? "true" : "false");
			if (node->uielement->GetEffect())
				printf ("Effect=\"yes\" ");
			if (node->uielement->GetOpacityMask ()) {
				Brush *b = node->uielement->GetOpacityMask();
				if (b->Is (Type::RADIALGRADIENTBRUSH))
					printf ("OpacityMask=\"RadialGradient\" ");
				else if (b->Is (Type::LINEARGRADIENTBRUSH))
					printf ("OpacityMask=\"LinearGradient\" ");
			}
			if (node->uielement->GetValue (Shape::FillProperty)) {
				Brush *b = node->uielement->GetValue (Shape::FillProperty)->AsBrush();
				if (b->Is (Type::RADIALGRADIENTBRUSH))
					printf ("Fill=\"RadialGradient\" ");
				else if (b->Is (Type::LINEARGRADIENTBRUSH))
					printf ("Fill=\"LinearGradient\" ");
			}
			if (node->uielement->GetValue (Shape::StrokeProperty)) {
				Brush *b = node->uielement->GetValue (Shape::StrokeProperty)->AsBrush();
				if (b->Is (Type::RADIALGRADIENTBRUSH))
					printf ("Stroke=\"RadialGradient\" ");
				else if (b->Is (Type::LINEARGRADIENTBRUSH))
					printf ("Stroke=\"LinearGradient\" ");
			}
			if (node->post_render)
				printf ("/>");
			else {
				printf (">");
				indent += 2;
			}
		}
		else if (node->post_render) {
			indent -= 2;
			for (int i = 0; i < indent; i ++) putc (' ', stdout);
			printf ("</Render Name=\"%s\">", node->uielement->GetName());
		}
		printf ("\n");
		node = (RenderNode*)node->next;
	}
}
#endif

void
Surface::Paint (cairo_t *cr, Region *region, bool transparent, bool clear_transparent)
{
	frames++;

#if OCCLUSION_CULLING_STATS
	uielements_rendered_with_occlusion_culling = 0;
	uielements_rendered_with_painters = 0;
#endif

	List *ctx = new List ();
	List *render_list = new List ();

	bool did_occlusion_culling = false;

	ctx->Append (new ContextNode (cr));

	int layer_count = layers->GetCount ();

	if (moonlight_flags & RUNTIME_INIT_OCCLUSION_CULLING) {
		Region *copy = new Region (region);

		for (int i = layer_count - 1; i >= 0; i --) {
			UIElement *layer = layers->GetValueAt (i)->AsUIElement ();

			layer->FrontToBack (copy, render_list);
		}

		if (!render_list->IsEmpty ()) {
			region->Draw (cr);

			cairo_clip (cr);

			if (!copy->IsEmpty()) {
				copy->Draw (cr);
				PaintBackground (cr, transparent, clear_transparent);
				cairo_new_path (cr);
			}

			cairo_save (cr);
			while (RenderNode *node = (RenderNode*)render_list->First()) {
				node->Render (ctx);

				render_list->Remove (node);
			}

			did_occlusion_culling = true;
			cairo_restore (cr);
		}
	}

	if (!did_occlusion_culling) {
		region->Draw (cr);

		PaintBackground (cr, transparent, clear_transparent);

		cairo_clip (cr);

		cairo_save (cr);
		for (int i = 0; i < layer_count; i ++) {
			UIElement *layer = layers->GetValueAt (i)->AsUIElement ();

			layer->DoRender (ctx, region);
		}
		cairo_restore (cr);
	}

#ifdef DEBUG
	if (debug_selected_element) {
		Rect bounds = debug_selected_element->GetSubtreeBounds();
// 			printf ("debug_selected_element is %s\n", debug_selected_element->GetName());
// 			printf ("bounds is %g %g %g %g\n", bounds.x, bounds.y, bounds.w, bounds.h);
		cairo_save (cr);
		//RenderClipPath (cr);
		cairo_new_path (cr);
		cairo_identity_matrix (cr);
		cairo_set_source_rgba (cr, 1.0, 0.5, 0.2, 1.0);
		cairo_set_line_width (cr, 1);
		cairo_rectangle (cr, bounds.x, bounds.y, bounds.width, bounds.height);
		cairo_stroke (cr);
		cairo_restore (cr);
	}
#endif

	if (GetEnableRedrawRegions ()) {
		// pink: 234, 127, 222
		// yellow: 234, 239, 110
		// purple: 127, 127, 222
		int r = abs (frames) % 3 == 2 ? 127 : 234;
		int g = abs (frames) % 3 == 1 ? 239 : 127;
		int b = abs (frames) % 3 == 1 ? 110 : 222;
		
		cairo_new_path (cr);
		region->Draw (cr);
		//cairo_set_line_width (cr, 2.0);
		cairo_set_source_rgba (cr, (double) r / 255.0, (double) g / 255.0, (double) b / 255.0, 0.75);
		cairo_fill (cr);
	}


	delete render_list;
	delete ctx;

#if OCCLUSION_CULLING_STATS
	printf ("%d UIElements rendered using occlusion culling for Surface::Paint (%p)\n", uielements_rendered_with_occlusion_culling, this);
	printf ("%d UIElements rendered using normal painter's algorithm for Surface::Paint (%p)\n", uielements_rendered_with_painters, this);
#endif
}

void
Surface::PaintBackground (cairo_t *ctx, bool transparent, bool clear_transparent)
{
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
Surface::EmitSourceDownloadProgressChanged (float progress)
{
	if (HasHandlers (SourceDownloadProgressChangedEvent))
		Emit (SourceDownloadProgressChangedEvent, new DownloadProgressEventArgs (progress));
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
	int layers_count = layers->GetCount ();
	for (int i = 0; i < layers_count; i++) {
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
	EmitFocusChangeEvents ();
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

	int layer_count = layers->GetCount ();
	for (int i = 0; i < layer_count && !ret; i++)
		ret = layers->GetValueAt (i)->AsUIElement () == top;

	return ret;
}

void
Surface::ShowIncompleteSilverlightSupportMessage ()
{
#if DEBUG
	if (getenv ("MOONLIGHT_DISABLE_INCOMPLETE_MESSAGE") != NULL)
		return;
#endif
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

	
	char *message_text = g_strdup_printf ("You are running a Silverlight %c application.  You may experience incompatibilities as Moonlight does not have full support for this runtime yet.", GetDeployment()->GetRuntimeVersion()[0]);
	message_block->SetValue (TextBlock::TextProperty, message_text);
	g_free (message_text);

	DependencyObject* storyboard_object = incomplete_support_message->FindName ("FadeOut");
	Storyboard* storyboard = (storyboard_object != NULL && storyboard_object->Is (Type::STORYBOARD)) ? (Storyboard*) storyboard_object : NULL;

	storyboard->AddHandler (Timeline::CompletedEvent, HideIncompleteSilverlightSupportMessageCallback, this);

	// make the message take up the full width of the window
	message->SetValue (FrameworkElement::WidthProperty, Value ((double)active_window->GetWidth()));
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
	drm_message->SetValue (FrameworkElement::WidthProperty, Value ((double)active_window->GetWidth()));
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
		DetachLayer (drm_message);
		drm_message->unref ();
		drm_message = NULL;
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
	
	DependencyObject* url_object = full_screen_message->FindName ("url");
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
		fullscreen_window = windowing_system->CreateWindow (true, -1, -1, normal_window, this);
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
Surface::SetEnableFrameRateCounter (bool value)
{
	enable_fps_counter = value;

	if (value) {
		ShowFrameRateCounter ();
	}
	else {
		HideFrameRateCounter ();
	}
}

bool
Surface::GetEnableFrameRateCounter ()
{
	return enable_fps_counter || (moonlight_flags & RUNTIME_INIT_SHOW_FPS);
}

void
Surface::ShowFrameRateCounter ()
{
	g_return_if_fail (framerate_counter_display == NULL);

	Type::Kind dummy;
	XamlLoader *loader = new XamlLoader (NULL, FRAMERATE_COUNTER_DISPLAY, this);
	DependencyObject* display = loader->CreateDependencyObjectFromString (FRAMERATE_COUNTER_DISPLAY, false, &dummy);
	delete loader;

	if (!display) {
		g_warning ("Unable to create frame rate counter display.\n");
		return;
	}
	
	if (!display->Is (Type::FRAMEWORKELEMENT)) {
		g_warning ("Unable to create framerate counter display, got a %s, expected at least a FrameworkElement.\n", display->GetTypeName ());
		display->unref ();
		return;
	}

	framerate_counter_display = (Panel *) display;
	AttachLayer (framerate_counter_display);

	DependencyObject* fps_textblock_object = framerate_counter_display->FindName ("framerate");
	framerate_textblock = (fps_textblock_object != NULL && fps_textblock_object->Is (Type::TEXTBLOCK)) ? (TextBlock*) fps_textblock_object : NULL;

	
	// make the message take up the full width of the window
	display->SetValue (FrameworkElement::WidthProperty, Value ((double)active_window->GetWidth()));
}

void
Surface::HideFrameRateCounter ()
{
	if (framerate_counter_display) {
		DetachLayer (framerate_counter_display);
		framerate_counter_display->unref ();
		framerate_counter_display = NULL;
		framerate_textblock = NULL;
	}
}

void
Surface::UpdateFrameRateCounter (gint64 now)
{
	if ((now = get_now ()) <= (fps_start + TIMESPANTICKS_IN_SECOND))
		return;
	if (!framerate_textblock)
		return;

	float nsecs = (now - fps_start) / TIMESPANTICKS_IN_SECOND_FLOAT;

	char *msg = g_strdup_printf ("%.3f FPS", fps_nframes / nsecs);

	framerate_textblock->SetText (msg);

	g_free (msg);

	fps_nframes = 0;
	fps_start = now;
}

bool
Surface::GetEnableRedrawRegions ()
{
	return enable_redraw_regions || (moonlight_flags & RUNTIME_INIT_SHOW_EXPOSE);
}

void
Surface::render_cb (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Surface *s = (Surface *) closure;
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

	if (s->GetEnableFrameRateCounter () && s->fps_start == 0)
		s->fps_start = get_now ();
	
	if (dirty) {
		s->ProcessUpdates ();
	}

	if (s->GetEnableFrameRateCounter ()) {
		s->fps_nframes++;
		s->UpdateFrameRateCounter (get_now ());
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

	Emit (Surface::WindowAvailableEvent);
}

void
Surface::HandleUIWindowUnavailable ()
{
	time_manager->RemoveHandler (TimeManager::RenderEvent, render_cb, this);
	time_manager->RemoveHandler (TimeManager::UpdateInputEvent, update_input_cb, this);

	Emit (Surface::WindowUnavailableEvent);
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
RenderNode::Render (List *ctx)
{
	bool use_occlusion_culling = uielement->UseOcclusionCulling ();

#if OCCLUSION_CULLING_STATS
	if (use_occlusion_culling)
		uielements_rendered_with_occlusion_culling ++;
#endif

	if (pre_render)
		pre_render (ctx, uielement, region, use_occlusion_culling);

	if (render_element) {
		uielement->Render (((ContextNode *) ctx->First ())->GetCr (), region);
	}
	
	if (post_render)
		post_render (ctx, uielement, region, use_occlusion_culling);
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
Surface::CreateArgsForEvent (int event_id, MoonEvent *event)
{
	if (event_id ==UIElement::InvalidatedEvent
	    || event_id ==UIElement::GotFocusEvent
	    || event_id ==UIElement::LostFocusEvent)
		return new RoutedEventArgs ();
	else if (event_id == UIElement::MouseLeaveEvent
		 || event_id ==UIElement::MouseMoveEvent
		 || event_id ==UIElement::MouseEnterEvent)
		return new MouseEventArgs((MoonMouseEvent*)event);
	else if (event_id ==UIElement::MouseLeftButtonMultiClickEvent
		 || event_id ==UIElement::MouseLeftButtonDownEvent
		 || event_id ==UIElement::MouseLeftButtonUpEvent
		 || event_id ==UIElement::MouseRightButtonDownEvent
		 || event_id ==UIElement::MouseRightButtonUpEvent)
		return new MouseButtonEventArgs((MoonButtonEvent*)event);
	else if (event_id == UIElement::MouseWheelEvent)
		return new MouseWheelEventArgs((MoonScrollWheelEvent*)event);
	else if (event_id == UIElement::KeyDownEvent
		 || event_id == UIElement::KeyUpEvent)
		return new KeyEventArgs((MoonKeyEvent*)event);
	else {
		g_warning ("Unknown event id %d\n", event_id);
		return new EventArgs();
	}
}

bool
Surface::EmitEventOnList (int event_id, List *element_list, MoonEvent *event, int end_idx)
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
		
		if (node->uielement->DoEmit (event_id, args))
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
Surface::HandleMouseEvent (int event_id, bool emit_leave, bool emit_enter, bool force_emit, MoonMouseEvent *event)
{
	bool handled = false;
	bool mouse_down = (event_id == UIElement::MouseLeftButtonDownEvent ||
			   event_id == UIElement::MouseRightButtonDownEvent);

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

		Point p (event->GetPosition ());

		cairo_t *ctx = measuring_context_create ();
		int layer_count = layers->GetCount ();
		for (int i = layer_count - 1; i >= 0 && new_input_list->IsEmpty (); i--)
			layers->GetValueAt (i)->AsUIElement ()->HitTest (ctx, p, new_input_list);

		if (mouse_down) {
			EmitFocusChangeEvents ();
			if (!GetFocusedElement ()) {
				int last = layer_count - 1;
				for (int i = last; i >= 0; i--) {
					if (TabNavigationWalker::Focus (layers->GetValueAt (i)->AsUIElement (), true))
						break;
				}
				if (!GetFocusedElement () && last != -1)
					FocusElement (layers->GetValueAt (last)->AsUIElement ());
			}
			EmitFocusChangeEvents ();
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
	CursorType new_cursor = CursorTypeDefault;
	
	// loop over the input list in order until we hit a node that
	// has its cursor set to the non-default.
	UIElementNode *node;
	for (node = (UIElementNode*)input_list->First(); node; node = (UIElementNode*)node->next) {
		new_cursor = node->uielement->GetCursor ();
		if (new_cursor != CursorTypeDefault)
			break;
	}

	SetCursor (new_cursor);
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
Surface::FullScreenKeyHandled (MoonKeyEvent *key)
{
	if (!GetFullScreen ())
		return false;
		
	// If we're in fullscreen mode no key events are passed through.
	// We only handle Esc, to exit fullscreen mode.

	switch (key->GetSilverlightKey()) {
		case KeyDOWN:
		case KeyUP:
		case KeyLEFT:
		case KeyRIGHT:
		case KeySPACE:
		case KeyTAB:
		case KeyPAGEDOWN:
		case KeyPAGEUP:
		case KeyHOME:
		case KeyEND:
		case KeyENTER:
			return false;
			
		// Explicitly listing KeyESCAPE here as it should never bubble up
		case KeyESCAPE:
			SetFullScreen (false);
			// fall through here
		default:
			return true;
	}
}

gboolean
Surface::HandleUIFocusIn (MoonFocusEvent *event)
{
	if (IsZombie ())
		return false;

	time_manager->InvokeTickCalls();

	if (GetFocusedElement ()) {
		List *focus_to_root = ElementPathToRoot (GetFocusedElement ());
		EmitEventOnList (UIElement::GotFocusEvent, focus_to_root, event, -1);
		delete focus_to_root;
	}

	return false;
}

gboolean
Surface::HandleUIFocusOut (MoonFocusEvent *event)
{
	if (IsZombie ())
		return false;

	time_manager->InvokeTickCalls();

	if (GetFocusedElement ()) {
		List *focus_to_root = ElementPathToRoot (GetFocusedElement ());
		EmitEventOnList (UIElement::LostFocusEvent, focus_to_root, event, -1);
		delete focus_to_root;
	}

	return false;
}

gboolean
Surface::HandleUIButtonRelease (MoonButtonEvent *event)
{
	time_manager->InvokeTickCalls();

	if (event->GetButton() != 1 && event->GetButton() != 3)
		return false;

	SetUserInitiatedEvent (true);
	
	delete mouse_event;
	
	mouse_event = (MoonMouseEvent*)event->Clone ();

	HandleMouseEvent ((event->GetButton () == 1
			   ? UIElement::MouseLeftButtonUpEvent
			   : UIElement::MouseRightButtonUpEvent),
			  true, true, true, mouse_event);

	UpdateCursorFromInputList ();
	SetUserInitiatedEvent (false);

	// XXX MS appears to do this here, which is completely stupid.
	if (captured)
		PerformReleaseCapture ();

	return !((moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) == 0 && event->GetButton () == 3);
}

gboolean
Surface::HandleUIButtonPress (MoonButtonEvent *event)
{
	bool handled;
	int event_id;
	
	active_window->GrabFocus ();

	time_manager->InvokeTickCalls();
	
	if (event->GetButton () != 1 && event->GetButton() != 3)
		return false;

	SetUserInitiatedEvent (true);

	delete mouse_event;
	
	mouse_event = (MoonMouseEvent*)event->Clone ();
	
	switch (event->GetNumberOfClicks ()) {
	case 3:
	case 2:
		if (event->GetButton() != 1) {
			SetUserInitiatedEvent (false);
			return false;
		}
		
		handled = HandleMouseEvent (UIElement::MouseLeftButtonMultiClickEvent, false, false, true, mouse_event);
		break;
	default:
		g_warning ("unhandled number of button clicks %d, treating as a single click",
			   event->GetNumberOfClicks ());
		// fallthrough
	case 1:
		if (event->GetButton() == 1)
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
Surface::HandleUIScroll (MoonScrollWheelEvent *event)
{
	time_manager->InvokeTickCalls();

	delete mouse_event;
	mouse_event = (MoonMouseEvent*)event->Clone();

	bool handled = false;

	handled = HandleMouseEvent (UIElement::MouseWheelEvent, true, true, true, mouse_event);

	UpdateCursorFromInputList ();

	return handled;
}

gboolean
Surface::HandleUIMotion (MoonMotionEvent *event)
{
	time_manager->InvokeTickCalls();

	delete mouse_event;
	mouse_event = (MoonMouseEvent*)event->Clone ();

	bool handled = HandleMouseEvent (UIElement::MouseMoveEvent, true, true, true, mouse_event);
	UpdateCursorFromInputList ();

	return handled;
}

gboolean
Surface::HandleUICrossing (MoonCrossingEvent *event)
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

	if (event->IsEnter ()) {
		delete mouse_event;

		mouse_event = (MoonMouseEvent*)event->Clone ();
		
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
Surface::EmitFocusChangeEvents()
{
	while (FocusChangedNode *node = (FocusChangedNode *) focus_changed_events->First ()) {
		if (node->lost_focus)
			node->lost_focus->EmitLostFocus ();
		if (node->got_focus)
			node->got_focus->EmitGotFocus ();
		focus_changed_events->Remove (node);
	}
}

bool
Surface::FocusElement (UIElement *focused)
{
	if (focused == focused_element)
		return true;

	while (focused_element) {
		focus_changed_events->Append (new FocusChangedNode (focused_element, NULL));
		focused_element = focused_element->GetVisualParent ();
	}

	focused_element = focused;

	while (focused) {
		focus_changed_events->Append (new FocusChangedNode (NULL, focused));
		focused = focused->GetVisualParent ();
	}

	if (FirstUserInitiatedEvent ())
		AddTickCall (Surface::EmitFocusChangeEventsAsync);
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
Surface::HandleUIKeyPress (MoonKeyEvent *event)
{
	time_manager->InvokeTickCalls();

	Key key = event->GetSilverlightKey ();

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
	if (Key == KeyD) {
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
		handled = EmitEventOnList (UIElement::KeyDownEvent, focus_to_root, event, -1);
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
Surface::HandleUIKeyRelease (MoonKeyEvent *event)
{
	time_manager->InvokeTickCalls();

	if (FullScreenKeyHandled (event))
		return true;

	SetUserInitiatedEvent (true);
	bool handled = false;

	Key key = event->GetSilverlightKey();
	Keyboard::OnKeyRelease (key);

	if (focused_element) {
		List *focus_to_root = ElementPathToRoot (focused_element);
		handled = EmitEventOnList (UIElement::KeyUpEvent, focus_to_root, event, -1);
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
	/* we support all 0.*, 1.0.*, 1.1.*, 2.0.*, 3.0.*, and 4.0.* versions. */
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
		case 4:
			supported &= numbers [1] == 0; // 4.0.*
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

static guint32
get_debug_options (const char *envname, MoonlightDebugOption options[])
{
	gint32 flags = 0;
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
				
				if (!strncmp (options[i].name, flag, n))
					flags |= options[i].flag;
			}

			while (*inptr == ',')
				inptr++;

			flag = inptr;
		}
	}

	return flags;
}

static RuntimeInitFlag
get_runtime_options (RuntimeInitFlag def)
{
	RuntimeInitFlag flags = def;
	const char *env;

	if ((env = g_getenv ("MOONLIGHT_OVERRIDES"))) {
		printf ("MOONLIGHT_OVERRIDES = %s\n", env);

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

			const char *equal = flag;

			while (equal < inptr && *equal != '=')
				equal++;
			
			n = (equal - flag);

			bool recognized = false;

			for (i = 0; options[i].name != NULL; i++) {

				if (strncmp (options[i].name, flag, n))
					continue;

				size_t v = (inptr - equal - 1);

				if (options[i].enable_value && !strncmp (options[i].enable_value, equal + 1, v)) {
					flags = (RuntimeInitFlag)(flags | options[i].flag);
					recognized = true;
					break;
				}
				else if (options[i].disable_value && !strncmp (options[i].disable_value, equal + 1, v)) {
					flags = (RuntimeInitFlag)(flags & ~options[i].flag);
					recognized = true;
					break;
				}
				else {
					char *value = g_strndup (equal + 1, v);
					g_warning ("unrecognized value for MOONLIGHT_OVERRIDE setting %s: %s.",
						   options[i].name, value);
					g_free (value);
					g_warning ("valid values are:");
					if (options[i].enable_value)
						g_warning ("   %s", options[i].enable_value);
					if (options[i].disable_value)
						g_warning ("   %s", options[i].disable_value);
				}
			}

			if (!recognized) {
				char *s = g_strndup (flag, inptr - flag);
				g_warning ("unrecognized MOONLIGHT_OVERRIDE setting %s", s);
				g_free (s);
			}

			while (*inptr == ',')
				inptr++;
			
			flag = inptr;
		}
	}
	return flags;
}

void
runtime_init (const char *platform_dir, RuntimeInitFlag flags)
{
	if (inited)
		return;

	if (cairo_version () < CAIRO_VERSION_ENCODE(1,4,0)) {
		printf ("*** WARNING ***\n");
		printf ("*** Cairo versions < 1.4.0 should not be used for Moon.\n");
		printf ("*** Moon is being run against version %s.\n", cairo_version_string ());
		printf ("*** Proceed at your own risk\n");
	}

	// Allow the user to override the flags via his/her environment
	flags = get_runtime_options (flags);
#if DEBUG || LOGGING
	debug_flags_ex = get_debug_options ("MOONLIGHT_DEBUG", debug_extras);
	debug_flags = get_debug_options ("MOONLIGHT_DEBUG", debugs);
#endif
	
	inited = true;

	if (!g_type_inited) {
		g_type_inited = true;
		g_type_init ();
	}
	
	moonlight_flags = flags;

	// FIXME add some ifdefs + runtime checks here
#if PAL_GTK_WINDOWING
	windowing_system = new MoonWindowingSystemGtk ();
	installer_service = new MoonInstallerServiceGtk ();
#else
#error "no PAL windowing system defined"
#endif

#if PAL_GLIB_MESSAGING
	messaging_service = new MoonMessagingServiceGlib ();
#else
#error "no PAL messaging service defined"
#endif

#if PAL_LINUX_CAPTURE
	capture_service = new MoonCaptureServiceLinux ();
#else
#error "no PAL capture service defined"
#endif

#if PAL_DBUS_NETWORKAVAILABILITY
	network_service = new MoonNetworkServiceDbus ();
#else
#error "no PAL network availability service defined"
#endif

	Deployment::Initialize (platform_dir, (flags & RUNTIME_INIT_CREATE_ROOT_DOMAIN) != 0);

	xaml_init ();
	Media::Initialize ();
	Effect::Initialize ();
}

MoonWindowingSystem *
runtime_get_windowing_system ()
{
	return windowing_system;
}

MoonInstallerService *
runtime_get_installer_service ()
{
	return installer_service;
}

MoonMessagingService *
runtime_get_messaging_service ()
{
	return messaging_service;
}

MoonCaptureService *
runtime_get_capture_service ()
{
	return capture_service;
}

MoonNetworkService *
runtime_get_network_service ()
{
	return network_service;
}

void
runtime_shutdown (void)
{
	if (!inited)
		return;

	Media::Shutdown ();
	Effect::Shutdown ();
	
	inited = false;

	delete windowing_system;
	windowing_system = NULL;

	delete messaging_service;
	messaging_service = NULL;
}

void
g_free_pinvoke (void *obj)
{
	g_free (obj);
}
