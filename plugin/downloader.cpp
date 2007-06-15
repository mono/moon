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

static gpointer
p_downloader_create_state (Downloader *dl)
{
	DEBUGMSG ("downloader_create_state");
	return NULL;
}

static void
p_downloader_destroy_state (gpointer data)
{
	DEBUGMSG ("downloader_destroy_state");
}

static void
p_downloader_open (char *verb, char *uri, bool async, gpointer state)
{
	DEBUGMSG ("downloader_open");
}

static void
p_downloader_send (gpointer state)
{
	DEBUGMSG ("downloader_send");
}

static void
p_downloader_abort (gpointer state)
{
	DEBUGMSG ("downloader_abort");
}

static char*
p_downloader_get_response_text (char *part, gpointer state)
{
	DEBUGMSG ("downloader_get_response_text");
}

void
downloader_initialize ()
{
	downloader_set_functions (
			p_downloader_create_state,
			p_downloader_destroy_state,
			p_downloader_open,
			p_downloader_send,
			p_downloader_abort,
			p_downloader_get_response_text);
}

