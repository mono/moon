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

/*** MoonlightPointClass  **************************************************************/
struct MoonlightPointType : NPClass {
	MoonlightPointType ();
};

extern MoonlightPointType* MoonlightPointClass;

struct MoonlightPoint : NPObject {
	MoonlightPoint () { x = y = 0.0; }

	double x;
	double y;
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

/*** MoonlightSettingsClass ***********************************************************/

struct MoonlightSettingsType : MoonlightObjectType {
	MoonlightSettingsType ();
};
extern MoonlightSettingsType* MoonlightSettingsClass;

/*** MoonlightContentClass ************************************************************/

struct MoonlightContentType : MoonlightObjectType {
	MoonlightContentType ();
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

/*** MoonlightCollectionClass ***************************************************/

struct MoonlightStoryboardType : MoonlightDependencyObjectType {
  MoonlightStoryboardType ();
};

extern MoonlightStoryboardType* MoonlightStoryboardClass;

#endif /* PLUGIN_CLASS */
