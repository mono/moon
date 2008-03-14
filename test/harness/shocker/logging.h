

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "netscape.h"

#ifdef DBUS_ENABLED
#include <dbus/dbus-glib.h>
#endif


//
//  Logs are always stored in shocker-log.txt
//  The log file format is:  <log level>:<test id>:<time stamp>:<message>
//

class LogProvider {

public:
	enum TestResult {
		PASS = 1,
		FAIL = -1,
	};

	static TestResult IntToResult (int i) { return (TestResult) i; }
	static int TestResultToInt (TestResult result) { return (int) result; }

	LogProvider (const char* test_name);
	virtual ~LogProvider ();

	void LogMessage (const char* str);
	void LogWarning (const char* str);
	void LogHelp (const char* str);
	void LogError (const char* str);
	void LogDebug (const char* str);

	void LogResult (TestResult result);

private:
	FILE* log_file;
	char* test_name;

#ifdef DBUS_ENABLED
	DBusGProxy* dbus_proxy;
#endif

	void Log (const char* level, const char* msg);
};


#endif

