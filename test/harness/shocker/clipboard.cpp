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

#include <gtk/gtk.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "shocker-plugin.h"
#include "clipboard.h"

/*
 * ClipboardExecutorData
 */

struct ClipboardExecutorData {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int result;

	ClipboardExecutorData ()
	{
		pthread_mutexattr_t attribs;
		pthread_mutexattr_init (&attribs);
		pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init (&mutex, NULL);
		pthread_mutexattr_destroy (&attribs);
		pthread_cond_init (&cond, NULL);
		result = 0;
	}

	~ClipboardExecutorData ()
	{
		pthread_cond_destroy (&cond);
		pthread_mutex_destroy (&mutex);
	}
};

struct ReadCustomFormatTextFromClipboardData : public ClipboardExecutorData {
	const char *customFormat;
	bool readAsUTF8Encoded;
	gunichar2 **textRead;
	gint32 *textLen;
	GtkSelectionData *selection_data;
};

struct WriteCustomFormatTextToClipboardData : public ClipboardExecutorData {
	char *customFormat;
	gunichar2 *textToWrite;
	gint32 textLen;
	bool makeUTF8Encoded;
};

struct WriteImageToClipboardData : public ClipboardExecutorData {
	const char *pathToImage;
};

/*
 * Clipboard
 */

gunichar2 *Clipboard::data = NULL;
gint32 Clipboard::data_size = 0;
char *Clipboard::format = NULL;
GdkAtom Clipboard::format_atom;

gunichar2 *
Clipboard::utf8_to_utf16 (const char *input, int length, int *out_len)
{
	const char *cur_char;
	gunichar2 *result;

	if (input == NULL)
		return NULL;

	/* we can have embedded null chars. g_utf8_to_utf16 can't cope with those, so we need to convert manually from utf8 to utf16 */
	result = (gunichar2 *) g_malloc0 ((length + 1) * 6);
	cur_char = input;

	*out_len = 0;
	while (cur_char < input + length) {
		if (*cur_char == 0) {
			result [*out_len] = 0;
			cur_char++;
			(*out_len)++;
		} else {
			result [*out_len] = g_utf8_get_char (cur_char);
			cur_char = g_utf8_next_char (cur_char);
			(*out_len)++;
		}
	}

	return result;
}

char *
Clipboard::utf16_to_utf8 (const gunichar2 *input, int length, int *out_len)
{
	char *result;
	char *out;

	if (input == NULL)
		return NULL;

	if (length == -1) {
		length = 0;
		while (input [length] != 0)
			length++;
	}

	/* we can have embedded null chars. g_utf16_to_utf8 can't cope with those, so we need to convert manually */
	result = (char *) g_malloc0 ((length + 1) * 2);

	out = result;
	for (int i = 0; i < length; i++) {
		if (input [i] == 0) {
			*out = 0;
			out++;
		} else {
			out += g_unichar_to_utf8 (input [i], out);
		}
	}

	if (out_len)
		*out_len = out - result;

	return result;
}

void
Clipboard::GetClipboardData (GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer user_data_or_owner)
{
	int utf8_len;
	char *data_utf8 = utf16_to_utf8 (data, data_size, &utf8_len);

	LOG_CLIPBOARD ("Clipboard::GetClipboardData (%i, %p) format: %s data: %*s, size: %i target: %p = %s\n", info, user_data_or_owner, format, utf8_len, data_utf8, data_size, selection_data->target, gdk_atom_name (selection_data->target));

	if (selection_data->target == format_atom) {
		/* we have an exact match */
		gtk_selection_data_set (selection_data, format_atom, 0, (guchar *) data, data_size * 2);
	} else if (format == NULL) {
		/* text */
		gtk_selection_data_set_text (selection_data, data_utf8, utf8_len);
	} else {
		gtk_selection_data_set (selection_data, format_atom, 0, (guchar *) data, data_size * 2);
	}

	g_free (data_utf8);
}

void
Clipboard::ClearClipboardData (GtkClipboard *clipboard, gpointer user_data_or_owner)
{
	LOG_CLIPBOARD ("Clipboard::ClearClipboardData (%p)\n", user_data_or_owner);

	g_free (data);
	data = NULL;
	data_size = 0;
	g_free (format);
	format = NULL;
}

void
Clipboard::InvokeOnMainThread (GSourceFunc func, ClipboardExecutorData *data)
{
	LOG_CLIPBOARD ("Clipboard (%p, %p) Marshalling to main thread...\n", func, data);
	pthread_mutex_lock (&data->mutex);
	g_timeout_add (1, (GSourceFunc) func, data);
	pthread_cond_wait (&data->cond, &data->mutex);
	pthread_mutex_unlock (&data->mutex);
	LOG_CLIPBOARD ("Clipboard (%p, %p) Marshalling to main thread [Done]\n", func, data);
}

int
Clipboard::ClearClipboard ()
{
	ClipboardExecutorData data;

	ClearClipboardMT (&data);

	return data.result;
}

gboolean
Clipboard::ClearClipboardMT (ClipboardExecutorData *data)
{
	LOG_CLIPBOARD ("Clipboard::ClearClipboardMT (%p)\n", data);

	if (!PluginObject::InMainThread ()) {
		InvokeOnMainThread ((GSourceFunc) ClearClipboardMT, data);
		return false;
	}

	gtk_clipboard_clear (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));

	LOG_CLIPBOARD ("Clipboard::ClearClipboardMT (%p) [Done] result: %i\n", data, data->result);

	pthread_mutex_lock (&data->mutex);
	pthread_cond_signal (&data->cond);
	pthread_mutex_unlock (&data->mutex);

	return false;
}

int
Clipboard::WriteCustomFormatTextToClipboard (const gunichar2 *customFormat, gunichar2 *textToWrite, gint32 textLen, bool makeUTF8Encoded)
{
	WriteCustomFormatTextToClipboardData data;

	data.customFormat = utf16_to_utf8 (customFormat, -1, NULL);
	data.textToWrite = textToWrite;
	data.textLen = textLen;
	data.makeUTF8Encoded = makeUTF8Encoded;

	WriteCustomFormatTextToClipboardMT (&data);

	g_free (data.customFormat);

	return data.result;
}

gboolean
Clipboard::WriteCustomFormatTextToClipboardMT (WriteCustomFormatTextToClipboardData *data)
{
	int utf8_len;
	char *textToWriteUTF8 = utf16_to_utf8 (data->textToWrite, data->textLen, &utf8_len);

	LOG_CLIPBOARD ("Clipboard::WriteCustomFormatTextToClipboardMT (%s, %*s, %i, %i).\n", data->customFormat, utf8_len, textToWriteUTF8, data->textLen, data->makeUTF8Encoded);

	if (!PluginObject::InMainThread ()) {
		g_free (textToWriteUTF8);
		InvokeOnMainThread ((GSourceFunc) WriteCustomFormatTextToClipboardMT, data);
		return data->result;
	}

	GtkTargetList *target_list;
	GtkTargetEntry *targets;
	gint n_targets;

	format_atom = gdk_atom_intern (data->customFormat == NULL ? "MOONLIGHT_NULL_TEXT_FORMAT" : data->customFormat, false);

	target_list = gtk_target_list_new (NULL, 0);
	if (data->customFormat == NULL) {
		gtk_target_list_add_text_targets (target_list, 0);
	}
	gtk_target_list_add (target_list, format_atom, 0, 0);
	targets = gtk_target_table_new_from_list (target_list, &n_targets);

	data->result = gtk_clipboard_set_with_data (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), targets, n_targets, GetClipboardData, ClearClipboardData, NULL);

	gtk_target_table_free (targets, n_targets);
	gtk_target_list_unref (target_list);

	Clipboard::data_size = data->textLen;
	Clipboard::data = (gunichar2 *) g_memdup (data->textToWrite, data_size * 2);
	Clipboard::format = g_strdup (data->customFormat);

	LOG_CLIPBOARD ("Clipboard::WriteCustomFormatTextToClipboardMT (%s, %*s, %i, %i) write result: %i format_atom: %p\n",
		data->customFormat, utf8_len, textToWriteUTF8, data->textLen, data->makeUTF8Encoded, data->result, format_atom);

	pthread_mutex_lock (&data->mutex);
	pthread_cond_signal (&data->cond);
	pthread_mutex_unlock (&data->mutex);

	g_free (textToWriteUTF8);

	return false;
}

int
Clipboard::ReadCustomFormatTextFromClipboard (const char *customFormat, bool readAsUTF8Encoded, gunichar2 **textRead, gint32 *textLen)
{
	ReadCustomFormatTextFromClipboardData data;

	LOG_CLIPBOARD ("Clipboard::ReadCustomFormatTextFromClipboard (customFormat: %s, readAsUTF8Encoded: %i, textRead: %p, textLen: %p).\n", customFormat, readAsUTF8Encoded, textRead, textLen);

	data.customFormat = customFormat;
	data.readAsUTF8Encoded = readAsUTF8Encoded;
	data.textRead = textRead;
	data.textLen = textLen;
	ReadCustomFormatTextFromClipboardMT (&data);

	LOG_CLIPBOARD ("Clipboard::ReadCustomFormatTextFromClipboard (customFormat: %s, readAsUTF8Encoded: %i, textRead: %p, textLen: %p) got: %p textLen: %i textRead: %s\n", customFormat, readAsUTF8Encoded, textRead, textLen, data.selection_data, *data.textLen, *data.textRead == NULL ? NULL : g_utf16_to_utf8 ((gunichar2 *) *data.textRead, *data.textLen, NULL, NULL, NULL));

	return data.result;
}

gboolean
Clipboard::ReadCustomFormatTextFromClipboardMT (ReadCustomFormatTextFromClipboardData *data)
{
	GdkAtom atom;

	LOG_CLIPBOARD ("Clipboard::ReadCustomFormatTextFromClipboardMT (%p)\n", data);

	if (!PluginObject::InMainThread ()) {
		InvokeOnMainThread ((GSourceFunc) ReadCustomFormatTextFromClipboardMT, data);
		return false;
	}

	atom = gdk_atom_intern (data->customFormat == NULL ? "MOONLIGHT_NULL_TEXT_FORMAT" : data->customFormat, false);
	data->selection_data = gtk_clipboard_wait_for_contents (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), atom);
	if (data->selection_data) {
		*data->textLen = data->selection_data->length / 2;
		*data->textRead = (gunichar2 *) data->selection_data->data;
	} else if (data->customFormat == NULL) {
		char *utf8 = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
		if (utf8 != NULL) {
			*data->textRead = utf8_to_utf16 (utf8, strlen (utf8), data->textLen);
		}
	}

	LOG_CLIPBOARD ("Clipboard::ReadCustomFormatTextFromClipboardMT (%p) [Done] read: %s\n", data, *data->textRead == NULL ? NULL : g_utf16_to_utf8 ((gunichar2 *) *data->textRead, *data->textLen, NULL, NULL, NULL));

	pthread_mutex_lock (&data->mutex);
	pthread_cond_signal (&data->cond);
	pthread_mutex_unlock (&data->mutex);

	return false;
}

int
Clipboard::WriteImageToClipboard (const char *pathToImage)
{
	WriteImageToClipboardData data;

	data.pathToImage = pathToImage;
	WriteImageToClipboardMT (&data);

	return data.result;
}

gboolean
Clipboard::WriteImageToClipboardMT (WriteImageToClipboardData *data)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf;

	LOG_CLIPBOARD ("Clipboard::WriteImageToClipboardMT (%s).\n", data->pathToImage);

	if (!PluginObject::InMainThread ()) {
		InvokeOnMainThread ((GSourceFunc) WriteImageToClipboardMT, data);
		return false;
	}

	pixbuf = gdk_pixbuf_new_from_file (data->pathToImage, &error);

	if (pixbuf == NULL) {
		printf ("Clipboard::WriteImageToClipboardMT (%s): Failed to create pixbuf: %s\n", data->pathToImage, error->message);
		Shocker_FailTestFast ("Clipboard::WriteImageToClipboard (): Failed to create pixbuf.");
		data->result = 1;
	} else {
		gtk_clipboard_set_image (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), pixbuf);
		g_object_unref (pixbuf);
	}

	LOG_CLIPBOARD ("Clipboard::WriteImageToClipboardMT (%s): Completed successfully\n", data->pathToImage);

	pthread_mutex_lock (&data->mutex);
	pthread_cond_signal (&data->cond);
	pthread_mutex_unlock (&data->mutex);

	return false;
}
