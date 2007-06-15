/*
 * downloader.cpp: Moonlight plugin download routines.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moonlight.h"
#include "runtime.h"
#include "downloader.h"

void
downloader_initialize ()
{
	downloader_set_functions (
			downloader_create_state,
			downloader_destroy_state,
			downloader_open,
			downloader_send,
			downloader_abort,
			downloader_get_response_text);
}

gpointer
downloader_create_state (Downloader *dl)
{
	DEBUGMSG ("downloader_create_state");
	return NULL;
}

void
downloader_destroy_state (gpointer data)
{
	DEBUGMSG ("downloader_destroy_state");
}

void
downloader_open (char *verb, char *uri, bool async, gpointer state)
{
	DEBUGMSG ("downloader_open");
}

void
downloader_send (gpointer state)
{
	DEBUGMSG ("downloader_send");
}

void
downloader_abort (gpointer state)
{
	DEBUGMSG ("downloader_abort");
}

char*
downloader_get_response_text (char *part, gpointer state)
{
	DEBUGMSG ("downloader_get_response_text");
}
