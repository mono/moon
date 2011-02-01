/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-ui.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_PIPELINE_UI_H__
#define __MOON_PIPELINE_UI_H__

#include <gtk/gtkwidget.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "downloader.h"
#include "runtime.h"
#include "moonlightconfiguration.h"

class MOON_API CodecDownloader : public EventObject {
private:
	static bool running; // If there already is another CodecDownloader running
	Surface *surface;
	bool is_user_initiated;
	// 0: initial, waiting for user input
	// 1: install clicked, downloading eula
	// 2: eula downloaded, waiting for user input
	// 3: accept clicked, downloading codec
	// 4: codecs downloaded
	// 5: user clicked don't install (or that value was read from the configuration)
	// 6: something went wrong (download failed for instance)
	int state;
	char *eula;
	Downloader *dl;
	MoonlightConfiguration configuration;
	
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *header_label;
	GtkWidget *message_label;
	GtkWidget *progress_bar;
	GtkWidget *eula_scrollwindow;
	GtkWidget *eula_view;
	GtkWidget *eula_evtbox;
	GtkWidget *accept_button;
	GtkWidget *cancel_button;
	GtkWidget *icon;
	GtkWidget *dont_ask;
	
	static void ResponseEventHandler (GtkDialog *dialog, gint response, gpointer data);
	void ResponseEvent (GtkDialog *dialog, GtkResponseType response);

	static void DownloadProgressChangedHandler (EventObject *sender, EventArgs *args, gpointer closure);
	static void DownloadCompletedHandler (EventObject *sender, EventArgs *args, gpointer closure);
	static void DownloadFailedHandler (EventObject *sender, EventArgs *args, gpointer closure);

	void DownloadProgressChanged (EventObject *sender, EventArgs *args);
	void DownloadCompleted (EventObject *sender, EventArgs *args);
	void DownloadFailed (EventObject *sender, EventArgs *args);

	void CreateDownloader ();
	void DestroyDownloader ();

	void ToggleEula (bool show);
	void ToggleProgress (bool show);
	void SetHeader (const gchar *message);
	void SetMessage (const gchar *message);
	void HideMessage ();
	void AdaptToParentWindow ();

	void AcceptClicked ();
	void Close (); // Closes the window and unrefs ourself
	void Show ();// Shows the codec installation ui and refs ourself
	
	bool VerifyDownload (const char *filename);

	CodecDownloader (Surface *surface, bool is_user_initiated);
	virtual ~CodecDownloader ();


public:
	static void ShowUI (Surface *surface, bool is_user_initiated); 
};

#endif /* __MOON_PIPELINE_UI_H__ */
