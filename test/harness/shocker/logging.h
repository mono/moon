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
 * logging.h: Allow tests to log message and set a test result to PASS or FAIL
 *
 */

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <glib.h>

class LogProvider {
private:
	LogProvider (const char *test_name);
	virtual ~LogProvider ();
	
	static LogProvider *instance;
public:
	enum TestResult {
		PASS = 1,
		FAIL = -1,
	};

	static TestResult IntToResult (int i) { return (TestResult) i; }
	static int TestResultToInt (TestResult result) { return (int) result; }
	
	static void CreateInstance (const char *test_name);
	static LogProvider *GetInstance ();
	static void DeleteInstance ();

	void LogMessage (const char* str);
	void LogWarning (const char* str);
	void LogHelp (const char* str);
	void LogError (const char* str);
	void LogDebug (const char* str);

	void LogResult (TestResult result);
private:
	void Log (const char* level, const char* msg);
	
	char *test_name;
};

G_BEGIN_DECLS

void LogDebug (const char *message);
void LogMessage (const char *message);
void LogResult (LogProvider::TestResult result);
void LogError (const char *message);
void LogWarning (const char *message);
void GetTestDefinition (char **result);

G_END_DECLS

#endif

