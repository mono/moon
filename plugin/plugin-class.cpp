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

#include <ctype.h>
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

void
PluginClass::StringToNPVariant (char *value, NPVariant *result)
{
	int len;
	char * retval;

	if (value) {
		len = strlen (value);
		retval = (char *) NPN_MemAlloc (len + 1);
		strcpy (retval, value);
	} else {
		len = 0;
		retval = (char *) NPN_MemAlloc (len);
		retval[0] = 0;
	}

	STRINGN_TO_NPVARIANT (retval, len, *result);
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

	if (name == NPID ("initParams")) {
		StringToNPVariant (npobj->plugin->getInitParams (), result);
		return true;
	} 

	if (name == NPID ("isLoaded")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getIsLoaded (), *result);
		return true;
	} 

	if (name == NPID ("source")) {
		StringToNPVariant (npobj->plugin->getSource (), result);
		return true;
	} 

	return false;
}

bool 
PluginRootClass::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	if (name == NPID ("source")) {
		npobj->plugin->setSource (NPVARIANT_TO_STRING (*value).utf8characters);
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
	if (name == NPID ("background")) {
		StringToNPVariant (npobj->plugin->getBackground (), result);
		return true;
	}

	if (name == NPID ("enableFramerateCounter")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getEnableFramerateCounter (), *result);
		return true;
	} 

	if (name == NPID ("enableRedrawRegions")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getEnableRedrawRegions (), *result);
		return true;
	} 

	if (name == NPID ("enableHtmlAccess")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getEnableHtmlAccess (), *result);
		return true;
	} 

	// not implemented yet, just return 0.
	if (name == NPID ("maxFrameRate")) {
		INT32_TO_NPVARIANT (0, *result);
		return true;
	}

	if (name == NPID ("version")) {
		StringToNPVariant (PLUGIN_VERSION, result);
		return true;
	}

	if (name == NPID ("windowless")) {
		BOOLEAN_TO_NPVARIANT (npobj->plugin->getWindowless (), *result);
		return true;
	} 

	return false;
}

bool 
PluginSettings::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	if (name == NPID ("background")) {
		npobj->plugin->setBackground (NPVARIANT_TO_STRING (*value).utf8characters);
		return true;
	}

	// Cant be set after initialization so return true
	if (name == NPID ("enableFramerateCounter")) {
		return true;
	} 

	if (name == NPID ("enableRedrawRegions")) {
		npobj->plugin->setEnableRedrawRegions (NPVARIANT_TO_BOOLEAN (*value));
		return true;
	} 

	// Cant be set after initialization so return true
	if (name == NPID ("enableHtmlAccess")) {
		return true;
	} 

	// not implemented yet.
	if (name == NPID ("maxFrameRate")) {
		return true;
	}

	// Cant be set after initialization so return true
	if (name == NPID ("windowless")) {
		return true;
	} 

	return false;
}

/*** PluginContent ************************************************************/

bool
PluginContent::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	// Silverlight always return 0.
	if (name == NPID ("actualHeight")) {
		INT32_TO_NPVARIANT (npobj->plugin->getActualHeight (), *result);
		return true;
	}

	// Silverlight always return 0.
	if (name == NPID ("actualWidth")) {
		INT32_TO_NPVARIANT (npobj->plugin->getActualWidth (), *result);
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
	if (name == NPID ("findName")) {

		if (!argCount)
			return true;

		UIElement *canvas = npobj->plugin->surface->toplevel;

		if (!canvas)
			return true;

		char *name = (char *) NPVARIANT_TO_STRING (args[0]).utf8characters;
		
		DependencyObject *element = canvas->FindName (name);
		if (!element)
			return true;

		PluginDependencyObject *depobj = new PluginDependencyObject ((DependencyObject*) element);
		
		NPObject *object = NPN_CreateObject (npobj->instance, depobj);
		OBJECT_TO_NPVARIANT (object, *result);
		return true;
	}

	return false;
}

/*** PluginDependencyObject ***************************************************/

bool
PluginDependencyObject::ClassHasProperty (PluginObject *npobj, NPIdentifier name)
{
	DEBUGMSG ("PluginDependencyObject::ClassHasProperty");

	if (HAS_PROPERTY (PluginDependencyObjectPropertyNames, name))
		return true;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);
	DependencyProperty *p = dob->GetDependencyProperty (strname);
	NPN_MemFree (strname);

	return (p != NULL);
}

static void
value_to_variant (PluginObject *npobj, Value *v, NPVariant *result)
{
	switch (v->GetKind ()) {
	case Type::BOOL:
	  BOOLEAN_TO_NPVARIANT (v->AsBool(), *result);
	  break;
	case Type::INT32:
	  INT32_TO_NPVARIANT (v->AsInt32(), *result);
	  break;
	case Type::DOUBLE:
	  DOUBLE_TO_NPVARIANT (v->AsDouble(), *result);
	  break;
	case Type::STRING:
	  STRINGZ_TO_NPVARIANT (v->AsString(), *result);
	  break;
	  /* more builtins.. */
	default:
	  if (v->GetKind () >= Type::DEPENDENCY_OBJECT) {
		PluginDependencyObject *depobj = new PluginDependencyObject (v->AsDependencyObject ());
		
		NPObject *object = NPN_CreateObject (npobj->instance, depobj);
		OBJECT_TO_NPVARIANT (object, *result);
	  }
	  break;
	}
}

static void
variant_to_value (PluginObject *npobj, const NPVariant *variant, Type::Kind property_type, Value *v)
{
	*v = Value (Type::INVALID);

	if (NPVARIANT_IS_BOOLEAN (*variant)) {
		*v = Value ((bool)NPVARIANT_TO_BOOLEAN (*variant));
	}
	else if (NPVARIANT_IS_INT32 (*variant)) {
		/* for some reason javascript likes to pass numbers as ints
		   even when you put .0 after them, or treat them in some
		   other fashion like floats. */
		if (property_type == Type::DOUBLE)
			*v = Value ((double)NPVARIANT_TO_INT32 (*variant));
		else
			*v = Value ((gint32)NPVARIANT_TO_INT32 (*variant));
	}
	else if (NPVARIANT_IS_DOUBLE (*variant)) {
		*v = Value (NPVARIANT_TO_DOUBLE (*variant));
	}
	else if (NPVARIANT_IS_STRING (*variant)) {
	  printf ("setting a string property to %s\n", NPVARIANT_TO_STRING (*variant).utf8characters);
		*v = Value ((char *)NPVARIANT_TO_STRING (*variant).utf8characters);
	}
}

bool
PluginDependencyObject::ClassGetProperty (PluginObject *npobj, NPIdentifier name, NPVariant *result)
{
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);
	DependencyProperty *p = dob->GetDependencyProperty (strname);
	NPN_MemFree (strname);

	if (!p)
		return false;

	Value *value =  dob->GetValue (p);
	if (!value)
		return false;

	value_to_variant (npobj, value, result);

	return true;
}

bool 
PluginDependencyObject::ClassSetProperty (PluginObject *npobj, NPIdentifier name, const NPVariant *value)
{
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);

	DEBUGMSG ("PluginDependencyObject::ClassSetProperty (%s)", strname);

	DependencyProperty *p = dob->GetDependencyProperty (strname);
	NPN_MemFree (strname);

	if (p == NULL)
		return false;

	Value val;

	variant_to_value (npobj, value, p->value_type, &val);

	if (val.GetKind () == Type::INVALID)
	  return false;

	dob->SetValue (p, val);

	return true;
}
