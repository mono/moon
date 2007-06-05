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
#include "npapi.h"
#include "npupp.h"

static NPNetscapeFuncs MozillaFuncs; // Global function table

// TODO: Just to fix NPN->NPP issue, must be checked later.
NPError NPP_SetValueX (NPP instance, NPPVariable variable, void *value);

/*** Wrapper functions ********************************************************/

void
NPN_Version (int* plugin_major, int* plugin_minor, int* netscape_major, int* netscape_minor)
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
NPN_GetURL (NPP instance, const char* url, const char* window)
{
	return CallNPN_GetURLProc (MozillaFuncs.geturl, instance, url, window);
}

NPError
NPN_GetURLNotify (NPP instance, const char* url, const char* window, void* notifyData)
{
	return CallNPN_GetURLNotifyProc (MozillaFuncs.geturlnotify, instance, url, window, notifyData);
}

NPError
NPN_PostURL (NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file)
{
	return CallNPN_PostURLProc (MozillaFuncs.posturl, instance, url, window, len, buf, file);
}

NPError
NPN_PostURLNotify (NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
	return CallNPN_PostURLNotifyProc (MozillaFuncs.posturlnotify, instance, url, window, len, buf, file, notifyData);
}

NPError
NPN_RequestRead (NPStream* stream, NPByteRange* rangeList)
{
	return CallNPN_RequestReadProc (MozillaFuncs.requestread, stream, rangeList);
}

NPError
NPN_NewStream (NPP instance, NPMIMEType type, const char *window, NPStream** stream_ptr)
{
	return CallNPN_NewStreamProc (MozillaFuncs.newstream, instance, type, window, stream_ptr);
}

int32
NPN_Write (NPP instance, NPStream* stream, int32 len, void* buffer)
{
	return CallNPN_WriteProc (MozillaFuncs.write, instance, stream, len, buffer);
}

NPError
NPN_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
	return CallNPN_DestroyStreamProc (MozillaFuncs.destroystream, instance, stream, reason);
}

void
NPN_Status (NPP instance, const char* message)
{
	CallNPN_StatusProc (MozillaFuncs.status, instance, message);
}

const char*
NPN_UserAgent (NPP instance)
{
	return CallNPN_UserAgentProc (MozillaFuncs.uagent, instance);
}

void*
NPN_MemAlloc (uint32 size)
{
	return CallNPN_MemAllocProc (MozillaFuncs.memalloc, size);
}

void
NPN_MemFree (void* ptr)
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

/*** NPRuntime support ********************************************************/

NPIdentifier
NPN_GetStringIdentifier (const NPUTF8 *name)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_GetStringIdentifierProc (MozillaFuncs.getstringidentifier, name);

    return NULL;
}

void
NPN_GetStringIdentifiers (const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		CallNPN_GetStringIdentifiersProc (MozillaFuncs.getstringidentifiers, names, nameCount, identifiers);
}

NPIdentifier
NPN_GetIntIdentifier (int32_t intid)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_GetIntIdentifierProc (MozillaFuncs.getintidentifier, intid);

    return NULL;
}

bool
NPN_IdentifierIsString (NPIdentifier identifier)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_IdentifierIsStringProc (MozillaFuncs.identifierisstring, identifier);

    return false;
}

NPUTF8
*NPN_UTF8FromIdentifier (NPIdentifier identifier)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_UTF8FromIdentifierProc (MozillaFuncs.utf8fromidentifier, identifier);

    return NULL;
}

int32_t
NPN_IntFromIdentifier (NPIdentifier identifier)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_IntFromIdentifierProc (MozillaFuncs.intfromidentifier, identifier);

    return 0;
}

NPObject
*NPN_CreateObject (NPP instance, NPClass *aClass)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_CreateObjectProc (MozillaFuncs.createobject, instance, aClass);
	return NULL;
}

NPObject
*NPN_RetainObject (NPObject *npobj)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_RetainObjectProc (MozillaFuncs.retainobject, npobj);
	return NULL;
}

void
NPN_ReleaseObject (NPObject *npobj)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		CallNPN_ReleaseObjectProc (MozillaFuncs.releaseobject, npobj);
}

bool
NPN_Invoke (NPP instance, NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_InvokeProc (MozillaFuncs.invoke, instance, npobj, methodName, args, argCount, result);

    return false;
}

bool
NPN_InvokeDefault (NPP instance, NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_InvokeDefaultProc (MozillaFuncs.invokeDefault, instance, npobj, args, argCount, result);

    return false;
}

bool
NPN_Evaluate (NPP instance, NPObject *npobj, NPString *script, NPVariant *result)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_EvaluateProc (MozillaFuncs.evaluate, instance, npobj, script, result);

    return false;
}

bool
NPN_GetProperty (NPP instance, NPObject *npobj, NPIdentifier propertyName, NPVariant *result)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_GetPropertyProc (MozillaFuncs.getproperty, instance, npobj, propertyName, result);

    return false;
}

bool
NPN_SetProperty (NPP instance, NPObject *npobj, NPIdentifier propertyName, const NPVariant *value)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_SetPropertyProc (MozillaFuncs.setproperty, instance, npobj, propertyName, value);

    return false;
}

bool
NPN_RemoveProperty (NPP instance, NPObject *npobj, NPIdentifier propertyName)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_RemovePropertyProc (MozillaFuncs.removeproperty, instance, npobj, propertyName);

    return false;
}

bool
NPN_HasProperty (NPP instance, NPObject *npobj, NPIdentifier propertyName)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_HasPropertyProc (MozillaFuncs.hasproperty, instance, npobj, propertyName);

    return false;
}

bool
NPN_HasMethod (NPP instance, NPObject *npobj, NPIdentifier methodName)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		return CallNPN_HasMethodProc (MozillaFuncs.hasmethod, instance, npobj, methodName);

    return false;
}

void
NPN_ReleaseVariantValue (NPVariant *variant)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		CallNPN_ReleaseVariantValueProc (MozillaFuncs.releasevariantvalue, variant);
}

void
NPN_SetException (NPObject *npobj, const NPUTF8 *message)
{
	int navMinorVers = MozillaFuncs.version & 0xFF;
	if (navMinorVers >= 14)
		CallNPN_SetExceptionProc (MozillaFuncs.setexception, npobj, message);
}

/*** These functions are located automagically by mozilla *********************/

NPError
NP_GetValue (void* future, NPPVariable variable, void *result)
{
	return NPP_GetValue (NULL, variable, result);
}

char *
NP_GetMIMEDescription (void)
{
	return NPP_GetMIMEDescription ();
}

NPError
NP_Initialize (NPNetscapeFuncs * mozilla_funcs, NPPluginFuncs * plugin_funcs)
{
	NPError err = NPERR_NO_ERROR;
	NPBool supportsXEmbed = PR_FALSE;
	NPNToolkitType toolkit = (NPNToolkitType) 0;

	/* XEMBED? */
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
								NPNVSupportsXEmbedBool,
								(void *)&supportsXEmbed);

	if (err != NPERR_NO_ERROR || supportsXEmbed != PR_TRUE)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	/* GTK+ 2.x Moz? */
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
								NPNVToolkit, 
								(void *)&toolkit);

	if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	if (mozilla_funcs == NULL || plugin_funcs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if ((mozilla_funcs->version >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	if (mozilla_funcs->size < sizeof (NPNetscapeFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if (plugin_funcs->size < sizeof (NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	/* Copy fields from Mozilla function table to global MozillaFuncs */
	MozillaFuncs.version                = mozilla_funcs->version;
	MozillaFuncs.size                   = mozilla_funcs->size;
	MozillaFuncs.posturl                = mozilla_funcs->posturl;
	MozillaFuncs.geturl                 = mozilla_funcs->geturl;
	MozillaFuncs.geturlnotify           = mozilla_funcs->geturlnotify;
	MozillaFuncs.requestread            = mozilla_funcs->requestread;
	MozillaFuncs.newstream              = mozilla_funcs->newstream;
	MozillaFuncs.write                  = mozilla_funcs->write;
	MozillaFuncs.destroystream          = mozilla_funcs->destroystream;
	MozillaFuncs.status                 = mozilla_funcs->status;
	MozillaFuncs.uagent                 = mozilla_funcs->uagent;
	MozillaFuncs.memalloc               = mozilla_funcs->memalloc;
	MozillaFuncs.memfree                = mozilla_funcs->memfree;
	MozillaFuncs.memflush               = mozilla_funcs->memflush;
	MozillaFuncs.reloadplugins          = mozilla_funcs->reloadplugins;
	MozillaFuncs.getJavaEnv             = mozilla_funcs->getJavaEnv;
	MozillaFuncs.getJavaPeer            = mozilla_funcs->getJavaPeer;
	MozillaFuncs.getvalue               = mozilla_funcs->getvalue;
	MozillaFuncs.setvalue               = mozilla_funcs->setvalue;
	MozillaFuncs.pushpopupsenabledstate = mozilla_funcs->pushpopupsenabledstate;
	MozillaFuncs.poppopupsenabledstate  = mozilla_funcs->poppopupsenabledstate;

	if ((mozilla_funcs->version & 0xFF) >= 14 )
	{   
		// NPRuntime support
		MozillaFuncs.getstringidentifier  = mozilla_funcs->getstringidentifier;
		MozillaFuncs.getstringidentifiers = mozilla_funcs->getstringidentifiers;
		MozillaFuncs.getintidentifier     = mozilla_funcs->getintidentifier;
		MozillaFuncs.identifierisstring   = mozilla_funcs->identifierisstring;
		MozillaFuncs.utf8fromidentifier   = mozilla_funcs->utf8fromidentifier;
		MozillaFuncs.intfromidentifier    = mozilla_funcs->intfromidentifier;
		MozillaFuncs.createobject         = mozilla_funcs->createobject;
		MozillaFuncs.retainobject         = mozilla_funcs->retainobject;
		MozillaFuncs.releaseobject        = mozilla_funcs->releaseobject;
		MozillaFuncs.invoke               = mozilla_funcs->invoke;
		MozillaFuncs.invokeDefault        = mozilla_funcs->invokeDefault;
		MozillaFuncs.evaluate             = mozilla_funcs->evaluate;
		MozillaFuncs.getproperty          = mozilla_funcs->getproperty;
		MozillaFuncs.setproperty          = mozilla_funcs->setproperty;
		MozillaFuncs.removeproperty       = mozilla_funcs->removeproperty;
		MozillaFuncs.hasproperty          = mozilla_funcs->hasproperty;
		MozillaFuncs.hasmethod            = mozilla_funcs->hasmethod;
		MozillaFuncs.releasevariantvalue  = mozilla_funcs->releasevariantvalue;
		MozillaFuncs.setexception         = mozilla_funcs->setexception;
	}

	/* Set up the plugin function table */
	plugin_funcs->version       = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
	plugin_funcs->size          = sizeof (NPPluginFuncs);
	plugin_funcs->newp          = NewNPP_NewProc(NPP_New);
	plugin_funcs->destroy       = NewNPP_DestroyProc(NPP_Destroy);
	plugin_funcs->setwindow     = NewNPP_SetWindowProc(NPP_SetWindow);
	plugin_funcs->newstream     = NewNPP_NewStreamProc(NPP_NewStream);
	plugin_funcs->destroystream = NewNPP_DestroyStreamProc(NPP_DestroyStream);
	plugin_funcs->asfile        = NewNPP_StreamAsFileProc(NPP_StreamAsFile);
	plugin_funcs->writeready    = NewNPP_WriteReadyProc(NPP_WriteReady);
	plugin_funcs->write         = NewNPP_WriteProc(NPP_Write);
	plugin_funcs->print         = NewNPP_PrintProc(NPP_Print);
	plugin_funcs->urlnotify     = NewNPP_URLNotifyProc(NPP_URLNotify);
	plugin_funcs->event         = NewNPP_HandleEventProc (NPP_HandleEvent);
	plugin_funcs->javaClass     = NULL;
	plugin_funcs->getvalue      = NewNPP_GetValueProc(NPP_GetValue);
	plugin_funcs->setvalue      = NewNPP_SetValueProc(NPP_SetValueX);

	return NPP_Initialize ();
}

NPError
NP_Shutdown (void)
{
	NPP_Shutdown ();
	return NPERR_NO_ERROR;
}
