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

#define DEBUG_SCRIPTABLE 0
#define DEBUG_JAVASCRIPT 1

#define COUNT(v)              (sizeof (v) / sizeof (v[0]))
#define LOOKUP_NAME(obj,n)     ((obj)->LookupName (n))
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
		MoonlightPoint *point = (MoonlightPoint*)NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightPointClass);
		point->point = *v->AsPoint ();

		OBJECT_TO_NPVARIANT (point, *result);
		break;
	}

	case Type::RECT: {
		MoonlightRect *rect = (MoonlightRect*)NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightRectClass);
		rect->rect = *v->AsRect ();

		OBJECT_TO_NPVARIANT (rect, *result);
		break;
	}
	case Type::DURATION: {
		MoonlightDuration *duration = (MoonlightDuration *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightDurationClass);
		duration->duration = *v->AsDuration ();

		OBJECT_TO_NPVARIANT (duration, *result);
		break;
	}

	case Type::TIMESPAN: {
		MoonlightTimeSpan *timespan = (MoonlightTimeSpan *) NPN_CreateObject (((MoonlightObject *) npobj)->instance, MoonlightTimeSpanClass);
		timespan->timespan = v->AsTimeSpan ();

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

	/* more builtins.. */
	default:
		if (v->GetKind () >= Type::DEPENDENCY_OBJECT) {
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
		*result = new Value ((gint32)NPVARIANT_TO_INT32(*v));
		break;
	case NPVariantType_Double:
		*result = new Value (NPVARIANT_TO_DOUBLE(*v));
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
		*result = new Value (Type::NPOBJ, NPVARIANT_TO_OBJECT (*v));
		NPN_RetainObject (NPVARIANT_TO_OBJECT (*v));
		break;
	}
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

// 	printf ("returning event listener proxy from %s - > %s\n", event_name, cb_name);
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
	
	if (is_func)
		NPN_ReleaseObject ((NPObject *) callback);
	else
		g_free (callback);
	
	g_free (event_name);
}
	
int
EventListenerProxy::AddHandler (EventObject *obj)
{
	target_object = obj;
	
	dtoken = obj->AddHandler (EventObject::DestroyedEvent, on_target_object_destroyed, this);
	
	event_id = obj->GetType()->LookupEvent (event_name);

	if (event_id == -1) {
		printf ("object of type `%s' does not provide an event named `%s'\n",
			obj->GetTypeName(), event_name);
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

EventArgsWrapper
EventListenerProxy::get_wrapper_for_event_name (const char *event_name)
{
	if (!g_ascii_strcasecmp ("mousemove", event_name) ||
	    !g_ascii_strcasecmp ("mouseleftbuttonup", event_name) ||
	    !g_ascii_strcasecmp ("mouseleftbuttondown", event_name) ||
	    !g_ascii_strcasecmp ("mouseenter", event_name)) {

		return mouse_event_wrapper;
	} else if (!g_ascii_strcasecmp ("keydown", event_name) || !g_ascii_strcasecmp ("keyup", event_name)) {
		return keyboard_event_wrapper;
	} else if (!g_ascii_strcasecmp ("markerreached", event_name)) {
		return timeline_marker_wrapper;
	} else
		return default_wrapper;
}

void
EventListenerProxy::timeline_marker_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	TimelineMarker *marker = (TimelineMarker *) calldata;
	MoonlightMarkerReachedEventArgsObject *obj = (MoonlightMarkerReachedEventArgsObject *) NPN_CreateObject (instance, MoonlightMarkerReachedEventArgsClass);
	obj->SetMarker (marker);

	OBJECT_TO_NPVARIANT (obj, *value);
}

void
EventListenerProxy::default_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	NULL_TO_NPVARIANT (*value);
}

void
EventListenerProxy::mouse_event_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	MouseEventArgs *ea = (MouseEventArgs *) calldata;
	MoonlightMouseEventArgsObject *jsea = (MoonlightMouseEventArgsObject *) NPN_CreateObject (instance, MoonlightMouseEventArgsClass);
	jsea->SetEventArgs (ea);

	OBJECT_TO_NPVARIANT (jsea, *value);
}

void
EventListenerProxy::keyboard_event_wrapper (NPP instance, gpointer calldata, NPVariant *value)
{
	KeyboardEventArgs *ea = (KeyboardEventArgs *) calldata;
	MoonlightKeyboardEventArgsObject *jsea = (MoonlightKeyboardEventArgsObject*)NPN_CreateObject (instance, MoonlightKeyboardEventArgsClass);
	KeyboardEventArgsPopulate (jsea, ea);

	OBJECT_TO_NPVARIANT (jsea, *value);
}

void
EventListenerProxy::on_target_object_destroyed (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy *) closure;
	
	proxy->target_object = NULL;
}

void
EventListenerProxy::proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventListenerProxy *proxy = (EventListenerProxy *) closure;
	EventObject *js_sender = sender;
	NPVariant args[2];
	NPVariant result;
	
	if (js_sender->GetObjectType () == Type::SURFACE) {
		// This is somewhat hackish, but is required for
		// the FullScreenChanged event (js expects the
		// sender to be the toplevel canvas, not the surface,
		// nor the content).
		js_sender = ((Surface*) js_sender)->GetToplevel ();
	}

	MoonlightEventObjectObject *depobj = EventObjectCreateWrapper (proxy->instance, js_sender);
	
	OBJECT_TO_NPVARIANT (depobj, args[0]);
	
	EventArgsWrapper event_args_wrapper = get_wrapper_for_event_name (proxy->event_name);
	
	//printf ("proxying event %s to javascript, sender = %p (%s)\n", proxy->event_name, sender, sender->GetTypeName ());
	
	event_args_wrapper (proxy->instance, calldata, &args[1]);
	
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
	NPN_ReleaseObject (NPVARIANT_TO_OBJECT (args [1]));
}


void
event_object_add_javascript_listener (EventObject *obj, PluginInstance *plugin, const char *event_name, const char *cb_name)
{
	EventListenerProxy *proxy = new EventListenerProxy (plugin->getNPP(), event_name, cb_name);
	proxy->AddHandler (obj);
}

/*** ErrorEventArgs ***/
static NPObject *
erroreventargs_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightErrorEventArgs (instance);
}

static const MoonNameIdMapping
erroreventargs_mapping[] = {
	{ "charnumber", MoonId_CharPosition },
	{ "errorcode", MoonId_ErrorCode },
	{ "errormessage", MoonId_ErrorMessage },
	{ "errortype", MoonId_ErrorType },
	{ "linenumber", MoonId_LineNumber },
	{ "methodname", MoonId_MethodName },
	{ "xamlfile", MoonId_XamlFile },
};

bool
MoonlightErrorEventArgs::GetProperty (int id, NPIdentifier, NPVariant *result)
{
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
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.lineNumber");
			INT32_TO_NPVARIANT (0, *result);
		}
		return true;

	case MoonId_CharPosition:
		if (args->error_type == ParserError) {
			INT32_TO_NPVARIANT (((ParserErrorEventArgs*)args)->char_position, *result);
		}
		else {
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
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("ErrorEventArgs.xamlFile");
			NULL_TO_NPVARIANT (*result);
		}
		return true;
	default:
		return false;
	}
}

MoonlightErrorEventArgsType::MoonlightErrorEventArgsType ()
{
	allocate = erroreventargs_allocate;

	AddMapping (erroreventargs_mapping, COUNT (erroreventargs_mapping));
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
MoonlightPoint::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	switch (id) {
	case MoonId_X:
		DOUBLE_TO_NPVARIANT (point.x, *result);
		return true;

	case MoonId_Y:
		DOUBLE_TO_NPVARIANT (point.y, *result);
		return true;

	default:
		return false;
	}
}

bool
MoonlightPoint::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	switch (id) {
	case MoonId_X:
		point.x = NPVARIANT_TO_DOUBLE (*value);
		return true;
	case MoonId_Y:
		point.y = NPVARIANT_TO_DOUBLE (*value);
		return true;
	default:
		return false;
	}
}

MoonlightPointType::MoonlightPointType ()
{
	allocate = point_allocate;

	AddMapping (point_mapping, COUNT (point_mapping));
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
MoonlightRect::GetProperty (int id, NPIdentifier, NPVariant *result)
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
		return false;
	}
}

bool
MoonlightRect::SetProperty (int id, NPIdentifier, const NPVariant *value)
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
		return false;
	}
}


MoonlightRectType::MoonlightRectType ()
{
	allocate = rect_allocate;

	AddMapping (rect_mapping, COUNT (rect_mapping));
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

bool
MoonlightDuration::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;

	case MoonId_Seconds:
		DOUBLE_TO_NPVARIANT (duration.ToSecondsFloat (), *result);
		return true;

	default:
		return false;
	}
}

bool
MoonlightDuration::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Seconds:
		duration = Duration::FromSecondsFloat (NPVARIANT_TO_DOUBLE (*value));
		return true;

	default:
		return false;
	}
}


MoonlightDurationType::MoonlightDurationType ()
{
	allocate = duration_allocate;

	AddMapping (duration_mapping, COUNT (duration_mapping));
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

bool
MoonlightTimeSpan::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	switch (id) {
	case MoonId_Name:
		string_to_npvariant ("", result);
		return true;
	case MoonId_Seconds:
		DOUBLE_TO_NPVARIANT (TimeSpan_ToSecondsFloat (timespan), *result);
		return true;
	default:
		return false;
	}
}

bool
MoonlightTimeSpan::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	switch (id) {
	case MoonId_Name:
		return true;

	case MoonId_Seconds:
		if (NPVARIANT_IS_INT32 (*value)) {
			timespan = TimeSpan_FromSecondsFloat (NPVARIANT_TO_INT32 (*value));
		} else if (NPVARIANT_IS_DOUBLE (*value)) {
			timespan = TimeSpan_FromSecondsFloat (NPVARIANT_TO_DOUBLE (*value));
		} else {
			return false;
		}
		return true;

	default:
		return false;
	}
}


MoonlightTimeSpanType::MoonlightTimeSpanType ()
{
	allocate = timespan_allocate;

	AddMapping (timespan_mapping, COUNT (timespan_mapping));
}

MoonlightTimeSpanType *MoonlightTimeSpanClass;



/*** MoonlightMouseEventArgsClass  **************************************************************/

static NPObject *
mouse_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMouseEventArgsObject (instance);
}

void
MoonlightMouseEventArgsObject::SetEventArgs (MouseEventArgs *args)
{
	event_args = args;
	event_args->ref ();
}

void
MoonlightMouseEventArgsObject::Dispose ()
{
	MoonlightObject::Dispose ();

	if (event_args) {
		event_args->unref();
		event_args = NULL;
	}
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
MoonlightMouseEventArgsObject::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	int state = event_args->GetState ();

	switch (id) {
	case MoonId_Shift:
		BOOLEAN_TO_NPVARIANT (state & GDK_SHIFT_MASK != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT (state & GDK_CONTROL_MASK != 0, *result);
		return true;

	default:
		return false;
	}
}

bool
MoonlightMouseEventArgsObject::Invoke (int id, NPIdentifier name,
				       const NPVariant *args, uint32_t argCount,
				       NPVariant *result)
{
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

		if (NPVARIANT_IS_OBJECT (args [0])) {
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

		if (NPVARIANT_IS_OBJECT (args [0])) {
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
		return false;
	}
}


MoonlightMouseEventArgsType::MoonlightMouseEventArgsType ()
{
	allocate = mouse_event_allocate;

	AddMapping (mouse_event_mapping, COUNT (mouse_event_mapping));
}

MoonlightMouseEventArgsType *MoonlightMouseEventArgsClass;


/*** MoonlightMarkerReachedEventArgsClass  **************************************************************/

static NPObject *
marker_reached_event_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightMarkerReachedEventArgsObject (instance);
}

void
MoonlightMarkerReachedEventArgsObject::Dispose ()
{
	MoonlightObject::Dispose ();

	if (marker) {
		marker->unref ();
		marker = NULL;
	}
}

static const MoonNameIdMapping
marker_reached_event_mapping[] = {
	{ "marker", MoonId_Marker },
	{ "tostring", MoonId_ToString }
};

bool
MoonlightMarkerReachedEventArgsObject::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	switch (id) {
	case MoonId_Marker: {
		MoonlightEventObjectObject *meoo = EventObjectCreateWrapper (instance, marker);
		OBJECT_TO_NPVARIANT (meoo, *result);
		return true;
	}
	default:
		return false;
	}
}

bool
MoonlightMarkerReachedEventArgsObject::Invoke (int id, NPIdentifier name,
		     const NPVariant *args, uint32_t argCount, NPVariant *result)
 {
 	switch (id) {
	case MoonId_ToString: {
		string_to_npvariant ("TimelineMarkerEventArgs", result);
		return true;
	}
	default:
		return false;
 	}
 }
MoonlightMarkerReachedEventArgsType::MoonlightMarkerReachedEventArgsType ()
{
	allocate = marker_reached_event_allocate;

	AddMapping (marker_reached_event_mapping, COUNT (marker_reached_event_mapping));
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
MoonlightKeyboardEventArgsObject::GetProperty (int id, NPIdentifier, NPVariant *result)
{
	switch (id) {
	case MoonId_Shift:
		BOOLEAN_TO_NPVARIANT (state & GDK_SHIFT_MASK != 0, *result);
		return true;

	case MoonId_Ctrl:
		BOOLEAN_TO_NPVARIANT (state & GDK_CONTROL_MASK != 0, *result);
		return true;

	case MoonId_Key:
		INT32_TO_NPVARIANT (key, *result);
		return true;

	case MoonId_PlatformKeyCode:
		INT32_TO_NPVARIANT (platformcode, *result);
		return true;

	default:
		return false;
	}
}

MoonlightKeyboardEventArgsType::MoonlightKeyboardEventArgsType ()
{
	allocate = keyboard_event_allocate;

	AddMapping (keyboard_event_mapping, COUNT (keyboard_event_mapping));
}

MoonlightKeyboardEventArgsType *MoonlightKeyboardEventArgsClass;

void
KeyboardEventArgsPopulate (MoonlightKeyboardEventArgsObject *ea, KeyboardEventArgs *args)
{
	ea->state = args->state;
	ea->platformcode = args->platformcode;
	ea->key = args->key;
}


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
	if (!obj->disposed)
		obj->Dispose ();
	delete obj;
}

MoonlightObject::~MoonlightObject ()
{
	if (!disposed) {
		// Can't call Dispose from here, since the derived classes' vtbls have already
		// been destroyed.
		printf ("Undisposed object.\n");
		// print_stack_trace ();
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
MoonlightObject::Dispose ()
{
	disposed = true;
	g_hash_table_destroy (event_listener_proxies);
}

bool
MoonlightObject::HasProperty (NPIdentifier name)
{
	return IS_PROPERTY (LookupName (name));
}

bool
MoonlightObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	return false;
}

bool
MoonlightObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	return false;
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
	obj->Dispose ();
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

	mapping = NULL;
	mapping_count = 0;

	last_lookup = NULL;
	last_id = 0;
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
	if (last_lookup == name) {
//  		printf ("%p:  fast %p => %d\n", this, name, last_id);
		return last_id;
	}

//  	printf ("%p: slow %p\n", this, name);


	int id = map_name_to_id (name, mapping, mapping_count);

	if (id) {
		/* only cache hits */
		last_lookup = name;
		last_id = id;
	}

// 	printf (" => %d\n", id);

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
	{ "isversionsupported", MoonId_IsVersionSupported },
	{ "settings", MoonId_Settings },
	{ "source", MoonId_Source },
};

void
MoonlightScriptControlObject::Dispose ()
{
	MoonlightObject::Dispose ();

 	if (settings)
 		NPN_ReleaseObject (settings);
	settings = NULL;

 	if (content)
 		NPN_ReleaseObject (content);
	content = NULL;
}

bool
MoonlightScriptControlObject::GetProperty (int id, NPIdentifier, NPVariant *result)
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

	default:
		return false;
	}
}

bool
MoonlightScriptControlObject::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_Source:
		plugin->setSource (STR_FROM_VARIANT (*value));
		return true;
	default:
		return false;
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
		/* we support all 1.0.* and 1.1.* versions. */
		BOOLEAN_TO_NPVARIANT (false, *result);
		if (argCount != 1 || !NPVARIANT_IS_STRING(args[0])) {
			printf ("invalid arg types\n");
			return true;
		}

		char *v = STR_FROM_VARIANT (args [0]);
		printf ("version requested = %s\n",v);

		gchar** versions = g_strsplit (v, ".", 3);

		if (versions[0] == NULL || versions[1] == NULL) {
			g_strfreev (versions);
			return true;
		}

		if (/* we advertise support for 0.9x, although this is probably a bad idea.. */
		    (!strcmp (versions[0], "0") &&
		     (versions[1][0] == '9' ||
		      versions[1][0] == '8'))

		    /* and we should work with any 1.0.* and 1.1.* instance */
		    || (!strcmp (versions[0], "1") &&
			(!strcmp (versions[1], "0")
			/* If we don't include the mono runtime, we won't work with 1.1.* instances */
#if INCLUDE_MONO_RUNTIME
			 || !strcmp (versions[1], "1")
#endif
			 ))
		   ){

			BOOLEAN_TO_NPVARIANT (true, *result);
		}

		g_strfreev (versions);

		return true;
	}

	default:
		return false;
	}
}

MoonlightScriptControlType::MoonlightScriptControlType ()
{
	allocate = moonlight_scriptable_control_allocate;

	AddMapping (scriptable_control_mapping, COUNT (scriptable_control_mapping));
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
MoonlightSettingsObject::GetProperty (int id, NPIdentifier, NPVariant *result)
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
		return false;
	}
}

bool
MoonlightSettingsObject::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {

	case MoonId_Background:
		plugin->setBackground (STR_FROM_VARIANT (*value));
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
		DEBUG_WARN_NOTIMPLEMENTED ("maxFrameRate property");
		return true;

	// Cant be set after initialization so return true
	case MoonId_Windowless:
		return true;
	}

	return false;
}

MoonlightSettingsType::MoonlightSettingsType ()
{
	allocate = moonlight_settings_allocate;
	AddMapping (moonlight_settings_mapping, COUNT (moonlight_settings_mapping));
}

MoonlightSettingsType *MoonlightSettingsClass;


/*** MoonlightContentClass ************************************************************/
static NPObject *
moonlight_content_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightContentObject (instance);
}

void
MoonlightContentObject::Dispose ()
{
	MoonlightObject::Dispose ();

	/* XXX free the registered_scriptable_objects hash */
	//DEBUG_WARN_NOTIMPLEMENTED ("need to free registered scriptable objects");
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

	// XXX this is still case sensitive (uses a direct hash on the NPIdentifier)
	gpointer p = g_hash_table_lookup (registered_scriptable_objects,
					  name);
	return p != NULL;
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
		MoonlightScriptableObjectObject *obj =
			(MoonlightScriptableObjectObject*)g_hash_table_lookup (registered_scriptable_objects, name);
		if (obj == NULL)
			return false;

		NPN_RetainObject (obj);
		OBJECT_TO_NPVARIANT (obj, *result);
		return true;
	}
	default:
		return false;
	}
}

bool
MoonlightContentObject::SetProperty (int id, NPIdentifier, const NPVariant *value)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	switch (id) {
	case MoonId_FullScreen:
		plugin->surface->SetFullScreen (NPVARIANT_TO_BOOLEAN (*value));
		return true;

	case MoonId_OnResize:
	case MoonId_OnFullScreenChange:
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
		return false;
	}
}

bool
MoonlightContentObject::Invoke (int id, NPIdentifier name,
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;
	
	switch (id) {
	case MoonId_FindName: {
		if (!argCount)
			THROW_JS_EXCEPTION ("findName");

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

		char *xaml = STR_FROM_VARIANT (args [0]);
		bool create_namescope = argCount >= 2 ? NPVARIANT_TO_BOOLEAN (args[1]) : false;
		
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

		char *fname = down->GetResponseFile (STR_FROM_VARIANT (args [1]));
		if (fname != NULL) {
			XamlLoader* loader = PluginXamlLoader::FromFilename (fname, plugin, plugin->surface);
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

	default:
		return false;
	}
}

MoonlightContentType::MoonlightContentType ()
{
	allocate = moonlight_content_allocate;

	AddMapping (moonlight_content_mapping, COUNT (moonlight_content_mapping));
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
	{ "tostring", MoonId_ToString }
};

static NPObject *
moonlight_dependency_object_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightDependencyObjectObject (instance);
}

void
MoonlightDependencyObjectObject::Dispose ()
{
	MoonlightEventObjectObject::Dispose ();
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
_set_dependency_property_value (DependencyObject *dob, DependencyProperty *p, const NPVariant *value)
{
	if (NPVARIANT_IS_OBJECT (*value)){
		MoonlightObject *obj = (MoonlightObject *) NPVARIANT_TO_OBJECT (*value);

		if (obj->moonlight_type >= Type::DEPENDENCY_OBJECT || obj->moonlight_type == Type::INVALID) {
			MoonlightDependencyObjectObject *depobj = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (*value);
			dob->SetValue (p, Value(depobj->GetDependencyObject ()));

			return true;
		}

		switch (obj->moonlight_type) {
		case Type::TIMESPAN: {
			MoonlightTimeSpan *ts = (MoonlightTimeSpan*) obj;
			dob->SetValue (p, Value(ts->timespan, Type::TIMESPAN));
			break;
		}
		case Type::DURATION: {
			MoonlightDuration *duration = (MoonlightDuration*) obj;
			dob->SetValue (p, Value(duration->duration));
			break;
		}
		case Type::RECT: {
			MoonlightRect *rect = (MoonlightRect*) obj;
			dob->SetValue (p, Value(rect->rect));
			break;
		}
		case Type::POINT: {
			MoonlightPoint *point = (MoonlightPoint*) obj;
			dob->SetValue (p, Value(point->point));
			break;
		}
		default:
			//printf ("unhandled object type %i - %s in do.set_property\n", obj->moonlight_type, Type::Find (obj->moonlight_type)->name);
			DEBUG_WARN_NOTIMPLEMENTED ("unhandled object type in do.set_property");
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
			if (p->value_type >= Type::DEPENDENCY_OBJECT) {
				DependencyObject *val = NULL;
				
				dob->SetValue (p, Value (val));
			} else if (p->value_type == Type::STRING) {
				char *val = NULL;
				
				dob->SetValue (p, Value (val));
			} else {
				DEBUGWARN ("Setting NULL for unsupported type (%s::%s)", dob->GetTypeName (), p->name);
			}
			
			return true;
		} else if (NPVARIANT_IS_VOID (*value)) {
			DEBUGWARN ("unhandled variant type VOID in do.set_property for (%s::%s)", dob->GetTypeName (), p->name);
			return true;
		} else {
			DEBUGWARN ("unhandled variant type in do.set_property for (%s::%s)", dob->GetTypeName (), p->name);
			return true;
		}
		
		g_assert (strval != NULL);
		
		xaml_set_property_from_str (dob, p, strval);
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
	if (!strname)
		return false;

	DependencyObject *dob = GetDependencyObject ();

	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	if (p) {
		Value *value = dob->GetValue (p);
		if (!value) {
			// strings aren't null, they seem to just be empty strings
			if (p->value_type == Type::STRING) {
				string_to_npvariant ("", result);
				return true;
			}

			NULL_TO_NPVARIANT (*result);
			return true;
		}

		value_to_variant (this, value, result);

		return true;
	}

	// it wasn't a dependency property.  let's see if it's an
	// event, and hook it up if it is valid on this object.
	const char *event_name = map_moon_id_to_event_name (id);
	int event_id = dob->GetType()->LookupEvent (event_name);
	if (event_id != -1) {
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

	return false;
}

bool
MoonlightDependencyObjectObject::SetProperty (int id, NPIdentifier name, const NPVariant *value)
{
	DependencyObject *dob = GetDependencyObject ();

	// don't need to downcase here since dependency property lookup is already case insensitive
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DependencyProperty *p = _get_dependency_property (dob, strname);
	NPN_MemFree (strname);

	if (p)
		return _set_dependency_property_value (dob, p, value);

	// turns out that on Silverlight you can't set regular events as properties.
#if false
	// it wasn't a dependency property.  let's see if it's an
	// event
	const char *event_name = map_moon_id_to_event_name (id);
	if (event_name != NULL) {
		int event_id = dob->GetType()->LookupEvent(event_name);
		if (event_id != -1) {
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
	return false;
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
		printf ("JS message: %s\n", STR_FROM_VARIANT (args [0]));
		VOID_TO_NPVARIANT (*result);
		return true;
#endif

	case MoonId_Equals: {
		if (argCount != 1 || !NPVARIANT_IS_OBJECT (args[0]))
			THROW_JS_EXCEPTION ("equals");

		NPObject *o = NPVARIANT_TO_OBJECT (args[0]);
		if (o->_class != MoonlightDependencyObjectClass)
			BOOLEAN_TO_NPVARIANT (false, *result);
		else {
			MoonlightDependencyObjectObject *obj = (MoonlightDependencyObjectObject *) o;
			BOOLEAN_TO_NPVARIANT (obj->GetDependencyObject() == dob, *result);
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
		if (!argCount)
			THROW_JS_EXCEPTION ("findName");

		char *name = STR_FROM_VARIANT (args [0]);

		DependencyObject *element = dob->FindName (name);
		if (!element) {
			NULL_TO_NPVARIANT (*result);
			return true;
		}

		OBJECT_TO_NPVARIANT (EventObjectCreateWrapper (instance, element), *result);
		return true;
	}

	case MoonId_ToString:
		string_to_npvariant (dob->GetTypeName(), result);
		return true;

	case MoonId_GetHost: {
		PluginInstance *plugin = (PluginInstance*) instance->pdata;

		OBJECT_TO_NPVARIANT ((NPObject *) plugin->getRootObject (), *result);

		return true;
	}

	case MoonId_GetParent: {
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
			/* XXX how do we check if args[1] is a function? */
			THROW_JS_EXCEPTION ("addEventListener");
		}

		char *name = g_strdup (STR_FROM_VARIANT (args [0]));

		name[0] = toupper(name[0]);

		EventListenerProxy *proxy = new EventListenerProxy (instance,
								    name,
								    &args[1]);

		int token = proxy->AddHandler (dob);

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
		
		if (NPVARIANT_IS_INT32 (args [1]))
			dob->RemoveHandler (STR_FROM_VARIANT (args[0]), NPVARIANT_TO_INT32 (args[1]));
		else
			g_warning ("removeEventListener for named callbacks is not yet implemented");

		return true;
	}

	// XXX these next two methods should live in a UIElement
	// wrapper class, not in the DependencyObject wrapper.
	case MoonId_CaptureMouse:
		BOOLEAN_TO_NPVARIANT (((UIElement*)dob)->CaptureMouse (), *result);
		return true;

	case MoonId_ReleaseMouseCapture:
		((UIElement*)dob)->ReleaseMouseCapture ();

		VOID_TO_NPVARIANT (*result);
		return true;
	
	default:
		return MoonlightObject::Invoke (id, name,
						args, argCount,
						result);
	}
}


MoonlightDependencyObjectType::MoonlightDependencyObjectType ()
{
	allocate = moonlight_dependency_object_allocate;

	AddMapping (moonlight_dependency_object_mapping, COUNT (moonlight_dependency_object_mapping));
}

MoonlightDependencyObjectType *MoonlightDependencyObjectClass;


/*** MoonlightEventObjectClass ***************************************************/

static NPObject *
moonlight_event_object_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightEventObjectObject (instance);
}

void
MoonlightEventObjectObject::Dispose ()
{
	MoonlightObject::Dispose ();

	if (eo) {
		PluginInstance *plugin = (PluginInstance*) instance->pdata;
		if (plugin != NULL) {
			plugin->RemoveWrappedObject (eo);
		}
		eo->unref ();
	}
	eo = NULL;
}

void
MoonlightEventObjectObject::SetEventObject (EventObject *eventobject)
{
	PluginInstance *plugin = (PluginInstance*) instance->pdata;

	// we shouldn't need to worry about this case, right?  we create
	// wrapper objects whenever we need them, and don't reuse them...
	if (eo) {
		plugin->RemoveWrappedObject (eo);
		eo->unref ();
	}

	eo = eventobject;

	if (eo) {
		plugin->AddWrappedObject (eo, this);
		eo->ref ();
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
	PluginInstance *plugin = (PluginInstance*) instance->pdata;
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
	case Type::IMAGEBRUSH:
		np_class = MoonlightImageBrushClass;
		break;
	case Type::TEXTBLOCK:
		np_class = MoonlightTextBlockClass;
		break;
	case Type::EVENTOBJECT:
	case Type::SURFACE:
		np_class = MoonlightEventObjectClass;
		break;
	case Type::STYLUSINFO:
		np_class = MoonlightStylusInfoClass;
		break;
	case Type::STYLUSPOINT_COLLECTION:
		np_class = MoonlightStylusPointCollectionClass;
		break;
	case Type::STROKE_COLLECTION:
		np_class = MoonlightStrokeCollectionClass;
		break;
	case Type::STROKE:
		np_class = MoonlightStrokeClass;
		break;
	default:
		if (Type::Find (kind)->IsSubclassOf (Type::COLLECTION))
			np_class = MoonlightCollectionClass;
		else
			np_class = MoonlightDependencyObjectClass;
	}

	depobj = (MoonlightEventObjectObject *) NPN_CreateObject (instance, np_class);

	depobj->SetEventObject (obj);

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
		if (argCount < 1)
			THROW_JS_EXCEPTION ("add");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);
		col->Add (el->GetDependencyObject ());
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}
	case MoonId_Remove: {
		if (argCount < 1)
			THROW_JS_EXCEPTION ("remove");
		
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT (args[0]);
		col->Remove (el->GetDependencyObject ());
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}
	case MoonId_RemoveAt: {
		if (argCount < 1)
			THROW_JS_EXCEPTION ("removeAt");
		
		int index = NPVARIANT_TO_INT32 (args [0]);
		col->RemoveAt (index);
		
		VOID_TO_NPVARIANT (*result);
		
		return true;
	}
	case MoonId_Insert: {
		if (argCount < 2)
			THROW_JS_EXCEPTION ("insert");
		
		int index = NPVARIANT_TO_INT32 (args[0]);
		MoonlightDependencyObjectObject *el = (MoonlightDependencyObjectObject*) NPVARIANT_TO_OBJECT (args[1]);
		
		col->Insert (index, el->GetDependencyObject ());
		
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
		if (argCount < 1)
			THROW_JS_EXCEPTION ("getItem");
		
		int index = NPVARIANT_TO_INT32 (args[0]);
		
		if (index < 0 || index >= col->list->Length ())
			THROW_JS_EXCEPTION ("getItem");
		
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
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}


MoonlightCollectionType::MoonlightCollectionType ()
{
	AddMapping (moonlight_collection_mapping, COUNT (moonlight_collection_mapping));

	allocate = moonlight_collection_allocate;
}

MoonlightCollectionType *MoonlightCollectionClass;


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
		// XXX JS doesn't have 64bit ints?
		if (argCount != 1 || !NPVARIANT_IS_INT32 (args[0]))
			THROW_JS_EXCEPTION ("seek");

		TimeSpan ts = (TimeSpan)NPVARIANT_TO_INT32(args[0]);
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
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightStoryboardType::MoonlightStoryboardType ()
{
	AddMapping (moonlight_storyboard_mapping, COUNT (moonlight_storyboard_mapping));

	allocate = moonlight_storyboard_allocate;
}

MoonlightStoryboardType *MoonlightStoryboardClass;

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
		if (argCount != 2
		    || !NPVARIANT_IS_OBJECT (args[0])
		    || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("setSource");

		DependencyObject *downloader = ((MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT(args[0]))->GetDependencyObject ();
		const char *part = STR_FROM_VARIANT (args [1]);

		media->SetSource (downloader, part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightMediaElementType::MoonlightMediaElementType ()
{
	AddMapping (moonlight_media_element_mapping, COUNT (moonlight_media_element_mapping));

	allocate = moonlight_media_element_allocate;
}

MoonlightMediaElementType *MoonlightMediaElementClass;

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
	Image *img = (Image*)GetDependencyObject ();

	switch (id) {

	case MoonId_SetSource: {
		if (argCount != 2
		    || !NPVARIANT_IS_OBJECT (args[0])
		    || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("setSource");

		DependencyObject *downloader = ((MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT(args[0]))->GetDependencyObject ();
		const char *part = STR_FROM_VARIANT (args [1]);

		img->SetSource (downloader, part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightImageType::MoonlightImageType ()
{
	AddMapping (moonlight_image_mapping, COUNT (moonlight_image_mapping));

	allocate = moonlight_image_allocate;
}

MoonlightImageType *MoonlightImageClass;

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
	ImageBrush *img = (ImageBrush*)GetDependencyObject ();

	switch (id) {

	case MoonId_SetSource: {
		if (argCount != 2
		    || !NPVARIANT_IS_OBJECT (args[0])
		    || !NPVARIANT_IS_STRING (args[1]))
			THROW_JS_EXCEPTION ("setSource");

		DependencyObject *downloader = ((MoonlightDependencyObjectObject*)NPVARIANT_TO_OBJECT(args[0]))->GetDependencyObject ();
		const char *part = STR_FROM_VARIANT (args [1]);

		img->SetSource (downloader, part);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightImageBrushType::MoonlightImageBrushType ()
{
	AddMapping (moonlight_image_brush_mapping, COUNT (moonlight_image_brush_mapping));

	allocate = moonlight_image_brush_allocate;
}

MoonlightImageBrushType *MoonlightImageBrushClass;


/*** MoonlightTextBlockClass ***************************************************/

static NPObject *
moonlight_text_block_allocate (NPP instance, NPClass *klass)
{
	return new MoonlightTextBlockObject (instance);
}


static const MoonNameIdMapping
moonlight_text_block_mapping [] = {
	{ "setfontsource", MoonId_SetFontSource }
};


bool
MoonlightTextBlockObject::Invoke (int id, NPIdentifier name,
				  const NPVariant *args, uint32_t argCount,
				  NPVariant *result)
{
	TextBlock *tb = (TextBlock *) GetDependencyObject ();

	switch (id) {
	case MoonId_SetFontSource: {
		if (argCount != 1 || !NPVARIANT_IS_OBJECT (args[0]))
			THROW_JS_EXCEPTION ("setFontSource");

		DependencyObject *downloader = ((MoonlightDependencyObjectObject *) NPVARIANT_TO_OBJECT(args[0]))->GetDependencyObject ();

		tb->SetFontSource (downloader);

		VOID_TO_NPVARIANT (*result);

		return true;
	}
	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightTextBlockType::MoonlightTextBlockType ()
{
	AddMapping (moonlight_text_block_mapping, COUNT (moonlight_text_block_mapping));

	allocate = moonlight_text_block_allocate;
}

MoonlightTextBlockType *MoonlightTextBlockClass;

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
	AddMapping (moonlight_stylus_info_mapping, COUNT (moonlight_stylus_info_mapping));

	allocate = moonlight_stylus_info_allocate;
}

MoonlightStylusInfoType *MoonlightStylusInfoClass;

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
	AddMapping (moonlight_stylus_point_collection_mapping, COUNT (moonlight_stylus_point_collection_mapping));

	allocate = moonlight_stylus_point_collection_allocate;
}

MoonlightStylusPointCollectionType *MoonlightStylusPointCollectionClass;

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
		if (argCount < 1)
			THROW_JS_EXCEPTION ("hitTest");

		if (!NPVARIANT_IS_OBJECT (args [0]))
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
	AddMapping (moonlight_stroke_collection_mapping, COUNT (moonlight_stroke_collection_mapping));

	allocate = moonlight_stroke_collection_allocate;
}

MoonlightStrokeCollectionType *MoonlightStrokeCollectionClass;

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
		if (argCount < 1)
			THROW_JS_EXCEPTION ("hitTest");

		if (!NPVARIANT_IS_OBJECT (args [0]))
			THROW_JS_EXCEPTION ("hitTest");

		DependencyObject *dob = DEPENDENCY_OBJECT_FROM_VARIANT (args [0]);
		if (!dob->Is (Type::STYLUSPOINT_COLLECTION))
			THROW_JS_EXCEPTION ("hitTest");

		BOOLEAN_TO_NPVARIANT (stroke->HitTest ((StylusPointCollection*)dob), *result);
		return true;
	}

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightStrokeType::MoonlightStrokeType ()
{
	AddMapping (moonlight_stroke_mapping, COUNT (moonlight_stroke_mapping));

	allocate = moonlight_stroke_allocate;
}

MoonlightStrokeType *MoonlightStrokeClass;

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
	{ "responsetext", MoonId_ResponseText },
	{ "send", MoonId_Send }
};

bool
MoonlightDownloaderObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	Downloader *dl = (Downloader*)GetDependencyObject ();

	switch (id) {
	case MoonId_ResponseText: {
		uint64_t size;
		char* buf = (char*)downloader_get_response_text (dl, NULL, &size);

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

	default:
		return MoonlightDependencyObjectObject::GetProperty (id, name, result);
	}
}

bool
MoonlightDownloaderObject::Invoke (int id, NPIdentifier name,
				   const NPVariant *args, uint32_t argCount,
				   NPVariant *result)
{
	Downloader *dl = (Downloader*)GetDependencyObject ();

	switch (id) {

	case MoonId_Abort:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("abort");

		downloader_abort (dl);

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_Open: {
		if (argCount > 3)
			THROW_JS_EXCEPTION ("open");

		char *verb = STR_FROM_VARIANT (args [0]);
		char *uri = STR_FROM_VARIANT (args [1]);

		downloader_open (dl, verb, uri);

		VOID_TO_NPVARIANT (*result);

		return true;
	}

	case MoonId_Send:
		if (argCount != 0)
			THROW_JS_EXCEPTION ("send");

		downloader_send (dl);

		VOID_TO_NPVARIANT (*result);

		return true;

	case MoonId_GetResponseText: {
		if (argCount != 1)
			THROW_JS_EXCEPTION ("getResponseText");

		char *part_name = STR_FROM_VARIANT (args [0]);

		uint64_t size;
		char* buf = (char*) downloader_get_response_text (dl, part_name, &size);

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

	default:
		return MoonlightDependencyObjectObject::Invoke (id, name,
								args, argCount,
								result);
	}
}

MoonlightDownloaderType::MoonlightDownloaderType ()
{
	AddMapping (moonlight_downloader_mapping, COUNT (moonlight_downloader_mapping));

	allocate = moonlight_downloader_allocate;
}

MoonlightDownloaderType *MoonlightDownloaderClass;

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

	return MoonlightDependencyObjectObject::HasProperty (name);
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

	value_to_variant (this, value, result);

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

	return _set_dependency_property_value (dob, p, value);
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
	return NPN_Invoke (instance, real_object,
			   name, args, argCount, result);
}

MoonlightControlType::MoonlightControlType ()
{
	allocate = moonlight_control_allocate;
}

MoonlightControlType *MoonlightControlClass;


/*** MoonlightScriptableObjectClass ***************************************************/

// XXX the property/method hashes here are case sensitive

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

void
MoonlightScriptableObjectObject::Dispose ()
{
	MoonlightObject::Dispose ();

	if (managed_scriptable) {
		// XXX unref the scriptable object however we need to.
	}
	managed_scriptable = NULL;

	// XXX free the properties, events, and methods hashes.
}

bool
MoonlightScriptableObjectObject::HasProperty (NPIdentifier name)
{
	return (g_hash_table_lookup (properties, name) != NULL
		|| g_hash_table_lookup (events, name));
}

bool
MoonlightScriptableObjectObject::GetProperty (int id, NPIdentifier name, NPVariant *result)
{
	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (properties, name);
	if (!prop)
		return false;

#if DEBUG_SCRIPTABLE
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("***************** getting scriptable object property %s", strname);
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
	// first we try the property hash
	ScriptableProperty *prop = (ScriptableProperty*)g_hash_table_lookup (properties, name);
	if (prop) {
#if DEBUG_SCRIPTABLE
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		DEBUGMSG ("***************** setting scriptable object property %s", strname);
		NPN_MemFree (strname);
#endif

		Value *v;

		variant_to_value (value, &v);

		setprop (managed_scriptable, prop->property_handle, v);

		delete v;

		return true;
	}
	// if that fails, look for the event of that name
	ScriptableEvent *event = (ScriptableEvent*)g_hash_table_lookup (events, name);
	if (event) {
#if DEBUG_SCRIPTABLE
		NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
		DEBUGMSG ("***************** adding scriptable object event %s", strname);
		NPN_MemFree (strname);
#endif

		if (NPVARIANT_IS_OBJECT (*value)) {
			NPObject *cb_obj = NPVARIANT_TO_OBJECT (*value);

			NPN_RetainObject (cb_obj);

			addevent (managed_scriptable, event->event_handle, this, cb_obj);
		}
		else {
			DEBUG_WARN_NOTIMPLEMENTED ("scriptableobject.register_event (non-object)");
		}
		return true;
	}

	return false;
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
	if (!method)
		return false;

#if DEBUG_SCRIPTABLE
	NPUTF8 *strname = NPN_UTF8FromIdentifier (name);
	DEBUGMSG ("***************** invoking scriptable object method %s", strname);
	NPN_MemFree (strname);
#endif

	Value rv;

	Value** vargs = NULL;

	if (argCount > 0) {
		vargs = new Value*[argCount];
		for (uint32_t i = 0; i < argCount; i ++) {
			variant_to_value (&args[i], &vargs[i]);
		}
	}

	invoke (managed_scriptable, method->method_handle, vargs, argCount, &rv);

	if (argCount > 0) {
		for (uint32_t i = 0; i < argCount; i ++) {
			delete vargs[i];
		}
		delete [] vargs;
	}

	if (method->method_return_type != 1 /* XXX this 1 is "void" */) {
		value_to_variant (this, &rv, result);
	}
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

#if DEBUG_SCRIPTABLE
	DEBUGMSG ("creating scriptable object wrapper => %p", obj);
#endif
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
#if DEBUG_SCRIPTABLE
	DEBUGMSG ("adding property named %s to scriptable object %p", property_name, obj);
#endif

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
#if DEBUG_SCRIPTABLE
	DEBUGMSG ("adding event named %s to scriptable object %p", event_name, obj);
#endif

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
#if DEBUG_SCRIPTABLE
	DEBUGMSG ("adding method named %s (return type = %d) to scriptable object %p", method_name, method_return_type, obj);
#endif

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
#if DEBUG_SCRIPTABLE
	DEBUGMSG ("registering scriptable object '%s' => %p", name, obj);
#endif

	MoonlightContentObject *content = (MoonlightContentObject *) plugin->getRootObject ()->content;

	g_hash_table_insert (content->registered_scriptable_objects,
			     NPID (name),
			     obj);

#if DEBUG_SCRIPTABLE
	DEBUGMSG (" => done");
#endif
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
		DEBUGMSG ("No DOM window available\n");
		return NULL;
	}

	nsCOMPtr<nsIDOMDocument> dom_document;
	dom_window->GetDocument (getter_AddRefs (dom_document));
	if (dom_document == nsnull) {
		DEBUGMSG ("No DOM document available\n");
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
	if (NS_FAILED (rv))
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
					DEBUGMSG ("Unable to find temp_id element\n");
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
	MoonlightCollectionClass = new MoonlightCollectionType ();
	MoonlightContentClass = new MoonlightContentType ();
	MoonlightControlClass = new MoonlightControlType ();
	MoonlightDependencyObjectClass = new MoonlightDependencyObjectType ();
	MoonlightDownloaderClass = new MoonlightDownloaderType ();
	MoonlightDurationClass = new MoonlightDurationType ();
	MoonlightErrorEventArgsClass = new MoonlightErrorEventArgsType ();
	MoonlightEventObjectClass = new MoonlightEventObjectType ();
	MoonlightImageBrushClass = new MoonlightImageBrushType ();
	MoonlightImageClass = new MoonlightImageType ();
	MoonlightKeyboardEventArgsClass = new MoonlightKeyboardEventArgsType ();
	MoonlightMarkerReachedEventArgsClass = new MoonlightMarkerReachedEventArgsType ();
	MoonlightMediaElementClass = new MoonlightMediaElementType ();
	MoonlightMouseEventArgsClass = new MoonlightMouseEventArgsType ();
	MoonlightObjectClass = new MoonlightObjectType ();
	MoonlightPointClass = new MoonlightPointType ();
	MoonlightRectClass = new MoonlightRectType ();
	MoonlightScriptableObjectClass = new MoonlightScriptableObjectType ();
	MoonlightScriptControlClass = new MoonlightScriptControlType ();
	MoonlightSettingsClass = new MoonlightSettingsType ();
	MoonlightStoryboardClass = new MoonlightStoryboardType ();
	MoonlightStylusInfoClass = new MoonlightStylusInfoType ();
	MoonlightStylusPointCollectionClass = new MoonlightStylusPointCollectionType ();
	MoonlightStrokeCollectionClass = new MoonlightStrokeCollectionType ();
	MoonlightStrokeClass = new MoonlightStrokeType ();
	MoonlightTextBlockClass = new MoonlightTextBlockType ();
	MoonlightTimeSpanClass = new MoonlightTimeSpanType ();
}

void
plugin_destroy_classes (void)
{

	delete MoonlightCollectionClass; MoonlightCollectionClass = NULL;
	delete MoonlightContentClass; MoonlightContentClass = NULL;
	delete MoonlightControlClass; MoonlightControlClass = NULL;
	delete MoonlightEventObjectClass; MoonlightEventObjectClass = NULL;
	delete MoonlightDependencyObjectClass; MoonlightDependencyObjectClass = NULL;
	delete MoonlightDownloaderClass; MoonlightDownloaderClass = NULL;
	delete MoonlightErrorEventArgsClass; MoonlightErrorEventArgsClass = NULL;
	delete MoonlightImageBrushClass; MoonlightImageBrushClass = NULL;
	delete MoonlightImageClass; MoonlightImageClass = NULL;
	delete MoonlightMediaElementClass; MoonlightMediaElementClass = NULL;
	delete MoonlightMouseEventArgsClass; MoonlightMouseEventArgsClass = NULL;
	delete MoonlightKeyboardEventArgsClass; MoonlightKeyboardEventArgsClass = NULL;
	delete MoonlightObjectClass; MoonlightObjectClass = NULL;
	delete MoonlightScriptableObjectClass; MoonlightScriptableObjectClass = NULL;
	delete MoonlightScriptControlClass; MoonlightScriptControlClass = NULL;
	delete MoonlightSettingsClass; MoonlightSettingsClass = NULL;
	delete MoonlightStoryboardClass; MoonlightStoryboardClass = NULL;
	delete MoonlightStylusInfoClass; MoonlightStylusInfoClass = NULL;
	delete MoonlightStylusPointCollectionClass; MoonlightStylusPointCollectionClass = NULL;
	delete MoonlightStrokeCollectionClass; MoonlightStrokeCollectionClass = NULL;
	delete MoonlightStrokeClass; MoonlightStrokeClass = NULL;
	delete MoonlightTextBlockClass; MoonlightTextBlockClass = NULL;
	delete MoonlightRectClass; MoonlightRectClass = NULL;
	delete MoonlightPointClass; MoonlightPointClass = NULL;
	delete MoonlightDurationClass; MoonlightDurationClass = NULL;
	delete MoonlightTimeSpanClass; MoonlightTimeSpanClass = NULL;
	delete MoonlightMarkerReachedEventArgsClass; MoonlightMarkerReachedEventArgsClass = NULL;
}
