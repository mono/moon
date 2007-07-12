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
	}

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

struct MoonlightScriptableObjectType : MoonlightObjectType {
	MoonlightScriptableObjectType ();
};

extern MoonlightScriptableObjectType* MoonlightScriptableObjectClass;

struct MoonlightScriptableObjectObject : public MoonlightObject
{
	MoonlightScriptableObjectObject (NPP instance) : MoonlightObject (instance)
	{
		scriptable = NULL;
	}

	// XXX this should be a MonoObject?  what does a GCHandle in
	// p/invoke result in on the unmanaged side?
	void *scriptable;
};

#endif /* PLUGIN_CLASS */
