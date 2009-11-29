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

#include <config.h>

 
#include "error.h"
#include "eventargs.h"

//
// MoonError
//

MoonError::MoonError ()
{
	number = (ExceptionType) 0;
	code = 0;
	message = 0;
	char_position = -1;
	line_number = -1;
	gchandle_ptr = NULL;
}

MoonError::MoonError (ExceptionType type, int code, const char *message)
{
	this->number = type;
	this->code = code;
	this->message = g_strdup (message);
	char_position = -1;
	line_number = -1;
	gchandle_ptr = NULL;
}

MoonError::MoonError (const MoonError &e)
{
	number = e.number;
	code = e.code;
	message = g_strdup (e.message);
	char_position = e.char_position;
	line_number = e.line_number;
	gchandle_ptr = e.gchandle_ptr;
}

MoonError::~MoonError ()
{
	Clear ();
}

void
MoonError::Clear ()
{
	number = NO_ERROR;
	code = 0;
	g_free (message);
	message = NULL;
}

MoonError&
MoonError::operator= (const MoonError& other)
{
	number = other.number;
	code = other.code;
	message = g_strdup (other.message);
	char_position = other.char_position;
	line_number = other.line_number;
	gchandle_ptr = other.gchandle_ptr;
	return *this;
}

void
MoonError::FillIn (MoonError *error, ExceptionType number, int code, const char *message)
{
	if (!error)
		return;

	error->number = number;
	error->code = code;
	error->message = g_strdup (message);
}
	
void
MoonError::FillIn (MoonError *error, ExceptionType type, const char *message)
{
	if (!error)
		return;

	FillIn (error, type, 0, message);
}


void
MoonError::FillIn (MoonError *error, ParserErrorEventArgs *error_args)
{
	if (!error)
		return;
	FillIn (error, MoonError::XAML_PARSE_EXCEPTION, error_args->GetErrorMessage());
	error->char_position = error_args->char_position;
	error->line_number = error_args->line_number;
}
