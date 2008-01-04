/*
 * plugin-class.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
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
G_END_DECLS

typedef struct {
	const char *name;
	int id;
} MoonNameIdMapping;

/*** EventListenerProxy */
typedef void (*EventArgsWrapper)(NPP instance, gpointer calldata, NPVariant *value);

class EventListenerProxy : public List::Node {
 public:
	EventListenerProxy (NPP instance, const char *event_name, const char *cb_name);
	EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb);
	~EventListenerProxy ();
	int AddHandler (EventObject *obj);
	void RemoveHandler ();
	const char *GetCallbackAsString ();

	int GetEventId () { return event_id; }
 private:
	NPP instance;

	bool is_func;

	/* if @is_func == true, callback is an NPObject (the function object)
	   if @is_func == false, callback is a char* (the function name)
	*/
	gpointer callback;

	char *event_name;

	int token;
	int event_id;
	EventObject *target_object;

	static EventArgsWrapper get_wrapper_for_event_name (const char *event_name);
	static void default_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void mouse_event_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void keyboard_event_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void timeline_marker_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void on_target_object_destroyed (EventObject *sender, gpointer calldata, gpointer closure);
	static void proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure);
};

/*** MoonlightObjectClass **************************************************************/

struct MoonlightObjectType : NPClass {
	MoonlightObjectType ();

	~MoonlightObjectType() { g_free (mapping); }

	void AddMapping (const MoonNameIdMapping* mapping, int count);

	int LookupName (NPIdentifier name);

	MoonNameIdMapping* mapping;
	int mapping_count;

	NPIdentifier last_lookup;
	int last_id;
};

extern MoonlightObjectType* MoonlightObjectClass;

struct MoonlightObject : public NPObject
{
	MoonlightObject (NPP instance)
	{
		this->instance = instance;
		this->moonlight_type = Type::INVALID;
		this->disposed = false;
		this->event_listener_proxies = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, destroy_proxy);
	}
	virtual void Dispose ();
	virtual ~MoonlightObject ();

	virtual bool HasProperty (NPIdentifier name);
	virtual bool GetProperty (int id, NPIdentifier name, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier name, const NPVariant *value);

	virtual bool HasMethod (NPIdentifier unmapped);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
	int LookupName (NPIdentifier name) { return ((MoonlightObjectType*)_class)->LookupName (name); }

	EventListenerProxy* LookupEventProxy (int event_id);
	void SetEventProxy (int event_id, EventListenerProxy* proxy);
	void ClearEventProxy (int event_id);

	static void destroy_proxy (gpointer data);

	NPP instance;
	Type::Kind moonlight_type;
	bool disposed;
	GHashTable *event_listener_proxies;
};

/*** MoonlightErrorEventArgsClass ******************************************************/
struct MoonlightErrorEventArgsType : MoonlightObjectType {
	MoonlightErrorEventArgsType ();
};

extern MoonlightErrorEventArgsType* MoonlightErrorEventArgsClass;

struct MoonlightErrorEventArgs : MoonlightObject {
	MoonlightErrorEventArgs (NPP instance) : MoonlightObject(instance)
	{
	}

	ErrorEventArgs *args;

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
};

/*** MoonlightPointClass  **************************************************************/
struct MoonlightPointType : MoonlightObjectType {
	MoonlightPointType ();
};

extern MoonlightPointType* MoonlightPointClass;

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

extern MoonlightRectType* MoonlightRectClass;

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

extern MoonlightDurationType* MoonlightDurationClass;

struct MoonlightDuration : MoonlightObject {
	MoonlightDuration (NPP instance) : MoonlightObject (instance), duration (Duration (0))
	{
		moonlight_type = Type::DURATION;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	Duration duration;
};

/*** MoonlightTimeSpanClass  **************************************************************/
struct MoonlightTimeSpanType : MoonlightObjectType {
	MoonlightTimeSpanType ();
};

extern MoonlightTimeSpanType* MoonlightTimeSpanClass;

struct MoonlightTimeSpan : MoonlightObject {
	MoonlightTimeSpan (NPP instance) : MoonlightObject (instance), timespan (0)
	{
		moonlight_type = Type::TIMESPAN;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	TimeSpan timespan;
};

/*** MoonlightMouseEventArgsClass  **************************************************************/
struct MoonlightMouseEventArgsType : MoonlightObjectType {
	MoonlightMouseEventArgsType ();
};

extern MoonlightMouseEventArgsType* MoonlightMouseEventArgsClass;

struct MoonlightMouseEventArgsObject : MoonlightObject {
	MoonlightMouseEventArgsObject (NPP instance)
	  : MoonlightObject (instance), state (0), x (-1), y (-1)
	{
	}

	virtual void Dispose ();

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	int state;
	double x;
	double y;
};

/*** MoonlightMarkerReachedEventArgsClass ******************************************/
struct MoonlightMarkerReachedEventArgsType : MoonlightObjectType {
	MoonlightMarkerReachedEventArgsType ();
};

extern MoonlightMarkerReachedEventArgsType* MoonlightMarkerReachedEventArgsClass;

struct MoonlightMarkerReachedEventArgsObject : MoonlightObject {
	MoonlightMarkerReachedEventArgsObject (NPP instance)
		: MoonlightObject (instance), marker (NULL)
	{
	}

	virtual void Dispose ();
	void SetMarker (TimelineMarker* tm)
	{
		if (marker)
			marker->unref ();
		marker = tm;
		if (marker)
			marker->ref ();
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);

	TimelineMarker* marker;
};

/*** MoonlightKeyboardEventArgsClass  **************************************************************/
struct MoonlightKeyboardEventArgsType : MoonlightObjectType {
	MoonlightKeyboardEventArgsType ();
};

extern MoonlightKeyboardEventArgsType* MoonlightKeyboardEventArgsClass;

struct MoonlightKeyboardEventArgsObject : MoonlightObject {
	MoonlightKeyboardEventArgsObject (NPP instance)
	  : MoonlightObject (instance), state (0), key (0), platformcode (0)
	{
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);

	int state;
	int key;
	int platformcode;
};

extern void KeyboardEventArgsPopulate (MoonlightKeyboardEventArgsObject *ea, KeyboardEventArgs *args);


/*** MoonlightSettingsClass ***********************************************************/

struct MoonlightSettingsType : MoonlightObjectType {
	MoonlightSettingsType ();
};
extern MoonlightSettingsType* MoonlightSettingsClass;

struct MoonlightSettingsObject : MoonlightObject {
	MoonlightSettingsObject (NPP instance)
	  : MoonlightObject (instance)
	{
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);
};


/*** MoonlightContentClass ************************************************************/

struct MoonlightContentType : MoonlightObjectType {
	MoonlightContentType ();
};

extern MoonlightContentType* MoonlightContentClass;

struct MoonlightContentObject : MoonlightObject {
	MoonlightContentObject (NPP instance)
	  : MoonlightObject (instance)
	{
		registered_scriptable_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	virtual void Dispose ();

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
extern MoonlightScriptControlType* MoonlightScriptControlClass;

struct MoonlightScriptControlObject : public MoonlightObject {
	MoonlightScriptControlObject (NPP instance) : MoonlightObject (instance)
	{
		content = NPN_CreateObject (instance, MoonlightContentClass);
		settings = NPN_CreateObject (instance, MoonlightSettingsClass);
	}

	virtual void Dispose ();

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	NPObject *content;
	NPObject *settings;
};

/*** MoonlightEventObjectClass ***************************************************/
struct MoonlightEventObjectType : MoonlightObjectType {
	MoonlightEventObjectType ();
};
extern MoonlightEventObjectType *MoonlightEventObjectClass;

struct MoonlightEventObjectObject : public MoonlightObject
{
	MoonlightEventObjectObject (NPP instance) : MoonlightObject (instance)
	{
		eo = NULL;
		moonlight_type = Type::EVENTOBJECT;
	}

	void SetEventObject (EventObject *eventobject);

	virtual void Dispose ();

	EventObject *eo;
};

extern MoonlightEventObjectObject* EventObjectCreateWrapper (NPP instance, EventObject *obj);

/*** MoonlightDependencyObjectClass ***************************************************/
struct MoonlightDependencyObjectType : MoonlightEventObjectType {
	MoonlightDependencyObjectType ();
};
extern MoonlightDependencyObjectType *MoonlightDependencyObjectClass;

struct MoonlightDependencyObjectObject : public MoonlightEventObjectObject
{
	MoonlightDependencyObjectObject (NPP instance) : MoonlightEventObjectObject (instance)
	{
		moonlight_type = Type::DEPENDENCY_OBJECT;
	}

	void SetDependencyObject (DependencyObject *dob)
	{
		SetEventObject (dob);
	}

	DependencyObject* GetDependencyObject ()
	{
		g_assert (eo->GetObjectType () >= Type::DEPENDENCY_OBJECT);
		return (DependencyObject*) eo;
	}

	virtual void Dispose ();

	virtual bool HasProperty (NPIdentifier unmapped);
	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool SetProperty (int id, NPIdentifier unmapped, const NPVariant *value);

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

};

extern MoonlightDependencyObjectObject* DependencyObjectCreateWrapper (NPP instance, DependencyObject *obj);

/*** MoonlightCollectionClass ***************************************************/

struct MoonlightCollectionType : MoonlightDependencyObjectType {
	MoonlightCollectionType ();
};
extern MoonlightCollectionType* MoonlightCollectionClass;

struct MoonlightCollectionObject : public MoonlightDependencyObjectObject {
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

extern MoonlightStoryboardType* MoonlightStoryboardClass;

struct MoonlightStoryboardObject : MoonlightDependencyObjectObject {
	MoonlightStoryboardObject (NPP instance)
	  : MoonlightDependencyObjectObject (instance)
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

extern MoonlightMediaElementType* MoonlightMediaElementClass;

struct MoonlightMediaElementObject : MoonlightDependencyObjectObject {
	MoonlightMediaElementObject (NPP instance)
	  : MoonlightDependencyObjectObject (instance)
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

extern MoonlightImageType* MoonlightImageClass;

struct MoonlightImageObject : MoonlightDependencyObjectObject {
	MoonlightImageObject (NPP instance)
	  : MoonlightDependencyObjectObject (instance)
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

extern MoonlightImageBrushType* MoonlightImageBrushClass;

struct MoonlightImageBrushObject : MoonlightDependencyObjectObject {
	MoonlightImageBrushObject (NPP instance)
	  : MoonlightDependencyObjectObject (instance)
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

extern MoonlightDownloaderType* MoonlightDownloaderClass;

struct MoonlightDownloaderObject : public MoonlightDependencyObjectObject {
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

extern MoonlightTextBlockType* MoonlightTextBlockClass;

struct MoonlightTextBlockObject : MoonlightDependencyObjectObject {
	MoonlightTextBlockObject (NPP instance)
	  : MoonlightDependencyObjectObject (instance)
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

extern MoonlightStylusInfoType *MoonlightStylusInfoClass;

struct MoonlightStylusInfoObject : MoonlightDependencyObjectObject {

	MoonlightStylusInfoObject (NPP instance)
		: MoonlightDependencyObjectObject (instance)
	{
		moonlight_type = Type::STYLUSINFO;
	}

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
};

/*** MoonlightStylusPointCollectionType *************************************/

struct MoonlightStylusPointCollectionType : MoonlightCollectionType {
	MoonlightStylusPointCollectionType ();
};

extern MoonlightStylusPointCollectionType *MoonlightStylusPointCollectionClass;

struct MoonlightStylusPointCollectionObject : MoonlightCollectionObject {

	MoonlightStylusPointCollectionObject (NPP instance)
		: MoonlightCollectionObject (instance)
	{
		moonlight_type = Type::STYLUSPOINT_COLLECTION;
	}

	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);
};

/*** MoonlightControl ***************************************************/

struct MoonlightControlType : MoonlightDependencyObjectType {
	MoonlightControlType ();
};

extern MoonlightControlType* MoonlightControlClass;;

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

extern MoonlightScriptableObjectType* MoonlightScriptableObjectClass;

struct MoonlightScriptableObjectObject : public MoonlightObject
{
	MoonlightScriptableObjectObject (NPP instance) : MoonlightObject (instance)
	{
		managed_scriptable = NULL;
		properties = g_hash_table_new (g_direct_hash, g_direct_equal);
		methods = g_hash_table_new (g_direct_hash, g_direct_equal);
		events = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	virtual void Dispose ();

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

extern "C" {
	// These are meant to be called by System.Silverlight.dll

	MoonlightScriptableObjectObject* moonlight_scriptable_object_wrapper_create (PluginInstance *plugin, gpointer scriptable,
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
}


/*** HtmlObject ***************************************************/

// int clientX, int clientY,

typedef void callback_dom_event (char *name, int client_x, int client_y, int offset_x, int offset_y, gboolean alt_key,
		gboolean ctrl_key, gboolean shift_key, int mouse_button);

extern "C" {

	const char *html_get_element_text (PluginInstance *plugin, const char *element_id);

	// These are meant to be called by System.Silverlight.dll

	void html_object_get_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *result);
	void html_object_set_property (PluginInstance *plugin, NPObject *npobj, char *name, Value *value);
	void html_object_invoke (PluginInstance *plugin, NPObject *npobj, char *name, Value *args, uint32_t arg_count, Value *result);
	gpointer html_object_attach_event (PluginInstance *plugin, NPObject *npobj, char *name, callback_dom_event *cb);
	void html_object_detach_event (PluginInstance *plugin, const char *name, gpointer listener);
	void html_object_release (PluginInstance *plugin, NPObject *npobj);

}


/*** Browser interaction utility classes ***/

extern "C" {

	void browser_do_alert (PluginInstance *plugin, char *msg);
}

#endif /* PLUGIN_CLASS */
