/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * logging.cpp: Allow tests to log message and set a test result to PASS or FAIL
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "debug.h"
#include "shocker.h"
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
}

LogProvider::~LogProvider ()
{
	g_free (test_name);
	g_free (platform_version);
	g_free (platform_name);
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

	if (result != PASS)
		printf ("\033[31;49m%s: %s: %s\033[39;49m\n", test_name, "LogResult", "Test run marked as failed");
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
		printf ("[%i shocker] LogProvider::GetTestDirectory (): MOONLIGHT_HARNESS_TESTDIRECTORY is not set, using /tmp instead.\n", getpid ());
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

	if (Harness::SendMessage (msg, &buffer, &output_length)) {
		test_definition = (char *) buffer;
		buffer = NULL;

		if (shocker_flags & SHOCKER_DEBUG_HARNESS) {
			printf ("[%i shocker] LogProvider::GetTestDefinition (isJson: %i):\n", getpid (), isJson);
			printf (test_definition);
			printf ("\n");
		}
	} else {
		printf ("[%i shocker] LogProvider::GetTestDefinition (): Could not get test definition: %s\n", getpid (), strerror (errno));
		test_definition = NULL;
	}

	return test_definition;
}

void
LogProvider::StartLog ()
{
	g_warning ("[%i shocker] LogProvider::StartLog (): Not implemented\n", getpid ());
}

void
LogProvider::EndLog ()
{
	g_warning ("[%i shocker] LogProvider::EndLog (): Not implemented\n", getpid ());
}

const char *
LogProvider::GetPlatformName ()
{
	if (platform_name == NULL)
		GetPlatformVersion ();

	printf ("[%i shocker] LogProvider::GetPlatformName (): Returning '%s', which may cause failures.\n", getpid (), platform_name);

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
			printf ("[%i shocker] LogProvider::GetPlatformVersion (): Returning '%s', which may cause failures.\n", getpid (), platform_version);
		} else {
			platform_name = g_strdup ("");
			platform_version = g_strdup ("");
			printf ("[%i shocker] LogProvider::GetPlatformVersion (): Could not get platform name/version: %s\n", getpid (), strerror (errno));
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
	Shocker_FailTestFast ("GetTestDefinition: Not implemented");
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
	char *utf8 = Harness::GetRuntimePropertyValue (propertyName);
	if (utf8 != NULL) {
		*value = g_utf8_to_utf16 (utf8, -1, NULL, NULL, NULL);
	} else {
		*value = NULL;
	}
	g_free (utf8);
}

void TestLogger_SetRuntimePropertyValue (const char *propertyName, const char *value)
{
	Harness::SetRuntimePropertyValue (propertyName, value);
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



