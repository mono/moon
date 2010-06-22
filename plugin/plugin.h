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

#include "moonlight.h"

class MoonlightScriptControlObject;
class PluginXamlLoader;
class PluginInstance;
class BrowserBridge;
#if PLUGIN_SL_2_0
class Xap;
#endif

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
	/* @GenerateCBinding,GeneratePInvoke */
	void ReportException (char *msg, char *details, /* @MarshalAs=string[] */ char **stack_trace, int num_frames);
	/* @GenerateCBinding,GeneratePInvoke */
	void *Evaluate (const char *code);
	
	NPObject *GetHost ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void *GetBrowserHost () { return GetHost (); } // same as GetHost, just without bleeding NPObjects into the cbindings

	void      AddWrappedObject    (EventObject *obj, NPObject *wrapper);
	void      RemoveWrappedObject (EventObject *obj);
	NPObject *LookupWrappedObject (EventObject *obj);
	
	void      AddCleanupPointer    (gpointer p);
	void      RemoveCleanupPointer (gpointer p);
	
	void Properties ();
	
	// Property getters and setters
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetInitParams () { return this->initParams; }
	void SetInitParams (const char *value);
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetSource () { return this->source; }
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetSourceOriginal () { return this->source_original; }
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetSourceLocation () { return this->source_location; }
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetSourceLocationOriginal () { return this->source_location_original; }
	char *GetId () { return this->id; }
	
	void SetSource (const char *value);
	
	char *GetBackground ();
	bool SetBackground (const char *value);
	bool GetEnableFramerateCounter ();
	void SetEnableFramerateCounter (bool value);
	bool GetEnableRedrawRegions ();
	void SetEnableRedrawRegions (bool value);
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetEnableHtmlAccess ();
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetAllowHtmlPopupWindow ();
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetWindowless ();
	bool IsLoaded ();

	void SetMaxFrameRate (int value);
	int  GetMaxFrameRate ();
	Deployment *GetDeployment ();
	
	BrowserBridge *GetBridge () { return bridge; }
	
	MoonlightScriptControlObject *GetRootObject ();
	NPP GetInstance ();
	NPWindow *GetWindow ();
	/* @GenerateCBinding,GeneratePInvoke */
	Surface *GetSurface () { return surface; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	gint32 GetActualHeight ();
	/* @GenerateCBinding,GeneratePInvoke */
	gint32 GetActualWidth ();

	bool IsCrossDomainApplication () { return cross_domain_app; }
	static gint32 GetPluginCount ();
	
	static gboolean plugin_button_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	
	static Downloader *CreateDownloader (PluginInstance *instance);
	
#if DEBUG
	struct moon_source : List::Node {
		char *uri;
		char *filename;
		virtual ~moon_source ()
		{
			g_free (uri);
			g_free (filename);
		}
	};
	void AddSource (const char *uri, const char *filename);
	List *GetSources ();
#endif

	bool CreatePluginDeployment ();

	gpointer ManagedCreateXamlLoaderForFile (XamlLoader* loader, const char *resourceBase, const char *file);
	gpointer ManagedCreateXamlLoaderForString (XamlLoader* loader, const char *resourceBase, const char *str);

	gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb, gpointer context);
	void HtmlObjectDetachEvent (NPP instance, const char *name, gpointer listener_ptr);

 private:
#if DEBUG
	List *moon_sources;
#endif

	// Gtk controls
	bool connected_to_container;
	GtkWidget *container;  // plugin container object
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
	char *relaxedMediaModeGuid;

	char *culture;
	char *uiCulture;

	int source_size;

	bool windowless;
	bool relaxed_media_mode;
	bool cross_domain_app;
	bool default_enable_html_access;
	bool enable_html_access;
	bool default_allow_html_popup_window;
	bool allow_html_popup_window;
	bool enable_framerate_counter;
	bool loading_splash;
	bool is_splash;
	bool is_reentrant_mess;
	int maxFrameRate;

	BrowserBridge *bridge;

	GtkWidget *properties_fps_label;
	GtkWidget *properties_cache_label;

	//
	// The XAML loader, contains a handle to a MonoObject *
	//
	PluginXamlLoader *xaml_loader;
	Deployment   *deployment;
#if PLUGIN_SL_2_0
	bool LoadXAP  (const char*url, const char *fname);
	void DestroyApplication ();
#endif

	// Private methods
	void CreateWindow ();
	void UpdateSource ();
	void UpdateSourceByReference (const char *value);
	bool LoadXAML ();
	void SetPageURL ();
	char* GetPageLocation ();
	void CrossDomainApplicationCheck (const char *source);
	void RelaxedMediaModeCheck (const char *guid);
	
	void TryLoadBridge (const char *prefix);
	
	static gboolean IdleUpdateSourceByReference (gpointer data);

	static void ReportFPS (Surface *surface, int nframes, float nsecs, void *user_data);
	static void ReportCache (Surface *surface, long bytes, void *user_data);
	static void properties_dialog_response (GtkWidget *dialog, int response, PluginInstance *plugin);

	static void network_error_tickcall (EventObject *data);
	static void splashscreen_error_tickcall (EventObject *data);
	
	EVENTHANDLER (PluginInstance, AppDomainUnloadedEvent, Deployment, EventArgs);
};

extern GSList *plugin_instances;

#define NPID(x) MOON_NPN_GetStringIdentifier (x)

#define STRDUP_FROM_VARIANT(v) (g_strndup ((char *) NPVARIANT_TO_STRING (v).utf8characters, NPVARIANT_TO_STRING (v).utf8length))
#define STRLEN_FROM_VARIANT(v) ((size_t) NPVARIANT_TO_STRING (v).utf8length)

#define STREAM_NOTIFY(x) ((StreamNotify*) x)

#define STREAM_NOTIFY_DATA(x) ((StreamNotify*) x)->pdata

#define IS_NOTIFY_SOURCE(x) \
	(!x ? false : (((StreamNotify*) x)->type == StreamNotify::SOURCE))

#define IS_NOTIFY_SPLASHSOURCE(x) \
	(!x ? false : (((StreamNotify*) x)->type == StreamNotify::SPLASHSOURCE))

#define IS_NOTIFY_DOWNLOADER(x) \
	(!x ? StreamNotify::NONE : (((StreamNotify*) x)->type == StreamNotify::DOWNLOADER))

class StreamNotify
{
 public:
	enum StreamNotifyFlags {
		NONE = 0,
		SOURCE = 1,
		SPLASHSOURCE = 2,
		DOWNLOADER = 3,
	};
	
	StreamNotifyFlags type;
	void *pdata;
	
	StreamNotify () : type (NONE), pdata (NULL) {};
	StreamNotify (void *data) : type (NONE), pdata (data) {};
	StreamNotify (StreamNotifyFlags type) : type (type), pdata (NULL) {};
	StreamNotify (StreamNotifyFlags type, void *data) : type (type), pdata (data) {};
	StreamNotify (StreamNotifyFlags type, DependencyObject *dob) : type (type), pdata (dob)
	{
		if (dob)
			dob->ref ();
	}
	
	~StreamNotify ()
	{
		if (type == DOWNLOADER && pdata)
			((DependencyObject *) pdata)->unref ();
	}
};

class PluginXamlLoader : public XamlLoader
{
	PluginXamlLoader (const char *resourceBase, const char *filename, const char *str, PluginInstance *plugin, Surface *surface);
	bool InitializeLoader ();
	PluginInstance *plugin;
	bool initialized;
	bool xaml_is_managed;
	
#if PLUGIN_SL_2_0
	gpointer managed_loader;
	Xap *xap;
#endif
 public:
	virtual ~PluginXamlLoader ();
	void TryLoad (int *error);

	bool SetProperty (void *parser, Value *top_level, const char *xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value* value, void* value_data, int flags = 0);

	static PluginXamlLoader *FromFilename (const char *resourceBase, const char *filename, PluginInstance *plugin, Surface *surface)
	{
		return new PluginXamlLoader (resourceBase, filename, NULL, plugin, surface);
	}
	
	static PluginXamlLoader *FromStr (const char *resourceBase, const char *str, PluginInstance *plugin, Surface *surface)
	{
		return new PluginXamlLoader (resourceBase, NULL, str, plugin, surface);
	}
	
	bool IsManaged () { return xaml_is_managed; }

	virtual bool LoadVM ();
};

G_BEGIN_DECLS

const char *get_plugin_dir (void);

char *plugin_instance_get_id (PluginInstance *instance);

void plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
						   bool *httpnet_access, bool *script_access);

void *plugin_instance_load_url (PluginInstance *instance, char *url, gint32 *length);

PluginXamlLoader *plugin_xaml_loader_from_str (const char *str, const char *resourceBase, PluginInstance *plugin, Surface *surface);

G_END_DECLS

#endif /* MOON_PLUGIN */
