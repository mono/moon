/*
 * plugin-entry.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>
#include <string.h>

#include "moonlight.h"

#include "npapi.h"
#include "npupp.h"

// Global function table
static NPNetscapeFuncs MozillaFuncs;

/*** Wrapper functions ********************************************************/

void
NPN_Version (int *plugin_major, int *plugin_minor, int *netscape_major, int *netscape_minor)
{
	*plugin_major = NP_VERSION_MAJOR;
	*plugin_minor = NP_VERSION_MINOR;
	*netscape_major = MozillaFuncs.version >> 8;
	*netscape_minor = MozillaFuncs.version & 0xFF;
}

NPError
NPN_GetValue (NPP instance, NPNVariable variable, void *r_value)
{
	return CallNPN_GetValueProc (MozillaFuncs.getvalue, instance, variable, r_value);
}

NPError
NPN_SetValue (NPP instance, NPPVariable variable, void *value)
{
	return CallNPN_SetValueProc (MozillaFuncs.setvalue, instance, variable, value);
}

NPError
NPN_GetURL (NPP instance, const char *url, const char *window)
{
	return CallNPN_GetURLProc (MozillaFuncs.geturl, instance, url, window);
}

NPError
NPN_GetURLNotify (NPP instance, const char *url,
		  const char *window, void *notifyData)
{
	return CallNPN_GetURLNotifyProc (MozillaFuncs.geturlnotify, instance,
					 url, window, notifyData);
}

NPError
NPN_PostURL (NPP instance, const char *url, const char *window,
	     uint32 len, const char *buf, NPBool file)
{
	return CallNPN_PostURLProc (MozillaFuncs.posturl, instance, url, window, len, buf, file);
}

NPError
NPN_PostURLNotify (NPP instance, const char *url, const char *window,
		   uint32 len, const char *buf, NPBool file, void *notifyData)
{
	return CallNPN_PostURLNotifyProc (MozillaFuncs.posturlnotify, instance, url,
					  window, len, buf, file, notifyData);
}

NPError
NPN_RequestRead (NPStream *stream, NPByteRange *rangeList)
{
	return CallNPN_RequestReadProc (MozillaFuncs.requestread, stream, rangeList);
}

NPError
NPN_NewStream (NPP instance, NPMIMEType type, const char *window, NPStream **stream_ptr)
{
	return CallNPN_NewStreamProc (MozillaFuncs.newstream, instance,
				      type, window, stream_ptr);
}

int32_t
NPN_Write (NPP instance, NPStream *stream, int32_t len, void *buffer)
{
	return CallNPN_WriteProc (MozillaFuncs.write, instance, stream, len, buffer);
}

NPError
NPN_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	return CallNPN_DestroyStreamProc (MozillaFuncs.destroystream, instance, stream, reason);
}

void NPN_Status (NPP instance, const char *message)
{
	if (strstr (NPN_UserAgent (instance), "Firefox"))
		CallNPN_StatusProc (MozillaFuncs.status, instance, message);
}

const char *
NPN_UserAgent (NPP instance)
{
	return CallNPN_UserAgentProc (MozillaFuncs.uagent, instance);
}

void *
NPN_MemAlloc (uint32 size)
{
	return CallNPN_MemAllocProc (MozillaFuncs.memalloc, size);
}

void
NPN_MemFree (void *ptr)
{
	CallNPN_MemFreeProc (MozillaFuncs.memfree, ptr);
}

uint32
NPN_MemFlush (uint32 size)
{
	return CallNPN_MemFlushProc (MozillaFuncs.memflush, size);
}

void
NPN_ReloadPlugins (NPBool reloadPages)
{
	CallNPN_ReloadPluginsProc (MozillaFuncs.reloadplugins, reloadPages);
}

void
NPN_InvalidateRect (NPP instance, NPRect *invalidRect)
{
	CallNPN_InvalidateRectProc (MozillaFuncs.invalidaterect, instance, invalidRect);
}

void
NPN_InvalidateRegion (NPP instance, NPRegion invalidRegion)
{
	CallNPN_InvalidateRegionProc (MozillaFuncs.invalidateregion, instance, invalidRegion);
}

void
NPN_ForceRedraw (NPP instance)
{
	CallNPN_ForceRedrawProc (MozillaFuncs.forceredraw, instance);
}

/*** Runtime support **********************************************************/

NPIdentifier
NPN_GetStringIdentifier (const NPUTF8 *name)
{
	return CallNPN_GetStringIdentifierProc (MozillaFuncs.getstringidentifier, name);
}

void
NPN_GetStringIdentifiers (const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers)
{
	CallNPN_GetStringIdentifiersProc (MozillaFuncs.getstringidentifiers,
					  names, nameCount, identifiers);
}

NPIdentifier
NPN_GetIntIdentifier (int32_t intid)
{
	return CallNPN_GetIntIdentifierProc (MozillaFuncs.getintidentifier, intid);
}

bool
NPN_IdentifierIsString (NPIdentifier identifier)
{
	return CallNPN_IdentifierIsStringProc (MozillaFuncs.identifierisstring, identifier);
}

NPUTF8 *
NPN_UTF8FromIdentifier (NPIdentifier identifier)
{
	return CallNPN_UTF8FromIdentifierProc (MozillaFuncs.utf8fromidentifier, identifier);
}

int32_t
NPN_IntFromIdentifier (NPIdentifier identifier)
{
	return CallNPN_IntFromIdentifierProc (MozillaFuncs.intfromidentifier, identifier);
}

NPObject *
NPN_CreateObject (NPP npp, NPClass *aClass)
{
	return CallNPN_CreateObjectProc (MozillaFuncs.createobject, npp, aClass);
}

NPObject *
NPN_RetainObject (NPObject *obj)
{
	return CallNPN_RetainObjectProc (MozillaFuncs.retainobject, obj);
}

void
NPN_ReleaseObject (NPObject *obj)
{
	return CallNPN_ReleaseObjectProc (MozillaFuncs.releaseobject, obj);
}

bool
NPN_Invoke (NPP npp, NPObject *obj, NPIdentifier methodName,
	    const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return CallNPN_InvokeProc (MozillaFuncs.invoke, npp, obj, methodName, args, argCount, result);
}

bool
NPN_InvokeDefault (NPP npp, NPObject *obj, const NPVariant *args,
		   uint32_t argCount, NPVariant *result)
{
	return CallNPN_InvokeDefaultProc (MozillaFuncs.invokeDefault, npp, obj, args, argCount, result);
}

bool
NPN_Evaluate (NPP npp, NPObject *obj, NPString *script, NPVariant *result)
{
	return CallNPN_EvaluateProc (MozillaFuncs.evaluate, npp, obj, script, result);
}

bool
NPN_GetProperty (NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
	return CallNPN_GetPropertyProc (MozillaFuncs.getproperty, npp, obj, propertyName, result);
}

bool
NPN_SetProperty (NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value)
{
	return CallNPN_SetPropertyProc (MozillaFuncs.setproperty, npp, obj, propertyName, value);
}

bool
NPN_RemoveProperty (NPP npp, NPObject *obj, NPIdentifier propertyName)
{
	return CallNPN_RemovePropertyProc (MozillaFuncs.removeproperty, npp, obj, propertyName);
}

bool
NPN_HasProperty (NPP npp, NPObject *obj, NPIdentifier propertyName)
{
	return CallNPN_HasPropertyProc (MozillaFuncs.hasproperty, npp, obj, propertyName);
}

bool
NPN_Enumerate (NPP npp, NPObject *obj, NPIdentifier **values,
	       uint32_t *count)
{
	return CallNPN_EnumerateProc (MozillaFuncs.enumerate, npp, obj, values, count);
}

bool
NPN_HasMethod (NPP npp, NPObject *obj, NPIdentifier methodName)
{
	return CallNPN_HasMethodProc (MozillaFuncs.hasmethod, npp, obj, methodName);
}

void
NPN_ReleaseVariantValue (NPVariant *variant)
{
	CallNPN_ReleaseVariantValueProc (MozillaFuncs.releasevariantvalue, variant);
}

void NPN_SetException (NPObject *obj, const NPUTF8 *message)
{
	CallNPN_SetExceptionProc (MozillaFuncs.setexception, obj, message);
}

/*** Popup support ************************************************************/

void
NPN_PushPopupsEnabledState (NPP instance, NPBool enabled)
{
	CallNPN_PushPopupsEnabledStateProc (MozillaFuncs.pushpopupsenabledstate, instance, enabled);
}

void
NPN_PopPopupsEnabledState (NPP instance)
{
	CallNPN_PopPopupsEnabledStateProc (MozillaFuncs.poppopupsenabledstate, instance);
}

/*** These functions are located automagically by mozilla *********************/

char *
LOADER_RENAMED_SYM(NP_GetMIMEDescription) (void)
{
	return NPP_GetMIMEDescription ();
}

NPError
LOADER_RENAMED_SYM(NP_GetValue) (void *future, NPPVariable variable, void *value)
{
	return NPP_GetValue ((NPP) future, variable, value);
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
	NPBool supportsXEmbed = FALSE;
	NPNToolkitType toolkit = (NPNToolkitType) 0;

	// XEmbed ?
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
				    NPNVSupportsXEmbedBool,
				    (void *) &supportsXEmbed);

	if (err != NPERR_NO_ERROR || supportsXEmbed != TRUE)
		g_warning ("It appears your browser may not support XEmbed");

	// GTK+ ?
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
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
	plugin_funcs->newp          = NewNPP_NewProc (NPP_New);
	plugin_funcs->destroy       = NewNPP_DestroyProc (NPP_Destroy);
	plugin_funcs->setwindow     = NewNPP_SetWindowProc (NPP_SetWindow);
	plugin_funcs->newstream     = NewNPP_NewStreamProc (NPP_NewStream);
	plugin_funcs->destroystream = NewNPP_DestroyStreamProc (NPP_DestroyStream);
	plugin_funcs->asfile        = NewNPP_StreamAsFileProc (NPP_StreamAsFile);
	plugin_funcs->writeready    = NewNPP_WriteReadyProc (NPP_WriteReady);
	plugin_funcs->write         = NewNPP_WriteProc (NPP_Write);
	plugin_funcs->print         = NewNPP_PrintProc (NPP_Print);
	plugin_funcs->urlnotify     = NewNPP_URLNotifyProc (NPP_URLNotify);
	plugin_funcs->event         = NewNPP_HandleEventProc (NPP_HandleEvent);
#ifdef OJI
	plugin_funcs->javaClass     = NULL;
#endif
	if (mozilla_funcs->version >= NPVERS_HAS_NPRUNTIME_SCRIPTING) {
		plugin_funcs->getvalue    = NewNPP_GetValueProc (NPP_GetValue);
		plugin_funcs->setvalue    = NewNPP_SetValueProc (NPP_SetValue);
	}

	return NPP_Initialize ();
}

NPError OSCALL
LOADER_RENAMED_SYM(NP_Shutdown) (void)
{
	NPP_Shutdown ();
	return NPERR_NO_ERROR;
}
