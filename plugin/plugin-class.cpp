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
#include "plugin.h"

/*** Static wrapper functions *************************************************/

static NPObject*
RuntimeClassAllocate (NPP instance, NPClass *aClass)
{
	PluginObject *object;
	object = (PluginObject*) NPN_MemAlloc (sizeof (PluginObject));
	if (!object)
		return NULL;

	object->instance = instance;
	object->plugin = (PluginInstance*) instance->pdata;

	return object;
}

static void
RuntimeClassDeallocate (NPObject *npobj)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		plugin->ClassDeallocate ((PluginObject *) npobj);
}

static void
RuntimeClassInvalidate (NPObject *npobj)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		plugin->ClassInvalidate ((PluginObject *) npobj);
}

static bool
RuntimeClassHasProperty (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassHasProperty ((PluginObject *) npobj, name);

	return false;
}

static bool
RuntimeClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassGetProperty ((PluginObject *) npobj, name, result);

	return false;
}

static bool
RuntimeClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassSetProperty ((PluginObject *) npobj, name, value);

	return false;
}

static bool
RuntimeClassRemoveProperty (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassRemoveProperty ((PluginObject *) npobj, name);

	return false;
}

static bool
RuntimeClassHasMethod (NPObject *npobj, NPIdentifier name)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassHasMethod ((PluginObject *) npobj, name);

	return false;
}

static bool
RuntimeClassInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, 
                  uint32_t argCount, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassInvoke ((PluginObject *) npobj, name, args, argCount, result);

	return false;
}

static bool
RuntimeClassInvokeDefault (NPObject *npobj, const NPVariant *args,
	                                uint32_t argCount, NPVariant *result)
{
	PluginClass *plugin = (PluginClass *) npobj->_class;
	if (plugin != NULL)
		return plugin->ClassInvokeDefault ((PluginObject *) npobj, args, argCount, result);

	return false;
}

/*** PluginClass **************************************************************/

PluginClass::PluginClass ()
{
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
PluginClass::ClassDeallocate (PluginObject *npobj)
{
	if (npobj)
		NPN_ReleaseObject (npobj);
}

void
PluginClass::ClassInvalidate (PluginObject *npobj)
{
	// nothing to do.
}

bool
PluginClass::ClassHasProperty (PluginObject *npobj, NPIdentifier name)
{
	NPUTF8 * strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("*** PluginClass::ClassHasProperty %s", strname);
	NPN_MemFree(strname);

	return false;
}

bool
PluginClass::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	DEBUGMSG ("*** PluginClass::ClassGetProperty");
	return false;
}

bool
PluginClass::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	DEBUGMSG ("*** PluginClass::ClassSetProperty");
	return false;
}

bool
PluginClass::ClassRemoveProperty (PluginObject *npobj, NPIdentifier name)
{
	return false;
}

bool
PluginClass::ClassHasMethod (PluginObject *npobj, NPIdentifier name)
{
	NPUTF8 * strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("*** PluginClass::ClassHasMethod %s", strname);
	NPN_MemFree(strname);

	return false;
}

bool
PluginClass::ClassInvoke (PluginObject *npobj, NPIdentifier name, const NPVariant *args, 
                  uint32_t argCount, NPVariant *result)
{
	DEBUGMSG ("*** PluginClass::ClassInvoke");
	return false;
}

bool
PluginClass::ClassInvokeDefault (PluginObject *npobj, const NPVariant *args,
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

PluginRootClass::PluginRootClass ()
{
	this->settings = new PluginSettings ();
	this->content = new PluginContent ();
}

bool
PluginRootClass::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	if (name == NPID ("settings")) {
		NPObject *object = NPN_CreateObject (npobj->instance, this->settings);
		OBJECT_TO_NPVARIANT (object, *result);
		return true;
	} 

	if (name == NPID ("content")) {
		NPObject *object = NPN_CreateObject (npobj->instance, this->content);
		OBJECT_TO_NPVARIANT (object, *result);
		return true;
	} 

	if (name == NPID ("isLoaded")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getIsLoaded (), *result);
		return true;
	} 

	if (name == NPID ("source")) {
		STRING_TO_NPVARIANT (npobj->plugin->getSource (), *result);
		return true;
	} 

	return false;
}

bool 
PluginRootClass::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	// In Silverlight you can set source but it dont change, so do nothing.
	if (name == NPID ("source")) {
		return true;
	} 

	return false;
}

bool
PluginRootClass::ClassInvoke (PluginObject *npobj, NPIdentifier name, 
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return false;
}

/*** PluginSettings ***********************************************************/

bool
PluginSettings::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	if (name == NPID ("version")) {
		STRING_TO_NPVARIANT (PLUGIN_VERSION, *result);
		return true;
	}

	return false;
}

bool 
PluginSettings::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return false;
}

/*** PluginContent ************************************************************/

bool
PluginContent::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
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
PluginContent::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	// not implemented yet.
	if (name == NPID ("fullScreen")) {
		return true;
	}

	return false;
}

bool
PluginContent::ClassInvoke (PluginObject *npobj, NPIdentifier name, 
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return false;
}

/*** PluginDependencyObject ***************************************************/

bool
PluginDependencyObject::ClassHasProperty (PluginObject *npobj, NPIdentifier name)
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
PluginDependencyObject:: ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	//
	// Am not sure if this is how you test this
	//
	if (name == NPN_GetStringIdentifier ("getHost()")){
		// Dont know what to do with refcount, nor wrapping, but you get the idea
		return FALSE;
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
