


#include "glib.h"
#include "gtk/gtk.h"
#include "libmoon.h"


bool
parse_time_span_test ()
{
	TimeSpan ts;

	printf ("time span tests starting\n");
	
	printf (" -- time span 1\n");
	if (!time_span_from_str ("0:0:10", &ts))
		return false;

	if (ts != TimeSpan_FromSeconds (10))
		return false;


	printf (" -- time span 2\n");
	if (!time_span_from_str ("0:10:00", &ts))
		return false;

	if (ts != TimeSpan_FromSeconds (10 * 60))
		return false;


	printf (" -- time span 3\n");
	if (!time_span_from_str ("10:00:00", &ts))
		return false;

	if (ts != TimeSpan_FromSeconds (10 * 60 * 60))
		return false;


	printf (" -- time span 4\n");
	if (!time_span_from_str ("-0:0:10", &ts))
		return false;

	if (ts != TimeSpan_FromSeconds (-10))
		return false;


	printf (" -- time span 5\n");
	if (!time_span_from_str ("10", &ts))
		return false;

	if (ts != TimeSpan_FromSeconds (10 * 60 * 60))
		return false;


	printf (" -- time span 6\n");
	if (time_span_from_str ("XXXX", &ts))
		return false; // Should fail to parse


	printf ("time span tests finished\n");
	return true;
}


bool repeat_behavior_from_str (const char *str, RepeatBehavior *res);

bool
parse_repeat_behavior_test ()
{
	printf ("repeat behavior tests starting\n");

	RepeatBehavior rb = RepeatBehavior::Forever;

	printf (" -- repeat behavior 1\n");
	if (!repeat_behavior_from_str ("Forever", &rb))
		return false;

	if (!rb.IsForever ())
		return false;

	printf (" -- repeat behavior 2\n");
	if (!repeat_behavior_from_str ("25x", &rb))
		return false;

	if (rb.GetCount () != 25)
		return false;

	printf (" -- repeat behavior 3\n");
	if (!repeat_behavior_from_str ("10", &rb))
		return false;

	if (rb.GetDuration () != TimeSpan_FromSeconds (10 * 60 * 60))
		return false;

	printf ("repeat behavior tests finished\n");
	return true;
}


bool
parse_value_test ()
{
	printf ("value tests starting\n");

	Value *v = NULL;

	printf (" -- value 1\n");
	bool valid = value_from_str (Type::BOOL, NULL, "true", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || !v->AsBool ())
		return false;
	delete v;

	printf (" -- value 2\n");
	valid = value_from_str (Type::BOOL, NULL, "TRUE", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || !v->AsBool ())
		return false;
	delete v;

	printf (" -- value 3\n");
	valid = value_from_str (Type::BOOL, NULL, "tRuE", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || !v->AsBool ())
		return false;
	delete v;
	
	printf (" -- value 4\n");
	valid = value_from_str (Type::BOOL, NULL, "false", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || v->AsBool ())
		return false;
	delete v;

	printf (" -- value 5\n");
	valid = value_from_str (Type::BOOL, NULL, "FALSE", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || v->AsBool ())
		return false;
	delete v;

	printf (" -- value 6\n");
	valid = value_from_str (Type::BOOL, NULL, "fAlSe", &v);
	if (!valid || !v || v->GetKind () != Type::BOOL || v->AsBool ())
		return false;
	delete v;

	printf (" -- value 7\n");
	valid = value_from_str (Type::DOUBLE, NULL, "1.0", &v);
	if (!valid || !v || v->GetKind () != Type::DOUBLE || v->AsDouble () != 1.0)
		return false;
	delete v;

	printf (" -- value 8\n");
	valid = value_from_str (Type::DOUBLE, NULL, "0", &v);
	if (!valid || !v || v->GetKind () != Type::DOUBLE || v->AsDouble () != 0)
		return false;
	delete v;
	
	printf (" -- value 9\n");
	valid = value_from_str (Type::DOUBLE, NULL, "1000", &v);
	if (!valid || !v || v->GetKind () != Type::DOUBLE || v->AsDouble () != 1000)
		return false;
	delete v;
	
	printf (" -- value 10\n");
	valid = value_from_str (Type::DOUBLE, NULL, "-1", &v);
	if (!valid || !v || v->GetKind () != Type::DOUBLE || v->AsDouble () != -1)
		return false;
	delete v;
	
	printf (" -- value 11\n");
	valid = value_from_str (Type::DOUBLE, NULL, "-1.0", &v);
	if (!valid || !v || v->GetKind () != Type::DOUBLE || v->AsDouble () != -1.0)
		return false;
	delete v;
	
	printf (" -- value 12\n");
	valid = value_from_str (Type::DOUBLE, NULL, "x", &v);
	if (valid || v)
		return false;
	delete v;

	printf (" -- value 13\n");
	valid = value_from_str (Type::DOUBLE, NULL, "1x", &v);
	if (valid || v)
		return false;
	delete v;

	printf (" -- value 14\n");
	valid = value_from_str (Type::DOUBLE, NULL, "x1", &v);
	if (valid || v)
		return false;
	delete v;

	printf (" -- value 15\n");
	valid = value_from_str (Type::INT64, NULL, "1", &v);
	if (!valid || !v || v->GetKind () != Type::INT64 || v->AsInt64 () != 1)
		return false;
	delete v;
	
	printf (" -- value 16\n");
	valid = value_from_str (Type::INT64, NULL, "100000", &v);
	if (!valid || !v || v->GetKind () != Type::INT64 || v->AsInt64 () != 100000)
		return false;
	delete v;

	printf (" -- value 17\n");
	valid = value_from_str (Type::INT64, NULL, "-10", &v);
	if (!valid || !v || v->GetKind () != Type::INT64 || v->AsInt64 () != -10)
		return false;
	delete v;

	printf (" -- value 18\n");
	valid = value_from_str (Type::INT64, NULL, "-10", &v);
	if (!valid || !v || v->GetKind () != Type::INT64 || v->AsInt64 () != -10)
		return false;
	delete v;

	printf (" -- value 19\n");
	valid = value_from_str (Type::INT64, NULL, "-10.0", &v);
	if (valid || v)
		return false;

	printf (" -- value 20\n");
	valid = value_from_str (Type::INT64, NULL, "xxx", &v);
	if (valid || v)
		return false;

	// Not really checking the parsing here, timespans have their own test
	// just the whole creating a Value from a timespan code path
	printf (" -- value 21\n");
	valid = value_from_str (Type::TIMESPAN, NULL, "10", &v);
	if (!valid || !v || v->GetKind () != Type::TIMESPAN || (TimeSpan_FromSeconds (10 * 60 * 60) != v->AsTimeSpan ()))
		return false;
	delete v;

// fails right now
#if FALSE
	printf (" -- value 22\n");
	valid = value_from_str (Type::TIMESPAN, NULL, "xxxx", &v);
	if (valid || v)
		return false;
#endif

	printf (" -- value 22\n");
	valid = value_from_str (Type::INT32, NULL, "1", &v);
	if (!valid || !v || v->GetKind () != Type::INT32 || v->AsInt32 () != 1)
		return false;
	delete v;

	printf (" -- value 23\n");
	valid = value_from_str (Type::INT32, NULL, "-1", &v);
	if (!valid || !v || v->GetKind () != Type::INT32 || v->AsInt32 () != -1)
		return false;
	delete v;

	printf (" -- value 24\n");
	valid = value_from_str (Type::INT32, NULL, "10000", &v);
	if (!valid || !v || v->GetKind () != Type::INT32 || v->AsInt32 () != 10000)
		return false;
	delete v;

	printf (" -- value 25\n");
	valid = value_from_str (Type::INT32, NULL, "xxx", &v);
	if (valid || v)
		return false;

	printf (" -- value 26\n");
	valid = value_from_str (Type::INT32, NULL, "x10", &v);
	if (valid || v)
		return false;

	printf (" -- value 27\n");
	valid = value_from_str (Type::INT32, NULL, "10x", &v);
	if (valid || v)
		return false;

	/// INT32 also covers enums, which gets it's own special test


	
	printf (" -- value 28\n");
	valid = value_from_str (Type::STRING, NULL, "hello", &v);
	if (!valid || !v || v->GetKind () != Type::STRING || strcmp (v->AsString (), "hello"))
		return false;
	delete v;

	
	// Just make sure colors can be created, actual parsing gets it's own special test function
	printf (" -- value 29\n");
	valid = value_from_str (Type::COLOR, NULL, "Red", &v);
	if (!valid || !v || v->GetKind () != Type::COLOR || *(v->AsColor ()) != Color (1.0, 0, 0, 1.0))
		return false;
	delete v;

// fails
//	
//	printf (" -- value 30\n");
//	valid = value_from_str (Type::COLOR, NULL, "XXXXXXXX", &v);
//	if (valid || v)
//		return false;
	
	printf ("value tests finished\n");
	return true;
}


int
main ()
{
	int res = 0;

	if (!parse_time_span_test ()) {
		printf ("********** time span tests failed **********\n");
		res = 1;
	}

	if (!parse_repeat_behavior_test ()) {
		printf ("********** repeat behavior tests failed **********\n");
		res = 1;
	}

	if (!parse_value_test ()) {
		printf ("********** value tests failed **********\n");
		res = 1;
	}

	return res;
}
