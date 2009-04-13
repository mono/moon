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

#include "logging.h"

LogProvider *LogProvider::instance = NULL;

void
LogProvider::CreateInstance (const char *test_name)
{
	g_return_if_fail (instance == NULL);
	
	instance = new LogProvider (test_name);
}

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
	return instance;
}

LogProvider::LogProvider (const char* test_name)
{
	this->test_name = strdup (test_name);
}

LogProvider::~LogProvider ()
{
	free (test_name);
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
	printf ("[shocker] GetTestDefinition: Not implemented\n");
}



