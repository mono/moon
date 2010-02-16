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

#include "netscape.h"
#include "shocker.h"
#include "plugin.h"
#include "browser.h"
#include "logging.h"
#include "input.h"
#include "shutdown-manager.h"
#include "harness.h"


// hack to track down the actual error message that
// gecko insists on dropping for some really stupid
// reason that I cannot begin to fathom
// enable if you're desperate to track down empty
// Script Errors and don't want to gdb
#define DEBUG_ERROR_GECKO 0

#if DEBUG_ERROR_GECKO == 1
  #include "config.h"
  #if DEBUG == 1 && HAVE_UNWIND == 1
    #define UNW_LOCAL_ONLY
    #include <libunwind.h>
  #endif
#endif

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

char *
NPN_strdup (const char *tocopy)
{
	int len = strlen(tocopy);
	char *ptr = (char *)MOON_NPN_MemAlloc (len+1);
	if (ptr != NULL) {
		strcpy (ptr, tocopy);
		// WebKit should calloc so we dont have to do this
		ptr[len] = 0;
	}

	return ptr;
}

static void print_stack_trace ()
{
	// int provoke_stacktrace = * (int *) NULL;
	// printf ("%i", provoke_stacktrace); // silence warnings
}

static void
Connect (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->Connect ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
SignalShutdown (ShockerScriptableControlObject *obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->SignalShutdown ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
LogMessage (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogMessage (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
LogWarning (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogWarning (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
LogHelp (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogHelp (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

#if DEBUG_ERROR_GECKO == 1
static void findRealErrorOnStack (ShockerScriptableControlObject* obj) {
	unw_context_t uc;
	unw_cursor_t cursor, prev;
	unw_word_t bp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	char framename [1024];
	int count = 0;

	while (unw_step(&cursor) > 0 && count < 18) {
		count++;
		unw_get_proc_name (&cursor, framename, sizeof(framename), 0);
		if (!*framename)
			continue;
		if (strstr (framename, "js_ReportErrorAgain")) {
#if (__i386__)
			unw_get_reg(&prev, UNW_X86_EBP, &bp);
#elif (__amd64__)
			unw_get_reg(&prev, UNW_X86_64_RBP, &bp);
#endif
			bp += 12;
			char ** n = (char**)bp;
			obj->GetLogProvider ()->LogError (*n);
			break;
		}
		prev = cursor;
	}
}
#endif

static void
LogError (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

#if DEBUG_ERROR_GECKO == 1
	findRealErrorOnStack (obj);
#endif
	obj->GetLogProvider ()->LogError (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}


static void
LogDebug (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	obj->GetLogProvider ()->LogDebug (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
LogResult (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));

	obj->GetLogProvider ()->LogResult (LogProvider::IntToResult (NUMBER_TO_INT32 (args [0])));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MoveMouseLogarithmic (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	obj->GetInputProvider ()->MoveMouseLogarithmic (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MoveMouse (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	obj->GetInputProvider ()->MoveMouse (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MouseIsAtPosition (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	BOOLEAN_TO_NPVARIANT (obj->GetInputProvider ()->MouseIsAtPosition (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1])), *result);
}

static void
MouseDoubleClick (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->GetInputProvider ()->MouseDoubleClick ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MouseLeftClick (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->GetInputProvider ()->MouseLeftClick ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MouseRightClick (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->GetInputProvider ()->MouseRightClick ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MouseLeftButtonDown (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->GetInputProvider ()->MouseLeftButtonDown ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
MouseLeftButtonUp (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	obj->GetInputProvider ()->MouseLeftButtonUp ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
SendKeyInput (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	//
	// There are two optional args here that I am just going to ignore for now
	//

	g_assert (arg_count >= 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [1]));

	obj->GetInputProvider ()->SendKeyInput (NUMBER_TO_INT32 (args [0]), NPVARIANT_TO_BOOLEAN (args [1]));

	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
SetKeyboardInputSpeed (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] SetKeyboardInputSpeed: Not implemented\n");
	print_stack_trace ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
CompareImages (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	bool res = false;
	char *msg;
	int output;
	
	g_assert (arg_count >= 6);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_STRING (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));
	g_assert (NPVARIANT_IS_STRING (args [3]));
	g_assert (NPVARIANT_IS_STRING (args [4]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [5]));
	
	msg = g_strdup_printf ("TestHost.CompareImages %s|%s|%i|%s|%s|%i", 
		STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]), NUMBER_TO_INT32 (args [2]), 
		STR_FROM_VARIANT (args [3]), STR_FROM_VARIANT (args [4]), NPVARIANT_TO_BOOLEAN (args [5]));
	
	if (send_harness_message (msg, &output))
		res = output == 0;
	g_free (msg);
	
	BOOLEAN_TO_NPVARIANT (res, *result);
}

static void
GetActiveInputLocaleId (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] GetActiveInputLocaleId: Not implemented\n");
	print_stack_trace ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
ActivateKeyboardLayout (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] ActivateKeyboardLayout: Not implemented\n");
	print_stack_trace ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
IsInputLocaleInstalled (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] IsInputLocaleInstalled: Not implemented\n");
	print_stack_trace ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
GetTestDirectory (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	const char* dir = getenv ("MOONLIGHT_HARNESS_TESTDIRECTORY");

	if (!dir) {
		printf ("[shocker] GetTestDirectory: MOONLIGHT_HARNESS_TESTDIRECTORY IS NOT SET, using /tmp instead.\n");
		dir = "/tmp";
	}

	char *retval;
	retval = NPN_strdup (dir);
	STRINGZ_TO_NPVARIANT (retval, *result);
}

static void
GetTestDefinition (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	char *test_definition;
	char *retval;
	
	g_assert (arg_count == 0);
	
	test_definition = getenv ("MOONLIGHT_HARNESS_TESTDEFINITION");
	
	if (test_definition == NULL || test_definition [0] == 0)
		printf ("[shocker] GetTestDefinition (): MOONLIGHT_HARNESS_TESTDEFINITION isn't set, this will probably cause the test to fail.\n");
	
	printf ("[shocker] GetTestDefinition ()\n");
	//printf (test_definition);
	//printf ("\n");
	
	retval = NPN_strdup (test_definition == NULL ? "" : test_definition);
	
	STRINGZ_TO_NPVARIANT (retval, *result);
}

static void
GetRuntimePropertyValue (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] GetRuntimePropertyValue: Not implemented\n");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void 
SetRuntimePropertyValue (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] SetRuntimePropertyValue: Not implemented\n");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
CleanDRM (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] CleanDRM: Not implemented\n");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
SwitchToHighContrastScheme (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	printf ("[shocker] SwitchToHighContrastScheme: Not implemented\n");
	BOOLEAN_TO_NPVARIANT (true, *result);
}
    
static void
CaptureSingleImage (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
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

	BOOLEAN_TO_NPVARIANT (true, *result);
}

static void
CaptureMultipleImages (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result)
{
	const char *path;
	
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

	path = STR_FROM_VARIANT (args [1]);
	if (path == NULL || path [0] == 0)
		path = obj->GetTestPath ();

	obj->GetImageCaptureProvider ()->CaptureMultipleImages (path, NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]),
			NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]),NUMBER_TO_INT32 (args [6]), NUMBER_TO_INT32 (args [7]),
			NUMBER_TO_INT32 (args [8]));

	BOOLEAN_TO_NPVARIANT (true, *result);
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
    void (* Invoke) (ShockerScriptableControlObject* obj, char* name, const NPVariant* args, uint32_t arg_count, NPVariant *result);
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
	{ "mouseDoubleClick", &MouseDoubleClick },
	{ "mouseLeftClick", &MouseLeftClick },
	{ "mouseRightClick", &MouseRightClick },
	{ "mouseLeftButtonDown", &MouseLeftButtonDown },
	{ "mouseLeftButtonUp", &MouseLeftButtonUp },
	{ "sendKeyInput", &SendKeyInput },

	{ "CaptureSingleImage", &CaptureSingleImage },
	{ "CaptureMultipleImages", &CaptureMultipleImages },

	// New test plugin methods in 2.0
    { "setKeyboardInputSpeed", &SetKeyboardInputSpeed },
    { "CompareImages", &CompareImages },
    { "GetActiveInputLocaleId", &GetActiveInputLocaleId },
    { "ActivateKeyboardLayout", &ActivateKeyboardLayout },
    { "IsInputLocaleInstalled", &IsInputLocaleInstalled },
    { "GetTestDirectory", &GetTestDirectory },
    { "GetTestDefinition", &GetTestDefinition },
    { "GetRuntimePropertyValue", &GetRuntimePropertyValue },
    { "SetRuntimePropertyValue", &SetRuntimePropertyValue },
    { "CleanDRM", &CleanDRM },
    { "SwitchToHighContrastScheme", &SwitchToHighContrastScheme },
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
shocker_scriptable_control_has_property (NPObject *npobj, NPIdentifier id)
{
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	bool res = false;
	
	res = !strcmp (name, "X") || !strcmp (name, "Y");
	
	free (name);
		
#ifdef SHOCKER_DEBUG
	printf ("scriptable control has property:  %s\n", Browser::Instance ()->UTF8FromIdentifier (name));
#endif

	return res;
}

static bool
shocker_scriptable_control_get_property (NPObject *npobj, NPIdentifier id, NPVariant *result)
{
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	bool res = false;
	ShockerScriptableControlObject * ssco = (ShockerScriptableControlObject *) npobj;
	PluginObject *plugin = ssco->GetPluginObject ();

#if SHOCKER_DEBUG
	printf ("[Shocker] shocker_scriptable_control_get_property ('%s') x: %i, y: %i\n", name, plugin->GetX (), plugin->GetY ());
#endif

	if (!strcmp (name, "X")) {
		INT32_TO_NPVARIANT (plugin->GetX (), *result);
		res = true;
	} else if (!strcmp (name, "Y")) {
		INT32_TO_NPVARIANT (plugin->GetY (), *result);
		res = true;
	}
	
	return res;
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
			walk->Invoke ((ShockerScriptableControlObject *) npobj, name, args, arg_count, result);
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
	LogProvider::CreateInstance (GetTestPath ());
	input_provider = new InputProvider ();
	image_capture = new ImageCaptureProvider ();
}

ShockerScriptableControlObject::~ShockerScriptableControlObject ()
{
	g_free (test_path);

	delete input_provider;
	delete image_capture;
	LogProvider::DeleteInstance ();
}

InputProvider *
ShockerScriptableControlObject::GetInputProvider ()
{
	if (!input_provider)
		input_provider = new InputProvider ();
	return input_provider;
}

ImageCaptureProvider *
ShockerScriptableControlObject::GetImageCaptureProvider ()
{
	if (!image_capture)
		image_capture = new ImageCaptureProvider ();
	return image_capture;
}

LogProvider *
ShockerScriptableControlObject::GetLogProvider ()
{
	return LogProvider::GetInstance ();
}

void
ShockerScriptableControlObject::Connect ()
{
}

void
ShockerScriptableControlObject::SignalShutdown ()
{
	delete input_provider; input_provider = NULL;

	LogProvider::DeleteInstance ();

	shutdown_manager_queue_shutdown ();
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

