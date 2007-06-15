/*
 * downloader.h: Moonlight plugin download routines.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

void downloader_initialize ();

gpointer downloader_create_state (Downloader* dl);
void downloader_destroy_state (gpointer data);
void downloader_open (char *verb, char *uri, bool async, gpointer state);
void downloader_send (gpointer state);
void downloader_abort (gpointer state);
void downloader_abort (gpointer state);
char* downloader_get_response_text (char *part, gpointer state);
