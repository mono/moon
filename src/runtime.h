/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * runtime.h: Core surface.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <cairo.h>

#include "point.h"
#include "uielement.h"
#include "dependencyobject.h"
#include "dirty.h"
#include "value.h"
#include "type.h"
#include "list.h"
#include "error.h"

#include "pal.h"
#include "mutex.h"

namespace Moonlight {

#define MAXIMUM_CACHE_SIZE 6000000

#define OCCLUSION_CULLING_STATS 0

#define TIMERS 0
#define DEBUG_MARKER_KEY 0
#if TIMERS
#define STARTTIMER(id,str) TimeSpan id##_t_start = get_now(); printf ("timing of '%s' started at %" G_GINT64_FORMAT "\n", str, id##_t_start)
#define ENDTIMER(id,str) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended at %" G_GINT64_FORMAT " (%f seconds)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 10000000)
#else
#define STARTTIMER(id,str)
#define ENDTIMER(id,str)
#endif

#if SANITY
#define VERIFY_MAIN_THREAD \
	if (!Surface::InMainThread ()) {	\
		printf ("Moonlight: This method should only be called from the main thread (%s)\n", __PRETTY_FUNCTION__);	\
		print_stack_trace (); \
	}
#else
#define VERIFY_MAIN_THREAD
#endif

enum RuntimeInitFlag {
	RUNTIME_INIT_MANUAL_TIMESOURCE     = 1 << 0,
	RUNTIME_INIT_DISABLE_AUDIO         = 1 << 1,
	RUNTIME_INIT_EMULATE_KEYCODES      = 1 << 2,
	RUNTIME_INIT_SHOW_EXPOSE           = 1 << 3,
	RUNTIME_INIT_SHOW_CLIPPING         = 1 << 4,
	RUNTIME_INIT_SHOW_BOUNDING_BOXES   = 1 << 5,
	RUNTIME_INIT_SHOW_TEXTBOXES        = 1 << 6,
	RUNTIME_INIT_SHOW_FPS              = 1 << 7,
	RUNTIME_INIT_OCCLUSION_CULLING     = 1 << 8,
	RUNTIME_INIT_SHOW_CACHE_SIZE	   = 1 << 9,
	RUNTIME_INIT_FFMPEG_YUV_CONVERTER  = 1 << 10,
	RUNTIME_INIT_USE_SHAPE_CACHE	   = 1 << 11,
	RUNTIME_INIT_USE_UPDATE_POSITION   = 1 << 12,
	RUNTIME_INIT_ALLOW_WINDOWLESS      = 1 << 13,
	RUNTIME_INIT_AUDIO_ALSA_MMAP       = 1 << 14,
	RUNTIME_INIT_AUDIO_ALSA_RW         = 1 << 15,
	RUNTIME_INIT_AUDIO_ALSA            = 1 << 16,
	RUNTIME_INIT_AUDIO_PULSE           = 1 << 17,
	RUNTIME_INIT_USE_IDLE_HINT         = 1 << 18,
	RUNTIME_INIT_KEEP_MEDIA            = 1 << 19,
	RUNTIME_INIT_ENABLE_MS_CODECS      = 1 << 20,
	RUNTIME_INIT_DISABLE_FFMPEG_CODECS = 1 << 21,
	RUNTIME_INIT_ALL_IMAGE_FORMATS     = 1 << 22,
	RUNTIME_INIT_CREATE_ROOT_DOMAIN    = 1 << 23,
	RUNTIME_INIT_DESKTOP_EXTENSIONS    = 1 << 24,
	RUNTIME_INIT_INTERMEDIATE_SURFACES = 1 << 25,
	RUNTIME_INIT_CURL_BRIDGE	   = 1 << 26,
	RUNTIME_INIT_ENABLE_TOGGLEREFS	   = 1 << 27,
	RUNTIME_INIT_OOB_LAUNCHER_FIREFOX  = 1 << 28,
	RUNTIME_INIT_HW_ACCELERATION       = 1 << 29,
};

struct MoonlightRuntimeOption {
	RuntimeInitFlag flag;
	const char *name;
	const char *enable_value;
	const char *disable_value;
	bool runtime_changeable;
	const char *description;
};

extern MOON_API guint32 moonlight_flags;

const MoonlightRuntimeOption * moonlight_get_runtime_options ();
void moonlight_set_runtime_option (RuntimeInitFlag flag, bool set);
bool moonlight_get_runtime_option (RuntimeInitFlag flag);

#if LOGGING || DEBUG
enum RuntimeDebugFlag {
	RUNTIME_DEBUG_ALSA              = 1 << 0,
	RUNTIME_DEBUG_AUDIO             = 1 << 1,
	RUNTIME_DEBUG_PULSE             = 1 << 2,
	RUNTIME_DEBUG_CURL              = 1 << 3,
	RUNTIME_DEBUG_MARKERS           = 1 << 4,
	RUNTIME_DEBUG_MMS               = 1 << 5,
	RUNTIME_DEBUG_MEDIAPLAYER       = 1 << 6,
	RUNTIME_DEBUG_PIPELINE          = 1 << 7,
	RUNTIME_DEBUG_PIPELINE_ERROR    = 1 << 8,
	RUNTIME_DEBUG_FRAMEREADERLOOP   = 1 << 9,
	RUNTIME_DEBUG_FFMPEG            = 1 << 10,
	RUNTIME_DEBUG_SEEK              = 1 << 11,
	RUNTIME_DEBUG_CODECS            = 1 << 12,
	RUNTIME_DEBUG_DP                = 1 << 13,
	RUNTIME_DEBUG_DOWNLOADER        = 1 << 14,
	RUNTIME_DEBUG_FONT              = 1 << 15,
	RUNTIME_DEBUG_LAYOUT            = 1 << 16,
	RUNTIME_DEBUG_MEDIA             = 1 << 17,
	RUNTIME_DEBUG_MEDIAELEMENT      = 1 << 18,
	RUNTIME_DEBUG_BUFFERING         = 1 << 19,
	RUNTIME_DEBUG_ASF               = 1 << 20,
	RUNTIME_DEBUG_PLAYLIST          = 1 << 21,
	RUNTIME_DEBUG_TEXT              = 1 << 22,
	RUNTIME_DEBUG_XAML              = 1 << 23,
	RUNTIME_DEBUG_DEPLOYMENT        = 1 << 24,
	RUNTIME_DEBUG_MSI               = 1 << 25,
	RUNTIME_DEBUG_MP3               = 1 << 26,
	RUNTIME_DEBUG_VALUE             = 1 << 27,
	RUNTIME_DEBUG_DEMUXERS          = 1 << 28,
	RUNTIME_DEBUG_MP4               = 1 << 29,
	RUNTIME_DEBUG_EFFECT            = 1 << 30,
	RUNTIME_DEBUG_OOB               = 1 << 31,
};

enum RuntimeDebugExtraFlag {
	RUNTIME_DEBUG_ALSA_EX           = 1 << 0,
	RUNTIME_DEBUG_AUDIO_EX          = 1 << 1,
	RUNTIME_DEBUG_PULSE_EX          = 1 << 2,
	RUNTIME_DEBUG_MARKERS_EX        = 1 << 3,
	RUNTIME_DEBUG_MEDIAPLAYER_EX    = 1 << 4,
	RUNTIME_DEBUG_MEDIAELEMENT_EX   = 1 << 5,
	RUNTIME_DEBUG_PLAYLIST_EX       = 1 << 6,
	RUNTIME_DEBUG_PIPELINE_EX       = 1 << 7,
	RUNTIME_DEBUG_MMS_EX            = 1 << 8,
};

struct MoonlightDebugOption {
	const char *name;
	guint32 flag;
};

extern MOON_API guint32 debug_flags_ex;
extern MOON_API guint32 debug_flags;

const MoonlightDebugOption * moonlight_get_debug_options ();
void moonlight_set_debug_option (guint32 flag, bool set);
bool moonlight_get_debug_option (guint32 flag);

const MoonlightDebugOption * moonlight_get_debug_ex_options ();
void moonlight_set_debug_ex_option (guint32 flag, bool set);
bool moonlight_get_debug_ex_option (guint32 flag);
#endif

class TimeManager;
class Surface;
class Downloader;

typedef void (* MoonlightFPSReportFunc) (Surface *surface, int nframes, float nsecs, void *user_data);
typedef void (* MoonlightCacheReportFunc) (Surface *surface, long size, void *user_data);
typedef void (* MoonlightExposeHandoffFunc) (Surface *surface, TimeSpan time, void *user_data);

enum MoonEventStatus {
	MoonEventNotSupported = -1,
	MoonEventNotHandled,
	MoonEventHandled
};

/* @Namespace=None,ManagedEvents=Manual */
class MOON_API Surface : public EventObject {
public:
	/* @GeneratePInvoke,SkipFactories */
	Surface (MoonWindow *window);
	
	MoonWindow *GetWindow () { return active_window; }
	MoonWindow *DetachWindow ();

	/* @GeneratePInvoke */
	MoonWindow *GetNormalWindow () { return normal_window; }
	
	// arbitrary context.
	void Paint (Context *ctx, Region *region);

	/* @GeneratePInvoke */
	void Paint (Context *ctx, int x, int y, int width, int height);

	void Paint (Context *ctx, Region *region, bool transparent, bool clear_transparent);

	/* @GeneratePInvoke */
	void Attach (UIElement *toplevel);

	void AttachLayer (UIElement *layer);

	void DetachLayer (UIElement *layer);
	
	void SetCursor (CursorType cursor);

	void ReleaseMouseCapture (UIElement *capture);
	bool SetMouseCapture (UIElement *capture);
	
	/* @GeneratePInvoke */
	void Resize (int width, int height);

	void EmitSourceDownloadComplete ();
	void EmitSourceDownloadProgressChanged (float progress);
	void EmitError (ErrorEventArgs *args);
	/* @GeneratePInvoke */
	void EmitError (DependencyObject *original_source, int number, int code, const char *message);
	
	void EmitLoad ();
	
	void SetBackgroundColor (Color *color);
	/* @GeneratePInvoke */
	Color *GetBackgroundColor ();
	
	int GetFrameCount () { return frames; }
	void ResetFrameCount () { frames = 0; }

	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

	/* @GeneratePInvoke */
	UIElement *GetToplevel() { return toplevel; }
	bool IsTopLevel (UIElement *top);

	/* @GeneratePInvoke */
	UIElement *GetFocusedElement () { return focused_element; }
	void EnsureElementFocused ();

	UIElementCollection *GetLayers () { return layers; }

	bool FocusElement (UIElement *element);

	/* @GeneratePInvoke */
	bool IsLoaded () { return toplevel != NULL && ticked_after_attach; }

	/* @GeneratePInvoke */
	static bool IsVersionSupported (const char *version);

	const static int ResizeEvent;
	const static int FullScreenChangeEvent;
	const static int ErrorEvent;
	const static int LoadEvent;
	const static int SourceDownloadProgressChangedEvent;
	const static int SourceDownloadCompleteEvent;
	const static int WindowAvailableEvent;
	const static int WindowUnavailableEvent;

	const static int ZoomedEvent;
	
	/* @GeneratePInvoke */
	bool GetFullScreen () { return full_screen; }
	/* @GeneratePInvoke */
	void SetFullScreen (bool value);

	/* @GeneratePInvoke */
	FullScreenOptions GetFullScreenOptions () { return full_screen_options; }
	/* @GeneratePInvoke */
	void SetFullScreenOptions (FullScreenOptions options);
	
	/* @GeneratePInvoke */
	double GetZoomFactor () { return zoom_factor; }
	void SetZoomFactor (double value);
	
	void SetEnableFrameRateCounter (bool value);
	bool GetEnableFrameRateCounter ();
	
	void SetEnableRedrawRegions (bool value) { enable_redraw_regions = value; }
	bool GetEnableRedrawRegions ();
	
	void SetUserInitiatedEvent (bool value);
	
	bool FirstUserInitiatedEvent () { return first_user_initiated_event; }
	/* @GeneratePInvoke */
	bool IsUserInitiatedEvent () { return user_initiated_event; }
	/* @GeneratePInvoke */
	int GetUserInitiatedCounter () { return user_initiated_monotonic_counter; }

	/* @GeneratePInvoke */
	const Uri* GetSourceLocation ();
	void SetSourceLocation (const Uri *location);
	bool FullScreenKeyHandled (MoonKeyEvent *key);

	/* @GeneratePInvoke */
	TimeManager *GetTimeManager () { return time_manager; }
	/* @GeneratePInvoke */
	TimeManager *GetTimeManagerReffed ();

	void SetCacheReportFunc (MoonlightCacheReportFunc report, void *user_data);
	void SetExposeHandoffFunc (MoonlightExposeHandoffFunc func, void *user_data);

	bool VerifyWithCacheSizeCounter (int w, int h);
	gint64 AddToCacheSizeCounter (int w, int h);
	void RemoveFromCacheSizeCounter (gint64 size);

	// called from the plugin if the surface is headed for death.
	// stops event emission (since the plugin counterparts to xaml
	// objects will be destroyed)
	void Zombify ();

	bool IsZombie () { return zombie; }

#if OCCLUSION_CULLING_STATS
	int uielements_rendered_with_occlusion_culling;
	int uielements_rendered_with_painters;
#endif

#ifdef DEBUG
	UIElement *debug_selected_element;
#endif

	MoonEventStatus HandleUIMotion (MoonMotionEvent *event);
	MoonEventStatus HandleUICrossing (MoonCrossingEvent *event);
	MoonEventStatus HandleUIKeyPress (MoonKeyEvent *event);
	MoonEventStatus HandleUIKeyRelease (MoonKeyEvent *event);
	MoonEventStatus HandleUIButtonRelease (MoonButtonEvent *event);
	MoonEventStatus HandleUIButtonPress (MoonButtonEvent *event);
	MoonEventStatus HandleUIScroll (MoonScrollWheelEvent *event);
	MoonEventStatus HandleUIFocusIn (MoonFocusEvent *event);
	MoonEventStatus HandleUIFocusOut (MoonFocusEvent *event);
	
	void HandleUIWindowAllocation (bool emit_resize);
	void HandleUIWindowAvailable ();
	void HandleUIWindowUnavailable ();
	void HandleUIWindowDestroyed (MoonWindow *window);

	// bad, but these live in dirty.cpp, not runtime.cpp
	void AddDirtyElement (UIElement *element, DirtyType dirt);
        bool UpdateLayout (MoonError *error);
	void RemoveDirtyElement (UIElement *element);
	bool ProcessDirtyElements ();
	void PropagateDirtyFlagToChildren (UIElement *element, DirtyType dirt);

	static bool main_thread_inited;
	static pthread_t main_thread;
	/* @GeneratePInvoke */
	static bool InMainThread () { return (!main_thread_inited || pthread_equal (main_thread, pthread_self ())); }

	void ShowDrmMessage ();
	void ShowJpegMessage ();

	guint32 GetRuntimeOptions ();
	void SetRuntimeOptions (guint32 flags);
	void SetRuntimeOption (RuntimeInitFlag flag, bool value);
	bool GetRuntimeOption (RuntimeInitFlag flag);

	void AddGPUSurface (gint64 size);
	void RemoveGPUSurface (gint64 size);

	const static void *LayersWeakRef;
	const static void *ToplevelWeakRef;

protected:
	// The current window we are drawing to
	MoonWindow *active_window;
	
	virtual ~Surface();

private:
	guint32 surface_flags;

	// are we headed for death?
	bool zombie;

	void PaintBackground (Context *ctx, Region *region, bool transparent, bool clear_transparent);

	// bad, but these two live in dirty.cpp, not runtime.cpp
	void ProcessDownDirtyElements ();
	void ProcessUpDirtyElements ();

	DirtyLists *down_dirty;
	DirtyLists *up_dirty;
	
	Color *background_color;
	
	// This is the normal-sized window
	MoonWindow *normal_window;
	
	// We set active_window to this whenever we are in fullscreen mode.
	MoonWindow *fullscreen_window;
	
	// We can have multiple top level elements, these are stored as layers
	WeakRef<UIElementCollection> layers;
	
	WeakRef<UIElement> toplevel;

	// The element holding the keyboard focus, and the one that
	// held it previously (so we can emit lostfocus events async)
	UIElement *focused_element;
	List *focus_changed_events;

	// the list of elements (from most deeply nested to the
	// toplevel) we've most recently sent a mouse event to.
	List *input_list;
	
	// is the mouse captured?  if it is, it'll be by the first element in input_list.
	UIElement *captured;
	UIElement *pendingCapture;
	bool pendingReleaseCapture;
	
	// are we currently emitting a mouse event?
	bool emittingMouseEvent;
	
	// the currently shown cursor
	CursorType cursor;

	// Fullscreen support
	bool full_screen;
	FullScreenOptions full_screen_options;
	bool full_screen_options_consented;
	Panel *full_screen_message;
	Uri *source_location;
	
	// Zoom support
	double zoom_factor;
	
	Panel *incomplete_support_message;
	Panel *drm_message;
	Panel *jpeg_message;
	
	// True once we have received at least one user initiated event
	bool first_user_initiated_event;
	// Should be set to true only while executing MouseLeftButtonDown, 
	// MouseLeftButtonUp, KeyDown, and KeyUp event handlers
	bool user_initiated_event;
	// some actions (like HtmlPage.PopupWindow) can only occur once 
	// per user-initiated event
	int user_initiated_monotonic_counter;

	bool enable_redraw_regions;
	
	void UpdateFullScreen (bool value);
	
	TimeManager *time_manager;
	Mutex time_manager_mutex;
	bool ticked_after_attach;
	static void tick_after_attach_reached (EventObject *data);

	int frames;
	
	MoonMouseEvent *mouse_event;
	
	// Variables for reporting FPS
	Panel *framerate_counter_display;
	TextBlock *framerate_textblock;
	TextBlock *videomemoryused_textblock;
	TextBlock *gpuenabledsurfaces_textblock;
	TextBlock *intermediatesurfaces_textblock;
	bool enable_fps_counter;
	gint64 fps_start;
	int fps_nframes;
	gint64 vmem_used;
	int gpu_surfaces;
	
	// Variables for reporting cache size
	MoonlightCacheReportFunc cache_report;
	gint64 cache_size_in_bytes;
	int cache_size_ticker;
	void *cache_data;
	int cache_size_multiplier;

	// Expose handoff
	TimeSpan expose_handoff_last_timespan;
	MoonlightExposeHandoffFunc expose_handoff;
	void *expose_handoff_data;
	
	void Realloc ();

	void ShowFrameRateCounter ();
	void HideFrameRateCounter ();
	void UpdateFrameRateCounter (gint64 now);

	void ShowFullScreenMessage ();
	void HideFullScreenMessage ();
	static void HideFullScreenMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);

	void HideDrmMessage ();
	static void HideDrmMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);

	void HideJpegMessage ();
	static void HideJpegMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);

	void ShowIncompleteSilverlightSupportMessage ();
	void HideIncompleteSilverlightSupportMessage ();
	static void HideIncompleteSilverlightSupportMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);
	
	void CreateSimilarSurface ();
	
	static void render_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void update_input_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	EventArgs* CreateArgsForEvent (int event_id, MoonEvent *event);

	List* ElementPathToRoot (UIElement *source);
	void EmitFocusChangeEvents();
	static void EmitFocusChangeEventsAsync (EventObject *sender);

	void FindFirstCommonElement (List *l1, int *index1, List *l2, int *index2);
	bool EmitEventOnList (int event_id, List *element_list, MoonEvent *event, int end_idx);
	void UpdateCursorFromInputList ();
	bool HandleMouseEvent (int event_id, bool emit_leave, bool emit_enter, bool force_emit, MoonMouseEvent *event);
	void PerformCapture (UIElement *capture);
	void PerformReleaseCapture ();

	static void toplevel_loaded (EventObject *sender, EventArgs *args, gpointer closure);
	void ToplevelLoaded (UIElement *element);
};

/* for hit testing */
class UIElementNode : public List::Node {
public:
	UIElement *uielement;
		
	UIElementNode (UIElement *el);
	virtual ~UIElementNode ();
};

/* for rendering */
typedef void (*RenderFunc) (Context *ctx, UIElement *uielement, Region *region, bool skip_children);

class RenderNode : public List::Node {
public:
	RenderNode (UIElement *el, Region *region, bool render_element, RenderFunc pre, RenderFunc post);
	
	void Render (Context *ctx);

	virtual ~RenderNode ();

	UIElement *uielement;
	Region *region;
	bool render_element;
	RenderFunc pre_render;
	RenderFunc post_render;
};

class MOON_API Runtime {
public:
	static void Init (const char *platform_dir, RuntimeInitFlag flags, bool out_of_browser);
	static void InitBrowser (const char *plugin_dir, bool out_of_browser);
	/* @GeneratePInvoke */
	static void InitDesktop ();

	static void Shutdown ();

	static GList *GetSurfaceList ();

	static void SetManualTimeSource (gboolean flag);
	static void SetShowFps (gboolean flag);
	static void SetUseShapeCache (gboolean flag);

	/* @GeneratePInvoke */
	static MoonWindowingSystem *GetWindowingSystem ();
	static MoonInstallerService *GetInstallerService ();
	static MoonMessagingService *GetMessagingService ();
	/* @GeneratePInvoke */
	static MoonCaptureService *GetCaptureService ();
	/* @GeneratePInvoke */
	static MoonNetworkService *GetNetworkService ();
	/* @GeneratePInvoke */
	static void GFree (void *ptr);
};

};
#endif
