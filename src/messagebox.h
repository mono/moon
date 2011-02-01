/*
 * messagebox.h: MessageBox dialog
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MESSAGEBOX_H__
#define __MESSAGEBOX_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "runtime.h"
#include "deployment.h"

G_BEGIN_DECLS

// much match values from System.Windows.MessageBoxButtons
#define MESSAGE_BOX_BUTTON_OK		0
#define MESSAGE_BOX_BUTTON_OK_CANCEL	1

// much match values from System.Windows.MessageBoxResult
#define MESSAGE_BOX_RESULT_NONE		0
#define MESSAGE_BOX_RESULT_OK		1
#define MESSAGE_BOX_RESULT_CANCEL	2
#define MESSAGE_BOX_RESULT_YES		6
#define MESSAGE_BOX_RESULT_NO		7

// older gtk+ (like 2.8 used in SLED10) don't support icon-less GTK_MESSAGE_OTHER
#ifndef GTK_MESSAGE_OTHER
#define GTK_MESSAGE_OTHER	GTK_MESSAGE_INFO
#endif

/* @GeneratePInvoke */
int message_box_show (const char *caption, const char* text, int buttons) MOON_API;

G_END_DECLS

#endif

