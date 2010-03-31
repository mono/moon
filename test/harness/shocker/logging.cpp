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
 * logging.cpp: Allow tests to log message and set a test result to PASS or FAIL
 *
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/utsname.h>

#include "logging.h"
#include "harness.h"

LogProvider *LogProvider::instance = NULL;

void
LogProvider::DeleteInstance ()
{
	g_return_if_fail (instance != NULL);
	
	//delete instance;
	//instance = NULL;
}

LogProvider *
LogProvider::GetInstance ()
{
	if (instance == NULL)
		instance = new LogProvider ();

	return instance;
}

LogProvider::LogProvider ()
{
	this->test_name = NULL;
	this->platform_version = NULL;
	this->platform_name = NULL;
	runtime_properties = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

LogProvider::~LogProvider ()
{
	g_free (test_name);
	g_free (platform_version);
	g_free (platform_name);
	g_hash_table_destroy (runtime_properties);
}

void
LogProvider::SetTestName (const char *test_name)
{
	g_free (this->test_name);
	this->test_name = g_strdup (test_name);
}

void
LogProvider::LogMessage (const char* str)
{
	Log ("Message", str);
}

void
LogProvider::LogWarning (const char* str)
{
	Log ("Warning", str);
}

void
LogProvider::LogHelp (const char* str)
{
	Log ("Help", str);
}

void
LogProvider::LogError (const char* str)
{
	Log ("Error", str);
}

void
LogProvider::LogDebug (const char* str)
{
	Log ("Debug", str);
}

void
LogProvider::LogResult (TestResult result)
{
	char *result_filename = getenv ("MOONLIGHT_HARNESS_RESULT_FILE");
	
	if (result_filename != NULL && result_filename [0] != 0) {
		FILE *result_fd = fopen (result_filename, "a");
		if (result_fd == NULL) {
			if (result != PASS) {
				// this is kinda rude, but if we can't log a failure,
				// just exit, since this is a test plugin after all.
				// this way the harness won't miss the failure.
				exit (1);
			}
		} else {
			gint8 res = TestResultToInt (result);
			fwrite (&res, 1, 1, result_fd);
			fclose (result_fd);
		}
	}
}

void
LogProvider::Log (const char* level, const char* msg)
{
	const char *forecolor = "39";
	if (strcmp (level, "Warning") == 0) {
		forecolor = "37";
	} else if (strcmp (level, "Error") == 0) {
		forecolor = "31";
	} else {
		forecolor = "34";
	} 
	printf ("\033[%s;49m%s: %s: %s\033[39;49m\n", forecolor, test_name, level, msg);
}

const char *
LogProvider::GetTestDirectory ()
{
	const char* dir = getenv ("MOONLIGHT_HARNESS_TESTDIRECTORY");

	if (!dir) {
		printf ("[shocker] LogProvider::GetTestDirectory (): MOONLIGHT_HARNESS_TESTDIRECTORY is not set, using /tmp instead.\n");
		dir = "/tmp";
	}

	return dir;
}

char *
LogProvider::GetTestDefinition (bool isJson)
{
	char *test_definition;
	const char *msg;
	guint8 *buffer = NULL;
	guint32 output_length;

	if (isJson) {
		msg = "TestDefinition.GetTestDefinitionJson";
	} else {
		msg = "TestDefinition.GetTestDefinition";
	}

	if (send_harness_message (msg, &buffer, &output_length)) {
		test_definition = (char *) buffer;
		buffer = NULL;

		//printf ("[shocker] LogProvider::GetTestDefinition (isJson: %i)\n", isJson);
		//printf (test_definition);
		//printf ("\n");
	} else {
		printf ("[shocker] LogProvider::GetTestDefinition (): Could not get test definition: %s\n", strerror (errno));
		test_definition = NULL;
	}

	return test_definition;
}

char *
LogProvider::GetRuntimePropertyValue (const char *propertyName)
{
	return g_strdup ((gchar *) g_hash_table_lookup (runtime_properties, propertyName));
}

void
LogProvider::SetRuntimePropertyValue (const char *propertyName, const char *value)
{
	g_hash_table_insert (runtime_properties, g_strdup (propertyName), g_strdup (value));
}

void
LogProvider::StartLog ()
{
	g_warning ("[shocker] LogProvider::StartLog (): Not implemented\n");
}

void
LogProvider::EndLog ()
{
	g_warning ("[shocker] LogProvider::EndLog (): Not implemented\n");
}

const char *
LogProvider::GetPlatformName ()
{
	if (platform_name == NULL)
		GetPlatformVersion ();

	printf ("[shocker] LogProvider::GetPlatformName (): Returning '%s', which may cause failures.\n", platform_name);

	return platform_name;
}

const char *
LogProvider::GetPlatformVersion ()
{
	utsname name;

	if (platform_version == NULL) {
		if (uname (&name) >= 0) {
			platform_name = g_strdup (name.sysname);
			platform_version = g_strdup (name.release);
			printf ("[shocker] LogProvider::GetPlatformVersion (): Returning '%s', which may cause failures.\n", platform_version);
		} else {
			platform_name = g_strdup ("");
			platform_version = g_strdup ("");
			printf ("[shocker] LogProvider::GetPlatformVersion (): Could not get platform name/version: %s\n", strerror (errno));
		}
	}

	return platform_version;
}

void 
LogDebug (const char *message)
{
	g_return_if_fail (LogProvider::GetInstance () != NULL);
	LogProvider::GetInstance ()->LogDebug (message);
}

void LogMessage (const char *message)
{
	g_return_if_fail (LogProvider::GetInstance () != NULL);
	LogProvider::GetInstance ()->LogMessage (message);
}

void LogResult (LogProvider::TestResult result)
{
	g_return_if_fail (LogProvider::GetInstance () != NULL);
	LogProvider::GetInstance ()->LogResult (result);
}

void LogError (const char *message)
{
	g_return_if_fail (LogProvider::GetInstance () != NULL);
	LogProvider::GetInstance ()->LogError (message);
}

void LogWarning (const char *message)
{
	g_return_if_fail (LogProvider::GetInstance () != NULL);
	LogProvider::GetInstance ()->LogWarning (message);
}

void GetTestDefinition (char **result)
{
	g_error ("[shocker] GetTestDefinition: Not implemented\n");
}

void TestLogger_StartLog (const char *message, const char *testDefinitionXml, const char *filePath)
{
	LogProvider::GetInstance ()->StartLog ();
}

void TestLogger_EndLog (const char *message)
{
	LogProvider::GetInstance ()->EndLog ();
}

void TestLogger_LogDebug (const char *message)
{
	LogDebug (message);
}

void TestLogger_LogMessage (const char *message)
{
	LogMessage (message);
}

void TestLogger_LogResult (guint32 result)
{
	LogResult ((LogProvider::TestResult) result);
}

void TestLogger_LogError (const char *message)
{
	LogError (message);
}

void TestLogger_LogWarning (const char *message)
{
	LogWarning (message);
}

void TestLogger_GetTestDefinition (bool isJson, gunichar2 **result)
{
	char *utf8 = LogProvider::GetInstance ()->GetTestDefinition (isJson);
	if (utf8 != NULL) {
		*result = g_utf8_to_utf16 (utf8, -1, NULL, NULL, NULL);
	} else {
		*result = NULL;
	}
	g_free (utf8);
}

void TestLogger_GetRuntimePropertyValue (const char *propertyName, gunichar2 **value)
{
	char *utf8 = LogProvider::GetInstance ()->GetRuntimePropertyValue (propertyName);
	if (utf8 != NULL) {
		*value = g_utf8_to_utf16 (utf8, -1, NULL, NULL, NULL);
	} else {
		*value = NULL;
	}
	g_free (utf8);
}

void TestLogger_SetRuntimePropertyValue (const char *propertyName, const char *value)
{
	LogProvider::GetInstance ()->SetRuntimePropertyValue (propertyName, value);
}

void TestHost_GetTestDirectory (gunichar2 **result)
{
	const char *utf8 = LogProvider::GetInstance ()->GetTestDirectory ();
	if (utf8 != NULL) {
			*result = g_utf8_to_utf16 (utf8, -1, NULL, NULL, NULL);
	} else {
			*result = NULL;
	}
}



