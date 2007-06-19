/*
 * plugin-class.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef PLUGIN_CLASS
#define PLUGIN_CLASS

#include "moonlight.h"
#include "plugin.h"

/*** Macros *******************************************************************/

#define NPID(x) NPN_GetStringIdentifier (x)

#define PLUGIN_PROPERTIES(x) \
	bool ClassHasProperty (PluginObject *npobj, NPIdentifier name) \
		{ return IndexOf (name, x, (sizeof (x) / sizeof (char *))) > -1; }; \
	virtual bool ClassGetProperty ( \
		PluginObject *npobj, NPIdentifier name, NPVariant *result); \
	virtual bool ClassSetProperty ( \
		PluginObject *npobj, NPIdentifier name, const NPVariant *value);

#define PLUGIN_METHODS(x) \
	bool ClassHasMethod (PluginObject *npobj, NPIdentifier name) \
		{ return IndexOf (name, x, (sizeof (x) / sizeof (char *))) > -1; }; \
	virtual bool ClassInvoke ( \
		PluginObject *npobj, NPIdentifier name, const NPVariant *args,  \
		uint32_t argCount, NPVariant *result);

#define HAS_PROPERTY(x,v) \
		(IndexOf (v, x, (sizeof (x) / sizeof (char *))) > -1)

/*** PluginObject *************************************************************/

class PluginObject : public NPObject
{
 public:
	NPP instance;
	PluginInstance *plugin;
};

/*** PluginClass **************************************************************/

class PluginClass : public NPClass
{
 protected:
	int IndexOf (NPIdentifier name, const char *const names[], int count);
	void StringToNPVariant (char *value, NPVariant *result);

 public:
	PluginClass ();
	virtual ~PluginClass ();

	virtual void ClassDeallocate (PluginObject *npobj);
	virtual void ClassInvalidate (PluginObject *npobj);
	virtual bool ClassHasProperty (PluginObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value);
	virtual bool ClassRemoveProperty (PluginObject *npobj, NPIdentifier name);
	virtual bool ClassHasMethod (PluginObject *npobj, NPIdentifier name);
	virtual bool ClassInvoke (PluginObject *npobj, NPIdentifier name,
	                         const NPVariant *args, uint32_t argCount,
	                         NPVariant *result);
	virtual bool ClassInvokeDefault (PluginObject *npobj, const NPVariant *args,
	                                uint32_t argCount, NPVariant *result);

};

/*** PluginSettings ***********************************************************/

static const char *const PluginSettingsPropertyNames [7] = 
{
	"background",             // read write
	"enableFramerateCounter", // read write (cant be set after initialization)
	"enableRedrawRegions",    // read write
	"enableHtmlAccess",       // read write (cant be set after initialization)
	"maxFrameRate",           // read write
	"version",                // read only
	"windowless"              // read write (cant be set after initialization)
};

class PluginSettings : public PluginClass
{
 public:
	PLUGIN_PROPERTIES (PluginSettingsPropertyNames);
};

/*** PluginContent ************************************************************/

static const char *const PluginContentPropertyNames [] = 
{
	"actualHeight", // read only
	"actualWidth",  // read only
	"fullScreen"    // read write
};

static const char *const PluginContentMethodNames [] = 
{
	"createFromXaml",
	"createFromXamlDownloader",
	"findName"
};

// TODO:
//onFullScreenChange = "eventhandlerFunction"
//onResize = "eventhandlerFunction"

class PluginContent : public PluginClass
{
 public:
	PLUGIN_PROPERTIES (PluginContentPropertyNames);
	PLUGIN_METHODS (PluginContentMethodNames);
};

/*** PluginRootClass **********************************************************/

static const char *const PluginRootClassPropertyNames [] = 
{
	"settings",   // read only
	"content",    // read only
	"initParams", // read only
	"isLoaded",   // read only
	"source"      // read write (cant be set after initialization)
};

static const char *const PluginRootClassMethodNames [] = 
{
	"createObject"
};

// TODO:
// onError = "eventhandlerFunction"

class PluginRootClass : public PluginClass
{
 private:
	PluginSettings *settings;
	PluginContent *content;

 public:
	PluginRootClass ();
	
	PLUGIN_PROPERTIES (PluginRootClassPropertyNames);
	PLUGIN_METHODS (PluginRootClassMethodNames);
};

static PluginRootClass* rootclass = NULL;

/*** PluginDependencyObject ***************************************************/

static const char *const PluginDependencyObjectPropertyNames [] = 
{
};

static const char *const PluginDependencyObjectMethodNames [] = 
{
	"getHost"
};

class PluginDependencyObject : public PluginClass
{
 public:
	DependencyObject *dob;
	
	PluginDependencyObject (DependencyObject *the_dob)
		: PluginClass (), dob(the_dob) {}

	virtual bool ClassHasProperty (PluginObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value);
};

#endif /* PLUGIN_CLASS */
