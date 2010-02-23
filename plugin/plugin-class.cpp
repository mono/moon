/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-class.cpp: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <ctype.h>

#include "plugin-class.h"
#include "plugin-accessibility.h"
#include "plugin.h"
#include "deployment.h"
#include "bitmapimage.h"
#include "uri.h"
#include "textbox.h"
#include "grid.h"
#include "multiscaleimage.h"
#include "tilesource.h"
#include "deepzoomimagetilesource.h"

#ifdef DEBUG
#define DEBUG_WARN_NOTIMPLEMENTED(x) printf ("not implemented: (%s) " G_STRLOC "\n", x)
#define d(x) x
#else
#define DEBUG_WARN_NOTIMPLEMENTED(x)
#define d(x)
#endif

// debug scriptable object
#define ds(x)

// warnings
#define w(x) x


static char*
npidentifier_to_downstr (NPIdentifier id)
{
	if (!MOON_NPN_IdentifierIsString (id))
		return NULL;

	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (id);
	char *p = strname;
	while (*p) {
		*p = g_ascii_tolower (*p);
		p++;
	}

	return strname;
}

/* for use with bsearch & qsort */
static int
compare_mapping (const void *m1, const void *m2)
{
	MoonNameIdMapping *map1 = (MoonNameIdMapping*) m1;
	MoonNameIdMapping *map2 = (MoonNameIdMapping*) m2;
	return strcmp(map1->name, map2->name);
}

static int
map_name_to_id (NPIdentifier name, const MoonNameIdMapping mapping[], int count)
{
	char *strname = npidentifier_to_downstr (name);
	if (!strname)
		return NoMapping;

	MoonNameIdMapping key, *result;

	key.name = strname;
	result = (MoonNameIdMapping*)bsearch(&key, mapping, count,
					     sizeof(MoonNameIdMapping), compare_mapping);


	MOON_NPN_MemFree (strname);
	if (!result)
		return NoMapping;
	
	return result->id;
}

static const char *
map_moon_id_to_event_name (int moon_id)
{
	const char *name = NULL;

	switch (moon_id) {
	case MoonId_BufferingProgressChanged: name = "BufferingProgressChanged"; break;
	case MoonId_CurrentStateChanged:  name = "CurrentStateChanged"; break;
	case MoonId_DownloadProgressChanged: name = "DownloadProgressChanged"; break;
	case MoonId_GotFocus: name = "GotFocus"; break;
	case MoonId_KeyDown: name = "KeyDown"; break;
	case MoonId_KeyUp: name = "KeyUp"; break;
	case MoonId_LostFocus: name = "LostFocus"; break;
	case MoonId_Loaded: name = "Loaded"; break;
	case MoonId_MarkerReached: name = "MarkerReached"; break;
	case MoonId_MediaEnded: name = "MediaEnded"; break;
	case MoonId_MediaFailed: name = "MediaFailed"; break;
	case MoonId_MediaOpened: name = "MediaOpened"; break;
	case MoonId_MouseEnter: name = "MouseEnter"; break;
	case MoonId_MouseLeave: name = "MouseLeave"; break;
	case MoonId_MouseMove: name = "MouseMove"; break;
	case MoonId_MouseLeftButtonDown: name = "MouseLeftButtonDown"; break;
	case MoonId_MouseLeftButtonUp: name = "MouseLeftButtonUp"; break;
	case MoonId_OnResize: name = "Resize"; break;
	case MoonId_OnFullScreenChange: name = "FullScreenChange"; break;
	case MoonId_OnError: name = "Error"; break;
	case MoonId_OnLoad: name = "Load"; break;
	case MoonId_OnSourceDownloadProgressChanged: name = "SourceDownloadProgressChanged"; break;
	case MoonId_OnSourceDownloadComplete: name = "SourceDownloadComplete"; break;
	}

	return name;
}


void
string_to_npvariant (const char *value, NPVariant *result)
{
	char *retval;

	if (value)
		retval = NPN_strdup ((char *)value);
	else
		retval = NPN_strdup ("");

	STRINGZ_TO_NPVARIANT (retval, *result);
}

static void
value_to_variant (NPObject *npobj, Value *v, NPVariant *result, DependencyObject *parent_obj = NULL, DependencyProperty *parent_property = NULL)
{
	char utf8[8];
	int n;
	
	if (!v) {
		NULL_TO_NPVARIANT (*result);
		return;
	}
	
	switch (v->GetKind ()) {
	case Type::BOOL:
		BOOLEAN_TO_NPVARIANT (v->AsBool(), *result);
		break;
	case Type::CHAR:
		n = g_unichar_to_utf8 (v->AsChar (), utf8);
		utf8[n] = '\0';
		string_to_npvariant (utf8, result);
		break;
	case Type::INT32:
		INT32_TO_NPVARIANT (v->AsInt32(), *result);
		break;
	case Type::UINT32:
		// XXX not much else we can do here...
		INT32_TO_NPVARIANT ((gint32)v->AsUInt32(), *result);
		break;
	case Type::DOUBLE:
		DOUBLE_TO_NPVARIANT (v->AsDouble(), *result);
		break;
	case Type::STRING:
		string_to_npvariant (v->AsString(), result);
		break;
	case Type::POINT: {
		MoonlightPoint *point = (MoonlightPoint *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightPointClass);
		point->point = *v->AsPoint ();
		OBJECT_TO_NPVARIANT (point, *result);
		break;
	}
	case Type::RECT: {
		MoonlightRect *rect = (MoonlightRect *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightRectClass);
		rect->rect = *v->AsRect ();
		OBJECT_TO_NPVARIANT (rect, *result);
		break;
	}
	case Type::DURATION: {
		MoonlightDuration *duration = (MoonlightDuration *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightDurationClass);
		duration->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (duration, *result);
		break;
	}
	case Type::TIMESPAN: {
		MoonlightTimeSpan *timespan = (MoonlightTimeSpan *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightTimeSpanClass);
		timespan->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (timespan, *result);
		break;
	}
	case Type::GRIDLENGTH: {
		MoonlightGridLength *gridlength = (MoonlightGridLength *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightGridLengthClass);
		gridlength->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (gridlength, *result);
		break;
	}
	case Type::URI: {
		char *uri_string = v->AsUri() ? v->AsUri()->ToString() : NULL;
		string_to_npvariant (uri_string ? uri_string : "", result);
		g_free (uri_string);
		break;
	}
	case Type::FONTFAMILY: {
		char *family = v->AsFontFamily() ? v->AsFontFamily()->source : NULL;
		string_to_npvariant (family ? family : "", result);
		break;
	}
	case Type::FONTWEIGHT: {
		const char *weight = enums_int_to_str ("FontWeight", v->AsFontWeight() ? v->AsFontWeight()->weight : FontWeightsNormal);
		string_to_npvariant (weight, result);
		break;
	}
	case Type::FONTSTYLE: {
		const char *style = enums_int_to_str ("FontStyle", v->AsFontStyle() ? v->AsFontStyle()->style : FontStylesNormal);
		string_to_npvariant (style, result);
		break;
	}
	case Type::FONTSTRETCH: {
		const char *stretch = enums_int_to_str ("FontStretch", v->AsFontStretch() ? v->AsFontStretch()->stretch : FontStretchesNormal);
		string_to_npvariant (stretch, result);
		break;
	}
	case Type::COLOR: {
		Color *c = v->AsColor ();
		gint32 color = ((((gint32)(c->a * 255.0)) << 24) | (((gint32)(c->r * 255.0)) << 16) | 
			(((gint32)(c->g * 255.0)) << 8) | ((gint32)(c->b * 255.0)));
		INT32_TO_NPVARIANT (color, *result);
		break;
	}
	case Type::KEYTIME: {
		MoonlightKeyTime *keytime = (MoonlightKeyTime *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightKeyTimeClass);
		keytime->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (keytime, *result);
		break;
	}
	case Type::THICKNESS: {
		MoonlightThickness *thickness = (MoonlightThickness *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightThicknessClass);
		thickness->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (thickness, *result);
		break;
	}
	case Type::CORNERRADIUS: {
		MoonlightCornerRadius *corner_radius = (MoonlightCornerRadius *) MOON_NPN_CreateObject (((MoonlightObject *) npobj)->GetInstance (), MoonlightCornerRadiusClass);
		corner_radius->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (corner_radius, *result);
		break;
	}
	case Type::NPOBJ: {
		NPObject *npobj = (NPObject *) v->AsNPObj ();
		OBJECT_TO_NPVARIANT (npobj, *result);
		MOON_NPN_RetainObject (npobj);
		break;
	}
	default:
		/* more builtins.. */
		if (v->Is (Deployment::GetCurrent (), Type::DEPENDENCY_OBJECT)) {
			MoonlightEventObjectObject *depobj =
				EventObjectCreateWrapper (((MoonlightObject *) npobj)->GetPlugin (), v->AsDependencyObject ());
			OBJECT_TO_NPVARIANT (depobj, *result);
		} else {
#if DEBUG
			printf ("value_to_variant, can't create a variant of a %i = %s\n", v->GetKind (), Type::Find (Deployment::GetCurrent (), v->GetKind ())->GetName ());
#endif
			NULL_TO_NPVARIANT (*result);
		}
		break;
	}
}

void
variant_to_value (const NPVariant *v, Value **result)
{
	switch (v->type) {
	case NPVariantType_Bool:
		*result = new Value (NPVARIANT_TO_BOOLEAN (*v));
		break;
	case NPVariantType_Int32:
		*result = new Value ((int32_t) NPVARIANT_TO_INT32 (*v));
		break;
	case NPVariantType_Double:
		*result = new Value (NPVARIANT_TO_DOUBLE (*v));
		break;
	case NPVariantType_String: {
		char *value = STRDUP_FROM_VARIANT (*v);
		*result = new Value (value);
		g_free (value);
		break;
	}
	case NPVariantType_Void:
		DEBUG_WARN_NOTIMPLEMENTED ("void variant type");
		*result = NULL;
		break;
	case NPVariantType_Null:
		DEBUG_WARN_NOTIMPLEMENTED ("null variant type");
		*result = new Value (Type::DEPENDENCY_OBJECT);
		break;
	case NPVariantType_Object:
		*result = new Value (Type::NPOBJ, NPVARIANT_TO_OBJECT (*v));
		break;
	default:
		d (printf ("Got invalid value from javascript.\n"));
		*result = NULL;
		break;
	}
}

enum DependencyObjectClassNames {
	COLLECTION_CLASS,
	CONTROL_CLASS,
	DEPENDENCY_OBJECT_CLASS,
	UI_ELEMENT_CLASS,
	DOWNLOADER_CLASS,
	IMAGE_BRUSH_CLASS,
	IMAGE_CLASS,
	MEDIA_ELEMENT_CLASS,
	STORYBOARD_CLASS,
	STYLUS_INFO_CLASS,
	STYLUS_POINT_COLLECTION_CLASS,
	STROKE_COLLECTION_CLASS,
	STROKE_CLASS,
	TEXT_BOX_CLASS,
	PASSWORD_BOX_CLASS,
	TEXT_BLOCK_CLASS,
	EVENT_ARGS_CLASS,
	ROUTED_EVENT_ARGS_CLASS,
	ERROR_EVENT_ARGS_CLASS,
	KEY_EVENT_ARGS_CLASS,
	TIMELINE_MARKER_ROUTED_EVENT_ARGS_CLASS,
	MOUSE_EVENT_ARGS_CLASS,
	DOWNLOAD_PROGRESS_EVENT_ARGS_CLASS,
	MULTI_SCALE_IMAGE_CLASS,

	DEPENDENCY_OBJECT_CLASS_NAMES_LAST
};

NPClass *dependency_object_classes[DEPENDENCY_OBJECT_CLASS_NAMES_LAST];

bool
npobject_is_dependency_object (NPObject *obj)
{
	for (int i = 0; i < DEPENDENCY_OBJECT_CLASS_NAMES_LAST; i++) {
		if (dependency_object_classes [i] == obj->_class)
			return true;
	}
	return false;
}

static bool
npvariant_is_dependency_object (NPVariant var)
{
	if (!NPVARIANT_IS_OBJECT (var))
		return false;
	
	return npobject_is_dependency_object (NPVARIANT_TO_OBJECT (var));
}

static bool
npvariant_is_object_class (NPVariant var, int type)
{
	NPObject *obj;
	
	if (type < 0 || type >= DEPENDENCY_OBJECT_CLASS_NAMES_LAST)
		return false;
	
	if (!NPVARIANT_IS_OBJECT (var))
		return false;
	
	obj = NPVARIANT_TO_OBJECT (var);
	
	return obj->_class == dependency_object_classes[type];
}

#define npvariant_is_downloader(v) npvariant_is_object_class (v, DOWNLOADER_CLASS)

static bool
npvariant_is_moonlight_object (NPVariant var)
{
	NPClass *moonlight_types[] = {
		MoonlightContentClass,
		MoonlightDurationClass,
		MoonlightObjectClass,
		MoonlightPointClass,
		MoonlightScriptableObjectClass,
		MoonlightScriptControlClass,
		MoonlightSettingsClass,
		MoonlightTimeSpanClass
	};
	NPObject *obj;
	guint i;
	
	if (!NPVARIANT_IS_OBJECT (var))
		return false;
	
	obj = NPVARIANT_TO_OBJECT (var);
	if (npobject_is_dependency_object (obj))
		return true;
	
	for (i = 0; i < G_N_ELEMENTS (moonlight_types); i++) {
		if (obj->_class == moonlight_types[i])
			return true;
	}
	
	return false;
}

EventListenerProxy::EventListenerProxy (PluginInstance *plugin, const char *event_name, const char *cb_name)
	: EventObject (Type::EVENTLISTENERPROXY)
{
	this->plugin = plugin;
	this->event_name = g_strdup (event_name);
	this->event_id = -1;
	this->target_object = NULL;
	this->owner = NULL;
	this->one_shot = false;
	this->is_func = false;
	if (!strncmp (cb_name, "javascript:", strlen ("javascript:")))
		cb_name += strlen ("javascript:");
	this->callback = g_strdup (cb_name);
}

EventListenerProxy::EventListenerProxy (PluginInstance *plugin, const char *event_name, const NPVariant *cb)
	: EventObject (Type::EVENTLISTENERPROXY)
{
	this->plugin = plugin;
	this->event_name = g_strdup (event_name);
	this->event_id = -1;
	this->target_object = NULL;
	this->owner = NULL;
	this->one_shot = false;

	if (NPVARIANT_IS_OBJECT (*cb)) {
		this->is_func = true;
		this->callback = NPVARIANT_TO_OBJECT (*cb);
		MOON_NPN_RetainObject ((NPObject *) this->callback);
	} else {
		this->is_func = false;
		this->callback = STRDUP_FROM_VARIANT (*cb);
	}
}

EventListenerProxy::~EventListenerProxy ()
{
	if (is_func) {
// XXX we *want* to be able to do this, we really do, but we have no
// good means to invalidate EventListenerProxy, and thereby set the
// callback to NULL.
//
// instead we do it in ::RemoveHandler, which is only invoked via JS's removeEventListener
//
// 		if (callback != NULL)
// 			MOON_NPN_ReleaseObject ((NPObject *) callback);
	}
	else {
		g_free (callback);
	}
	
	g_free (event_name);
}
	
const char *
EventListenerProxy::GetCallbackAsString ()
{
	if (is_func)
		return "";
	
	return (const char *)callback;
}

void
EventListenerProxy::SetOwner (MoonlightObject *owner)
{
	this->owner = owner;
}

int
EventListenerProxy::AddHandler (EventObject *obj)
{
	target_object = obj;
	
	event_id = obj->GetType()->LookupEvent (event_name);

	if (event_id == -1) {
		d(printf ("object of type `%s' does not provide an event named `%s'\n",
			  obj->GetTypeName(), event_name));
		return -1;
	}

	token = obj->AddHandler (event_id, proxy_listener_to_javascript, this, on_handler_removed);
	return token;
}

int
EventListenerProxy::AddXamlHandler (EventObject *obj)
{
	target_object = obj;
	
	event_id = obj->GetType()->LookupEvent (event_name);
	
	if (event_id == -1) {
		d(printf ("object of type `%s' does not provide an event named `%s'\n",
			  obj->GetTypeName(), event_name));
		return -1;
	}
	
	token = obj->AddXamlHandler (event_id, proxy_listener_to_javascript, this, on_handler_removed);
	
	return token;
}

void
EventListenerProxy::RemoveHandler ()
{
	if (target_object && event_id != -1) {
		target_object->RemoveHandler (event_id, token);
		if (is_func && callback) {
			MOON_NPN_ReleaseObject ((NPObject *) callback);
			callback = NULL;
		}
	}
	else {
		on_handler_removed (this);
	}
}

void
EventListenerProxy::on_handler_removed (gpointer closure)
{
	// by the time we get here, the target_object has disclaimed
	// all knowledge of this proxy.

	EventListenerProxy *proxy = (EventListenerProxy *) closure;

	if (proxy->owner) {
		proxy->owner->ClearEventProxy (proxy);
	}
	else {
		// we don't have an owner, so there's nothing special
		// for us to do here.
	}

	proxy->target_object = NULL;
	proxy->event_id = -1;
	proxy->unref_delayed();
}

void
EventListenerProxy::proxy_listener_to_javascript (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy *) closure;
	EventObject *js_sender = sender;
	NPVariant args[2];
	NPVariant result;
	int argcount = 1;
	PluginInstance *plugin = proxy->GetPlugin ();
	Deployment *previous_deployment;
	
	if (plugin == NULL || plugin->HasShutdown ()) {
		// Firefox can invalidate our NPObjects after the plugin itself
		// has been destroyed. During this invalidation our NPObjects call 
		// into the moonlight runtime, which then emits events.
		d(printf ("Moonlight: The plugin has been deleted, but we're still emitting events?\n"));
		return;
	}

	// do not let cross-domain application re-register the events (e.g. via scripting)
	if (plugin->IsCrossDomainApplication ()) {
//		g_warning ("xdomain restriction on javascript event: %s", proxy->GetCallbackAsString ());
		return;
	}

	previous_deployment = Deployment::GetCurrent ();
	Deployment::SetCurrent (plugin->GetDeployment ());

	if (js_sender->GetObjectType () == Type::SURFACE) {
		// This is somewhat hackish, but is required for
		// the FullScreenChanged event (js expects the
		// sender to be the toplevel canvas, not the surface,
		// nor the content).
		js_sender = ((Surface*) js_sender)->GetToplevel ();
	}

	MoonlightEventObjectObject *depobj = NULL; 
	if (js_sender) {
		depobj = EventObjectCreateWrapper (plugin, js_sender);
		plugin->AddCleanupPointer (&depobj);
		OBJECT_TO_NPVARIANT (depobj, args[0]);
	} else {
		NULL_TO_NPVARIANT (args[0]);
	}

	//printf ("proxying event %s to javascript, sender = %p (%s)\n", proxy->event_name, sender, sender->GetTypeName ());
	MoonlightEventObjectObject *depargs = NULL; 
	if (calldata) {
		depargs = EventObjectCreateWrapper (plugin, calldata);
		plugin->AddCleanupPointer (&depargs);
		OBJECT_TO_NPVARIANT (depargs, args[1]);
		argcount++;
	}
	
	if (proxy->is_func && proxy->callback) {
		/* the event listener was added with a JS function object */
		if (MOON_NPN_InvokeDefault (proxy->GetInstance (), (NPObject *) proxy->callback, args, argcount, &result))
			MOON_NPN_ReleaseVariantValue (&result);
	} else {
		/* the event listener was added with a JS string (the function name) */
		NPObject *object = NULL;
		
		if (MOON_NPN_GetValue (proxy->GetInstance (), NPNVWindowNPObject, &object) == NPERR_NO_ERROR) {
			if (MOON_NPN_Invoke (proxy->GetInstance (), object, NPID ((char *) proxy->callback), args, argcount, &result))
				MOON_NPN_ReleaseVariantValue (&result);
		}
	}

	if (depobj) {
		plugin->RemoveCleanupPointer (&depobj);
		MOON_NPN_ReleaseObject (depobj);
	}
	if (depargs) {
		plugin->RemoveCleanupPointer (&depargs);
		MOON_NPN_ReleaseObject (depargs);
	}
	if (proxy->one_shot)
		proxy->RemoveHandler();
	
	Deployment::SetCurrent (previous_deployment);
}

void
event_object_add_xaml_listener (EventObject *obj, PluginInstance *plugin, const char *event_name, const char *cb_name)
{
	EventListenerProxy *proxy = new EventListenerProxy (plugin, event_name, cb_name);
	proxy->AddXamlHandler (obj);
}

class NamedProxyPredicate {
public:
	NamedProxyPredicate (char *name) { this->name = g_strdup (name); }
	~NamedProxyPredicate () { g_free (name); }

	static bool matches (EventHandler cb_handler, gpointer cb_data, gpointer data)
	{
		if (cb_handler != EventListenerProxy::proxy_listener_to_javascript)
			return false;
		if (cb_data == NULL)
			return false;
		EventListenerProxy *proxy = (EventListenerProxy*)cb_data;
		NamedProxyPredicate *pred = (NamedProxyPredicate*)data;

		return !strcasecmp (proxy->GetCallbackAsString(), pred->name);
	}
private:
	char *name;
};

/*** EventArgs **/

static NPObject *
event_args_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightEventArgs (instance);
}

MoonlightEventArgsType::MoonlightEventArgsType ()
{
	allocate = event_args_allocate;
}

MoonlightEventArgsType *MoonlightEventArgsClass;

/*** RoutedEventArgs ***/
static NPObject *
routedeventargs_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightRoutedEventArgs (instance);
}

static const MoonNameIdMapping
routedeventargs_mapping[] = {
	{ "source", MoonId_Source },
};

bool
MoonlightRoutedEventArgs::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	RoutedEventArgs *args = GetRoutedEventArgs ();

	switch (id) {
	case MoonId_Source: {
		DependencyObject *source = args->GetSource ();
		if (source) {
			MoonlightEventObjectObject *source_obj = EventObjectCreateWrapper (GetPlugin (), source);
			OBJECT_TO_NPVARIANT (source_obj, *result);
		}
		else {
			NULL_TO_NPVARIANT (*result);
		}

		return true;
	}

	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}
}

MoonlightRoutedEventArgsType::MoonlightRoutedEventArgsType ()
{
	allocate = routedeventargs_allocate;

	AddMapping (routedeventargs_mapping, G_N_ELEMENTS (routedeventargs_mapping));
}

MoonlightRoutedEventArgsType *MoonlightRoutedEventArgsClass;


/*** ErrorEventArgs ***/
static NPObject *
erroreventargs_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightErrorEventArgs (instance);
}

static const MoonNameIdMapping
erroreventargs_mapping[] = {
	{ "charposition", MoonId_CharPosition },
	{ "errorcode", MoonId_ErrorCode },
	{ "errormessage", MoonId_ErrorMessage },
	{ "errortype", MoonId_ErrorType },
	{ "linenumber", MoonId_LineNumber },
	{ "methodname", MoonId_MethodName },
	{ "xamlfile", MoonId_XamlFile },
};

bool
MoonlightErrorEventArgs::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	ErrorEventArgs *args = GetErrorEventArgs ();

	switch (id) {
	case MoonId_ErrorCode:
		INT32_TO_NPVARIANT (args->GetErrorCode(), *result);
		return true;

	case MoonId_ErrorType:
		switch (args->GetErrorType()) {
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
	case MoonId_ErrorMessage:
		string_to_npvariant (args->GetErrorMessage(), result);
		return true;
	case MoonId_LineNumber:
		if (args->GetErrorType() == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)args)->line_number, *result);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.lineNumber");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;
	case MoonId_CharPosition:
		if (args->GetErrorType() == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)args)->char_position, *result);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.charPosition");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;
	case MoonId_MethodName:
		DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.methodName");
		NULL_TO_NPVARIANT (*result);
		return true;
	case MoonId_XamlFile:
		if (args->GetErrorType() == ParserError) {
			string_to_npvariant (((ParserErrorEventArgs*)args)->xaml_file, result);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.xamlFile");
			NULL_TO_NPVARIANT (*result);
		}
		return true;
	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}
}

MoonlightErrorEventArgsType::MoonlightErrorEventArgsType ()
{
	allocate = erroreventargs_allocate;

	AddMapping (erroreventargs_mapping, G_N_ELEMENTS (erroreventargs_mapping));
}

MoonlightErrorEventArgsType *MoonlightErrorEventArgsClass;

/*** Points ***/
static NPObject *
point_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightPoint (instance);
}

static const MoonNameIdMapping
point_mapping[] = {
	{ "x", MoonId_X },
	{ "y", MoonId_Y }
};


bool
MoonlightPoint::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_X:
		DOUBLE_TO_NPVARIANT (point.x, *result);
		return true;

	case MoonId_Y:
		DOUBLE_TO_NPVARIANT (point.y, *result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightPoint::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_X:
		point.x = NPVARIANT_TO_DOUBLE (*value);
		return true;
	case MoonId_Y:
		point.y = NPVARIANT_TO_DOUBLE (*value);
		return true;
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightPointType::MoonlightPointType ()
{
	allocate = point_allocate;

	AddMapping (point_mapping, G_N_ELEMENTS (point_mapping));
}

MoonlightPointType *MoonlightPointClass;

/*** Rects ***/
static NPObject *
rect_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightRect (instance);
}

static const MoonNameIdMapping
rect_mapping[] = {
	{ "height", MoonId_Height },
	{ "width", MoonId_Width },
	{ "x", MoonId_X },
	{ "y", MoonId_Y },
};

bool
MoonlightRect::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_X:
		DOUBLE_TO_NPVARIANT (rect.x, *result);
		return true;

	case MoonId_Y:
		DOUBLE_TO_NPVARIANT (rect.y, *result);
		return true;

	case MoonId_Width:
		DOUBLE_TO_NPVARIANT (rect.width, *result);
		return true;

	case MoonId_Height:
		DOUBLE_TO_NPVARIANT (rect.height, *result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightRect::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_X:
		rect.x = NPVARIANT_TO_DOUBLE (*value);
		return true;

	case MoonId_Y:
		rect.y = NPVARIANT_TO_DOUBLE (*value);
		return true;

	case MoonId_Width:
		rect.width = NPVARIANT_TO_DOUBLE (*value);
		return true;

	case MoonId_Height:
		rect.height = NPVARIANT_TO_DOUBLE (*value);
		return true;

	default:
		return MoonlightObject::SetProperty (id, name, value);;
	}
}


MoonlightRectType::MoonlightRectType ()
{
	allocate = rect_allocate;

	AddMapping (rect_mapping, G_N_ELEMENTS (rect_mapping));
}

MoonlightRectType *MoonlightRectClass;


/*** Durations ***/
static NPObject *
duration_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightDuration (instance);
}

static const MoonNameIdMapping
duration_mapping[] = {
	{ "name", MoonId_Name },
	{ "seconds", MoonId_Seconds }
};

void
MoonlightDuration::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

double
MoonlightDuration::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return v ? v->AsDuration()->ToSecondsFloat () : 0.0;
}

bool
MoonlightDuration::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;

	case MoonId_Seconds:
		DOUBLE_TO_NPVARIANT (GetValue(), *result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightDuration::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Seconds:
		parent_obj->SetValue (parent_property, Value(Duration::FromSecondsFloat (NPVARIANT_TO_DOUBLE (*value))));
		return true;

	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightDuration::~MoonlightDuration ()
{
	if (parent_obj)
		parent_obj->unref();
}

MoonlightDurationType::MoonlightDurationType ()
{
	allocate = duration_allocate;

	AddMapping (duration_mapping, G_N_ELEMENTS (duration_mapping));
}

MoonlightDurationType *MoonlightDurationClass;


/*** TimeSpans ***/
static NPObject *
timespan_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightTimeSpan (instance);
}

static const MoonNameIdMapping
timespan_mapping[] = {
	{ "name", MoonId_Name },
	{ "seconds", MoonId_Seconds }
};

void
MoonlightTimeSpan::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

TimeSpan
MoonlightTimeSpan::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return v ? v->AsTimeSpan() : (TimeSpan)0;
}

bool
MoonlightTimeSpan::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_Seconds:
		DOUBLE_TO_NPVARIANT (TimeSpan_ToSecondsFloat (GetValue ()), *result);
		return true;
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightTimeSpan::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Seconds:
		if (NPVARIANT_IS_INT32 (*value)) {
			parent_obj->SetValue (parent_property, Value(TimeSpan_FromSecondsFloat (NPVARIANT_TO_INT32 (*value)), Type::TIMESPAN));
		} else if (NPVARIANT_IS_DOUBLE (*value)) {
			parent_obj->SetValue (parent_property, Value(TimeSpan_FromSecondsFloat (NPVARIANT_TO_DOUBLE (*value)), Type::TIMESPAN));
		} else {
			return false;
		}
		return true;

	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightTimeSpan::~MoonlightTimeSpan ()
{
	if (parent_obj)
		parent_obj->unref ();
}

MoonlightTimeSpanType::MoonlightTimeSpanType ()
{
	allocate = timespan_allocate;

	AddMapping (timespan_mapping, G_N_ELEMENTS (timespan_mapping));
}

MoonlightTimeSpanType *MoonlightTimeSpanClass;

/*** KeyTime ***/
static NPObject *
keytime_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightKeyTime (instance);
}

static const MoonNameIdMapping
keytime_mapping[] = {
	{ "name", MoonId_Name },
	{ "seconds", MoonId_Seconds }
};

void
MoonlightKeyTime::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

KeyTime*
MoonlightKeyTime::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return (v ? v->AsKeyTime() : NULL);
}

bool
MoonlightKeyTime::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_Seconds:
		DOUBLE_TO_NPVARIANT (TimeSpan_ToSecondsFloat (GetValue ()->GetTimeSpan ()), *result);
		return true;
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightKeyTime::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Seconds:
		if (NPVARIANT_IS_INT32 (*value))
			parent_obj->SetValue (parent_property, Value(KeyTime::FromTimeSpan (TimeSpan_FromSecondsFloat (NPVARIANT_TO_INT32 (*value)))));
		else if (NPVARIANT_IS_DOUBLE (*value)) 
			parent_obj->SetValue (parent_property, Value(KeyTime::FromTimeSpan (TimeSpan_FromSecondsFloat (NPVARIANT_TO_DOUBLE (*value)))));

		return true;
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightKeyTime::~MoonlightKeyTime ()
{
	if (parent_obj)
		parent_obj->unref ();
}

MoonlightKeyTimeType::MoonlightKeyTimeType ()
{
	allocate = keytime_allocate;

	AddMapping (keytime_mapping, G_N_ELEMENTS (keytime_mapping));
}

MoonlightKeyTimeType *MoonlightKeyTimeClass;


/*** Thickness ***/
static NPObject *
thickness_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightThickness (instance);
}

static const MoonNameIdMapping
thickness_mapping[] = {
	{ "bottom", MoonId_Bottom },
	{ "left", MoonId_Left },
	{ "right", MoonId_Right },
	{ "top", MoonId_Top }
};

void
MoonlightThickness::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

Thickness*
MoonlightThickness::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return (v ? v->AsThickness() : NULL);
}

bool
MoonlightThickness::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_Left:
		DOUBLE_TO_NPVARIANT (GetValue ()->left, *result);
		return true;
	case MoonId_Top:
		DOUBLE_TO_NPVARIANT (GetValue ()->top, *result);
		return true;
	case MoonId_Right:
		DOUBLE_TO_NPVARIANT (GetValue ()->right, *result);
		return true;
	case MoonId_Bottom:
		DOUBLE_TO_NPVARIANT (GetValue ()->bottom, *result);
		return true;
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightThickness::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Left:
	case MoonId_Top:
	case MoonId_Right:
	case MoonId_Bottom:
		return true;

	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightThickness::~MoonlightThickness ()
{
	if (parent_obj)
		parent_obj->unref ();
}

MoonlightThicknessType::MoonlightThicknessType ()
{
	allocate = thickness_allocate;

	AddMapping (thickness_mapping, G_N_ELEMENTS (thickness_mapping));
}

MoonlightThicknessType *MoonlightThicknessClass;

/*** CornerRadius ***/
static NPObject *
CornerRadius_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightCornerRadius (instance);
}

static const MoonNameIdMapping
CornerRadius_mapping[] = {
	{ "bottomright", MoonId_BottomRight },
	{ "bottomleft", MoonId_BottomLeft },
	{ "topright", MoonId_TopRight },
	{ "topleft", MoonId_TopLeft }
};

void
MoonlightCornerRadius::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

CornerRadius*
MoonlightCornerRadius::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return (v ? v->AsCornerRadius() : NULL);
}

bool
MoonlightCornerRadius::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_TopLeft:
		DOUBLE_TO_NPVARIANT (GetValue ()->topLeft, *result);
		return true;
	case MoonId_TopRight:
		DOUBLE_TO_NPVARIANT (GetValue ()->topRight, *result);
		return true;
	case MoonId_BottomLeft:
		DOUBLE_TO_NPVARIANT (GetValue ()->bottomLeft, *result);
		return true;
	case MoonId_BottomRight:
		DOUBLE_TO_NPVARIANT (GetValue ()->bottomRight, *result);
		return true;
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightCornerRadius::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_TopLeft:
	case MoonId_TopRight:
	case MoonId_BottomLeft:
	case MoonId_BottomRight:
		return true;

	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightCornerRadius::~MoonlightCornerRadius ()
{
	if (parent_obj)
		parent_obj->unref ();
}

MoonlightCornerRadiusType::MoonlightCornerRadiusType ()
{
	allocate = CornerRadius_allocate;

	AddMapping (CornerRadius_mapping, G_N_ELEMENTS (CornerRadius_mapping));
}

MoonlightCornerRadiusType *MoonlightCornerRadiusClass;

/*** GridLength ***/
static NPObject *
gridlength_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightGridLength (instance);
}

static const MoonNameIdMapping
gridlength_mapping[] = {
	{ "gridunittype", MoonId_GridUnitType },
	{ "value", MoonId_Value },
};

void
MoonlightGridLength::SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property)
{
	this->parent_obj = parent_obj;
	this->parent_property = parent_property;
	parent_obj->ref();
}

GridLength*
MoonlightGridLength::GetValue()
{
	Value *v = parent_obj->GetValue (parent_property);
	return (v ? v->AsGridLength() : NULL);
}

bool
MoonlightGridLength::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_Value:
		DOUBLE_TO_NPVARIANT (GetValue ()->val, *result);
		return true;
	case MoonId_GridUnitType:
		string_to_npvariant (enums_int_to_str ("GridUnitType", GetValue()->type), result);
		return true;
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightGridLength::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	GridLength *current_gridlength = GetValue();
	GridLength gridlength;

	if (current_gridlength)
		gridlength = *current_gridlength;

	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Value:
		gridlength.val = NPVARIANT_TO_DOUBLE (*value);
		parent_obj->SetValue (parent_property, Value(gridlength.val));
		return true;
	case MoonId_GridUnitType: {
		int unit_type = enums_str_to_int ("GridUnitType", NPVARIANT_TO_STRING (*value).utf8characters);
		if (unit_type == -1)
			return false; // FIXME: throw an exception?

		gridlength.type = (GridUnitType)unit_type;

		parent_obj->SetValue (parent_property, Value(gridlength));
		return true;
	}
		
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

MoonlightGridLength::~MoonlightGridLength ()
{
	if (parent_obj)
		parent_obj->unref ();
}

MoonlightGridLengthType::MoonlightGridLengthType ()
{
	allocate = gridlength_allocate;

	AddMapping (gridlength_mapping, G_N_ELEMENTS (gridlength_mapping));
}

MoonlightGridLengthType *MoonlightGridLengthClass;

/*** MoonlightDownloadProgressEventArgsClass  **************************************************************/

static NPObject *
download_progress_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightDownloadProgressEventArgs (instance);
}

static const MoonNameIdMapping
download_progress_event_mapping[] = {
	{ "progress", MoonId_Progress },
};

bool
MoonlightDownloadProgressEventArgs::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	DownloadProgressEventArgs *event_args = GetDownloadProgressEventArgs ();

	switch (id) {
	case MoonId_Progress:
		DOUBLE_TO_NPVARIANT (event_args->GetProgress (), *result);
		return true;
	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}

	return false;
}

MoonlightDownloadProgressEventArgsType::MoonlightDownloadProgressEventArgsType ()
{
	allocate = download_progress_event_allocate;

	AddMapping (download_progress_event_mapping, G_N_ELEMENTS (download_progress_event_mapping));
}

MoonlightDownloadProgressEventArgsType *MoonlightDownloadProgressEventArgsClass;


/*** MoonlightMouseEventArgsClass  **************************************************************/

static NPObject *
mouse_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMouseEventArgsObject (instance);
}

static const MoonNameIdMapping
mouse_event_mapping[] = {
	{ "ctrl", MoonId_Ctrl },
	{ "handled", MoonId_Handled },
	{ "getposition", MoonId_GetPosition },
	{ "getstylusinfo", MoonId_GetStylusInfo },
	{ "getstyluspoints", MoonId_GetStylusPoints },
	{ "shift", MoonId_Shift },
};

bool
MoonlightMouseEventArgsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	MouseEventArgs *event_args = GetMouseEventArgs ();
	int state = event_args->GetState ();

	switch (id) {
	case MoonId_Shift:
		BOOLEAN_TO_NPVARIANT ((state & GDK_SHIFT_MASK) != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT ((state & GDK_CONTROL_MASK) != 0, *result);
		return true;

	case MoonId_Handled:
		BOOLEAN_TO_NPVARIANT (event_args->GetHandled(), *result);
		return true;

	default:
		return MoonlightRoutedEventArgs::GetProperty (id, name, result);
	}
}

bool
MoonlightMouseEventArgsObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	MouseEventArgs *event_args = GetMouseEventArgs ();

	switch (id) {
	case MoonId_Handled:
		if (NPVARIANT_IS_BOOLEAN (*value))
			event_args->SetHandled (NPVARIANT_TO_BOOLEAN (*value));
		return true;
	default:
		return MoonlightRoutedEventArgs::SetProperty (id, name, value);
	}
}

bool
MoonlightMouseEventArgsObject::Invoke (int id, NPIdentifier name,
				       const NPVariant *args, guint32 argCount,
				       NPVariant *result)
{
	MouseEventArgs *event_args = GetMouseEventArgs ();

	switch (id) {
	case MoonId_GetPosition: {
		if (!check_arg_list ("(no)", argCount, args) && (!NPVARIANT_IS_NULL(args[0]) || !npvariant_is_dependency_object (args[0])))
			return true;

		double x;
		double y;

		// The argument is an element
		// to calculate the position with respect to (or null
		// for screen space)

		UIElement *el = NULL;

		if (npvariant_is_dependency_object (args[0])) {
			DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
			if (dob->Is (Type::UIELEMENT))
				el = (UIElement *)dob;
		}

		event_args->GetPosition (el, &x, &y);

		MoonlightPoint *point = (MoonlightPoint*)MOON_NPN_CreateObject (GetInstance (), MoonlightPointClass);
		point->point = Point (x, y);

		OBJECT_TO_NPVARIANT (point, *result);

		return true;
	}
	case MoonId_GetStylusInfo: {
		if (argCount != 0)
			THROW_JS_EXCEPTION ("getStylusInfo");

		StylusInfo *info = event_args->GetStylusInfo ();
		MoonlightEventObjectObject *info_obj = EventObjectCreateWrapper (GetPlugin (), info);
		info->unref ();
		OBJECT_TO_NPVARIANT (info_obj, *result);
		
		return true;
	}
	case MoonId_GetStylusPoints: {
		if (!check_arg_list ("o", argCount, args))
			THROW_JS_EXCEPTION ("getStylusPoints");

		if (npvariant_is_dependency_object (args[0])) {
			DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
			if (!dob->Is (Type::INKPRESENTER))
				THROW_JS_EXCEPTION ("getStylusPoints");
			
			StylusPointCollection *points = event_args->GetStylusPoints ((UIElement*)dob);
			MoonlightEventObjectObject *col_obj = EventObjectCreateWrapper (GetPlugin (), points);
			points->unref ();
			OBJECT_TO_NPVARIANT (col_obj, *result);
		}

		return true;
	}
	default:
		return MoonlightRoutedEventArgs::Invoke (id, name, args, argCount, result);
	}
}


MoonlightMouseEventArgsType::MoonlightMouseEventArgsType ()
{
	allocate = mouse_event_allocate;

	AddMapping (mouse_event_mapping, G_N_ELEMENTS (mouse_event_mapping));
}

MoonlightMouseEventArgsType *MoonlightMouseEventArgsClass;


/*** MoonlightTimelineMarkerRoutedEventArgsClass  **************************************************************/

static NPObject *
timeline_marker_routed_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightTimelineMarkerRoutedEventArgsObject (instance);
}

static const MoonNameIdMapping
timeline_marker_routed_event_mapping[] = {
	{ "marker", MoonId_Marker }
};

bool
MoonlightTimelineMarkerRoutedEventArgsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	TimelineMarkerRoutedEventArgs *args = GetTimelineMarkerRoutedEventArgs ();
	TimelineMarker *marker = args ? args->GetMarker () : NULL;

	switch (id) {
	case MoonId_Marker: {
		MoonlightEventObjectObject *meoo = EventObjectCreateWrapper (GetPlugin (), marker);
		OBJECT_TO_NPVARIANT (meoo, *result);
		return true;
	}
	default:
		return MoonlightEventArgs::GetProperty (id, name, result);;
	}
}

MoonlightTimelineMarkerRoutedEventArgsType::MoonlightTimelineMarkerRoutedEventArgsType ()
{
	allocate = timeline_marker_routed_event_allocate;

	AddMapping (timeline_marker_routed_event_mapping, G_N_ELEMENTS (timeline_marker_routed_event_mapping));
}

MoonlightTimelineMarkerRoutedEventArgsType *MoonlightTimelineMarkerRoutedEventArgsClass;

/*** MoonlightKeyEventArgsClass  **************************************************************/

static NPObject *
keyboard_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightKeyEventArgsObject (instance);
}

static const MoonNameIdMapping
keyboard_event_mapping[] = {
	{ "ctrl", MoonId_Ctrl },
	{ "handled", MoonId_Handled },
	{ "key", MoonId_Key },
	{ "platformkeycode", MoonId_PlatformKeyCode },
	{ "shift", MoonId_Shift },
};


bool
MoonlightKeyEventArgsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	KeyEventArgs *args = GetKeyEventArgs ();

	switch (id) {
	case MoonId_Shift:
		BOOLEAN_TO_NPVARIANT ((args->GetModifiers () & GDK_SHIFT_MASK) != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT ((args->GetModifiers () & GDK_CONTROL_MASK) != 0, *result);
		return true;

	case MoonId_Handled:
		BOOLEAN_TO_NPVARIANT (args->GetHandled(), *result);
		return true;

	case MoonId_Key:
		INT32_TO_NPVARIANT (args->GetKey (), *result);
		return true;

	case MoonId_PlatformKeyCode:
		INT32_TO_NPVARIANT (args->GetPlatformKeyCode (), *result);
		return true;

	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}
}

MoonlightKeyEventArgsType::MoonlightKeyEventArgsType ()
{
	allocate = keyboard_event_allocate;

	AddMapping (keyboard_event_mapping, G_N_ELEMENTS (keyboard_event_mapping));
}

MoonlightKeyEventArgsType *MoonlightKeyEventArgsClass;

/*** our object base class */
NPObject *
_allocate (NPP instance, NPClass *klass)
{
	PluginInstance *plugin = (PluginInstance *)instance->pdata;

	if (plugin)
		Deployment::SetCurrent (plugin->GetDeployment ());

	return new MoonlightObject (instance);
}

static void
_set_deployment (NPObject *npobj)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	PluginInstance *instance = obj->GetPlugin ();

	if (instance)
		Deployment::SetCurrent (instance->GetDeployment ());
}

static void
_deallocate (NPObject *npobj)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	
	delete obj;
}

static void
detach_xaml_proxy (gpointer key, gpointer value, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy*)value;
	proxy->SetOwner (NULL);
}

NPObject *
moonlight_object_to_npobject (MoonlightObject *obj)
{
	return (NPObject *) obj;
}

MoonlightObject *
npobject_to_moonlight_object (NPObject *obj)
{
	return (MoonlightObject *) obj;
}

MoonlightObject::~MoonlightObject ()
{
	if (event_listener_proxies) {
		g_hash_table_foreach (event_listener_proxies, detach_xaml_proxy, NULL);
		g_hash_table_destroy (event_listener_proxies);
		event_listener_proxies = NULL;
	}
	if (plugin)
		plugin->unref ();
}

void
MoonlightObject::destroy_proxy (gpointer data)
{
	EventListenerProxy *proxy = (EventListenerProxy*)data;
	proxy->RemoveHandler ();
}

bool
MoonlightObject::HasProperty (NPIdentifier name)
{
	return IS_PROPERTY (LookupName (name));
}

bool
MoonlightObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	NULL_TO_NPVARIANT (*result);
	THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETVALUE");
	return true;
}

bool
MoonlightObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
	return true;
}

bool
MoonlightObject::HasMethod (NPIdentifier name)
{
	return IS_METHOD (LookupName (name));
}

bool
MoonlightObject::Invoke (int id, NPIdentifier name, const NPVariant *args, guint32 argCount, NPVariant *result)
{
	switch (id) {
	case MoonId_ToString:
		if (argCount != 0)
			return false;

		if (moonlight_type != Type::INVALID) {
			string_to_npvariant (Type::Find (GetPlugin ()->GetDeployment (), moonlight_type)->GetName (), result);
			return true;
		} else {
			//string_to_npvariant ("", result);
			NULL_TO_NPVARIANT (*result);
			return true;
		}
		break;
	}

	return false;
}


EventListenerProxy *
MoonlightObject::LookupEventProxy (int event_id)
{
	return (EventListenerProxy*)g_hash_table_lookup (event_listener_proxies, GINT_TO_POINTER (event_id));
}

void
MoonlightObject::SetEventProxy (EventListenerProxy *proxy)
{
	g_hash_table_insert (event_listener_proxies, GINT_TO_POINTER (proxy->GetEventId()), proxy);
}

void
MoonlightObject::ClearEventProxies ()
{
	g_hash_table_foreach (event_listener_proxies, detach_xaml_proxy, NULL);
#if GTK_CHECK_VERSION(2,12,0)
	g_hash_table_remove_all (event_listener_proxies);
#else
	g_hash_table_destroy (event_listener_proxies);
	event_listener_proxies = g_hash_table_new (g_direct_hash, g_direct_equal);
#endif
}

void
MoonlightObject::ClearEventProxy (EventListenerProxy *proxy)
{
	proxy->SetOwner (NULL);

#if SANITY
	EventListenerProxy *p = LookupEventProxy (proxy->GetEventId());
	if (!p)
		abort();
#endif
	g_hash_table_remove (event_listener_proxies, GINT_TO_POINTER (proxy->GetEventId()));
}


static void
_invalidate (NPObject *npobj)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	
	obj->Invalidate ();
}

static bool
_has_method (NPObject *npobj, NPIdentifier name)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	return obj->HasMethod (name);
}

static bool
_has_property (NPObject *npobj, NPIdentifier name)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	return obj->HasProperty (name);
}

static bool
_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	_set_deployment (npobj);
#if ds(!)0
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	printf ("getting object property %s\n", strname);
	MOON_NPN_MemFree (strname);
#endif

	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	bool ret = obj->GetProperty (id, name, result);
#if ds(!)0
	printf ("==>done ==> %p\n", NPVARIANT_TO_OBJECT (*result));
#endif
	return ret;
}

static bool
_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	return obj->SetProperty (id, name, value);
}

static bool
_remove_property (NPObject *npobj, NPIdentifier name)
{
	_set_deployment (npobj);
	g_warning ("moonlight_object_remove_property reached");
	return false;
}

static bool
_enumerate (NPObject *npobj, NPIdentifier **value, guint32 *count)
{
	_set_deployment (npobj);
	return ((MoonlightObjectType*)npobj->_class)->Enumerate (value, count);
}

static bool
_invoke (NPObject *npobj, NPIdentifier name,
	 const NPVariant *args, guint32 argCount,
	 NPVariant *result)
{
	_set_deployment (npobj);
	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	return obj->Invoke (id, name, args, argCount, result);
}

static bool
_invoke_default (NPObject *npobj,
		 const NPVariant *args, guint32 argCount,
		 NPVariant *result)
{
	_set_deployment (npobj);
	g_warning ("moonlight_object_invoke_default reached");
	return false;
}

static const MoonNameIdMapping
object_mapping[] = {
	{ "tostring", MoonId_ToString },
};

MoonlightObjectType::MoonlightObjectType ()
{
	structVersion  = 0; /* strictly not needed, but it prevents head-scratching due to uninitialized fields in gdb */
	allocate       = _allocate;
	construct      = NULL;
	deallocate     = _deallocate;
	invalidate     = _invalidate;
	hasMethod      = _has_method;
	invoke         = _invoke;
	invokeDefault  = _invoke_default;
	hasProperty    = _has_property;
	getProperty    = _get_property;
	setProperty    = _set_property;
	removeProperty = _remove_property;
	enumerate      = _enumerate;

	mapping = NULL;
	mapping_count = 0;

	AddMapping (object_mapping, G_N_ELEMENTS (object_mapping));

	last_lookup = NULL;
	last_id = 0;
}

bool
MoonlightObjectType::Enumerate (NPIdentifier **value, guint32 *count)
{
	if (mapping_count == 0) {
		*value = NULL;
		*count = 0;
		return true;
	}

	// caller frees this
	NPIdentifier *ids = (NPIdentifier*)MOON_NPN_MemAlloc (sizeof (NPIdentifier) * mapping_count);

	for (int i = 0; i < mapping_count; i ++)
		ids[i] = MOON_NPN_GetStringIdentifier (mapping[i].name);

	*count = mapping_count;
	*value = ids;

	return true;
}

void
MoonlightObjectType::AddMapping (const MoonNameIdMapping *mapping, int count)
{
	if (this->mapping) {
		MoonNameIdMapping *new_mapping = (MoonNameIdMapping *) g_new (MoonNameIdMapping, count + mapping_count);
		
		memmove (new_mapping, this->mapping, mapping_count * sizeof (MoonNameIdMapping));
		memmove ((char *) new_mapping + (mapping_count * sizeof (MoonNameIdMapping)), mapping, count * sizeof (MoonNameIdMapping));
		g_free (this->mapping);
		this->mapping = new_mapping;
		mapping_count += count;
	} else {
		this->mapping = (MoonNameIdMapping *) g_new (MoonNameIdMapping, count);
		
		memmove (this->mapping, mapping, count * sizeof (MoonNameIdMapping));
		mapping_count = count;
	}
	
	qsort (this->mapping, mapping_count, sizeof (MoonNameIdMapping), compare_mapping);
}

int
MoonlightObjectType::LookupName (NPIdentifier name)
{
	if (last_lookup == name)
		return last_id;
	
	int id = map_name_to_id (name, mapping, mapping_count);
	
	if (id) {
		/* only cache hits */
		last_lookup = name;
		last_id = id;
	}
	
	return id;
}

MoonlightObjectType *MoonlightObjectClass;

/*** MoonlightScriptControlClass **********************************************************/
static NPObject *
moonlight_scriptable_control_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightScriptControlObject (instance);
}

static const MoonNameIdMapping
scriptable_control_mapping[] = {
	{ "content", MoonId_Content },
	{ "isloaded", MoonId_IsLoaded },
	{ "createobject", MoonId_CreateObject },
	{ "initparams", MoonId_InitParams },
	{ "id", MoonId_Id },
	{ "isversionsupported", MoonId_IsVersionSupported },
	{ "onerror", MoonId_OnError },
	{ "onload", MoonId_OnLoad },
	{ "settings", MoonId_Settings },
	{ "source", MoonId_Source },
	{ "onsourcedownloadprogresschanged", MoonId_OnSourceDownloadProgressChanged },
	{ "onsourcedownloadcomplete", MoonId_OnSourceDownloadComplete },
};

MoonlightScriptControlObject::~MoonlightScriptControlObject ()
{
	if (settings) {
		MOON_NPN_ReleaseObject (settings);
		settings = NULL;
	}
	
 	if (content) {
		MOON_NPN_ReleaseObject (content);
		content = NULL;
	}
}

void
MoonlightScriptControlObject::Invalidate ()
{
	MoonlightObject::Invalidate ();

	settings = NULL;
	content = NULL;
}

bool
MoonlightScriptControlObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = GetPlugin ();
	
	switch (id) {
	case MoonId_Settings:
		MOON_NPN_RetainObject (settings);
		OBJECT_TO_NPVARIANT (settings, *result);
		return true;
	case MoonId_Content:
		MOON_NPN_RetainObject (content);
		OBJECT_TO_NPVARIANT (content, *result);
		return true;
	case MoonId_InitParams:
		string_to_npvariant (plugin->GetInitParams (), result);
		return true;
	case MoonId_IsLoaded:
		BOOLEAN_TO_NPVARIANT (plugin->IsLoaded(), *result);
		return true;
	case MoonId_OnError:
	case MoonId_OnLoad:
	case MoonId_OnSourceDownloadProgressChanged:
	case MoonId_OnSourceDownloadComplete: {
		const char *event_name = map_moon_id_to_event_name (id);
		EventObject *obj = plugin->GetSurface ();

		if (obj != NULL) {
			int event_id = obj->GetType()->LookupEvent (event_name);
			EventListenerProxy *proxy = LookupEventProxy (event_id);
			string_to_npvariant (proxy == NULL ? "" : proxy->GetCallbackAsString (), result);
		} else {
			string_to_npvariant ("", result);
		}
		return true;
	}
	case MoonId_Source:
		string_to_npvariant (plugin->GetSource (), result);
		return true;

	case MoonId_Id: {
		char *id = plugin->GetId ();
		if (id)
			string_to_npvariant (id, result);
		else 
			NULL_TO_NPVARIANT (*result);

		return true;
	}
	
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightScriptControlObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = GetPlugin ();

	switch (id) {
	case MoonId_Source: {
		char *source = STRDUP_FROM_VARIANT (*value);
		plugin->SetSource (source);
		g_free (source);
		return true;
	}
	case MoonId_OnError:
	case MoonId_OnLoad:
	case MoonId_OnSourceDownloadProgressChanged:
	case MoonId_OnSourceDownloadComplete: {
		const char *event_name = map_moon_id_to_event_name (id);
		EventObject *obj = plugin->GetSurface ();

		if (obj != NULL) {
			int event_id = obj->GetType()->LookupEvent (event_name);

			if (event_id != -1) {
				EventListenerProxy *proxy = LookupEventProxy (event_id);
				if (proxy)
					proxy->RemoveHandler ();

				if (!NPVARIANT_IS_NULL (*value)) {
					EventListenerProxy *proxy = new EventListenerProxy (plugin,
											    event_name,
											    value);
					proxy->SetOwner (this);
					proxy->AddHandler (plugin->GetSurface());
					// we only emit that event once, when
					// the plugin is initialized, so don't
					// leave it in the event list
					// afterward.
					if (id == MoonId_OnLoad)
						proxy->SetOneShot ();
					SetEventProxy (proxy);
				}

				return true;
			}
		}
		return false;
	}
	case MoonId_InitParams: {
		char *init_params = STRDUP_FROM_VARIANT (*value);
		plugin->SetInitParams (init_params);
		g_free (init_params);
		return true;
	}
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

void
MoonlightScriptControlObject::PreSwitchPlugin (PluginInstance *old_plugin, PluginInstance *new_plugin)
{
	events_count = 6;
	events_is_func = (bool *) g_malloc0 (sizeof (bool) * events_count);
	events_callbacks = (gpointer *) g_malloc0 (sizeof (gpointer) * events_count);
	events_object = (MoonlightObject **) g_malloc0 (sizeof (MoonlightObject *) * events_count);
	events_to_switch = (int *) g_malloc0 (sizeof (int) * events_count);
	events_to_switch [0] = MoonId_OnError;
	events_object [0] = this;
	events_to_switch [1] = MoonId_OnLoad;
	events_object [1] = this;
	events_to_switch [2] = MoonId_OnSourceDownloadProgressChanged;
	events_object [2] = this;
	events_to_switch [3] = MoonId_OnSourceDownloadComplete;
	events_object [3] = this;
	events_to_switch [4] = MoonId_OnResize;
	events_object [4] = content;
	events_to_switch [5] = MoonId_OnFullScreenChange;
	events_object [5] = content;
	
	for (int i = 0; i < events_count; i++) {
		MoonlightObject *moonobj = events_object [i];
		const char *event_name = map_moon_id_to_event_name (events_to_switch [i]);
		EventObject *obj = old_plugin->GetSurface ();
		EventListenerProxy *proxy = NULL;
		
		if (obj == NULL || moonobj == NULL)
			continue;
		
		/* don't cause us to enter Deployment::GetCurrent here */
		Deployment *old_depl = old_plugin->GetDeployment ();
		/* obj->GetType () will call obj->GetDeployment (), which shows a warning if Deployment::GetCurrent doesn't match obj->GetDeployment () */
		Type *old_type = old_depl->GetTypes ()->Find (obj->GetObjectType ());
		proxy = moonobj->LookupEventProxy (old_type->LookupEvent (event_name));

		if (proxy == NULL)
			continue;
			
		events_callbacks [i] = proxy->GetCallback ();
		events_is_func [i] = proxy->IsFunc ();
		/*
		printf ("MoonlightScriptControlObject::PreSwitchPlugin () got event data %p / %s: IsFunc: %i\n", 
			events_callbacks [i], (const char *)  (events_is_func [i] ? NULL : events_callbacks [i]), events_is_func [i]);
		*/
		if (events_is_func [i])
			MOON_NPN_RetainObject ((NPObject *) events_callbacks [i]);
	}
	
	settings->SetPlugin (new_plugin);
	content->SetPlugin (new_plugin);
	SetPlugin (new_plugin);
	
	settings->ClearEventProxies ();
	content->ClearEventProxies ();
	ClearEventProxies ();
}

void
MoonlightScriptControlObject::PostSwitchPlugin (PluginInstance *old_plugin, PluginInstance *new_plugin)
{
	for (int i = 0; i < events_count; i++) {
		MoonlightObject *moonobj = events_object [i];
		
		if (events_callbacks [i] == NULL || moonobj == NULL)
			continue;
		
		NPVariant value;
		if (events_is_func [i]) {
			OBJECT_TO_NPVARIANT ((NPObject *) events_callbacks [i], value);
		} else {
			string_to_npvariant ((const char *) events_callbacks [i], &value);
		}
		
		moonobj->SetProperty (events_to_switch [i], 0, &value);

		if (events_is_func [i]) {
			MOON_NPN_ReleaseObject ((NPObject *) events_callbacks [i]);
		}
	}
}

bool
MoonlightScriptControlObject::Invoke (int id, NPIdentifier name,
				      const NPVariant *args, guint32 argCount,
				      NPVariant *result)
{
	switch (id) {
	case MoonId_CreateObject: {
		if (!check_arg_list ("s", argCount, args)) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		NPObject *obj = NULL;
		char *object_type = STRDUP_FROM_VARIANT (args [0]);
		if (!g_ascii_strcasecmp ("downloader", object_type)) {
			PluginInstance *plugin = GetPlugin ();
			Downloader *dl = PluginInstance::CreateDownloader (plugin);

			obj = EventObjectCreateWrapper (plugin, dl);
			dl->unref ();

			OBJECT_TO_NPVARIANT (obj, *result);
			g_free (object_type);
			return true;
		} else {
			NULL_TO_NPVARIANT (*result);
			g_free (object_type);

			THROW_JS_EXCEPTION ("createObject");
			return true;
		}
	}

	case MoonId_IsVersionSupported: {
		if (!check_arg_list ("s", argCount, args))
			return false;
		
		gchar *version_list = STRDUP_FROM_VARIANT (args [0]);
		bool supported = Surface::IsVersionSupported (version_list);
		g_free (version_list);
		
		BOOLEAN_TO_NPVARIANT (supported, *result);

		return true;
	}

	default:
		return MoonlightObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightScriptControlType::MoonlightScriptControlType ()
{
	allocate = moonlight_scriptable_control_allocate;

	AddMapping (scriptable_control_mapping, G_N_ELEMENTS (scriptable_control_mapping));
}

MoonlightScriptControlType *MoonlightScriptControlClass;

/*** MoonlightSettingsClass ***********************************************************/

static NPObject *
moonlight_settings_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightSettingsObject (instance);
}

static const MoonNameIdMapping
moonlight_settings_mapping [] = {
	{ "background", MoonId_Background },
	{ "enableframeratecounter", MoonId_EnableFramerateCounter },
	{ "enablehtmlaccess", MoonId_EnableHtmlAccess },
	{ "enableredrawregions", MoonId_EnableRedrawRegions },
	{ "maxframerate", MoonId_MaxFrameRate },
	{ "version", MoonId_Version },
	{ "windowless", MoonId_Windowless }
};

bool
MoonlightSettingsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = GetPlugin ();

	switch (id) {
	case MoonId_Background:
		string_to_npvariant (plugin->GetBackground (), result);
		return true;

	case MoonId_EnableFramerateCounter:
		BOOLEAN_TO_NPVARIANT (plugin->GetEnableFramerateCounter (), *result);
		return true;

	case MoonId_EnableRedrawRegions:
		BOOLEAN_TO_NPVARIANT (plugin->GetEnableRedrawRegions (), *result);
		return true;

	case MoonId_EnableHtmlAccess:
		BOOLEAN_TO_NPVARIANT (plugin->GetEnableHtmlAccess (), *result);
		return true;

	case MoonId_MaxFrameRate:
		INT32_TO_NPVARIANT (plugin->GetMaxFrameRate (), *result);
		return true;

	case MoonId_Version:
		string_to_npvariant (PLUGIN_VERSION, result);
		return true;

	case MoonId_Windowless:
		BOOLEAN_TO_NPVARIANT (plugin->GetWindowless (), *result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightSettingsObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = GetPlugin ();

	switch (id) {

	case MoonId_Background: {
		char *color = STRDUP_FROM_VARIANT (*value);
		if (!plugin->SetBackground (color)) {
			g_free (color);
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
		}
		g_free (color);

		return true;
	}
	// Cant be set after initialization so return true
	case MoonId_EnableFramerateCounter:
		plugin->SetEnableFramerateCounter (NPVARIANT_TO_BOOLEAN (*value));
		return true;
 
	case MoonId_EnableRedrawRegions:
		plugin->SetEnableRedrawRegions (NPVARIANT_TO_BOOLEAN (*value));
		return true;

	// Cant be set after initialization so return true
	case MoonId_EnableHtmlAccess:
		return true;

	// not implemented yet.
	case MoonId_MaxFrameRate:
		plugin->SetMaxFrameRate (NPVARIANT_TO_INT32 (*value));
		return true;

	// Cant be set after initialization so return true
	case MoonId_Windowless:
		return true;
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

bool
MoonlightSettingsObject::Invoke (int id, NPIdentifier name,
				 const NPVariant *args, guint32 argCount, NPVariant *result)
{
	switch (id) {
	case MoonId_ToString:
		if (argCount != 0)
			return false;

		string_to_npvariant ("Settings", result);
		return true;
	default:
		return MoonlightObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightSettingsType::MoonlightSettingsType ()
{
	allocate = moonlight_settings_allocate;
	AddMapping (moonlight_settings_mapping, G_N_ELEMENTS (moonlight_settings_mapping));
}

MoonlightSettingsType *MoonlightSettingsClass;


/*** MoonlightContentClass ************************************************************/
static NPObject *
moonlight_content_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightContentObject (instance);
}

MoonlightContentObject::~MoonlightContentObject ()
{
	// FIXME: need to free registered scriptable objects
	if (registered_scriptable_objects) {
		g_hash_table_destroy (registered_scriptable_objects);
		registered_scriptable_objects = NULL;
	}
	if (accessibility)
		accessibility->unref();
}

static const MoonNameIdMapping
moonlight_content_mapping[] = {
	{ "accessibility", MoonId_Accessibility },
	{ "actualheight", MoonId_ActualHeight },
	{ "actualwidth", MoonId_ActualWidth },
	{ "createfromxaml", MoonId_CreateFromXaml },
	{ "createfromxamldownloader", MoonId_CreateFromXamlDownloader },
	{ "createobject", MoonId_CreateObject },
	{ "findname", MoonId_FindName },
	{ "fullscreen", MoonId_FullScreen },
	{ "onfullscreenchange", MoonId_OnFullScreenChange },
	{ "onresize", MoonId_OnResize },
	{ "root", MoonId_Root },
};

bool
MoonlightContentObject::HasProperty (NPIdentifier name)
{
#if ds(!)0
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	printf ("content has property %s\n", strname);
	MOON_NPN_MemFree (strname);
#endif

	if (MoonlightObject::HasProperty (name))
		return true;

	bool ret = g_hash_table_lookup (registered_scriptable_objects, name) != NULL;
	return ret;
}

bool
MoonlightContentObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = GetPlugin ();

	switch (id) {
	case MoonId_Accessibility: {
		if (!accessibility)
			accessibility = new Accessibility ();
		MoonlightEventObjectObject *acc = EventObjectCreateWrapper (plugin, accessibility);

		OBJECT_TO_NPVARIANT (acc, *result);
		return true;
	}
	case MoonId_ActualHeight:
		INT32_TO_NPVARIANT (plugin->GetActualHeight (), *result);
		return true;
	case MoonId_ActualWidth:
		INT32_TO_NPVARIANT (plugin->GetActualWidth (), *result);
		return true;
	case MoonId_FullScreen:
		if (!plugin->GetSurface ()) {
			BOOLEAN_TO_NPVARIANT (false, *result);
		} else {
			BOOLEAN_TO_NPVARIANT (plugin->GetSurface()->GetFullScreen (), *result);
		}
		return true;
	case MoonId_OnResize:
	case MoonId_OnFullScreenChange: {
		Surface *surface = plugin->GetSurface ();
		const char *event_name;
		int event_id;
	
		if (surface == NULL) {
			string_to_npvariant ("", result);
		} else {
			event_name = map_moon_id_to_event_name (id);
			event_id = surface->GetType()->LookupEvent (event_name);
			EventListenerProxy *proxy = LookupEventProxy (event_id);
			string_to_npvariant (proxy == NULL ? "" : proxy->GetCallbackAsString (), result);
		}
		return true;
	}
	case MoonId_Root: {
		Surface *surface = plugin->GetSurface ();
		DependencyObject *top;

		if (surface == NULL) {
			NULL_TO_NPVARIANT (*result);
		} else if ((top = surface->GetToplevel ()) == NULL) {
			NULL_TO_NPVARIANT (*result);
		} else {
			MoonlightEventObjectObject *topobj = EventObjectCreateWrapper (plugin, top);

			OBJECT_TO_NPVARIANT (topobj, *result);
		}
		return true;
	}
	case NoMapping: {
		MoonlightScriptableObjectObject *obj;
		gpointer val;
		
		if (!(val = g_hash_table_lookup (registered_scriptable_objects, name)))
			return false;

		obj = (MoonlightScriptableObjectObject *) val;
		
		MOON_NPN_RetainObject (obj);
		OBJECT_TO_NPVARIANT (obj, *result);
		return true;
	}
	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightContentObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = GetPlugin ();
	Surface *surface = NULL;

	switch (id) {
	case MoonId_Accessibility:
		THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
		break;
	case MoonId_FullScreen:
		surface = plugin->GetSurface ();
		if (surface != NULL)
			surface->SetFullScreen (NPVARIANT_TO_BOOLEAN (*value));
		return true;
	case MoonId_OnFullScreenChange:
	case MoonId_OnResize: {
		const char *event_name = map_moon_id_to_event_name (id);
		int event_id;

		surface = plugin->GetSurface ();
		if (surface == NULL)
			return true;
			
		event_id  = surface->GetType()->LookupEvent (event_name);

		if (event_id != -1) {
			EventListenerProxy *proxy = LookupEventProxy (event_id);
			if (proxy)
				proxy->RemoveHandler();

			if (!NPVARIANT_IS_NULL (*value)) {
				EventListenerProxy *proxy = new EventListenerProxy (plugin,
										    event_name,
										    value);
				proxy->SetOwner (this);
				proxy->AddHandler (plugin->GetSurface());
				SetEventProxy (proxy);
			}

			return true;
		}
	}
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

bool
MoonlightContentObject::Invoke (int id, NPIdentifier name,
				const NPVariant *args, guint32 argCount, NPVariant *result)
{
	PluginInstance *plugin = GetPlugin ();
	
	switch (id) {
	case MoonId_FindName: {
		if (!check_arg_list ("s", argCount, args))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_FINDNAME");

		if (plugin->IsCrossDomainApplication ())
			THROW_JS_EXCEPTION ("XDomain Restriction");

		if (!plugin->GetSurface() || !plugin->GetSurface()->GetToplevel ())
			return true;

		char *name = STRDUP_FROM_VARIANT (args [0]);
		DependencyObject *element = plugin->GetSurface()->GetToplevel ()->FindName (name);
		g_free (name);

		if (!element) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (plugin, element), *result);
		return true;
	}

	case MoonId_CreateObject:
		// not implemented yet
		DEBUG_WARN_NOTIMPLEMENTED ("content.createObject");
		return true;

	case MoonId_CreateFromXaml: {
		if (!check_arg_list ("s[b]", argCount, args))
			THROW_JS_EXCEPTION ("createFromXaml argException");
		
		bool create_namescope = argCount >= 2 ? NPVARIANT_TO_BOOLEAN (args[1]) : false;
		char *xaml = STRDUP_FROM_VARIANT (args[0]);
		
		if (!xaml)
			THROW_JS_EXCEPTION ("createFromXaml argNullException");
		
		Type::Kind element_type;
		MoonError error;
		DependencyObject *dep = NULL;
		XamlLoader *loader = PluginXamlLoader::FromStr (NULL/*FIXME*/, xaml, plugin, plugin->GetSurface());

		loader->LoadVM ();

		Value *val = loader->CreateFromStringWithError (xaml, create_namescope, &element_type, XamlLoader::IMPORT_DEFAULT_XMLNS, &error);
		if (val && val->Is (plugin->GetDeployment (), Type::DEPENDENCY_OBJECT))
			dep = val->AsDependencyObject ();
			
		delete loader;
		g_free (xaml);
		
		if (!dep) {
			char *msg = g_strdup_printf ("createFromXaml error: %s", error.message);
			THROW_JS_EXCEPTION (msg);
			g_free (msg);
		}

		MoonlightEventObjectObject *depobj = EventObjectCreateWrapper (plugin, dep);
		delete val;

		OBJECT_TO_NPVARIANT (depobj, *result);
		return true;
	}

	case MoonId_CreateFromXamlDownloader: {
		if (!check_arg_list ("os", argCount, args))
			THROW_JS_EXCEPTION ("createFromXamlDownloader");
		
		Downloader *down = (Downloader*)((MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (args [0]))->GetDependencyObject ();
		DependencyObject *dep = NULL;
		Type::Kind element_type;
		
		char *path = STRDUP_FROM_VARIANT (args [1]);
		char *fname = down->GetDownloadedFilename (path);
		g_free (path);
		
		if (fname != NULL) {
			XamlLoader *loader = PluginXamlLoader::FromFilename (NULL/*FIXME*/, fname, plugin, plugin->GetSurface());
			dep = loader->CreateDependencyObjectFromFile (fname, false, &element_type);
			delete loader;

			g_free (fname);
		}

		if (!dep)
			THROW_JS_EXCEPTION ("createFromXamlDownloader");

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (plugin, dep), *result);
		dep->unref ();
		return true;
	}

	case MoonId_ToString: {
		if (argCount != 0)
			return false;

		string_to_npvariant ("Content", result);
		return true;
	}

	default:
		return MoonlightObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightContentType::MoonlightContentType ()
{
	allocate = moonlight_content_allocate;

	AddMapping (moonlight_content_mapping, G_N_ELEMENTS (moonlight_content_mapping));
}

MoonlightContentType *MoonlightContentClass;



/*** MoonlightDependencyObjectClass ***************************************************/

static const MoonNameIdMapping
moonlight_dependency_object_mapping [] = {
	{ "addeventlistener", MoonId_AddEventListener },
#if DEBUG_JAVASCRIPT
	{ "dumpnamescope", MoonId_DumpNameScope },
#endif
	{ "equals", MoonId_Equals },
	{ "findname", MoonId_FindName },
	{ "gethost", MoonId_GetHost },
	{ "getparent", MoonId_GetParent },
	{ "getvalue", MoonId_GetValue },
	{ "gotfocus", MoonId_GotFocus },
	{ "keydown", MoonId_KeyDown },
	{ "keyup", MoonId_KeyUp },
	{ "loaded", MoonId_Loaded },
	{ "lostfocus", MoonId_LostFocus },
	{ "mouseenter", MoonId_MouseEnter },
	{ "mouseleave", MoonId_MouseLeave },
	{ "mouseleftbuttondown", MoonId_MouseLeftButtonDown },
	{ "mouseleftbuttonup", MoonId_MouseLeftButtonUp },
	{ "mousemove", MoonId_MouseMove },
#if DEBUG_JAVASCRIPT
	{ "printf", MoonId_Printf },
#endif
	{ "removeeventlistener", MoonId_RemoveEventListener },
	{ "setvalue", MoonId_SetValue },
};

static NPObject *
moonlight_dependency_object_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightDependencyObjectObject (instance);
}

static DependencyProperty *
_get_dependency_property (DependencyObject *obj, char *attrname)
{
	// don't need to downcase here since dependency property lookup is already case insensitive
	DependencyProperty *p = obj->GetDependencyProperty (attrname);

	if (p)
		return p;

	char *period = strchr (attrname, '.');
	if (period) {
		char *type_name = g_strndup (attrname, period-attrname);
		attrname = period + 1;

		Type *type = Type::Find (obj->GetDeployment (), type_name);

		if (type != NULL)
			p = DependencyProperty::GetDependencyProperty (type, attrname);

		g_free (type_name);
	}

	return p;
}

static bool
_set_dependency_property_value (DependencyObject *dob, DependencyProperty *prop, const NPVariant *value, MoonError *error)
{
	if (npvariant_is_moonlight_object (*value)) {
		MoonlightObject *obj = (MoonlightObject *) NPVARIANT_TO_OBJECT (*value);
		MoonlightDuration *duration;
		MoonlightTimeSpan *ts;
		MoonlightPoint *point;
		MoonlightRect *rect;
		
		if (Type::IsSubclassOf (dob->GetDeployment (), obj->moonlight_type, Type::DEPENDENCY_OBJECT) && obj->moonlight_type != Type::INVALID) {
			MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (*value);
			dob->SetValueWithError (prop, Value (depobj->GetDependencyObject ()), error);
			
			return error->number == 0;
		}
		
		switch (obj->moonlight_type) {
		case Type::TIMESPAN:
			ts = (MoonlightTimeSpan *) obj;
			dob->SetValue (prop, Value (ts->GetValue (), Type::TIMESPAN));
			break;
		case Type::DURATION:
			duration = (MoonlightDuration *) obj;
			dob->SetValue (prop, Value (duration->GetValue ()));
			break;
		case Type::RECT:
			rect = (MoonlightRect *) obj;
			dob->SetValue (prop, Value (rect->rect));
			break;
		case Type::POINT:
			point = (MoonlightPoint *) obj;
			dob->SetValue (prop, Value (point->point));
			break;
		default:
			d(printf ("unhandled object type %d - %s in do.set_property\n",
				  obj->moonlight_type, Type::Find (dob->GetDeployment (), obj->moonlight_type)->GetName ()));
			w(printf ("unhandled object type in do.set_property\n"));
			return true;
		}
	} else {
		char *strval = NULL;
		char strbuf[64];
		bool rv;
		
		if (NPVARIANT_IS_BOOLEAN (*value)) {
			if (NPVARIANT_TO_BOOLEAN (*value))
				strcpy (strbuf, "true");
			else
				strcpy (strbuf, "false");
			
			strval = strbuf;
		} else if (NPVARIANT_IS_INT32 (*value)) {
			g_snprintf (strbuf, sizeof (strbuf), "%d", NPVARIANT_TO_INT32 (*value));
			
			strval = strbuf;
		} else if (NPVARIANT_IS_DOUBLE (*value)) {
			g_ascii_dtostr (strbuf, sizeof (strbuf), NPVARIANT_TO_DOUBLE (*value));
			
			strval = strbuf;
		} else if (NPVARIANT_IS_STRING (*value)) {
			strval = STRDUP_FROM_VARIANT (*value);
		} else if (NPVARIANT_IS_NULL (*value)) {
			if (Type::IsSubclassOf (dob->GetDeployment (), prop->GetPropertyType(), Type::DEPENDENCY_OBJECT)) {
				DependencyObject *val = NULL;
				
				dob->SetValueWithError (prop, Value (val), error);
			} else if (prop->GetPropertyType() == Type::STRING) {
				char *val = NULL;
				
				dob->SetValueWithError (prop, Value (val), error);
			} else 
				dob->SetValueWithError (prop, NULL, error);
			
			return error->number == 0;
		} else if (NPVARIANT_IS_VOID (*value)) {
			d(printf ("unhandled variant type VOID in do.set_property for (%s::%s)\n",
				  dob->GetTypeName (), prop->GetName()));
			return true;
		} else {
			d(printf ("unhandled variant type in do.set_property for (%s::%s)\n",
				  dob->GetTypeName (), prop->GetName()));
			return true;
		}
		
		rv = xaml_set_property_from_str (dob, prop, strval, error);
		
		if (strval != strbuf)
			g_free (strval);

		return rv;
	}
	
	return true;
}


bool
MoonlightDependencyObjectObject::HasProperty (NPIdentifier name)
{
	if (MoonlightObject::HasProperty (name))
		return true;

	DependencyObject *dob = GetDependencyObject ();

	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	if (!strname)
		return false;

	DependencyProperty *p = _get_dependency_property (dob, strname);
	MOON_NPN_MemFree (strname);

	return (p != NULL);
}

bool
MoonlightDependencyObjectObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	DependencyObject *dob = GetDependencyObject ();
	DependencyProperty *prop;
	const char *event_name;
	int event_id;

	if (!strname)
		return false;
	
	prop = _get_dependency_property (dob, strname);
	MOON_NPN_MemFree (strname);
	
	if (prop) {
		Value *value;
		// some default values are different between SL1 and SL2 (even if the SL1 apps runs under SL2)
		if (prop->GetId () == UIElement::RenderTransformProperty) {
			// e.g. the default RenderTransform is NULL for Javascript, unless it was setted by application code
			value = dob->ReadLocalValue (prop);
		} else if ((prop->GetId () == FrameworkElement::HeightProperty) || (prop->GetId () == FrameworkElement::WidthProperty)) {
			// e.g. the Width and Height are NaN in SL2 and 0 in Javascript (unless set to NaN in managed code)
			value = dob->ReadLocalValue (prop);
			if (!value) {
				DOUBLE_TO_NPVARIANT (0.0, *result);
				return true;
			}
		} else if (prop->GetId () == MediaElement::CurrentStateProperty) {
			// Javascript applications use strings, while managed use an enum
			int enum_value = dob->GetValue (prop)->AsInt32 ();
			const char *name = enums_int_to_str ("MediaState", enum_value);
			string_to_npvariant (name, result);
			return true;
		} else {
			value = dob->GetValue (prop);
		}

		if (!value) {
			// strings aren't null, they seem to just be empty strings
			if (prop->GetPropertyType() == Type::STRING) {
				string_to_npvariant ("", result);
				return true;
			}
			
			NULL_TO_NPVARIANT (*result);
			return true;
		}
		
		if (value->GetKind () == Type::INT32) {
			const char *s = enums_int_to_str (prop->GetName(), value->AsInt32 ());
			if (s)
				string_to_npvariant (s, result);
			else
				value_to_variant (this, value, result, dob, prop);
		} else
			value_to_variant (this, value, result, dob, prop);
		
		return true;
	}
	
	// it wasn't a dependency property.  let's see if it's an
	// event, and hook it up if it is valid on this object.
	if (!(event_name = map_moon_id_to_event_name (id)))
		return MoonlightObject::GetProperty (id, name, result);
	
	if ((event_id = dob->GetType()->LookupEvent (event_name)) == -1) {
#if false
		EventListenerProxy *proxy = LookupEventProxy (event_id);
		string_to_npvariant (proxy == NULL ? "" : proxy->GetCallbackAsString (), result);
		return true;
#else
		// on silverlight, these seem to always return ""
		// regardless of how we attempt to set them.
		string_to_npvariant ("", result);
		return true;
#endif
	}
	
	return MoonlightObject::GetProperty (id, name, result);
}

bool
MoonlightDependencyObjectObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	DependencyObject *dob = GetDependencyObject ();
	DependencyProperty *prop;
	
	if (!strname)
		return false;
	
	prop = _get_dependency_property (dob, strname);
	MOON_NPN_MemFree (strname);
	
	if (prop) {
		MoonError error;
		if (_set_dependency_property_value (dob, prop, value, &error)) {
			return true;
		} else {
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
		}
	}
	
	return MoonlightObject::SetProperty (id, name, value);
}

bool
MoonlightDependencyObjectObject::Invoke (int id, NPIdentifier name,
					 const NPVariant *args, guint32 argCount,
					 NPVariant *result)
{
	DependencyObject *dob = GetDependencyObject ();

	switch (id) {
#if DEBUG_JAVASCRIPT
	// Some debug code...
	// with this it is possible to do obj.printf ("msg") from js
	case MoonId_Printf: {
		char *message = STRDUP_FROM_VARIANT (args [0]);
		fprintf (stderr, "JS message: %s\n", message);
		g_free (message);
		VOID_TO_NPVARIANT (*result);
		return true;
	}
	case MoonId_DumpNameScope: {
		fprintf (stderr, "dumping namescope for object %p (%s)\n", dob, dob->GetTypeName());
		DependencyObject *ns_dob = dob;
		NameScope *ns;
		while (!(ns = NameScope::GetNameScope(ns_dob)))
			ns_dob = ns_dob->GetParent();
		if (ns_dob == NULL)
			fprintf (stderr, " no namescope in logical hierarchy!\n");
		else {
			if (ns_dob != dob)
				fprintf (stderr, "namescope is actually on object %p (%s)\n", ns_dob, ns_dob->GetTypeName());
			ns->Dump ();
		}
		return true;
	}
#endif
	case MoonId_Equals: {
		if (!check_arg_list ("o", argCount, args))
			THROW_JS_EXCEPTION ("equals");

		NPObject *o = NPVARIANT_TO_OBJECT (args[0]);
		if (npobject_is_dependency_object (o)) {
			MoonlightDependencyObjectObject *obj = (MoonlightDependencyObjectObject *) o;
			
			BOOLEAN_TO_NPVARIANT (obj->GetDependencyObject() == dob, *result);
		} else {
			BOOLEAN_TO_NPVARIANT (false, *result);
		}
		  
		return true;
	}

	case MoonId_SetValue: {
		/* obj.setValue (prop, val) is another way of writing obj[prop] = val (or obj.prop = val) */
		if (!check_arg_list ("s*", argCount, args))
			THROW_JS_EXCEPTION ("setValue");
		
		char *value = STRDUP_FROM_VARIANT (args [0]);
		_class->setProperty (this, NPID (value), &args[1]);
		g_free (value);

		VOID_TO_NPVARIANT (*result);
		return true;
	}

	case MoonId_GetValue: {
		if (!check_arg_list ("s", argCount, args))
			THROW_JS_EXCEPTION ("getValue");

		char *value = STRDUP_FROM_VARIANT (args [0]);
		_class->getProperty (this, NPID (value), result);
		g_free (value);
		
		return true;
	}

	case MoonId_FindName: {
		if (!check_arg_list ("s", argCount, args))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_FINDNAME");

		PluginInstance *plugin = GetPlugin ();
		if (plugin->IsCrossDomainApplication ())
			THROW_JS_EXCEPTION ("XDomain Restriction");

		char *name = STRDUP_FROM_VARIANT (args [0]);

		DependencyObject *element = dob->FindName (name);
		if (!element) {
			if (dob != plugin->GetSurface()->GetToplevel()) {
				// fall back to looking the object up on the toplevel element
				element = plugin->GetSurface()->GetToplevel ()->FindName (name);
			}

			if (!element) {
				g_free (name);
				NULL_TO_NPVARIANT (*result);
				return true;
			}
		}

		g_free (name);
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (plugin, element), *result);
		return true;
	}

	case MoonId_GetHost: {
		PluginInstance *plugin = GetPlugin ();
		
		if (argCount != 0)
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETHOST");

		OBJECT_TO_NPVARIANT (plugin->GetHost (), *result);

		return true;
	}

	case MoonId_GetParent: {
		if (argCount != 0 || !dob->GetType ()->IsSubclassOf (Type::FRAMEWORKELEMENT))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETPARENT");
		
		DependencyObject *parent = ((FrameworkElement*)dob)->GetLogicalParent ();
		if (parent)
			OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), parent), *result);
		else
			NULL_TO_NPVARIANT (*result);

		return true;
	}

	case MoonId_AddEventListener: {
		/* FIXME: how do we check if args[1] is a function? */
		if (!check_arg_list ("s(so)", argCount, args))
			THROW_JS_EXCEPTION ("addEventListener");
		
		char *name = STRDUP_FROM_VARIANT (args [0]);
		name[0] = toupper(name[0]);

		EventListenerProxy *proxy = new EventListenerProxy (GetPlugin (), name, &args[1]);
		int token = proxy->AddHandler (dob);
		g_free (name);

		if (token == -1)
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_ADDEVENT");

		INT32_TO_NPVARIANT (token, *result);
		return true;
	}
	case MoonId_RemoveEventListener: {
		if (!check_arg_list ("s(is)", argCount, args))
			THROW_JS_EXCEPTION ("removeEventListener");
		
		char *event = STRDUP_FROM_VARIANT (args[0]);
		int id = dob->GetType()->LookupEvent (event);
		g_free (event);
		
		if (id == -1) {
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_DELEVENT");
		} else if (NPVARIANT_IS_INT32 (args [1])) {
			dob->RemoveHandler (id, NPVARIANT_TO_INT32 (args[1]));
		} else if (NPVARIANT_IS_STRING (args[1])) {
			char *value = STRDUP_FROM_VARIANT (args[1]);
			NamedProxyPredicate predicate (value);
			g_free (value);
			
			dob->RemoveMatchingHandlers (id, NamedProxyPredicate::matches, &predicate);
		}
		
		return true;
	}

	// FIXME: these next two methods should live in a UIElement
	// wrapper class, not in the DependencyObject wrapper.
/*
	case MoonId_CaptureMouse:
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->CaptureMouse (), *result);
		return true;
	case MoonId_ReleaseMouseCapture:
		((UIElement*)dob)->ReleaseMouseCapture ();

		VOID_TO_NPVARIANT (*result);
		return true;
*/
	default:
		return MoonlightObject::Invoke (id, name, args, argCount, result);
	}
}


MoonlightDependencyObjectType::MoonlightDependencyObjectType ()
{
	allocate = moonlight_dependency_object_allocate;
	
	AddMapping (moonlight_dependency_object_mapping, G_N_ELEMENTS (moonlight_dependency_object_mapping));
}



/*** MoonlightEventObjectClass ***************************************************/

static NPObject *
moonlight_event_object_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightEventObjectObject (instance);
}

MoonlightEventObjectObject::~MoonlightEventObjectObject ()
{
	if (eo) {
		PluginInstance *plugin;
		if ((plugin = GetPlugin ()))
			plugin->RemoveWrappedObject (eo);
		
		moonlight_type = Type::INVALID;
		
		eo->unref ();
		eo = NULL;
	}
}

MoonlightEventObjectType::MoonlightEventObjectType ()
{
	allocate = moonlight_event_object_allocate;
}

MoonlightEventObjectType *MoonlightEventObjectClass;

MoonlightEventObjectObject *
EventObjectCreateWrapper (PluginInstance *plugin, EventObject *obj)
{
	NPP instance = plugin->GetInstance ();
	MoonlightEventObjectObject *depobj;
	NPClass *np_class;
	
	if (obj == NULL)
		return NULL;
	
	depobj = (MoonlightEventObjectObject *) plugin->LookupWrappedObject (obj);
	
	if (depobj) {
		MOON_NPN_RetainObject (depobj);
		return depobj;
	}
	
	/* for EventObject subclasses which have special plugin classes, check here */
	Type::Kind kind = obj->GetObjectType ();
	switch (kind) {
	case Type::STORYBOARD:
		np_class = dependency_object_classes [STORYBOARD_CLASS];
		break;
	case Type::MEDIAELEMENT:
		np_class = dependency_object_classes [MEDIA_ELEMENT_CLASS];
		break;
	case Type::DOWNLOADER:
		np_class = dependency_object_classes [DOWNLOADER_CLASS];
		break;
	case Type::IMAGE:
		np_class = dependency_object_classes [IMAGE_CLASS];
		break;
	case Type::IMAGEBRUSH:
		np_class = dependency_object_classes [IMAGE_BRUSH_CLASS];
		break;
	case Type::TEXTBLOCK:
		np_class = dependency_object_classes [TEXT_BLOCK_CLASS];
		break;
	case Type::TEXTBOX:
		np_class = dependency_object_classes [TEXT_BOX_CLASS];
		break;
	case Type::PASSWORDBOX:
		np_class = dependency_object_classes [PASSWORD_BOX_CLASS];
		break;
	case Type::MULTISCALEIMAGE:
		np_class = dependency_object_classes [MULTI_SCALE_IMAGE_CLASS];
		break;
	case Type::EVENTOBJECT: 
	case Type::SURFACE: 
		np_class = MoonlightEventObjectClass;
		break;
	case Type::STYLUSINFO:
		np_class = dependency_object_classes [STYLUS_INFO_CLASS];
		break;
	case Type::STYLUSPOINT_COLLECTION:
		np_class = dependency_object_classes [STYLUS_POINT_COLLECTION_CLASS];
		break;
	case Type::STROKE_COLLECTION:
		np_class = dependency_object_classes [STROKE_COLLECTION_CLASS];
		break;
	case Type::STROKE:
		np_class = dependency_object_classes [STROKE_CLASS];
		break;
	case Type::ROUTEDEVENTARGS:
		np_class = dependency_object_classes [ROUTED_EVENT_ARGS_CLASS];
		break;
	case Type::MOUSEEVENTARGS:
	case Type::MOUSEBUTTONEVENTARGS:
	case Type::MOUSEWHEELEVENTARGS:
		np_class = dependency_object_classes [MOUSE_EVENT_ARGS_CLASS];
		break;
	case Type::DOWNLOADPROGRESSEVENTARGS:
		np_class = dependency_object_classes [DOWNLOAD_PROGRESS_EVENT_ARGS_CLASS];
		break;
	case Type::KEYEVENTARGS:
		np_class = dependency_object_classes [KEY_EVENT_ARGS_CLASS];
		break;
	case Type::TIMELINEMARKERROUTEDEVENTARGS:
		np_class = dependency_object_classes [TIMELINE_MARKER_ROUTED_EVENT_ARGS_CLASS];
		break;
	case Type::ERROREVENTARGS:
	case Type::PARSERERROREVENTARGS:
	case Type::IMAGEERROREVENTARGS:
		np_class = dependency_object_classes [ERROR_EVENT_ARGS_CLASS];
		break;
	case Type::UIELEMENT:
		np_class = dependency_object_classes [UI_ELEMENT_CLASS];
		break;
	default:
		if (Type::Find (plugin->GetDeployment (), kind)->IsSubclassOf (Type::CONTROL))
			np_class = dependency_object_classes [CONTROL_CLASS];
		else if (Type::Find (plugin->GetDeployment (), kind)->IsSubclassOf (Type::UIELEMENT))
			np_class = dependency_object_classes [UI_ELEMENT_CLASS];
		else if (Type::Find (plugin->GetDeployment (), kind)->IsSubclassOf (Type::COLLECTION))
			np_class = dependency_object_classes [COLLECTION_CLASS];
		else if (Type::Find (plugin->GetDeployment (), kind)->IsSubclassOf (Type::EVENTARGS)) 
			np_class = dependency_object_classes [EVENT_ARGS_CLASS];
		else
			np_class = dependency_object_classes [DEPENDENCY_OBJECT_CLASS];
	}
	
	depobj = (MoonlightEventObjectObject *) MOON_NPN_CreateObject (instance, np_class);
	depobj->moonlight_type = obj->GetObjectType ();
	depobj->eo = obj;
	obj->ref ();
	
	plugin->AddWrappedObject (obj, depobj);
	
	return depobj;
}



/*** MoonlightCollectionClass ***************************************************/

static NPObject *
moonlight_collection_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightCollectionObject (instance);
}

static const MoonNameIdMapping
moonlight_collection_mapping [] = {
	{ "add", MoonId_Add },
	{ "clear", MoonId_Clear },
	{ "count", MoonId_Count },
	{ "getitem", MoonId_GetItem },
	{ "getitembyname", MoonId_GetItemByName },
	{ "insert", MoonId_Insert },
	{ "remove", MoonId_Remove },
	{ "removeat", MoonId_RemoveAt },
};

bool
MoonlightCollectionObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	Collection *col = (Collection *) GetDependencyObject ();

	switch (id) {
	case MoonId_Count:
		INT32_TO_NPVARIANT (col->GetCount (), *result);
		return true;
	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightCollectionObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, guint32 argCount,
				   NPVariant *result)
{
	Collection *col = (Collection *) GetDependencyObject ();
	
	switch (id) {
	case MoonId_Add: {
		if (!check_arg_list ("o", argCount, args) ||
		    !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("add");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);
		int n = col->Add (Value (el->GetDependencyObject ()));
		
		if (n == -1)
			THROW_JS_EXCEPTION ("add");
		
		INT32_TO_NPVARIANT (n, *result);
		
		return true;
	}
	case MoonId_Remove: {
		if (col->GetObjectType () == Type::RESOURCE_DICTIONARY) {
			if (check_arg_list ("s", argCount, args)) {
				bool res = ((ResourceDictionary*)col)->Remove (NPVARIANT_TO_STRING (args[0]).utf8characters);
				BOOLEAN_TO_NPVARIANT (res, *result);
				return true;
			}
		}

		if (!check_arg_list ("o", argCount, args) ||
		    !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("remove");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);
		bool res = col->Remove (Value (el->GetDependencyObject ()));
		
		BOOLEAN_TO_NPVARIANT (res, *result);
		
		return true;
	}
	case MoonId_RemoveAt: {
		if (!check_arg_list ("i", argCount, args))
			THROW_JS_EXCEPTION ("removeAt");
		
		int index = NPVARIANT_TO_INT32 (args [0]);
		
		if (index < 0 || index >= col->GetCount ())
			THROW_JS_EXCEPTION ("removeAt");
		
		DependencyObject *obj = col->GetValueAt (index)->AsDependencyObject ();
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), obj), *result);
		
		col->RemoveAt (index);
		
		return true;
	}
	case MoonId_Insert: {
		if (!check_arg_list ("i[o]", argCount, args)) {
			g_warning ("insert 1");
			THROW_JS_EXCEPTION ("insert");
		}
		
		if (argCount < 2) {
			VOID_TO_NPVARIANT (*result);
			return true;
		}
		
		if (!npvariant_is_dependency_object (args[1])) {
			g_warning ("insert 2");
			THROW_JS_EXCEPTION ("insert");
		}
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (args[1]);
		int index = NPVARIANT_TO_INT32 (args[0]);
		MoonError err;
		Value val (el->GetDependencyObject ());

		if (!col->InsertWithError (index, &val, &err)) {
			g_warning ("insert 2: %s", err.message);
			THROW_JS_EXCEPTION ("insert");
		}
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}
	case MoonId_Clear: {
		if (argCount != 0)
			THROW_JS_EXCEPTION ("clear");
		
		col->Clear ();
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}
	case MoonId_GetItem: {
		if (col->GetObjectType () == Type::RESOURCE_DICTIONARY) {
			if (check_arg_list ("s", argCount, args)) {
				// handle the string case here, we
				// handle the int case in the common
				// code below.
				bool unused;

				Value *v = ((ResourceDictionary*)col)->Get (NPVARIANT_TO_STRING (args[0]).utf8characters, &unused);
				value_to_variant (this, v, result);
				return true;
			}
		}

		if (!check_arg_list ("i", argCount, args))
			THROW_JS_EXCEPTION ("getItem");
		
		int index = NPVARIANT_TO_INT32 (args[0]);
		
		if (index < 0)
			THROW_JS_EXCEPTION ("getItem");
		
		if (index >= col->GetCount ()) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}
		
		DependencyObject *obj = col->GetValueAt (index)->AsDependencyObject ();
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), obj), *result);
		
		return true;
	}
	case MoonId_GetItemByName: {
		if (col->GetObjectType () != Type::MEDIAATTRIBUTE_COLLECTION ||
		    !check_arg_list ("s", argCount, args))
			THROW_JS_EXCEPTION ("getItemByName");
		
		char *name = STRDUP_FROM_VARIANT (args[0]);
		DependencyObject *obj = ((MediaAttributeCollection *) col)->GetItemByName (name);
		g_free (name);
		
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), obj), *result);
		
		return true;
	}
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}


MoonlightCollectionType::MoonlightCollectionType ()
{
	AddMapping (moonlight_collection_mapping, G_N_ELEMENTS (moonlight_collection_mapping));

	allocate = moonlight_collection_allocate;
}


/*** MoonlightStoryboardClass ***************************************************/

static NPObject *
moonlight_storyboard_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightStoryboardObject (instance);
}

static const MoonNameIdMapping
moonlight_storyboard_mapping [] = {
	{ "begin", MoonId_Begin },
	{ "completed", MoonId_Completed },
	{ "pause", MoonId_Pause },
	{ "resume", MoonId_Resume },
	{ "seek", MoonId_Seek },
	{ "stop", MoonId_Stop }
};

bool
MoonlightStoryboardObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, guint32 argCount,
				   NPVariant *result)
{
	// FIXME: all the WithError calls should be using a MoonError,
	// which we convert to a JS exception and throw if there's a
	// problem.
	Storyboard *sb = (Storyboard*)GetDependencyObject ();

	switch (id) {
	case MoonId_Begin:
		if (argCount != 0 || !sb->BeginWithError (NULL))
			THROW_JS_EXCEPTION ("begin");
		
		VOID_TO_NPVARIANT (*result);

		return true;
	case MoonId_Pause:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("pause");

		sb->PauseWithError (NULL);

		VOID_TO_NPVARIANT (*result);

		return true;
	case MoonId_Resume:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("resume");

		sb->ResumeWithError (NULL);

		VOID_TO_NPVARIANT (*result);

		return true;
	case MoonId_Seek: {
		if (!check_arg_list ("(is)", argCount, args))
			THROW_JS_EXCEPTION ("seek");
		
		TimeSpan ts;
		bool ok;
		
		if (NPVARIANT_IS_INT32 (args[0])) {
			ts = (TimeSpan) NPVARIANT_TO_INT32 (args[0]);
		} else if (NPVARIANT_IS_STRING (args[0])) {
			char *span = STRDUP_FROM_VARIANT (args[0]);
			ok = time_span_from_str (span, &ts);
			g_free (span);
			
			if (!ok)
				THROW_JS_EXCEPTION ("seek");
		}
		
		sb->SeekWithError (ts, NULL);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	case MoonId_Stop:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("stop");

		sb->StopWithError (NULL);

		VOID_TO_NPVARIANT (*result);

		return true;
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightStoryboardType::MoonlightStoryboardType ()
{
	AddMapping (moonlight_storyboard_mapping, G_N_ELEMENTS (moonlight_storyboard_mapping));

	allocate = moonlight_storyboard_allocate;
}


/*** MoonlightMediaElementClass ***************************************************/

static NPObject *
moonlight_media_element_allocate (NPP instance, NPClass *)
{
	return new MoonlightMediaElementObject (instance);
}

static const MoonNameIdMapping
moonlight_media_element_mapping [] = {
	{ "bufferingprogresschanged", MoonId_BufferingProgressChanged },
	{ "currentstatechanged", MoonId_CurrentStateChanged },
	{ "downloadprogresschanged", MoonId_DownloadProgressChanged },
	{ "markerreached", MoonId_MarkerReached },
	{ "mediaended", MoonId_MediaEnded },
	{ "mediafailed", MoonId_MediaFailed },
	{ "mediaopened", MoonId_MediaOpened },
	{ "pause", MoonId_Pause },
	{ "play", MoonId_Play },
	{ "setsource", MoonId_SetSource },
	{ "stop", MoonId_Stop },
};

bool
MoonlightMediaElementObject::Invoke (int id, NPIdentifier name,
				     const NPVariant *args, guint32 argCount,
				     NPVariant *result)
{
	MediaElement *media = (MediaElement*)GetDependencyObject ();
	
	switch (id) {
	case MoonId_Play:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("play");

		media->Play ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Pause:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("pause");

		media->Pause ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Stop:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("stop");

		media->Stop ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_SetSource: {
		if (!check_arg_list ("os", argCount, args) ||
		    !npvariant_is_downloader (args[0]))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_METHOD");
		
		DependencyObject *downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();

		char *part = STRDUP_FROM_VARIANT (args [1]);
		media->SetSource ((Downloader *) downloader, part);
		g_free (part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	default:
		return MoonlightUIElementObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightMediaElementType::MoonlightMediaElementType ()
{
	AddMapping (moonlight_media_element_mapping, G_N_ELEMENTS (moonlight_media_element_mapping));

	allocate = moonlight_media_element_allocate;
}

/*** MoonlightMultiScaleImageClass *********/
static NPObject *
moonlight_multiscaleimage_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMultiScaleImageObject (instance);
}

static const MoonNameIdMapping moonlight_multiscaleimage_mapping[] = {
	{"getithsubimage", MoonId_MultiScaleImage_GetIthSubImage},
	{"getsubimagecount", MoonId_MultiScaleImage_GetSubImageCount},
	{"logicaltoelementx", MoonId_MultiScaleImage_LogicalToElementX},
	{"logicaltoelementy", MoonId_MultiScaleImage_LogicalToElementY},
	{"source", MoonId_MultiScaleImage_Source },
	{"zoomaboutlogicalpoint", MoonId_MultiScaleImage_ZoomAboutLogicalPoint}
};

bool
MoonlightMultiScaleImageObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, guint32 argCount,
				   NPVariant *result)
{
	MultiScaleImage *dob = (MultiScaleImage*)GetDependencyObject ();

	switch (id) {

		case MoonId_MultiScaleImage_GetIthSubImage: {
			if (!check_arg_list ("i", argCount, args))
				THROW_JS_EXCEPTION ("GetIthSubImage");
			int arg0 = NPVARIANT_TO_INT32 (args[0]);
			MultiScaleSubImage * ret = dob->GetIthSubImage(arg0);
			if (ret)
				OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), ret), *result);
			else
				NULL_TO_NPVARIANT (*result);
			return true;
			break;
		}

		case MoonId_MultiScaleImage_GetSubImageCount: {
			int ret = dob->GetSubImageCount();
			INT32_TO_NPVARIANT (ret, *result);
			return true;
			break;
		}

		case MoonId_MultiScaleImage_LogicalToElementX: {
			if (!check_arg_list ("ii", argCount, args))
				THROW_JS_EXCEPTION ("LogicalToElementX");
			int arg0 = NPVARIANT_TO_INT32 (args[0]);
			int arg1 = NPVARIANT_TO_INT32 (args[1]);
			int ret = dob->LogicalToElementX(arg0,arg1);
			INT32_TO_NPVARIANT (ret, *result);
			return true;
			break;
		}

		case MoonId_MultiScaleImage_LogicalToElementY: {
			if (!check_arg_list ("ii", argCount, args))
				THROW_JS_EXCEPTION ("LogicalToElementY");
			int arg0 = NPVARIANT_TO_INT32 (args[0]);
			int arg1 = NPVARIANT_TO_INT32 (args[1]);
			int ret = dob->LogicalToElementY(arg0,arg1);
			INT32_TO_NPVARIANT (ret, *result);
			return true;
			break;
		}

		case MoonId_MultiScaleImage_ZoomAboutLogicalPoint: {
			if (!check_arg_list ("ddd", argCount, args))
				THROW_JS_EXCEPTION ("ZoomAboutLogicalPoint");
			double arg0 = NPVARIANT_TO_DOUBLE (args[0]);
			double arg1 = NPVARIANT_TO_DOUBLE (args[1]);
			double arg2 = NPVARIANT_TO_DOUBLE (args[2]);
			dob->ZoomAboutLogicalPoint(arg0,arg1,arg2);
			VOID_TO_NPVARIANT (*result);
			return true;
			break;
		}
	}

	return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
}

bool
MoonlightMultiScaleImageObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	MultiScaleImage *msi = (MultiScaleImage *) GetDependencyObject ();
	switch (id) {
	case MoonId_MultiScaleImage_Source: {
		MultiScaleTileSource *ts = (MultiScaleTileSource *) msi->GetSource ();
		if (ts && ts->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
			Uri *uri = new Uri ();

			uri->Parse (NPVARIANT_TO_STRING (*value).utf8characters);
			((DeepZoomImageTileSource *)ts)->SetUriSource (uri);
			delete uri;
			return true;
		}
	}
	default:
		return MoonlightDependencyObjectObject::SetProperty (id, name, value);;
	}
}

MoonlightMultiScaleImageType::MoonlightMultiScaleImageType ()
{
	AddMapping (moonlight_multiscaleimage_mapping, G_N_ELEMENTS (moonlight_multiscaleimage_mapping));

	allocate = moonlight_multiscaleimage_allocate;
}

/*** MoonlightImageClass ***************************************************/

static NPObject *
moonlight_image_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightImageObject (instance);
}

static const MoonNameIdMapping
moonlight_image_mapping [] = {
	{ "downloadprogresschanged", MoonId_DownloadProgressChanged },
	{ "imagefailed", MoonId_ImageFailed },
	{ "source", MoonId_Source },
	{ "setsource", MoonId_SetSource }
};

bool
MoonlightImageObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	Image *img = (Image *) GetDependencyObject ();

	switch (id) {
	case MoonId_Source: {
		ImageSource *source = (ImageSource *) img->GetSource ();
		if (source && source->Is (Type::BITMAPIMAGE)) {
			char *uri = ((BitmapImage *)source)->GetUriSource()->ToString();
			STRINGN_TO_NPVARIANT (uri, strlen (uri), *result);
		} else {
			NULL_TO_NPVARIANT (*result);
		}

		return true;
	}

	default:
		return MoonlightUIElementObject::GetProperty (id, name, result);
	}
}

bool
MoonlightImageObject::Invoke (int id, NPIdentifier name,
			      const NPVariant *args, guint32 argCount,
			      NPVariant *result)
{
	Image *img = (Image *) GetDependencyObject ();
	DependencyObject *downloader;
	char *part;
	
	switch (id) {
	case MoonId_SetSource:
		if (!check_arg_list ("os", argCount, args) ||
		    !npvariant_is_downloader (args[0]))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_METHOD");
		
		downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();

		part = STRDUP_FROM_VARIANT (args [1]);
		img->SetSource ((Downloader *) downloader, part);
		g_free (part);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	default:
		return MoonlightUIElementObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightImageType::MoonlightImageType ()
{
	AddMapping (moonlight_image_mapping, G_N_ELEMENTS (moonlight_image_mapping));

	allocate = moonlight_image_allocate;
}


/*** MoonlightImageBrushClass ***************************************************/

static NPObject *
moonlight_image_brush_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightImageBrushObject (instance);
}


static const MoonNameIdMapping
moonlight_image_brush_mapping [] = {
	{ "downloadprogresschanged", MoonId_DownloadProgressChanged },
	{ "imagesource", MoonId_Source },
	{ "setsource", MoonId_SetSource }
};

bool
MoonlightImageBrushObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	ImageBrush *brush = (ImageBrush *) GetDependencyObject ();

	switch (id) {
	case MoonId_Source: {
		ImageSource *source = (ImageSource *) brush->GetImageSource ();
		if (source && source->Is (Type::BITMAPIMAGE)) {
			char *uri = ((BitmapImage *)source)->GetUriSource()->ToString();
			STRINGN_TO_NPVARIANT (uri, strlen (uri), *result);
		} else {
			NULL_TO_NPVARIANT (*result);
		}

		return true;
	}

	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightImageBrushObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, guint32 argCount,
				   NPVariant *result)
{
	ImageBrush *img = (ImageBrush *) GetDependencyObject ();
	DependencyObject *downloader;
	
	switch (id) {
	case MoonId_SetSource: {
		if (!check_arg_list ("os", argCount, args) ||
		    !npvariant_is_downloader (args[0]))
			THROW_JS_EXCEPTION ("setSource");
		
		downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();

		char *part = STRDUP_FROM_VARIANT (args [1]);
		img->SetSource ((Downloader *) downloader, part);
		g_free (part);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightImageBrushType::MoonlightImageBrushType ()
{
	AddMapping (moonlight_image_brush_mapping, G_N_ELEMENTS (moonlight_image_brush_mapping));

	allocate = moonlight_image_brush_allocate;
}


/*** MoonlightControlClass ***************************************************/

static NPObject *
moonlight_control_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightControlObject (instance);
}


static const MoonNameIdMapping moonlight_control_mapping[] = {
	{ "focus", MoonId_Focus }
};


bool
MoonlightControlObject::Invoke (int id, NPIdentifier name,
				  const NPVariant *args, guint32 argCount,
				  NPVariant *result)
{
	Control *control = (Control *) GetDependencyObject ();
	
	switch (id) {
	case MoonId_Focus:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("focus");

		BOOLEAN_TO_NPVARIANT (control->Focus (), *result);
		
		return true;
	default:
		return MoonlightUIElementObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightControlType::MoonlightControlType ()
{
	AddMapping (moonlight_control_mapping, G_N_ELEMENTS (moonlight_control_mapping));

	allocate = moonlight_control_allocate;
}


/*** MoonlightTextBoxClass ***************************************************/

static NPObject *
moonlight_text_box_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightTextBoxObject (instance);
}


static const MoonNameIdMapping moonlight_text_box_mapping[] = {
	{ "select", MoonId_Select },
	{ "selectall", MoonId_SelectAll }
};


bool
MoonlightTextBoxObject::Invoke (int id, NPIdentifier name,
				  const NPVariant *args, guint32 argCount,
				  NPVariant *result)
{
	TextBox *textbox = (TextBox *) GetDependencyObject ();
	MoonError err;
	
	switch (id) {
	case MoonId_Select:
		if (!check_arg_list ("ii", argCount, args))
			THROW_JS_EXCEPTION ("select");

		if (!textbox->SelectWithError (NPVARIANT_TO_INT32 (args[0]),
					       NPVARIANT_TO_INT32 (args[1]),
					       &err)) {
			THROW_JS_EXCEPTION (err.message);
		}

		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_SelectAll:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("selectAll");

		textbox->SelectAll ();

		VOID_TO_NPVARIANT (*result);

		return true;
	default:
		return MoonlightControlObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightTextBoxType::MoonlightTextBoxType ()
{
	AddMapping (moonlight_text_box_mapping, G_N_ELEMENTS (moonlight_text_box_mapping));

	allocate = moonlight_text_box_allocate;
}

/*** MoonlightPasswordBoxClass ***************************************************/

static NPObject *
moonlight_password_box_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightPasswordBoxObject (instance);
}


static const MoonNameIdMapping moonlight_password_box_mapping[] = {
	{ "select", MoonId_Select },
	{ "selectall", MoonId_SelectAll }
};


bool
MoonlightPasswordBoxObject::Invoke (int id, NPIdentifier name,
				  const NPVariant *args, guint32 argCount,
				  NPVariant *result)
{
	PasswordBox *passwordbox = (PasswordBox *) GetDependencyObject ();
	MoonError err;
	
	switch (id) {
	case MoonId_Select:
		if (!check_arg_list ("ii", argCount, args))
			THROW_JS_EXCEPTION ("select");

		if (!passwordbox->SelectWithError (NPVARIANT_TO_INT32 (args[0]),
						   NPVARIANT_TO_INT32 (args[1]),
						   &err)) {
			THROW_JS_EXCEPTION (err.message);
		}

		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_SelectAll:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("selectAll");

		passwordbox->SelectAll ();

		VOID_TO_NPVARIANT (*result);

		return true;
	default:
		return MoonlightControlObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightPasswordBoxType::MoonlightPasswordBoxType ()
{
	AddMapping (moonlight_password_box_mapping, G_N_ELEMENTS (moonlight_password_box_mapping));

	allocate = moonlight_password_box_allocate;
}


/*** MoonlightTextBlockClass ***************************************************/

static NPObject *
moonlight_text_block_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightTextBlockObject (instance);
}


static const MoonNameIdMapping moonlight_text_block_mapping[] = {
	{ "setfontsource", MoonId_SetFontSource }
};


bool
MoonlightTextBlockObject::Invoke (int id, NPIdentifier name,
				  const NPVariant *args, guint32 argCount,
				  NPVariant *result)
{
	TextBlock *tb = (TextBlock *) GetDependencyObject ();
	DependencyObject *downloader = NULL;
	
	switch (id) {
	case MoonId_SetFontSource:
		if (!check_arg_list ("(no)", argCount, args) && (!NPVARIANT_IS_NULL(args[0]) || !npvariant_is_downloader (args[0])))
			THROW_JS_EXCEPTION ("setFontSource");
		
		if (NPVARIANT_IS_OBJECT (args[0]))
			downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();
		
		tb->SetFontSource ((Downloader *) downloader);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	default:
		return MoonlightUIElementObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightTextBlockType::MoonlightTextBlockType ()
{
	AddMapping (moonlight_text_block_mapping, G_N_ELEMENTS (moonlight_text_block_mapping));

	allocate = moonlight_text_block_allocate;
}


/*** MoonlightStylusInfoClass ***************************************************/

static NPObject *
moonlight_stylus_info_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightStylusInfoObject (instance);
}

static const MoonNameIdMapping
moonlight_stylus_info_mapping [] = {
	{ "devicetype", MoonId_DeviceType },
	{ "isinverted", MoonId_IsInverted },
};

bool
MoonlightStylusInfoObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	StylusInfo *info = (StylusInfo *) GetDependencyObject ();

	switch (id) {
	case MoonId_DeviceType: {
		switch (info->GetDeviceType ()) {
		case TabletDeviceTypeMouse:
			string_to_npvariant ("Mouse", result);
			break;
		case TabletDeviceTypeStylus:
			string_to_npvariant ("Stylus", result);
			break;
		case TabletDeviceTypeTouch:
			string_to_npvariant ("Touch", result);
			break;
		default:
			THROW_JS_EXCEPTION ("deviceType");
		}
		return true;
	}
	case MoonId_IsInverted: {
		BOOLEAN_TO_NPVARIANT (info->GetIsInverted (), *result);
		return true;
	}

	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

MoonlightStylusInfoType::MoonlightStylusInfoType ()
{
	AddMapping (moonlight_stylus_info_mapping, G_N_ELEMENTS (moonlight_stylus_info_mapping));

	allocate = moonlight_stylus_info_allocate;
}


/*** MoonlightStylusPointCollectionClass ****************************************/

static NPObject *
moonlight_stylus_point_collection_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightStylusPointCollectionObject (instance);
}

static const MoonNameIdMapping
moonlight_stylus_point_collection_mapping [] = {
	{ "addstyluspoints", MoonId_AddStylusPoints },
};

bool
MoonlightStylusPointCollectionObject::Invoke (int id, NPIdentifier name, const NPVariant *args, guint32 argCount, NPVariant *result)
{
	StylusPointCollection *col = (StylusPointCollection *) GetDependencyObject ();
	
	switch (id) {
	case MoonId_AddStylusPoints: {
		if (!col || !check_arg_list ("o", argCount, args))
			return false;
		
		MoonlightStylusPointCollectionObject *spco = (MoonlightStylusPointCollectionObject*) NPVARIANT_TO_OBJECT(args[0]);
		double ret = col->AddStylusPoints ((StylusPointCollection*) spco->GetDependencyObject ());
		DOUBLE_TO_NPVARIANT (ret, *result);
		return true;
	}
	default:
		return MoonlightCollectionObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightStylusPointCollectionType::MoonlightStylusPointCollectionType ()
{
	AddMapping (moonlight_stylus_point_collection_mapping, G_N_ELEMENTS (moonlight_stylus_point_collection_mapping));

	allocate = moonlight_stylus_point_collection_allocate;
}


/*** MoonlightStrokeCollectionClass ****************************************/

static NPObject *
moonlight_stroke_collection_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightStrokeCollectionObject (instance);
}

static const MoonNameIdMapping
moonlight_stroke_collection_mapping [] = {
	{ "getbounds", MoonId_GetBounds },
	{ "hittest", MoonId_HitTest }
};

bool
MoonlightStrokeCollectionObject::Invoke (int id, NPIdentifier name,
	const NPVariant *args, guint32 argCount, NPVariant *result)
{
	StrokeCollection *col = (StrokeCollection *) GetDependencyObject ();

	switch (id) {
	case MoonId_GetBounds: {
		Rect r = col->GetBounds ();
		Value v (r);
		value_to_variant (this, &v, result);
		return true;
	}

	case MoonId_HitTest: {
		if (!check_arg_list ("o", argCount, args) ||
		    !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("hitTest");
		
		DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
		if (!dob->Is (Type::STYLUSPOINT_COLLECTION))
			THROW_JS_EXCEPTION ("hitTest");

		StrokeCollection *hit_col = col->HitTest ((StylusPointCollection*)dob);

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (GetPlugin (), hit_col), *result);
		hit_col->unref ();
		return true;
	}

	default:
		return MoonlightCollectionObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightStrokeCollectionType::MoonlightStrokeCollectionType ()
{
	AddMapping (moonlight_stroke_collection_mapping, G_N_ELEMENTS (moonlight_stroke_collection_mapping));

	allocate = moonlight_stroke_collection_allocate;
}


/*** MoonlightStrokeClass ****************************************/

static NPObject *
moonlight_stroke_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightStrokeObject (instance);
}

static const MoonNameIdMapping
moonlight_stroke_mapping [] = {
	{ "getbounds", MoonId_GetBounds },
	{ "hittest", MoonId_HitTest }
};

bool
MoonlightStrokeObject::Invoke (int id, NPIdentifier name,
	const NPVariant *args, guint32 argCount, NPVariant *result)
{
	Stroke *stroke = (Stroke *) GetDependencyObject ();
	
	switch (id) {
	case MoonId_GetBounds: {
		Rect r = stroke->GetBounds ();
		Value v (r);
		value_to_variant (this, &v, result);
		return true;
	}

	case MoonId_HitTest: {
		if (!check_arg_list ("o", argCount, args) ||
		    !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("hitTest");
		
		DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
		if (!dob->Is (Type::STYLUSPOINT_COLLECTION))
			THROW_JS_EXCEPTION ("hitTest");

		BOOLEAN_TO_NPVARIANT (stroke->HitTest ((StylusPointCollection*)dob), *result);
		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightStrokeType::MoonlightStrokeType ()
{
	AddMapping (moonlight_stroke_mapping, G_N_ELEMENTS (moonlight_stroke_mapping));

	allocate = moonlight_stroke_allocate;
}


/*** MoonlightDownloaderClass ***************************************************/

static NPObject *
moonlight_downloader_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightDownloaderObject (instance);
}


static const MoonNameIdMapping
moonlight_downloader_mapping [] = {
	{ "abort", MoonId_Abort },
	{ "completed", MoonId_Completed },
	{ "downloadprogresschanged", MoonId_DownloadProgressChanged },
	{ "getresponsetext", MoonId_GetResponseText },
	{ "open", MoonId_Open },
	{ "responsetext", MoonId_ResponseText },
	{ "send", MoonId_Send }
};

bool
MoonlightDownloaderObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	Downloader *downloader = (Downloader *) GetDependencyObject ();
	gint64 size;
	char *text;
	
	switch (id) {
	case MoonId_ResponseText:
		if ((text = downloader->GetResponseText (NULL, &size))) {
			char *s = (char *) MOON_NPN_MemAlloc (size + 1);
			memcpy (s, text, size + 1);
			g_free (text);
			
			STRINGN_TO_NPVARIANT (s, (guint32) size, *result);
		} else {
			string_to_npvariant ("", result);
		}
		
		return true;
	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightDownloaderObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, guint32 argCount,
				   NPVariant *result)
{
	Downloader *downloader = (Downloader *) GetDependencyObject ();
	char *part, *verb, *uri, *text;
	gint64 size;
	
	switch (id) {
	case MoonId_Abort:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("abort");

		downloader->Abort ();
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_Open:
		if (!check_arg_list ("s(ns)", argCount, args))
			THROW_JS_EXCEPTION ("open");
		
		verb = STRDUP_FROM_VARIANT (args[0]);
		
		if (NPVARIANT_IS_STRING (args[1]))
			uri = STRDUP_FROM_VARIANT (args[1]);
		else
			uri = NULL;
		
		downloader->Open (verb, uri, DownloaderPolicy);
		g_free (verb);
		g_free (uri);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_Send:
		if (argCount != 0 || downloader->GetSurface () == NULL)
			THROW_JS_EXCEPTION ("send");
		
		downloader->Send ();
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_GetResponseText:
		if (!check_arg_list ("s", argCount, args))
			THROW_JS_EXCEPTION ("getResponseText");
		
		part = STRDUP_FROM_VARIANT (args[0]);
		if ((text = downloader->GetResponseText (part, &size))) {
			char *s = (char *) MOON_NPN_MemAlloc (size + 1);
			memcpy (s, text, size + 1);
			g_free (text);
			
			STRINGN_TO_NPVARIANT (s, (guint32) size, *result);
		} else {
			string_to_npvariant ("", result);
		}
		g_free (part);

		return true;
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightDownloaderType::MoonlightDownloaderType ()
{
	AddMapping (moonlight_downloader_mapping, G_N_ELEMENTS (moonlight_downloader_mapping));

	allocate = moonlight_downloader_allocate;
}

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


static NPObject *
moonlight_scriptable_object_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightScriptableObjectObject (instance);
}

MoonlightScriptableObjectObject::~MoonlightScriptableObjectObject ()
{
	if (managed_scriptable) {
		// FIXME: unref the scriptable object however we need to.
		managed_scriptable = NULL;
	}
	
	// FIXME: free the properties, events, and methods hashes.
	g_hash_table_destroy (properties);
	g_hash_table_destroy (methods);
	g_hash_table_destroy (events);
}

bool
MoonlightScriptableObjectObject::HasProperty (NPIdentifier name)
{
	bool result;

#if ds(!)0
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	printf ("is indexer: %s\n", MOON_NPN_IdentifierIsString (name) ? "false" : "true");
	if (MOON_NPN_IdentifierIsString (name)) {
		printf ("scriptable has property %x = %s\n", name, strname);
	} else {
		printf ("scriptable has property this[%d]\n", MOON_NPN_IntFromIdentifier (name));
	}
#endif

	if (!MOON_NPN_IdentifierIsString (name))
		name = MOON_NPN_GetStringIdentifier ("item");

	result = (g_hash_table_lookup (properties, name) != NULL
		|| g_hash_table_lookup (events, name)) || MoonlightObject::HasProperty (name);

#if ds(!)0
	printf ("scriptable has property %x = %s: result: %i\n", name, strname, result);
	MOON_NPN_MemFree (strname);
#endif


	return result;
}

bool
MoonlightScriptableObjectObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	bool res;
	
	Value **vargs = NULL;
	guint32 argCount = 0;
	if (!MOON_NPN_IdentifierIsString (name)) {
		argCount = 1;
		vargs = new Value*[argCount];
		vargs[0] = new Value (MOON_NPN_IntFromIdentifier (name));
		name = MOON_NPN_GetStringIdentifier ("item");
#if ds(!)0
		printf ("index: %d\n", vargs[0]->AsInt32 ());
#endif
	}

	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
#if ds(!)0
	printf ("getting scriptable object property %x = %s\n", name, strname);	
#endif

	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (properties, name);
	if (!prop) {
		res = MoonlightObject::GetProperty (id, name, result);
	} else {
		Value v;

		getprop (managed_scriptable, strname, vargs, argCount, &v);

		value_to_variant (this, &v, result);
		res = true;		
	}

#if ds(!)0
	printf ("getting scriptable object property %x = %s result: %i\n", name, strname, result);
#endif

	if (argCount > 0) {
		for (int i = 0; i < argCount; i++)
			delete vargs[i];
		delete [] vargs;
	}

	MOON_NPN_MemFree (strname);

	return res;
}

bool
MoonlightScriptableObjectObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	ScriptableProperty *prop;
	ScriptableEvent *event;
	Value *v;

	Value **vargs = NULL;
	guint32 argCount = 0;
	if (!MOON_NPN_IdentifierIsString (name)) {
		argCount = 1;
		vargs = new Value*[argCount];
		vargs[0] = new Value (MOON_NPN_IntFromIdentifier (name));
		name = MOON_NPN_GetStringIdentifier ("item");
#if ds(!)0
		printf ("index: %d\n", vargs[0]->AsInt32 ());
#endif
	}

	// first we try the property hash
	if ((prop = (ScriptableProperty *) g_hash_table_lookup (properties, name))) {

		NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);

		ds(printf ("setting scriptable object property %s\n", strname));

		variant_to_value (value, &v);
		setprop (managed_scriptable, strname, vargs, argCount, v);
		delete v;

		if (argCount > 0) {
			for (int i = 0; i < argCount; i++)
				delete vargs[i];
			delete [] vargs;
		}

		MOON_NPN_MemFree (strname);
		
		return true;
	}
	
	// if that fails, look for the event of that name
	if ((event = (ScriptableEvent *) g_hash_table_lookup (events, name))) {
#if ds(!)0
		NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
		printf ("adding scriptable object event %s\n", strname);
		MOON_NPN_MemFree (strname);
#endif
		
		if (NPVARIANT_IS_OBJECT (*value)) {
			NPObject *cb_obj = NPVARIANT_TO_OBJECT (*value);

			MOON_NPN_RetainObject (cb_obj);

			addevent (managed_scriptable, event->event_handle, this, cb_obj);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("scriptableobject.register_event (non-object)");
		}
		
		return true;
	}

	return MoonlightObject::SetProperty (id, name, value);
}

#if ds(!)0
void 
dump_ptr_npid_hash (gpointer key, gpointer value, gpointer user_data)
{
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier ((NPIdentifier) key );
	printf (" %i (%s) => %p\n", key, strname, value);
	MOON_NPN_MemFree (strname);
}
#endif

bool
MoonlightScriptableObjectObject::HasMethod (NPIdentifier name)
{
	bool result;
	
#if ds(!)0
	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
	printf ("MoonlightScriptableObjectObject::HasMethod (%x = %s) this: %p hash table: %p\n", name, strname, this, methods);
#endif

	result = g_hash_table_lookup (methods, name) != NULL;


#if ds(!)0
	if (!result) {
		printf ("MoonlightScriptableObjectObject::HasMethod (%x = %s) failed, this: %p\n", name, strname, this);
		g_hash_table_foreach (methods, dump_ptr_npid_hash, NULL);		
	} else {
		printf ("MoonlightScriptableObjectObject::HasMethod (%x = %s) this: %p result: %i\n", name, strname, this, result);
	}
	MOON_NPN_MemFree (strname);
#endif
	
	return result;
}

bool
MoonlightScriptableObjectObject::Invoke (int id, NPIdentifier name,
					 const NPVariant *args, guint32 argCount,
					 NPVariant *result)
{
	PluginInstance *plugin = GetPlugin ();
	if (plugin->IsCrossDomainApplication ()) {
		if (Deployment::GetCurrent ()->GetExternalCallersFromCrossDomain () == CrossDomainAccessNoAccess)
			THROW_JS_EXCEPTION ("XDomain Restriction");
	}

	ScriptableMethod *method = (ScriptableMethod*)g_hash_table_lookup (methods, name);
	Value rv, **vargs = NULL;
	guint32 i;
	
	if (!method)
		return MoonlightObject::Invoke (id, name, args, argCount, result);

	NPUTF8 *strname = MOON_NPN_UTF8FromIdentifier (name);
#if ds(!)0
	printf ("invoking scriptable object method %s\n", strname);
#endif
	
	if (argCount > 0) {
		vargs = new Value*[argCount];
		for (i = 0; i < argCount; i++)
			variant_to_value (&args[i], &vargs[i]);
	}
	
	invoke (managed_scriptable, method->method_handle, strname, vargs, argCount, &rv);
	
	if (argCount > 0) {
		for (i = 0; i < argCount; i++)
			delete vargs[i];
		
		delete [] vargs;
	}

	ds(printf ("result of invoking %s was %p\n", strname, rv.ToString()));
	
	/* Note: this 1 is "void" */
	if (method->method_return_type != 0)
		value_to_variant (this, &rv, result);
	else
		VOID_TO_NPVARIANT (*result);
	
	MOON_NPN_MemFree (strname);
	return true;
}


MoonlightScriptableObjectType::MoonlightScriptableObjectType ()
{
	allocate = moonlight_scriptable_object_allocate;
}

MoonlightScriptableObjectType *MoonlightScriptableObjectClass;

MoonlightScriptableObjectObject *
moonlight_scriptable_object_wrapper_create_root (PluginInstance *plugin, gpointer scriptable,
					    InvokeDelegate invoke_func,
					    SetPropertyDelegate setprop_func,
					    GetPropertyDelegate getprop_func,
					    EventHandlerDelegate addevent_func,
					    EventHandlerDelegate removeevent_func)

{
	MoonlightScriptControlObject *root_object = plugin->GetRootObject ();
	return moonlight_scriptable_object_wrapper_create (root_object, scriptable,
							   invoke_func,
							   setprop_func,
							   getprop_func,
							   addevent_func,
							   removeevent_func);
}

MoonlightScriptableObjectObject *
moonlight_scriptable_object_wrapper_create (NPObject *parent, gpointer scriptable,
					    InvokeDelegate invoke_func,
					    SetPropertyDelegate setprop_func,
					    GetPropertyDelegate getprop_func,
					    EventHandlerDelegate addevent_func,
					    EventHandlerDelegate removeevent_func)
{
	MoonlightScriptableObjectObject *obj = (MoonlightScriptableObjectObject *)
		MOON_NPN_CreateObject (((MoonlightObject *) parent)->GetInstance (),
				  MoonlightScriptableObjectClass);

	obj->managed_scriptable = scriptable;
	obj->invoke = invoke_func;
	obj->setprop = setprop_func;
	obj->getprop = getprop_func;
	obj->addevent = addevent_func;
	obj->removeevent = removeevent_func;
	
	ds(printf ("creating scriptable object wrapper => %p\n", obj));
	
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
	ds(printf ("adding property named %s to scriptable object %p\n", property_name, obj));
	
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
	ds(printf ("adding event named %s to scriptable object %p\n", event_name, obj));
	
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
	NPIdentifier id = NPID (method_name);

	ds(printf ("adding method named %s (npid: %i) (return type = %d) to scriptable object %p\n", method_name, id, method_return_type, obj));
	
	ScriptableMethod *method = new ScriptableMethod ();
	method->method_handle = method_handle;
	method->method_return_type = method_return_type;
	method->method_parameter_types = new int[parameter_count];
	memcpy (method->method_parameter_types, method_parameter_types, sizeof (int) * parameter_count);
	method->parameter_count = parameter_count;

	g_hash_table_insert (obj->methods, id, method);
}

void
moonlight_scriptable_object_register (PluginInstance *plugin,
				      char *name,
				      MoonlightScriptableObjectObject *obj)
{
	MoonlightContentObject *content = (MoonlightContentObject *) plugin->GetRootObject ()->content;
	
	ds(printf ("registering scriptable object '%s' => %p\n", name, obj));
	
	g_hash_table_insert (content->registered_scriptable_objects, NPID (name), obj);
	
	ds(printf (" => done\n"));
}

void
moonlight_scriptable_object_emit_event (PluginInstance *plugin,
					MoonlightScriptableObjectObject *sobj,
					MoonlightScriptableObjectObject *event_args,
					NPObject *cb_obj)
{
	NPVariant args[2];
	NPVariant result;

	OBJECT_TO_NPVARIANT (sobj, args[0]);
	OBJECT_TO_NPVARIANT (event_args, args[1]);

	if (MOON_NPN_InvokeDefault (plugin->GetInstance (), cb_obj, args, 2, &result))
		MOON_NPN_ReleaseVariantValue (&result);
}


/****************************** HtmlObject *************************/

#if 0
// Debug method to reflect over a NPObject and dump it to stdout.
static void
enumerate_html_object (NPP npp, NPObject *npobj, int recurse, int initial_recurse)
{
	NPVariant npresult;
	NPIdentifier *identifiers = NULL;
	guint32 id_count = 0;

	if (recurse == 0)
		return;
		
	char *tab = (char *) g_malloc (sizeof (char *) * (initial_recurse + 1) * 4);
	char *tab2 = (char *) g_malloc (sizeof (char *) * (initial_recurse + 1) * 4);
	
	memset (tab, 0, (initial_recurse + 1) * 4);
	memset (tab, ' ', (initial_recurse - recurse) * 4);
	
	memset (tab2, 0, (initial_recurse + 1) * 4);
	memset (tab2, ' ', (initial_recurse - recurse + 1) * 4);

	if (MOON_NPN_Enumerate (npp, npobj, &identifiers, &id_count)) {
		//printf ("%senumerate_html_object (%p, %p, %i, %i): Enumerating %i identifiers.\n", tab, npp, npobj, recurse, initial_recurse, id_count);
		for (guint32 i = 0; i < id_count; i++) {
			if (MOON_NPN_IdentifierIsString (identifiers [i])) {
				printf ("%s'%s': ", tab2, MOON_NPN_UTF8FromIdentifier (identifiers [i]));
			} else {
				printf ("%s%i: ", tab2, MOON_NPN_IntFromIdentifier (identifiers [i]));
			}

			MOON_NPN_GetProperty (npp, npobj, identifiers [i], &npresult);

			if (NPVARIANT_IS_VOID (npresult)) {
				printf ("void\n");
			} else if (NPVARIANT_IS_NULL (npresult)) {
				printf ("NULL\n");
			} else if (NPVARIANT_IS_STRING (npresult)) {
				printf ("String: %s\n", NPVARIANT_TO_STRING (npresult).utf8characters);
			} else if (NPVARIANT_IS_BOOLEAN (npresult)) {
				printf ("Boolean: %i\n", NPVARIANT_TO_BOOLEAN (npresult));
			} else if (NPVARIANT_IS_INT32 (npresult)) {
				printf ("Int32: %i\n", NPVARIANT_TO_INT32 (npresult));
			} else if (NPVARIANT_IS_DOUBLE (npresult)) {
				printf ("Double: %f\n", NPVARIANT_TO_DOUBLE (npresult));
			} else if (NPVARIANT_IS_OBJECT (npresult)) {
				printf ("Object.\n");
				
				if (recurse >= 1) {
					enumerate_html_object (npp, NPVARIANT_TO_OBJECT (npresult), recurse - 1, initial_recurse);
				}
			} else {
				printf ("Invalid value.\n");
			}
		}
	}

	g_free (tab);
	g_free (tab2);
}
#endif

bool
html_object_has_property (PluginInstance *plugin, NPObject *npobj, char *name)
{
	NPP npp = plugin->GetInstance ();
	NPObject *window = NULL;
	NPIdentifier identifier = MOON_NPN_GetStringIdentifier (name);
	if (npobj == NULL) {
		MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	return MOON_NPN_HasProperty (npp, npobj, identifier);
}

void
html_object_get_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *result)
{
	NPVariant npresult;
	NPObject *window = NULL;
	NPP npp = plugin->GetInstance ();
	NPIdentifier identifier = MOON_NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	bool ret = MOON_NPN_GetProperty (npp, npobj, identifier, &npresult);

	if (ret) {
		Value *res = NULL;
		if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
			variant_to_value (&npresult, &res);
			*result = *res;
		} else {
			*result = Value (Type::INVALID);
		}
	} else {
		*result = Value (Type::INVALID);
	}
}

void
html_object_set_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *value)
{
	NPVariant npvalue;
	NPObject *window = NULL;
	NPP npp = plugin->GetInstance ();
	NPIdentifier identifier = MOON_NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	value_to_variant (npobj, value, &npvalue);

	bool ret = MOON_NPN_SetProperty (npp, npobj, identifier, &npvalue);
	if (!ret)
		d (printf ("Error setting property %s.\n", name));
	
	MOON_NPN_ReleaseVariantValue (&npvalue);
}

bool
html_object_invoke (PluginInstance *plugin, NPObject *npobj, char *name,
		Value *args, guint32 arg_count, Value *result)
{
	NPVariant npresult;
	NPVariant *npargs = NULL;
	NPObject *window = NULL;
	NPP npp = plugin->GetInstance ();
	NPIdentifier identifier = MOON_NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	if (arg_count) {
		npargs = new NPVariant [arg_count];
		for (guint32 i = 0; i < arg_count; i++)
			value_to_variant (npobj, &args [i], &npargs [i]);
	}

	bool ret = MOON_NPN_Invoke (npp, npobj, identifier, npargs, arg_count, &npresult);

	if (arg_count) {
		for (guint32 i = 0; i < arg_count; i++)
			MOON_NPN_ReleaseVariantValue (&npargs [i]);
		delete [] npargs;
	}

	if (ret)
	{
		Value *res = NULL;
		if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
			variant_to_value (&npresult, &res);
			*result = *res;
		    } else {
			*result = Value (Type::INVALID);
		}
	} else {
		*result = Value (Type::INVALID);
	}
	return ret;
}

bool
html_object_invoke_self (PluginInstance *plugin, NPObject *npobj,
		Value *args, guint32 arg_count, Value *result)
{
	NPVariant npresult;
	NPVariant *npargs = NULL;
	NPObject *window = NULL;
	NPP npp = plugin->GetInstance ();

	if (npobj == NULL) {
		MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	if (arg_count) {
		npargs = new NPVariant [arg_count];
		for (guint32 i = 0; i < arg_count; i++)
			value_to_variant (npobj, &args [i], &npargs [i]);
	}

	bool ret = MOON_NPN_InvokeDefault (npp, npobj, npargs, arg_count, &npresult);

	if (arg_count) {
		for (guint32 i = 0; i < arg_count; i++)
			MOON_NPN_ReleaseVariantValue (&npargs [i]);
		delete [] npargs;
	}

	if (ret)
	{
		Value *res = NULL;
		if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
			variant_to_value (&npresult, &res);
			*result = *res;
		    } else {
			*result = Value (Type::INVALID);
		}
	} else {
		*result = Value (Type::INVALID);
	}
	return ret;
}

gpointer
html_object_attach_event (PluginInstance *plugin, NPObject *npobj, char *name, callback_dom_event *cb, gpointer context)
{
	return plugin->HtmlObjectAttachEvent (plugin->GetInstance(), npobj, name, cb, context);
}

void
html_object_detach_event (PluginInstance *plugin, const char *name, gpointer listener_ptr)
{
	return plugin->HtmlObjectDetachEvent (plugin->GetInstance(), name, listener_ptr);
}

struct release_data {
	PluginInstance *plugin;
	NPObject *npobj;
};

static gboolean
html_object_release_callback (gpointer user_data)
{
	release_data *d = (release_data *) user_data;
	html_object_release (d->plugin, d->npobj);
	d->plugin->unref ();
	g_free (d);
	return false;
}

void
html_object_release (PluginInstance *plugin, NPObject *npobj)
{
	if (npobj == NULL)
		return;
	
	if (!Surface::InMainThread ()) {	
		release_data *d = (release_data *) g_malloc (sizeof (release_data));
		plugin->ref ();
		d->plugin = plugin;
		d->npobj = npobj;
		g_timeout_add_full (1, G_PRIORITY_DEFAULT, html_object_release_callback, d, NULL);
		return;
	}
	
	if (plugin->HasShutdown ()) {
		// printf ("html_object_release (%p, %p): plugin has shut down, object has most probably been deleted already.\n", plugin, npobj);
		return;
	}
	
	MOON_NPN_ReleaseObject (npobj);
}

void
html_object_retain (PluginInstance *plugin, NPObject *npobj)
{
	if (npobj == NULL)
		return;
	
	/*
	 * trying to use a NPObject when the plugin has shut down is a very bad idea
	 * since the NPObject has most probably been deleted already.
	 */
	g_return_if_fail (!plugin->HasShutdown ());

	/*
	 * we can't marshal this to the main thread (if we're not already executing
	 * in it), since the object might end up deleted by the time the main thread
	 * retains it
	 */	

	MOON_NPN_RetainObject (npobj);
}

void
browser_do_alert (PluginInstance *plugin, char *msg)
{
	NPVariant npresult, npargs;
	NPObject *window = NULL;
	NPP npp = plugin->GetInstance ();
	NPIdentifier identifier = MOON_NPN_GetStringIdentifier ("alert");

	MOON_NPN_GetValue (npp, NPNVWindowNPObject, &window);
	string_to_npvariant (msg, &npargs);

	MOON_NPN_Invoke (npp, window, identifier, &npargs, 1, &npresult);
}


void
plugin_init_classes (void)
{
	/*
	 * All classes that derive from MoonlightDependencyObject should be stored in the dependency_object_classes
	 * array, so that we can verify arguments passed from JS code are valid dependency objects, and not random
	 * JS objects.  ie element.children.add (new Array ())
	 */

	dependency_object_classes [COLLECTION_CLASS] = new MoonlightCollectionType ();
	dependency_object_classes [DEPENDENCY_OBJECT_CLASS] = new MoonlightDependencyObjectType ();
	dependency_object_classes [DOWNLOADER_CLASS] = new MoonlightDownloaderType ();
	dependency_object_classes [IMAGE_BRUSH_CLASS] = new MoonlightImageBrushType ();
	dependency_object_classes [IMAGE_CLASS] = new MoonlightImageType ();
	dependency_object_classes [MEDIA_ELEMENT_CLASS] = new MoonlightMediaElementType ();
	dependency_object_classes [STORYBOARD_CLASS] = new MoonlightStoryboardType ();
	dependency_object_classes [STYLUS_INFO_CLASS] = new MoonlightStylusInfoType ();
	dependency_object_classes [STYLUS_POINT_COLLECTION_CLASS] = new MoonlightStylusPointCollectionType ();
	dependency_object_classes [STROKE_COLLECTION_CLASS] = new MoonlightStrokeCollectionType ();
	dependency_object_classes [STROKE_CLASS] = new MoonlightStrokeType ();
	dependency_object_classes [TEXT_BLOCK_CLASS] = new MoonlightTextBlockType ();
	dependency_object_classes [UI_ELEMENT_CLASS] = new MoonlightUIElementType ();
	dependency_object_classes [CONTROL_CLASS] = new MoonlightControlType ();
	dependency_object_classes [TEXT_BOX_CLASS] = new MoonlightTextBoxType ();
	dependency_object_classes [PASSWORD_BOX_CLASS] = new MoonlightPasswordBoxType ();
	dependency_object_classes [MULTI_SCALE_IMAGE_CLASS] = new MoonlightMultiScaleImageType ();
	/* Event Arg Types */
	dependency_object_classes [EVENT_ARGS_CLASS] = new MoonlightEventArgsType ();
	dependency_object_classes [ROUTED_EVENT_ARGS_CLASS] = new MoonlightRoutedEventArgsType ();
	dependency_object_classes [ERROR_EVENT_ARGS_CLASS] = new MoonlightErrorEventArgsType ();
	dependency_object_classes [KEY_EVENT_ARGS_CLASS] = new MoonlightKeyEventArgsType ();
	dependency_object_classes [TIMELINE_MARKER_ROUTED_EVENT_ARGS_CLASS] = new MoonlightTimelineMarkerRoutedEventArgsType ();
	dependency_object_classes [MOUSE_EVENT_ARGS_CLASS] = new MoonlightMouseEventArgsType ();
	dependency_object_classes [DOWNLOAD_PROGRESS_EVENT_ARGS_CLASS] = new MoonlightDownloadProgressEventArgsType ();
	
	MoonlightContentClass = new MoonlightContentType ();
	MoonlightDurationClass = new MoonlightDurationType ();
	MoonlightEventObjectClass = new MoonlightEventObjectType ();
	MoonlightObjectClass = new MoonlightObjectType ();
	MoonlightPointClass = new MoonlightPointType ();
	MoonlightRectClass = new MoonlightRectType ();
	MoonlightScriptableObjectClass = new MoonlightScriptableObjectType ();
	MoonlightScriptControlClass = new MoonlightScriptControlType ();
	MoonlightSettingsClass = new MoonlightSettingsType ();
	MoonlightTimeSpanClass = new MoonlightTimeSpanType ();
	MoonlightKeyTimeClass = new MoonlightKeyTimeType ();
	MoonlightThicknessClass = new MoonlightThicknessType ();
	MoonlightCornerRadiusClass = new MoonlightCornerRadiusType ();
	MoonlightGridLengthClass = new MoonlightGridLengthType ();
}

void
plugin_destroy_classes (void)
{
	for (int i = 0; i < DEPENDENCY_OBJECT_CLASS_NAMES_LAST; i++) {
		delete dependency_object_classes [i];
		dependency_object_classes [i] = NULL;
	}

	delete MoonlightContentClass; MoonlightContentClass = NULL;
	delete MoonlightEventObjectClass; MoonlightEventObjectClass = NULL;
	delete MoonlightErrorEventArgsClass; MoonlightErrorEventArgsClass = NULL;
	delete MoonlightMouseEventArgsClass; MoonlightMouseEventArgsClass = NULL;
	delete MoonlightDownloadProgressEventArgsClass; MoonlightDownloadProgressEventArgsClass = NULL;
	delete MoonlightKeyEventArgsClass; MoonlightKeyEventArgsClass = NULL;
	delete MoonlightObjectClass; MoonlightObjectClass = NULL;
	delete MoonlightScriptableObjectClass; MoonlightScriptableObjectClass = NULL;
	delete MoonlightScriptControlClass; MoonlightScriptControlClass = NULL;
	delete MoonlightSettingsClass; MoonlightSettingsClass = NULL;
	delete MoonlightRectClass; MoonlightRectClass = NULL;
	delete MoonlightPointClass; MoonlightPointClass = NULL;
	delete MoonlightDurationClass; MoonlightDurationClass = NULL;
	delete MoonlightTimeSpanClass; MoonlightTimeSpanClass = NULL;
	delete MoonlightKeyTimeClass; MoonlightKeyTimeClass = NULL;
	delete MoonlightThicknessClass; MoonlightThicknessClass = NULL;
	delete MoonlightCornerRadiusClass; MoonlightCornerRadiusClass = NULL;
	delete MoonlightGridLengthClass; MoonlightGridLengthClass = NULL;
	delete MoonlightTimelineMarkerRoutedEventArgsClass; MoonlightTimelineMarkerRoutedEventArgsClass = NULL;
}
