/*
 * messagebox.cpp: MessageBox dialog
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "messagebox.h"

int
message_box_show (const char *caption, const char* text, int buttons)
{
	if (!caption || !text)
		return MESSAGE_BOX_RESULT_NONE;

	// NOTE: this dialog is displayed even WITHOUT any user action
	//if (!Deployment::GetCurrent ()->GetSurface ()->IsUserInitiatedEvent ())
	//	return MESSAGE_BOX_RESULT_NONE;

	GtkButtonsType bt = buttons == MESSAGE_BOX_BUTTON_OK ? GTK_BUTTONS_OK : GTK_BUTTONS_OK_CANCEL;

	GtkWidget *widget = gtk_message_dialog_new (NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_OTHER,
						bt,
						text);

	gtk_window_set_title (GTK_WINDOW (widget), caption);
	
	gint result = gtk_dialog_run (GTK_DIALOG (widget));
	gtk_widget_destroy (widget);

	switch (result) {
	case GTK_RESPONSE_OK:
		return MESSAGE_BOX_RESULT_OK;
	case GTK_RESPONSE_CANCEL:
		return MESSAGE_BOX_RESULT_CANCEL;
	case GTK_RESPONSE_YES:
		return MESSAGE_BOX_RESULT_YES;
	case GTK_RESPONSE_NO:
		return MESSAGE_BOX_RESULT_NO;
	case GTK_RESPONSE_NONE:
	default:
		return MESSAGE_BOX_RESULT_NONE;
	}
}

