/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * error.h: ErrorEventArgs and its subclasses
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_ERROR_H__
#define __MOON_ERROR_H__

class ErrorEventArgs;


#include "enums.h"
#include "eventargs.h"

/* @Namespace=None,ManagedDependencyProperties=None */
class ErrorEventArgs : public EventArgs  {
protected:
	virtual ~ErrorEventArgs ();

public:
	ErrorEventArgs (ErrorType type, int code, const char *msg);
	ErrorEventArgs (ErrorType type, int code, const char *msg, int extended_code, const char *extended_msg);

	int error_code;
	char *error_message;
	ErrorType error_type;
	// To match SL behaviour we need to match SL error messages, which aren't all that helpful
	// This is here to keep extra error information we have, 
	// but don't want to report to the user.
	// extended_code:
	//  3 (MEDIA_UNKNOWN_CODEC): used by playlist to determine if we should raise a MediaFailed event or just continue to play the next entry.
	int extended_code;
	char *extended_message;
};

/* @Namespace=None,ManagedDependencyProperties=None */
class ImageErrorEventArgs : public ErrorEventArgs {
protected:
	virtual ~ImageErrorEventArgs ();

public:
	ImageErrorEventArgs (const char *msg);
};

/* @Namespace=None,ManagedDependencyProperties=None */
class ParserErrorEventArgs : public ErrorEventArgs {
protected:
	virtual ~ParserErrorEventArgs ();

public:
	ParserErrorEventArgs (const char *msg, const char *file,
			      int line, int column, int error_code, 
			      const char *element, const char *attribute);
	
	int char_position;
	int line_number;
	char *xaml_file;
	char *xml_element;
	char *xml_attribute;
};

class MoonError {
public:
	enum ErrorType {
		NO_ERROR = 0,
		EXCEPTION = 1,
		ARGUMENT = 2,
		ARGUMENT_NULL = 3,
		ARGUMENT_OUT_OF_RANGE = 4,
		INVALID_OPERATION = 5,
		XAML_PARSE_EXCEPTION = 6,
		UNAUTHORIZED_ACCESS = 7,
		EXECUTION_ENGINE_EXCEPTION = 8,
		GCHANDLE_EXCEPTION = 9
	};

	// non-zero if an error occurred.
	ErrorType number;

	// the silverlight error code
	int code;

	// Used for xaml parsing exceptions
	int char_position;
	int line_number;

	// the caller of the method which returned the error must call Dispose to free this value
	// (only necessary if there were any errors)
	char *message;

	// managed code has thrown an exception, we store a gchandle
	// to the exception here.
	void* gchandle_ptr;
	
	MoonError ();
	~MoonError ();
	
	void Clear ();
	
	static void FillIn (MoonError *error, ErrorType type, int code, char *message /* this message must be allocated using glib methods */);
  	static void FillIn (MoonError *error, ErrorType type, int code, const char *message);

	static void FillIn (MoonError *error, ErrorType type, char *message /* this message must be allocated using glib methods */);
  	static void FillIn (MoonError *error, ErrorType type, const char *message);

	static void SetXamlPositionInfo (MoonError *error, int char_position, int line_number);
};

#endif /* __MOON_ERROR_H__ */
