/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-ui.cpp: 
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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pipeline-ui.h"
#include "downloader.h"
#include "utils.h"
#include "pipeline.h"

#define EULA_URL "http://anonsvn.mono-project.com/viewvc/trunk/moon/LICENSE?revision=112447"
#define CODEC_URL "http://localhost:8080/libmscodecs.so"

#define LOG_UI(...)// printf (__VA_ARGS__);

bool CodecDownloader::running = false;

CodecDownloader::CodecDownloader (Surface *surf)
{
	surface = surf;
	eula = NULL;
	state = 0;
	dl = NULL;
	window = NULL;
	vbox = NULL;
	hbox = NULL;
	label = NULL;
	accept_button = NULL;
	cancel_button = NULL;
	progress_bar = NULL;
	eula_scrollwindow = NULL;
	eula_buffer = NULL;
	eula_view = NULL;
	logo = NULL;
}

CodecDownloader::~CodecDownloader ()
{
	g_free (eula);
	if (dl)
		dl->unref ();
	running = false;
}

void
CodecDownloader::ShowUI (Surface *surface)
{
	CodecDownloader *cd;
	
	if (running)
		return;
		
	cd = new CodecDownloader (surface);
	cd->Show ();
	cd->unref ();
}

void
CodecDownloader::Close ()
{
	gtk_widget_destroy (GTK_WIDGET (window));
	unref ();
	running = false;
}

gboolean
CodecDownloader::DeleteEventHandler (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	return ((CodecDownloader *) data)->DeleteEvent (widget, e);
}

gboolean
CodecDownloader::CancelClickedHandler (GtkButton *widget, CodecDownloader *cd)
{
	return cd->CancelClicked (widget);
}

gboolean
CodecDownloader::AcceptClickedHandler (GtkButton *widget, CodecDownloader *cd)
{
	return cd->AcceptClicked (widget);
}

void
CodecDownloader::DownloadFailedHandler (EventObject *sender, EventArgs *args, gpointer closure)
{
	((CodecDownloader *) closure)->DownloadFailed (sender, args);
}

void
CodecDownloader::DownloadCompletedHandler (EventObject *sender, EventArgs *args, gpointer closure)
{
	((CodecDownloader *) closure)->DownloadCompleted (sender, args);
}

void
CodecDownloader::DownloadProgressChangedHandler (EventObject *sender, EventArgs *args, gpointer closure)
{
	((CodecDownloader *) closure)->DownloadProgressChanged (sender, args);
}

gboolean
CodecDownloader::DeleteEvent (GtkWidget *widget, GdkEvent *e)
{
	LOG_UI ("CodecDownloader::DeleteEvent ()\n");
	
	Close ();
	
	return TRUE;
}

gboolean
CodecDownloader::CancelClicked (GtkButton *widget)
{
	LOG_UI ("CodecDownloader::CancelClicked ()\n");
	
	configuration.SetBooleanValue ("Codecs", "DontInstallMSCodecs", true);
	configuration.Save ();
	state = 5;
	Close ();
	
	return TRUE;
}

gboolean
CodecDownloader::AcceptClicked (GtkButton *widget)
{
	LOG_UI ("CodecDownloader::AcceptClicked\n");
	
	gtk_progress_bar_set_fraction (progress_bar, 0.0);
	gtk_widget_show_all (GTK_WIDGET (progress_bar));

	if (!dl) {
		dl = surface->CreateDownloader ();
		dl->AddHandler (Downloader::DownloadProgressChangedEvent, DownloadProgressChangedHandler, this);
		dl->AddHandler (Downloader::DownloadFailedEvent, DownloadFailedHandler, this);
		dl->AddHandler (Downloader::CompletedEvent, DownloadCompletedHandler, this);
	}
	
	switch (state) {
	case 0: // initial, waiting for user input
		gtk_label_set_text (label, "Downloading eula...");
		gtk_widget_set_sensitive (GTK_WIDGET (accept_button), false);
		
		dl->Open ("GET", EULA_URL, NoPolicy);
		dl->Send ();

		state = 1;
		break;
	case 2: // eula downloaded, waiting for user input
		gtk_label_set_text (label, "Downloading codecs...");
		gtk_widget_hide (GTK_WIDGET (eula_scrollwindow));
		gtk_widget_set_sensitive (GTK_WIDGET (accept_button), false);

		dl->Open ("GET", CODEC_URL, NoPolicy);
		dl->Send ();

		state = 3;
		break;
	case 4:
	case 6:
		Close ();
		break;
	default:
		printf ("CodecDownloader::AcceptClicked (): Invalid state: %i\n", state);
		break;
	}
	
	return TRUE;
}


void
CodecDownloader::DownloadFailed (EventObject *sender, EventArgs *args)
{
	ErrorEventArgs *eea = (ErrorEventArgs *) args;
	gchar *msg;
	
	LOG_UI ("CodecDownloader::DownloadFailed ()\n");

	msg = g_strdup_printf ("There was an error while downloading the %s: %s.", state == 1 ? "eula" : "codec", eea->error_message);
	
	gtk_label_set_text (label, msg);
	gtk_button_set_label (accept_button, "_Close");
	gtk_widget_set_sensitive (GTK_WIDGET (accept_button), true);
	gtk_widget_hide (GTK_WIDGET (cancel_button));
	gtk_widget_hide (GTK_WIDGET (progress_bar));
	
	g_free (msg);
	
	state = 6;
}

void
CodecDownloader::DownloadCompleted (EventObject *sender, EventArgs *args)
{
	guint64 size;
	gchar *msg = NULL;
	gchar *downloaded_file = NULL;
	gchar *codec_path = NULL;
	gchar *codec_dir = NULL;
	int codec_fd = 0;
	
	LOG_UI ("CodecDownloader::DownloadCompleted ()\n");
	
	gtk_widget_hide (GTK_WIDGET (progress_bar));

	switch (state) {
	case 1: // downloading eula, we're now finished downloading the eula
		eula = dl->GetResponseText (NULL, &size);
		gtk_label_set_text (label, "You need to accept the EULA before installing the codecs");
		gtk_button_set_label (accept_button, "_Accept");
		gtk_text_buffer_set_text (eula_buffer, eula, strlen (eula));
		gtk_widget_show_all (GTK_WIDGET (eula_scrollwindow));
		gtk_widget_set_sensitive (GTK_WIDGET (accept_button), true);

		state = 2;
		break;
	case 3: // downloading codec, we're now finished downloading the codec

		// TODO: Save the codec into some other directory
		codec_path = g_build_filename (g_get_user_config_dir (), "moonlight", "libmscodecs.so", NULL);
		codec_dir = g_path_get_dirname (codec_path);

		downloaded_file = dl->GetDownloadedFilename (NULL);

		errno = 0;
		if (g_mkdir_with_parents (codec_dir, 0700) == -1) {
			msg = g_strdup_printf ("Error while installing codecs: %s.\n", strerror (errno));
		} else if ((codec_fd = open (codec_path, O_CREAT | O_TRUNC | O_WRONLY, 0700)) == -1) {
			msg = g_strdup_printf ("Error while installing codecs: %s.\n", strerror (errno));
		} else if (CopyFileTo (downloaded_file, codec_fd) == -1) {
			msg = g_strdup_printf ("Error while installing codecs: %s.\n", strerror (errno));
		} else {
			msg = g_strdup ("Codecs successfully downloaded and installed.\nYou have to refresh the web page for the change to take effect.");
			configuration.SetStringValue ("Codecs", "MSCodecsPath", codec_path);
			configuration.Save ();
			Media::RegisterMSCodecs ();
		}
		
		g_free (codec_path);
		g_free (codec_dir);
		g_free (downloaded_file);
				
		gtk_label_set_text (label, msg);
		gtk_widget_hide (GTK_WIDGET (cancel_button));
		gtk_button_set_label (accept_button, "_Close");
		gtk_widget_set_sensitive (GTK_WIDGET (accept_button), true);

		g_free (msg);

		state = 4;
		break;
	default:
		printf ("CodecDownloader::DownloadCompleted (): Invalid state: %i\n", state);
		break;
	}
}

void
CodecDownloader::DownloadProgressChanged (EventObject *sender, EventArgs *args)
{
	double progress = dl->GetDownloadProgress ();
	
	LOG_UI ("CodecDownloader::DownloadProgressChanged (): %.2f\n", progress);
	
	gtk_progress_bar_set_fraction (progress_bar, progress);
}

void
CodecDownloader::Show ()
{
	extern const char moonlight_logo [];
	extern int moonlight_logo_size;
	const int pad = 5;
	
	if (configuration.GetBooleanValue ("Codecs", "DontInstallMSCodecs")) {
		state = 5;
		return;
	}

	if (state != 0) {
		fprintf (stderr, "CodecDownloader::Show (): Can't call Show more than once.\n");
		state = 6;
		return;
	}

	// Create ui widgets
	window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	vbox = GTK_VBOX (gtk_vbox_new (FALSE, 0));
	hbox = GTK_HBOX (gtk_hbox_new (FALSE, 5));
	label = GTK_LABEL (gtk_label_new (NULL));
	accept_button = GTK_BUTTON (gtk_button_new_with_mnemonic ("_Install"));
	cancel_button = GTK_BUTTON (gtk_button_new_with_mnemonic ("_Don't install"));
	progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
	eula_scrollwindow = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
	eula_buffer = GTK_TEXT_BUFFER (gtk_text_buffer_new (NULL));
	eula_view = GTK_TEXT_VIEW (gtk_text_view_new_with_buffer (eula_buffer));

	// Try to load the logo
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
	if (gdk_pixbuf_loader_write (loader, (const guchar *) moonlight_logo, moonlight_logo_size, NULL) == TRUE) {
		GdkPixbuf *buf = gdk_pixbuf_loader_get_pixbuf (loader);
		if (buf != NULL)
			logo = GTK_IMAGE (gtk_image_new_from_pixbuf (buf));
	}
	gdk_pixbuf_loader_close (loader, NULL);
	g_object_unref (loader);

	// try to find a parent for our window
	// there must be a better way of doing this though :|
	GList *toplevels = gtk_window_list_toplevels ();
	GtkWindow *parent = NULL;
	while (toplevels != NULL) {
		if (strstr (gtk_window_get_title (GTK_WINDOW (toplevels->data)), "Mozilla Firefox") != NULL) {
			parent = GTK_WINDOW (toplevels->data);
			break;
		}
		toplevels = toplevels->next;
	}
	if (parent != NULL) {
		gtk_window_set_transient_for (window, parent);
		gtk_window_set_position (window, GTK_WIN_POS_CENTER_ON_PARENT);
	} else {
		// If no parent could be found, just center in the screen
		gtk_window_set_position (window, GTK_WIN_POS_CENTER);
	}

	// set properties
	gtk_widget_set_app_paintable (GTK_WIDGET (window), TRUE);
	gtk_window_set_default_size (window, 400, 300);
	gtk_window_set_title (window, "Moonlight Codecs Installer");

	gtk_label_set_text (label, "You do not have the codecs required to properly decode the media on this page, do you want to install the required codecs?");
	gtk_label_set_line_wrap (label, TRUE);
	gtk_misc_set_padding (GTK_MISC (label), pad, pad);

	if (logo)
		gtk_misc_set_padding (GTK_MISC (logo), pad, pad);
	
	gtk_text_view_set_editable (eula_view, FALSE);

	// create hierarchy	
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));

	gtk_container_add (GTK_CONTAINER (eula_scrollwindow), GTK_WIDGET (eula_view));

	if (logo)
		gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (logo), FALSE, TRUE, pad);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, TRUE, pad);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (eula_scrollwindow), TRUE, TRUE, pad);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (progress_bar), FALSE, TRUE, pad);
	gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET (hbox), FALSE, TRUE, pad);

	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (cancel_button), TRUE, TRUE, pad);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (accept_button), TRUE, TRUE, pad);

	gtk_signal_connect (GTK_OBJECT (window), "delete-event", G_CALLBACK (DeleteEventHandler), this);
	gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked", G_CALLBACK (CancelClickedHandler), this);
	gtk_signal_connect (GTK_OBJECT (accept_button), "clicked", G_CALLBACK (AcceptClickedHandler), this);
	
	gtk_widget_show_all (GTK_WIDGET (window));

	gtk_widget_hide (GTK_WIDGET (progress_bar));
	gtk_widget_hide (GTK_WIDGET (eula_scrollwindow));

	ref (); // We manage our lifetime ourself
	running = true;
}
