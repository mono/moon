/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#include <stdlib.h>
#include <string.h>

#include "browser.h"
#include "plugin.h"
#include "logging.h"

#define MIME_TYPES_HANDLED  	"application/x-jolttest"
#define PLUGIN_NAME         	"The Shocker"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":jolttest:"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  	PLUGIN_NAME ":  Test Harness Plugin for testing Moonlight files."

Window PluginObject::browser_app_context = 0;

char*
Plugin_GetMIMEDescription (void)
{
	return (char *) MIME_TYPES_DESCRIPTION;
}


PluginObject::PluginObject (NPP npp, int argc, char *argn[], char *argv[])
{
	shocker_control = NULL;
	auto_capture = NULL;
	instance = npp;
	x = 0;
	y = 0;

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

		if (!strcasecmp (argn [i], "capturex")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetCaptureX (strtol (argv [i], NULL, 10));
		}

		if (!strcasecmp (argn [i], "capturey")) {
			if (!auto_capture)
				auto_capture = new AutoCapture ();
			auto_capture->SetCaptureY (strtol (argv [i], NULL, 10));
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
		*((char **) value) = (char *) PLUGIN_NAME;
		break;
	case NPPVpluginDescriptionString:
		*((char **) value) = (char *) PLUGIN_DESCRIPTION;
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

int
PluginObject::GetX ()
{
	UpdateXY ();
	return x;
}

int
PluginObject::GetY ()
{
	UpdateXY ();
	return y;
}

void
PluginObject::UpdateXY ()
{
	Display *display = XOpenDisplay (NULL);
	Window root = XDefaultRootWindow (display);
	Window src = (Window) window->window;
	Window dummy;
	XTranslateCoordinates (display, src, root, -window->x, -window->y, &x, &y, &dummy);
	XCloseDisplay (display);

#if SHOCKER_DEBUG
	printf ("[Shocker] PluginObject::SetWindow (window: %p, window->x: %i, window->y: %i, window->width: %i, window->height: %i, x: %i, y: %i)\n", window->window, window->x, window->y, window->width, window->height, x, y);
#endif
}

NPError
PluginObject::SetWindow (NPWindow* window)
{
	if (!window)
		return NPERR_NO_ERROR;

	// SetWindow does not get called when size = 0,0 (default in 1.0 tests)
	// for 2.0 tests we set size = 1,1.
	this->window = window;

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
Plugin_New (NPMIMEType type, NPP instance, guint16 mode, gint16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	PluginObject* plugin = new PluginObject (instance, argc, argn, argv);
	if (!plugin)
		return NPERR_OUT_OF_MEMORY_ERROR;

	instance->pdata = (void *) plugin;

	NPError rv = Browser::Instance ()->GetValue (instance, NPNVnetscapeWindow, (void *) &PluginObject::browser_app_context);

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

static NPError Plugin_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, guint16* stype)
{
	return NPERR_NO_ERROR;
}

static int Plugin_WriteReady (NPP instance, NPStream *stream)
{
	return NPERR_NO_ERROR;
}

static int Plugin_Write (NPP instance, NPStream *stream, int offset, int len, void *buffer)
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

static gint16 Plugin_HandleEvent(NPP instance, void* event)
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

