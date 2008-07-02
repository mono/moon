/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-class.h: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef PLUGIN_CLASS
#define PLUGIN_CLASS

#include "moonlight.h"
#include "error.h"
#include "plugin.h"


G_BEGIN_DECLS
void plugin_init_classes (void);
void plugin_destroy_classes (void);

void event_object_add_javascript_listener (EventObject *obj, PluginInstance *instance, const char *event_name, const char *cb_name);

void string_to_npvariant (const char *value, NPVariant *result);

G_END_DECLS

struct MoonNameIdMapping {
	const char *name;
	int id;
};

/*** EventListenerProxy */
typedef void (*EventArgsWrapper)(NPP instance, EventArgs *calldata, NPVariant *value);

class EventListenerProxy : public List::Node {
	static void on_target_object_destroyed (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	EventObject *target_object;
	
	NPP instance;
	bool is_func;
	
	/* if @is_func == true, callback is an NPObject (the function object)
	   if @is_func == false, callback is a char* (the function name)
	*/
	gpointer callback;
	
	char *event_name;
	int event_id;
	int dtoken;
	int token;
	
	bool one_shot;

 public:
	EventListenerProxy (NPP instance, const char *event_name, const char *cb_name);
	EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb);
	virtual ~EventListenerProxy ();
	
	int AddHandler (EventObject *obj);
	void RemoveHandler ();
	void Invalidate ();
	const char *GetCallbackAsString ();

	int GetEventId () { return event_id; }

	void SetOneShot () { one_shot = true; }

	static void proxy_listener_to_javascript (EventObject *sender, EventArgs *calldata, gpointer closure);
};

/*** MoonlightObjectClass **************************************************************/

struct MoonlightObjectType : NPClass {
	MoonlightObjectType ();
	
	~MoonlightObjectType() { g_free (mapping); }
	
	void AddMapping (const MoonNameIdMapping *mapping, int count);

	bool Enumerate (NPIdentifier **value, uint32_t *count);

	int LookupName (NPIdentifier name);

	MoonNameIdMapping *mapping;
	int mapping_count;

	NPIdentifier last_lookup;
	int last_id;
};

extern MoonlightObjectType *MoonlightObjectClass;

struct MoonlightObject : NPObject {
	MoonlightObject (NPP instance)
	{
		this->instance = instance;
		this->moonlight_type = Type::INVALID;
		this->event_listener_proxies = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, destroy_proxy);
	}
	
	virtual void Invalidate ();
	virtual ~MoonlightObject ();
	
	virtual bool HasProperty (NPIdentifier name);
	virtual bool GetProperty (int id, NPIdentifier name, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier name, const NPVariant *value);
	
	virtual bool HasMethod (NPIdentifier unmapped);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
	int LookupName (NPIdentifier name) { return ((MoonlightObjectType *)_class)->LookupName (name); }
	
	EventListenerProxy *LookupEventProxy (int event_id);
	void SetEventProxy (int event_id, EventListenerProxy* proxy);
	void ClearEventProxy (int event_id);
	
	static void destroy_proxy (gpointer data);
	static void invalidate_proxy (gpointer key, gpointer value, gpointer data);
	
	NPP instance;
	Type::Kind moonlight_type;
	GHashTable *event_listener_proxies;
};

/*** MoonlightPointClass  **************************************************************/
struct MoonlightPointType : MoonlightObjectType {
	MoonlightPointType ();
};

extern MoonlightPointType *MoonlightPointClass;

struct MoonlightPoint : MoonlightObject {
	MoonlightPoint (NPP instance) : MoonlightObject(instance), point (Point())
	{
		moonlight_type = Type::POINT;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	Point point;
};

/*** MoonlightRectClass  **************************************************************/
struct MoonlightRectType : MoonlightObjectType {
	MoonlightRectType ();
};

extern MoonlightRectType *MoonlightRectClass;

struct MoonlightRect : MoonlightObject {
	MoonlightRect (NPP instance) : MoonlightObject(instance), rect (Rect())
	{
		moonlight_type = Type::RECT;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	Rect rect;
};


/*** MoonlightDurationClass  **************************************************************/
struct MoonlightDurationType : MoonlightObjectType {
	MoonlightDurationType ();
};

extern MoonlightDurationType *MoonlightDurationClass;

struct MoonlightDuration : MoonlightObject {
	MoonlightDuration (NPP instance) : MoonlightObject (instance)
	{
		moonlight_type = Type::DURATION;
		parent_property = NULL;
		parent_obj = NULL;
	}

	virtual ~MoonlightDuration ();

	void SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property);
	
	double GetValue ();
	
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
	
	DependencyProperty *parent_property;
	DependencyObject *parent_obj;
};

/*** MoonlightTimeSpanClass  **************************************************************/
struct MoonlightTimeSpanType : MoonlightObjectType {
	MoonlightTimeSpanType ();
};

extern MoonlightTimeSpanType *MoonlightTimeSpanClass;

struct MoonlightTimeSpan : MoonlightObject {
	MoonlightTimeSpan (NPP instance) : MoonlightObject (instance)
	{
		moonlight_type = Type::TIMESPAN;
		parent_property = NULL;
		parent_obj = NULL;
	}
	
	virtual ~MoonlightTimeSpan ();
	
	void SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property);
	
	TimeSpan GetValue ();
	
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
	
	DependencyProperty *parent_property;
	DependencyObject *parent_obj;
};

/*** MoonlightKeyTimeClass  ****************************************************************/
struct MoonlightKeyTimeType : MoonlightObjectType {
	MoonlightKeyTimeType ();
};

extern MoonlightKeyTimeType *MoonlightKeyTimeClass;

struct MoonlightKeyTime : MoonlightObject {
	MoonlightKeyTime (NPP instance) : MoonlightObject (instance)
	{
		moonlight_type = Type::KEYTIME;
		parent_property = NULL;
		parent_obj = NULL;
	}
	
	virtual ~MoonlightKeyTime ();
	
	void SetParentInfo (DependencyObject *parent_obj, DependencyProperty *parent_property);
	
	KeyTime* GetValue ();
	
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
	
	DependencyProperty *parent_property;
	DependencyObject *parent_obj;
};

/*** MoonlightSettingsClass ***********************************************************/

struct MoonlightSettingsType : MoonlightObjectType {
	MoonlightSettingsType ();
};

extern MoonlightSettingsType *MoonlightSettingsClass;

struct MoonlightSettingsObject : MoonlightObject {
	MoonlightSettingsObject (NPP instance) : MoonlightObject (instance)
	{
	}
	
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};


/*** MoonlightContentClass ************************************************************/

struct MoonlightContentType : MoonlightObjectType {
	MoonlightContentType ();
};

extern MoonlightContentType *MoonlightContentClass;

struct MoonlightContentObject : MoonlightObject {
	MoonlightContentObject (NPP instance) : MoonlightObject (instance)
	{
		registered_scriptable_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	}

	virtual ~MoonlightContentObject ();
	
	virtual bool HasProperty (NPIdentifier unmapped);
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
	
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
	
	GHashTable *registered_scriptable_objects;
};

/*** MoonlightScriptControlClass **********************************************************/

struct MoonlightScriptControlType : MoonlightObjectType {
	MoonlightScriptControlType ();
};

extern MoonlightScriptControlType *MoonlightScriptControlClass;

struct MoonlightScriptControlObject : MoonlightObject {
	MoonlightScriptControlObject (NPP instance) : MoonlightObject (instance)
	{
		settings = NPN_CreateObject (instance, MoonlightSettingsClass);
		content = NPN_CreateObject (instance, MoonlightContentClass);
	}

	virtual ~MoonlightScriptControlObject ();

	virtual void Invalidate ();
	
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
	
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
	
	NPObject *settings;
	NPObject *content;
};

/*** MoonlightEventObjectClass ***************************************************/
struct MoonlightEventObjectType : MoonlightObjectType {
	MoonlightEventObjectType ();
};

extern MoonlightEventObjectType *MoonlightEventObjectClass;

struct MoonlightEventObjectObject : MoonlightObject {
	MoonlightEventObjectObject (NPP instance) : MoonlightObject (instance)
	{
		moonlight_type = Type::EVENTOBJECT;
		eo = NULL;
	}
	
	virtual ~MoonlightEventObjectObject ();

	EventObject *eo;
};

extern MoonlightEventObjectObject *EventObjectCreateWrapper (NPP instance, EventObject *obj);

/*** MoonlightDependencyObjectClass ***************************************************/
struct MoonlightDependencyObjectType : MoonlightEventObjectType {
	MoonlightDependencyObjectType ();
};


struct MoonlightDependencyObjectObject : MoonlightEventObjectObject {
	MoonlightDependencyObjectObject (NPP instance) : MoonlightEventObjectObject (instance)
	{
		moonlight_type = Type::DEPENDENCY_OBJECT;
	}
	
	DependencyObject *GetDependencyObject ()
	{
		if (eo == NULL || !eo->Is (Type::DEPENDENCY_OBJECT)) {
			g_warning ("MoonlightDependencyObjectObject::GetDependencyObject (): Not a dependency object!");
			return NULL;
		}
		
		return (DependencyObject*) eo;
	}

	virtual bool HasProperty (NPIdentifier unmapped);
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

};

extern MoonlightDependencyObjectObject *DependencyObjectCreateWrapper (NPP instance, DependencyObject *obj);

/*** MoonlightEventArgsClass ******************************************************/
struct MoonlightEventArgsType : MoonlightDependencyObjectType {
	MoonlightEventArgsType ();
};

struct MoonlightEventArgs : MoonlightDependencyObjectObject {
	MoonlightEventArgs (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::EVENTARGS;
	}
};

/*** MoonlightMouseEventArgsClass  **************************************************************/
struct MoonlightMouseEventArgsType : MoonlightEventArgsType {
	MoonlightMouseEventArgsType ();
};

struct MoonlightMouseEventArgsObject : MoonlightEventArgs {
	MoonlightMouseEventArgsObject (NPP instance) : MoonlightEventArgs (instance)
	{
		moonlight_type = Type::MOUSEEVENTARGS;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	MouseEventArgs *GetMouseEventArgs () { return (MouseEventArgs *) eo; };
};

/*** MoonlightMarkerReachedEventArgsClass ******************************************/
struct MoonlightMarkerReachedEventArgsType : MoonlightEventArgsType {
	MoonlightMarkerReachedEventArgsType ();
};

extern MoonlightMarkerReachedEventArgsType *MoonlightMarkerReachedEventArgsClass;

struct MoonlightMarkerReachedEventArgsObject : MoonlightEventArgs {
	MoonlightMarkerReachedEventArgsObject (NPP instance) : MoonlightEventArgs (instance)
	{
		moonlight_type = Type::MARKERREACHEDEVENTARGS;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);

	MarkerReachedEventArgs *GetMarkerReachedEventArgs () { return (MarkerReachedEventArgs *) eo; }
};

/*** MoonlightKeyboardEventArgsClass  **************************************************************/
struct MoonlightKeyboardEventArgsType : MoonlightEventArgsType {
	MoonlightKeyboardEventArgsType ();
};

struct MoonlightKeyboardEventArgsObject : MoonlightEventArgs {
	MoonlightKeyboardEventArgsObject (NPP instance) : MoonlightEventArgs (instance)
	{
		moonlight_type = Type::KEYBOARDEVENTARGS;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	KeyboardEventArgs *GetKeyboardEventArgs () { return (KeyboardEventArgs *) eo; }
};

/*** MoonlightErrorEventArgsClass ******************************************************/
struct MoonlightErrorEventArgsType : MoonlightEventArgsType {
	MoonlightErrorEventArgsType ();
};

struct MoonlightErrorEventArgs : MoonlightEventArgs {
	MoonlightErrorEventArgs (NPP instance) : MoonlightEventArgs (instance)
	{
		moonlight_type = Type::ERROREVENTARGS;
	}

	ErrorEventArgs *GetErrorEventArgs () { return (ErrorEventArgs *) eo; }
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
};

/*** MoonlightCollectionClass ***************************************************/

struct MoonlightCollectionType : MoonlightDependencyObjectType {
	MoonlightCollectionType ();
};

struct MoonlightCollectionObject : MoonlightDependencyObjectObject {
	MoonlightCollectionObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::COLLECTION;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightStoryboardClass ***************************************************/

struct MoonlightStoryboardType : MoonlightDependencyObjectType {
	MoonlightStoryboardType ();
};

struct MoonlightStoryboardObject : MoonlightDependencyObjectObject {
	MoonlightStoryboardObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::STORYBOARD;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightMediaElement ***************************************************/

struct MoonlightMediaElementType : MoonlightDependencyObjectType {
	MoonlightMediaElementType ();
};


struct MoonlightMediaElementObject : MoonlightDependencyObjectObject {
	MoonlightMediaElementObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::MEDIAELEMENT;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightImage ***************************************************/

struct MoonlightImageType : MoonlightDependencyObjectType {
	MoonlightImageType ();
};


struct MoonlightImageObject : MoonlightDependencyObjectObject {
	MoonlightImageObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::IMAGE;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightImageBrush ***************************************************/

struct MoonlightImageBrushType : MoonlightDependencyObjectType {
	MoonlightImageBrushType ();
};


struct MoonlightImageBrushObject : MoonlightDependencyObjectObject {
	MoonlightImageBrushObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::IMAGEBRUSH;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightDownloader ***************************************************/

struct MoonlightDownloaderType : MoonlightDependencyObjectType {
	MoonlightDownloaderType ();
};


struct MoonlightDownloaderObject : MoonlightDependencyObjectObject {
	MoonlightDownloaderObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::DOWNLOADER;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightTextBlock ***************************************************/

struct MoonlightTextBlockType : MoonlightDependencyObjectType {
	MoonlightTextBlockType ();
};


struct MoonlightTextBlockObject : MoonlightDependencyObjectObject {
	MoonlightTextBlockObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::TEXTBLOCK;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightStylusInfoType ***************************************************/

struct MoonlightStylusInfoType : MoonlightDependencyObjectType {
	MoonlightStylusInfoType ();
};


struct MoonlightStylusInfoObject : MoonlightDependencyObjectObject {
	MoonlightStylusInfoObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::STYLUSINFO;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
};

/*** MoonlightStylusPointCollectionType *************************************/

struct MoonlightStylusPointCollectionType : MoonlightCollectionType {
	MoonlightStylusPointCollectionType ();
};


struct MoonlightStylusPointCollectionObject : MoonlightCollectionObject {
	MoonlightStylusPointCollectionObject (NPP instance) : MoonlightCollectionObject (instance)
	{
		moonlight_type = Type::STYLUSPOINT_COLLECTION;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};


/*** MoonlightStrokeCollectionType *************************************/

struct MoonlightStrokeCollectionType : MoonlightCollectionType {
	MoonlightStrokeCollectionType ();
};


struct MoonlightStrokeCollectionObject : MoonlightCollectionObject {

	MoonlightStrokeCollectionObject (NPP instance)
		: MoonlightCollectionObject (instance)
	{
		moonlight_type = Type::STROKE_COLLECTION;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};


/*** MoonlightStrokeType *************************************/

struct MoonlightStrokeType : MoonlightDependencyObjectType {
	MoonlightStrokeType ();
};


struct MoonlightStrokeObject : MoonlightDependencyObjectObject {

	MoonlightStrokeObject (NPP instance)
		: MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::STROKE;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightControl ***************************************************/

struct MoonlightControlType : MoonlightDependencyObjectType {
	MoonlightControlType ();
};


struct MoonlightControlObject : MoonlightDependencyObjectObject {
	MoonlightControlObject (NPP instance) : MoonlightDependencyObjectObject (instance)
	{
		real_object = NULL;
	}

	virtual bool HasProperty (NPIdentifier name);
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool HasMethod (NPIdentifier name);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	MoonlightEventObjectObject *real_object;
};

/*** MoonlightScriptableObject ***************************************************/

struct MoonlightScriptableObjectObject;

typedef void (*InvokeDelegate) (gpointer managed_obj_handle, gpointer method_handle, Value** args, int arg_count, Value* return_value);
typedef void (*SetPropertyDelegate) (gpointer managed_obj_handle, gpointer property_handle, Value *value);
typedef void (*GetPropertyDelegate) (gpointer managed_obj_handle, gpointer property_handle, Value *value);
typedef void (*EventHandlerDelegate) (gpointer managed_obj_handle, gpointer event_handle, MoonlightScriptableObjectObject* scriptable_obj, gpointer closure);

struct MoonlightScriptableObjectType : MoonlightObjectType {
	MoonlightScriptableObjectType ();
};

extern MoonlightScriptableObjectType *MoonlightScriptableObjectClass;

struct MoonlightScriptableObjectObject : MoonlightObject {
	MoonlightScriptableObjectObject (NPP instance) : MoonlightObject (instance)
	{
		managed_scriptable = NULL;
		properties = g_hash_table_new (g_direct_hash, g_direct_equal);
		methods = g_hash_table_new (g_direct_hash, g_direct_equal);
		events = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	
	virtual ~MoonlightScriptableObjectObject ();

	virtual bool HasProperty (NPIdentifier name);
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool HasMethod (NPIdentifier name);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	gpointer managed_scriptable;
	GHashTable *properties;
	GHashTable *methods;
	GHashTable *events;

	InvokeDelegate invoke;
	SetPropertyDelegate setprop;
	GetPropertyDelegate getprop;
	EventHandlerDelegate addevent;
	EventHandlerDelegate removeevent;
};


G_BEGIN_DECLS

// These are meant to be called by System.Silverlight.dll

MoonlightScriptableObjectObject *moonlight_scriptable_object_wrapper_create (PluginInstance *plugin, gpointer scriptable,
									     InvokeDelegate invoke,
									     SetPropertyDelegate setprop,
									     GetPropertyDelegate getprop,
									     EventHandlerDelegate addevent,
									     EventHandlerDelegate removeevent);

void moonlight_scriptable_object_add_property (PluginInstance *plugin,
					       MoonlightScriptableObjectObject *obj,
					       gpointer property_handle,
					       char *property_name,
					       int property_type,
					       bool can_read,
					       bool can_write);

void moonlight_scriptable_object_add_event (PluginInstance *plugin,
					    MoonlightScriptableObjectObject *obj,
					    gpointer event_handle,
					    char *event_name);

void moonlight_scriptable_object_add_method (PluginInstance *plugin,
					     MoonlightScriptableObjectObject *obj,
					     gpointer method_handle,
					     char *method_name,
					     int method_return_type,
					     int *method_parameter_types,
					     int parameter_count);

void moonlight_scriptable_object_register (PluginInstance *plugin,
					   char *name,
					   MoonlightScriptableObjectObject *obj);

void moonlight_scriptable_object_emit_event (PluginInstance *plugin,
					     MoonlightScriptableObjectObject *obj,
					     MoonlightScriptableObjectObject *event_args,
					     NPObject *cb_obj);

/*** HtmlObject ***************************************************/

// int clientX, int clientY,

typedef void callback_dom_event (char *name, int client_x, int client_y, int offset_x, int offset_y, gboolean alt_key,
				 gboolean ctrl_key, gboolean shift_key, int mouse_button);


const char *html_get_element_text (PluginInstance *plugin, const char *element_id);

// These are meant to be called by System.Silverlight.dll

void html_object_get_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *result);
void html_object_set_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *value);
void html_object_invoke (PluginInstance *plugin, NPObject *npobj, char *name, Value *args, uint32_t arg_count, Value *result);
gpointer html_object_attach_event (PluginInstance *plugin, NPObject *npobj, char *name, callback_dom_event *cb);
void html_object_detach_event (PluginInstance *plugin, const char *name, gpointer listener);
void html_object_release (PluginInstance *plugin, NPObject *npobj);

/*** Browser interaction utility classes ***/
void browser_do_alert (PluginInstance *plugin, char *msg);
G_END_DECLS

#endif /* PLUGIN_CLASS */
