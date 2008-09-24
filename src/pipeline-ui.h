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

class CodecDownloader : public EventObject {
private:
	static bool running; // If there already is another CodecDownloader running
	Surface *surface;
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
	
	GtkWindow *window;
	GtkVBox *vbox;
	GtkHBox *hbox;
	GtkLabel *label;
	GtkButton *accept_button;
	GtkButton *cancel_button;
	GtkProgressBar *progress_bar;
	GtkScrolledWindow *eula_scrollwindow;
	GtkTextBuffer *eula_buffer;
	GtkTextView *eula_view;
	GtkImage *logo;
	
	static gboolean DeleteEventHandler (GtkWidget *widget, GdkEvent *e, gpointer data);
	static gboolean AcceptClickedHandler (GtkButton *widget, CodecDownloader *cd);
	static gboolean CancelClickedHandler (GtkButton *widget, CodecDownloader *cd);
	gboolean AcceptClicked (GtkButton *widget);
	gboolean CancelClicked (GtkButton *widget);
	gboolean DeleteEvent (GtkWidget *widget, GdkEvent *e);

	static void DownloadProgressChangedHandler (EventObject *sender, EventArgs *args, gpointer closure);
	static void DownloadCompletedHandler (EventObject *sender, EventArgs *args, gpointer closure);
	static void DownloadFailedHandler (EventObject *sender, EventArgs *args, gpointer closure);

	void DownloadProgressChanged (EventObject *sender, EventArgs *args);
	void DownloadCompleted (EventObject *sender, EventArgs *args);
	void DownloadFailed (EventObject *sender, EventArgs *args);

	void Close (); // Closes the window and unrefs ourself
	void Show ();// Shows the codec installation ui and refs ourself
	
	CodecDownloader (Surface *surface);
	virtual ~CodecDownloader ();


public:
	static void ShowUI (Surface *surface); 
};

#endif /* __MOON_PIPELINE_UI_H__ */
