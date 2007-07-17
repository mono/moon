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
#include "plugin.h"

void plugin_init_classes (void);

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
	    resizeScript (NULL), resizeMethodName (NULL),
	    resizeIsScript (false), resizeSet (false)
	{
		registered_scriptable_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	}

	GHashTable *registered_scriptable_objects;

	NPObject *resizeScript;
	char *resizeMethodName;
	bool resizeIsScript;
	bool resizeSet;
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

typedef void (*InvokeDelegate) (gpointer obj_handle, gpointer method_handle, Value* args, int arg_count, Value* return_value);
typedef void (*SetPropertyDelegate) (gpointer obj_handle, gpointer property_handle, Value value);
typedef void (*GetPropertyDelegate) (gpointer obj_handle, gpointer property_handle, Value *value);

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
	}

	gpointer managed_scriptable;
	GHashTable *properties;
	GHashTable *methods;

	InvokeDelegate invoke;
	SetPropertyDelegate setprop;
	GetPropertyDelegate getprop;
};

extern "C" {
	// These are meant to be called by System.Silverlight.dll

	MoonlightScriptableObjectObject* moonlight_scriptable_object_wrapper_create (PluginInstance *plugin, gpointer scriptable,
										     InvokeDelegate invoke,
										     SetPropertyDelegate setprop,
										     GetPropertyDelegate getprop);

	void moonlight_scriptable_object_add_property (PluginInstance *plugin,
						       MoonlightScriptableObjectObject *obj,
						       gpointer property_handle,
						       char *property_name,
						       int property_type,
						       bool can_read,
						       bool can_write);

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
