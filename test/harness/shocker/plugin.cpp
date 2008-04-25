/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * plugin.cpp: A wrapper around some Mozilla plugin functions
 * 
 */

#include <strings.h>
#include "browser.h"
#include "plugin.h"
#include "logging.h"


#define MIME_TYPES_HANDLED  	"application/x-jolttest"
#define PLUGIN_NAME         	"The Shocker"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":jolttest:"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  	PLUGIN_NAME ":  Test Harness Plugin for testing Moonlight files."


char*
Plugin_GetMIMEDescription (void)
{
	return (MIME_TYPES_DESCRIPTION);
}


PluginObject::PluginObject (NPP npp, int argc, char* argn[], char* argv[]) : instance (npp), shocker_control (NULL), auto_capture (NULL)
{
	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL)
			continue;

		if (!strcasecmp (argn [i], "captureinterval")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetCaptureInterval (strtol (argv [i], NULL, 10));
		}

		if (!strcasecmp (argn [i], "maximagestocapture")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetMaxImagesToCapture (strtol (argv [i], NULL, 10));
		}

		if (!strcasecmp (argn [i], "initialdelay")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetInitialDelay (strtol (argv [i], NULL, 10));
		}

		if (!strcasecmp (argn [i], "capturewidth")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetCaptureWidth (strtol (argv [i], NULL, 10));
		}

		if (!strcasecmp (argn [i], "captureheight")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetCaptureHeight (strtol (argv [i], NULL, 10));
		}
	}
}

PluginObject::~PluginObject()
{
	if (shocker_control)
		Browser::Instance ()->ReleaseObject ((NPObject *) shocker_control);
}

void
PluginObject::Shutdown ()
{
}

NPError
PluginObject::GetValue (NPPVariable variable, void *value)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
	case NPPVpluginNameString:
		*((char **) value) = PLUGIN_NAME;
		break;
	case NPPVpluginDescriptionString:
		*((char **) value) = PLUGIN_DESCRIPTION;
		break;
	case NPPVpluginScriptableNPObject:
		*((NPObject**) value) = GetShockerControl ();
		break;
	default:
		err = NPERR_INVALID_PARAM;
		break;
	}
	return err;
}

NPError
PluginObject::SetWindow (NPWindow* window)
{
	if (!window)
		return NPERR_NO_ERROR;

	return NPERR_NO_ERROR;
}


ShockerScriptableControlObject *
PluginObject::GetShockerControl ()
{
	if (!shocker_control) {
		shocker_control = (ShockerScriptableControlObject *) Browser::Instance ()->CreateObject (instance, ShockerScriptableControlClass);
		// ugh, this doesn't really make sense here
		if (auto_capture) {
			auto_capture->Run (shocker_control->GetTestPath (), shocker_control->GetImageCaptureProvider ());
		}
	}

	Browser::Instance ()->RetainObject (shocker_control);
	return shocker_control;
}

///// C <-> C++ wrapper junk

static NPError
Plugin_New (NPMIMEType type, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPError rv = NPERR_NO_ERROR;

	PluginObject* plugin = new PluginObject (instance, argc, argn, argv);
	if (!plugin)
		return NPERR_OUT_OF_MEMORY_ERROR;

	instance->pdata = (void *) plugin;

#ifdef SHOCKER_DEBUG
	printf ("Plugin_New created:   %p\n", plugin);
#endif

	return rv;
}

static NPError
Plugin_Destroy (NPP instance, NPSavedData** save)
{
#ifdef SHOCKER_DEBUG
	printf ("Plugin_Destroy destroying: %p\n", instance->pdata);
#endif

	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginObject* plugin = (PluginObject *) instance->pdata;
	if (plugin) {
		plugin->Shutdown ();
		delete plugin;
	}

	return NPERR_NO_ERROR;
}

static NPError
Plugin_SetWindow (NPP instance, NPWindow* window)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (!window)
		return NPERR_GENERIC_ERROR;

	PluginObject* plugin = (PluginObject *) instance->pdata;

	if (plugin == NULL) 
		return NPERR_GENERIC_ERROR;

	return plugin->SetWindow (window);
}


//
// This is visible to the world because the browser needs to call it
//
NPError
Plugin_GetValue (NPP instance, NPPVariable variable, void *value)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginObject* plugin = (PluginObject *) instance->pdata;
	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	return plugin->GetValue (variable, value);
}

static NPError
Plugin_SetValue (NPP instance, NPNVariable variable, void *value)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginObject* plugin = (PluginObject *) instance->pdata;
	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	return plugin->SetValue (variable, value);
}


//
// TODO:  If I just set these as NULL instead of creating stubs, what happens??
//

static NPError Plugin_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	return NPERR_NO_ERROR;
}

static int32 Plugin_WriteReady (NPP instance, NPStream *stream)
{
	return NPERR_NO_ERROR;
}

static int32 Plugin_Write (NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	return NPERR_NO_ERROR;
}

static NPError Plugin_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

static void Plugin_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
}

static void Plugin_Print (NPP instance, NPPrint* printInfo)
{
}

static void Plugin_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
}

static int16 Plugin_HandleEvent(NPP instance, void* event)
{
	return 0;
}

void Plugin_Initialize (NPPluginFuncs* npp_funcs)
{
	npp_funcs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;

	npp_funcs->newp          = Plugin_New;
	npp_funcs->destroy       = Plugin_Destroy;
	npp_funcs->setwindow     = Plugin_SetWindow;
	npp_funcs->newstream     = Plugin_NewStream;
	npp_funcs->destroystream = Plugin_DestroyStream;
	npp_funcs->asfile        = Plugin_StreamAsFile;
	npp_funcs->writeready    = Plugin_WriteReady;
	npp_funcs->write         = Plugin_Write;
	npp_funcs->print         = Plugin_Print;
	npp_funcs->event         = Plugin_HandleEvent;
	npp_funcs->urlnotify     = Plugin_URLNotify;
	npp_funcs->getvalue      = Plugin_GetValue;
	npp_funcs->setvalue      = Plugin_SetValue;
}

