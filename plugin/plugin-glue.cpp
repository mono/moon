/*
 * plugin-glue.cpp: MoonLight browser plugin.
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
#include "moon-plugin.h"

NPError 
NPP_New (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	DEBUG ("NPP_New");

	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = new PluginInstance (instance, mode);
	if (plugin == NULL)
		return NPERR_OUT_OF_MEMORY_ERROR;

	plugin->Initialize (argc, argn, argv);
	instance->pdata = plugin;

	return NPERR_NO_ERROR;
}

NPError 
NPP_Destroy (NPP instance, NPSavedData** save)
{
	DEBUG ("NPP_Destroy");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	delete plugin;
	instance->pdata = NULL;

	return NPERR_NO_ERROR;
}

NPError 
NPP_SetWindow (NPP instance, NPWindow* window)
{
	DEBUG ("NPP_SetWindow %d %d", window->width, window->height);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetWindow (window);
}

NPError
NPP_NewStream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	DEBUG ("NPP_NewStream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->NewStream (type, stream, seekable, stype);
}

NPError
NPP_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
	DEBUG ("NPP_DestroyStream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->DestroyStream (stream, reason);
}

void
NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
	DEBUG ("NPP_StreamAsFile");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->StreamAsFile (stream, fname);
}

int32
NPP_WriteReady (NPP instance, NPStream* stream)
{
	DEBUG ("NPP_WriteReady");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPN_DestroyStream (instance, stream, NPRES_DONE);

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->WriteReady (stream);
}

int32
NPP_Write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
	DEBUG ("NPP_Write");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPN_DestroyStream (instance, stream, NPRES_DONE);

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->Write (stream, offset, len, buffer);
}

void
NPP_Print (NPP instance, NPPrint* platformPrint)
{
	DEBUG ("NPP_Print");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Print (platformPrint);
}

void
NPP_URLNotify (NPP instance, const char* url, NPReason reason, void* notifyData)
{
	DEBUG ("NPP_URLNotify");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->UrlNotify (url, reason, notifyData);
}


int16 
NPP_HandleEvent (NPP instance, void* event)
{
	DEBUG ("NPP_HandleEvent");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->EventHandle (event);
}

NPError 
NPP_GetValue (NPP instance, NPPVariable variable, void *result)
{
	DEBUG ("NPP_GetValue %d (%x)", variable, variable);

	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)result) = PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char **)result) = PLUGIN_DESCRIPTION;
			break;
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;
		default:
			if (instance == NULL)
				return NPERR_INVALID_INSTANCE_ERROR;

			PluginInstance *plugin = (PluginInstance *) instance->pdata;
			err = plugin->GetValue (variable, result);
	}

	return err;
}

NPError 
NPP_SetValue (NPP instance, NPNVariable variable, void *value)
{
	DEBUG ("NPP_SetValue %d (%x)", variable, value);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetValue (variable, value);
}

char *
NPP_GetMIMEDescription (void)
{
	DEBUG ("NPP_GetMIMEDescription");
    return (MIME_TYPES_HANDLED);
}

NPError
NPP_Initialize (void)
{
	DEBUG ("NP_Initialize");

	gtk_init (0, 0);
	runtime_init ();

	return NPERR_NO_ERROR;
}

void
NPP_Shutdown (void)
{
	DEBUG ("NP_Shutdown");
}
