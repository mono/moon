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
#include <time.h>
#include "logging.h"

#define SHOCKER_LOGGER_SERVICE    "mono.moonlight.tests"
#define SHOCKER_LOGGER_PATH       "/mono/moonlight/tests/logger"
#define SHOCKER_LOGGER_INTERFACE  "mono.moonlight.tests.logger.ITestLogger"

#define SHOCKER_LOG_FILE 	"shocker-log.txt"

#define PASS_MESSAGE		"PASS"
#define FAIL_MESSAGE		"FAIL"

#define LOG_TIME_FORMAT		"%Y-%m-%d %H:%M:%S"



LogProvider::LogProvider (const char* test_name) : log_file (NULL)
{
	g_assert (test_name);

	this->test_name = strdup (test_name);

	log_file = fopen (SHOCKER_LOG_FILE, "a+");
	if (!log_file) {
		g_warning ("Unable to open log file, logging disabled.\n");
		log_file = NULL;
	}

#ifdef DBUS_ENABLED
	g_type_init ();

	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus: %s\n", error->message);
		g_error_free (error);
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			SHOCKER_LOGGER_SERVICE,
			SHOCKER_LOGGER_PATH,
			SHOCKER_LOGGER_INTERFACE);
#endif

}

LogProvider::~LogProvider ()
{
	free (test_name);

	if (log_file)
		fclose (log_file);
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
#ifdef DBUS_ENABLED
	g_return_if_fail (dbus_proxy);

	dbus_g_proxy_call_no_reply (dbus_proxy, "LogResult", G_TYPE_STRING, test_name, G_TYPE_INT, TestResultToInt (result), G_TYPE_INVALID);
#else
	g_warning ("DBUS NOT ENABLED, result will not be logged: %d\n", TestResultToInt (result));
#endif
}

void
LogProvider::Log (const char* level, const char* msg)
{
	g_return_if_fail (log_file);

	char timestr [64];
	time_t timet = time (NULL);
	struct tm* timeinfo = localtime (&timet);

	strftime (timestr, 64, LOG_TIME_FORMAT, timeinfo);

	fprintf (log_file, "%s:%s:%s:%s\n", level, timestr, test_name, msg);
	fflush (log_file);

#ifdef DBUS_ENABLED
	g_return_if_fail (dbus_proxy);
	
	dbus_g_proxy_call_no_reply (dbus_proxy, "Log", G_TYPE_STRING, test_name, G_TYPE_STRING, level, G_TYPE_STRING, msg, G_TYPE_INVALID);
#endif

}





