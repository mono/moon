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

#include "moonlight.h"

class PluginClass : public NPClass {
 private:

 public:
	PluginClass ();
	virtual ~PluginClass ();

	virtual NPObject *ClassAllocate (NPP instance, NPClass *aClass);
	virtual void ClassDeallocate (NPObject *npobj);
	virtual void ClassInvalidate (NPObject *npobj);
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
	virtual bool ClassSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value);
	virtual bool ClassRemoveProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassHasMethod (NPObject *npobj, NPIdentifier name);
	virtual bool ClassInvoke (NPObject *npobj, NPIdentifier name,
	                         const NPVariant *args, uint32_t argCount,
	                         NPVariant *result);
	virtual bool ClassInvokeDefault (NPObject *npobj, const NPVariant *args,
	                                uint32_t argCount, NPVariant *result);
};

class PluginRootClass : public PluginClass
{
 private:
	NPP instance;
 public:	
	PluginRootClass (NPP instance);

	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
};

class PluginSettings : public PluginClass
{
 public:
	virtual bool ClassHasProperty (NPObject *npobj, NPIdentifier name);
	virtual bool ClassGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result);
};
