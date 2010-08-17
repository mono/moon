/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * im-gtk.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
#include "im-gtk.h"

#include <gtk/gtkimmulticontext.h>

using namespace Moonlight;

MoonIMContextGtk::MoonIMContextGtk ()
{
	im = gtk_im_multicontext_new ();
	im_widget = NULL;
}

MoonIMContextGtk::~MoonIMContextGtk ()
{
	g_object_unref (im);
	im = NULL;
	if (im_widget != NULL)
		g_signal_handlers_disconnect_matched (im_widget, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
}

void
MoonIMContextGtk::SetUsePreedit (bool flag)
{
	gtk_im_context_set_use_preedit (im, flag);
}

void
MoonIMContextGtk::ImWindowDestroyed (GtkWindow *window, gpointer data)
{
	((MoonIMContextGtk *) data)->SetClientWindow (NULL);
}

void
MoonIMContextGtk::SetClientWindow (MoonWindow* window)
{
	GdkWindow *gdk_window = NULL;
	GdkWindow *w;
	gpointer user_data = NULL;
	GtkWidget *im_w = NULL;

	/*
	 * We need to clear the client window when the GtkWindow the im context uses is destroyed, otherwise
	 * we'll get random crashes upon shutdown when the im code tries to access a widget which has already
	 * been destroyed. So we listen to the destroyed signal of the gtk window and clear the client window
	 * when signalled.
	 */
	if (window != NULL) {
		gdk_window = GDK_WINDOW (window->GetPlatformWindow());
		/* Find the parent gtk widget of the gdk window. */
		w = gdk_window;
		while (w != NULL) {
			gdk_window_get_user_data (w, &user_data);
			if (user_data) {
				im_w = (GtkWidget *) user_data;
				break;
			}
			w = gdk_window_get_parent (w);
		}
	}
	if (im_w != this->im_widget) {
		if (this->im_widget != NULL)
			g_signal_handlers_disconnect_matched (this->im_widget, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);

		if (im_w != NULL)
			g_signal_connect (im_w, "destroy", G_CALLBACK (ImWindowDestroyed), this);
	}
	this->im_widget = im_w;

	gtk_im_context_set_client_window (im, window ? GDK_WINDOW(window->GetPlatformWindow ()) : NULL);
}

bool
MoonIMContextGtk::FilterKeyPress (MoonKeyEvent* event)
{
	return gtk_im_context_filter_keypress (im, (GdkEventKey*)event->GetPlatformEvent());
}

void
MoonIMContextGtk::SetSurroundingText (const char *text, int offset, int length)
{
	gtk_im_context_set_surrounding (im, text, offset, length);
}

void
MoonIMContextGtk::Reset ()
{
	gtk_im_context_reset (im);
}


void
MoonIMContextGtk::FocusIn ()
{
	gtk_im_context_focus_in (im);
}

void
MoonIMContextGtk::FocusOut ()
{
	gtk_im_context_focus_out (im);
}

void
MoonIMContextGtk::SetCursorLocation (Rect r)
{
	GdkRectangle area;
	Rect rect;

	area = r.ToGdkRectangle ();

	gtk_im_context_set_cursor_location (im, &area);
}

void
MoonIMContextGtk::SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data)
{
	g_signal_connect (im, "retrieve-surrounding", G_CALLBACK (cb), data);
}

void
MoonIMContextGtk::SetDeleteSurroundingCallback (MoonCallback cb, gpointer data)
{
	g_signal_connect (im, "delete-surrounding", G_CALLBACK (cb), data);
}

void
MoonIMContextGtk::SetCommitCallback (MoonCallback cb, gpointer data)
{
	g_signal_connect (im, "commit", G_CALLBACK (cb), data);
}

gpointer
MoonIMContextGtk::GetPlatformIMContext ()
{
	return im;
}

