/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-glue.cpp: MoonLight browser plugin.
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

#include "moonlight.h"
#include "plugin.h"
#include "plugin-class.h"
#include "plugin-downloader.h"

static int plugins_alive = 0;

static void plugin_surface_destroyed (EventObject *sender, EventArgs *args, gpointer closure);

NPError
NPP_New (NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = new PluginInstance (pluginType, instance, mode);
	if (plugin == NULL)
		return NPERR_OUT_OF_MEMORY_ERROR;

	plugin->Initialize (argc, argn, argv);
	instance->pdata = plugin;

	plugins_alive ++;

	return NPERR_NO_ERROR;
}

NPError
NPP_Destroy (NPP instance, NPSavedData **save)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	Deployment::SetCurrent (plugin->GetDeployment ());
	if (plugin->GetSurface())
		plugin->GetSurface()->AddHandler (EventObject::DestroyedEvent, plugin_surface_destroyed, NULL);
	plugin->Finalize ();

	instance->pdata = NULL;
	delete plugin;

	return NPERR_NO_ERROR;
}

NPError
NPP_SetWindow (NPP instance, NPWindow *window)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	return plugin->SetWindow (window);
}

NPError
NPP_NewStream (NPP instance, NPMIMEType type, NPStream *stream, NPBool seekable, uint16_t *stype)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	return plugin->NewStream (type, stream, seekable, stype);
}

NPError
NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	return plugin->DestroyStream (stream, reason);
}

void
NPP_StreamAsFile (NPP instance, NPStream *stream, const char *fname)
{
	if (instance == NULL)
		return;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	plugin->StreamAsFile (stream, fname);
}

int32_t
NPP_WriteReady (NPP instance, NPStream *stream)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	return plugin->WriteReady (stream);
}

int32_t
NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	return plugin->Write (stream, offset, len, buffer);
}

void
NPP_Print (NPP instance, NPPrint *platformPrint)
{
	if (instance == NULL)
		return;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	plugin->Print (platformPrint);
}

void
NPP_URLNotify (NPP instance, const char *url, NPReason reason, void *notifyData)
{
	if (instance == NULL)
		return;
	
	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	
	plugin->UrlNotify (url, reason, notifyData);
}


int16_t
NPP_HandleEvent (NPP instance, void *event)
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
	case NPPVpluginNeedsXEmbed:
		*((NPBool *)result) = true;
		break;
	case NPPVpluginNameString:
		*((char **)result) = (char *) PLUGIN_NAME;
		break;
	case NPPVpluginDescriptionString:
		*((char **)result) = (char *) PLUGIN_DESCRIPTION;
		break;
	default:
		if (instance == NULL)
			return NPERR_INVALID_INSTANCE_ERROR;
		
		PluginInstance *plugin = (PluginInstance *) instance->pdata;
		err = plugin->GetValue (variable, result);
		break;
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
	return (char *) (MIME_TYPES_HANDLED);
}

static bool runtime_initialized = false;
static bool runtime_shutdown_pending = false;

NPError
NPP_Initialize (void)
{
	NPNToolkitType toolkit = (NPNToolkitType)0;

	NPN_GetValue (NULL, NPNVToolkit, &toolkit);
	if (toolkit != (NPNToolkitType)NPNVGtk2) {
		g_warning ("we don't have the toolkit we need");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	// We dont need to initialize mono vm and gtk more than one time.
	if (!g_thread_supported ()) {
		g_warning ("host has not initialized threads");
		//g_thread_init (NULL);
	} 

	downloader_initialize ();

	if (!runtime_initialized) {
		runtime_initialized = true;
		runtime_init_browser (get_plugin_dir ());
	}

	plugin_init_classes ();

	return NPERR_NO_ERROR;
}

static gboolean
shutdown_moonlight (gpointer data)
{
	if (plugins_alive)
		return FALSE;
		
	downloader_destroy ();
	plugin_destroy_classes ();
	runtime_shutdown ();
	runtime_initialized = false;
	//MoonlightObject::Summarize ();
	runtime_shutdown_pending = false;

	return FALSE;
}

static void
plugin_surface_destroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	plugins_alive --;
	if (plugins_alive == 0 && runtime_shutdown_pending) {
		g_idle_add (shutdown_moonlight, NULL);
	}
}

void
NPP_Shutdown (void)
{
	if (plugins_alive)
		runtime_shutdown_pending = true;
	else
		shutdown_moonlight (NULL);
}

