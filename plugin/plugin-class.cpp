/*
 * plugin-class.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *   Michael Dominic K. <mdk@mdk.am>
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

#include <nsCOMPtr.h>
#include <nsIDOMElement.h>
#include <nsIDOMRange.h>
#include <nsIDOMDocumentRange.h>
#include <nsIDOMDocument.h>
#include <nsIDOMWindow.h>
#include <nsStringAPI.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMNodeList.h>
#include <nsIDOMHTMLElement.h>

// Events
#include <nsIDOMEvent.h>
#include <nsIDOMMouseEvent.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMEventListener.h>

#ifdef DEBUG
#define DEBUG_WARN_NOTIMPLEMENTED(x) printf ("not implemented: (%s)" G_STRLOC, x)
#define d(x) x
#else
#define DEBUG_WARN_NOTIMPLEMENTED(x)
#define d(x)
#endif

// debug scriptable object
#define ds(x)

// debug javascript
#define DEBUG_JAVASCRIPT 1

// warnings
#define w(x) x

#define IS_METHOD(id)         (((id) & 0x8000) != 0)
#define IS_PROPERTY(id)       (((id) & 0x4000) != 0)

enum PluginPropertyId {
	NoMapping = 0,

	// property names
	MoonId_ErrorCode = 0x4000,
	MoonId_ErrorType,
	MoonId_ErrorMessage,
	MoonId_LineNumber,
	MoonId_CharPosition,
	MoonId_XamlFile,
	MoonId_MethodName,
	MoonId_X,
	MoonId_Y,
	MoonId_Width,
	MoonId_Height,
	MoonId_Seconds,
	MoonId_Name,
	MoonId_Shift,
	MoonId_Ctrl,
	MoonId_Marker,
	MoonId_Key,
	MoonId_PlatformKeyCode,
	MoonId_Settings,
	MoonId_Content,
	MoonId_InitParams,
	MoonId_Id,
	MoonId_IsLoaded,
	MoonId_Source,
	MoonId_Background,
	MoonId_EnableFramerateCounter,
	MoonId_EnableRedrawRegions,
	MoonId_EnableHtmlAccess,
	MoonId_MaxFrameRate,
	MoonId_Version,
	MoonId_Windowless,
	MoonId_ActualHeight,
	MoonId_ActualWidth,
	MoonId_FullScreen,
	MoonId_Root,
	MoonId_Count,
	MoonId_ResponseText,
	MoonId_DeviceType,
	MoonId_IsInverted,

	// event names (handled by the property setters)
	MoonId_BufferingProgressChanged,
	MoonId_Completed,
	MoonId_CurrentStateChanged,
	MoonId_DownloadProgressChanged,
	MoonId_GotFocus,
	MoonId_ImageFailed,
	MoonId_KeyDown,
	MoonId_KeyUp,
	MoonId_LostFocus,
	MoonId_Loaded,
	MoonId_MarkerReached,
	MoonId_MediaEnded,
	MoonId_MediaFailed,
	MoonId_MediaOpened,
	MoonId_MouseEnter,
	MoonId_MouseLeave,
	MoonId_MouseMove,
	MoonId_MouseLeftButtonDown,
	MoonId_MouseLeftButtonUp,
	MoonId_OnResize,
	MoonId_OnFullScreenChange,
	MoonId_OnError,

	// method names
	MoonId_GetPosition = 0x8000,
	MoonId_CreateObject,
	MoonId_IsVersionSupported,
	MoonId_FindName,
	MoonId_CreateFromXaml,
	MoonId_CreateFromXamlDownloader,
	MoonId_Equals,
	MoonId_GetHost,
	MoonId_GetParent,
	MoonId_GetStylusInfo,
	MoonId_GetStylusPoints,
	MoonId_CaptureMouse,
	MoonId_ReleaseMouseCapture,
	MoonId_AddEventListener,
	MoonId_RemoveEventListener,
	MoonId_SetValue,
	MoonId_GetValue,
	MoonId_ToString,     
#if DEBUG_JAVASCRIPT
	MoonId_Printf,
#endif
	MoonId_Add,
	MoonId_Remove,
	MoonId_RemoveAt,
	MoonId_Insert,
	MoonId_Clear,
	MoonId_GetItem,
	MoonId_GetItemByName,
	MoonId_Begin,
	MoonId_Pause,
	MoonId_Resume,
	MoonId_Seek,
	MoonId_Stop,
	MoonId_Play,
	MoonId_SetSource,
	MoonId_SetFontSource,
	MoonId_Abort,
	MoonId_Open,
	MoonId_GetResponseText,
	MoonId_Send,
	MoonId_AddStylusPoints,
	MoonId_GetBounds,
	MoonId_HitTest,
};

static char*
npidentifier_to_downstr (NPIdentifier id)
{
	if (!NPN_IdentifierIsString (id))
		return NULL;

	NPUTF8 *strname = NPN_UTF8FromIdentifier (id);
	char *p = strname;
	while (*p) {
		*p = g_ascii_tolower (*p);
		p++;
	}

	return strname;
}


#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)
#define STRLEN_FROM_VARIANT(v) ((size_t) NPVARIANT_TO_STRING (v).utf8length)

#define DEPENDENCY_OBJECT_FROM_VARIANT(obj) (((MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (obj))->GetDependencyObject ())

#define THROW_JS_EXCEPTION(meth)	\
	do {	\
		char *message = g_strdup_printf ("Error calling method: %s", meth);	\
		NPN_SetException (this, message);	\
		g_free (message);	\
		return true; \
	} while (0);	\

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


	NPN_MemFree (strname);
	return result ? result->id : NoMapping;
}

static const char *
map_moon_id_to_event_name (int moon_id)
{
	const char *name = NULL;

	switch (moon_id) {
	case MoonId_BufferingProgressChanged: name = "BufferingProgressChanged"; break;
	case MoonId_CurrentStateChanged:  name = "CurrentStateChanged"; break;
	case MoonId_DownloadProgressChanged: name = "DownloadProgressCanged"; break;
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
	}

	return name;
}


static void
string_to_npvariant (const char *value, NPVariant *result)
{
	char *retval;

	if (value)
		retval = PL_strdup (value);
	else
		retval = PL_strdup ("");

	STRINGZ_TO_NPVARIANT (retval, *result);
}

static void
value_to_variant (NPObject *npobj, Value *v, NPVariant *result, DependencyObject *parent_obj = NULL, DependencyProperty *parent_property = NULL)
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
		MoonlightPoint *point = (MoonlightPoint *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightPointClass);
		point->point = *v->AsPoint ();
		OBJECT_TO_NPVARIANT (point, *result);
		break;
	}
	case Type::RECT: {
		MoonlightRect *rect = (MoonlightRect *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightRectClass);
		rect->rect = *v->AsRect ();
		OBJECT_TO_NPVARIANT (rect, *result);
		break;
	}
	case Type::DURATION: {
		MoonlightDuration *duration = (MoonlightDuration *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightDurationClass);
		duration->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (duration, *result);
		break;
	}
	case Type::TIMESPAN: {
		MoonlightTimeSpan *timespan = (MoonlightTimeSpan *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightTimeSpanClass);
		timespan->SetParentInfo (parent_obj, parent_property);
		OBJECT_TO_NPVARIANT (timespan, *result);
		break;
	}
	case Type::COLOR: {
		Color *c = v->AsColor ();
		gint32 color = ((((gint32)(c->a * 255.0)) << 24) | (((gint32)(c->r * 255.0)) << 16) | 
			(((gint32)(c->g * 255.0)) << 8) | ((gint32)(c->b * 255.0)));
		INT32_TO_NPVARIANT (color, *result);
		break;
	}
	default:
		/* more builtins.. */
		if (v->Is (Type::DEPENDENCY_OBJECT)) {
			MoonlightEventObjectObject *depobj =
				EventObjectCreateWrapper (((MoonlightObject *) npobj)->instance, v->AsDependencyObject ());
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
		*result = new Value ((int32_t) NPVARIANT_TO_INT32 (*v));
		break;
	case NPVariantType_Double:
		*result = new Value (NPVARIANT_TO_DOUBLE (*v));
		break;
	case NPVariantType_String:
		*result = new Value (STR_FROM_VARIANT (*v));
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
		// This should never happen, we should do type checking of the
		// arguments before this point and refuse arguments we don't understand.
		d(printf ("Got invalid value from javascript.\n"));
		*result = new Value ();
		break;
	}
}

enum DependencyObjectClassNames {
	COLLECTION_CLASS,
	CONTROL_CLASS,
	DEPENDENCY_OBJECT_CLASS,
	DOWNLOADER_CLASS,
	IMAGE_BRUSH_CLASS,
	IMAGE_CLASS,
	MEDIA_ELEMENT_CLASS,
	STORYBOARD_CLASS,
	STYLUS_INFO_CLASS,
	STYLUS_POINT_COLLECTION_CLASS,
	STROKE_COLLECTION_CLASS,
	STROKE_CLASS,
	TEXT_BLOCK_CLASS,
	EVENT_ARGS_CLASS,
	ERROR_EVENT_ARGS_CLASS,
	KEYBOARD_EVENT_ARGS_CLASS,
	MARKER_REACHED_EVENT_ARGS_CLASS,
	MOUSE_EVENT_ARGS_CLASS,

	DEPENDENCY_OBJECT_CLASS_NAMES_LAST
};

NPClass *dependency_object_classes[DEPENDENCY_OBJECT_CLASS_NAMES_LAST];

static bool
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

EventListenerProxy::EventListenerProxy (NPP instance, const char *event_name, const char *cb_name)
{
	this->instance = instance;
	this->event_name = g_strdup (event_name);
	this->event_id = -1;
	this->target_object = NULL;

	this->is_func = false;
	if (!strncmp (cb_name, "javascript:", strlen ("javascript:")))
		cb_name += strlen ("javascript:");
	this->callback = g_strdup (cb_name);
}

const char *
EventListenerProxy::GetCallbackAsString ()
{
	if (is_func)
		return "";
	
	return (const char *)callback;
}

EventListenerProxy::EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb)
{
	this->instance = instance;
	this->event_name = g_strdup (event_name);
	this->event_id = -1;
	this->target_object = NULL;

	if (NPVARIANT_IS_OBJECT (*cb)) {
		this->is_func = true;
		this->callback = NPVARIANT_TO_OBJECT (*cb);
		NPN_RetainObject ((NPObject *) this->callback);
	} else {
		this->is_func = false;
		this->callback = g_strdup (STR_FROM_VARIANT (*cb));
	}
}

EventListenerProxy::~EventListenerProxy ()
{
	if (target_object) {
		if (event_id != -1)
			RemoveHandler ();
		
		target_object->RemoveHandler ("destroyed", dtoken);
	}
	
	if (is_func) {
		if (callback != NULL)
			NPN_ReleaseObject ((NPObject *) callback);
	}
	else {
		g_free (callback);
	}
	
	g_free (event_name);
}
	
int
EventListenerProxy::AddHandler (EventObject *obj)
{
	target_object = obj;
	
	dtoken = obj->AddHandler (EventObject::DestroyedEvent, on_target_object_destroyed, this);
	
	event_id = obj->GetType()->LookupEvent (event_name);

	if (event_id == -1) {
		d(printf ("object of type `%s' does not provide an event named `%s'\n",
			  obj->GetTypeName(), event_name));
		return -1;
	}

	token = obj->AddHandler (event_id, proxy_listener_to_javascript, this);
	return token;
}

void
EventListenerProxy::RemoveHandler ()
{
	if (target_object && event_id != -1) {
		target_object->RemoveHandler (event_id, token);
		event_id = -1;
	}
}

void
EventListenerProxy::Invalidate ()
{
	if (is_func)
		callback = NULL;
}

void
EventListenerProxy::on_target_object_destroyed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy *) closure;
	
	proxy->target_object = NULL;
}

void
EventListenerProxy::proxy_listener_to_javascript (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy *) closure;
	EventObject *js_sender = sender;
	NPVariant args[2];
	NPVariant result;
	
	if (proxy->instance->pdata == NULL) {
		// Firefox can invalidate our NPObjects after the plugin itself
		// has been destroyed. During this invalidation our NPObjects call 
		// into the moonlight runtime, which then emits events.
		d(printf ("Moonlight: The plugin has been deleted, but we're still emitting events?\n"));
		return;
	}

	if (js_sender->GetObjectType () == Type::SURFACE) {
		// This is somewhat hackish, but is required for
		// the FullScreenChanged event (js expects the
		// sender to be the toplevel canvas, not the surface,
		// nor the content).
		js_sender = ((Surface*) js_sender)->GetToplevel ();
	}

	MoonlightEventObjectObject *depobj = NULL; 
	if (js_sender) {
		depobj = EventObjectCreateWrapper (proxy->instance, js_sender);
		OBJECT_TO_NPVARIANT (depobj, args[0]);
	} else {
		NULL_TO_NPVARIANT (args[0]);
	}

	//printf ("proxying event %s to javascript, sender = %p (%s)\n", proxy->event_name, sender, sender->GetTypeName ());
	MoonlightEventObjectObject *depargs = NULL; 
	if (calldata) {
		depargs = EventObjectCreateWrapper (proxy->instance, calldata);
		OBJECT_TO_NPVARIANT (depargs, args[1]);
	} else {
		NULL_TO_NPVARIANT (args[1]);
	}
	
	if (proxy->is_func) {
		/* the event listener was added with a JS function object */
		if (NPN_InvokeDefault (proxy->instance, (NPObject *) proxy->callback, args, 2, &result))
			NPN_ReleaseVariantValue (&result);
	} else {
		/* the event listener was added with a JS string (the function name) */
		NPObject *object = NULL;
		
		if (NPN_GetValue (proxy->instance, NPNVWindowNPObject, &object) == NPERR_NO_ERROR) {
			if (NPN_Invoke (proxy->instance, object, NPID ((char *) proxy->callback), args, 2, &result))
				NPN_ReleaseVariantValue (&result);
		}
	}
	
	NPN_ReleaseObject (depobj);
	NPN_ReleaseObject (depargs);
}


void
event_object_add_javascript_listener (EventObject *obj, PluginInstance *plugin, const char *event_name, const char *cb_name)
{
	EventListenerProxy *proxy = new EventListenerProxy (plugin->getNPP(), event_name, cb_name);
	proxy->AddHandler (obj);
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
		INT32_TO_NPVARIANT (args->error_code, *result);
		return true;

	case MoonId_ErrorType:
		switch (args->error_type) {
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
		string_to_npvariant (args->error_message, result);
		return true;
	case MoonId_LineNumber:
		if (args->error_type == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)args)->line_number, *result);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.lineNumber");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;
	case MoonId_CharPosition:
		if (args->error_type == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)args)->char_position, *result);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.charPosition");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;
	case MoonId_MethodName:
		DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.methodName");
		INT32_TO_NPVARIANT (0, *result);
		return true;
	case MoonId_XamlFile:
		if (args->error_type == ParserError) {
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
		DOUBLE_TO_NPVARIANT (rect.w, *result);
		return true;

	case MoonId_Height:
		DOUBLE_TO_NPVARIANT (rect.h, *result);
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
		rect.w = NPVARIANT_TO_DOUBLE (*value);
		return true;

	case MoonId_Height:
		rect.h = NPVARIANT_TO_DOUBLE (*value);
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



/*** MoonlightMouseEventArgsClass  **************************************************************/

static NPObject *
mouse_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMouseEventArgsObject (instance);
}

static const MoonNameIdMapping
mouse_event_mapping[] = {
	{ "ctrl", MoonId_Ctrl },
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
		BOOLEAN_TO_NPVARIANT (state & GDK_SHIFT_MASK != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT (state & GDK_CONTROL_MASK != 0, *result);
		return true;

	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}
}

bool
MoonlightMouseEventArgsObject::Invoke (int id, NPIdentifier name,
				       const NPVariant *args, uint32_t argCount,
				       NPVariant *result)
{
	MouseEventArgs *event_args = GetMouseEventArgs ();

	switch (id) {
	case MoonId_GetPosition: {
		if (argCount != 1)
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

		MoonlightPoint *point = (MoonlightPoint*)NPN_CreateObject (instance, MoonlightPointClass);
		point->point = Point (x, y);

		OBJECT_TO_NPVARIANT (point, *result);

		return true;
	}
	case MoonId_GetStylusInfo: {
		if (argCount != 0)
			THROW_JS_EXCEPTION ("getStylusInfo");

		StylusInfo *info = event_args->GetStylusInfo ();
		MoonlightEventObjectObject *info_obj = EventObjectCreateWrapper (instance, info);
		info->unref ();
		OBJECT_TO_NPVARIANT (info_obj, *result);
		
		return true;
	}
	case MoonId_GetStylusPoints: {
		if (argCount != 1)
			THROW_JS_EXCEPTION ("getStylusPoints");

		if (npvariant_is_dependency_object (args[0])) {
			DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
			if (!dob->Is (Type::INKPRESENTER))
				THROW_JS_EXCEPTION ("getStylusPoints");
			
			StylusPointCollection *points = event_args->GetStylusPoints ((UIElement*)dob);
			MoonlightEventObjectObject *col_obj = EventObjectCreateWrapper (instance, points);
			points->unref ();
			OBJECT_TO_NPVARIANT (col_obj, *result);
		}

		return true;
	}
	default:
		return MoonlightEventArgs::Invoke (id, name, args, argCount, result);
	}
}


MoonlightMouseEventArgsType::MoonlightMouseEventArgsType ()
{
	allocate = mouse_event_allocate;

	AddMapping (mouse_event_mapping, G_N_ELEMENTS (mouse_event_mapping));
}

MoonlightMouseEventArgsType *MoonlightMouseEventArgsClass;


/*** MoonlightMarkerReachedEventArgsClass  **************************************************************/

static NPObject *
marker_reached_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMarkerReachedEventArgsObject (instance);
}

static const MoonNameIdMapping
marker_reached_event_mapping[] = {
	{ "marker", MoonId_Marker }
};

bool
MoonlightMarkerReachedEventArgsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	MarkerReachedEventArgs *args = GetMarkerReachedEventArgs ();
	TimelineMarker *marker = args ? args->GetMarker () : NULL;

	switch (id) {
	case MoonId_Marker: {
		MoonlightEventObjectObject *meoo = EventObjectCreateWrapper (instance, marker);
		OBJECT_TO_NPVARIANT (meoo, *result);
		return true;
	}
	default:
		return MoonlightEventArgs::GetProperty (id, name, result);;
	}
}

MoonlightMarkerReachedEventArgsType::MoonlightMarkerReachedEventArgsType ()
{
	allocate = marker_reached_event_allocate;

	AddMapping (marker_reached_event_mapping, G_N_ELEMENTS (marker_reached_event_mapping));
}

MoonlightMarkerReachedEventArgsType *MoonlightMarkerReachedEventArgsClass;

/*** MoonlightKeyboardEventArgsClass  **************************************************************/

static NPObject *
keyboard_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightKeyboardEventArgsObject (instance);
}

static const MoonNameIdMapping
keyboard_event_mapping[] = {
	{ "ctrl", MoonId_Ctrl },
	{ "key", MoonId_Key },
	{ "platformkeycode", MoonId_PlatformKeyCode },
	{ "shift", MoonId_Shift },
};


bool
MoonlightKeyboardEventArgsObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	KeyboardEventArgs *args = GetKeyboardEventArgs ();

	switch (id) {
	case MoonId_Shift:
		BOOLEAN_TO_NPVARIANT (args->state & GDK_SHIFT_MASK != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT (args->state & GDK_CONTROL_MASK != 0, *result);
		return true;

	case MoonId_Key:
		INT32_TO_NPVARIANT (args->key, *result);
		return true;

	case MoonId_PlatformKeyCode:
		INT32_TO_NPVARIANT (args->platformcode, *result);
		return true;

	default:
		return MoonlightEventArgs::GetProperty (id, name, result);
	}
}

MoonlightKeyboardEventArgsType::MoonlightKeyboardEventArgsType ()
{
	allocate = keyboard_event_allocate;

	AddMapping (keyboard_event_mapping, G_N_ELEMENTS (keyboard_event_mapping));
}

MoonlightKeyboardEventArgsType *MoonlightKeyboardEventArgsClass;

/*** our object base class */
NPObject *
_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightObject (instance);
}

static void
_deallocate (NPObject *npobj)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	
	delete obj;
}

MoonlightObject::~MoonlightObject ()
{
	if (event_listener_proxies) {
		g_hash_table_destroy (event_listener_proxies);
		event_listener_proxies = NULL;
	}
}

void
MoonlightObject::destroy_proxy (gpointer data)
{
	EventListenerProxy *proxy = (EventListenerProxy*)data;
	proxy->RemoveHandler ();
	delete (EventListenerProxy*)data;
}

void
MoonlightObject::invalidate_proxy (gpointer key, gpointer value, gpointer data)
{
	EventListenerProxy *proxy = (EventListenerProxy*)value;
	proxy->Invalidate ();
}

void
MoonlightObject::Invalidate ()
{
	g_hash_table_foreach (event_listener_proxies, invalidate_proxy, NULL);
}

bool
MoonlightObject::HasProperty (NPIdentifier name)
{
	return IS_PROPERTY (LookupName (name));
}

bool
MoonlightObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETVALUE");
	NULL_TO_NPVARIANT (*result);
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
MoonlightObject::Invoke (int id, NPIdentifier name,
			 const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	switch (id) {
	case MoonId_ToString:
		if (argCount != 0)
			return false;

		if (moonlight_type != Type::INVALID) {
			string_to_npvariant (Type::Find (moonlight_type)->name, result);
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
MoonlightObject::SetEventProxy (int event_id, EventListenerProxy *proxy)
{
	g_hash_table_insert (event_listener_proxies, GINT_TO_POINTER (event_id), proxy);
}

void
MoonlightObject::ClearEventProxy (int event_id)
{
	g_hash_table_remove (event_listener_proxies, GINT_TO_POINTER (event_id));
}


static void
_invalidate (NPObject *npobj)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	
	obj->Invalidate ();
}

static bool
_has_method (NPObject *npobj, NPIdentifier name)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	return obj->HasMethod (name);
}

static bool
_has_property (NPObject *npobj, NPIdentifier name)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	return obj->HasProperty (name);
}

static bool
_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	return obj->GetProperty (id, name, result);
}

static bool
_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	return obj->SetProperty (id, name, value);
}

static bool
_remove_property (NPObject *npobj, NPIdentifier name)
{
	g_warning ("moonlight_object_remove_property reached");
	return false;
}

static bool
_enumerate (NPObject *npobj, NPIdentifier **value, uint32_t *count)
{
	return ((MoonlightObjectType*)npobj->_class)->Enumerate (value, count);
}

static bool
_invoke (NPObject *npobj, NPIdentifier name,
	 const NPVariant *args, uint32_t argCount,
	 NPVariant *result)
{
	MoonlightObject *obj = (MoonlightObject *) npobj;
	int id = obj->LookupName (name);
	return obj->Invoke (id, name, args, argCount, result);
}

static bool
_invoke_default (NPObject *npobj,
		 const NPVariant *args, uint32_t argCount,
		 NPVariant *result)
{
	g_warning ("moonlight_object_invoke_default reached");
	return false;
}

static const MoonNameIdMapping
object_mapping[] = {
	{ "tostring", MoonId_ToString },
};

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
	enumerate      = _enumerate;

	mapping = NULL;
	mapping_count = 0;

	AddMapping (object_mapping, G_N_ELEMENTS (object_mapping));

	last_lookup = NULL;
	last_id = 0;
}

bool
MoonlightObjectType::Enumerate (NPIdentifier **value, uint32_t *count)
{
	if (mapping_count == 0) {
		*value = NULL;
		*count = 0;
		return true;
	}

	// caller frees this
	NPIdentifier *ids = (NPIdentifier*)NPN_MemAlloc (sizeof (NPIdentifier) * mapping_count);

	for (int i = 0; i < mapping_count; i ++)
		ids[i] = NPN_GetStringIdentifier (mapping[i].name);

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
	{ "settings", MoonId_Settings },
	{ "source", MoonId_Source },
};

MoonlightScriptControlObject::~MoonlightScriptControlObject ()
{
 	if (settings)
		NPN_ReleaseObject (settings);
	
 	if (content)
		NPN_ReleaseObject (content);
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
	PluginInstance *plugin = (PluginInstance*) instance->pdata;
	
	switch (id) {
	case MoonId_Settings:
		NPN_RetainObject (settings);
		OBJECT_TO_NPVARIANT (settings, *result);
		return true;
	case MoonId_Content:
		NPN_RetainObject (content);
		OBJECT_TO_NPVARIANT (content, *result);
		return true;
	case MoonId_InitParams:
		string_to_npvariant (plugin->getInitParams (), result);
		return true;
	case MoonId_IsLoaded:
		BOOLEAN_TO_NPVARIANT (plugin->getIsLoaded (), *result);
		return true;
	case MoonId_Source:
		string_to_npvariant (plugin->getSource (), result);
		return true;

	case MoonId_Id:
		// FIXME we need to look at how the plugin deals with 
		// unknown params on the dom node
		NULL_TO_NPVARIANT (*result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightScriptControlObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_Source:
		plugin->setSource (STR_FROM_VARIANT (*value));
		return true;
	default:
		return MoonlightObject::SetProperty (id, name, value);
	}
}

bool
MoonlightScriptControlObject::Invoke (int id, NPIdentifier name,
				      const NPVariant *args, uint32_t argCount,
				      NPVariant *result)
{
	switch (id) {
	case MoonId_CreateObject: {
		if (argCount != 1 || !NPVARIANT_IS_STRING(args[0])) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		NPObject *obj = NULL;
		const char *object_type = STR_FROM_VARIANT (args [0]);
		if (!g_ascii_strcasecmp ("downloader", object_type)) {
			PluginInstance *plugin = (PluginInstance*) instance->pdata;
			Downloader *dl = PluginInstance::CreateDownloader (plugin);

			obj = EventObjectCreateWrapper (instance, dl);
			dl->unref ();

			OBJECT_TO_NPVARIANT (obj, *result);
			return true;
		} else {
			THROW_JS_EXCEPTION ("createObject");

			NULL_TO_NPVARIANT (*result);
			return true;
		}
	}

	case MoonId_IsVersionSupported: {
		/* we support all 0.*, 1.0.* and 1.1.* versions. */
		
		if (argCount != 1 || !NPVARIANT_IS_STRING (args [0])) {
			return false;
		}

		bool supported = true;
		gchar **versions = g_strsplit (STR_FROM_VARIANT (args [0]), ".", 4);
		char *version = NULL;
		uint64_t numbers [4];

		supported = versions [0] != NULL && versions [1] != NULL;

		if (supported) {
			for (int k = 0; k < 4; k++) {
				numbers [k] = 0;
				version = versions [k];
				
				if (version == NULL)
					break;
							
				// Only allow ascii 0-9 characters in the numbers
				for (int i = 0; version [i] != 0; i++) {
					if (version [i] < '0' || version [i] > '9') {
						supported = false;
						break;
					}
				}
				
				numbers [k] = atoll (version	);
			}
			
			switch (numbers [0]) {
			case 0: // We support all versions of the format "0.*"
				break;
#if INCLUDE_MONO_RUNTIME
			case 1:
				supported &= numbers [1] <= 1; // 1.0* or 1.1*
				break;
#else				
			case 1:
				supported &= numbers [1] == 0; // 1.0*
				break;
#endif
			default:
				supported = false;
				break;
			}
		}
		
		d(printf ("version requested = '%s' (%s)\n", STR_FROM_VARIANT (args [0]), supported ? "yes" : "no"));
		
		BOOLEAN_TO_NPVARIANT (supported, *result);

		g_strfreev (versions);

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
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_Background:
		string_to_npvariant (plugin->getBackground (), result);
		return true;

	case MoonId_EnableFramerateCounter:
		BOOLEAN_TO_NPVARIANT (plugin->getEnableFramerateCounter (), *result);
		return true;

	case MoonId_EnableRedrawRegions:
		BOOLEAN_TO_NPVARIANT (plugin->getEnableRedrawRegions (), *result);
		return true;

	case MoonId_EnableHtmlAccess:
		BOOLEAN_TO_NPVARIANT (plugin->getEnableHtmlAccess (), *result);
		return true;

	// not implemented yet, just return 0.
	case MoonId_MaxFrameRate:
		INT32_TO_NPVARIANT (0, *result);
		return true;

	case MoonId_Version:
		string_to_npvariant (PLUGIN_VERSION, result);
		return true;

	case MoonId_Windowless:
		BOOLEAN_TO_NPVARIANT (plugin->getWindowless (), *result);
		return true;

	default:
		return MoonlightObject::GetProperty (id, name, result);
	}
}

bool
MoonlightSettingsObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {

	case MoonId_Background:
		if (!plugin->setBackground (STR_FROM_VARIANT (*value)))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
		
		return true;

	// Cant be set after initialization so return true
	case MoonId_EnableFramerateCounter:
		return true;
 
	case MoonId_EnableRedrawRegions:
		plugin->setEnableRedrawRegions (NPVARIANT_TO_BOOLEAN (*value));
		return true;

	// Cant be set after initialization so return true
	case MoonId_EnableHtmlAccess:
		return true;

	// not implemented yet.
	case MoonId_MaxFrameRate:
		plugin->setMaxFrameRate (NPVARIANT_TO_INT32 (*value));
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
				 const NPVariant *args, uint32_t argCount, NPVariant *result)
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
}

static const MoonNameIdMapping
moonlight_content_mapping[] = {
	{ "actualheight", MoonId_ActualHeight },
	{ "actualwidth", MoonId_ActualWidth },
	{ "createfromxaml", MoonId_CreateFromXaml },
	{ "createfromxamldownloader", MoonId_CreateFromXamlDownloader },
	{ "createobject", MoonId_CreateObject },
	{ "findname", MoonId_FindName },
	{ "fullscreen", MoonId_FullScreen },
	{ "onerror", MoonId_OnError },
	{ "onfullscreenchange", MoonId_OnFullScreenChange },
	{ "onresize", MoonId_OnResize },
	{ "root", MoonId_Root },
};

bool
MoonlightContentObject::HasProperty (NPIdentifier name)
{
	if (MoonlightObject::HasProperty (name))
		return true;

	// FIXME: this is still case sensitive (uses a direct hash on the NPIdentifier)
	return g_hash_table_lookup (registered_scriptable_objects, name) != NULL;
}

bool
MoonlightContentObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_ActualHeight:
		INT32_TO_NPVARIANT (plugin->getActualHeight (), *result);
		return true;
	case MoonId_ActualWidth:
		INT32_TO_NPVARIANT (plugin->getActualWidth (), *result);
		return true;
	case MoonId_FullScreen:
		BOOLEAN_TO_NPVARIANT (plugin->surface->GetFullScreen (), *result);
		return true;
	case MoonId_OnResize:
	case MoonId_OnFullScreenChange:
	case MoonId_OnError: {
		const char *event_name = map_moon_id_to_event_name (id);
		int event_id = plugin->surface->GetType()->LookupEvent (event_name);
		EventListenerProxy *proxy = LookupEventProxy (event_id);
		string_to_npvariant (proxy == NULL ? "" : proxy->GetCallbackAsString (), result);
		return true;
	}
	case MoonId_Root: {
		DependencyObject *top = plugin->surface->GetToplevel ();
		if (top == NULL) {
			NULL_TO_NPVARIANT (*result);
		} else {
			MoonlightEventObjectObject *topobj = EventObjectCreateWrapper (instance, top);

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
		
		NPN_RetainObject (obj);
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
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_FullScreen:
		plugin->surface->SetFullScreen (NPVARIANT_TO_BOOLEAN (*value));
		return true;
	case MoonId_OnFullScreenChange:
	case MoonId_OnResize:
	case MoonId_OnError: {
		const char *event_name = map_moon_id_to_event_name (id);
		int event_id = plugin->surface->GetType()->LookupEvent (event_name);

		if (event_id != -1) {
			// If we have a handler, remove it.
			ClearEventProxy (event_id);

			if (!NPVARIANT_IS_NULL (*value)) {
				EventListenerProxy *proxy = new EventListenerProxy (instance,
										    event_name,
										    value);
				proxy->AddHandler (plugin->surface);
				SetEventProxy (event_id, proxy);
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
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;
	
	switch (id) {
	case MoonId_FindName: {
		if (argCount != 1)
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_FINDNAME");

		char *name = STR_FROM_VARIANT (args [0]);

		if (!plugin->surface || !plugin->surface->GetToplevel ())
			return true;

		DependencyObject *element = plugin->surface->GetToplevel ()->FindName (name);
		if (!element) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, element), *result);
		return true;
	}

	case MoonId_CreateObject:
		// not implemented yet
		DEBUG_WARN_NOTIMPLEMENTED ("content.createObject");
		return true;

	case MoonId_CreateFromXaml: {
		if (!argCount)
			THROW_JS_EXCEPTION ("createFromXaml");
		
		bool create_namescope = argCount >= 2 ? NPVARIANT_TO_BOOLEAN (args[1]) : false;
		char *xaml = STR_FROM_VARIANT (args[0]);
		
		if (!xaml)
			THROW_JS_EXCEPTION ("createFromXaml");
		
		Type::Kind element_type;
		XamlLoader *loader = PluginXamlLoader::FromStr (xaml, plugin, plugin->surface);
		DependencyObject *dep = xaml_create_from_str (loader, xaml, create_namescope, &element_type);
		delete loader;
		
		if (!dep)
			THROW_JS_EXCEPTION ("createFromXaml");

		MoonlightEventObjectObject *depobj = EventObjectCreateWrapper (instance, dep);
		dep->unref ();

		OBJECT_TO_NPVARIANT (depobj, *result);
		return true;
	}

	case MoonId_CreateFromXamlDownloader: {
		if (argCount < 2)
			THROW_JS_EXCEPTION ("createFromXamlDownloader");

		Downloader *down = (Downloader*)((MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (args [0]))->GetDependencyObject ();

		Type::Kind element_type;

		DependencyObject* dep = NULL;

		char *fname = down->GetDownloadedFilePart (STR_FROM_VARIANT (args[1]));
		if (fname != NULL) {
			XamlLoader *loader = PluginXamlLoader::FromFilename (fname, plugin, plugin->surface);
			dep = xaml_create_from_file (loader, fname, false, &element_type);
			delete loader;

			g_free (fname);
		}

		if (!dep)
			THROW_JS_EXCEPTION ("createFromXamlDownloader");

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, dep), *result);
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
	{ "capturemouse", MoonId_CaptureMouse },
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
	{ "releasemousecapture", MoonId_ReleaseMouseCapture },
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

		Type *type = Type::Find (type_name);

		if (type != NULL)
			p = dependency_property_lookup (type->type, attrname);

		g_free (type_name);
	}

	return p;
}

static bool
_set_dependency_property_value (DependencyObject *dob, DependencyProperty *prop, const NPVariant *value)
{
	if (npvariant_is_moonlight_object (*value)) {
		MoonlightObject *obj = (MoonlightObject *) NPVARIANT_TO_OBJECT (*value);
		MoonlightDuration *duration;
		MoonlightTimeSpan *ts;
		MoonlightPoint *point;
		MoonlightRect *rect;
		
		if (Type::IsSubclassOf (obj->moonlight_type, Type::DEPENDENCY_OBJECT) && obj->moonlight_type != Type::INVALID) {
			MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (*value);
			dob->SetValue (prop, Value (depobj->GetDependencyObject ()));
			
			return true;
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
				  obj->moonlight_type, Type::Find (obj->moonlight_type)->name));
			w(printf ("unhandled object type in do.set_property"));
			return true;
		}
	} else {
		const char *strval = NULL;
		char strbuf[64];
		
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
			g_snprintf (strbuf, sizeof (strbuf), "%g", NPVARIANT_TO_DOUBLE (*value));
			
			strval = strbuf;
		} else if (NPVARIANT_IS_STRING (*value)) {
			strval = STR_FROM_VARIANT (*value);
		} else if (NPVARIANT_IS_NULL (*value)) {
			if (Type::IsSubclassOf (prop->value_type, Type::DEPENDENCY_OBJECT)) {
				DependencyObject *val = NULL;
				
				dob->SetValue (prop, Value (val));
			} else if (prop->value_type == Type::STRING) {
				char *val = NULL;
				
				dob->SetValue (prop, Value (val));
			} else 
				dob->SetValue (prop, NULL);
			
			return true;
		} else if (NPVARIANT_IS_VOID (*value)) {
			d(printf ("unhandled variant type VOID in do.set_property for (%s::%s)",
				  dob->GetTypeName (), prop->name));
			return true;
		} else {
			d(printf ("unhandled variant type in do.set_property for (%s::%s)",
				  dob->GetTypeName (), prop->name));
			return true;
		}
		
		g_assert (strval != NULL);
		
		return xaml_set_property_from_str (dob, prop, strval);
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
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	if (!strname)
		return false;

	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	return (p != NULL);
}

bool
MoonlightDependencyObjectObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyObject *dob = GetDependencyObject ();
	DependencyProperty *prop;
	const char *event_name;
	int event_id;
	Value *value;
	
	if (!strname)
		return false;
	
	prop = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);
	
	if (prop) {
		if (!(value = dob->GetValue (prop))) {
			// strings aren't null, they seem to just be empty strings
			if (prop->value_type == Type::STRING) {
				string_to_npvariant ("", result);
				return true;
			}
			
			NULL_TO_NPVARIANT (*result);
			return true;
		}
		
		if (value->GetKind () == Type::INT32) {
			const char *s = enums_int_to_str (prop->name, value->AsInt32 ());
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
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyObject *dob = GetDependencyObject ();
	DependencyProperty *prop;
	
	if (!strname)
		return false;
	
	prop = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);
	
	if (prop) {
		if (_set_dependency_property_value (dob, prop, value)) {
			return true;
		} else {
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
		}
	}
	
	// turns out that on Silverlight you can't set regular events as properties.
#if 0
	// it wasn't a dependency property.  let's see if it's an
	// event
	const char *event_name;
	
	if ((event_name = map_moon_id_to_event_name (id))) {
		int event_id;
		
		if ((event_id = dob->GetType ()->LookupEvent (event_name)) != -1) {
			// If we have a handler, remove it.
			ClearEventProxy (event_id);
			
			if (!NPVARIANT_IS_NULL (*value)) {
				EventListenerProxy *proxy = new EventListenerProxy (instance,
										    event_name,
										    value);
				proxy->AddHandler (dob);
				SetEventProxy (event_id, proxy);
			}
			
			return true;
		}
	}
#endif
	
	return MoonlightObject::SetProperty (id, name, value);
}

bool
MoonlightDependencyObjectObject::Invoke (int id, NPIdentifier name,
					 const NPVariant *args, uint32_t argCount,
					 NPVariant *result)
{
	DependencyObject *dob = GetDependencyObject ();

	switch (id) {
#if DEBUG_JAVASCRIPT
	// Some debug code...
	// with this it is possible to do obj.printf ("msg") from js
	case MoonId_Printf:
		fprintf (stderr, "JS message: %s\n", STR_FROM_VARIANT (args[0]));
		VOID_TO_NPVARIANT (*result);
		return true;
#endif
	case MoonId_Equals: {
		if (argCount != 1 || !NPVARIANT_IS_OBJECT (args[0]))
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

	case MoonId_SetValue:
		/* obj.setValue (prop, val) is another way of writing obj[prop] = val (or obj.prop = val) */
		if (argCount < 2 || !NPVARIANT_IS_STRING (args[0]))
			THROW_JS_EXCEPTION ("setValue");

		_class->setProperty (this,
				     NPID (STR_FROM_VARIANT (args [0])),
				     &args[1]);

		VOID_TO_NPVARIANT (*result);
		return true;

	case MoonId_GetValue:
		if (argCount < 1 || !NPVARIANT_IS_STRING (args[0]))
			THROW_JS_EXCEPTION ("getValue");

		_class->getProperty (this,
				     NPID (STR_FROM_VARIANT (args [0])),
				     result);
		return true;

	case MoonId_FindName: {
		if (argCount != 1 || !NPVARIANT_IS_STRING (args [0]))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_FINDNAME");

		char *name = STR_FROM_VARIANT (args [0]);

		DependencyObject *element = dob->FindName (name);
		if (!element) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, element), *result);
		return true;
	}

	case MoonId_GetHost: {
		PluginInstance *plugin = (PluginInstance*) instance->pdata;
		
		if (argCount != 0)
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETHOST");

		OBJECT_TO_NPVARIANT ((NPObject *) plugin->getRootObject (), *result);

		return true;
	}

	case MoonId_GetParent: {
		if (argCount != 0 || !dob->GetType ()->IsSubclassOf (Type::UIELEMENT))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_GETPARENT");
		
		DependencyObject *parent = dob->GetLogicalParent ();
		if (parent)
			OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, parent), *result);
		else
			NULL_TO_NPVARIANT (*result);

		return true;
	}

	case MoonId_AddEventListener: {
		if (argCount != 2)
			THROW_JS_EXCEPTION ("addEventListener");

		if (!NPVARIANT_IS_STRING (args[0])
		    || (!NPVARIANT_IS_STRING (args[1]) && !NPVARIANT_IS_OBJECT (args[1]))) {
			/* FIXME: how do we check if args[1] is a function? */
			THROW_JS_EXCEPTION ("addEventListener");
		}

		char *name = g_strdup (STR_FROM_VARIANT (args [0]));

		name[0] = toupper(name[0]);

		EventListenerProxy *proxy = new EventListenerProxy (instance, name, &args[1]);
		
		int token = proxy->AddHandler (dob);
		
		if (token == -1)
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_ADDEVENT");

		g_free (name);

		INT32_TO_NPVARIANT (token, *result);
		return true;
	}

	case MoonId_RemoveEventListener: {
		if (argCount < 2)
			THROW_JS_EXCEPTION ("removeEventListener");
		if (!NPVARIANT_IS_STRING (args[0])
		    || (!NPVARIANT_IS_INT32 (args [1])
			&& !NPVARIANT_IS_STRING (args [1])))
			THROW_JS_EXCEPTION ("removeEventListener");
		
		int id = dob->GetType()->LookupEvent (STR_FROM_VARIANT (args[0]));

		if (id == -1) {
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_DELEVENT");
		} else if (NPVARIANT_IS_INT32 (args [1])) {
			dob->RemoveHandler (STR_FROM_VARIANT (args[0]), NPVARIANT_TO_INT32 (args[1]));
		} else if (NPVARIANT_IS_STRING (args[1])) {
			NamedProxyPredicate predicate (STR_FROM_VARIANT (args [1]));

			dob->RemoveMatchingHandlers (STR_FROM_VARIANT (args[0]), NamedProxyPredicate::matches, &predicate);
		}

		return true;
	}

	// FIXME: these next two methods should live in a UIElement
	// wrapper class, not in the DependencyObject wrapper.
	case MoonId_CaptureMouse:
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->CaptureMouse (), *result);
		return true;
	case MoonId_ReleaseMouseCapture:
		((UIElement*)dob)->ReleaseMouseCapture ();

		VOID_TO_NPVARIANT (*result);
		return true;
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
		if ((plugin = (PluginInstance *) instance->pdata))
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
EventObjectCreateWrapper (NPP instance, EventObject *obj)
{
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	MoonlightEventObjectObject *depobj;
	NPClass *np_class;
	
	depobj = (MoonlightEventObjectObject *) plugin->LookupWrappedObject (obj);
	
	if (depobj) {
		NPN_RetainObject (depobj);
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
	case Type::CONTROL:
		np_class = dependency_object_classes [CONTROL_CLASS];
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
	case Type::EVENTOBJECT: 
	case Type::SURFACE: 
		np_class = MoonlightEventObjectClass;
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
	case Type::MOUSEEVENTARGS:
		np_class = dependency_object_classes [MOUSE_EVENT_ARGS_CLASS];
		break;
	case Type::KEYBOARDEVENTARGS:
		np_class = dependency_object_classes [KEYBOARD_EVENT_ARGS_CLASS];
		break;
	case Type::MARKERREACHEDEVENTARGS:
		np_class = dependency_object_classes [MARKER_REACHED_EVENT_ARGS_CLASS];
		break;
	case Type::ERROREVENTARGS:
	case Type::PARSERERROREVENTARGS:
	case Type::IMAGEERROREVENTARGS:
	case Type::MEDIAERROREVENTARGS:
		np_class = dependency_object_classes [ERROR_EVENT_ARGS_CLASS];
		break;
	default:
		if (Type::Find (kind)->IsSubclassOf (Type::COLLECTION))
			np_class = dependency_object_classes [COLLECTION_CLASS];
		else if (Type::Find (kind)->IsSubclassOf (Type::EVENTARGS)) 
			np_class = dependency_object_classes [EVENT_ARGS_CLASS];
		else
			np_class = dependency_object_classes [DEPENDENCY_OBJECT_CLASS];
	}
	
	depobj = (MoonlightEventObjectObject *) NPN_CreateObject (instance, np_class);
	depobj->moonlight_type = obj->GetObjectType ();
	depobj->eo = obj;
	obj->ref ();
	
	plugin->AddWrappedObject (obj, depobj);
	
	/* do any post creation initialization here */
	switch (obj->GetObjectType()) {
	case Type::CONTROL:
		((MoonlightControlObject *) depobj)->real_object = EventObjectCreateWrapper (instance,
											     ((Control *) obj)->real_object);
		break;
	default:
		/* nothing to do */
		break;
	}

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
		INT32_TO_NPVARIANT (col->list->Length (), *result);
		return true;
	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightCollectionObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, uint32_t argCount,
				   NPVariant *result)
{
	Collection *col = (Collection *) GetDependencyObject ();

	switch (id) {
	case MoonId_Add: {
		if (argCount < 1 || !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("add");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);

		int n = col->Add (el->GetDependencyObject ());

		if (n == -1)
			THROW_JS_EXCEPTION ("add");
		
		INT32_TO_NPVARIANT (n, *result);

		return true;
	}
	case MoonId_Remove: {
		if (argCount < 1 || !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("remove");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);
		bool res = col->Remove (el->GetDependencyObject ());
		
		BOOLEAN_TO_NPVARIANT (res, *result);
		
		return true;
	}
	case MoonId_RemoveAt: {
		if (argCount < 1 || !NPVARIANT_IS_INT32 (args[0]))
			THROW_JS_EXCEPTION ("removeAt");
		
		int index = NPVARIANT_TO_INT32 (args [0]);

		if (index < 0 || index >= col->list->Length ())
			THROW_JS_EXCEPTION ("removeAt");

		Collection::Node *n = (Collection::Node *) col->list->Index (index);
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, n->obj), *result);

		col->RemoveAt (index);
		
		return true;
	}
	case MoonId_Insert: {
		if (argCount < 1)
			THROW_JS_EXCEPTION ("insert");

		if (argCount < 2) {
			VOID_TO_NPVARIANT (*result);
			return true;
		}

		if (!NPVARIANT_IS_INT32 (args[0]) || !npvariant_is_dependency_object (args[1]))
			THROW_JS_EXCEPTION ("insert");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (args[1]);
		int index = NPVARIANT_TO_INT32 (args[0]);
		
		if (!col->Insert (index, el->GetDependencyObject ()))
			THROW_JS_EXCEPTION ("insert");
		
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
		if (argCount < 1 || !NPVARIANT_IS_INT32 (args[0]))
			THROW_JS_EXCEPTION ("getItem");
		
		int index = NPVARIANT_TO_INT32 (args[0]);

		if (index < 0)
			THROW_JS_EXCEPTION ("getItem");
		
		if (index >= col->list->Length ()) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}
		
		Collection::Node *n = (Collection::Node *) col->list->Index (index);
		
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, n->obj), *result);
		
		return true;
	}
	case MoonId_GetItemByName: {
		if (argCount < 1 || col->GetObjectType () != Type::MEDIAATTRIBUTE_COLLECTION)
			THROW_JS_EXCEPTION ("getItemByName");
		
		const char *name = STR_FROM_VARIANT (args[0]);
		
		DependencyObject *obj = media_attribute_collection_get_item_by_name ((MediaAttributeCollection *) col, name);
		
		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, obj), *result);
		
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
				   const NPVariant *args, uint32_t argCount,
				   NPVariant *result)
{
	Storyboard *sb = (Storyboard*)GetDependencyObject ();

	switch (id) {
	case MoonId_Begin:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("begin");

		sb->Begin ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Pause:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("pause");

		sb->Pause ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Resume:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("resume");

		sb->Resume ();

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Seek: {
		if (argCount != 1)
			THROW_JS_EXCEPTION ("seek");
		
		TimeSpan ts;

		if (NPVARIANT_IS_INT32 (args[0])) {
			ts = (TimeSpan)NPVARIANT_TO_INT32(args[0]);
		}
		else if (NPVARIANT_IS_STRING (args[0])) {
			if (!time_span_from_str (STR_FROM_VARIANT (args[0]), &ts))
				THROW_JS_EXCEPTION ("seek");
		}
		else
			THROW_JS_EXCEPTION ("seek");
		      
		sb->Seek (ts);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	case MoonId_Stop:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("stop");

		sb->Stop ();

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
				     const NPVariant *args, uint32_t argCount,
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
		if (argCount != 2 || !npvariant_is_downloader (args[0]) || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_METHOD");
		
		DependencyObject *downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();
		const char *part = STR_FROM_VARIANT (args [1]);
		
		media->SetSource ((Downloader *) downloader, part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightMediaElementType::MoonlightMediaElementType ()
{
	AddMapping (moonlight_media_element_mapping, G_N_ELEMENTS (moonlight_media_element_mapping));

	allocate = moonlight_media_element_allocate;
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
	{ "setsource", MoonId_SetSource }
};


bool
MoonlightImageObject::Invoke (int id, NPIdentifier name,
			      const NPVariant *args, uint32_t argCount,
			      NPVariant *result)
{
	Image *img = (Image *) GetDependencyObject ();
	DependencyObject *downloader;
	const char *part;
	
	switch (id) {
	case MoonId_SetSource:
		if (argCount != 2 || !npvariant_is_downloader (args[0]) || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("AG_E_RUNTIME_METHOD");
		
		downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();
		part = STR_FROM_VARIANT (args [1]);
		
		img->SetSource ((Downloader *) downloader, part);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
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
	{ "setsource", MoonId_SetSource }
};


bool
MoonlightImageBrushObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, uint32_t argCount,
				   NPVariant *result)
{
	ImageBrush *img = (ImageBrush *) GetDependencyObject ();
	DependencyObject *downloader;
	const char *part;
	
	switch (id) {
	case MoonId_SetSource:
		if (argCount != 2 || !npvariant_is_downloader (args[0]) || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("setSource");
		
		downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();
		part = STR_FROM_VARIANT (args [1]);
		
		img->SetSource ((Downloader *) downloader, part);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
	}
}

MoonlightImageBrushType::MoonlightImageBrushType ()
{
	AddMapping (moonlight_image_brush_mapping, G_N_ELEMENTS (moonlight_image_brush_mapping));

	allocate = moonlight_image_brush_allocate;
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
				  const NPVariant *args, uint32_t argCount,
				  NPVariant *result)
{
	TextBlock *tb = (TextBlock *) GetDependencyObject ();
	DependencyObject *downloader = NULL;
	
	switch (id) {
	case MoonId_SetFontSource:
		if (argCount != 1 || !(NPVARIANT_IS_NULL (args[0]) || npvariant_is_downloader (args[0])))
			THROW_JS_EXCEPTION ("setFontSource");
		
		if (NPVARIANT_IS_OBJECT (args[0]))
			downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]))->GetDependencyObject ();
		
		tb->SetFontSource ((Downloader *) downloader);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name, args, argCount, result);
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
		switch (stylus_info_get_device_type (info)) {
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
		BOOLEAN_TO_NPVARIANT (stylus_info_get_inverted (info), *result);
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
MoonlightStylusPointCollectionObject::Invoke (int id, NPIdentifier name,
	const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	StylusPointCollection *col = (StylusPointCollection *) GetDependencyObject ();

	switch (id) {
	case MoonId_AddStylusPoints: {
		if (!col || !args || argCount < 1)
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
	const NPVariant *args, uint32_t argCount, NPVariant *result)
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
		if (argCount < 1 || !npvariant_is_dependency_object (args[0]))
			THROW_JS_EXCEPTION ("hitTest");
		
		DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
		if (!dob->Is (Type::STYLUSPOINT_COLLECTION))
			THROW_JS_EXCEPTION ("hitTest");

		StrokeCollection *hit_col = col->HitTest ((StylusPointCollection*)dob);

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, hit_col), *result);
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
	const NPVariant *args, uint32_t argCount, NPVariant *result)
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
		if (argCount < 1 || !npvariant_is_dependency_object (args[0]))
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
	uint64_t size;
	char *text;
	
	switch (id) {
	case MoonId_ResponseText:
		if ((text = downloader->GetResponseText (NULL, &size))) {
			char *s = (char *) NPN_MemAlloc (size + 1);
			memcpy (s, text, size + 1);
			g_free (text);
			
			STRINGN_TO_NPVARIANT (s, (uint32_t) size, *result);
		} else {
			NULL_TO_NPVARIANT (*result);
		}
		
		return true;
	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightDownloaderObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, uint32_t argCount,
				   NPVariant *result)
{
	Downloader *downloader = (Downloader *) GetDependencyObject ();
	char *part, *verb, *uri, *text;
	uint64_t size;
	
	switch (id) {
	case MoonId_Abort:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("abort");

		downloader->Abort ();
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_Open:
		if (argCount > 3)
			THROW_JS_EXCEPTION ("open");
		
		verb = STR_FROM_VARIANT (args[0]);
		uri = STR_FROM_VARIANT (args[1]);
		
		downloader->Open (verb, uri);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_Send:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("send");
		
		downloader->Send ();
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	case MoonId_GetResponseText:
		if (argCount != 1)
			THROW_JS_EXCEPTION ("getResponseText");
		
		part = STR_FROM_VARIANT (args[0]);
		
		if ((text = downloader->GetResponseText (part, &size))) {
			char *s = (char *) NPN_MemAlloc (size + 1);
			memcpy (s, text, size + 1);
			g_free (text);
			
			STRINGN_TO_NPVARIANT (s, (uint32_t) size, *result);
		} else {
			NULL_TO_NPVARIANT (*result);
		}
		
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


/*** MoonlightControlClass ***************************************************/

static NPObject *
moonlight_control_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightControlObject (instance);
}

bool
MoonlightControlObject::HasProperty (NPIdentifier name)
{
	if (real_object->HasProperty (name))
		return true;

	return false;
}

bool
MoonlightControlObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	DependencyObject *dob = GetDependencyObject ();
	DependencyObject *real_dob = ((MoonlightDependencyObjectObject*)real_object)->GetDependencyObject ();
	DependencyProperty *p;

	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	if (!strname)
		return false;

	p = _get_dependency_property (real_dob, strname);
	if (!p)
		p = _get_dependency_property(dob, strname);

	NPN_MemFree (strname);

	if (!p)
		return false;

	Value *value = dob->GetValue (p);
	if (!value)
		return false;

	value_to_variant (this, value, result, dob, p);

	return true;
}

bool
MoonlightControlObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	DependencyObject *dob = GetDependencyObject ();
	DependencyObject *real_dob = ((MoonlightDependencyObjectObject*)real_object)->GetDependencyObject ();
	DependencyProperty *p;

	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	if (!strname)
		return false;

	p = _get_dependency_property (real_dob, strname);
	if (!p)
		p = _get_dependency_property(dob, strname);

	NPN_MemFree (strname);

	if (!p)
		return false;

	if (_set_dependency_property_value (dob, p, value))
		return true;
	else
		THROW_JS_EXCEPTION ("AG_E_RUNTIME_SETVALUE");
}

bool
MoonlightControlObject::HasMethod (NPIdentifier name)
{
	return real_object->HasMethod (name);
}

bool
MoonlightControlObject::Invoke (int id, NPIdentifier name,
				const NPVariant *args, uint32_t argCount,
				NPVariant *result)
{
	return NPN_Invoke (instance, real_object, name, args, argCount, result);
}

MoonlightControlType::MoonlightControlType ()
{
	allocate = moonlight_control_allocate;
}



/*** MoonlightScriptableObjectClass ***************************************************/

// FIXME: the property/method hashes here are case sensitive

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
}

bool
MoonlightScriptableObjectObject::HasProperty (NPIdentifier name)
{
	return (g_hash_table_lookup (properties, name) != NULL
		|| g_hash_table_lookup (events, name)) || MoonlightObject::HasProperty (name);
}

bool
MoonlightScriptableObjectObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (properties, name);
	if (!prop)
		return MoonlightObject::GetProperty (id, name, result);

#if ds(!)0
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	printf ("getting scriptable object property %s", strname);
	NPN_MemFree (strname);
#endif

	Value v;

	getprop (managed_scriptable, prop->property_handle, &v);

	value_to_variant (this, &v, result);

	return true;
}

bool
MoonlightScriptableObjectObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	ScriptableProperty *prop;
	ScriptableEvent *event;
	Value *v;
	
	// first we try the property hash
	if ((prop = (ScriptableProperty *) g_hash_table_lookup (properties, name))) {
#if ds(!)0
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		printf ("setting scriptable object property %s", strname);
		NPN_MemFree (strname);
#endif
		
		variant_to_value (value, &v);
		setprop (managed_scriptable, prop->property_handle, v);
		delete v;
		
		return true;
	}
	
	// if that fails, look for the event of that name
	if ((event = (ScriptableEvent *) g_hash_table_lookup (events, name))) {
#if ds(!)0
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		printf ("adding scriptable object event %s", strname);
		NPN_MemFree (strname);
#endif
		
		if (NPVARIANT_IS_OBJECT (*value)) {
			NPObject *cb_obj = NPVARIANT_TO_OBJECT (*value);

			NPN_RetainObject (cb_obj);

			addevent (managed_scriptable, event->event_handle, this, cb_obj);
		} else {
			DEBUG_WARN_NOTIMPLEMENTED ("scriptableobject.register_event (non-object)");
		}
		
		return true;
	}

	return MoonlightObject::SetProperty (id, name, value);
}

bool
MoonlightScriptableObjectObject::HasMethod (NPIdentifier name)
{
	return g_hash_table_lookup (methods, name) != NULL;
}

bool
MoonlightScriptableObjectObject::Invoke (int id, NPIdentifier name,
					 const NPVariant *args, uint32_t argCount,
					 NPVariant *result)
{
	ScriptableMethod *method = (ScriptableMethod*)g_hash_table_lookup (methods, name);
	Value rv, **vargs = NULL;
	uint32_t i;
	
	if (!method)
		return MoonlightObject::Invoke (id, name, args, argCount, result);

#if ds(!)0
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	printf ("invoking scriptable object method %s", strname);
	NPN_MemFree (strname);
#endif
	
	if (argCount > 0) {
		vargs = new Value*[argCount];
		for (i = 0; i < argCount; i++)
			variant_to_value (&args[i], &vargs[i]);
	}
	
	invoke (managed_scriptable, method->method_handle, vargs, argCount, &rv);
	
	if (argCount > 0) {
		for (i = 0; i < argCount; i++)
			delete vargs[i];
		
		delete [] vargs;
	}
	
	/* Note: this 1 is "void" */
	if (method->method_return_type != 1)
		value_to_variant (this, &rv, result);
	
	return true;
}


MoonlightScriptableObjectType::MoonlightScriptableObjectType ()
{
	allocate = moonlight_scriptable_object_allocate;
}

MoonlightScriptableObjectType *MoonlightScriptableObjectClass;

MoonlightScriptableObjectObject *
moonlight_scriptable_object_wrapper_create (PluginInstance *plugin, gpointer scriptable,
					    InvokeDelegate invoke_func,
					    SetPropertyDelegate setprop_func,
					    GetPropertyDelegate getprop_func,
					    EventHandlerDelegate addevent_func,
					    EventHandlerDelegate removeevent_func)

{
	MoonlightScriptControlObject *root_object = plugin->getRootObject ();

	MoonlightScriptableObjectObject *obj = (MoonlightScriptableObjectObject *)
		NPN_CreateObject (((MoonlightObject *) root_object)->instance,
				  MoonlightScriptableObjectClass);

	obj->managed_scriptable = scriptable;
	obj->invoke = invoke_func;
	obj->setprop = setprop_func;
	obj->getprop = getprop_func;
	obj->addevent = addevent_func;
	obj->removeevent = removeevent_func;
	
	ds(printf ("creating scriptable object wrapper => %p", obj));
	
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
	ds(printf ("adding property named %s to scriptable object %p", property_name, obj));
	
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
	ds(printf ("adding event named %s to scriptable object %p", event_name, obj));
	
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
	ds(printf ("adding method named %s (return type = %d) to scriptable object %p", method_name, method_return_type, obj));
	
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
	ds(printf ("registering scriptable object '%s' => %p", name, obj));
	
	MoonlightContentObject *content = (MoonlightContentObject *) plugin->getRootObject ()->content;
	
	g_hash_table_insert (content->registered_scriptable_objects, NPID (name), obj);
	
	ds(printf (" => done"));
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

	if (NPN_InvokeDefault (plugin->getNPP(), cb_obj, args, 2, &result))
		NPN_ReleaseVariantValue (&result);
}


/****************************** HtmlObject *************************/


class DomEventWrapper : public nsIDOMEventListener {

	NS_DECL_ISUPPORTS
	NS_DECL_NSIDOMEVENTLISTENER

 public:

	DomEventWrapper () {
		callback = NULL;

		NS_INIT_ISUPPORTS ();
	}

	callback_dom_event *callback;
	nsCOMPtr<nsIDOMEventTarget> target;
};

NS_IMPL_ISUPPORTS1(DomEventWrapper, nsIDOMEventListener)

NS_IMETHODIMP
DomEventWrapper::HandleEvent (nsIDOMEvent *aDOMEvent)
{
	int client_x, client_y, offset_x, offset_y, mouse_button;
	gboolean alt_key, ctrl_key, shift_key;
	nsString str_event;
	
	aDOMEvent->GetType (str_event);

	client_x = client_y = offset_x = offset_y = mouse_button = 0;
	alt_key = ctrl_key = shift_key = FALSE;

	nsCOMPtr<nsIDOMMouseEvent> mouse_event = do_QueryInterface (aDOMEvent);
	if (mouse_event != nsnull) {
		int screen_x, screen_y;

		mouse_event->GetScreenX (&screen_x);
		mouse_event->GetScreenY (&screen_y);

		mouse_event->GetClientX (&client_x);
		mouse_event->GetClientY (&client_y);

		offset_x = screen_x - client_x;
		offset_y = screen_y - client_y;

		mouse_event->GetAltKey (&alt_key);
		mouse_event->GetCtrlKey (&ctrl_key);
		mouse_event->GetShiftKey (&shift_key);

		PRUint16 umouse_button;
		mouse_event->GetButton (&umouse_button);
		mouse_button = umouse_button;
	}

	callback (strdup (NS_ConvertUTF16toUTF8 (str_event).get ()), client_x, client_y, offset_x, offset_y,
			alt_key, ctrl_key, shift_key, mouse_button);

	return NS_OK;
}

void
html_object_get_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *result)
{
	NPVariant npresult;
	NPObject *window = NULL;
	NPP npp = plugin->getInstance ();
	NPIdentifier identifier = NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	NPN_GetProperty (npp, npobj, identifier, &npresult);

	Value *res = NULL;
	if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
		variant_to_value (&npresult, &res);
		*result = *res;
	} else {
		*result = Value (Type::INVALID);
	}
}

void
html_object_set_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *value)
{
	NPVariant npvalue;
	NPObject *window = NULL;
	NPP npp = plugin->getInstance ();
	NPIdentifier identifier = NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	value_to_variant (npobj, value, &npvalue);

	NPN_SetProperty (npp, npobj, identifier, &npvalue);
}

void
html_object_invoke (PluginInstance *plugin, NPObject *npobj, char *name,
		Value *args, uint32_t arg_count, Value *result)
{
	NPVariant npresult;
	NPVariant *npargs = NULL;
	NPObject *window = NULL;
	NPP npp = plugin->getInstance ();
	NPIdentifier identifier = NPN_GetStringIdentifier (name);

	if (npobj == NULL) {
		NPN_GetValue (npp, NPNVWindowNPObject, &window);
		npobj = window;
	}

	if (arg_count) {
		npargs = new NPVariant [arg_count];
		for (uint32_t i = 0; i < arg_count; i++)
			value_to_variant (npobj, &args [i], &npargs [i]);
	}

	NPN_Invoke (npp, npobj, identifier, npargs, arg_count, &npresult);

	if (arg_count) {
		for (uint32_t i = 0; i < arg_count; i++)
			NPN_ReleaseVariantValue (&npargs [i]);
	}

	Value *res = NULL;
	if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
		variant_to_value (&npresult, &res);
		*result = *res;
	} else {
		*result = Value (Type::INVALID);
	}
}

static nsCOMPtr<nsIDOMDocument>
get_dom_document (NPP npp)
{
	nsCOMPtr<nsIDOMWindow> dom_window;
	NPN_GetValue (npp, NPNVDOMWindow, NS_STATIC_CAST(nsIDOMWindow **, getter_AddRefs(dom_window)));
	if (!dom_window) {
		d(printf ("No DOM window available\n"));
		return NULL;
	}

	nsCOMPtr<nsIDOMDocument> dom_document;
	dom_window->GetDocument (getter_AddRefs (dom_document));
	if (dom_document == nsnull) {
		d(printf ("No DOM document available\n"));
		return NULL;
	}

	return dom_document;
}

const char *
html_get_element_text (PluginInstance *plugin, const char *element_id)
{
	nsresult rv = NS_OK;
	NPP npp = plugin->getInstance ();

	nsCOMPtr<nsIDOMDocument> document;
	document = get_dom_document (npp);
	if (!document)
		return NULL;

	nsString ns_id = NS_ConvertUTF8toUTF16 (element_id, strlen (element_id));
	nsCOMPtr<nsIDOMElement> element;
	rv = document->GetElementById (ns_id, getter_AddRefs (element));
	if (NS_FAILED (rv) || element == NULL)
		return NULL;

	nsCOMPtr<nsIDOMDocument> owner;
	element->GetOwnerDocument (getter_AddRefs (owner));
	
	nsCOMPtr<nsIDOMDocumentRange> doc_range = do_QueryInterface (owner);
	if (!doc_range)
		return NULL;

	nsCOMPtr<nsIDOMRange> range;
	doc_range->CreateRange (getter_AddRefs (range));
	if (!range)
		return NULL;

	range->SelectNodeContents (element);

	nsString text;
	range->ToString (text);
	return g_strdup (NS_ConvertUTF16toUTF8 (text).get ());
}

gpointer
html_object_attach_event (PluginInstance *plugin, NPObject *npobj, char *name, callback_dom_event *cb)
{
	nsresult rv;
	NPVariant npresult;
	NPP npp = plugin->getInstance ();
	NPIdentifier id_identifier = NPN_GetStringIdentifier ("id");
	nsCOMPtr<nsISupports> item;

	NPN_GetProperty (npp, npobj, id_identifier, &npresult);

	if (NPVARIANT_IS_STRING (npresult) && strlen (STR_FROM_VARIANT (npresult)) > 0) {
		NPString np_id = NPVARIANT_TO_STRING (npresult);

		nsString ns_id = NS_ConvertUTF8toUTF16 (np_id.utf8characters, strlen (np_id.utf8characters));
		nsCOMPtr<nsIDOMDocument> dom_document = get_dom_document (npp);

		nsCOMPtr<nsIDOMElement> element;
		rv = dom_document->GetElementById (ns_id, getter_AddRefs (element));
		if (NS_FAILED (rv) || element == nsnull) {
			return NULL;
		}

		item = element;
	} else {
		NPObject *window = NULL;
		NPIdentifier document_identifier = NPN_GetStringIdentifier ("document");

		NPN_GetValue (npp, NPNVWindowNPObject, &window);

		if (npobj == window) {
			NPN_GetValue (npp, NPNVDOMWindow, NS_STATIC_CAST (nsISupports **, getter_AddRefs (item)));
		} else {
			NPVariant docresult;
			NPN_GetProperty (npp, window, document_identifier, &docresult);

			if (npobj == NPVARIANT_TO_OBJECT (docresult)) {
				item = get_dom_document (npp);
			} else {
				const char *temp_id = "__moonlight_temp_id";
				NPVariant npvalue;

				string_to_npvariant (temp_id, &npvalue);
				NPN_SetProperty (npp, npobj, id_identifier, &npvalue);
				NPN_ReleaseVariantValue (&npvalue);

				nsString ns_id = NS_ConvertUTF8toUTF16 (temp_id, strlen (temp_id));
				nsCOMPtr<nsIDOMDocument> dom_document = get_dom_document (npp);

				nsCOMPtr<nsIDOMElement> element;
				dom_document->GetElementById (ns_id, getter_AddRefs (element));
				if (element == nsnull) {
					d(printf ("Unable to find temp_id element\n"));
					return NULL;
				}
				
				item = element;
				
				// reset to it's original empty value
				NPN_SetProperty (npp, npobj, id_identifier, &npresult);
			}
		}
	}
	
	nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface (item);
	
	DomEventWrapper *wrapper = new DomEventWrapper ();
	wrapper->callback = cb;
	wrapper->target = target;
	
	rv = target->AddEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), wrapper, PR_TRUE);
	
	return wrapper;
}

void
html_object_detach_event (PluginInstance *plugin, const char *name, gpointer listener_ptr)
{
	DomEventWrapper *wrapper = (DomEventWrapper *) listener_ptr;

	wrapper->target->RemoveEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), wrapper, PR_TRUE);
}

void
html_object_release (PluginInstance *plugin, NPObject *npobj)
{
	NPN_ReleaseObject (npobj);
}

void
browser_do_alert (PluginInstance *plugin, char *msg)
{
	NPVariant npresult;
	NPVariant *npargs = new NPVariant [1];
	NPObject *window = NULL;
	NPP npp = plugin->getInstance ();
	NPIdentifier identifier = NPN_GetStringIdentifier ("alert");

	NPN_GetValue (npp, NPNVWindowNPObject, &window);
	string_to_npvariant (msg, &npargs [0]);

	NPN_Invoke (npp, window, identifier, npargs, 1, &npresult);
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
	dependency_object_classes [CONTROL_CLASS] = new MoonlightControlType ();
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
	/* Event Arg Types */
	dependency_object_classes [EVENT_ARGS_CLASS] = new MoonlightEventArgsType ();
	dependency_object_classes [ERROR_EVENT_ARGS_CLASS] = new MoonlightErrorEventArgsType ();
	dependency_object_classes [KEYBOARD_EVENT_ARGS_CLASS] = new MoonlightKeyboardEventArgsType ();
	dependency_object_classes [MARKER_REACHED_EVENT_ARGS_CLASS] = new MoonlightMarkerReachedEventArgsType ();
	dependency_object_classes [MOUSE_EVENT_ARGS_CLASS] = new MoonlightMouseEventArgsType ();
	
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
	delete MoonlightKeyboardEventArgsClass; MoonlightKeyboardEventArgsClass = NULL;
	delete MoonlightObjectClass; MoonlightObjectClass = NULL;
	delete MoonlightScriptableObjectClass; MoonlightScriptableObjectClass = NULL;
	delete MoonlightScriptControlClass; MoonlightScriptControlClass = NULL;
	delete MoonlightSettingsClass; MoonlightSettingsClass = NULL;
	delete MoonlightRectClass; MoonlightRectClass = NULL;
	delete MoonlightPointClass; MoonlightPointClass = NULL;
	delete MoonlightDurationClass; MoonlightDurationClass = NULL;
	delete MoonlightTimeSpanClass; MoonlightTimeSpanClass = NULL;
	delete MoonlightMarkerReachedEventArgsClass; MoonlightMarkerReachedEventArgsClass = NULL;
}
