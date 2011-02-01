/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pipeline-nocodec-ui.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_PIPELINE_NOCODEC_UI_H__
#define __MOON_PIPELINE_NOCODEC_UI_H__

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
	int state;
	MoonlightConfiguration configuration;
	
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *header_label;
	GtkWidget *message_label;
	GtkWidget *ok_button;
	GtkWidget *icon;
	GtkWidget *dont_ask;
	
	static void ResponseEventHandler (GtkDialog *dialog, gint response, gpointer data);
	void ResponseEvent (GtkDialog *dialog, GtkResponseType response);

	void SetHeader (const gchar *message);
	void SetMessage (const gchar *message);
	void HideMessage ();
	void AdaptToParentWindow ();

	void Close (); // Closes the window and unrefs ourself
	void Show ();// Shows the codec installation ui and refs ourself
	
	CodecDownloader (Surface *surface);
	virtual ~CodecDownloader ();


public:
	static void ShowUI (Surface *surface); 
};

#endif /* __MOON_PIPELINE_UI_H__ */
