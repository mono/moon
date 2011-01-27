/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * clipboard.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include <glib.h>
#include <gtk/gtk.h>

struct ClipboardExecutorData;
struct WriteCustomFormatTextToClipboardData;
struct ReadCustomFormatTextFromClipboardData;
struct WriteImageToClipboardData;

class Clipboard {
private:
	static gunichar2 *data;
	static gint32 data_size; // number of characters in data
	static char *format;
	static GdkAtom format_atom;

	static void GetClipboardData (GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer user_data_or_owner);
	static void ClearClipboardData (GtkClipboard *clipboard, gpointer user_data_or_owner);

	static void InvokeOnMainThread (GSourceFunc func, ClipboardExecutorData *data);

	static gboolean ReadCustomFormatTextFromClipboardMT (ReadCustomFormatTextFromClipboardData *data);
	static gboolean WriteCustomFormatTextToClipboardMT (WriteCustomFormatTextToClipboardData *data);
	static gboolean WriteImageToClipboardMT (WriteImageToClipboardData *data);
	static gboolean ClearClipboardMT (ClipboardExecutorData *data);

	static gunichar2 *utf8_to_utf16 (const char *input, int length, int *out_len /* in number of gunichar2 */);
	static char *utf16_to_utf8 (const gunichar2 *input, int length, int *out_len /* in bytes */);

public:
	static int ReadCustomFormatTextFromClipboard (const char *customFormat, bool readAsUTF8Encoded, gunichar2 **textRead, gint32 *textLen);
	static int WriteCustomFormatTextToClipboard (const gunichar2 *customFormat, gunichar2 *textToWrite, gint32 textLen, bool makeUTF8Encoded);
	static int WriteImageToClipboard (const char *pathToImage);
	static int ClearClipboard ();
};

#endif /* __CLIPBOARD_H__ */

