/*
 * moon-plugin.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *   Michael Dominic K. <mdk@mdk.am>
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

typedef void plugin_unload_callback (PluginInstance *plugin);

class PluginInstance
{
 private:
  	uint16_t mode;         // NP_EMBED, NP_FULL, or NP_BACKGROUND
	NPWindow *window;      // Mozilla window object
	NPP instance;          // Mozilla instance object
	NPObject *rootobject;  // Mozilla jscript object wrapper
	bool xembed_supported; // XEmbed Extension supported

	GHashTable *wrapped_objects; // wrapped object cache

	// Property fields
	char *initParams;
	bool isLoaded;
	char *source;
	guint source_idle;
	char *onLoad;
	char *background;
	char *onError;

	bool windowless;
	int maxFrameRate;

	GtkWidget *properties_fps_label;
	GtkWidget *properties_cache_label;

	//
	// The XAML loader, contains a handle to a MonoObject *
	//
	PluginXamlLoader *xaml_loader;
	
	//
	// A (managed) callback to call when the plugin is unloaded.
	//
	plugin_unload_callback *plugin_unload;

	// The name of the file that we are missing, and we requested to be loaded
	char *vm_missing_file;

	// Private methods
	void CreateWindow ();
	void UpdateSource ();
	void UpdateSourceByReference (const char *value);
	void TryLoad ();
	void SetPageURL ();

	static gboolean IdleUpdateSourceByReference (gpointer data);

	static void RenderSurface (Surface *surface, void *user_data);
	static void InvalidateSurface (Surface *surface, Rect rect, void *user_data);
	static void ReportFPS (Surface *surface, int nframes, float nsecs, void *user_data);
	static void ReportCache (Surface *surface, long bytes, void *user_data);
	static void properties_dialog_response (GtkWidget *dialog, int response, PluginInstance *plugin);
	
 public:
 
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
	List *moon_sources;
	void AddSource (const char *uri, const char *filename);
	List *GetSources ();
#endif

	PluginInstance (NPP instance, uint16_t mode);
	~PluginInstance ();
	
	void SetUnloadCallback (plugin_unload_callback *puc);
	void Initialize (int argc, char *const argn[], char *const argv[]);
	void Finalize ();

	// Mozilla plugin related methods
	NPError GetValue (NPPVariable variable, void *result);
	NPError SetValue (NPNVariable variable, void *value);
	NPError SetWindow (NPWindow *window);
	NPError NewStream (NPMIMEType type, NPStream *stream, NPBool seekable, uint16_t *stype);
	NPError DestroyStream (NPStream *stream, NPError reason);
	void StreamAsFile (NPStream *stream, const char *fname);
	int32_t WriteReady (NPStream *stream);
	int32_t Write (NPStream *stream, int32_t offset, int32_t len, void *buffer);
	void UrlNotify (const char *url, NPReason reason, void *notifyData);
	void Print (NPPrint *platformPrint);
	int16 EventHandle (void *event);
	bool JsRunOnload ();
	void ReportException (char *msg, char *details, char **stack_trace, int num_frames);
	void *LoadUrl (char *url, int32_t *length);

	void      AddWrappedObject    (EventObject *obj, NPObject *wrapper);
	void      RemoveWrappedObject (EventObject *obj);
	NPObject *LookupWrappedObject (EventObject *obj);

	void Properties ();

	NPP getNPP () { return instance; }

	static Downloader *CreateDownloader (PluginInstance *instance)
	{
		if (instance) {
			return instance->surface->CreateDownloader ();
		} else {
			printf ("PluginInstance::CreateDownloader (%p): Unable to create contextual downloader.\n", instance);
			return new Downloader ();
		}
	}

	// Gtk controls
	GtkWidget *container;  // plugin container object
 	Surface *surface;      // plugin surface object
	GdkDisplay *display;

	// Property getters and setters
	char *getInitParams () { return this->initParams; }
	bool getIsLoaded () { return this->isLoaded; };
	char *getSource () { return this->source; }
	void setSource (const char *value);

	char *getBackground ();
	void setBackground (const char *value);
	bool getEnableFramerateCounter ();
	bool getEnableRedrawRegions ();
	void setEnableRedrawRegions (bool value);
	bool getEnableHtmlAccess ();
	bool getWindowless ();
	void setMaxFrameRate (int value);
	int  getMaxFrameRate ();

	MoonlightScriptControlObject *getRootObject ();
	NPP getInstance ();

	int32_t getActualHeight ();
	int32_t getActualWidth ();

	void getBrowserInformation (char **name, char **version,
				    char **platform, char **userAgent,
				    bool *cookieEnabled);
	GSList *timers;
};

extern GSList *plugin_instances;

#define NPID(x) NPN_GetStringIdentifier (x)

#define STREAM_NOTIFY(x) ((StreamNotify*) x)

#define STREAM_NOTIFY_DATA(x) ((StreamNotify*) x)->pdata

#define IS_NOTIFY_SOURCE(x) \
	(!x ? StreamNotify::NONE : (((StreamNotify*) x)->type == StreamNotify::SOURCE))

#define IS_NOTIFY_DOWNLOADER(x) \
	(!x ? StreamNotify::NONE : (((StreamNotify*) x)->type == StreamNotify::DOWNLOADER))

#define IS_NOTIFY_REQUEST(x) \
	(!x ? StreamNotify::NONE : (((StreamNotify*) x)->type == StreamNotify::REQUEST))

class StreamNotify
{
 public:
	enum StreamNotifyFlags {
		NONE = 0,
		SOURCE = 1,
		DOWNLOADER = 2,
		REQUEST = 3
	};

	StreamNotify () : type (NONE), pdata (NULL) {};
	StreamNotify (void *data) : type (NONE), pdata (data) {};
	StreamNotify (StreamNotifyFlags type) : type (type), pdata (NULL) {};
	StreamNotify (StreamNotifyFlags type, void *data) : type (type), pdata (data) {};
	StreamNotify (StreamNotifyFlags type, DependencyObject *dob) : type (type), pdata (dob)
	{
		base_ref (dob);
	}
	
	~StreamNotify ()
	{
		if (type == DOWNLOADER)
			base_unref ((DependencyObject *) pdata);
	}

	StreamNotifyFlags type;
	void *pdata;
};

class PluginXamlLoader : public XamlLoader
{
	PluginXamlLoader (const char *filename, const char *str, PluginInstance *plugin, Surface *surface);
	bool InitializeLoader ();
	PluginInstance *plugin;
	bool initialized;
	bool xaml_is_managed;
	
#if INCLUDE_MONO_RUNTIME
	gpointer managed_loader;
#endif
public:
	virtual ~PluginXamlLoader ();
	const char *TryLoad (int *error);
	
	static PluginXamlLoader *FromFilename (const char *filename, PluginInstance *plugin, Surface *surface)
	{
		return new PluginXamlLoader (filename, NULL, plugin, surface);
	}
	static PluginXamlLoader *FromStr (const char *str, PluginInstance *plugin, Surface *surface)
	{
		return new PluginXamlLoader (NULL, str, plugin, surface);
	}
	
	bool IsManaged () { return xaml_is_managed; }
	virtual bool HookupEvent (void *target, const char *name, const char *value);
	virtual bool LoadVM ();
};

G_BEGIN_DECLS

int32_t plugin_instance_get_actual_width  (PluginInstance *instance);
int32_t plugin_instance_get_actual_height (PluginInstance *instance);

Surface *plugin_instance_get_surface (PluginInstance *instance);

char *plugin_instance_get_init_params  (PluginInstance *instance);

void plugin_instance_get_browser_information (PluginInstance *instance,
					      char **name, char **version,
					      char **platform, char **userAgent,
					      bool *cookieEnabled);

void plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
						   bool *httpnet_access, bool *script_access);

void plugin_instance_report_exception (PluginInstance *instance, char *msg, char *details, char **stack_trace, int num_frames);
void *plugin_instance_load_url (PluginInstance *instance, char *url, int32_t *length);

void     plugin_html_timer_timeout_stop (PluginInstance *instance, uint32_t source_id);
uint32_t plugin_html_timer_timeout_add (PluginInstance *instance, int32_t interval, GSourceFunc callback, gpointer data);
void     plugin_set_unload_callback (PluginInstance *instance, plugin_unload_callback *puc);
PluginXamlLoader *plugin_xaml_loader_from_str (const char *str, PluginInstance *plugin, Surface *surface);

G_END_DECLS

#endif /* MOON_PLUGIN */
