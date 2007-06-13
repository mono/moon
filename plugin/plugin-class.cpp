/*
 * plugin-class.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "plugin-class.h"

/*** Static wrapper functions *************************************************/

static NPObject*
RuntimeClassAllocate (NPP instance, NPClass *aClass)
{
	NPObject *object;
	object = (NPObject*) NPN_MemAlloc (sizeof (NPObject));
	if (!object)
		return NULL;

	return object;
}

static void
RuntimeClassDeallocate (NPObject *npobj)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		plugin->ClassDeallocate (npobj);
}

static void
RuntimeClassInvalidate (NPObject *npobj)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		plugin->ClassInvalidate (npobj);
}

static bool
RuntimeClassHasProperty (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassHasProperty (npobj, name);

	return false;
}

static bool
RuntimeClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassGetProperty (npobj, name, result);

	return false;
}

static bool
RuntimeClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassSetProperty (npobj, name, value);

	return false;
}

static bool
RuntimeClassRemoveProperty (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassRemoveProperty (npobj, name);

	return false;
}

static bool
RuntimeClassHasMethod (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassHasMethod (npobj, name);

	return false;
}

static bool
RuntimeClassInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, 
                  uint32_t argCount, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassInvoke (npobj, name, args, argCount, result);

	return false;
}

static bool
RuntimeClassInvokeDefault (NPObject *npobj, const NPVariant *args,
	                                uint32_t argCount, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassInvokeDefault (npobj, args, argCount, result);

	return false;
}

/*** PluginClass **************************************************************/

PluginClass::PluginClass (NPP instance)
{
	this->instance = instance;

	this->allocate       = &RuntimeClassAllocate;
	this->deallocate     = &RuntimeClassDeallocate;
	this->invalidate     = &RuntimeClassInvalidate;
	this->hasProperty    = &RuntimeClassHasProperty;
	this->getProperty    = &RuntimeClassGetProperty;
	this->setProperty    = &RuntimeClassSetProperty;
	this->removeProperty = &RuntimeClassRemoveProperty;
	this->hasMethod      = &RuntimeClassHasMethod;
	this->invoke         = &RuntimeClassInvoke;
	this->invokeDefault  = &RuntimeClassInvokeDefault;
}

PluginClass::~PluginClass ()
{
	// nothing to do.
}

void
PluginClass::ClassDeallocate (NPObject *npobj)
{
	if (npobj)
		NPN_ReleaseObject (npobj);
}

void
PluginClass::ClassInvalidate (NPObject *npobj)
{
	// nothing to do.
}

bool
PluginClass::ClassHasProperty (NPObject *npobj, NPIdentifier name)
{
	NPUTF8 * strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("*** PluginClass::ClassHasProperty %s", strname);
	NPN_MemFree(strname);

	return false;
}

bool
PluginClass::ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	DEBUGMSG ("*** PluginClass::ClassGetProperty");
	return false;
}

bool
PluginClass::ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	DEBUGMSG ("*** PluginClass::ClassSetProperty");
	return false;
}

bool
PluginClass::ClassRemoveProperty (NPObject *npobj, NPIdentifier name)
{
	return false;
}

bool
PluginClass::ClassHasMethod (NPObject *npobj, NPIdentifier name)
{
	NPUTF8 * strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("*** PluginClass::ClassHasMethod %s", strname);
	NPN_MemFree(strname);

	return false;
}

bool
PluginClass::ClassInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, 
                  uint32_t argCount, NPVariant *result)
{
	DEBUGMSG ("*** PluginClass::ClassInvoke");
	return false;
}

bool
PluginClass::ClassInvokeDefault (NPObject *npobj, const NPVariant *args,
	                                uint32_t argCount, NPVariant *result)
{
	DEBUGMSG ("*** PluginClass::ClassInvokeDefault");
	return false;
}

int
PluginClass::IndexOf (NPIdentifier name, const char *const names[], int count)
{
	for (int i = 0; i < count; i++) {
		if (name == NPN_GetStringIdentifier (names [i]))
			return i;
	}

	return -1;
}

/*** PluginRootClass **********************************************************/

PluginRootClass::PluginRootClass (NPP instance) : PluginClass (instance)
{
	this->settings = new PluginSettings (instance);
	this->content = new PluginContent (instance);
}

bool
PluginRootClass::ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	if (name == NPID ("settings"))
	{
		NPObject *object = NPN_CreateObject (this->instance, this->settings);
		OBJECT_TO_NPVARIANT (object, *result);
		return true;
	} 

	if (name == NPID ("content"))
	{
		NPObject *object = NPN_CreateObject (this->instance, this->content);
		OBJECT_TO_NPVARIANT (object, *result);
		return true;
	} 

	return false;
}

bool 
PluginRootClass::ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return false;
}

bool
PluginRootClass::ClassInvoke (NPObject *npobj, NPIdentifier name, 
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return false;
}

/*** PluginSettings ***********************************************************/

bool
PluginSettings::ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	if (name == NPID ("version")) {
		int len = strlen (PLUGIN_VERSION);
		char *version = (char *) NPN_MemAlloc (len + 1);
		memcpy (version, PLUGIN_VERSION, len + 1);
		STRINGN_TO_NPVARIANT (version, len, *result);

		return true;
	}

	return false;
}

bool 
PluginSettings::ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return false;
}

/*** PluginContent ************************************************************/

bool
PluginContent::ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	// Silverlight always return 0.
	if (name == NPID ("actualHeight")) {
		INT32_TO_NPVARIANT (0, *result);
		return true;
	}

	// Silverlight always return 0.
	if (name == NPID ("actualWidth")) {
		INT32_TO_NPVARIANT (0, *result);
		return true;
	}

	// not implemented yet.
	if (name == NPID ("fullScreen")) {
		BOOLEAN_TO_NPVARIANT (false, *result);
		return true;
	}

	return false;
}

bool 
PluginContent::ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	// not implemented yet.
	if (name == NPID ("fullScreen")) {
		return true;
	}

	return false;
}

bool
PluginContent::ClassInvoke (NPObject *npobj, NPIdentifier name, 
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return false;
}

/*** PluginDependencyObject ***************************************************/

bool
PluginDependencyObject::ClassHasProperty (NPObject *npobj, NPIdentifier name)
{
	//
	// Am not sure if this is how you test this
	//
	if (name == NPN_GetStringIdentifier ("getHost ()")){
		return TRUE;
	}

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyProperty *p = dob->GetDependencyProperty (strname);
	NPN_MemFree (strname);

	if (p == NULL)
		return FALSE;

	Value *val =  dob->GetValue (p);

	if (val == 0)
		return FALSE;

	return TRUE;
}

bool
PluginDependencyObject:: ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	//
	// Am not sure if this is how you test this
	//
	if (name == NPN_GetStringIdentifier ("getHost()")){
		// Dont know what to do with refcount, nor wrapping, but you get the idea
		return NULL;
	}

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyProperty *p = dob->GetDependencyProperty (strname);
	NPN_MemFree (strname);
	if (p == NULL)
		return FALSE;

	Value *val =  dob->GetValue (p);

	if (val == 0)
		return FALSE;


	//
	// TODO: convert_val_to_something_jscript_can_use ();
	//
	return TRUE;
}
