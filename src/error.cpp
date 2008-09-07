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

MoonError::~MoonError ()
{
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
