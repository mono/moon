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
#include "plstr.h"

#define HAS_PROPERTY(x,v) \
		(index_of_name (v, x, (sizeof (x) / sizeof (char *))) > -1)
#define HAS_METHOD(x,v) \
		(index_of_name (v, x, (sizeof (x) / sizeof (char *))) > -1)


static bool
name_matches (NPIdentifier id, const char *name)
{
	if (id == NPN_GetStringIdentifier (name))
		return true;

	bool rv = false;

	if (islower (name[0])) {
		char *n = g_strdup (name);
		n[0] = toupper (n[0]);

		rv = (id == NPN_GetStringIdentifier (n));
		g_free (n);
	}

	return rv;
}

static int
index_of_name (NPIdentifier name, const char *const names[], int count)
{
	for (int i = 0; i < count; i++) {
		if (name_matches (name, names[i]))
			return i;
	}

	return -1;
}

static void
string_to_npvariant (char *value, NPVariant *result)
{
	char * retval;

	if (value)
		retval = PL_strdup (value);
	else
		retval = PL_strdup ("");

	STRINGZ_TO_NPVARIANT (retval, *result);
}

/*** Points ***/
static NPObject* point_allocate (NPP instance, NPClass *)
{
	MoonlightPoint *p = new MoonlightPoint ();
	p->x = p->y = 0.0;
}

static void point_deallocate (NPObject *npobject)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightPoint*)npobject;
}

static bool point_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "x") ||
		name_matches (name, "y"));
}

static bool point_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightPoint *p = (MoonlightPoint*)npobj;
	if (name_matches (name, "x")) {
		DOUBLE_TO_NPVARIANT (p->x, *result);
		return true;
	}
	else if (name_matches (name, "y")) {
		DOUBLE_TO_NPVARIANT (p->y, *result);
		return true;
	}
	else
		return false;
}

static bool point_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightPoint *p = (MoonlightPoint*)npobj;
	if (name_matches (name, "x")) {
		p->x = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else if (name_matches (name, "y")) {
		p->y = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else
		return false;
}


MoonlightPointType::MoonlightPointType ()
{
	allocate = point_allocate;
	deallocate = point_deallocate;
	hasProperty = point_has_property;
	getProperty = point_get_property;
	setProperty = point_set_property;

	invalidate = NULL;
	hasMethod = NULL;
	invoke = NULL;
	invokeDefault = NULL;
}

MoonlightPointType *MoonlightPointClass;


/*** our object base class */
NPObject*
moonlight_object_allocate (NPP instance, NPClass*)
{
	return new MoonlightObject (instance);
}

static void
moonlight_object_deallocate (NPObject *npobj)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightObject*)npobj;
}

static void
moonlight_object_invalidate (NPObject *npobj)
{
	// nothing to do
}

static bool
moonlight_object_has_method (NPObject *npobj, NPIdentifier name)
{
	return false;
}

static bool
moonlight_object_has_property (NPObject *npobj, NPIdentifier name)
{
	return false;
}

static bool
moonlight_object_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	g_warning ("moonlight_object_get_property reached");
	return false;
}

static bool
moonlight_object_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	g_warning ("moonlight_object_set_property reached");
	return false;
}

static bool
moonlight_object_remove_property (NPObject *npobj, NPIdentifier name)
{
	g_warning ("moonlight_object_remove_property reached");
	return false;
}

static bool
moonlight_object_invoke (NPObject *npobj, NPIdentifier name,
			 const NPVariant *args, uint32_t argCount,
			 NPVariant *result)
{
	g_warning ("moonlight_object_invoke reached");
	return false;
}

static bool
moonlight_object_invoke_default (NPObject *npobj,
				 const NPVariant *args, uint32_t argCount,
				 NPVariant *result)
{
	g_warning ("moonlight_object_invoke_default reached");
	return false;
}

MoonlightObjectType::MoonlightObjectType ()
{
	allocate       = moonlight_object_allocate;
	deallocate     = moonlight_object_deallocate;
	invalidate     = moonlight_object_invalidate;
	hasMethod      = moonlight_object_has_method;
	invoke         = moonlight_object_invoke;
	invokeDefault  = moonlight_object_invoke_default;
	hasProperty    = moonlight_object_has_property;
	getProperty    = moonlight_object_get_property;
	setProperty    = moonlight_object_set_property;
	removeProperty = moonlight_object_remove_property;
}

MoonlightObjectType* MoonlightObjectClass;

/*** MoonlightControlClass **********************************************************/
static NPObject*
moonlight_control_allocate (NPP instance, NPClass*)
{
  printf ("in moonlight_control_allocate!  yay!\n");
	return new MoonlightControlObject (instance);
}

static void
moonlight_control_deallocate (NPObject *npobj)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightControlObject*)npobj;
}

static void
moonlight_control_invalidate (NPObject *npobj)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;

	if (control->settings)
		NPN_ReleaseObject (control->settings);
	control->settings = NULL;

	if (control->content)
		NPN_ReleaseObject (control->content);
	control->content = NULL;
}

static bool
moonlight_control_has_property (NPObject *npobj, NPIdentifier name)
{
  printf ("YO NO\n");
	return (name_matches (name, "settings") ||
		name_matches (name, "content") ||
		name_matches (name, "initParams") ||
		name_matches (name, "isLoaded") ||
		name_matches (name, "source"));
}

static bool
moonlight_control_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
  printf ("YO!\n");

	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;
	MoonlightControlObject *rootobj = (MoonlightControlObject*)npobj;

	if (name_matches (name, "settings")) {
		OBJECT_TO_NPVARIANT (rootobj->settings, *result);
		return true;
	} 

	if (name_matches (name, "content")) {
		OBJECT_TO_NPVARIANT (rootobj->content, *result);
		return true;
	} 

	if (name_matches (name, "initParams")) {
		string_to_npvariant (plugin->getInitParams (), result);
		return true;
	} 

	if (name_matches (name, "isLoaded")) {
		BOOLEAN_TO_NPVARIANT (plugin->getIsLoaded (), *result);
		return true;
	} 

	if (name_matches (name, "source")) {
		string_to_npvariant (plugin->getSource (), result);
		return true;
	} 

	return false;
}

static bool
moonlight_control_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

	if (name_matches (name, "source")) {
		plugin->setSource (NPVARIANT_TO_STRING (*value).utf8characters);
		return true;
	}

	return false;
}

MoonlightControlType::MoonlightControlType ()
{
	allocate = moonlight_control_allocate;
	deallocate = moonlight_control_deallocate;
	invalidate = moonlight_control_invalidate;

	hasProperty = moonlight_control_has_property;
	getProperty = moonlight_control_get_property;
	setProperty = moonlight_control_set_property;
}

MoonlightControlType* MoonlightControlClass;

/*** MoonlightSettingsClass ***********************************************************/

static const char *const
moonlight_settings_properties [] = {
	"background",             // read write
	"enableFramerateCounter", // read write (cant be set after initialization)
	"enableRedrawRegions",    // read write
	"enableHtmlAccess",       // read write (cant be set after initialization)
	"maxFrameRate",           // read write
	"version",                // read only
	"windowless"              // read write (cant be set after initialization)
};

static bool
moonlight_settings_has_property (NPObject *npobj, NPIdentifier name)
{
	return HAS_PROPERTY (moonlight_settings_properties, name);
}

static bool
moonlight_settings_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

	if (name_matches (name, "background")) {
		string_to_npvariant (plugin->getBackground (), result);
		return true;
	}

	if (name_matches (name, "enableFramerateCounter")) {
		BOOLEAN_TO_NPVARIANT (plugin->getEnableFramerateCounter (), *result);
		return true;
	} 

	if (name_matches (name, "enableRedrawRegions")) {
		BOOLEAN_TO_NPVARIANT (plugin->getEnableRedrawRegions (), *result);
		return true;
	} 

	if (name_matches (name, "enableHtmlAccess")) {
		BOOLEAN_TO_NPVARIANT (plugin->getEnableHtmlAccess (), *result);
		return true;
	} 

	// not implemented yet, just return 0.
	if (name_matches (name, "maxFrameRate")) {
		INT32_TO_NPVARIANT (0, *result);
		return true;
	}

	if (name_matches (name, "version")) {
		string_to_npvariant (PLUGIN_VERSION, result);
		return true;
	}

	if (name_matches (name, "windowless")) {
		BOOLEAN_TO_NPVARIANT (plugin->getWindowless (), *result);
		return true;
	} 

	return false;
}

static bool 
moonlight_settings_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;
	if (name_matches (name, "background")) {
		plugin->setBackground (NPVARIANT_TO_STRING (*value).utf8characters);
		return true;
	}

	// Cant be set after initialization so return true
	if (name_matches (name, "enableFramerateCounter")) {
		return true;
	} 

	if (name_matches (name, "enableRedrawRegions")) {
		plugin->setEnableRedrawRegions (NPVARIANT_TO_BOOLEAN (*value));
		return true;
	} 

	// Cant be set after initialization so return true
	if (name_matches (name, "enableHtmlAccess")) {
		return true;
	} 

	// not implemented yet.
	if (name_matches (name, "maxFrameRate")) {
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}

	// Cant be set after initialization so return true
	if (name_matches (name, "windowless")) {
		return true;
	} 

	return false;
}

MoonlightSettingsType::MoonlightSettingsType ()
{
	hasProperty = moonlight_settings_has_property;
	getProperty = moonlight_settings_get_property;
	setProperty = moonlight_settings_set_property;
}

MoonlightSettingsType* MoonlightSettingsClass;


/*** MoonlightContentClass ************************************************************/

static const char *const
moonlight_content_properties[] = {
	"actualHeight", // read only
	"actualWidth",  // read only
	"fullScreen",   // read write
	"onResize"
};

static const char *const
moonlight_content_methods[] = {
	"findName",
	"createObject",
	"createFromXaml",
	"createFromXamlDownloader"
};

static bool
moonlight_content_has_property (NPObject *npobj, NPIdentifier name)
{
	return HAS_PROPERTY (moonlight_content_properties, name);
}

static bool
moonlight_content_has_method (NPObject *npobj, NPIdentifier name)
{
	return HAS_METHOD (moonlight_content_methods, name);
}

static bool
moonlight_content_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

	if (name_matches (name, "actualHeight")) {
		// not implemented correctly yet - these only have values in fullscreen mode
		INT32_TO_NPVARIANT (plugin->getActualHeight (), *result);
		return true;
	}
	else if (name_matches (name, "actualWidth")) {
		// not implemented correctly yet - these only have values in fullscreen mode
		INT32_TO_NPVARIANT (plugin->getActualWidth (), *result);
		return true;
	}
	else if (name_matches (name, "fullScreen")) {
		// not implemented yet.
		BOOLEAN_TO_NPVARIANT (false, *result);
		return true;
	}
	else if (name_matches (name, "onResize")) {
		// not implemented yet.
		NULL_TO_NPVARIANT (*result);
		return true;
	}

	return false;
}

static bool
moonlight_content_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	// not implemented yet.
	if (name_matches (name, "fullScreen")) {
		return true;
	}

	return false;
}

static bool
moonlight_content_invoke (NPObject *npobj, NPIdentifier name, 
			  const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	if (name_matches (name, "findName")) {
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

		if (!argCount)
			return true;

		char *name = (char *) NPVARIANT_TO_STRING (args[0]).utf8characters;
		
		DependencyObject *element = plugin->surface->GetToplevel()->FindName (name);
		if (!element)
			return true;

		MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, element);

		OBJECT_TO_NPVARIANT (depobj, *result);
		return true;
	}
	else if (name_matches (name, "createObject")) {
		// not implemented yet
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}
	else if (name_matches (name, "createFromXaml")) {
		// create a Control object

		if (argCount < 1)
			return true;

		char *xaml = (char *) NPVARIANT_TO_STRING (args[0]).utf8characters;

		Control *control = new Control ();
		Type::Kind element_type;
		control->InitializeFromXaml (xaml, &element_type);

		MoonlightDependencyObjectObject *depobj =
			DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, control);

		OBJECT_TO_NPVARIANT (depobj, *result);

		return true;
	}
	else if (name_matches (name, "createFromXamlDownloader")) {
		// not implemented yet
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}

	return false;
}

MoonlightContentType::MoonlightContentType ()
{
	hasProperty = moonlight_content_has_property;
	getProperty = moonlight_content_get_property;
	setProperty = moonlight_content_set_property;

	hasMethod = moonlight_content_has_method;
	invoke = moonlight_content_invoke;
}

MoonlightContentType* MoonlightContentClass;



/*** MoonlightDependencyObjectClass ***************************************************/

static void
value_to_variant (NPObject *npobj, Value *v, NPVariant *result)
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
		string_to_npvariant (v->AsString(), result);
		break;

	/* more builtins.. */
	default:
		if (v->GetKind () >= Type::DEPENDENCY_OBJECT) {
			MoonlightDependencyObjectObject *depobj =
				DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, v->AsDependencyObject ());
			OBJECT_TO_NPVARIANT (depobj, *result);
		}
		break;
	}
}

static const char *const
moonlight_dependency_object_methods [] = {
	"getHost",
	"captureMouse",
	"releaseMouseCapture",
	"addEventListener",
	"removeEventListener",
	"findName"
};

static NPObject*
moonlight_dependency_object_allocate (NPP instance, NPClass*)
{
	return new MoonlightDependencyObjectObject (instance);
}

static void
moonlight_dependency_object_deallocate (NPObject *npobj)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightDependencyObjectObject*)npobj;
}

static void
moonlight_dependency_object_invalidate (NPObject *npobj)
{
	MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*)npobj;

	if (depobj->dob)
		depobj->dob->unref ();
	depobj->dob = NULL;
}

static DependencyProperty*
_get_dependency_property (DependencyObject *obj, char *attrname)
{
	attrname[0] = toupper(attrname[0]);
	DependencyProperty *p = obj->GetDependencyProperty (attrname);

	if (p)
		return p;

	char *period = strchr (attrname, '.');
	if (period) {
		char *type_name = g_strndup (attrname, period-attrname);
		attrname = period + 1;

		Type* type = Type::Find (type_name);
		if (type != NULL) {
			p = dependency_property_lookup (type->type, attrname);
		}
		g_free (type_name);
	}

	return p;
}

static bool
moonlight_dependency_object_has_property (NPObject *npobj, NPIdentifier name)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	return (p != NULL);
}

static bool
moonlight_dependency_object_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);
	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	if (!p)
		return false;

	Value *value = dob->GetValue (p);
	if (!value)
		return false;

	value_to_variant (npobj, value, result);

	return true;
}

static bool 
moonlight_dependency_object_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);
	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	if (!p)
		return false;

	char *strvalue = (char *) NPN_MemAlloc (20);

	if (NPVARIANT_IS_BOOLEAN (*value)) {
		strcpy (strvalue, (NPVARIANT_TO_BOOLEAN (*value) ? "true" : "false"));
	} else if (NPVARIANT_IS_INT32 (*value)) {
		sprintf (strvalue, "%d", NPVARIANT_TO_INT32 (*value));
	} else if (NPVARIANT_IS_DOUBLE (*value)) {
		sprintf (strvalue, "%d", NPVARIANT_TO_DOUBLE (*value));
	} else if (NPVARIANT_IS_OBJECT(*value)) {
		// not implemented yet.
		DEBUG_WARN_NOTIMPLEMENTED ();
	}

	if (NPVARIANT_IS_STRING (*value))
		xaml_set_property_from_str (dob, p, (char *) NPVARIANT_TO_STRING (*value).utf8characters);
	else
		xaml_set_property_from_str (dob, p, strvalue);

	NPN_MemFree (strvalue);

	return true;
}

static bool
moonlight_dependency_object_has_method (NPObject *npobj, NPIdentifier name)
{
	return HAS_METHOD (moonlight_dependency_object_methods, name);
}

struct EventListenerProxy {
	NPP instance;
	NPObject *callback;
};

static void
proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy*)closure;

	//
	// XXX we need to proxy the event args (ugh!)
	//
	// for now just pass NULL for that arg.
	//

	NPVariant args[2];
	NPVariant result;

	MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (proxy->instance,
										 /* XXX ew */ (DependencyObject*)sender);

	NPN_RetainObject (depobj); // XXX

	OBJECT_TO_NPVARIANT (depobj, args[0]);
	//NULL_TO_NPVARIANT (args[1]);

	NPN_InvokeDefault(proxy->instance, proxy->callback, args, 1, &result);

	NPN_ReleaseVariantValue (&result);
}

static bool
moonlight_dependency_object_invoke (NPObject *npobj, NPIdentifier name,
				    const NPVariant *args, uint32_t argCount,
				    NPVariant *result)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "findName")) {
		if (!argCount)
			return true;

		char *name = (char *) NPVARIANT_TO_STRING (args[0]).utf8characters;
		
		DependencyObject *element = dob->FindName (name);
		if (!element)
			return true;

		MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, element);

		OBJECT_TO_NPVARIANT (depobj, *result);
		return true;
	}
	else if (name_matches (name, "getHost")) {
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

		OBJECT_TO_NPVARIANT ((NPObject*)plugin->getRootObject(), *result);
	}
	else if (name_matches (name, "addEventListener")) {
		char *name = g_strdup ((char *) NPVARIANT_TO_STRING (args[0]).utf8characters);

		name[0] = toupper(name[0]);

		EventListenerProxy *proxy = new EventListenerProxy ();
		proxy->instance = ((MoonlightObject*)npobj)->instance;
		proxy->callback = NPVARIANT_TO_OBJECT (args[1]);

		NPN_RetainObject (proxy->callback);

		dob->AddHandler (name, proxy_listener_to_javascript, proxy);

		g_free (name);

		return true;
	}
	else if (name_matches (name, "removeEventlistener")) {
		// not yet implemented
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}
	// XXX these next two methods should live in a UIElement
	// wrapper class, not in the DependencyObject wrapper.
	else if (name_matches (name, "captureMouse")) {
		// not yet implemented
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}
	else if (name_matches (name, "releaseMouseCapture")) {
		// not yet implemented
		DEBUG_WARN_NOTIMPLEMENTED ();
		return true;
	}
	else
		return MoonlightObjectClass->invoke (npobj, name,
						     args, argCount,
						     result);
}


MoonlightDependencyObjectType::MoonlightDependencyObjectType ()
{
	allocate = moonlight_dependency_object_allocate;
	deallocate = moonlight_dependency_object_deallocate;
	invalidate = moonlight_dependency_object_invalidate;

	hasProperty = moonlight_dependency_object_has_property;
	setProperty = moonlight_dependency_object_set_property;
	getProperty = moonlight_dependency_object_get_property;

	hasMethod = moonlight_dependency_object_has_method;
	invoke    = moonlight_dependency_object_invoke;
}

MoonlightDependencyObjectType* MoonlightDependencyObjectClass;


MoonlightDependencyObjectObject*
DependencyObjectCreateWrapper (NPP instance, DependencyObject *obj)
{
	NPClass *np_class;

	/* for DependencyObject subclasses which have special plugin classes, check here */
	if (Type::Find (obj->GetObjectType ())->IsSubclassOf (Type::COLLECTION))
		np_class = MoonlightCollectionClass;
	else {
		switch (obj->GetObjectType()) {
		case Type::STORYBOARD:
			np_class = MoonlightStoryboardClass;
			break;
		case Type::MEDIAELEMENT:
			np_class = MoonlightMediaElementClass;
			break;
		default:
			np_class = MoonlightDependencyObjectClass;
		}
	}

	MoonlightDependencyObjectObject *depobj
		= (MoonlightDependencyObjectObject*)NPN_CreateObject (instance,
								      np_class);

	depobj->SetDependencyObject (obj);

	return depobj;
}



/*** MoonlightCollectionClass ***************************************************/

static const char *const
moonlight_collection_properties [] = {
	"count"
};

static const char *const
moonlight_collection_methods [] = {
	"add",
	"remove",
	"insert",
	"clear",
	"getItem"
};

static bool
moonlight_collection_has_property (NPObject *npobj, NPIdentifier name)
{
	if (HAS_PROPERTY (moonlight_collection_properties, name))
		return true;

	return MoonlightDependencyObjectClass->hasProperty (npobj, name);
}

static bool
moonlight_collection_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	Collection *col = (Collection*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "count")) {
		INT32_TO_NPVARIANT (col->list->Length(), *result);	  
		return true;
	}
	else
		return MoonlightDependencyObjectClass->getProperty (npobj, name, result);

	return false;
}



static bool
moonlight_collection_has_method (NPObject *npobj, NPIdentifier name)
{
	if (HAS_METHOD (moonlight_collection_methods, name))
		return true;

	return MoonlightDependencyObjectClass->hasMethod (npobj, name);
}

static bool
moonlight_collection_invoke (NPObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result)
{
	Collection *col = (Collection*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "add")) {
		if (argCount < 1)
			return true;

		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT (args[0]);
		col->Add (el->dob);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "remove")) {
		if (argCount < 1)
			return true;

		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT (args[0]);
		col->Remove (el->dob);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "insert")) {
		if (argCount < 2)
			return true;

		int index = NPVARIANT_TO_INT32 (args[0]);
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT (args[1]);

		col->Insert (index, el->dob);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "clear")) {
		if (argCount != 0)
		  return true;

		col->Clear ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "getItem")) {
		if (argCount < 1)
			return true;

		int index = NPVARIANT_TO_INT32 (args[0]);

		if (index < 0 || index >= col->list->Length())
			return true;

		Collection::Node *n = (Collection::Node*)col->list->Index (index);

		MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance,
											 n->obj);
		
		OBJECT_TO_NPVARIANT ((NPObject*)depobj, *result);

		return true;
	}
	else
		return MoonlightDependencyObjectClass->invoke (npobj, name,
							       args, argCount,
							       result);
}


MoonlightCollectionType::MoonlightCollectionType ()
{
	hasProperty = moonlight_collection_has_property;
	getProperty = moonlight_collection_get_property;
	hasMethod = moonlight_collection_has_method;
	invoke = moonlight_collection_invoke;
}

MoonlightCollectionType* MoonlightCollectionClass;


/*** MoonlightStoryboardClass ***************************************************/

static const char *const
moonlight_storyboard_methods [] = {
	"begin",
	"pause",
	"resume",
	"seek",
	"stop"
};


static bool
moonlight_storyboard_has_method (NPObject *npobj, NPIdentifier name)
{
	if (HAS_METHOD (moonlight_storyboard_methods, name))
		return true;

	return MoonlightDependencyObjectClass->hasMethod (npobj, name);
}

static bool
moonlight_storyboard_invoke (NPObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result)
{
	Storyboard *sb = (Storyboard*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "begin")) {
		if (argCount != 0)
			return true;

		sb->Begin ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "pause")) {
		if (argCount != 0)
			return true;

		sb->Pause ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "resume")) {
		if (argCount != 0)
			return true;

		sb->Resume ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "seek")) {
		// not yet implemented
		DEBUG_WARN_NOTIMPLEMENTED ();
#if notyet
		if (argCount != 1)
			return true;

		sb->Seek (...);

		VOID_TO_NPVARIANT (*result);
#endif
		return true;
	}
	else if (name_matches (name, "stop")) {
		if (argCount != 0)
			return true;

		sb->Stop ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else
		return MoonlightDependencyObjectClass->invoke (npobj, name,
							       args, argCount,
							       result);
}

MoonlightStoryboardType::MoonlightStoryboardType ()
{
	hasMethod = moonlight_storyboard_has_method;
	invoke = moonlight_storyboard_invoke;
}

MoonlightStoryboardType* MoonlightStoryboardClass;

/*** MoonlightMediaElementClass ***************************************************/

static const char *const
moonlight_media_element_methods [] = {
	"stop",
	"play",
	"pause"
};


static bool
moonlight_media_element_has_method (NPObject *npobj, NPIdentifier name)
{
	if (HAS_METHOD (moonlight_media_element_methods, name))
		return true;

	return MoonlightDependencyObjectClass->hasMethod (npobj, name);
}

static bool
moonlight_media_element_invoke (NPObject *npobj, NPIdentifier name,
				const NPVariant *args, uint32_t argCount,
				NPVariant *result)
{
	MediaElement *media = (MediaElement*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "play")) {
		if (argCount != 0)
			return true;

		media->Play ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "pause")) {
		if (argCount != 0)
			return true;

		media->Pause ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "stop")) {
		if (argCount != 0)
			return true;

		media->Stop ();

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else 
		return MoonlightDependencyObjectClass->invoke (npobj, name,
							       args, argCount,
							       result);
}

MoonlightMediaElementType::MoonlightMediaElementType ()
{
	hasMethod = moonlight_media_element_has_method;
	invoke = moonlight_media_element_invoke;
}

MoonlightMediaElementType* MoonlightMediaElementClass;


void
plugin_init_classes ()
{
	MoonlightPointClass = new MoonlightPointType ();
	MoonlightObjectClass = new MoonlightObjectType ();
	MoonlightControlClass = new MoonlightControlType ();
	MoonlightContentClass = new MoonlightContentType ();
	MoonlightSettingsClass = new MoonlightSettingsType ();
	MoonlightDependencyObjectClass = new MoonlightDependencyObjectType ();
	MoonlightCollectionClass = new MoonlightCollectionType ();
	MoonlightStoryboardClass = new MoonlightStoryboardType ();
	MoonlightMediaElementClass = new MoonlightMediaElementType ();
}

