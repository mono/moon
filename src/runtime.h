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

#include <gtk/gtkwidget.h>
#include <cairo.h>

#include "moonbuild.h"
#include "point.h"
#include "uielement.h"
#include "dependencyobject.h"
#include "dirty.h"
#include "value.h"
#include "type.h"
#include "list.h"
#include "error.h"
#include "window.h"

#define MAXIMUM_CACHE_SIZE 6000000

#define FRONT_TO_BACK_STATS 0

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

enum RuntimeInitFlags {
	RUNTIME_INIT_PANGO_TEXT_LAYOUT     = 1 << 0,
	// (not used)                      = 1 << 1,
	RUNTIME_INIT_MANUAL_TIMESOURCE     = 1 << 2,
	RUNTIME_INIT_DISABLE_AUDIO         = 1 << 3,
	RUNTIME_INIT_EMULATE_KEYCODES      = 1 << 4,
	RUNTIME_INIT_SHOW_EXPOSE           = 1 << 5,
	RUNTIME_INIT_SHOW_CLIPPING         = 1 << 6,
	RUNTIME_INIT_SHOW_BOUNDING_BOXES   = 1 << 7,
	RUNTIME_INIT_SHOW_TEXTBOXES        = 1 << 8,
	RUNTIME_INIT_SHOW_FPS              = 1 << 9,
	RUNTIME_INIT_RENDER_FRONT_TO_BACK  = 1 << 10,
	RUNTIME_INIT_SHOW_CACHE_SIZE	   = 1 << 11,
	RUNTIME_INIT_FFMPEG_YUV_CONVERTER  = 1 << 12,
	RUNTIME_INIT_USE_SHAPE_CACHE	   = 1 << 13,
	RUNTIME_INIT_USE_UPDATE_POSITION   = 1 << 14,
	RUNTIME_INIT_ALLOW_WINDOWLESS      = 1 << 15,
	RUNTIME_INIT_AUDIO_ALSA_MMAP       = 1 << 16,
	RUNTIME_INIT_AUDIO_ALSA_RW         = 1 << 17,
	RUNTIME_INIT_AUDIO_ALSA            = 1 << 18,
	RUNTIME_INIT_AUDIO_PULSE           = 1 << 19,
	RUNTIME_INIT_USE_IDLE_HINT         = 1 << 20,
	RUNTIME_INIT_USE_BACKEND_XLIB      = 1 << 21,
	RUNTIME_INIT_KEEP_MEDIA            = 1 << 22,
	RUNTIME_INIT_ENABLE_MS_CODECS      = 1 << 23,
	RUNTIME_INIT_DISABLE_FFMPEG_CODECS = 1 << 24,
	RUNTIME_INIT_ALL_IMAGE_FORMATS     = 1 << 25,
	RUNTIME_INIT_CREATE_ROOT_DOMAIN    = 1 << 26,
	RUNTIME_INIT_DESKTOP_EXTENSIONS    = 1 << 27,
	RUNTIME_INIT_OUT_OF_BROWSER        = 1 << 28,
	RUNTIME_INIT_CURL_BRIDGE	       = 1 << 29
};

extern MOON_API guint32 moonlight_flags;


#if LOGGING || DEBUG
enum RuntimeDebugFlags {
	RUNTIME_DEBUG_ALSA              = 1 << 0,
	RUNTIME_DEBUG_AUDIO             = 1 << 1,
	RUNTIME_DEBUG_PULSE             = 1 << 2,
	RUNTIME_DEBUG_HTTPSTREAMING     = 1 << 3,
	RUNTIME_DEBUG_MARKERS           = 1 << 4,
	RUNTIME_DEBUG_MMS               = 1 << 5,
	RUNTIME_DEBUG_MEDIAPLAYER       = 1 << 6,
	RUNTIME_DEBUG_PIPELINE          = 1 << 7,
	RUNTIME_DEBUG_PIPELINE_ERROR    = 1 << 8,
	RUNTIME_DEBUG_FRAMEREADERLOOP   = 1 << 9,
	RUNTIME_DEBUG_FFMPEG            = 1 << 10,
	RUNTIME_DEBUG_UI                = 1 << 11,
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
};

enum RuntimeDebugFlagsExtra {
	RUNTIME_DEBUG_ALSA_EX           = 1 << 0,
	RUNTIME_DEBUG_AUDIO_EX          = 1 << 1,
	RUNTIME_DEBUG_PULSE_EX          = 1 << 2,
	RUNTIME_DEBUG_MARKERS_EX        = 1 << 3,
	RUNTIME_DEBUG_MEDIAPLAYER_EX    = 1 << 4,
	RUNTIME_DEBUG_MEDIAELEMENT_EX   = 1 << 5,
	RUNTIME_DEBUG_PLAYLIST_EX       = 1 << 6,
	RUNTIME_DEBUG_PIPELINE_EX       = 1 << 7,
};

extern MOON_API guint32 debug_flags_ex;
extern MOON_API guint32 debug_flags;
#endif


class TimeManager;
class Surface;
class Downloader;

typedef void (* MoonlightFPSReportFunc) (Surface *surface, int nframes, float nsecs, void *user_data);
typedef void (* MoonlightCacheReportFunc) (Surface *surface, long size, void *user_data);
typedef bool (* MoonlightEventEmitFunc) (UIElement *element, GdkEvent *event);
typedef void (* MoonlightExposeHandoffFunc) (Surface *surface, TimeSpan time, void *user_data);

/* @Namespace=None,ManagedEvents=Manual */
class MOON_API Surface : public EventObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	Surface (MoonWindow *window);
	virtual void Dispose ();
	
	/* @GenerateCBinding */
	MoonWindow *GetWindow () { return active_window; }
	MoonWindow *DetachWindow ();
	
	// allows you to redirect painting of the surface to an
	// arbitrary cairo context.
	void Paint (cairo_t *ctx, Region *region);
	/* @GenerateCBinding,GeneratePInvoke */
	void Paint (cairo_t *ctx, int x, int y, int width, int height);

	/* @GenerateCBinding,GeneratePInvoke */
	void Attach (UIElement *toplevel);

	void AttachLayer (UIElement *layer);

	void DetachLayer (UIElement *layer);
	
	void SetCursor (MouseCursor cursor);

	void ReleaseMouseCapture (UIElement *capture);
	bool SetMouseCapture (UIElement *capture);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Resize (int width, int height);

	void EmitSourceDownloadComplete ();
	void EmitSourceDownloadProgressChanged (DownloadProgressEventArgs *args);
	void EmitError (ErrorEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void EmitError (int number, int code, const char *message);
	
	void EmitLoad ();
	
	void SetBackgroundColor (Color *color);
	/* @GenerateCBinding,GeneratePInvoke */
	Color *GetBackgroundColor ();

	int GetFrameCount () { return frames; }
	void ResetFrameCount () { frames = 0; }

	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

	/* @GenerateCBinding,GeneratePInvoke */
	UIElement *GetToplevel() { return toplevel; }
	bool IsTopLevel (UIElement *top);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	UIElement *GetFocusedElement () { return focused_element; }

	bool FocusElement (UIElement *element);

	/* @GenerateCBinding,GeneratePInvoke */
	bool IsLoaded () { return toplevel != NULL && ticked_after_attach; }

	/* @GenerateCBinding,GeneratePInvoke */
	static bool IsVersionSupported (const char *version);

	const static int ResizeEvent;
	const static int FullScreenChangeEvent;
	const static int ErrorEvent;
	const static int LoadEvent;
	const static int SourceDownloadProgressChangedEvent;
	const static int SourceDownloadCompleteEvent;
	const static int ZoomedEvent;
	
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	bool GetFullScreen () { return full_screen; }
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	void SetFullScreen (bool value);
	
	/* @GenerateCBinding,GeneratePInvoke,Version=3.0 */
	double GetZoomFactor () { return zoom_factor; }
	void SetZoomFactor (double value);
	
	void SetUserInitiatedEvent (bool value);
	
	bool FirstUserInitiatedEvent () { return first_user_initiated_event; }
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	bool IsUserInitiatedEvent () { return user_initiated_event; }
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	int GetUserInitiatedCounter () { return user_initiated_monotonic_counter; }

	const char* GetSourceLocation ();
	void SetSourceLocation (const char *location);
	bool FullScreenKeyHandled (GdkEventKey *key);

	/* @GenerateCBinding,GeneratePInvoke */
	TimeManager *GetTimeManager () { return time_manager; }

	void SetDownloaderContext (gpointer context) { downloader_context = context; }
	gpointer GetDownloaderContext () { return downloader_context; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Downloader *CreateDownloader ();
	static Downloader *CreateDownloader (EventObject *obj);

	bool GetRelaxedMediaMode (void) { return relaxed_media_mode; }
	void SetRelaxedMediaMode (bool value) { relaxed_media_mode = value; }

	void SetFPSReportFunc (MoonlightFPSReportFunc report, void *user_data);
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

	void DetachDownloaders ();
	
#if FRONT_TO_BACK_STATS
	int uielements_rendered_front_to_back;
	int uielements_rendered_back_to_front;
#endif

#ifdef DEBUG
	UIElement *debug_selected_element;
#endif

	void PaintToDrawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y, bool transparent, bool clear_transparent);


	gboolean HandleUIMotion (GdkEventMotion *event);
	gboolean HandleUICrossing (GdkEventCrossing *event);
	gboolean HandleUIKeyPress (GdkEventKey *event);
	gboolean HandleUIKeyRelease (GdkEventKey *event);
	gboolean HandleUIButtonRelease (GdkEventButton *event);
	gboolean HandleUIButtonPress (GdkEventButton *event);
	gboolean HandleUIScroll (GdkEventScroll *event);
	gboolean HandleUIFocusIn (GdkEventFocus *event);
	gboolean HandleUIFocusOut (GdkEventFocus *event);
	void HandleUIWindowAllocation (bool emit_resize);
	void HandleUIWindowAvailable ();
	void HandleUIWindowUnavailable ();
	void HandleUIWindowDestroyed (MoonWindow *window);

	// bad, but these live in dirty.cpp, not runtime.cpp
	void AddDirtyElement (UIElement *element, DirtyType dirt);
	void UpdateLayout ();
	void RemoveDirtyElement (UIElement *element);
	bool ProcessDirtyElements ();
	void PropagateDirtyFlagToChildren (UIElement *element, DirtyType dirt);

	static bool main_thread_inited;
	static pthread_t main_thread;
	/* @GenerateCBinding,GeneratePInvoke */
	static bool InMainThread () { return (!main_thread_inited || pthread_equal (main_thread, pthread_self ())); }

	void ShowDrmMessage ();

protected:
	// The current window we are drawing to
	MoonWindow *active_window;
	
	virtual ~Surface();

private:
	// are we headed for death?
	bool zombie;

	// bad, but these two live in dirty.cpp, not runtime.cpp
	void ProcessDownDirtyElements ();
	void ProcessUpDirtyElements ();

	DirtyLists *down_dirty;
	DirtyLists *up_dirty;
	
	gpointer downloader_context;
	List *downloaders;
	static void OnDownloaderDestroyed (EventObject *sender, EventArgs *args, gpointer closure);
	
	Color *background_color;
	
	// This is the normal-sized window
	MoonWindow *normal_window;
	
	// We set active_window to this whenever we are in fullscreen mode.
	MoonWindow *fullscreen_window;
	
	// We can have multiple top level elements, these are stored as layers
	HitTestCollection *layers;
	
	UIElement *toplevel;

	// The element holding the keyboard focus, and the one that
	// held it previously (so we can emit lostfocus events async)
	UIElement *focused_element;
	Queue *focus_changed_events;

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
	MouseCursor cursor;

	// Fullscreen support
	bool full_screen;
	Panel *full_screen_message;
	char *source_location;
	
	// Zoom support
	double zoom_factor;
	
	Panel *incomplete_support_message;
	Panel *drm_message;
	
	// True once we have received at least one user initiated event
	bool first_user_initiated_event;
	// Should be set to true only while executing MouseLeftButtonDown, 
	// MouseLeftButtonUp, KeyDown, and KeyUp event handlers
	bool user_initiated_event;
	// some actions (like HtmlPage.PopupWindow) can only occur once 
	// per user-initiated event
	int user_initiated_monotonic_counter;

	void UpdateFullScreen (bool value);
	
	TimeManager *time_manager;
	bool ticked_after_attach;
	static void tick_after_attach_reached (EventObject *data);

	int frames;
	
	GdkEvent *mouse_event;
	
	// Relaxed mode enables local file and cross domain playback
	// and relaxes playlist parsing to better handle poorly
	// formed ASX playlists, etc. (for Moonshine, love abock)
	bool relaxed_media_mode;

	// Variables for reporting FPS
	MoonlightFPSReportFunc fps_report;
	gint64 fps_start;
	int fps_nframes;
	void *fps_data;
	
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
	
	void AutoFocus ();
	static void AutoFocusAsync (EventObject *sender);
	
	void Realloc ();

	void ShowFullScreenMessage ();
	void HideFullScreenMessage ();
	static void HideFullScreenMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);

	void HideDrmMessage ();
	static void HideDrmMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);

	void ShowIncompleteSilverlightSupportMessage ();
	void HideIncompleteSilverlightSupportMessage ();
	static void HideIncompleteSilverlightSupportMessageCallback (EventObject *sender, EventArgs *args, gpointer closure);
	
	void CreateSimilarSurface ();
	
	static void render_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void update_input_cb (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void widget_destroyed (GtkWidget *w, gpointer data);
	
	EventArgs* CreateArgsForEvent (int event_id, GdkEvent *event);

	List* ElementPathToRoot (UIElement *source);
	void GenerateFocusChangeEvents();

	void FindFirstCommonElement (List *l1, int *index1, List *l2, int *index2);
	bool EmitEventOnList (int event_id, List *element_list, GdkEvent *event, int end_idx);
	void UpdateCursorFromInputList ();
	bool HandleMouseEvent (int event_id, bool emit_leave, bool emit_enter, bool force_emit, GdkEvent *event);
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
typedef void (*RenderFunc) (cairo_t *ctx, UIElement *uielement, Region *region, bool front_to_back);

class RenderNode : public List::Node {
public:
	RenderNode (UIElement *el, Region *region, bool render_element, RenderFunc pre, RenderFunc post);
	
	void Render (cairo_t *cr);

	virtual ~RenderNode ();

private:
	UIElement *uielement;
	Region *region;
	bool render_element;
	RenderFunc pre_render;
	RenderFunc post_render;

};

G_BEGIN_DECLS

void     runtime_init (const char *platform_dir, guint32 flags);

void     runtime_init_browser (const char *plugin_dir) MOON_API;
/* @GeneratePInvoke */
void     runtime_init_desktop () MOON_API;
/* @GeneratePInvoke */
bool     runtime_is_running_out_of_browser () MOON_API;

GList   *runtime_get_surface_list (void);

void	 runtime_flags_set_manual_timesource (gboolean flag);
void	 runtime_flags_set_show_fps (gboolean flag);
void	 runtime_flags_set_use_shapecache (gboolean flag);

 void     runtime_shutdown (void) MOON_API;

G_END_DECLS

#endif
