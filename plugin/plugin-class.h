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

/*** EventListenerProxy */
typedef void (*EventArgsWrapper)(NPP instance, gpointer calldata, NPVariant *value);

class EventListenerProxy {
 public:
	EventListenerProxy (NPP instance, const char *event_name, const NPVariant *cb);
	~EventListenerProxy ();
	void AddHandler (EventObject *obj);
	void RemoveHandler (EventObject *obj);

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
	static void proxy_listener_to_javascript (EventObject *sender, gpointer calldata, gpointer closure);
};

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
	}

	NPP instance;
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
	MoonlightPoint (NPP instance) : MoonlightObject(instance), point (Point()) { }

	Point point;
};

/*** MoonlightRectClass  **************************************************************/
struct MoonlightRectType : MoonlightObjectType {
	MoonlightRectType ();
};

extern MoonlightRectType* MoonlightRectClass;

struct MoonlightRect : MoonlightObject {
	MoonlightRect (NPP instance) : MoonlightObject(instance), rect (Rect()) { }

	Rect rect;
};

/*** MoonlightMouseEventArgsClass  **************************************************************/
struct MoonlightMouseEventArgsType : MoonlightObjectType {
	MoonlightMouseEventArgsType ();
};

extern MoonlightMouseEventArgsType* MoonlightMouseEventArgsClass;

struct MoonlightMouseEventArgsObject : MoonlightObject {
	MoonlightMouseEventArgsObject (NPP instance)
	  : MoonlightObject (instance), state (0), position (NULL) { }

	int state;

	NPObject *position;
};

extern void MouseEventArgsPopulate (MoonlightMouseEventArgsObject *ea, MouseEventArgs *args);


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
	    resizeProxy (NULL)
	{
		registered_scriptable_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	}

	GHashTable *registered_scriptable_objects;

	EventListenerProxy *resizeProxy;
};

extern MoonlightContentType* MoonlightContentClass;

/*** MoonlightControlClass **********************************************************/

struct MoonlightControlType : MoonlightObjectType {
	MoonlightControlType ();
};
extern MoonlightControlType* MoonlightControlClass;

struct MoonlightControlObject : public MoonlightObject {
	MoonlightControlObject (NPP instance) : MoonlightObject (instance)
	{
		content = NPN_CreateObject (instance, MoonlightContentClass);
		settings = NPN_CreateObject (instance, MoonlightSettingsClass);
	}

	NPObject *content;
	NPObject *settings;
};

/*** MoonlightDependencyObjectClass ***************************************************/

struct MoonlightDependencyObjectType : MoonlightObjectType {
	MoonlightDependencyObjectType ();
};
extern MoonlightDependencyObjectType *MoonlightDependencyObjectClass;

struct MoonlightDependencyObjectObject : public MoonlightObject
{
	MoonlightDependencyObjectObject (NPP instance) : MoonlightObject (instance)
	{
		dob = NULL;
	}

	void SetDependencyObject (DependencyObject *dob)
	{
		this->dob = dob;
		dob->ref ();
	}

	DependencyObject *dob;
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

/*** MoonlightDownloader ***************************************************/

struct MoonlightDownloaderType : MoonlightDependencyObjectType {
	MoonlightDownloaderType ();
};

extern MoonlightDownloaderType* MoonlightDownloaderClass;


/*** MoonlightScriptableObject ***************************************************/

typedef void (*InvokeDelegate) (gpointer obj_handle, gpointer method_handle, Value** args, int arg_count, Value* return_value);
typedef void (*SetPropertyDelegate) (gpointer obj_handle, gpointer property_handle, Value *value);
typedef void (*GetPropertyDelegate) (gpointer obj_handle, gpointer property_handle, Value *value);
typedef void (*EventHandlerDelegate) (gpointer obj_handle, gpointer event_handle);

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
}

#endif /* PLUGIN_CLASS */
