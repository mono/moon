/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin.h: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef MOON_PLUGIN
#define MOON_PLUGIN

#include "pal.h"
#include "plugin-accessibility-bridge.h"
#include "moonlight.h"

namespace Moonlight {

class MoonlightScriptControlObject;
class PluginXamlLoader;
class PluginInstance;
class BrowserBridge;
class Xap;

char *NPN_strdup (const char *val);
typedef void callback_dom_event (gpointer context, char *name, int client_x, int client_y, int offset_x, int offset_y, gboolean alt_key,
				 gboolean ctrl_key, gboolean shift_key, int mouse_button,
				 int key_code, int char_code,
				 gpointer domEvent);

class PluginInstance
{
 private:
	~PluginInstance ();
	
	void Recreate (const char *source);	
 public:
	PluginInstance (NPP instance, guint16 mode);
	
	void ref ();
	void unref ();

	void Initialize (int argc, char * argn[], char * argv[]);
	void Shutdown ();

#if PAL_GTK_A11Y
	AtkObject* GetRootAccessible ();
#endif

	/* @GeneratePInvoke */
	AccessibilityBridge* GetAccessibilityBridge ();

	// Mozilla plugin related methods
	NPError GetValue (NPPVariable variable, void *result);
	NPError SetValue (NPNVariable variable, void *value);
	NPError SetWindow (NPWindow *window);
	NPError NewStream (NPMIMEType type, NPStream *stream, NPBool seekable, guint16 *stype);
	NPError DestroyStream (NPStream *stream, NPError reason);
	void StreamAsFile (NPStream *stream, const char *fname);
	gint32 WriteReady (NPStream *stream);
	gint32 Write (NPStream *stream, gint32 offset, gint32 len, void *buffer);
	void UrlNotify (const char *url, NPReason reason, void *notifyData);
	bool LoadSplash ();
	void FlushSplash ();
	void Print (NPPrint *platformPrint);
	int16_t EventHandle (void *event);
	/* @GeneratePInvoke */
	void *Evaluate (const char *code);
	
	NPObject *GetHost ();
	
	/* @GeneratePInvoke */
	void *GetBrowserHost () { return GetHost (); } // same as GetHost, just without bleeding NPObjects into the cbindings

	void      AddWrappedObject    (EventObject *obj, NPObject *wrapper);
	void      RemoveWrappedObject (EventObject *obj);
	NPObject *LookupWrappedObject (EventObject *obj);
	
	void      AddCleanupPointer    (gpointer p);
	void      RemoveCleanupPointer (gpointer p);
	
	void Properties ();
	
	// Property getters and setters
	/* @GeneratePInvoke */
	const char *GetInitParams () { return this->initParams; }
	void SetInitParams (const char *value);
	/* @GeneratePInvoke */
	const char *GetSource () { return this->source; }
	/* @GeneratePInvoke */
	const char *GetSourceOriginal () { return this->source_original; }
	/* @GeneratePInvoke */
	const char *GetSourceLocation () { return this->source_location; }
	/* @GeneratePInvoke */
	const char *GetSourceLocationOriginal () { return this->source_location_original; }
	char *GetId () { return this->id; }
	
	void SetSource (const char *value);
	
	char *GetBackground ();
	bool SetBackground (const char *value);
	/* @GeneratePInvoke */
	bool GetEnableFrameRateCounter ();
	/* @GeneratePInvoke */
	void SetEnableFrameRateCounter (bool value);
	/* @GeneratePInvoke */
	bool GetEnableRedrawRegions ();
	/* @GeneratePInvoke */
	void SetEnableRedrawRegions (bool value);
	/* @GeneratePInvoke */
	bool GetEnableHtmlAccess ();
	/* @GeneratePInvoke */
	bool GetEnableNavigation ();
	/* @GeneratePInvoke */
	bool GetAllowHtmlPopupWindow ();
	/* @GeneratePInvoke */
	bool GetWindowless ();
	bool GetEnableGpuAcceleration ();
	bool IsLoaded ();
	
	/* @GeneratePInvoke */
	void SetMaxFrameRate (int value);
	/* @GeneratePInvoke */
	int  GetMaxFrameRate ();
	
	Deployment *GetDeployment ();
	
	BrowserBridge *GetBridge () { return bridge; }
	
	MoonlightScriptControlObject *GetRootObject ();
	NPP GetInstance ();
	NPWindow *GetWindow ();
	/* @GeneratePInvoke */
	void *GetNPWindow () { return GetWindow (); }
	/* @GeneratePInvoke */
	Surface *GetSurface () { return surface; }
	
	/* @GeneratePInvoke */
	gint32 GetActualHeight ();
	/* @GeneratePInvoke */
	gint32 GetActualWidth ();

	bool IsCrossDomainApplication () { return cross_domain_app; }
	static gint32 GetPluginCount ();
	
	Downloader *CreateDownloader ();
	
	bool CreatePluginDeployment ();

	GCHandle CreateManagedXamlLoader (XamlLoader* loader, const Uri *resourceBase);
	static void progress_changed_handler (EventObject *sender, EventArgs *args, gpointer closure);
	int progress_changed_token;

	static void SourceProgressChangedHandler (EventObject *obj, EventArgs *args, gpointer closure);
	static void SourceStoppedHandler (EventObject *obj, EventArgs *args, gpointer closure);
	void SourceProgressChanged (HttpRequest *request, HttpRequestProgressChangedEventArgs *args);
	void SourceStopped (HttpRequest *request, HttpRequestStoppedEventArgs *args);

	static void SplashProgressChangedHandler (EventObject *obj, EventArgs *args, gpointer closure);
	static void SplashStoppedHandler (EventObject *obj, EventArgs *args, gpointer closure);
	void SplashProgressChanged (HttpRequest *request, HttpRequestProgressChangedEventArgs *args);
	void SplashStopped (HttpRequest *request, HttpRequestStoppedEventArgs *args);

 private:
	// Gtk controls
	bool connected_to_container;
 	Surface *surface;      // plugin surface object
	MoonWindow *moon_window;

	GSList *timers;

  	guint16 mode;          // NP_EMBED, NP_FULL, or NP_BACKGROUND
	NPWindow *window;      // Mozilla window object
	NPP instance;          // Mozilla instance object
	MoonlightScriptControlObject *rootobject;  // Mozilla jscript object wrapper
	guint32 xembed_supported; // XEmbed Extension supported

	GHashTable *wrapped_objects; // wrapped object cache

	/*
	 * Mozilla has a slightly different view on refcounting when dealing with
	 * NPObjects: normal refcounting until NPP_Destroy is called, after
	 * NPP_Destroy returns, all NPObjects are deleted no matter their refcount.
	 *
	 * See this link for some fun reading:
	 * - https://bugzilla.mozilla.org/show_bug.cgi?id=421217	 
	 * 
	 * Apparently this behaviour is documented here: http://developer.mozilla.org/en/NPClass
	 * - The NPObjects' invalidate method is:
	 *   "Called on live objects that belong to a plugin instance that is being destroyed."
	 *   "This call is always followed by a call to the deallocate function called when"
	 *   "the plugin is destroyed"
	 * 
	 * However the documentation also says this about the deallocate method:
	 *  "Called by NPN_ReleaseObject() when an object's reference count reaches zero."
	 *
	 * This contradicting documentation results in different behaviour between browsers,
	 * Safari and Opera comply 100% with refcounting laws, while firefox doesn't.
	 *
	 * This has a profound implication on for our shutdown code: it means that
	 * we can't access any NPObjects after returning from NPP_Destroy. Parts of
	 * our current shutdown is async (it wouldn't be completely impossible to
	 * make it sync, just dangerously difficult, we'd have to block the main thread
	 * until all other threads have shut down, while executing code we have little
	 * control over, exposing us to possible deadlocks).
	 * 
	 * To protect against accessing NPObjects after returning from NPP_Destroy
	 * we keep a flag telling whether it's safe or not to access npobjects (which
	 * is set upon shutdown)
	 *
	 */

public:	
	bool IsShuttingDown (); /* Not thread-safe */
	bool HasShutdown (); /* It is not safe to access any NPObjects when this returns true. Not thread-safe. */

private:
	bool is_shutting_down;
	bool has_shutdown;
	gint32 refcount;
	
	GSList *cleanup_pointers;

	// Property fields
	// If you add new property fields remember to handle them properly in Recreate too.
	char *initParams;
	char *source;
	char *source_original;
	char *source_location;
	char *source_location_original;
	guint source_idle;
	char *onLoad;
	char *background;
	char *onError;
	char *onResize;
	char *id;
	char *splashscreensource;
	char *onSourceDownloadProgressChanged;
	char *onSourceDownloadComplete;

	char *culture;
	char *uiCulture;

	int source_size;

	bool windowless;
	bool cross_domain_app;
	bool default_enable_html_access;
	bool enable_html_access;
	bool default_allow_html_popup_window;
	bool allow_html_popup_window;
	bool enable_framerate_counter;
	bool enable_redraw_regions;
	bool enable_navigation;
	bool loading_splash;
	bool is_splash;
	bool is_reentrant_mess;
	bool enable_gpu_acceleration;
	int maxFrameRate;

	BrowserBridge *bridge;

	// The directory where we place all downloaded files
	char *download_dir;

	//
	// The XAML loader, contains a handle to a MonoObject *
	//
	PluginXamlLoader *xaml_loader;
	Deployment   *deployment;
	bool LoadXAP  (const Uri *url, const char *fname);
	void DestroyApplication ();

	// Private methods
	void CreateWindow ();
	void UpdateSource ();
	void UpdateSourceByReference (const char *value);
	bool LoadXAML ();
	void SetPageURL ();
	char* GetPageLocation ();
	void CrossDomainApplicationCheck (const char *source);
	
	void TryLoadBridge (const char *prefix);

	static bool IdleUpdateSourceByReference (gpointer data);

	static void ReportFPS (Surface *surface, int nframes, float nsecs, void *user_data);
	static void ReportCache (Surface *surface, long bytes, void *user_data);

	static void network_error_tickcall (EventObject *data);
	static void splashscreen_error_tickcall (EventObject *data);
	
	EVENTHANDLER (PluginInstance, AppDomainUnloadedEvent, Deployment, EventArgs);

	AccessibilityBridge *accessibility_bridge;
};

extern GSList *plugin_instances;

#define NPID(x) MOON_NPN_GetStringIdentifier (x)

#define STRDUP_FROM_VARIANT(v) (g_strndup ((char *) NPVARIANT_TO_STRING (v).utf8characters, NPVARIANT_TO_STRING (v).utf8length))
#define STRLEN_FROM_VARIANT(v) ((size_t) NPVARIANT_TO_STRING (v).utf8length)

class PluginXamlLoader : public SL3XamlLoader
{
	PluginXamlLoader (const Uri *resourceBase, PluginInstance *plugin, Surface *surface);

	bool InitializeLoader ();
	PluginInstance *plugin;
	bool initialized;

	char* xaml_string;
	char* xaml_file;

	GCHandle managed_loader;
 public:
	virtual ~PluginXamlLoader ();
	void TryLoad (int *error);

	bool SetProperty (void *parser, Value *top_level, const char *xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value* value, void* value_data, int flags = 0);

	const char* GetXamlString ()
	{
		return xaml_string;
	}

	const char* GetXamlFile ()
	{
		return xaml_file;
	}
	
	static PluginXamlLoader *FromFilename (const Uri *resourceBase, const char *filename, PluginInstance *plugin, Surface *surface)
	{
		PluginXamlLoader *loader = new PluginXamlLoader (resourceBase, plugin, surface);

		loader->xaml_file = g_strdup (filename);
		return loader;
	}
	
	static PluginXamlLoader *FromStr (const Uri *resourceBase, const char *str, PluginInstance *plugin, Surface *surface)
	{
		PluginXamlLoader *loader = new PluginXamlLoader (resourceBase, plugin, surface);

		loader->xaml_string = g_strdup (str);
		return loader;
	}
	
	virtual bool LoadVM ();
};

G_BEGIN_DECLS

const char *get_plugin_dir (void);

char *plugin_instance_get_id (PluginInstance *instance);

void plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
						   bool *httpnet_access, bool *script_access);

void *plugin_instance_load_url (PluginInstance *instance, char *url, gint32 *length);

PluginXamlLoader *plugin_xaml_loader_from_str (const char *str, const Uri *resourceBase, PluginInstance *plugin, Surface *surface);

G_END_DECLS

};
#endif /* MOON_PLUGIN */
