/*
 * error.cpp: ErrorEventArgs and its subclasses
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#include "error.h"

//
// ErrorEventArgs
//
ErrorEventArgs::ErrorEventArgs (ErrorType type, int code, const char *msg)
{
	SetObjectType(Type::ERROREVENTARGS);

	error_type = type;
	error_code = code;
	error_message = g_strdup (msg);
}

ErrorEventArgs::~ErrorEventArgs ()
{
	g_free (error_message);
}


//
// ImageErrorEventArgs
//

ImageErrorEventArgs::ImageErrorEventArgs (const char *msg)
  : ErrorEventArgs (ImageError, 4001, msg)
{
	SetObjectType(Type::IMAGEERROREVENTARGS);
}

ImageErrorEventArgs::~ImageErrorEventArgs ()
{
}


//
// ParserErrorEventArgs
//

ParserErrorEventArgs::ParserErrorEventArgs (const char *msg, const char *file,
					    int line, int column, int error_code, 
					    const char *element, const char *attribute)
  : ErrorEventArgs (ParserError, error_code, msg)
{
	SetObjectType(Type::PARSERERROREVENTARGS);
	xml_attribute = g_strdup (attribute);
	xml_element = g_strdup (element);
	xaml_file = g_strdup (file);
	char_position = column;
	line_number = line;
}

ParserErrorEventArgs::~ParserErrorEventArgs ()
{
	g_free (xaml_file);
	g_free (xml_element);
	g_free (xml_attribute);
}

//
// MoonError
//

MoonError::MoonError ()
  : number ((ErrorType)0), code (0), message (0), gchandle_ptr (NULL)
{
}

MoonError::~MoonError ()
{
	g_free (message);
	message = NULL;
}

void
MoonError::Clear ()
{
	number = NO_ERROR;
	code = 0;
	g_free (message);
	message = NULL;
}

void
MoonError::FillIn (MoonError *error, ErrorType number, int code, char *message)
{
	if (!error)
		return;

	error->number = number;
	error->code = code;
	error->message = message;
}
	
void
MoonError::FillIn (MoonError *error, ErrorType number, int code, const char *message)
{
	if (!error)
		return;

	FillIn (error, number, code, g_strdup (message));
}

void
MoonError::FillIn (MoonError *error, ErrorType type, char *message)
{
	if (!error)
		return;

	FillIn (error, type, 0, message);
}

void
MoonError::FillIn (MoonError *error, ErrorType type, const char *message)
{
	if (!error)
		return;

	FillIn (error, type, 0, message);
}
