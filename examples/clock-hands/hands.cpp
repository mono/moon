/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <gtkmozembed.h>
#include <nsXPCOMGlue.h>

static const GREVersionRange gre_version = {
	"1.9", PR_TRUE,
	"9.9", PR_TRUE
};

int
main(int argc, char **argv)
{
	gtk_init (&argc, &argv);

	GtkWindow *window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	char xpcom_lib_path [PATH_MAX];
	char* xpcom_dir_path;

	GRE_GetGREPathWithProperties (&gre_version, 1, nsnull, 0, xpcom_lib_path, sizeof (xpcom_lib_path));
	xpcom_dir_path = g_path_get_dirname (xpcom_lib_path);

	gtk_moz_embed_set_path (xpcom_dir_path);
	g_free (xpcom_dir_path);

	GtkWidget *moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (window), moz_embed);

	gtk_widget_set_usize (moz_embed, 416, 416);

    char *current_directory = g_get_current_dir ();
    char *html_path = g_strdup_printf ("file://%s/hands.html", current_directory);
	gtk_moz_embed_load_url (GTK_MOZ_EMBED (moz_embed), html_path);

    g_free (current_directory);
    g_free (html_path);

	gtk_widget_show_all (moz_embed);
	gtk_widget_show_all (GTK_WIDGET (window));
	gtk_main ();

	return 0;
}
