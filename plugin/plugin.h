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
#include "plugin-class.h"

class PluginSettings : public PluginClass
{
 public:
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
};

class PluginInstance : public PluginClass
{
 private:
  	uint16 mode;           // NP_EMBED, NP_FULL, or NP_BACKGROUND
	NPWindow *window;      // Mozilla window object
	NPP instance;          // Mozilla instance object
	bool xembed_supported; // XEmbed Extension supported
	NPObject *object;      // JavaScript object

	// Settings
	PluginSettings *settings;
	NPObject *obj_settings;

	// Private methods
	void CreateWindow ();
	void CreateDemoControls ();

 public:	
	PluginInstance (NPP instance, uint16 mode);
	~PluginInstance ();
	void Initialize (int argc, char* const argn[], char* const argv[]);

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

	// Runtime related
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);

 };

#endif /* MOON_PLUGIN */
