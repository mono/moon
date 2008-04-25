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
 * shocker.cpp: A scriptable plugin object that exposes some testing functions
 * 		to test authors.
 * 
 */


#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "plstr.h"

#include "netscape.h"
#include "shocker.h"
#include "plugin.h"
#include "browser.h"
#include "logging.h"
#include "input.h"
#include "shutdown-manager.h"






#define MOONLIGHT_PLUGIN_ID	"joltControl"

#define NPVARIANT_IS_NUMBER(v)(NPVARIANT_IS_INT32 (v) || NPVARIANT_IS_DOUBLE (v))
#define NUMBER_TO_INT32(v)(NPVARIANT_IS_INT32 (v) ? NPVARIANT_TO_INT32 (v) : (int) NPVARIANT_TO_DOUBLE (v))


bool
Shocker_Initialize (void)
{
	ShockerScriptableControlClass = new ShockerScriptableControlType ();

	return NPERR_NO_ERROR;
}

void
Shocker_Shutdown (void)
{
	if (ShockerScriptableControlClass)
		delete ShockerScriptableControlClass;
}


static bool
Connect (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->Connect ();
	return true;
}

static bool
SignalShutdown (ShockerScriptableControlObject *obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->SignalShutdown ();
	return true;
}

bool
LogMessage (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogMessage (STR_FROM_VARIANT (args [0]));
	return true;
}

bool
LogWarning (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogWarning (STR_FROM_VARIANT (args [0]));
	return true;
}

bool
LogHelp (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogHelp (STR_FROM_VARIANT (args [0]));
	return true;
}

bool
LogError (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogError (STR_FROM_VARIANT (args [0]));
	return true;
}


bool
LogDebug (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogDebug (STR_FROM_VARIANT (args [0]));
	return true;
}

bool
LogResult (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));

	obj->GetLogProvider ()->LogResult (LogProvider::IntToResult (NUMBER_TO_INT32 (args [0])));
	return true;
}

bool
MoveMouseLogarithmic (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	obj->GetInputProvider ()->MoveMouseLogarithmic (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	return true;
}

bool
MoveMouse (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	obj->GetInputProvider ()->MoveMouse (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	return true;
}

bool
MouseIsAtPosition (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	return obj->GetInputProvider ()->MouseIsAtPosition (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
}

bool
MouseLeftClick (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->GetInputProvider ()->MouseLeftClick ();
	return true;
}

bool
MouseRightClick (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->GetInputProvider ()->MouseRightClick ();
	return true;
}

bool
MouseLeftButtonDown (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->GetInputProvider ()->MouseLeftButtonDown ();
	return true;
}

bool
MouseLeftButtonUp (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	obj->GetInputProvider ()->MouseLeftButtonUp ();
	return true;
}

bool
SendKeyInput (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	//
	// There are two optional args here that I am just going to ignore for now
	//

	g_assert (arg_count >= 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [1]));

	obj->GetInputProvider ()->SendKeyInput (NUMBER_TO_INT32 (args [0]), NPVARIANT_TO_BOOLEAN (args [1]));

	return true;
}

bool
CaptureSingleImage (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 6);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_STRING (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));
	g_assert (NPVARIANT_IS_NUMBER (args [3]));
	g_assert (NPVARIANT_IS_NUMBER (args [4]));
	g_assert (NPVARIANT_IS_NUMBER (args [5]));

	obj->GetImageCaptureProvider ()->CaptureSingleImage (STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]),
			NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]), NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]));

	return true;
}

bool
CaptureMultipleImages (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count)
{
	g_assert (arg_count == 9);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_STRING (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));
	g_assert (NPVARIANT_IS_NUMBER (args [3]));
	g_assert (NPVARIANT_IS_NUMBER (args [4]));
	g_assert (NPVARIANT_IS_NUMBER (args [5]));
	g_assert (NPVARIANT_IS_NUMBER (args [6]));
	g_assert (NPVARIANT_IS_NUMBER (args [7]));
	g_assert (NPVARIANT_IS_NUMBER (args [8]));

	obj->GetImageCaptureProvider ()->CaptureMultipleImages (obj->GetTestPath (), NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]),
			NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]),NUMBER_TO_INT32 (args [6]), NUMBER_TO_INT32 (args [7]),
			NUMBER_TO_INT32 (args [8]));

	return true;
}

static void
shocker_scriptable_control_deallocate (NPObject *npobj)
{
	ShockerScriptableControlObject* obj = (ShockerScriptableControlObject *) npobj;
	delete obj;
}

static NPObject*
shocker_scriptable_control_allocate (NPP instance, NPClass *)
{
#ifdef SHOCKER_DEBUG
	printf ("creating the new scriptable control\n");
#endif
	return new ShockerScriptableControlObject (instance);
}


static void
shocker_scriptable_control_invalidate (NPObject *npobj)
{
	
}

typedef struct 
{
    const char* name;
    bool (* Invoke) (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count);
} ScriptableMethod;


static ScriptableMethod scriptable_methods [] = {
	{ "Connect", &Connect },
	{ "SignalShutdown", &SignalShutdown },

	{ "LogMessage", &LogMessage },
	{ "LogWarning", &LogWarning },
	{ "LogHelp", &LogHelp },
	{ "LogError", &LogError },
	{ "LogDebug", &LogDebug },

	// Do the same thing
	{ "LogResult", &LogResult },
	{ "TryLogResult", &LogResult },

	{ "moveMouseLogarithmic", &MoveMouseLogarithmic },
	{ "moveMouse", &MoveMouse },
	{ "mouseIsAtPosition", &MouseIsAtPosition },
	{ "mouseLeftClick", &MouseLeftClick },
	{ "mouseRightClick", &MouseRightClick },
	{ "mouseLeftButtonDown", &MouseLeftButtonDown },
	{ "mouseLeftButtonUp", &MouseLeftButtonUp },
	{ "sendKeyInput", &SendKeyInput },

	{ "CaptureSingleImage", &CaptureSingleImage },
	{ "CaptureMultipleImages", &CaptureMultipleImages },

	{ NULL, NULL }
};


static bool
shocker_scriptable_control_has_method (NPObject *npobj, NPIdentifier id)
{
	bool res = false;
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	ScriptableMethod *walk = scriptable_methods;

	while (walk->name) {
		if (!strcmp (name, walk->name)) {
			res = true;
			break;
		}
		walk++;
	}

#ifdef SHOCKER_DEBUG
	printf ("scriptable control has method:  %s  %d\n", name, res);
#endif

	free (name);
	return res;
}

static bool
shocker_scriptable_control_has_property (NPObject *npobj, NPIdentifier name)
{
#ifdef SHOCKER_DEBUG
	printf ("scriptable control has property:  %s\n", Browser::Instance ()->UTF8FromIdentifier (name));
#endif
	return false;
}

static bool
shocker_scriptable_control_get_property (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	return false;
}

static bool
shocker_scriptable_control_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return false;
}

static bool
shocker_scriptable_control_remove_property (NPObject *npobj, NPIdentifier name)
{
	return false;
}

static bool
shocker_scriptable_control_invoke (NPObject *npobj, NPIdentifier id, const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	bool res = false;
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	ScriptableMethod *walk = scriptable_methods;

	while (walk->name) {
		if (!strcmp (name, walk->name)) {
			BOOLEAN_TO_NPVARIANT (walk->Invoke ((ShockerScriptableControlObject *) npobj, name, args, arg_count), *result);
			res = true;
			break;
		}
		walk++;
	}
#ifdef SHOCKER_DEBUG
	printf ("scriptable control invoke:  %s  %d  result:%d\n", name, res, NPVARIANT_TO_BOOLEAN (*result));
#endif
	free (name);
	return res;
}

static bool
shocker_scriptable_control_invoke_default (NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return false;
}



ShockerScriptableControlType::ShockerScriptableControlType ()
{
	allocate       = shocker_scriptable_control_allocate;
	deallocate     = shocker_scriptable_control_deallocate;
	invalidate     = shocker_scriptable_control_invalidate;
	hasMethod      = shocker_scriptable_control_has_method;
	invoke         = shocker_scriptable_control_invoke;
	invokeDefault  = shocker_scriptable_control_invoke_default;
	hasProperty    = shocker_scriptable_control_has_property;
	getProperty    = shocker_scriptable_control_get_property;
	setProperty    = shocker_scriptable_control_set_property;
	removeProperty = shocker_scriptable_control_remove_property;
}

ShockerScriptableControlType *ShockerScriptableControlClass = NULL;


ShockerScriptableControlObject::ShockerScriptableControlObject (NPP instance) : instance (instance), test_path (NULL)
{	
	log_provider = new LogProvider (GetTestPath ());
	input_provider = new InputProvider ();
	image_capture = new ImageCaptureProvider ();
}

ShockerScriptableControlObject::~ShockerScriptableControlObject ()
{
	g_free (test_path);
}
		
void
ShockerScriptableControlObject::Connect ()
{
}

void
ShockerScriptableControlObject::SignalShutdown ()
{
	shutdown_manager_queue_shutdown (this);
}

char *
ShockerScriptableControlObject::GetTestPath ()
{
	if (test_path)
		return test_path;

	NPVariant nplocation;
	NPVariant nppath;
	NPObject *window = NULL;
	NPIdentifier identifier = Browser::Instance ()->GetStringIdentifier ("location");


	Browser::Instance ()->GetValue (instance, NPNVWindowNPObject, &window);
	Browser::Instance ()->GetProperty (instance, window, identifier, &nplocation);

	identifier = Browser::Instance ()->GetStringIdentifier ("pathname");
	Browser::Instance ()->GetProperty (instance, NPVARIANT_TO_OBJECT (nplocation), identifier, &nppath);

	test_path = g_path_get_basename (STR_FROM_VARIANT (nppath));

	return test_path;
}

void
ShockerScriptableControlObject::SetJsStatus (const char* str)
{
	NPVariant window;
	NPVariant status;
	NPIdentifier identifier = Browser::Instance ()->GetStringIdentifier ("status");

	char* val = PL_strdup (str);
	STRINGZ_TO_NPVARIANT (val, status);

	printf ("setting js status to:  %s\n", val);
	Browser::Instance ()->GetValue (instance, NPNVWindowNPObject, &window);
	Browser::Instance ()->SetProperty (instance, NPVARIANT_TO_OBJECT (window), identifier, &status);
}
