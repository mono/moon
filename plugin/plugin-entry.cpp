/*
 * plugin-entry.cpp: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>

#include "moonlight.h"
#include "deployment.h"

// Global function table
static NPNetscapeFuncs MozillaFuncs;

/*** Wrapper functions ********************************************************/

void
MOON_NPN_Version (int *plugin_major, int *plugin_minor, int *netscape_major, int *netscape_minor)
{
	*plugin_major = NP_VERSION_MAJOR;
	*plugin_minor = NP_VERSION_MINOR;
	*netscape_major = MozillaFuncs.version >> 8;
	*netscape_minor = MozillaFuncs.version & 0xFF;
}

NPError
MOON_NPN_GetValue (NPP instance, NPNVariable variable, void *r_value)
{
	// This will end up calling Deployment::SetCurrent before mono has initialized, which crashes.
	// in any case NPN_GetValue should not end up running js code nor in other plugins.
	// DeploymentStack deployment_push_pop;
	return MozillaFuncs.getvalue (instance, variable, r_value);
}

NPError
MOON_NPN_SetValue (NPP instance, NPPVariable variable, void *value)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.setvalue (instance, variable, value);
}

NPError
MOON_NPN_GetURL (NPP instance, const char *url, const char *window)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.geturl (instance, url, window);
}

NPError
MOON_NPN_GetURLNotify (NPP instance, const char *url,
		  const char *window, void *notifyData)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.geturlnotify (instance,
					  url, window, notifyData);
}

NPError
MOON_NPN_PostURL (NPP instance, const char *url, const char *window,
	     uint32_t len, const char *buf, NPBool file)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.posturl (instance, url, window, len, buf, file);
}

NPError
MOON_NPN_PostURLNotify (NPP instance, const char *url, const char *window,
		   uint32_t len, const char *buf, NPBool file, void *notifyData)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.posturlnotify (instance, url,
					   window, len, buf, file, notifyData);
}

NPError
MOON_NPN_RequestRead (NPStream *stream, NPByteRange *rangeList)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.requestread (stream, rangeList);
}

NPError
MOON_NPN_NewStream (NPP instance, NPMIMEType type, const char *window, NPStream **stream_ptr)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.newstream (instance,
				       type, window, stream_ptr);
}

int32_t
MOON_NPN_Write (NPP instance, NPStream *stream, int32_t len, void *buffer)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.write (instance, stream, len, buffer);
}

NPError
MOON_NPN_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.destroystream (instance, stream, reason);
}

void MOON_NPN_Status (NPP instance, const char *message)
{
	DeploymentStack deployment_push_pop;
	if (strstr (MOON_NPN_UserAgent (instance), "Firefox"))
		MozillaFuncs.status (instance, message);
}

const char *
MOON_NPN_UserAgent (NPP instance)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.uagent (instance);
}

void *
MOON_NPN_MemAlloc (uint32_t size)
{
	return MozillaFuncs.memalloc (size);
}

void
MOON_NPN_MemFree (void *ptr)
{
	MozillaFuncs.memfree (ptr);
}

uint32_t
MOON_NPN_MemFlush (uint32_t size)
{
	return MozillaFuncs.memflush (size);
}

void
MOON_NPN_ReloadPlugins (NPBool reloadPages)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.reloadplugins (reloadPages);
}

void
MOON_NPN_InvalidateRect (NPP instance, NPRect *invalidRect)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.invalidaterect (instance, invalidRect);
}

void
MOON_NPN_InvalidateRegion (NPP instance, NPRegion invalidRegion)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.invalidateregion (instance, invalidRegion);
}

void
MOON_NPN_ForceRedraw (NPP instance)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.forceredraw (instance);
}

/*** Runtime support **********************************************************/

NPIdentifier
MOON_NPN_GetStringIdentifier (const NPUTF8 *name)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.getstringidentifier (name);
}

void
MOON_NPN_GetStringIdentifiers (const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.getstringidentifiers (names, nameCount, identifiers);
}

NPIdentifier
MOON_NPN_GetIntIdentifier (int32_t intid)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.getintidentifier (intid);
}

bool
MOON_NPN_IdentifierIsString (NPIdentifier identifier)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.identifierisstring (identifier);
}

NPUTF8 *
MOON_NPN_UTF8FromIdentifier (NPIdentifier identifier)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.utf8fromidentifier (identifier);
}

int32_t
MOON_NPN_IntFromIdentifier (NPIdentifier identifier)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.intfromidentifier (identifier);
}

NPObject *
MOON_NPN_CreateObject (NPP npp, NPClass *aClass)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.createobject (npp, aClass);
}

NPObject *
MOON_NPN_RetainObject (NPObject *obj)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.retainobject (obj);
}

void
MOON_NPN_ReleaseObject (NPObject *obj)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.releaseobject (obj);
}

bool
MOON_NPN_Invoke (NPP npp, NPObject *obj, NPIdentifier methodName,
	    const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.invoke (npp, obj, methodName, args, argCount, result);
}

bool
MOON_NPN_InvokeDefault (NPP npp, NPObject *obj, const NPVariant *args,
		   uint32_t argCount, NPVariant *result)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.invokeDefault (npp, obj, args, argCount, result);
}

bool
MOON_NPN_Evaluate (NPP npp, NPObject *obj, NPString *script, NPVariant *result)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.evaluate (npp, obj, script, result);
}

bool
MOON_NPN_GetProperty (NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.getproperty (npp, obj, propertyName, result);
}

bool
MOON_NPN_SetProperty (NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.setproperty (npp, obj, propertyName, value);
}

bool
MOON_NPN_RemoveProperty (NPP npp, NPObject *obj, NPIdentifier propertyName)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.removeproperty (npp, obj, propertyName);
}

bool
MOON_NPN_HasProperty (NPP npp, NPObject *obj, NPIdentifier propertyName)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.hasproperty (npp, obj, propertyName);
}

bool
MOON_NPN_Enumerate (NPP npp, NPObject *obj, NPIdentifier **values,
	       uint32_t *count)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.enumerate (npp, obj, values, count);
}

bool
MOON_NPN_HasMethod (NPP npp, NPObject *obj, NPIdentifier methodName)
{
	DeploymentStack deployment_push_pop;
	return MozillaFuncs.hasmethod (npp, obj, methodName);
}

void
MOON_NPN_ReleaseVariantValue (NPVariant *variant)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.releasevariantvalue (variant);
}

void MOON_NPN_SetException (NPObject *obj, const NPUTF8 *message)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.setexception (obj, message);
}

/*** Popup support ************************************************************/

void
MOON_NPN_PushPopupsEnabledState (NPP instance, NPBool enabled)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.pushpopupsenabledstate (instance, enabled);
}

void
MOON_NPN_PopPopupsEnabledState (NPP instance)
{
	DeploymentStack deployment_push_pop;
	MozillaFuncs.poppopupsenabledstate (instance);
}

/*** These functions are located automagically by mozilla *********************/

NPError
LOADER_RENAMED_SYM(NP_GetValue) (void *future, NPPVariable variable, void *value)
{
	return MOON_NPP_GetValue ((NPP) future, variable, value);
}

NPError OSCALL
LOADER_RENAMED_SYM(NP_Initialize) (NPNetscapeFuncs *mozilla_funcs, NPPluginFuncs *plugin_funcs)
{
	if (mozilla_funcs == NULL || plugin_funcs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	// remove these checks, since we compile against trunk firefox
	// np*.h now, sizeof (struct) will be > if we run against
	// firefox 2.
	//
	// everything is ok, though.  we just need to check against
	// the NPVERS_HAS_* defines when filling in the function
	// table.
#if 0
	if (mozilla_funcs->size < sizeof (NPNetscapeFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;


	if (plugin_funcs->size < sizeof (NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;
#endif

	if ((mozilla_funcs->version >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	NPError err = NPERR_NO_ERROR;
	guint32 supportsXEmbed = FALSE; // NPBool + padding
	NPNToolkitType toolkit = (NPNToolkitType) 0;

	// XEmbed ?
	err = mozilla_funcs->getvalue (NULL,
				       NPNVSupportsXEmbedBool,
				       (void *) &supportsXEmbed);

	if (err != NPERR_NO_ERROR || supportsXEmbed != TRUE)
		g_warning ("It appears your browser may not support XEmbed");

	// GTK+ ?
	err = mozilla_funcs->getvalue (NULL,
				       NPNVToolkit,
				       (void *) &toolkit);

	if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2)
		g_warning ("It appears your browser may not support Gtk2");

	MozillaFuncs.size                    = mozilla_funcs->size;
	MozillaFuncs.version                 = mozilla_funcs->version;
	MozillaFuncs.geturlnotify            = mozilla_funcs->geturlnotify;
	MozillaFuncs.geturl                  = mozilla_funcs->geturl;
	MozillaFuncs.posturlnotify           = mozilla_funcs->posturlnotify;
	MozillaFuncs.posturl                 = mozilla_funcs->posturl;
	MozillaFuncs.requestread             = mozilla_funcs->requestread;
	MozillaFuncs.newstream               = mozilla_funcs->newstream;
	MozillaFuncs.write                   = mozilla_funcs->write;
	MozillaFuncs.destroystream           = mozilla_funcs->destroystream;
	MozillaFuncs.status                  = mozilla_funcs->status;
	MozillaFuncs.uagent                  = mozilla_funcs->uagent;
	MozillaFuncs.memalloc                = mozilla_funcs->memalloc;
	MozillaFuncs.memfree                 = mozilla_funcs->memfree;
	MozillaFuncs.memflush                = mozilla_funcs->memflush;
	MozillaFuncs.reloadplugins           = mozilla_funcs->reloadplugins;
	MozillaFuncs.getJavaEnv              = mozilla_funcs->getJavaEnv;
	MozillaFuncs.getJavaPeer             = mozilla_funcs->getJavaPeer;
	MozillaFuncs.getvalue                = mozilla_funcs->getvalue;
	MozillaFuncs.setvalue                = mozilla_funcs->setvalue;
	MozillaFuncs.invalidaterect          = mozilla_funcs->invalidaterect;
	MozillaFuncs.invalidateregion        = mozilla_funcs->invalidateregion;
	MozillaFuncs.forceredraw             = mozilla_funcs->forceredraw;

	if (mozilla_funcs->version >= NPVERS_HAS_NPRUNTIME_SCRIPTING) {
		MozillaFuncs.getstringidentifier    = mozilla_funcs->getstringidentifier;
		MozillaFuncs.getstringidentifiers   = mozilla_funcs->getstringidentifiers;
		MozillaFuncs.getintidentifier       = mozilla_funcs->getintidentifier;
		MozillaFuncs.identifierisstring     = mozilla_funcs->identifierisstring;
		MozillaFuncs.utf8fromidentifier     = mozilla_funcs->utf8fromidentifier;
		MozillaFuncs.intfromidentifier      = mozilla_funcs->intfromidentifier;
		MozillaFuncs.createobject           = mozilla_funcs->createobject;
		MozillaFuncs.retainobject           = mozilla_funcs->retainobject;
		MozillaFuncs.releaseobject          = mozilla_funcs->releaseobject;
		MozillaFuncs.invoke                 = mozilla_funcs->invoke;
		MozillaFuncs.invokeDefault          = mozilla_funcs->invokeDefault;
		MozillaFuncs.evaluate               = mozilla_funcs->evaluate;
		MozillaFuncs.getproperty            = mozilla_funcs->getproperty;
		MozillaFuncs.setproperty            = mozilla_funcs->setproperty;
		MozillaFuncs.removeproperty         = mozilla_funcs->removeproperty;
		MozillaFuncs.hasproperty            = mozilla_funcs->hasproperty;
		MozillaFuncs.hasmethod              = mozilla_funcs->hasmethod;
		MozillaFuncs.releasevariantvalue    = mozilla_funcs->releasevariantvalue;
		MozillaFuncs.setexception           = mozilla_funcs->setexception;
	}

	if (mozilla_funcs->version >= NPVERS_HAS_NPOBJECT_ENUM) {
		MozillaFuncs.enumerate = mozilla_funcs->enumerate;
	}

	if (mozilla_funcs->version >= NPVERS_HAS_POPUPS_ENABLED_STATE) {
		MozillaFuncs.pushpopupsenabledstate = mozilla_funcs->pushpopupsenabledstate;
		MozillaFuncs.poppopupsenabledstate  = mozilla_funcs->poppopupsenabledstate;
	}

	if (plugin_funcs->size < sizeof (NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	plugin_funcs->version       = ((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR);
	plugin_funcs->size          = sizeof (NPPluginFuncs);
	plugin_funcs->newp          = MOON_NPP_New;
	plugin_funcs->destroy       = MOON_NPP_Destroy;
	plugin_funcs->setwindow     = MOON_NPP_SetWindow;
	plugin_funcs->newstream     = MOON_NPP_NewStream;
	plugin_funcs->destroystream = MOON_NPP_DestroyStream;
	plugin_funcs->asfile        = MOON_NPP_StreamAsFile;
	plugin_funcs->writeready    = MOON_NPP_WriteReady;
	plugin_funcs->write         = MOON_NPP_Write;
	plugin_funcs->print         = MOON_NPP_Print;
	plugin_funcs->urlnotify     = MOON_NPP_URLNotify;
	plugin_funcs->event         = MOON_NPP_HandleEvent;
#ifdef OJI
	plugin_funcs->javaClass     = NULL;
#endif
	if (mozilla_funcs->version >= NPVERS_HAS_NPRUNTIME_SCRIPTING) {
		plugin_funcs->getvalue    = MOON_NPP_GetValue;
		plugin_funcs->setvalue    = MOON_NPP_SetValue;
	}

	return MOON_NPP_Initialize ();
}

NPError OSCALL
LOADER_RENAMED_SYM(NP_Shutdown) (void)
{
	MOON_NPP_Shutdown ();
	return NPERR_NO_ERROR;
}
