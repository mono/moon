/*
 * moon-plugin.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PLUGIN
#define MOON_PLUGIN

#include "moonlight.h"
//#include "plugin-class.h"

class PluginInstance
{
 private:
  	uint16 mode;           // NP_EMBED, NP_FULL, or NP_BACKGROUND
	NPWindow *window;      // Mozilla window object
	NPP instance;          // Mozilla instance object
	NPObject* rootobject;  // Mozilla jscript object wrapper
	bool xembed_supported; // XEmbed Extension supported

	const char *sourceUrl;

	// Property fields
	char *initParams;
	bool isLoaded;
	char *source;

	//
	// The XAML loader, contains a MonoObject *
	//
	gpointer mono_loader_object;

	// The name of the file that we are missing, and we requested to be loaded
	char *vm_missing_file;

	// The mode that we want to set on the next stream opening.
	const char *vm_missing_url;

	// Private methods
	void CreateWindow ();
	void TryLoad ();
	void LoadFromXaml (const char* fname);
	void RuntimeLoadFromXaml (const char* fname);

 public:	
	PluginInstance (NPP instance, uint16 mode);
	~PluginInstance ();
	void Initialize (int argc, char* const argn[], char* const argv[]);
	void Finalize ();

	// Mozilla plugin related methods
	NPError GetValue (NPPVariable variable, void *result);
	NPError SetValue (NPNVariable variable, void *value);
	NPError SetWindow (NPWindow* window);
	NPError NewStream (NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
	NPError DestroyStream (NPStream* stream, NPError reason);
	void StreamAsFile (NPStream* stream, const char* fname);
	int32 WriteReady (NPStream* stream);
	int32 Write (NPStream* stream, int32 offset, int32 len, void* buffer);
	void UrlNotify (const char* url, NPReason reason, void* notifyData);
	void Print (NPPrint* platformPrint);
	int16 EventHandle (void* event);

	// Gtk controls
	GtkWidget *container;  // plugin container object
	Canvas *canvas;        // plugin canvas object
 	Surface *surface;      // plugin surface object

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

	int32 getActualHeight ();
	int32 getActualWidth ();
};

#endif /* MOON_PLUGIN */
