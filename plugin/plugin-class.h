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
	bool ClassHasProperty (NPObject *npobj, NPIdentifier name) \
		{ return IndexOf (name, x, (sizeof (x) / sizeof (char *))) > -1; }; \
	virtual bool ClassGetProperty ( \
		NPObject *npobj, NPIdentifier name, NPVariant *result); \
	virtual bool ClassSetProperty ( \
		NPObject *npobj, NPIdentifier name, const NPVariant *value);

#define PLUGIN_METHODS(x) \
	bool ClassHasMethod (NPObject *npobj, NPIdentifier name) \
		{ return IndexOf (name, x, (sizeof (x) / sizeof (char *))) > -1; }; \
	virtual bool ClassInvoke ( \
		NPObject *npobj, NPIdentifier name, const NPVariant *args,  \
		uint32_t argCount, NPVariant *result);

#define STRING_TO_NPVARIANT(x,z) \
		int len = strlen (x); \
		char *retval = (char *) NPN_MemAlloc (len + 1); \
		memcpy (retval, x, len + 1); \
		STRINGN_TO_NPVARIANT (retval, len, z);

/*** PluginClass **************************************************************/

class PluginClass : public NPClass
{
 protected:
	NPP instance;

	int IndexOf (NPIdentifier name, const char *const names[], int count);

 public:
	PluginClass (NPP instance);
	virtual ~PluginClass ();

	virtual void ClassDeallocate (NPObject *npobj);
	virtual void ClassInvalidate (NPObject *npobj);
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value);
	virtual bool ClassRemoveProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassHasMethod (NPObject *npobj, NPIdentifier name);
	virtual bool ClassInvoke (NPObject *npobj, NPIdentifier name,
	                         const NPVariant *args, uint32_t argCount,
	                         NPVariant *result);
	virtual bool ClassInvokeDefault (NPObject *npobj, const NPVariant *args,
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
	PluginSettings (NPP instance) : PluginClass (instance) {};
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
	PluginContent (NPP instance) : PluginClass (instance) {};

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
	PluginRootClass (NPP instance);
	
	PLUGIN_PROPERTIES (PluginRootClassPropertyNames);
	PLUGIN_METHODS (PluginRootClassMethodNames);
};

/*** PluginDependencyObject ***************************************************/

class PluginDependencyObject : public PluginClass
{
 public:
	DependencyObject *dob;
	
	PluginDependencyObject (NPP instance, DependencyObject *the_dob)
		: PluginClass (instance), dob(the_dob) {}

	
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
};

#endif /* PLUGIN_CLASS */
