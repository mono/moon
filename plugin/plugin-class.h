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

#include "moonlight.h"

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

/*** PluginClass **************************************************************/

class PluginClass : public NPClass
{
 protected:
	int IndexOf (NPIdentifier name, const char *const names[], int count);

 public:
	PluginClass ();
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
	"background",
	"enableFramerateCounter",
	"enableRedrawRegions",
	"enableHtmlAccess",
	"maxFrameRate",
	"version",
	"windowless"
};

class PluginSettings : public PluginClass
{
 public:
	PLUGIN_PROPERTIES (PluginSettingsPropertyNames);
};

/*** PluginContent ************************************************************/

static const char *const PluginContentPropertyNames [] = 
{
	"actualHeight",
	"actualWidth",
	"fullScreen"
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
	"settings",
	"content",
	"initParams",
	"isLoaded",
	"source"
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
	NPP instance;
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
	PluginRootClass *host;
	
	PluginDependencyObject (PluginRootClass *the_host, DependencyObject *the_dob)
		: host(the_host), dob(the_dob) {}

	
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
};
