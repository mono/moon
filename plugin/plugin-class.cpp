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
// The listener proxies we create for dealing with events are
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
string_to_npvariant (const char *value, NPVariant *result)
{
	char * retval;

	if (value)
		retval = PL_strdup (value);
	else
		retval = PL_strdup ("");

	STRINGZ_TO_NPVARIANT (retval, *result);
}

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

static void
variant_to_value (const NPVariant *v, Value **result)
{
	switch (v->type) {
	case NPVariantType_Bool:
		*result = new Value (NPVARIANT_TO_BOOLEAN (*v));
		break;
	case NPVariantType_Int32:
		*result = new Value ((gint32)NPVARIANT_TO_INT32(*v));
		break;
	case NPVariantType_Double:
		*result = new Value (NPVARIANT_TO_DOUBLE(*v));
		break;
	case NPVariantType_String:
		*result = new Value (NPVARIANT_TO_STRING(*v).utf8characters);
		break;
	case NPVariantType_Void:
		DEBUG_WARN_NOTIMPLEMENTED ("void variant type");
		*result = NULL;
		break;
	case NPVariantType_Null:
		DEBUG_WARN_NOTIMPLEMENTED ("null variant type");
		*result = new Value (Type::DEPENDENCY_OBJECT);
		break;
	case NPVariantType_Object:
		DEBUG_WARN_NOTIMPLEMENTED ("object variant type");
		*result = NULL;
		break;
	}
}


EventListenerProxy::EventListenerProxy (NPP instance, const char *event_name, const char *cb_name)
{
	this->instance = instance;
	this->event_name = g_strdup (event_name);

	this->is_func = false;
	if (!strncmp (cb_name, "javascript:", strlen ("javascript:")))
	  cb_name += strlen ("javascript:");
	this->callback = g_strdup (cb_name);

	printf ("returning event listener proxy from %s - > %s\n", event_name, cb_name);
}

EventListenerProxy::EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb)
{
	this->instance = instance;
	this->event_name = g_strdup (event_name);

	if (NPVARIANT_IS_OBJECT (*cb)) {
		this->is_func = true;
		this->callback = NPVARIANT_TO_OBJECT (*cb);
		NPN_RetainObject ((NPObject*)this->callback);
	}
	else {
		this->is_func = false;
		this->callback = g_strdup (NPVARIANT_TO_STRING (*cb).utf8characters);
	}
}

EventListenerProxy::~EventListenerProxy ()
{
	if (is_func)
		NPN_ReleaseObject ((NPObject*)this->callback);
	else
		g_free ((char*)this->callback);

	g_free (event_name);
}

void
EventListenerProxy::AddHandler (EventObject *obj)
{
	obj->AddHandler (event_name, proxy_listener_to_javascript, this);
}

void
EventListenerProxy::RemoveHandler (EventObject *obj)
{
	obj->RemoveHandler (event_name, proxy_listener_to_javascript, this);
}

EventArgsWrapper
EventListenerProxy::get_wrapper_for_event_name (const char *event_name)
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

void
EventListenerProxy::default_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	NULL_TO_NPVARIANT (*value);
}

void
EventListenerProxy::mouse_event_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	MouseEventArgs *ea = (MouseEventArgs*)calldata;
	MoonlightMouseEventArgsObject *jsea = (MoonlightMouseEventArgsObject*)NPN_CreateObject (instance, MoonlightMouseEventArgsClass);
	MouseEventArgsPopulate (jsea, ea);

	OBJECT_TO_NPVARIANT (jsea, *value);
}

void
EventListenerProxy::proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy*)closure;

	NPVariant args[2];
	NPVariant result;

	MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (proxy->instance,
										 /* XXX ew */ (DependencyObject*)sender);

	NPN_RetainObject (depobj); // XXX leak?

	OBJECT_TO_NPVARIANT (depobj, args[0]);

	EventArgsWrapper event_args_wrapper = get_wrapper_for_event_name (proxy->event_name);

	event_args_wrapper (proxy->instance, calldata, &args[1]);

	if (proxy->is_func) {
		/* the event listener was added with a JS function object */
		if (NPN_InvokeDefault (proxy->instance, (NPObject*)proxy->callback, args, 2, &result))
			NPN_ReleaseVariantValue (&result);
	}
	else {
		/* the event listener was added with a JS string (the function name) */
		NPObject *object = NULL;
		if (NPERR_NO_ERROR != NPN_GetValue(proxy->instance, NPNVWindowNPObject, &object)) {
			return;
		}

		if (NPN_Invoke (proxy->instance, object, NPID ((char*)proxy->callback),
				args, 2, &result))
			NPN_ReleaseVariantValue (&result);

		// XXX not needed
		// NPN_ReleaseObject (object);
	}
}


void
event_object_add_javascript_listener (EventObject *obj, PluginInstance *plugin, const char *event_name, const char *cb_name)
{
	EventListenerProxy *proxy = new EventListenerProxy (plugin->getNPP(), event_name, cb_name);
	proxy->AddHandler (obj);
}

/*** ErrorEventArgs ***/
static NPObject*
erroreventargs_allocate (NPP instance, NPClass *)
{
	return new MoonlightErrorEventArgs (instance);
}

static void
erroreventargs_deallocate (NPObject *npobject)
{
	delete (MoonlightErrorEventArgs*)npobject;
}

static bool
erroreventargs_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "errorCode") ||
		name_matches (name, "errorType") ||
		name_matches (name, "errorMessage") ||
		name_matches (name, "lineNumber") ||
		name_matches (name, "charNumber") ||
		/* parser errors */
		name_matches (name, "xamlFile") ||
		/* runtime errors */
		name_matches (name, "methodName"));
}

static bool
erroreventargs_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightErrorEventArgs *ea = (MoonlightErrorEventArgs*)npobj;
	if (name_matches (name, "errorCode")) {
		INT32_TO_NPVARIANT (ea->args->error_code, *result);
		return true;
	}
	else if (name_matches (name, "errorType")) {
		switch (ea->args->error_type) {
		case NoError:          string_to_npvariant ("NoError", result); break;
		case UnknownError:     string_to_npvariant ("UnknownError", result); break;
		case InitializeError:  string_to_npvariant ("InitializeError", result); break;
		case ParserError:      string_to_npvariant ("ParserError", result); break;
		case ObjectModelError: string_to_npvariant ("ObjectModelError", result); break;
		case RuntimeError:     string_to_npvariant ("RuntimeError", result); break;
		case DownloadError:    string_to_npvariant ("DownloadError", result); break;
		case MediaError:       string_to_npvariant ("MediaError", result); break;
		case ImageError:       string_to_npvariant ("ImageError", result); break;
		}
		return true;
	}
	else if (name_matches (name, "ErrorMessage")) {
		string_to_npvariant (ea->args->error_message, result);
		return true;
	}
	else if (name_matches (name, "lineNumber")) {
		if (ea->args->error_type == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)ea->args)->line_number, *result);
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.lineNumber");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;
	}
	else if (name_matches (name, "charPosition")) {
		if (ea->args->error_type == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)ea->args)->char_position, *result);
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.charPosition");
			INT32_TO_NPVARIANT (0, *result);
			return true;
		}
	}
	else if (name_matches (name, "methodName")) {
		DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.methodName");
		INT32_TO_NPVARIANT (0, *result);
		return true;
	}
	else if (name_matches (name, "xamlFile")) {
		if (ea->args->error_type == ParserError) {
			string_to_npvariant (((ParserErrorEventArgs*)ea->args)->xaml_file, result);
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.xamlFile");
			NULL_TO_NPVARIANT (*result);
			return true;
		}
	}
	else
		return false;
}

MoonlightErrorEventArgsType::MoonlightErrorEventArgsType ()
{
	allocate = erroreventargs_allocate;
	deallocate = erroreventargs_deallocate;
	hasProperty = erroreventargs_has_property;
	getProperty = erroreventargs_get_property;
}

MoonlightErrorEventArgsType *MoonlightErrorEventArgsClass;

/*** Points ***/
static NPObject*
point_allocate (NPP instance, NPClass *)
{
	return new MoonlightPoint (instance);
}

static void
point_deallocate (NPObject *npobject)
{
	delete (MoonlightPoint*)npobject;
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
	delete (MoonlightRect*)npobject;
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
	delete (MoonlightMouseEventArgsObject*)npobject;
}

static void
mouse_event_invalidate (NPObject *npobject)
{
	MoonlightMouseEventArgsObject *ea = (MoonlightMouseEventArgsObject*)npobject;
// XXX apparently we don't need to do this?
//  	if (ea->position)
//  		NPN_ReleaseObject (ea->position);
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
		DEBUG_WARN_NOTIMPLEMENTED ("shift MouseEvent property");
		BOOLEAN_TO_NPVARIANT (false, *result);
		return true;
	}
	else if (name_matches (name, "ctrl")) {
		DEBUG_WARN_NOTIMPLEMENTED ("ctrl MouseEvent property");
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
	delete (MoonlightObject*)npobj;
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

/*** MoonlightScriptControlClass **********************************************************/
static NPObject*
moonlight_scriptable_control_allocate (NPP instance, NPClass*)
{
	return new MoonlightScriptControlObject (instance);
}

static void
moonlight_scriptable_control_deallocate (NPObject *npobj)
{
	delete (MoonlightScriptControlObject*)npobj;
}

static void
moonlight_scriptable_control_invalidate (NPObject *npobj)
{
	MoonlightScriptControlObject *control = (MoonlightScriptControlObject*)npobj;

// XXX apparently we don't need to do this?
// 	if (control->settings)
// 		NPN_ReleaseObject (control->settings);
	control->settings = NULL;

// XXX apparently we don't need to do this?
// 	if (control->content)
// 		NPN_ReleaseObject (control->content);
	control->content = NULL;
}

static bool
moonlight_scriptable_control_has_property (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "settings") ||
		name_matches (name, "content") ||
		name_matches (name, "initParams") ||
		name_matches (name, "isLoaded") ||
		name_matches (name, "source"));
}

static bool
moonlight_scriptable_control_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;
	MoonlightScriptControlObject *rootobj = (MoonlightScriptControlObject*)npobj;

	if (name_matches (name, "settings")) {
		NPN_RetainObject (rootobj->settings);
		OBJECT_TO_NPVARIANT (rootobj->settings, *result);
		return true;
	} 

	if (name_matches (name, "content")) {
		NPN_RetainObject (rootobj->content);
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
moonlight_scriptable_control_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

	if (name_matches (name, "source")) {
		plugin->setSource (NPVARIANT_TO_STRING (*value).utf8characters);
		return true;
	}

	return false;
}

static bool
moonlight_scriptable_control_has_method (NPObject *npobj, NPIdentifier name)
{
	return (name_matches (name, "createObject"));
}

static bool
moonlight_scriptable_control_invoke (NPObject *npobj, NPIdentifier name,
			  const NPVariant *args, uint32_t argCount,
			  NPVariant *result)
{
	if (name_matches (name, "createObject")) {
		if (argCount != 1 || !NPVARIANT_IS_STRING(args[0])) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		NPObject *obj = NULL;
		const char *object_type = NPVARIANT_TO_STRING (args[0]).utf8characters;
		if (!g_strcasecmp ("downloader", object_type)) {

			Downloader *dl = new Downloader ();


			obj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance, dl);

			OBJECT_TO_NPVARIANT (obj, *result);
			return true;
		}
		else {
			// XXX the docs say we're supposed to throw an exception here,
			// since downloader is the only type you can create.
			NULL_TO_NPVARIANT (*result);
			return true;
		}
	}
	else
		return false;
}

MoonlightScriptControlType::MoonlightScriptControlType ()
{
	allocate = moonlight_scriptable_control_allocate;
	deallocate = moonlight_scriptable_control_deallocate;
	invalidate = moonlight_scriptable_control_invalidate;

	hasProperty = moonlight_scriptable_control_has_property;
	getProperty = moonlight_scriptable_control_get_property;
	setProperty = moonlight_scriptable_control_set_property;

	hasMethod   = moonlight_scriptable_control_has_method;
	invoke      = moonlight_scriptable_control_invoke;
}

MoonlightScriptControlType* MoonlightScriptControlClass;

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
		DEBUG_WARN_NOTIMPLEMENTED ("maxFrameRate property");
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
static NPObject*
moonlight_content_allocate (NPP instance, NPClass*)
{
	return new MoonlightContentObject (instance);
}

static void
moonlight_content_deallocate (NPObject *npobj)
{
	delete (MoonlightContentObject*)npobj;
}

static void
moonlight_content_invalidate (NPObject *npobj)
{
	MoonlightContentObject *content = (MoonlightContentObject*)npobj;

	/* XXX free the registered_scriptable_objects hash */
	DEBUG_WARN_NOTIMPLEMENTED ("need to free registered scriptable objects");

	if (content->resizeProxy)
		delete content->resizeProxy;
	content->resizeProxy = NULL;
}

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
	if (HAS_PROPERTY (moonlight_content_properties, name))
		return true;

	MoonlightContentObject *content = (MoonlightContentObject*)npobj;

	gpointer p = g_hash_table_lookup (content->registered_scriptable_objects,
					  name);

	return p != NULL;
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
	else {
		MoonlightContentObject *content = (MoonlightContentObject*)npobj;
		MoonlightScriptableObjectObject *obj =
			(MoonlightScriptableObjectObject*)g_hash_table_lookup (content->registered_scriptable_objects, name);
		if (obj == NULL)
			return false;

		NPN_RetainObject (obj);
		OBJECT_TO_NPVARIANT (obj, *result);
		return true;
	}
}

static bool
moonlight_content_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	if (name_matches (name, "fullScreen")) {
		// not implemented yet.
		return true;
	}
	else if (name_matches (name, "onResize")) {
#if notyet /* XXX Surface needs to inherit from DependencyObject */
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;
		EventListenerProxy *proxy = new EventListenerProxy (((MoonlightObject*)npobj)->instance,
								    "Resize",
								    value);

		proxy->AddHandler (plugin->surface);

		// XXX store the proxy someplace in this object
#endif
		DEBUG_WARN_NOTIMPLEMENTED ("content onResize");
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
		DEBUG_WARN_NOTIMPLEMENTED ("content.createObject");
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
		DEBUG_WARN_NOTIMPLEMENTED ("content.createFromXamlDownloader");
		return true;
	}

	return false;
}

MoonlightContentType::MoonlightContentType ()
{
	allocate = moonlight_content_allocate;
	deallocate = moonlight_content_deallocate;
	invalidate = moonlight_content_invalidate;

	hasProperty = moonlight_content_has_property;
	getProperty = moonlight_content_get_property;
	setProperty = moonlight_content_set_property;

	hasMethod = moonlight_content_has_method;
	invoke = moonlight_content_invoke;
}

MoonlightContentType* MoonlightContentClass;



/*** MoonlightDependencyObjectClass ***************************************************/

static const char *const
moonlight_dependency_object_methods [] = {
	"getHost",
	"getParent",
	"captureMouse",
	"releaseMouseCapture",
	"addEventListener",
	"removeEventListener",
	"findName",
	"setValue",
	"getValue"
};

static NPObject*
moonlight_dependency_object_allocate (NPP instance, NPClass*)
{
	return new MoonlightDependencyObjectObject (instance);
}

static void
moonlight_dependency_object_deallocate (NPObject *npobj)
{
	delete (MoonlightDependencyObjectObject*)npobj;
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
_set_dependency_property_value (DependencyObject *dob, DependencyProperty *p, const NPVariant *value)
{

	if (NPVARIANT_IS_OBJECT (*value)) {
		MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT (*value);
		dob->SetValue (p, Value(depobj->dob));
	}
	else {
		char *strvalue = NULL;

		if (NPVARIANT_IS_BOOLEAN (*value)) {
			strvalue = g_strdup (NPVARIANT_TO_BOOLEAN (*value) ? "true" : "false");
		}
		else if (NPVARIANT_IS_INT32 (*value)) {
			strvalue = g_strdup_printf ("%d", NPVARIANT_TO_INT32 (*value));
		}
		else if (NPVARIANT_IS_DOUBLE (*value)) {
			strvalue = g_strdup_printf ("%g", NPVARIANT_TO_DOUBLE (*value));
		}
		else if (NPVARIANT_IS_STRING (*value)) {
			strvalue = g_strdup (NPVARIANT_TO_STRING (*value).utf8characters);
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("unhandled variant type in do.set_property");
			return true;
		}

		xaml_set_property_from_str (dob, p, strvalue);

		g_free (strvalue);
	}

	return true;
}


static bool
moonlight_dependency_object_has_property (NPObject *npobj, NPIdentifier name)
{
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)npobj)->dob;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);
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

	return _set_dependency_property_value (dob, p, value);
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

	if (name_matches (name, "setValue")) {
		/* obj.setValue (prop, val) is another way of writing obj[prop] = val (or obj.prop = val) */
		if (argCount < 2
		    || !NPVARIANT_IS_STRING (args[0])) {
			return true;
		}

		moonlight_dependency_object_set_property (npobj,
							  NPID (NPVARIANT_TO_STRING (args[0]).utf8characters),
							  &args[1]);

		VOID_TO_NPVARIANT (*result);
		return true;
	}
	else if (name_matches (name, "getValue")) {
		if (argCount < 2
		    || !NPVARIANT_IS_STRING (args[0])) {
			return true;
		}

		moonlight_dependency_object_get_property (npobj,
							  NPID (NPVARIANT_TO_STRING (args[0]).utf8characters),
							  result);
		return true;
	}
	else if (name_matches (name, "findName")) {
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
	else if (name_matches (name, "getHost")) {
		PluginInstance *plugin = (PluginInstance*) ((MoonlightObject*)npobj)->instance->pdata;

		OBJECT_TO_NPVARIANT ((NPObject*)plugin->getRootObject(), *result);

		return true;
	}
	else if (name_matches (name, "getParent")) {
		MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (((MoonlightObject*)npobj)->instance,
											 dob->GetParent());

		OBJECT_TO_NPVARIANT (depobj, *result);

		return true;
	}
	else if (name_matches (name, "addEventListener")) {
		if (argCount != 2)
			return true;

		if (!NPVARIANT_IS_STRING (args[0])
		    || (!NPVARIANT_IS_STRING (args[1]) && !NPVARIANT_IS_OBJECT (args[1]))) {
			/* XXX how do we check if args[1] is a function? */
			return true;
		}
		char *name = g_strdup ((char *) NPVARIANT_TO_STRING (args[0]).utf8characters);

		name[0] = toupper(name[0]);

		EventListenerProxy *proxy = new EventListenerProxy (((MoonlightObject*)npobj)->instance,
								    name,
								    &args[1]);

		proxy->AddHandler (dob);

		g_free (name);

		return true;
	}
	else if (name_matches (name, "removeEventlistener")) {
		// not yet implemented
		DEBUG_WARN_NOTIMPLEMENTED ("do.removeEventListener");
		return true;
	}
	// XXX these next two methods should live in a UIElement
	// wrapper class, not in the DependencyObject wrapper.
	else if (name_matches (name, "captureMouse")) {
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->CaptureMouse (), *result);
		return true;
	}
	else if (name_matches (name, "releaseMouseCapture")) {
		((UIElement*)dob)->ReleaseMouseCapture ();

		VOID_TO_NPVARIANT (*result);
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
		case Type::CONTROL:
			np_class = MoonlightControlClass;
			break;
		case Type::IMAGE:
			np_class = MoonlightImageClass;
			break;
		default:
			np_class = MoonlightDependencyObjectClass;
		}
	}

	MoonlightDependencyObjectObject *depobj
		= (MoonlightDependencyObjectObject*)NPN_CreateObject (instance,
								      np_class);

	depobj->SetDependencyObject (obj);

	/* do any post creation initialization here */
	switch (obj->GetObjectType()) {
	case Type::CONTROL:
		((MoonlightControlObject *)depobj)->real_object = DependencyObjectCreateWrapper (instance,
												 ((Control*)obj)->real_object);
		break;
	}

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
		// XXX JS doesn't have 64bit ints?
		if (argCount != 1 || !NPVARIANT_IS_INT32 (args[0]))
			return true;

		TimeSpan ts = (TimeSpan)NPVARIANT_TO_INT32(args[0]);
		sb->Seek (ts);

		VOID_TO_NPVARIANT (*result);

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
	"pause",
	"setSource"
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
	else if (name_matches (name, "setSource")) {
		if (argCount != 2
		    || !NPVARIANT_IS_OBJECT (args[0])
		    || !NPVARIANT_IS_STRING (args[1]))
			return true;

		DependencyObject *downloader = ((MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT(args[0]))->dob;
		const char *part = NPVARIANT_TO_STRING (args[0]).utf8characters;
	  
		media->SetSource (downloader, part);

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

/*** MoonlightImageClass ***************************************************/

static const char *const
moonlight_image_methods [] = {
	"setSource"
};


static bool
moonlight_image_has_method (NPObject *npobj, NPIdentifier name)
{
	if (HAS_METHOD (moonlight_image_methods, name))
		return true;

	return MoonlightDependencyObjectClass->hasMethod (npobj, name);
}

static bool
moonlight_image_invoke (NPObject *npobj, NPIdentifier name,
			const NPVariant *args, uint32_t argCount,
			NPVariant *result)
{
	Image *img = (Image*)((MoonlightDependencyObjectObject*)npobj)->dob;

	if (name_matches (name, "setSource")) {
		if (argCount != 2
		    || !NPVARIANT_IS_OBJECT (args[0])
		    || !NPVARIANT_IS_STRING (args[1]))
			return true;

		DependencyObject *downloader = ((MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT(args[0]))->dob;
		const char *part = NPVARIANT_TO_STRING (args[1]).utf8characters;
	  
		img->SetSource (downloader, part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	else 
		return MoonlightDependencyObjectClass->invoke (npobj, name,
							       args, argCount,
							       result);
}

MoonlightImageType::MoonlightImageType ()
{
	hasMethod = moonlight_image_has_method;
	invoke = moonlight_image_invoke;
}

MoonlightImageType* MoonlightImageClass;

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

/*** MoonlightControlClass ***************************************************/

static NPObject*
moonlight_control_allocate (NPP instance, NPClass *)
{
	return new MoonlightControlObject (instance);
}

static void
moonlight_control_deallocate (NPObject *npobject)
{
	delete (MoonlightControlObject*)npobject;
}

static bool
moonlight_control_has_property (NPObject *npobj, NPIdentifier name)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;

	if (control->real_object->_class->hasProperty (control->real_object, name))
		return true;

	return MoonlightDependencyObjectClass->hasProperty (npobj, name);
}

static bool
moonlight_control_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)control)->dob;
	DependencyObject *real_object = ((MoonlightDependencyObjectObject*)control->real_object)->dob;
	DependencyProperty *p;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);

	p = _get_dependency_property (real_object, strname);
	if (!p)
		p = _get_dependency_property(dob, strname);

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
moonlight_control_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;
	DependencyObject *dob = ((MoonlightDependencyObjectObject*)control)->dob;
	DependencyObject *real_object = ((MoonlightDependencyObjectObject*)control->real_object)->dob;
	DependencyProperty *p;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	strname[0] = toupper(strname[0]);

	p = _get_dependency_property (real_object, strname);
	if (!p)
		p = _get_dependency_property(dob, strname);

	NPN_MemFree (strname);

	if (!p)
		return false;

	_set_dependency_property_value (dob, p, value);
}

static bool
moonlight_control_has_method (NPObject *npobj, NPIdentifier name)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;

	return control->real_object->_class->hasMethod (control->real_object, name);
}

static bool
moonlight_control_invoke (NPObject *npobj, NPIdentifier name,
			  const NPVariant *args, uint32_t argCount,
			  NPVariant *result)
{
	MoonlightControlObject *control = (MoonlightControlObject*)npobj;

	return NPN_Invoke (control->instance, control->real_object,
			   name, args, argCount, result);
}

MoonlightControlType::MoonlightControlType ()
{
	allocate = moonlight_control_allocate;
	deallocate = moonlight_control_deallocate;
	hasProperty = moonlight_control_has_property;
	getProperty = moonlight_control_get_property;
	setProperty = moonlight_control_set_property;
	hasMethod = moonlight_control_has_method;
	invoke = moonlight_control_invoke;
}

MoonlightControlType* MoonlightControlClass;


/*** MoonlightScriptableObjectClass ***************************************************/
struct ScriptableProperty {
	gpointer property_handle;
	int property_type;
	bool can_read;
	bool can_write;
};

struct ScriptableEvent {
	gpointer event_handle;
};

struct ScriptableMethod {
	gpointer method_handle;
	int method_return_type;
	int *method_parameter_types;
	int parameter_count;
};


static NPObject*
moonlight_scriptable_object_allocate (NPP instance, NPClass*)
{
	return new MoonlightScriptableObjectObject (instance);
}

static void
moonlight_scriptable_object_deallocate (NPObject *npobj)
{
	delete (MoonlightScriptableObjectObject*)npobj;
}

static void
moonlight_scriptable_object_invalidate (NPObject *npobj)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;

	if (sobj->managed_scriptable) {
		// XXX unref the scriptable object however we need to.
	}
	sobj->managed_scriptable = NULL;

	// XXX free the properties, events, and methods hashes.
}

static bool
moonlight_scriptable_object_has_property (NPObject *npobj, NPIdentifier name)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;

	return (g_hash_table_lookup (sobj->properties, name) != NULL
		|| g_hash_table_lookup (sobj->events, name));
}

static bool
moonlight_scriptable_object_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;
	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (sobj->properties, name);
	if (!prop)
		return false;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("***************** getting scriptable object property %s", strname);
	NPN_MemFree (strname);

	Value v;

	sobj->getprop (sobj->managed_scriptable, prop->property_handle, &v);

	value_to_variant (npobj, &v, result);

	v.FreeValue ();

	return true;
}

static bool 
moonlight_scriptable_object_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;

	// first we try the property hash
	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (sobj->properties, name);
	if (prop) {
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		DEBUGMSG ("***************** setting scriptable object property %s", strname);
		NPN_MemFree (strname);

		Value *v;

		variant_to_value (value, &v);
		
		sobj->setprop (sobj->managed_scriptable, prop->property_handle, v);

		delete v;

		return true;
	}
	// if that fails, look for the event of that name
	ScriptableEvent *event = (ScriptableEvent*)g_hash_table_lookup (sobj->events, name);
	if (event) {
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		DEBUGMSG ("***************** adding scriptable object event %s", strname);
		NPN_MemFree (strname);

		DEBUG_WARN_NOTIMPLEMENTED ("scriptableobject.register_event");
		return true;
	}
}

static bool
moonlight_scriptable_object_has_method (NPObject *npobj, NPIdentifier name)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;

	return g_hash_table_lookup (sobj->methods, name) != NULL;
}

static bool
moonlight_scriptable_object_invoke (NPObject *npobj, NPIdentifier name,
				    const NPVariant *args, uint32_t argCount,
				    NPVariant *result)
{
	MoonlightScriptableObjectObject *sobj = (MoonlightScriptableObjectObject*)npobj;
	ScriptableMethod *method = (ScriptableMethod*)g_hash_table_lookup (sobj->methods, name);
	if (!method)
		return false;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("***************** invoking scriptable object method %s", strname);
	NPN_MemFree (strname);

	Value rv;

	Value** vargs = NULL;

	if (argCount > 0) {
		vargs = new Value*[argCount];
		for (int i = 0; i < argCount; i ++) {
			variant_to_value (&args[i], &vargs[i]);
		}
	}

	sobj->invoke (sobj->managed_scriptable, method->method_handle, vargs, argCount, &rv);

	if (argCount > 0) {
		for (int i = 0; i < argCount; i ++) {
			delete vargs[i];
		}
		delete [] vargs;
	}

	if (method->method_return_type != 1 /* XXX this 1 is "void" */) {
		value_to_variant (sobj, &rv, result);
	}
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

MoonlightScriptableObjectObject*
moonlight_scriptable_object_wrapper_create (PluginInstance *plugin, gpointer scriptable,
					    InvokeDelegate invoke_func,
					    SetPropertyDelegate setprop_func,
					    GetPropertyDelegate getprop_func,
					    EventHandlerDelegate addevent_func,
					    EventHandlerDelegate removeevent_func)

{
	MoonlightScriptControlObject *root_object = plugin->getRootObject ();

	MoonlightScriptableObjectObject* obj = (MoonlightScriptableObjectObject*)NPN_CreateObject (((MoonlightObject*)root_object)->instance,
												   MoonlightScriptableObjectClass);

	obj->managed_scriptable = scriptable;
	obj->invoke = invoke_func;
	obj->setprop = setprop_func;
	obj->getprop = getprop_func;
	obj->addevent = addevent_func;
	obj->removeevent = removeevent_func;

	DEBUGMSG ("creating scriptable object wrapper => %p", obj);
	return obj;
}

void
moonlight_scriptable_object_add_property (PluginInstance *plugin,
					  MoonlightScriptableObjectObject *obj,
					  gpointer property_handle,
					  char *property_name,
					  int property_type,
					  bool can_read,
					  bool can_write)
{
	DEBUGMSG ("adding property named %s to scriptable object %p", property_name, obj);

	ScriptableProperty *prop = new ScriptableProperty ();
	prop->property_handle = property_handle;
	prop->property_type = property_type;
	prop->can_read = can_read;
	prop->can_write = can_write;

	g_hash_table_insert (obj->properties, NPID(property_name), prop);
}

void
moonlight_scriptable_object_add_event (PluginInstance *plugin,
				       MoonlightScriptableObjectObject *obj,
				       gpointer event_handle,
				       char *event_name)
{
	DEBUGMSG ("adding event named %s to scriptable object %p", event_name, obj);

	ScriptableEvent *event = new ScriptableEvent ();
	event->event_handle = event_handle;

	g_hash_table_insert (obj->events, NPID(event_name), event);
}

void
moonlight_scriptable_object_add_method (PluginInstance *plugin,
					MoonlightScriptableObjectObject *obj,
					gpointer method_handle,
					char *method_name,
					int method_return_type,
					int *method_parameter_types,
					int parameter_count)

{
	DEBUGMSG ("adding method named %s (return type = %d) to scriptable object %p", method_name, method_return_type, obj);

	ScriptableMethod *method = new ScriptableMethod ();
	method->method_handle = method_handle;
	method->method_return_type = method_return_type;
	method->method_parameter_types = new int[parameter_count];
	memcpy (method->method_parameter_types, method_parameter_types, sizeof (int) * parameter_count);
	method->parameter_count = parameter_count;

	g_hash_table_insert (obj->methods, NPID(method_name), method);
}

void
moonlight_scriptable_object_register (PluginInstance *plugin,
				      char *name,
				      MoonlightScriptableObjectObject *obj)
{
	DEBUGMSG ("registering scriptable object '%s' => %p", name, obj);

	MoonlightContentObject* content = (MoonlightContentObject*)plugin->getRootObject()->content;

	g_hash_table_insert (content->registered_scriptable_objects,
			     NPID (name),
			     obj);

	DEBUGMSG (" => done");
}

void
html_object_get_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *result)
{
	/* we lookup on the window object if no object was supplied */
	NPObject *win = NULL;
	if (npobj == NULL) {
		NPN_GetValue (plugin->getInstance (), NPNVWindowNPObject, &win);
		npobj = win;
	}

	NPVariant *npresult;
	NPIdentifier str_name = NPN_GetStringIdentifier (name);
	NPN_GetProperty (plugin->getInstance (), npobj, str_name, npresult);

	Value *res;
	variant_to_value (npresult, &res);

	*result = *res;
	if (win)
		NPN_ReleaseObject (win);
}

void
html_object_set_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *value)
{
	/* we lookup on the window object if no object was supplied */
	NPObject *win = NULL;
	if (npobj == NULL) {
		NPN_GetValue (plugin->getInstance (), NPNVWindowNPObject, &win);
		npobj = win;
	}

	NPVariant *npvalue;
	NPIdentifier str_name = NPN_GetStringIdentifier (name);

	value_to_variant (npobj, value, npvalue);
	NPN_SetProperty (plugin->getInstance (), npobj, str_name, npvalue);

	if (win)
		NPN_ReleaseObject (win);
}

void
plugin_init_classes ()
{
	MoonlightErrorEventArgsClass = new MoonlightErrorEventArgsType ();
	MoonlightPointClass = new MoonlightPointType ();
	MoonlightObjectClass = new MoonlightObjectType ();
	MoonlightScriptControlClass = new MoonlightScriptControlType ();
	MoonlightContentClass = new MoonlightContentType ();
	MoonlightSettingsClass = new MoonlightSettingsType ();
	MoonlightDependencyObjectClass = new MoonlightDependencyObjectType ();
	MoonlightCollectionClass = new MoonlightCollectionType ();
	MoonlightStoryboardClass = new MoonlightStoryboardType ();
	MoonlightMediaElementClass = new MoonlightMediaElementType ();
	MoonlightImageClass = new MoonlightImageType ();
	MoonlightDownloaderClass = new MoonlightDownloaderType ();
	MoonlightControlClass = new MoonlightControlType ();
	MoonlightMouseEventArgsClass = new MoonlightMouseEventArgsType ();
	MoonlightScriptableObjectClass = new MoonlightScriptableObjectType ();
}

