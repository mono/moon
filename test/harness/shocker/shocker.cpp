/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * shocker.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "netscape.h"
#include "shocker.h"
#include "plugin.h"
#include "browser.h"
#include "logging.h"
#include "input.h"
#include "shutdown-manager.h"
#include "harness.h"
#include "image-capture.h"


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
Shocker_Initialize ()
{
	ShockerScriptableControlClass = new ShockerScriptableControlType ();
	RenderDataCapturerClass = new RenderDataCapturerType ();

	return NPERR_NO_ERROR;
}

void
Shocker_Shutdown ()
{
	delete ShockerScriptableControlClass;
	delete RenderDataCapturerClass;
}

void
Shocker_FailTestFast (const char *msg)
{
	const char *results_file = getenv ("MOONLIGHT_HARNESS_RESULT_FILE");
	bool crash = true;

	if (results_file != NULL && results_file [0] != 0) {
		FILE *fd = fopen (results_file, "a");
		if (fd != NULL) {
			guint8 failure = 255;
			if (fwrite (&failure, 1, 1, fd) == 1)
				crash = false; // the harness will now see that the test failed.
			fclose (fd);
		}
	}
	LogProvider::GetInstance ()->LogError (g_strdup_printf ("[%i shocker] libshocker requested a fast test failure: %s\n", getpid (), msg));
	exit (crash ? 1 : 0);
}

static char *
NPN_strdup (const char *tocopy)
{
	int len = strlen (tocopy);
	char *ptr = (char *) Browser::Instance ()->MemAlloc (len + 1);
	if (ptr != NULL) {
		strcpy (ptr, tocopy);
		// WebKit should calloc so we dont have to do this
		ptr[len] = 0;
	}

	return ptr;
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

/*
 * ShockerScriptableObject
 */

static void
shocker_scriptable_object_deallocate (NPObject *npobj)
{
	delete ((ShockerScriptableObject *) npobj);
}

static void
shocker_scriptable_object_invalidate (NPObject *npobj)
{
	((ShockerScriptableObject *) npobj)->Invalidate ();
}

static bool
shocker_scriptable_object_has_method (NPObject *npobj, NPIdentifier id)
{
	return ((ShockerScriptableObject *) npobj)->HasMethod (id);
}

static bool
shocker_scriptable_object_has_property (NPObject *npobj, NPIdentifier id)
{
	return ((ShockerScriptableObject *) npobj)->HasProperty (id);
}

static bool
shocker_scriptable_object_get_property (NPObject *npobj, NPIdentifier id, NPVariant *result)
{
	return ((ShockerScriptableObject *) npobj)->GetProperty (id, result);
}

static bool
shocker_scriptable_object_set_property (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return ((ShockerScriptableObject *) npobj)->SetProperty (name, value);
}

static bool
shocker_scriptable_object_remove_property (NPObject *npobj, NPIdentifier name)
{
	return ((ShockerScriptableObject *) npobj)->RemoveProperty (name);
}

static bool
shocker_scriptable_object_invoke (NPObject *npobj, NPIdentifier id, const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	return ((ShockerScriptableObject *) npobj)->Invoke (id, args, arg_count, result);
}

static bool
shocker_scriptable_object_invoke_default (NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((ShockerScriptableObject *) npobj)->InvokeDefault (args, argCount, result);
}

ShockerScriptableObjectType::ShockerScriptableObjectType ()
{
	deallocate     = shocker_scriptable_object_deallocate;
	invalidate     = shocker_scriptable_object_invalidate;
	hasMethod      = shocker_scriptable_object_has_method;
	invoke         = shocker_scriptable_object_invoke;
	invokeDefault  = shocker_scriptable_object_invoke_default;
	hasProperty    = shocker_scriptable_object_has_property;
	getProperty    = shocker_scriptable_object_get_property;
	setProperty    = shocker_scriptable_object_set_property;
	removeProperty = shocker_scriptable_object_remove_property;
}

bool
ShockerScriptableObject::HasMethod (NPIdentifier id)
{
	bool res = false;
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	ShockerScriptableObjectMethod *method = GetMethods ();

	while (method->name) {
		if (!strcmp (name, method->name)) {
			res = true;
			break;
		}
		method++;
	}

	LOG_PLUGIN ("[%i shocker] %s::HasMethod (%s): %i\n", getpid (), GetTypeName (), name, res);

	free (name);
	return res;
}

bool
ShockerScriptableObject::Invoke (NPIdentifier id, const NPVariant *args, guint32 arg_count, NPVariant *result)
{
	bool res = false;
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	ShockerScriptableObjectMethod *method = GetMethods ();

	while (method->name) {
		if (!strcmp (name, method->name)) {
			scriptable_method m = method->Invoke;
			(this->*m) (args, arg_count, result);
			res = true;
			break;
		}
		method++;
	}

	LOG_PLUGIN ("[%i shocker] %s::Invoke (%s): %i\n", getpid (), GetTypeName (), name, res);

	free (name);
	return res;
}

/*
 * ShockerScriptableControl
 */
ShockerScriptableControlType *ShockerScriptableControlClass = NULL;

static NPObject*
shocker_scriptable_control_allocate (NPP instance, NPClass *)
{
	return new ShockerScriptableControlObject (instance);
}

ShockerScriptableControlType::ShockerScriptableControlType ()
{
	allocate = shocker_scriptable_control_allocate;
}

ShockerScriptableControlObject::ShockerScriptableControlObject (NPP instance)
	: ShockerScriptableObject (instance)
{
	image_capture = new ImageCaptureProvider ();
	test_path = NULL;
	render_data_capturer = NULL;
	LogProvider::GetInstance ()->SetTestName (GetTestPath ());
}

ShockerScriptableControlObject::~ShockerScriptableControlObject ()
{
	g_free (test_path);

	delete image_capture;
	LogProvider::DeleteInstance ();
}

InputProvider *
ShockerScriptableControlObject::GetInputProvider ()
{
	return InputProvider::GetInstance ();
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

const char *
ShockerScriptableControlObject::GetTestPath ()
{
	if (test_path == NULL) {
		NPVariant nplocation;
		NPVariant nppath;
		NPObject *window = NULL;
		NPIdentifier identifier = Browser::Instance ()->GetStringIdentifier ("location");
	
	
		Browser::Instance ()->GetValue (instance, NPNVWindowNPObject, &window);
		Browser::Instance ()->GetProperty (instance, window, identifier, &nplocation);
	
		identifier = Browser::Instance ()->GetStringIdentifier ("pathname");
		Browser::Instance ()->GetProperty (instance, NPVARIANT_TO_OBJECT (nplocation), identifier, &nppath);
	
		test_path = g_path_get_basename (STR_FROM_VARIANT (nppath));
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetTestPath (): %s\n", getpid (), test_path);

	return test_path;
}


ShockerScriptableControlMethod ShockerScriptableControlObject::methods [] = {
	{ "Connect", &ShockerScriptableControlObject::Connect },
	{ "SignalShutdown", &ShockerScriptableControlObject::SignalShutdown },

	{ "LogMessage", &ShockerScriptableControlObject::LogMessage },
	{ "LogWarning", &ShockerScriptableControlObject::LogWarning },
	{ "LogHelp", &ShockerScriptableControlObject::LogHelp },
	{ "LogError", &ShockerScriptableControlObject::LogError },
	{ "LogDebug", &ShockerScriptableControlObject::LogDebug },

	// Do the same thing
	{ "LogResult", &ShockerScriptableControlObject::LogResult },
	{ "TryLogResult", &ShockerScriptableControlObject::LogResult },

	{ "moveMouseLogarithmic", &ShockerScriptableControlObject::MoveMouseLogarithmic },
	{ "moveMouse", &ShockerScriptableControlObject::MoveMouse },
	{ "mouseIsAtPosition", &ShockerScriptableControlObject::MouseIsAtPosition },
	{ "mouseDoubleClick", &ShockerScriptableControlObject::MouseDoubleClick },
	{ "mouseLeftClick", &ShockerScriptableControlObject::MouseLeftClick },
	{ "mouseRightClick", &ShockerScriptableControlObject::MouseRightClick },
	{ "mouseLeftButtonDown", &ShockerScriptableControlObject::MouseLeftButtonDown },
	{ "mouseLeftButtonUp", &ShockerScriptableControlObject::MouseLeftButtonUp },
	{ "sendKeyInput", &ShockerScriptableControlObject::SendKeyInput },

	{ "CaptureSingleImage", &ShockerScriptableControlObject::CaptureSingleImage },
	{ "CaptureMultipleImages", &ShockerScriptableControlObject::CaptureMultipleImages },

	// New test plugin methods in 2.0
	{ "setKeyboardInputSpeed", &ShockerScriptableControlObject::SetKeyboardInputSpeed },
	{ "CompareImages", &ShockerScriptableControlObject::CompareImages },
	{ "GetActiveInputLocaleId", &ShockerScriptableControlObject::GetActiveInputLocaleId },
	{ "ActivateKeyboardLayout", &ShockerScriptableControlObject::ActivateKeyboardLayout },
	{ "IsInputLocaleInstalled", &ShockerScriptableControlObject::IsInputLocaleInstalled },
	{ "GetTestDirectory", &ShockerScriptableControlObject::GetTestDirectory },
	{ "GetTestDefinition", &ShockerScriptableControlObject::GetTestDefinition },
	{ "GetRuntimePropertyValue", &ShockerScriptableControlObject::GetRuntimePropertyValue },
	{ "SetRuntimePropertyValue", &ShockerScriptableControlObject::SetRuntimePropertyValue },
	{ "CleanDRM", &ShockerScriptableControlObject::CleanDRM },
	{ "SwitchToHighContrastScheme", &ShockerScriptableControlObject::SwitchToHighContrastScheme },
	
	// New test plugin methods in 3.0/4.0
	{ "StartLog", &ShockerScriptableControlObject::StartLog },
	{ "EndLog", &ShockerScriptableControlObject::EndLog },
	{ "mouseWheel", &ShockerScriptableControlObject::MouseWheel },
	{ "EnablePrivacyPrompts", &ShockerScriptableControlObject::EnablePrivacyPrompts },
	{ "GetRenderDataCapturer", &ShockerScriptableControlObject::GetRenderDataCapturer },
	{ "LogPerfEvent", &ShockerScriptableControlObject::LogPerfEvent },
	{ "InitializePerfProvider", &ShockerScriptableControlObject::InitializePerfProvider },
	{ "UninitializePerfProvider", &ShockerScriptableControlObject::UninitializePerfProvider },
	{ "GetPlatformName", &ShockerScriptableControlObject::GetPlatformName },
	{ "GetPlatformVersion", &ShockerScriptableControlObject::GetPlatformVersion },
	{ "StartWebCamWriter", &ShockerScriptableControlObject::StartWebCamWriter },
	{ "StopWebCamWriter", &ShockerScriptableControlObject::StopWebCamWriter },
    
	{ NULL, NULL }
};

bool
ShockerScriptableControlObject::HasProperty (NPIdentifier id)
{
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	bool res = false;
	
	res = !strcmp (name, "X") || !strcmp (name, "Y");
	
	free (name);
		
	LOG_PLUGIN ("ShockerScriptableControlObject::HasProperty (%s): %i\n", name, res);

	return res;
}

bool
ShockerScriptableControlObject::GetProperty (NPIdentifier id, NPVariant *result)
{
	char *name = Browser::Instance ()->UTF8FromIdentifier (id);
	bool res = false;
	PluginObject *plugin = GetPluginObject ();

	if (!strcmp (name, "X")) {
		INT32_TO_NPVARIANT (plugin->GetX (), *result);
		res = true;
	} else if (!strcmp (name, "Y")) {
		INT32_TO_NPVARIANT (plugin->GetY (), *result);
		res = true;
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetProperty (%s) x: %i, y: %i => %i\n", getpid (), name, plugin->GetX (), plugin->GetY (), res);

	return res;
}

void
ShockerScriptableControlObject::Connect (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::Connect ()\n", getpid ());
	// Nothing to do here
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::SignalShutdown (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::SignalShutdown ()\n", getpid ());
	LogProvider::DeleteInstance ();
	shutdown_manager_queue_shutdown ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogMessage (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogMessage ('%s')\n", getpid (), STR_FROM_VARIANT (args [0]));

	GetLogProvider ()->LogMessage (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogWarning (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogWarning ('%s')\n", getpid (), STR_FROM_VARIANT (args [0]));

	GetLogProvider ()->LogWarning (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogHelp (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogHelp ('%s')\n", getpid (), STR_FROM_VARIANT (args [0]));

	GetLogProvider ()->LogHelp (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogError (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogError ('%s')\n", getpid (), STR_FROM_VARIANT (args [0]));

#if DEBUG_ERROR_GECKO == 1
	findRealErrorOnStack (this);
#endif
	GetLogProvider ()->LogError (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogDebug (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_STRING (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogDebug ('%s')\n", getpid (), STR_FROM_VARIANT (args [0]));

	GetLogProvider ()->LogDebug (STR_FROM_VARIANT (args [0]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::LogResult (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::LogResult (%i )\n", getpid (), NUMBER_TO_INT32 (args [0]));

	GetLogProvider ()->LogResult (LogProvider::IntToResult (NUMBER_TO_INT32 (args [0])));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MoveMouseLogarithmic (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MoveMouseLogarithmic (%i, %i)\n", getpid (), NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));

	GetInputProvider ()->MoveMouseLogarithmic (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MoveMouse (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MoveMouse (%i, %i)\n", getpid (), NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));

	GetInputProvider ()->MoveMouse (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseIsAtPosition (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 2);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseIsAtPosition (%i, %i)\n", getpid (), NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1]));
;
	BOOLEAN_TO_NPVARIANT (GetInputProvider ()->MouseIsAtPosition (NUMBER_TO_INT32 (args [0]), NUMBER_TO_INT32 (args [1])), *result);
}

void
ShockerScriptableControlObject::MouseDoubleClick (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	unsigned int delay = 0;
	if (arg_count >= 1) {
		g_assert (NPVARIANT_IS_NUMBER (args [0]));
		delay = NUMBER_TO_INT32 (args [0]);
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseDoubleClick (delay: %i)\n", getpid (), delay);

	GetInputProvider ()->MouseDoubleClick (delay);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseLeftClick (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	unsigned int delay = 0;
	if (arg_count >= 1) {
		g_assert (NPVARIANT_IS_NUMBER (args [0]));
		delay = NUMBER_TO_INT32 (args [0]);
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseLeftClick (delay: %i)\n", getpid (), delay);

	GetInputProvider ()->MouseLeftClick (delay);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseRightClick (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	unsigned int delay = 0;
	if (arg_count >= 1) {
		g_assert (NPVARIANT_IS_NUMBER (args [0]));
		delay = NUMBER_TO_INT32 (args [0]);
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseRightClick (delay: %i)\n", getpid (), delay);

	GetInputProvider ()->MouseRightClick (delay);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseLeftButtonDown (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	unsigned int delay = 0;
	if (arg_count >= 1) {
		g_assert (NPVARIANT_IS_NUMBER (args [0]));
		delay = NUMBER_TO_INT32 (args [0]);
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseLeftButtonDown (delay: %i)\n", getpid (), delay);

	GetInputProvider ()->MouseLeftButtonDown (delay);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseLeftButtonUp (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	unsigned int delay = 0;
	if (arg_count >= 1) {
		g_assert (NPVARIANT_IS_NUMBER (args [0]));
		delay = NUMBER_TO_INT32 (args [0]);
	}

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::MouseLeftButtonUp (delay: %i)\n", getpid (), delay);

	GetInputProvider ()->MouseLeftButtonUp (delay);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::SendKeyInput (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count >= 4);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [1]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [2]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [3]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::SendKeyInput (key_code: %i, key_down: %i extended: %i, unicode: %i)\n", getpid (),
		NUMBER_TO_INT32 (args [0]), NPVARIANT_TO_BOOLEAN (args [1]), NPVARIANT_TO_BOOLEAN (args [2]), NPVARIANT_TO_BOOLEAN (args [3]));

	GetInputProvider ()->SendKeyInput (NUMBER_TO_INT32 (args [0]), NPVARIANT_TO_BOOLEAN (args [1]),
		NPVARIANT_TO_BOOLEAN (args [2]), NPVARIANT_TO_BOOLEAN (args [3]));

	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::SetKeyboardInputSpeed (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 1);
	g_assert (NPVARIANT_IS_NUMBER (args [0]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::SetKeyboardInputSpeed (%i)\n", getpid (), NUMBER_TO_INT32 (args [0]));

	GetInputProvider ()->SetKeyboardInputSpeed (NUMBER_TO_INT32 (args [0]));

	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::CompareImages (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	guint8 res = false;

	g_assert (arg_count >= 5);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_STRING (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));
	g_assert (NPVARIANT_IS_STRING (args [3]));
	g_assert (NPVARIANT_IS_BOOLEAN (args [4]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::CompareImages (imageFile1: '%s', imageFile2: '%s', tolerance: %i, diffFileName: '%s', copySourceFiles: %i)\n", getpid (),
		STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]), NUMBER_TO_INT32 (args [2]),
		STR_FROM_VARIANT (args [3]), NPVARIANT_TO_BOOLEAN (args [4]));


	::CompareImages (STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]), NUMBER_TO_INT32 (args [2]),
		STR_FROM_VARIANT (args [3]), NPVARIANT_TO_BOOLEAN (args [4]), &res);
	
	BOOLEAN_TO_NPVARIANT ((bool) res, *result);
}

void
ShockerScriptableControlObject::GetActiveInputLocaleId (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("GetActiveInputLocaleId: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::ActivateKeyboardLayout (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	const char *friendlyName;
	int x, y;
	guint8 res = false;

	g_assert (arg_count >= 3);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_NUMBER (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));

	friendlyName = STR_FROM_VARIANT (args [0]);
	x = NUMBER_TO_INT32 (args [1]);
	y = NUMBER_TO_INT32 (args [2]);

	printf ("[%i shocker] ShockerScriptableControlObject::ActivateKeyboardLayout (friendlyName: '%s' x: %i y: %i)\n", getpid (), friendlyName, x, y);

	Shocker_FailTestFast ("ActivateKeyboardLayout: Not implemented");
	BOOLEAN_TO_NPVARIANT (res != 0, *result);
}

void
ShockerScriptableControlObject::IsInputLocaleInstalled (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("IsInputLocaleInstalled: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::GetTestDirectory (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	char *retval;
	retval = NPN_strdup (LogProvider::GetInstance ()->GetTestDirectory ());
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetTestDirectory (): %s\n", getpid (), retval);
	STRINGZ_TO_NPVARIANT (retval, *result);
}

void
ShockerScriptableControlObject::GetTestDefinition (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	char *test_definition;
	bool free_test_definition = false;
	char *retval;
	bool isJson = false;
	
	g_assert (arg_count <= 1);

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetTestDefinition ()\n", getpid ());

	test_definition = getenv ("MOONLIGHT_HARNESS_TESTDEFINITION");

	if (test_definition == NULL) {

		if (arg_count > 0) {
			g_assert (NPVARIANT_IS_BOOLEAN (args [0]));
			isJson = NPVARIANT_TO_BOOLEAN (args [0]);
		}

		test_definition = LogProvider::GetInstance ()->GetTestDefinition (isJson);
		free_test_definition = true;
	}

	retval = NPN_strdup (test_definition == NULL ? "" : test_definition);
	
	STRINGZ_TO_NPVARIANT (retval, *result);
	if (free_test_definition)
		g_free (test_definition);
}

void
ShockerScriptableControlObject::GetRuntimePropertyValue (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("GetRuntimePropertyValue: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::SetRuntimePropertyValue (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("SetRuntimePropertyValue: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::CleanDRM (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("CleanDRM: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::SwitchToHighContrastScheme (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("SwitchToHighContrastScheme: Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::StartLog (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::StartLog ()\n", getpid ());
	LogProvider::GetInstance ()->StartLog ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::EndLog (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::EndLog ()\n", getpid ());
	LogProvider::GetInstance ()->EndLog ();
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::MouseWheel (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
        int clicks = 0;
        if (arg_count >= 1) {
                g_assert (NPVARIANT_IS_NUMBER (args [0]));
                clicks = NUMBER_TO_INT32 (args [0]);
        }

        GetInputProvider ()->MouseWheel (clicks);
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::EnablePrivacyPrompts (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("EnablePrivacyPrompts (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::GetRenderDataCapturer (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetRenderDataCapturer ()\n", getpid ());
	if (render_data_capturer == NULL) {
		render_data_capturer = (RenderDataCapturerObject *) Browser::Instance ()->CreateObject (instance, RenderDataCapturerClass);
	}
	OBJECT_TO_NPVARIANT (render_data_capturer, *result);
	Browser::Instance ()->RetainObject (render_data_capturer);
}

void
ShockerScriptableControlObject::LogPerfEvent (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("LogPerfEvent (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::InitializePerfProvider (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("InitializePerfProvider (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::UninitializePerfProvider (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("UninitializePerfProvider (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::GetPlatformName (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	char *retval;
	retval = NPN_strdup (LogProvider::GetInstance ()->GetPlatformName ());
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetPlatformName (): %s\n", getpid (), retval);
	STRINGZ_TO_NPVARIANT (retval, *result);
}

void
ShockerScriptableControlObject::GetPlatformVersion (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	char *retval;
	retval = NPN_strdup (LogProvider::GetInstance ()->GetPlatformVersion ());
	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::GetPlatformVersion (): %s\n", getpid (), retval);
	STRINGZ_TO_NPVARIANT (retval, *result);
}

void
ShockerScriptableControlObject::StartWebCamWriter (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("StartWebCamWriter (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::StopWebCamWriter (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("StopWebCamWriter (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::CaptureSingleImage (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	g_assert (arg_count == 6);
	g_assert (NPVARIANT_IS_STRING (args [0]));
	g_assert (NPVARIANT_IS_STRING (args [1]));
	g_assert (NPVARIANT_IS_NUMBER (args [2]));
	g_assert (NPVARIANT_IS_NUMBER (args [3]));
	g_assert (NPVARIANT_IS_NUMBER (args [4]));
	g_assert (NPVARIANT_IS_NUMBER (args [5]));

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::CaptureSingleImage (image_dir: '%s', file_name: '%s', x: %i, y: %i, width: %i, height: %i)\n", getpid (), 
		STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]), NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]), NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]));

	GetImageCaptureProvider ()->CaptureSingleImage (STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]),
			NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]), NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]));

	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
ShockerScriptableControlObject::CaptureMultipleImages (const NPVariant *args, uint32_t arg_count, NPVariant *result)
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
		path = GetTestPath ();

	LOG_PLUGIN ("[%i shocker] ShockerScriptableControlObject::CaptureMultipleImages (base_dir: '%s', file_name: '%s', x: %i, y: %i, width: %i, height: %i, count: %i, interval: %i, initial_delay: %i)\n", getpid (), 
		STR_FROM_VARIANT (args [0]), STR_FROM_VARIANT (args [1]),  NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]),
		NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]),NUMBER_TO_INT32 (args [6]), NUMBER_TO_INT32 (args [7]),
		NUMBER_TO_INT32 (args [8]));

	GetImageCaptureProvider ()->CaptureMultipleImages (path, NUMBER_TO_INT32 (args [2]), NUMBER_TO_INT32 (args [3]),
			NUMBER_TO_INT32 (args [4]), NUMBER_TO_INT32 (args [5]),NUMBER_TO_INT32 (args [6]), NUMBER_TO_INT32 (args [7]),
			NUMBER_TO_INT32 (args [8]));

	BOOLEAN_TO_NPVARIANT (true, *result);
}

/*
 * RenderDataCapturer
 */
RenderDataCapturerType *RenderDataCapturerClass = NULL;
RenderCapturerDataMethod RenderDataCapturerObject::methods [] = {
	{ "CaptureSingleImage", &RenderDataCapturerObject::CaptureSingleImage },
	{ "RegisterGetNextFrameDataRequest", &RenderDataCapturerObject::RegisterGetNextFrameDataRequest },
	{ "IsFrameDataAvailable",&RenderDataCapturerObject::IsFrameDataAvailable },
	{ "GetFrameData", &RenderDataCapturerObject::GetFrameData },
	{ "IsEnabled", &RenderDataCapturerObject::IsEnabled },
	{ "SetFailMode", &RenderDataCapturerObject::SetFailMode },
	{ "GetFailMode", &RenderDataCapturerObject::GetFailMode },

	{ NULL, NULL },
};

static NPObject*
render_data_capturer_allocate (NPP instance, NPClass *)
{
	return new RenderDataCapturerObject (instance);
}

RenderDataCapturerType::RenderDataCapturerType ()
{
	allocate = render_data_capturer_allocate;
}

RenderDataCapturerObject::RenderDataCapturerObject (NPP instance)
	: ShockerScriptableObject (instance)
{
}

void RenderDataCapturerObject::CaptureSingleImage (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::CaptureSingleImage (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::RegisterGetNextFrameDataRequest (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::RegisterGetNextFrameDataRequest (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::IsFrameDataAvailable (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::IsFrameDataAvailable (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::GetFrameData (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::GetFrameData (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::IsEnabled (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::IsEnabled (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::SetFailMode (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::SetFailMode (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}

void
RenderDataCapturerObject::GetFailMode (const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	Shocker_FailTestFast ("RenderDataCapturerObject::GetFailMode (): Not implemented");
	BOOLEAN_TO_NPVARIANT (true, *result);
}


bool
RenderDataCapturerObject::HasProperty (NPIdentifier id)
{
	LOG_PLUGIN ("[%i shocker] RenderDataCapturerObject::HasProperty (%s): nope\n", getpid (), Browser::Instance ()->UTF8FromIdentifier (id));
	return false;
}

bool
RenderDataCapturerObject::GetProperty (NPIdentifier id, NPVariant *result)
{
	LOG_PLUGIN ("[%i shocker] RenderDataCapturerObject::GetProperty (%s): nope\n", getpid (), Browser::Instance ()->UTF8FromIdentifier (id));
	return false;
}
