/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-nocodec-ui.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pipeline-nocodec-ui.h"
#include "downloader.h"
#include "utils.h"
#include "pipeline.h"
#include "debug.h"

bool CodecDownloader::running = false;

CodecDownloader::CodecDownloader (Surface *surf)
{
	surface = surf;
	state = 0;
	dialog = NULL;
	vbox = NULL;
	header_label = NULL;
	message_label = NULL;
	ok_button = NULL;
	icon = NULL;
	dont_ask = NULL;
}

CodecDownloader::~CodecDownloader ()
{
	running = false;
}

void
CodecDownloader::ShowUI (Surface *surface)
{
	if (running) {
		return;
	}

	if (!(moonlight_flags & RUNTIME_INIT_ENABLE_MS_CODECS))
		return;

	CodecDownloader *cd = new CodecDownloader (surface);
	cd->Show ();
	cd->unref ();
}

// ----- Event Proxies -----

void
CodecDownloader::ResponseEventHandler (GtkDialog *dialog, gint response, gpointer data)
{
	((CodecDownloader *) data)->ResponseEvent (dialog, (GtkResponseType)response);
}

// ----- Event Handlers -----

void
CodecDownloader::ResponseEvent (GtkDialog *dialog, GtkResponseType response)
{
	LOG_UI ("CodecDownloader::ResponseEvent (%d)\n", response);
	SetCurrentDeployment ();

	switch (response) {
	case GTK_RESPONSE_DELETE_EVENT:
		Close ();
		return;
	case GTK_RESPONSE_OK:
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dont_ask))) {
			LOG_UI ("Setting DontWarnUnsupportedCodecs\n");
			configuration.SetBooleanValue ("Codecs", "DontWarnUnsupportedCodecs", true);
			configuration.Save ();
		}
		Close ();
		return;
	default:
		return;
	}
}

void
CodecDownloader::SetHeader (const gchar *message)
{
	gchar *message_full = g_strdup_printf ("<big><b>%s</b></big>", message);
	gtk_label_set_markup (GTK_LABEL (header_label), message_full);
	g_free (message_full);
}

void
CodecDownloader::SetMessage (const gchar *message)
{
	gtk_label_set_text (GTK_LABEL (message_label), message);
	gtk_widget_show (message_label);
}

void
CodecDownloader::HideMessage ()
{
	gtk_widget_hide (message_label);
}

void
CodecDownloader::AdaptToParentWindow ()
{
	// try to find a parent for our window
	// there must be a better way of doing this though :|
	GList *toplevels = gtk_window_list_toplevels ();
	GList *current = toplevels;
	GtkWindow *parent = NULL;

	while (current != NULL) {
		const char *title = gtk_window_get_title (GTK_WINDOW (current->data));
		if (title != NULL && strstr (title, "Mozilla Firefox") != NULL) {
			parent = GTK_WINDOW (current->data);
			break;
		}

		current = current->next;
	}
	g_list_free (toplevels);

	if (parent != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	} else {
		// If no parent could be found, just center in the screen
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	}
}

// ----- Dialog Create/Destroy -----

void
CodecDownloader::Show ()
{
	if (configuration.GetBooleanValue ("Codecs", "DontWarnUnsupportedCodecs")) {
		state = 1;
		return;
	}

	if (state != 0) {
		fprintf (stderr, "CodecDownloader::Show (): Can't call Show more than once.\n");
		state = 2;
		return;
	}
	
	gint label_width = 400;

	// Build HIG Dialog Box
	dialog = gtk_dialog_new_with_buttons ("Moonlight Codecs Installer", NULL, (GtkDialogFlags)
		(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR), NULL);
	ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog), "_Ok", GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	
	AdaptToParentWindow ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_object_set (GTK_OBJECT (dialog), "resizable", false, NULL);

	// HIG HBox
	GtkWidget *hbox = gtk_hbox_new (false, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, true, true, 0);

	// Message box icon
	icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
	gtk_misc_set_alignment (GTK_MISC (icon), 0.5f, 0.0f);
	gtk_box_pack_start (GTK_BOX (hbox), icon, false, false, 0);

	// Contents container
	vbox = gtk_vbox_new (false, 0);
	gtk_box_set_spacing (GTK_BOX (vbox), 10);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, true, true, 0);

	// Header Label
	header_label = gtk_label_new (NULL);
	SetHeader ("This page requires the Microsoft Media Pack to be installed to play multimedia content.");
	gtk_label_set_line_wrap (GTK_LABEL (header_label), true);
	gtk_label_set_justify (GTK_LABEL (header_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (header_label), 0.0f, 0.5f);
	gtk_widget_set_size_request (header_label, label_width, -1);
	gtk_box_pack_start (GTK_BOX (vbox), header_label, false, false, 0);

	// Secondary Label
	message_label = gtk_label_new (NULL);
	SetMessage ("The Microsoft Media Pack is currently unavailable for your "
                    "Operating System or Architecture.");
	gtk_label_set_line_wrap (GTK_LABEL (message_label), true);
	gtk_label_set_justify (GTK_LABEL (message_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (message_label), 0.0f, 0.5f);
	gtk_widget_set_size_request (message_label, label_width, -1);
	gtk_box_pack_start (GTK_BOX (vbox), message_label, false, false, 0);

	dont_ask = gtk_check_button_new_with_label ("Do not show me this message again");
	gtk_box_pack_start (GTK_BOX (vbox), dont_ask, false, false, 0);

	// Connect and go
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (ResponseEventHandler), this);

	gtk_object_set (GTK_OBJECT (ok_button), "has-focus", true, "has-default", true, NULL);

	gtk_widget_show_all (dialog);

	ref (); // We manage our lifetime ourself
	running = true;
}

void
CodecDownloader::Close ()
{
	LOG_UI ("CodecDownloader::Close ()\n");

	gtk_widget_destroy (dialog);
	unref ();
	running = false;
}

