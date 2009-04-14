/*
 * openfile.h: File open / save interfaces
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __OPENFILE_H__
#define __OPENFILE_H__

#include <glib.h>

G_BEGIN_DECLS

/* @GeneratePInvoke */
char **open_file_dialog_show (const char *title, bool multsel, const char *filter, int idx);

/* @GeneratePInvoke */
char *save_file_dialog_show (const char *title, const char *filter, int idx);

G_END_DECLS

#endif
