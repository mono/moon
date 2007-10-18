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


void plugin_init_classes (void);
void plugin_destroy_classes (void);

/*** EventListenerProxy */
typedef void (*EventArgsWrapper)(NPP instance, gpointer calldata, NPVariant *value);

class EventListenerProxy : public List::Node {
 public:
	EventListenerProxy (NPP instance, const char *event_name, const char *cb_name);
	EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb);
	~EventListenerProxy ();
	void AddHandler (EventObject *obj);
	void RemoveHandler (EventObject *obj);
	NPObject* GetCallbackAsNPObject ();
 private:
	NPP instance;

	bool is_func;

	/* if @is_func == true, callback is an NPObject (the function object)
	   if @is_func == false, callback is a char* (the function name)
	*/
	gpointer callback;

	char *event_name;

	static EventArgsWrapper get_wrapper_for_event_name (const char *event_name);
	static void default_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void mouse_event_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void keyboard_event_wrapper (NPP instance, gpointer calldata, NPVariant *value);
	static void proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure);
};

extern "C" {
  void event_object_add_javascript_listener (EventObject *obj, PluginInstance *instance, const char *event_name, const char *cb_name);
}

/*** MoonlightObjectClass **************************************************************/

struct MoonlightObjectType : NPClass {
	MoonlightObjectType ();
};

extern MoonlightObjectType* MoonlightObjectClass;

struct MoonlightObject : public NPObject
{
	MoonlightObject (NPP instance)
	{
		this->instance = instance;
		this->moonlight_type = Type::INVALID;
		this->disposed = false;
	}
	virtual void Dispose ();
	virtual ~MoonlightObject ();

	NPP instance;
	Type::Kind moonlight_type;
	bool disposed;
};

/*** MoonlightEventListenerObject ******************************************************/
struct MoonlightEventListenerType : MoonlightObjectType {
	MoonlightEventListenerType ();
};

extern MoonlightEventListenerType* MoonlightEventListenerClass;

struct MoonlightEventListenerObject : MoonlightObject {
	MoonlightEventListenerObject (NPP instance) : MoonlightObject (instance), proxy (NULL), target (NULL) { }

	EventListenerProxy *proxy;
	EventObject *target;
};

/*** MoonlightErrorEventArgsClass ******************************************************/
struct MoonlightErrorEventArgsType : MoonlightObjectType {
	MoonlightErrorEventArgsType ();
};

extern MoonlightErrorEventArgsType* MoonlightErrorEventArgsClass;

struct MoonlightErrorEventArgs : MoonlightObject {
	MoonlightErrorEventArgs (NPP instance) : MoonlightObject(instance) { }

	ErrorEventArgs *args;
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

	TimeSpan timespan;
};

/*** MoonlightMouseEventArgsClass  **************************************************************/
struct MoonlightMouseEventArgsType : MoonlightObjectType {
	MoonlightMouseEventArgsType ();
};

extern MoonlightMouseEventArgsType* MoonlightMouseEventArgsClass;

struct MoonlightMouseEventArgsObject : MoonlightObject {
	MoonlightMouseEventArgsObject (NPP instance)
	  : MoonlightObject (instance), state (0), x (-1), y (-1) { }

	virtual void Dispose ();
	int state;
	double x;
	double y;
};

/*** MoonlightKeyboardEventArgsClass  **************************************************************/
struct MoonlightKeyboardEventArgsType : MoonlightObjectType {
	MoonlightKeyboardEventArgsType ();
};

extern MoonlightKeyboardEventArgsType* MoonlightKeyboardEventArgsClass;

struct MoonlightKeyboardEventArgsObject : MoonlightObject {
	MoonlightKeyboardEventArgsObject (NPP instance)
	  : MoonlightObject (instance), state (0), key (0), platformcode (0) { }

	virtual void Dispose ();
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

/*** MoonlightContentClass ************************************************************/

struct MoonlightContentType : MoonlightObjectType {
	MoonlightContentType ();
};

struct MoonlightContentObject : MoonlightObject {
	MoonlightContentObject (NPP instance)
	  : MoonlightObject (instance),
	    resizeProxy (NULL), 
	    fullScreenChangeProxy (NULL)
	{
		registered_scriptable_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	virtual void Dispose ();

	GHashTable *registered_scriptable_objects;

	EventListenerProxy *resizeProxy;
	EventListenerProxy *fullScreenChangeProxy;
};

extern MoonlightContentType* MoonlightContentClass;

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
};

extern MoonlightDependencyObjectObject* DependencyObjectCreateWrapper (NPP instance, DependencyObject *obj);

/*** MoonlightCollectionClass ***************************************************/

struct MoonlightCollectionType : MoonlightDependencyObjectType {
	MoonlightCollectionType ();
};
extern MoonlightCollectionType* MoonlightCollectionClass;

/*** MoonlightStoryboardClass ***************************************************/

struct MoonlightStoryboardType : MoonlightDependencyObjectType {
	MoonlightStoryboardType ();
};

extern MoonlightStoryboardType* MoonlightStoryboardClass;

/*** MoonlightMediaElement ***************************************************/

struct MoonlightMediaElementType : MoonlightDependencyObjectType {
	MoonlightMediaElementType ();
};

extern MoonlightMediaElementType* MoonlightMediaElementClass;

/*** MoonlightImage ***************************************************/

struct MoonlightImageType : MoonlightDependencyObjectType {
	MoonlightImageType ();
};

extern MoonlightImageType* MoonlightImageClass;

/*** MoonlightImageBrush ***************************************************/

struct MoonlightImageBrushType : MoonlightDependencyObjectType {
	MoonlightImageBrushType ();
};

extern MoonlightImageBrushType* MoonlightImageBrushClass;

/*** MoonlightDownloader ***************************************************/

struct MoonlightDownloaderType : MoonlightDependencyObjectType {
	MoonlightDownloaderType ();
};

extern MoonlightDownloaderType* MoonlightDownloaderClass;

/*** MoonlightTextBlock ***************************************************/

struct MoonlightTextBlockType : MoonlightDependencyObjectType {
	MoonlightTextBlockType ();
};

extern MoonlightTextBlockType* MoonlightTextBlockClass;


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

	NPObject *real_object;
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
