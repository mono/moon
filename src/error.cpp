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
 
void
MoonError::FillIn (MoonError *error, ErrorType number, char *message)
{
	if (error) {
		error->number = number;
		error->message = message;
	}
}
	
void
MoonError::FillIn (MoonError *error, ErrorType number, const char *message)
{
	FillIn (error, number, g_strdup (message));
}

void
MoonError::Dispose ()
{
	g_free (message);
	message = NULL;
}
