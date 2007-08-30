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

#include "moonlight.h"
#include "plugin.h"
#include "plugin-class.h"
#include "moon-mono.h"

NPError 
NPP_New (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
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
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Finalize ();

	delete plugin;
	instance->pdata = NULL;

	return NPERR_NO_ERROR;
}

NPError 
NPP_SetWindow (NPP instance, NPWindow* window)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetWindow (window);
}

NPError
NPP_NewStream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->NewStream (type, stream, seekable, stype);
}

NPError
NPP_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->DestroyStream (stream, reason);
}

void
NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->StreamAsFile (stream, fname);
}

int32
NPP_WriteReady (NPP instance, NPStream* stream)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->WriteReady (stream);
}

int32
NPP_Write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->Write (stream, offset, len, buffer);
}

void
NPP_Print (NPP instance, NPPrint* platformPrint)
{
	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Print (platformPrint);
}

void
NPP_URLNotify (NPP instance, const char* url, NPReason reason, void* notifyData)
{
	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->UrlNotify (url, reason, notifyData);
}


int16 
NPP_HandleEvent (NPP instance, void* event)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->EventHandle (event);
}

NPError 
NPP_GetValue (NPP instance, NPPVariable variable, void *result)
{
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
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetValue (variable, value);
}

char *
NPP_GetMIMEDescription (void)
{
	return (MIME_TYPES_HANDLED);
}

void downloader_initialize ();
void downloader_destroy ();

static bool vm_initialized = false;
static bool gtk_initialized = false;
static bool already_initialized = false;
static bool runtime_initialized = false;

NPError
NPP_Initialize (void)
{
	// We dont need to initialize mono vm and gtk more than one time.
	if (!gtk_initialized) {
		gtk_initialized = true;
		gtk_init (0, 0);
	}
	if (!vm_initialized) {
		vm_initialized = true;
		vm_init ();
	}
	downloader_initialize ();
	
	if (!runtime_initialized) {
		runtime_initialized = true;
		runtime_init ();
	}
	
	plugin_init_classes ();

	return NPERR_NO_ERROR;
}

void
NPP_Shutdown (void)
{
	plugin_destroy_classes ();
	// runtime_shutdown is broken at moment so let us just shutdown TimeManager,
	// when fixed please uncomment above line and remove time manger shutdown.
	runtime_shutdown ();
	//TimeManager::Instance()->Shutdown ();
	runtime_initialized = false;
	downloader_destroy ();
}
