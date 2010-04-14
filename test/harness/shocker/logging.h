/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * logging.h: Allow tests to log message and set a test result to PASS or FAIL
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <glib.h>

class LogProvider {
private:
	LogProvider ();
	virtual ~LogProvider ();
	
	static LogProvider *instance;
public:
	enum TestResult {
		PASS = 1,
		FAIL = -1,
	};

	static TestResult IntToResult (int i) { return (TestResult) i; }
	static int TestResultToInt (TestResult result) { return (int) result; }
	
	static LogProvider *GetInstance ();
	static void DeleteInstance ();

	void LogMessage (const char* str);
	void LogWarning (const char* str);
	void LogHelp (const char* str);
	void LogError (const char* str);
	void LogDebug (const char* str);

	void LogResult (TestResult result);
	char *GetTestDefinition (bool isJson);
	const char *GetTestDirectory ();
	char *GetRuntimePropertyValue (const char *propertyName);
	void SetRuntimePropertyValue (const char *propertyName, const char *value);
	const char *GetPlatformName ();
	const char *GetPlatformVersion ();

	void SetTestName (const char *test_name);
	void StartLog ();
	void EndLog ();

private:
	void Log (const char* level, const char* msg);
	
	char *test_name;
	char *platform_version;
	char *platform_name;
	GHashTable *runtime_properties;
};

G_BEGIN_DECLS

void LogDebug (const char *message);
void LogMessage (const char *message);
void LogResult (LogProvider::TestResult result);
void LogError (const char *message);
void LogWarning (const char *message);
void GetTestDefinition (char **result);

void TestLogger_StartLog (const char *message, const char *testDefinitionXml, const char *filePath);
void TestLogger_EndLog (const char *message);
void TestLogger_LogDebug (const char *message);
void TestLogger_LogMessage (const char *message);
void TestLogger_LogResult (guint32 result);
void TestLogger_LogError (const char *message);
void TestLogger_LogWarning (const char *message);
void TestLogger_GetTestDefinition (bool isJson, gunichar2 **result);
void TestLogger_GetRuntimePropertyValue (const char *propertyName, gunichar2 **value);
void TestLogger_SetRuntimePropertyValue (const char *propertyName, const char *value);

void TestHost_GetTestDirectory (gunichar2 **result);

G_END_DECLS

#endif

