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

/*** Macros *******************************************************************/

#define NPID(x) NPN_GetStringIdentifier (x)

#define PLUGIN_PROPERTIES(x) \
	bool HasProperty (MoonlightObject *npobj, NPIdentifier name); \
	virtual bool GetProperty (MoonlightObject *npobj, NPIdentifier name, NPVariant *result); \
	virtual bool SetProperty (MoonlightObject *npobj, NPIdentifier name, const NPVariant *value);

#define PLUGIN_METHODS(x) \
	bool HasMethod (MoonlightObject *npobj, NPIdentifier name); \
	virtual bool Invoke (MoonlightObject *npobj, NPIdentifier name, const NPVariant *args,  \
			     uint32_t argCount, NPVariant *result);

#define HAS_PROPERTY(x,v) \
		(IndexOf (v, x, (sizeof (x) / sizeof (char *))) > -1)
#define HAS_METHOD(x,v) \
		(IndexOf (v, x, (sizeof (x) / sizeof (char *))) > -1)

/*** MoonlightObject *************************************************************/

class MoonlightObject : public NPObject
{
 public:
	NPP instance;

	MoonlightObject (NPP instance)
	{
		this->instance = instance;
	}
};

/*** MoonlightClass **************************************************************/

class MoonlightClass : public NPClass
{
 public:
	virtual MoonlightObject* AllocateObject (NPP instance);
	virtual void DeallocateObject (MoonlightObject *npobj);
	virtual void InvalidateObject (MoonlightObject *npobj);

	virtual bool HasProperty (MoonlightObject *npobj, NPIdentifier name);
	virtual bool GetProperty (MoonlightObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool SetProperty (MoonlightObject *npobj, NPIdentifier name, const NPVariant *value);
	virtual bool RemoveProperty (MoonlightObject *npobj, NPIdentifier name);

	virtual bool HasMethod (MoonlightObject *npobj, NPIdentifier name);
	virtual bool Invoke (MoonlightObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result);
	virtual bool InvokeDefault (MoonlightObject *npobj, const NPVariant *args,
				    uint32_t argCount, NPVariant *result);

 protected:
	int IndexOf (NPIdentifier name, const char *const names[], int count);
	void StringToNPVariant (char *value, NPVariant *result);

	MoonlightClass ();
	virtual ~MoonlightClass ();

	/* these are the entry points through which the NPN_ api calls our code */
	static NPObject* moonlightAllocate (NPP instance, NPClass *aClass);
	static void moonlightDeallocate (NPObject *npobj);
	static void moonlightInvalidate (NPObject *npobj);
	static bool moonlightHasProperty (NPObject *npobj, NPIdentifier name);
	static bool moonlightGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
	static bool moonlightSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value);
	static bool moonlightRemoveProperty (NPObject *npobj, NPIdentifier name);
	static bool moonlightHasMethod (NPObject *npobj, NPIdentifier name);
	static bool moonlightInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, 
				     uint32_t argCount, NPVariant *result);
	static bool moonlightInvokeDefault (NPObject *npobj, const NPVariant *args, 
					    uint32_t argCount, NPVariant *result);
};

/*** MoonlightSettingsClass ***********************************************************/

class MoonlightSettingsClass : public MoonlightClass
{
 public:
	PLUGIN_PROPERTIES (properties);

	static MoonlightSettingsClass* Class() {
		if (_class == NULL)
			_class = new MoonlightSettingsClass ();
		return _class;
	}

 public:
	MoonlightSettingsClass ();

 private:
	static MoonlightSettingsClass *_class;

	static const char *const properties [];
};

/*** MoonlightContentClass ************************************************************/

// TODO:
//onFullScreenChange = "eventhandlerFunction"
//onResize = "eventhandlerFunction"

class MoonlightContentClass : public MoonlightClass
{
 public:
	PLUGIN_PROPERTIES (properties);
	PLUGIN_METHODS (methods);

	static MoonlightContentClass* Class() {
		if (_class == NULL)
			_class = new MoonlightContentClass ();
		return _class;
	}

 protected:
	MoonlightContentClass ();

 private:
	static MoonlightContentClass *_class;

	static const char *const properties [];
	static const char *const methods [];
};

/*** MoonlightControlClass **********************************************************/

class MoonlightControlObject : public MoonlightObject
{
 public:
	NPObject *content;
	NPObject *settings;

	MoonlightControlObject (NPP instance) : MoonlightObject (instance)
	{
		content = NPN_CreateObject (instance, MoonlightContentClass::Class());
		settings = NPN_CreateObject (instance, MoonlightSettingsClass::Class());
	}
};

// TODO:
// onError = "eventhandlerFunction"

class MoonlightControlClass : public MoonlightClass
{
 public:
	virtual MoonlightObject* AllocateObject (NPP instance);
	virtual void InvalidateObject (MoonlightObject *npobj);

	PLUGIN_PROPERTIES (properties);
	PLUGIN_METHODS (methods);

	static MoonlightControlClass* Class() {
		if (_class == NULL)
			_class = new MoonlightControlClass ();
		return _class;
	}

 protected:
	MoonlightControlClass ();

 private:
	static MoonlightControlClass *_class;

	static const char *const properties [];
	static const char *const methods [];
};

/*** MoonlightDependencyObjectClass ***************************************************/

class MoonlightDependencyObjectObject : public MoonlightObject
{
 public:
	DependencyObject *dob;

	MoonlightDependencyObjectObject (NPP instance) : MoonlightObject (instance)
	{
		dob = NULL;
	}

	void SetDependencyObject (DependencyObject *dob)
	{
		this->dob = dob;
		dob->ref ();
	}
};

class MoonlightDependencyObjectClass : public MoonlightClass
{
 public:
	virtual MoonlightObject *AllocateObject (NPP instance);
	virtual void InvalidateObject (MoonlightObject *npobj);

	virtual bool HasProperty (MoonlightObject *npobj, NPIdentifier name);
	virtual bool GetProperty (MoonlightObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool SetProperty (MoonlightObject *npobj, NPIdentifier name, const NPVariant *value);

	virtual bool HasMethod (MoonlightObject *npobj, NPIdentifier name);
	virtual bool Invoke (MoonlightObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result);

	static MoonlightDependencyObjectClass* Class() {
		if (_class == NULL)
			_class = new MoonlightDependencyObjectClass ();
		return _class;
	}

	static MoonlightDependencyObjectObject* CreateWrapper (NPP instance, DependencyObject *obj);

 protected:
	MoonlightDependencyObjectClass ();

 private:
	DependencyProperty* GetDependencyProperty (DependencyObject *obj, char *attrname);

	static MoonlightDependencyObjectClass *_class;

	static const char *const properties [];
	static const char *const methods [];
};

/*** MoonlightCollectionClass ***************************************************/

class MoonlightCollectionClass : public MoonlightDependencyObjectClass
{
 public:
	virtual bool HasProperty (MoonlightObject *npobj, NPIdentifier name);
	virtual bool GetProperty (MoonlightObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool HasMethod (MoonlightObject *npobj, NPIdentifier name);
	virtual bool Invoke (MoonlightObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result);

	static MoonlightCollectionClass * Class() {
		if (_class == NULL)
			_class = new MoonlightCollectionClass ();
		return _class;
	}

 protected:
	MoonlightCollectionClass ();

 private:
	static MoonlightCollectionClass *_class;

	static const char *const properties [];
	static const char *const methods [];
};

/*** MoonlightCollectionClass ***************************************************/

class MoonlightStoryboardClass : public MoonlightDependencyObjectClass /* this should really inherit from the parallel timeline wrapper class */
{
 public:
	virtual bool HasMethod (MoonlightObject *npobj, NPIdentifier name);
	virtual bool Invoke (MoonlightObject *npobj, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount,
			     NPVariant *result);

	static MoonlightStoryboardClass * Class() {
		if (_class == NULL)
			_class = new MoonlightStoryboardClass ();
		return _class;
	}

 protected:
	MoonlightStoryboardClass ();

 private:
	static MoonlightStoryboardClass *_class;

	static const char *const methods [];
};

#endif /* PLUGIN_CLASS */
