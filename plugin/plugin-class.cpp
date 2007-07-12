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

// XXXXXXXXXXx
//
// we leak a lot in this file.
//
// all of the wrapper objects we create are leaked, since for some
// reason delete'ing them doesn't work.
//
// Also, the listener proxies we create for dealing with events are
// leaked.  we need to figure out how to deal with them (where to
// store them so we can free them on removeListener, etc.)
// 

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


typedef void (*EventArgsWrapper)(NPP instance, gpointer calldata, NPVariant *value);

struct EventListenerProxy {
	NPP instance;
	NPObject *callback;
	EventArgsWrapper event_args_wrapper;
};

static void
default_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	NULL_TO_NPVARIANT (*value);
}

static void
mouse_event_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	MouseEventArgs *ea = (MouseEventArgs*)calldata;
	MoonlightMouseEventArgsObject *jsea = (MoonlightMouseEventArgsObject*)NPN_CreateObject (instance, MoonlightMouseEventArgsClass);
	MouseEventArgsPopulate (jsea, ea);

	OBJECT_TO_NPVARIANT (jsea, *value);
}

static void
proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy*)closure;

	NPVariant args[2];
	NPVariant result;

	MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (proxy->instance,
										 /* XXX ew */ (DependencyObject*)sender);

	NPN_RetainObject (depobj); // XXX leak?

	OBJECT_TO_NPVARIANT (depobj, args[0]);
	proxy->event_args_wrapper (proxy->instance, calldata, &args[1]);

	if (NPN_InvokeDefault(proxy->instance, proxy->callback, args, 2, &result))
		NPN_ReleaseVariantValue (&result);
}

static EventArgsWrapper
get_wrapper_for_event_name (const char *event_name)
{
	if (!g_strcasecmp ("mousemove", event_name) ||
	    !g_strcasecmp ("mouseleftbuttonup", event_name) ||
	    !g_strcasecmp ("mouseleftbuttondown", event_name) ||
	    !g_strcasecmp ("mouseenter", event_name)) {

		return mouse_event_wrapper;
	}
	// XXX need to handle key events
	else {
		return default_wrapper;
	}
}


/*** Points ***/
static NPObject*
point_allocate (NPP instance, NPClass *)
{
	return new MoonlightPoint (instance);
}

static void
point_deallocate (NPObject *npobject)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightPoint*)npobject;
}

static bool
point_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "x") ||
		name_matches (name, "y"));
}

static bool
point_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightPoint *p = (MoonlightPoint*)npobj;
	if (name_matches (name, "x")) {
		DOUBLE_TO_NPVARIANT (p->point.x, *result);
		return true;
	}
	else if (name_matches (name, "y")) {
		DOUBLE_TO_NPVARIANT (p->point.y, *result);
		return true;
	}
	else
		return false;
}

static bool
point_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightPoint *p = (MoonlightPoint*)npobj;
	if (name_matches (name, "x")) {
		p->point.x = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else if (name_matches (name, "y")) {
		p->point.y = NPVARIANT_TO_DOUBLE (*value);
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
}

MoonlightPointType *MoonlightPointClass;

/*** Rects ***/
static NPObject*
rect_allocate (NPP instance, NPClass *)
{
	return new MoonlightRect (instance);
}

static void
rect_deallocate (NPObject *npobject)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightRect*)npobject;
}

static bool
rect_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "x") ||
		name_matches (name, "y") ||
		name_matches (name, "width") ||
		name_matches (name, "height"));
}

static bool
rect_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightRect *r = (MoonlightRect*)npobj;
	if (name_matches (name, "x")) {
		DOUBLE_TO_NPVARIANT (r->rect.x, *result);
		return true;
	}
	else if (name_matches (name, "y")) {
		DOUBLE_TO_NPVARIANT (r->rect.y, *result);
		return true;
	}
	else if (name_matches (name, "width")) {
		DOUBLE_TO_NPVARIANT (r->rect.w, *result);
		return true;
	}
	else if (name_matches (name, "height")) {
		DOUBLE_TO_NPVARIANT (r->rect.h, *result);
		return true;
	}
	else
		return false;
}

static bool
rect_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightRect *r = (MoonlightRect*)npobj;
	if (name_matches (name, "x")) {
		r->rect.x = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else if (name_matches (name, "y")) {
		r->rect.y = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else if (name_matches (name, "width")) {
		r->rect.w = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else if (name_matches (name, "height")) {
		r->rect.h = NPVARIANT_TO_DOUBLE (*value);
		return true;
	}
	else
		return false;
}


MoonlightRectType::MoonlightRectType ()
{
	allocate = rect_allocate;
	deallocate = rect_deallocate;
	hasProperty = rect_has_property;
	getProperty = rect_get_property;
	setProperty = rect_set_property;
}

MoonlightRectType *MoonlightRectClass;

/*** MoonlightMouseEventArgsClass  **************************************************************/

static NPObject*
mouse_event_allocate (NPP instance, NPClass *)
{
	return new MoonlightMouseEventArgsObject (instance);
}

static void
mouse_event_deallocate (NPObject *npobject)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightMouseEventArgs*)npobject;
}

static void
mouse_event_invalidate (NPObject *npobject)
{
	MoonlightMouseEventArgsObject *ea = (MoonlightMouseEventArgsObject*)npobject;
	if (ea->position)
		NPN_ReleaseObject (ea->position);
	ea->position = NULL;
}

static bool
mouse_event_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "shift") ||
		name_matches (name, "ctrl"));
}

static bool
mouse_event_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightRect *r = (MoonlightRect*)npobj;
	if (name_matches (name, "shift")) {
		DEBUG_WARN_NOTIMPLEMENTED ();
		BOOLEAN_TO_NPVARIANT (false, *result);
		return true;
	}
	else if (name_matches (name, "ctrl")) {
		DEBUG_WARN_NOTIMPLEMENTED ();
		BOOLEAN_TO_NPVARIANT (false, *result);
		return true;
	}
	else
		return false;
}

static bool
mouse_event_has_method (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "getPosition"));
}

static bool
mouse_event_invoke (NPObject *npobj, NPIdentifier name,
		    const NPVariant *args, uint32_t argCount,
		    NPVariant *result)
{
	MoonlightMouseEventArgsObject *ea = (MoonlightMouseEventArgsObject*)npobj;

	if (name_matches (name, "getPosition")) {
		if (argCount != 1)
			return true;

		// XXX we need to handle the arg, it'll be an element
		// to calculate the position with respect to (or null
		// for screen space)

		NPN_RetainObject (ea->position);
		
		MoonlightPoint* point = (MoonlightPoint*)ea->position;

		OBJECT_TO_NPVARIANT (ea->position, *result);

		return true;
	}
	else
		return false;
}


MoonlightMouseEventArgsType::MoonlightMouseEventArgsType ()
{
	allocate = mouse_event_allocate;
	deallocate = mouse_event_deallocate;
	invalidate = mouse_event_invalidate;

	hasProperty = mouse_event_has_property;
	getProperty = mouse_event_get_property;

	hasMethod = mouse_event_has_method;
	invoke = mouse_event_invoke;
}

MoonlightMouseEventArgsType* MoonlightMouseEventArgsClass;

void
MouseEventArgsPopulate (MoonlightMouseEventArgsObject *ea, MouseEventArgs *args)
{
	ea->state = args->state;

	MoonlightPoint *point = (MoonlightPoint*)NPN_CreateObject (((MoonlightObject*)ea)->instance, MoonlightPointClass);

	point->point = Point (args->x, args->y);
	ea->position = point;
}


/*** our object base class */
NPObject*
_allocate (NPP instance, NPClass*)
{
	return new MoonlightObject (instance);
}

static void
_deallocate (NPObject *npobj)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightObject*)npobj;
}

static void
_invalidate (NPObject *npobj)
{
	// nothing to do
}

static bool
_has_method (NPObject *npobj, NPIdentifier name)
{
	return false;
}

static bool
_has_property (NPObject *npobj, NPIdentifier name)
{
	return false;
}

static bool
_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	g_warning ("moonlight_object_get_property reached");
	return false;
}

static bool
_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	g_warning ("moonlight_object_set_property reached");
	return false;
}

static bool
_remove_property (NPObject *npobj, NPIdentifier name)
{
	g_warning ("moonlight_object_remove_property reached");
	return false;
}

static bool
_invoke (NPObject *npobj, NPIdentifier name,
	 const NPVariant *args, uint32_t argCount,
	 NPVariant *result)
{
	g_warning ("moonlight_object_invoke reached");
	return false;
}

static bool
_invoke_default (NPObject *npobj,
		 const NPVariant *args, uint32_t argCount,
		 NPVariant *result)
{
	g_warning ("moonlight_object_invoke_default reached");
	return false;
}

MoonlightObjectType::MoonlightObjectType ()
{
	allocate       = _allocate;
	deallocate     = _deallocate;
	invalidate     = _invalidate;
	hasMethod      = _has_method;
	invoke         = _invoke;
	invokeDefault  = _invoke_default;
	hasProperty    = _has_property;
	getProperty    = _get_property;
	setProperty    = _set_property;
	removeProperty = _remove_property;
}

MoonlightObjectType* MoonlightObjectClass;

/*** MoonlightControlClass **********************************************************/
static NPObject*
moonlight_control_allocate (NPP instance, NPClass*)
{
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
	return (name_matches (name, "settings") ||
		name_matches (name, "content") ||
		name_matches (name, "initParams") ||
		name_matches (name, "isLoaded") ||
		name_matches (name, "source"));
}

static bool
moonlight_control_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
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

static bool
moonlight_control_has_method (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "createObject"));
}

static bool
moonlight_control_invoke (NPObject *npobj, NPIdentifier name,
			  const NPVariant *args, uint32_t argCount,
			  NPVariant *result)
{
	if (name_matches (name, "createObject")) {
	  printf ("createObject!!$&^!$\n");
		if (argCount != 1 || !NPVARIANT_IS_STRING(args[0])) {
			NULL_TO_NPVARIANT (*result);
			printf ("+ arg mismatch\n");
			return true;
		}

		NPObject *obj = NULL;
		const char *object_type = NPVARIANT_TO_STRING (args[0]).utf8characters;
		if (!g_strcasecmp ("downloader", object_type)) {

			Downloader *dl = new Downloader ();


			obj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, dl);

			printf ("+ downloader = %p\n", obj);
			OBJECT_TO_NPVARIANT (obj, *result);
			return true;
		}
		else {
			printf ("+ not downloader, bah\n");
			// XXX the docs say we're supposed to throw an exception here,
			// since downloader is the only type you can create.
			NULL_TO_NPVARIANT (*result);
			return true;
		}
	}
	else
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

	hasMethod   = moonlight_control_has_method;
	invoke      = moonlight_control_invoke;
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
	if (name_matches (name, "fullScreen")) {
		// not implemented yet.
		return true;
	}
	else if (name_matches (name, "onResize")) {
#if notyet
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

		if (!NPVARIANT_IS_OBJECT (*value)) {
			DEBUG_WARN_NOTIMPLEMENTED ();
			return true;
		}

		EventListenerProxy *proxy = new EventListenerProxy ();
		proxy->instance = ((MoonlightObject*)npobj)->instance;
		proxy->callback = NPVARIANT_TO_OBJECT (*value);
		proxy->event_args_wrapper = get_wrapper_for_event_name ("Resize");

		NPN_RetainObject (proxy->callback);

		plugin->surface->AddHandler ("Resize", proxy_listener_to_javascript, proxy);
#endif		
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
		
		printf ("findName (%s)\n", name);
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

	case Type::POINT: {
		MoonlightPoint *point = (MoonlightPoint*)NPN_CreateObject (((MoonlightObject*)npobj)->instance, MoonlightPointClass);
		point->point = *v->AsPoint ();
		break;
	}

	case Type::RECT: {
		MoonlightRect *rect = (MoonlightRect*)NPN_CreateObject (((MoonlightObject*)npobj)->instance, MoonlightRectClass);
		rect->rect = *v->AsRect ();
		break;
	}

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
			attrname[0] = toupper (attrname[0]);
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

	if (NPVARIANT_IS_OBJECT (*value)) {
		MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT (*value);
		dob->SetValue (p, Value(depobj->dob));
	}
	else {
		char *strvalue;

		if (NPVARIANT_IS_BOOLEAN (*value)) {
			strvalue = g_strdup (NPVARIANT_TO_BOOLEAN (*value) ? "true" : "false");
		} else if (NPVARIANT_IS_INT32 (*value)) {
			strvalue = g_strdup_printf ("%d", NPVARIANT_TO_INT32 (*value));
		} else if (NPVARIANT_IS_DOUBLE (*value)) {
			strvalue = g_strdup_printf ("%g", NPVARIANT_TO_DOUBLE (*value));
		}
		else if (!NPVARIANT_IS_STRING (*value)) {
			DEBUG_WARN_NOTIMPLEMENTED ();
			return true;
		}

		if (NPVARIANT_IS_STRING (*value))
			xaml_set_property_from_str (dob, p, (char *) NPVARIANT_TO_STRING (*value).utf8characters);
		else
			xaml_set_property_from_str (dob, p, strvalue);

		g_free (strvalue);
	}

	return true;
}

static bool
moonlight_dependency_object_has_method (NPObject *npobj, NPIdentifier name)
{
	return HAS_METHOD (moonlight_dependency_object_methods, name);
}

static bool
moonlight_dependency_object_invoke (NPObject *npobj, NPIdentifier name,
				    const NPVariant *args, uint32_t argCount,
				    NPVariant *result)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "findName")) {
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

		if (!argCount)
			return true;

		char *name = (char *) NPVARIANT_TO_STRING (args[0]).utf8characters;
		
		printf ("findName (%s)\n", name);
		DependencyObject *element = plugin->surface->GetToplevel()->FindName (name);
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
		proxy->event_args_wrapper = get_wrapper_for_event_name (name);

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
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->CaptureMouse (), *result);
		return true;
	}
	else if (name_matches (name, "releaseMouseCapture")) {
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->ReleaseMouseCapture (), *result);
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
		case Type::DOWNLOADER:
			np_class = MoonlightDownloaderClass;
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

/*** MoonlightDownloaderClass ***************************************************/

static const char *const
moonlight_downloader_methods [] = {
	"abort",
	"getResponseText",
	"open",
	"send"
};

static bool
moonlight_downloader_has_method (NPObject *npobj, NPIdentifier name)
{
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	printf ("moonlight_downloadeR_has_method (%s)\n", strname);

	if (HAS_METHOD (moonlight_downloader_methods, name))
		return true;

	return MoonlightDependencyObjectClass->hasMethod (npobj, name);
}

static bool
moonlight_downloader_invoke (NPObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result)
{
	Downloader *dl = (Downloader*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "abort")) {
		if (argCount != 0)
			return true;

		downloader_abort (dl);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "open")) {
	  printf ("OPEN!\n");
		if (argCount > 3)
			return true;

		const char *verb = NPVARIANT_TO_STRING (args[0]).utf8characters;
		const char *uri = NPVARIANT_TO_STRING (args[1]).utf8characters;

		downloader_open (dl, (char*)verb, (char*)uri, true);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "send")) {
		if (argCount != 0)
			return true;

		downloader_send (dl);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else if (name_matches (name, "getResponseText")) {
		if (argCount != 1)
			return true;

		const char *part_name = NPVARIANT_TO_STRING (args[0]).utf8characters;

		uint64_t size;
		char* buf = (char*)downloader_get_response_text (dl, (char*)part_name, &size);

		if (buf) {
			char *s = (char*)NPN_MemAlloc (size);
			memcpy (s, buf, size);
			STRINGN_TO_NPVARIANT (s, size, *result);
			g_free (buf);
		}
		else
			NULL_TO_NPVARIANT (*result);

		return true;
	}
	else 
		return MoonlightDependencyObjectClass->invoke (npobj, name,
							       args, argCount,
							       result);
}

MoonlightDownloaderType::MoonlightDownloaderType ()
{
	hasMethod = moonlight_downloader_has_method;
	invoke = moonlight_downloader_invoke;
}

MoonlightDownloaderType* MoonlightDownloaderClass;

/*** MoonlightScriptableObjectClass ***************************************************/

static NPObject*
moonlight_scriptable_object_allocate (NPP instance, NPClass*)
{
	return new MoonlightScriptableObjectObject (instance);
}

static void
moonlight_scriptable_object_deallocate (NPObject *npobj)
{
	// XXX is delete broken in plugins?
	// delete (MoonlightScriptableObjectObject*)npobj;
}

static void
moonlight_scriptable_object_invalidate (NPObject *npobj)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;

	if (sobj->scriptable) {
		// XXX unref the scriptable object however we need to.
	}
	sobj->scriptable = NULL;
}

static bool
moonlight_scriptable_object_has_property (NPObject *npobj, NPIdentifier name)
{
	DEBUG_WARN_NOTIMPLEMENTED ();
	return false;
}

static bool
moonlight_scriptable_object_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	DEBUG_WARN_NOTIMPLEMENTED ();
	return true;
}

static bool 
moonlight_scriptable_object_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	DEBUG_WARN_NOTIMPLEMENTED ();
	return true;
}

static bool
moonlight_scriptable_object_has_method (NPObject *npobj, NPIdentifier name)
{
	DEBUG_WARN_NOTIMPLEMENTED ();
	return false;
}

static bool
moonlight_scriptable_object_invoke (NPObject *npobj, NPIdentifier name,
				    const NPVariant *args, uint32_t argCount,
				    NPVariant *result)
{
	DEBUG_WARN_NOTIMPLEMENTED ();
	return true;
}


MoonlightScriptableObjectType::MoonlightScriptableObjectType ()
{
	allocate = moonlight_scriptable_object_allocate;
	deallocate = moonlight_scriptable_object_deallocate;
	invalidate = moonlight_scriptable_object_invalidate;

	hasProperty = moonlight_scriptable_object_has_property;
	setProperty = moonlight_scriptable_object_set_property;
	getProperty = moonlight_scriptable_object_get_property;

	hasMethod = moonlight_scriptable_object_has_method;
	invoke    = moonlight_scriptable_object_invoke;
}

MoonlightScriptableObjectType* MoonlightScriptableObjectClass;

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
	MoonlightDownloaderClass = new MoonlightDownloaderType ();
	MoonlightMouseEventArgsClass = new MoonlightMouseEventArgsType ();
	MoonlightScriptableObjectClass = new MoonlightScriptableObjectType ();
}

